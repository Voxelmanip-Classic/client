/*
 * Minetest
 * Copyright (C) 2010-2014 celeron55, Perttu Ahola <celeron55@gmail.com>
 * Copyright (C) 2010-2014 kwolekr, Ryan Kwolek <kwolekr@minetest.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this list of
 *     conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list
 *     of conditions and the following disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cmath>
#include "noise.h"
#include <iostream>
#include <cstring> // memset
#include "debug.h"
#include "util/numeric.h"
#include "util/string.h"
#include "exceptions.h"

#define NOISE_MAGIC_X    1619
#define NOISE_MAGIC_Y    31337
#define NOISE_MAGIC_Z    52591
// Unsigned magic seed prevents undefined behavior.
#define NOISE_MAGIC_SEED 1013U

FlagDesc flagdesc_noiseparams[] = {
	{"defaults",    NOISE_FLAG_DEFAULTS},
	{"eased",       NOISE_FLAG_EASED},
	{"absvalue",    NOISE_FLAG_ABSVALUE},
	{"pointbuffer", NOISE_FLAG_POINTBUFFER},
	{"simplex",     NOISE_FLAG_SIMPLEX},
	{NULL,          0}
};

///////////////////////////////////////////////////////////////////////////////

PcgRandom::PcgRandom(u64 state, u64 seq)
{
	seed(state, seq);
}

void PcgRandom::seed(u64 state, u64 seq)
{
	m_state = 0U;
	m_inc = (seq << 1u) | 1u;
	next();
	m_state += state;
	next();
}


u32 PcgRandom::next()
{
	u64 oldstate = m_state;
	m_state = oldstate * 6364136223846793005ULL + m_inc;

	u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
	u32 rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}


u32 PcgRandom::range(u32 bound)
{
	// If the bound is 0, we cover the whole RNG's range
	if (bound == 0)
		return next();

	/*
		This is an optimization of the expression:
		  0x100000000ull % bound
		since 64-bit modulo operations typically much slower than 32.
	*/
	u32 threshold = -bound % bound;
	u32 r;

	/*
		If the bound is not a multiple of the RNG's range, it may cause bias,
		e.g. a RNG has a range from 0 to 3 and we take want a number 0 to 2.
		Using rand() % 3, the number 0 would be twice as likely to appear.
		With a very large RNG range, the effect becomes less prevalent but
		still present.

		This can be solved by modifying the range of the RNG to become a
		multiple of bound by dropping values above the a threshold.

		In our example, threshold == 4 % 3 == 1, so reject values < 1
		(that is, 0), thus making the range == 3 with no bias.

		This loop may look dangerous, but will always terminate due to the
		RNG's property of uniformity.
	*/
	while ((r = next()) < threshold)
		;

	return r % bound;
}


s32 PcgRandom::range(s32 min, s32 max)
{
	if (max < min)
		throw PrngException("Invalid range (max < min)");

	// We have to cast to s64 because otherwise this could overflow,
	// and signed overflow is undefined behavior.
	u32 bound = (s64)max - (s64)min + 1;
	return range(bound) + min;
}


void PcgRandom::bytes(void *out, size_t len)
{
	u8 *outb = (u8 *)out;
	int bytes_left = 0;
	u32 r;

	while (len--) {
		if (bytes_left == 0) {
			bytes_left = sizeof(u32);
			r = next();
		}

		*outb = r & 0xFF;
		outb++;
		bytes_left--;
		r >>= CHAR_BIT;
	}
}


s32 PcgRandom::randNormalDist(s32 min, s32 max, int num_trials)
{
	s32 accum = 0;
	for (int i = 0; i != num_trials; i++)
		accum += range(min, max);
	return myround((float)accum / num_trials);
}

///////////////////////////////////////////////////////////////////////////////

float noise2d(int x, int y, s32 seed)
{
	unsigned int n = (NOISE_MAGIC_X * x + NOISE_MAGIC_Y * y
			+ NOISE_MAGIC_SEED * seed) & 0x7fffffff;
	n = (n >> 13) ^ n;
	n = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.f - (float)(int)n / 0x40000000;
}


