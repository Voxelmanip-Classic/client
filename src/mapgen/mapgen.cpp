/*
Minetest
Copyright (C) 2010-2018 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2013-2018 kwolekr, Ryan Kwolek <kwolekr@minetest.net>
Copyright (C) 2015-2018 paramat

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <cmath>
#include "mapgen.h"
#include "voxel.h"
#include "noise.h"
#include "gamedef.h"
#include "mapblock.h"
#include "mapnode.h"
#include "map.h"
#include "nodedef.h"
#include "emerge.h"
#include "voxelalgorithms.h"
#include "porting.h"
#include "profiler.h"
#include "settings.h"
#include "serialization.h"
#include "util/serialize.h"
#include "util/numeric.h"
#include "util/directiontables.h"
#include "filesys.h"
#include "log.h"
#include "mapgen_singlenode.h"

FlagDesc flagdesc_mapgen[] = {
	{"caves",       MG_CAVES},
	{"dungeons",    MG_DUNGEONS},
	{"light",       MG_LIGHT},
	{NULL,          0}
};

struct MapgenDesc {
	const char *name;
	bool is_user_visible;
};

////
//// Built-in mapgens
////

// Order used here defines the order of appearance in mainmenu.
// v6 always last to discourage selection.
// Special mapgens flat, fractal, singlenode, next to last. Of these, singlenode
// last to discourage selection.
// Of the remaining, v5 last due to age, v7 first due to being the default.
// The order of 'enum MapgenType' in mapgen.h must match this order.
static MapgenDesc g_reg_mapgens[] = {
	{"singlenode", true},
};

STATIC_ASSERT(
	ARRLEN(g_reg_mapgens) == MAPGEN_INVALID,
	registered_mapgens_is_wrong_size);

////
//// Mapgen
////

Mapgen::Mapgen(int mapgenid, MapgenParams *params, EmergeParams *emerge)
{
	id           = mapgenid;
	water_level  = params->water_level;
	mapgen_limit = params->mapgen_limit;
	flags        = params->flags;
	csize        = v3s16(1, 1, 1) * (params->chunksize * MAP_BLOCKSIZE);

	/*
		We are losing half our entropy by doing this, but it is necessary to
		preserve reverse compatibility.  If the top half of our current 64 bit
		seeds ever starts getting used, existing worlds will break due to a
		different hash outcome and no way to differentiate between versions.

		A solution could be to add a new bit to designate that the top half of
		the seed value should be used, essentially a 1-bit version code, but
		this would require increasing the total size of a seed to 9 bytes (yuck)

		It's probably okay if this never gets fixed.  4.2 billion possibilities
		ought to be enough for anyone.
	*/
	seed = (s32)params->seed;

	ndef      = emerge->ndef;
}


MapgenType Mapgen::getMapgenType(const std::string &mgname)
{
	for (size_t i = 0; i != ARRLEN(g_reg_mapgens); i++) {
		if (mgname == g_reg_mapgens[i].name)
			return (MapgenType)i;
	}

	return MAPGEN_INVALID;
}


const char *Mapgen::getMapgenName(MapgenType mgtype)
{
	size_t index = (size_t)mgtype;
	if (index == MAPGEN_INVALID || index >= ARRLEN(g_reg_mapgens))
		return "invalid";

	return g_reg_mapgens[index].name;
}


Mapgen *Mapgen::createMapgen(MapgenType mgtype, MapgenParams *params,
	EmergeParams *emerge)
{
	switch (mgtype) {
	case MAPGEN_SINGLENODE:
		return new MapgenSinglenode((MapgenSinglenodeParams *)params, emerge);
	default:
		return nullptr;
	}
}


MapgenParams *Mapgen::createMapgenParams(MapgenType mgtype)
{
	switch (mgtype) {
	case MAPGEN_SINGLENODE:
		return new MapgenSinglenodeParams;
	default:
		return nullptr;
	}
}


void Mapgen::getMapgenNames(std::vector<const char *> *mgnames, bool include_hidden)
{
	for (u32 i = 0; i != ARRLEN(g_reg_mapgens); i++) {
		if (include_hidden || g_reg_mapgens[i].is_user_visible)
			mgnames->push_back(g_reg_mapgens[i].name);
	}
}

void Mapgen::setDefaultSettings(Settings *settings)
{
	settings->setDefault("mg_flags", flagdesc_mapgen,
		 MG_CAVES | MG_DUNGEONS | MG_LIGHT | MG_BIOMES);

	for (int i = 0; i < (int)MAPGEN_INVALID; ++i) {
		MapgenParams *params = createMapgenParams((MapgenType)i);
		params->setDefaultSettings(settings);
		delete params;
	}
}

u32 Mapgen::getBlockSeed(v3s16 p, s32 seed)
{
	return (u32)seed   +
		p.Z * 38134234 +
		p.Y * 42123    +
		p.X * 23;
}


u32 Mapgen::getBlockSeed2(v3s16 p, s32 seed)
{
	// Multiply by unsigned number to avoid signed overflow (UB)
	u32 n = 1619U * p.X + 31337U * p.Y + 52591U * p.Z + 1013U * seed;
	n = (n >> 13) ^ n;
	return (n * (n * n * 60493 + 19990303) + 1376312589);
}


