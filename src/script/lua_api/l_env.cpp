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

#include <algorithm>
#include "lua_api/l_env.h"
#include "lua_api/l_internal.h"
#include "lua_api/l_nodemeta.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "scripting_server.h"
#include "environment.h"
#include "mapblock.h"
#include "server.h"
#include "nodedef.h"
#include "daynightratio.h"
#include "util/pointedthing.h"
#include "face_position_cache.h"
#include "remoteplayer.h"
#include "util/string.h"
#include "translation.h"
#ifndef SERVER
#include "client/client.h"
#endif

///////////////////////////////////////////////////////////////////////////////



int LuaRaycast::l_next(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	bool csm = false;
#ifndef SERVER
	csm = getClient(L) != nullptr;
#endif

	LuaRaycast *o = checkObject<LuaRaycast>(L, 1);
	PointedThing pointed;
	env->continueRaycast(&o->state, &pointed);
	if (pointed.type == POINTEDTHING_NOTHING)
		lua_pushnil(L);
	else
		push_pointed_thing(L, pointed, csm, true);

	return 1;
}

int LuaRaycast::create_object(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	bool objects = true;
	bool liquids = false;

	v3f pos1 = checkFloatPos(L, 1);
	v3f pos2 = checkFloatPos(L, 2);
	if (lua_isboolean(L, 3)) {
		objects = readParam<bool>(L, 3);
	}
	if (lua_isboolean(L, 4)) {
		liquids = readParam<bool>(L, 4);
	}

	LuaRaycast *o = new LuaRaycast(core::line3d<f32>(pos1, pos2),
		objects, liquids);

	*(void **) (lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
	return 1;
}

int LuaRaycast::gc_object(lua_State *L)
{
	LuaRaycast *o = *(LuaRaycast **) (lua_touserdata(L, 1));
	delete o;
	return 0;
}

void LuaRaycast::Register(lua_State *L)
{
	static const luaL_Reg metamethods[] = {
		{"__call", l_next},
		{"__gc", gc_object},
		{0, 0}
	};
	registerClass(L, className, methods, metamethods);

	lua_register(L, className, create_object);
}

const char LuaRaycast::className[] = "Raycast";
const luaL_Reg LuaRaycast::methods[] =
{
	luamethod(LuaRaycast, next),
	{ 0, 0 }
};

// Exported functions

// get_node_light(pos, timeofday)
// pos = {x=num, y=num, z=num}
// timeofday: nil = current time, 0 = night, 0.5 = day
int ModApiEnvMod::l_get_node_light(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	// Do it
	v3s16 pos = read_v3s16(L, 1);
	u32 time_of_day = env->getTimeOfDay();
	if(lua_isnumber(L, 2))
		time_of_day = 24000.0 * lua_tonumber(L, 2);
	time_of_day %= 24000;
	u32 dnr = time_to_daynight_ratio(time_of_day, true);

	bool is_position_ok;
	MapNode n = env->getMap().getNode(pos, &is_position_ok);
	if (is_position_ok) {
		const NodeDefManager *ndef = env->getGameDef()->ndef();
		lua_pushinteger(L, n.getLightBlend(dnr, ndef->getLightingFlags(n)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

// get_node_max_level(pos)
// pos = {x=num, y=num, z=num}
int ModApiEnvMod::l_get_node_max_level(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	v3s16 pos = read_v3s16(L, 1);
	MapNode n = env->getMap().getNode(pos);
	lua_pushnumber(L, n.getMaxLevel(env->getGameDef()->ndef()));
	return 1;
}

// get_node_level(pos)
// pos = {x=num, y=num, z=num}
int ModApiEnvMod::l_get_node_level(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	v3s16 pos = read_v3s16(L, 1);
	MapNode n = env->getMap().getNode(pos);
	lua_pushnumber(L, n.getLevel(env->getGameDef()->ndef()));
	return 1;
}

// find_nodes_with_meta(pos1, pos2)
int ModApiEnvMod::l_find_nodes_with_meta(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	std::vector<v3s16> positions = env->getMap().findNodesWithMetadata(
		check_v3s16(L, 1), check_v3s16(L, 2));

	lua_createtable(L, positions.size(), 0);
	for (size_t i = 0; i != positions.size(); i++) {
		push_v3s16(L, positions[i]);
		lua_rawseti(L, -2, i + 1);
	}

	return 1;
}

// get_timeofday() -> 0...1
int ModApiEnvMod::l_get_timeofday(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	// Do it
	int timeofday_mh = env->getTimeOfDay();
	float timeofday_f = (float)timeofday_mh / 24000.0f;
	lua_pushnumber(L, timeofday_f);
	return 1;
}

void ModApiEnvMod::collectNodeIds(lua_State *L, int idx, const NodeDefManager *ndef,
	std::vector<content_t> &filter)
{
	if (lua_istable(L, idx)) {
		lua_pushnil(L);
		while (lua_next(L, idx) != 0) {
			// key at index -2 and value at index -1
			luaL_checktype(L, -1, LUA_TSTRING);
			ndef->getIds(readParam<std::string>(L, -1), filter);
			// removes value, keeps key for next iteration
			lua_pop(L, 1);
		}
	} else if (lua_isstring(L, idx)) {
		ndef->getIds(readParam<std::string>(L, 3), filter);
	}
}

// find_node_near(pos, radius, nodenames, [search_center]) -> pos or nil
// nodenames: eg. {"ignore", "group:tree"} or "default:dirt"
int ModApiEnvMod::l_find_node_near(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	const NodeDefManager *ndef = env->getGameDef()->ndef();
	Map &map = env->getMap();

	v3s16 pos = read_v3s16(L, 1);
	int radius = luaL_checkinteger(L, 2);
	std::vector<content_t> filter;
	collectNodeIds(L, 3, ndef, filter);

	int start_radius = (lua_isboolean(L, 4) && readParam<bool>(L, 4)) ? 0 : 1;

#ifndef SERVER
	// Client API limitations
	if (Client *client = getClient(L))
		radius = client->CSMClampRadius(pos, radius);
#endif

	for (int d = start_radius; d <= radius; d++) {
		const std::vector<v3s16> &list = FacePositionCache::getFacePositions(d);
		for (const v3s16 &i : list) {
			v3s16 p = pos + i;
			content_t c = map.getNode(p).getContent();
			if (CONTAINS(filter, c)) {
				push_v3s16(L, p);
				return 1;
			}
		}
	}
	return 0;
}

static void checkArea(v3s16 &minp, v3s16 &maxp)
{
	auto volume = VoxelArea(minp, maxp).getVolume();
	// Volume limit equal to 8 default mapchunks, (80 * 2) ^ 3 = 4,096,000
	if (volume > 4096000) {
		throw LuaError("Area volume exceeds allowed value of 4096000");
	}

	// Clamp to map range to avoid problems
#define CLAMP(arg) core::clamp(arg, (s16)-MAX_MAP_GENERATION_LIMIT, (s16)MAX_MAP_GENERATION_LIMIT)
	minp = v3s16(CLAMP(minp.X), CLAMP(minp.Y), CLAMP(minp.Z));
	maxp = v3s16(CLAMP(maxp.X), CLAMP(maxp.Y), CLAMP(maxp.Z));
#undef CLAMP
}

// find_nodes_in_area(minp, maxp, nodenames, [grouped])
int ModApiEnvMod::l_find_nodes_in_area(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	v3s16 minp = read_v3s16(L, 1);
	v3s16 maxp = read_v3s16(L, 2);
	sortBoxVerticies(minp, maxp);

	const NodeDefManager *ndef = env->getGameDef()->ndef();
	Map &map = env->getMap();

#ifndef SERVER
	if (Client *client = getClient(L)) {
		minp = client->CSMClampPos(minp);
		maxp = client->CSMClampPos(maxp);
	}
#endif

	checkArea(minp, maxp);

	std::vector<content_t> filter;
	collectNodeIds(L, 3, ndef, filter);

	bool grouped = lua_isboolean(L, 4) && readParam<bool>(L, 4);

	if (grouped) {
		// create the table we will be returning
		lua_createtable(L, 0, filter.size());
		int base = lua_gettop(L);

		// create one table for each filter
		std::vector<u32> idx;
		idx.resize(filter.size());
		for (u32 i = 0; i < filter.size(); i++)
			lua_newtable(L);

		map.forEachNodeInArea(minp, maxp, [&](v3s16 p, MapNode n) -> bool {
			content_t c = n.getContent();

			auto it = std::find(filter.begin(), filter.end(), c);
			if (it != filter.end()) {
				// Calculate index of the table and append the position
				u32 filt_index = it - filter.begin();
				push_v3s16(L, p);
				lua_rawseti(L, base + 1 + filt_index, ++idx[filt_index]);
			}

			return true;
		});

		// last filter table is at top of stack
		u32 i = filter.size() - 1;
		do {
			if (idx[i] == 0) {
				// No such node found -> drop the empty table
				lua_pop(L, 1);
			} else {
				// This node was found -> put table into the return table
				lua_setfield(L, base, ndef->get(filter[i]).name.c_str());
			}
		} while (i-- != 0);

		assert(lua_gettop(L) == base);
		return 1;
	} else {
		std::vector<u32> individual_count;
		individual_count.resize(filter.size());

		lua_newtable(L);
		u32 i = 0;
		map.forEachNodeInArea(minp, maxp, [&](v3s16 p, MapNode n) -> bool {
			content_t c = n.getContent();

			auto it = std::find(filter.begin(), filter.end(), c);
			if (it != filter.end()) {
				push_v3s16(L, p);
				lua_rawseti(L, -2, ++i);

				u32 filt_index = it - filter.begin();
				individual_count[filt_index]++;
			}

			return true;
		});

		lua_createtable(L, 0, filter.size());
		for (u32 i = 0; i < filter.size(); i++) {
			lua_pushinteger(L, individual_count[i]);
			lua_setfield(L, -2, ndef->get(filter[i]).name.c_str());
		}
		return 2;
	}
}

// find_nodes_in_area_under_air(minp, maxp, nodenames) -> list of positions
// nodenames: e.g. {"ignore", "group:tree"} or "default:dirt"
int ModApiEnvMod::l_find_nodes_in_area_under_air(lua_State *L)
{
	/* Note: A similar but generalized (and therefore slower) version of this
	 * function could be created -- e.g. find_nodes_in_area_under -- which
	 * would accept a node name (or ID?) or list of names that the "above node"
	 * should be.
	 * TODO
	 */

	GET_PLAIN_ENV_PTR;

	v3s16 minp = read_v3s16(L, 1);
	v3s16 maxp = read_v3s16(L, 2);
	sortBoxVerticies(minp, maxp);

	const NodeDefManager *ndef = env->getGameDef()->ndef();
	Map &map = env->getMap();

#ifndef SERVER
	if (Client *client = getClient(L)) {
		minp = client->CSMClampPos(minp);
		maxp = client->CSMClampPos(maxp);
	}
#endif

	checkArea(minp, maxp);

	std::vector<content_t> filter;
	collectNodeIds(L, 3, ndef, filter);

	lua_newtable(L);
	u32 i = 0;
	v3s16 p;
	for (p.X = minp.X; p.X <= maxp.X; p.X++)
	for (p.Z = minp.Z; p.Z <= maxp.Z; p.Z++) {
		p.Y = minp.Y;
		content_t c = map.getNode(p).getContent();
		for (; p.Y <= maxp.Y; p.Y++) {
			v3s16 psurf(p.X, p.Y + 1, p.Z);
			content_t csurf = map.getNode(psurf).getContent();
			if (c != CONTENT_AIR && csurf == CONTENT_AIR &&
					CONTAINS(filter, c)) {
				push_v3s16(L, p);
				lua_rawseti(L, -2, ++i);
			}
			c = csurf;
		}
	}
	return 1;
}

// line_of_sight(pos1, pos2) -> true/false, pos
int ModApiEnvMod::l_line_of_sight(lua_State *L)
{
	GET_PLAIN_ENV_PTR;

	// read position 1 from lua
	v3f pos1 = checkFloatPos(L, 1);
	// read position 2 from lua
	v3f pos2 = checkFloatPos(L, 2);

	v3s16 p;

	bool success = env->line_of_sight(pos1, pos2, &p);
	lua_pushboolean(L, success);
	if (!success) {
		push_v3s16(L, p);
		return 2;
	}
	return 1;
}

int ModApiEnvMod::l_raycast(lua_State *L)
{
	return LuaRaycast::create_object(L);
}

void ModApiEnvMod::Initialize(lua_State *L, int top)
{
	API_FCT(get_node_light);
	API_FCT(get_timeofday);
	API_FCT(get_node_max_level);
	API_FCT(get_node_level);
	API_FCT(find_nodes_with_meta);
	API_FCT(find_node_near);
	API_FCT(find_nodes_in_area);
	API_FCT(find_nodes_in_area_under_air);
	API_FCT(line_of_sight);
	API_FCT(raycast);
}

void ModApiEnvMod::InitializeClient(lua_State *L, int top)
{
	API_FCT(get_node_light);
	API_FCT(get_timeofday);
	API_FCT(get_node_max_level);
	API_FCT(get_node_level);
	API_FCT(find_nodes_with_meta);
	API_FCT(find_node_near);
	API_FCT(find_nodes_in_area);
	API_FCT(find_nodes_in_area_under_air);
	API_FCT(line_of_sight);
	API_FCT(raycast);
}