float noise3d(int x, int y, int z, s32 seed)
{
	unsigned int n = (NOISE_MAGIC_X * x + NOISE_MAGIC_Y * y + NOISE_MAGIC_Z * z
			+ NOISE_MAGIC_SEED * seed) & 0x7fffffff;
	n = (n >> 13) ^ n;
	n = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.f - (float)(int)n / 0x40000000;
}


inline float dotProduct(float vx, float vy, float wx, float wy)
{
	return vx * wx + vy * wy;
}


inline float linearInterpolation(float v0, float v1, float t)
{
	return v0 + (v1 - v0) * t;
}


inline float biLinearInterpolation(
	float v00, float v10,
	float v01, float v11,
	float x, float y,
	bool eased)
{
	// Inlining will optimize this branch out when possible
	if (eased) {
		x = easeCurve(x);
		y = easeCurve(y);
	}
	float u = linearInterpolation(v00, v10, x);
	float v = linearInterpolation(v01, v11, x);
	return linearInterpolation(u, v, y);
}


inline float triLinearInterpolation(
	float v000, float v100, float v010, float v110,
	float v001, float v101, float v011, float v111,
	float x, float y, float z,
	bool eased)
{
	// Inlining will optimize this branch out when possible
	if (eased) {
		x = easeCurve(x);
		y = easeCurve(y);
		z = easeCurve(z);
	}
	float u = biLinearInterpolation(v000, v100, v010, v110, x, y, false);
	float v = biLinearInterpolation(v001, v101, v011, v111, x, y, false);
	return linearInterpolation(u, v, z);
}

float noise2d_gradient(float x, float y, s32 seed, bool eased)
{
	// Calculate the integer coordinates
	int x0 = myfloor(x);
	int y0 = myfloor(y);
	// Calculate the remaining part of the coordinates
	float xl = x - (float)x0;
	float yl = y - (float)y0;
	// Get values for corners of square
	float v00 = noise2d(x0, y0, seed);
	float v10 = noise2d(x0+1, y0, seed);
	float v01 = noise2d(x0, y0+1, seed);
	float v11 = noise2d(x0+1, y0+1, seed);
	// Interpolate
	return biLinearInterpolation(v00, v10, v01, v11, xl, yl, eased);
}


float noise3d_gradient(float x, float y, float z, s32 seed, bool eased)
{
	// Calculate the integer coordinates
	int x0 = myfloor(x);
	int y0 = myfloor(y);
	int z0 = myfloor(z);
	// Calculate the remaining part of the coordinates
	float xl = x - (float)x0;
	float yl = y - (float)y0;
	float zl = z - (float)z0;
	// Get values for corners of cube
	float v000 = noise3d(x0,     y0,     z0,     seed);
	float v100 = noise3d(x0 + 1, y0,     z0,     seed);
	float v010 = noise3d(x0,     y0 + 1, z0,     seed);
	float v110 = noise3d(x0 + 1, y0 + 1, z0,     seed);
	float v001 = noise3d(x0,     y0,     z0 + 1, seed);
	float v101 = noise3d(x0 + 1, y0,     z0 + 1, seed);
	float v011 = noise3d(x0,     y0 + 1, z0 + 1, seed);
	float v111 = noise3d(x0 + 1, y0 + 1, z0 + 1, seed);
	// Interpolate
	return triLinearInterpolation(
		v000, v100, v010, v110,
		v001, v101, v011, v111,
		xl, yl, zl,
		eased);
}


float noise2d_perlin(float x, float y, s32 seed,
	int octaves, float persistence, bool eased)
{
	float a = 0;
	float f = 1.0;
	float g = 1.0;
	for (int i = 0; i < octaves; i++)
	{
		a += g * noise2d_gradient(x * f, y * f, seed + i, eased);
		f *= 2.0;
		g *= persistence;
	}
	return a;
}