// Returns -MAX_MAP_GENERATION_LIMIT if not found
s16 Mapgen::findGroundLevel(v2s16 p2d, s16 ymin, s16 ymax)
{
	const v3s16 &em = vm->m_area.getExtent();
	u32 i = vm->m_area.index(p2d.X, ymax, p2d.Y);
	s16 y;

	for (y = ymax; y >= ymin; y--) {
		MapNode &n = vm->m_data[i];
		if (ndef->get(n).walkable)
			break;

		VoxelArea::add_y(em, i, -1);
	}
	return (y >= ymin) ? y : -MAX_MAP_GENERATION_LIMIT;
}


// Returns -MAX_MAP_GENERATION_LIMIT if not found or if ground is found first
s16 Mapgen::findLiquidSurface(v2s16 p2d, s16 ymin, s16 ymax)
{
	const v3s16 &em = vm->m_area.getExtent();
	u32 i = vm->m_area.index(p2d.X, ymax, p2d.Y);
	s16 y;

	for (y = ymax; y >= ymin; y--) {
		MapNode &n = vm->m_data[i];
		if (ndef->get(n).walkable)
			return -MAX_MAP_GENERATION_LIMIT;

		if (ndef->get(n).isLiquid())
			break;

		VoxelArea::add_y(em, i, -1);
	}
	return (y >= ymin) ? y : -MAX_MAP_GENERATION_LIMIT;
}


void Mapgen::updateHeightmap(v3s16 nmin, v3s16 nmax)
{
	if (!heightmap)
		return;

	//TimeTaker t("Mapgen::updateHeightmap", NULL, PRECISION_MICRO);
	int index = 0;
	for (s16 z = nmin.Z; z <= nmax.Z; z++) {
		for (s16 x = nmin.X; x <= nmax.X; x++, index++) {
			s16 y = findGroundLevel(v2s16(x, z), nmin.Y, nmax.Y);

			heightmap[index] = y;
		}
	}
}

void Mapgen::setLighting(u8 light, v3s16 nmin, v3s16 nmax)
{

}




////
//// MapgenParams
////


MapgenParams::~MapgenParams()
{

}


void MapgenParams::readParams(const Settings *settings)
{
	// should always be used via MapSettingsManager
	assert(settings != g_settings);

	std::string seed_str;
	if (settings->getNoEx("seed", seed_str)) {
		if (!seed_str.empty())
			seed = read_seed(seed_str.c_str());
		else
			myrand_bytes(&seed, sizeof(seed));
	}

	std::string mg_name;
	if (settings->getNoEx("mg_name", mg_name)) {
		mgtype = Mapgen::getMapgenType(mg_name);
		if (mgtype == MAPGEN_INVALID)
			mgtype = MAPGEN_DEFAULT;
	}

	settings->getS16NoEx("water_level", water_level);
	settings->getS16NoEx("mapgen_limit", mapgen_limit);
	settings->getS16NoEx("chunksize", chunksize);
	settings->getFlagStrNoEx("mg_flags", flags, flagdesc_mapgen);

	chunksize = rangelim(chunksize, 1, 10);
}


void MapgenParams::writeParams(Settings *settings) const
{
	settings->set("mg_name", Mapgen::getMapgenName(mgtype));
	settings->setU64("seed", seed);
	settings->setS16("water_level", water_level);
	settings->setS16("mapgen_limit", mapgen_limit);
	settings->setS16("chunksize", chunksize);
	settings->setFlagStr("mg_flags", flags, flagdesc_mapgen);
}


s32 MapgenParams::getSpawnRangeMax()
{
	if (!m_mapgen_edges_calculated) {
		std::pair<s16, s16> edges = get_mapgen_edges(mapgen_limit, chunksize);
		mapgen_edge_min = edges.first;
		mapgen_edge_max = edges.second;
		m_mapgen_edges_calculated = true;
	}

	return MYMIN(-mapgen_edge_min, mapgen_edge_max);
}


std::pair<s16, s16> get_mapgen_edges(s16 mapgen_limit, s16 chunksize)
{
	// Central chunk offset, in blocks
	s16 ccoff_b = -chunksize / 2;
	// Chunksize, in nodes
	s32 csize_n = chunksize * MAP_BLOCKSIZE;
	// Minp/maxp of central chunk, in nodes
	s16 ccmin = ccoff_b * MAP_BLOCKSIZE;
	s16 ccmax = ccmin + csize_n - 1;
	// Fullminp/fullmaxp of central chunk, in nodes
	s16 ccfmin = ccmin - MAP_BLOCKSIZE;
	s16 ccfmax = ccmax + MAP_BLOCKSIZE;
	// Effective mapgen limit, in blocks
	// Uses same calculation as ServerMap::blockpos_over_mapgen_limit(v3s16 p)
	s16 mapgen_limit_b = rangelim(mapgen_limit,
		0, MAX_MAP_GENERATION_LIMIT) / MAP_BLOCKSIZE;
	// Effective mapgen limits, in nodes
	s16 mapgen_limit_min = -mapgen_limit_b * MAP_BLOCKSIZE;
	s16 mapgen_limit_max = (mapgen_limit_b + 1) * MAP_BLOCKSIZE - 1;
	// Number of complete chunks from central chunk fullminp/fullmaxp
	// to effective mapgen limits.
	s16 numcmin = MYMAX((ccfmin - mapgen_limit_min) / csize_n, 0);
	s16 numcmax = MYMAX((mapgen_limit_max - ccfmax) / csize_n, 0);
	// Mapgen edges, in nodes
	return std::pair<s16, s16>(ccmin - numcmin * csize_n, ccmax + numcmax * csize_n);
}
