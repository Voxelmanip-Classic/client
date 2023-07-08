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

#pragma once

#include "lua_api/l_base.h"
#include "serverenvironment.h"
#include "raycast.h"

// base class containing helpers
class ModApiEnvBase : public ModApiBase {
protected:

	static void collectNodeIds(lua_State *L, int idx,
		const NodeDefManager *ndef, std::vector<content_t> &filter);

	static void checkArea(v3s16 &minp, v3s16 &maxp);

	// F must be (v3s16 pos) -> MapNode
	template <typename F>
	static int findNodeNear(lua_State *L, v3s16 pos, int radius,
		const std::vector<content_t> &filter, int start_radius, F &&getNode);

	// F must be (G callback) -> void
	// with G being (v3s16 p, MapNode n) -> bool
	// and behave like Map::forEachNodeInArea
	template <typename F>
	static int findNodesInArea(lua_State *L,  const NodeDefManager *ndef,
		const std::vector<content_t> &filter, bool grouped, F &&iterate);

	// F must be (v3s16 pos) -> MapNode
	template <typename F>
	static int findNodesInAreaUnderAir(lua_State *L, v3s16 minp, v3s16 maxp,
		const std::vector<content_t> &filter, F &&getNode);

	static const EnumString es_ClearObjectsMode[];
	static const EnumString es_BlockStatusType[];

};

class ModApiEnv : public ModApiEnvBase {
private:
	// get_node_light(pos, timeofday)
	// pos = {x=num, y=num, z=num}
	// timeofday: nil = current time, 0 = night, 0.5 = day
	static int l_get_node_light(lua_State *L);

	// get_node_max_level(pos)
	// pos = {x=num, y=num, z=num}
	static int l_get_node_max_level(lua_State *L);

	// get_node_level(pos)
	// pos = {x=num, y=num, z=num}
	static int l_get_node_level(lua_State *L);

	// find_nodes_with_meta(pos1, pos2)
	static int l_find_nodes_with_meta(lua_State *L);

	// get_timeofday() -> 0...1
	static int l_get_timeofday(lua_State *L);

	// find_node_near(pos, radius, nodenames, search_center) -> pos or nil
	// nodenames: eg. {"ignore", "group:tree"} or "default:dirt"
	static int l_find_node_near(lua_State *L);

	// find_nodes_in_area(minp, maxp, nodenames) -> list of positions
	// nodenames: eg. {"ignore", "group:tree"} or "default:dirt"
	static int l_find_nodes_in_area(lua_State *L);

	// find_surface_nodes_in_area(minp, maxp, nodenames) -> list of positions
	// nodenames: eg. {"ignore", "group:tree"} or "default:dirt"
	static int l_find_nodes_in_area_under_air(lua_State *L);

	// line_of_sight(pos1, pos2) -> true/false
	static int l_line_of_sight(lua_State *L);

	// raycast(pos1, pos2, objects, liquids) -> Raycast
	static int l_raycast(lua_State *L);

	/* Helpers */

	static void collectNodeIds(lua_State *L, int idx,
		const NodeDefManager *ndef, std::vector<content_t> &filter);

public:
	static void Initialize(lua_State *L, int top);
	static void InitializeClient(lua_State *L, int top);
};

//! Lua wrapper for RaycastState objects
class LuaRaycast : public ModApiBase
{
private:
	static const luaL_Reg methods[];
	//! Inner state
	RaycastState state;

	// Exported functions

	// garbage collector
	static int gc_object(lua_State *L);

	/*!
	 * Raycast:next() -> pointed_thing
	 * Returns the next pointed thing on the ray.
	 */
	static int l_next(lua_State *L);
public:
	//! Constructor with the same arguments as RaycastState.
	LuaRaycast(
		const core::line3d<f32> &shootline,
		bool objects_pointable,
		bool liquids_pointable) :
		state(shootline, objects_pointable, liquids_pointable)
	{}

	//! Creates a LuaRaycast and leaves it on top of the stack.
	static int create_object(lua_State *L);

	//! Registers Raycast as a Lua userdata type.
	static void Register(lua_State *L);

	static const char className[];
};

struct ScriptCallbackState {
	ServerScripting *script;
	int callback_ref;
	int args_ref;
	unsigned int refcount;
	std::string origin;
};
