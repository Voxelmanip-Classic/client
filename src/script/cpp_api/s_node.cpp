/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "cpp_api/s_node.h"
#include "cpp_api/s_internal.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "nodedef.h"
#include "server.h"
#include "environment.h"
#include "util/pointedthing.h"


// Should be ordered exactly like enum NodeDrawType in nodedef.h
struct EnumString ScriptApiNode::es_DrawType[] =
	{
		{NDT_NORMAL, "normal"},
		{NDT_AIRLIKE, "airlike"},
		{NDT_LIQUID, "liquid"},
		{NDT_FLOWINGLIQUID, "flowingliquid"},
		{NDT_GLASSLIKE, "glasslike"},
		{NDT_ALLFACES, "allfaces"},
		{NDT_ALLFACES_OPTIONAL, "allfaces_optional"},
		{NDT_TORCHLIKE, "torchlike"},
		{NDT_SIGNLIKE, "signlike"},
		{NDT_PLANTLIKE, "plantlike"},
		{NDT_FENCELIKE, "fencelike"},
		{NDT_RAILLIKE, "raillike"},
		{NDT_NODEBOX, "nodebox"},
		{NDT_GLASSLIKE_FRAMED, "glasslike_framed"},
		{NDT_FIRELIKE, "firelike"},
		{NDT_GLASSLIKE_FRAMED_OPTIONAL, "glasslike_framed_optional"},
		{NDT_MESH, "mesh"},
		{NDT_PLANTLIKE_ROOTED, "plantlike_rooted"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_ContentParamType2[] =
	{
		{CPT2_NONE, "none"},
		{CPT2_FULL, "full"},
		{CPT2_FLOWINGLIQUID, "flowingliquid"},
		{CPT2_FACEDIR, "facedir"},
		{CPT2_WALLMOUNTED, "wallmounted"},
		{CPT2_LEVELED, "leveled"},
		{CPT2_DEGROTATE, "degrotate"},
		{CPT2_MESHOPTIONS, "meshoptions"},
		{CPT2_COLOR, "color"},
		{CPT2_COLORED_FACEDIR, "colorfacedir"},
		{CPT2_COLORED_WALLMOUNTED, "colorwallmounted"},
		{CPT2_GLASSLIKE_LIQUID_LEVEL, "glasslikeliquidlevel"},
		{CPT2_COLORED_DEGROTATE, "colordegrotate"},
		{CPT2_4DIR, "4dir"},
		{CPT2_COLORED_4DIR, "color4dir"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_LiquidType[] =
	{
		{LIQUID_NONE, "none"},
		{LIQUID_FLOWING, "flowing"},
		{LIQUID_SOURCE, "source"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_ContentParamType[] =
	{
		{CPT_NONE, "none"},
		{CPT_LIGHT, "light"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_NodeBoxType[] =
	{
		{NODEBOX_REGULAR, "regular"},
		{NODEBOX_FIXED, "fixed"},
		{NODEBOX_WALLMOUNTED, "wallmounted"},
		{NODEBOX_LEVELED, "leveled"},
		{NODEBOX_CONNECTED, "connected"},
		{0, NULL},
	};

struct EnumString ScriptApiNode::es_TextureAlphaMode[] =
	{
		{ALPHAMODE_OPAQUE, "opaque"},
		{ALPHAMODE_CLIP, "clip"},
		{ALPHAMODE_BLEND, "blend"},
		{0, NULL},
	};
