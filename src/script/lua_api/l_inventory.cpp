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

#include "lua_api/l_inventory.h"
#include "lua_api/l_internal.h"
#include "lua_api/l_item.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "server.h"
#include "remoteplayer.h"

/*
	InvRef
*/

// Exported functions

// garbage collector
int InvRef::gc_object(lua_State *L) {
	InvRef *o = *(InvRef **)(lua_touserdata(L, 1));
	delete o;
	return 0;
}

// is_empty(self, listname) -> true/false
int InvRef::l_is_empty(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	return 1;
}

InvRef::InvRef(const InventoryLocation &loc):
	m_loc(loc)
{
}

// Creates an InvRef and leaves it on top of stack
// Not callable from Lua; all references are created on the C side.
void InvRef::create(lua_State *L, const InventoryLocation &loc)
{
	NO_MAP_LOCK_REQUIRED;
	InvRef *o = new InvRef(loc);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

void InvRef::Register(lua_State *L)
{
	static const luaL_Reg metamethods[] = {
		{"__gc", gc_object},
		{0, 0}
	};
	registerClass(L, className, methods, metamethods);

	// Cannot be created from Lua
	//lua_register(L, className, create_object);
}

const char InvRef::className[] = "InvRef";
const luaL_Reg InvRef::methods[] = {
	luamethod(InvRef, is_empty),
	{0,0}
};
