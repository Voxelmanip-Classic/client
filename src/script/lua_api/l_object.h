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
#include "irrlichttypes.h"

class ServerActiveObject;
class LuaEntitySAO;
class PlayerSAO;
class RemotePlayer;

/*
	ObjectRef
*/

class ObjectRef : public ModApiBase {
public:
	ObjectRef(ServerActiveObject *object);

	~ObjectRef() = default;

	// Creates an ObjectRef and leaves it on top of stack
	// Not callable from Lua; all references are created on the C side.
	static void create(lua_State *L, ServerActiveObject *object);

	static void set_null(lua_State *L);

	static void Register(lua_State *L);

	static ServerActiveObject* getobject(ObjectRef *ref);

	static const char className[];
private:
	ServerActiveObject *m_object = nullptr;
	static luaL_Reg methods[];


	static LuaEntitySAO* getluaobject(ObjectRef *ref);

	static PlayerSAO* getplayersao(ObjectRef *ref);

	static RemotePlayer *getplayer(ObjectRef *ref);

	// Exported functions

	// garbage collector
	static int gc_object(lua_State *L);

	// remove(self)
	static int l_remove(lua_State *L);

	// get_pos(self)
	static int l_get_pos(lua_State *L);

	// set_pos(self, pos)
	static int l_set_pos(lua_State *L);

};
