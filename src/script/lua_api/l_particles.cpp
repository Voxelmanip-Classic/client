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

#include "lua_api/l_particles.h"
#include "lua_api/l_object.h"
#include "lua_api/l_internal.h"
#include "lua_api/l_particleparams.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "server.h"
#include "particles.h"

void LuaParticleParams::readTexValue(lua_State* L, ServerParticleTexture& tex)
{
	StackUnroller unroll(L);

	tex.animated = false;
	if (lua_isstring(L, -1)) {
		tex.string = lua_tostring(L, -1);
		return;
	}

	luaL_checktype(L, -1, LUA_TTABLE);
	lua_getfield(L, -1, "name");
	tex.string = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "animation");
	if (! lua_isnil(L, -1)) {
		tex.animated = true;
		tex.animation = read_animation_definition(L, -1);
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "blend");
	LuaParticleParams::readLuaValue(L, tex.blendmode);
	lua_pop(L, 1);

	LuaParticleParams::readTweenTable(L, "alpha", tex.alpha);
	LuaParticleParams::readTweenTable(L, "scale", tex.scale);

}
