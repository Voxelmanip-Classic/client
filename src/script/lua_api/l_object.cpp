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

#include "lua_api/l_object.h"
#include <cmath>
#include "lua_api/l_internal.h"
#include "lua_api/l_item.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "log.h"
#include "tool.h"
#include "server.h"
#include "hud.h"
#include "scripting_server.h"

/*
	ObjectRef
*/


ServerActiveObject* ObjectRef::getobject(ObjectRef *ref)
{
	ServerActiveObject *sao = ref->m_object;
	if (sao && sao->isGone())
		return nullptr;
	return sao;
}

// Exported functions

// garbage collector
int ObjectRef::gc_object(lua_State *L) {
	ObjectRef *obj = *(ObjectRef **)(lua_touserdata(L, 1));
	delete obj;
	return 0;
}

// remove(self)
int ObjectRef::l_remove(lua_State *L)
{
	GET_ENV_PTR;

	return 0;
}

ObjectRef::ObjectRef(ServerActiveObject *object):
	m_object(object)
{}

// Creates an ObjectRef and leaves it on top of stack
// Not callable from Lua; all references are created on the C side.
void ObjectRef::create(lua_State *L, ServerActiveObject *object)
{
	ObjectRef *obj = new ObjectRef(object);
	*(void **)(lua_newuserdata(L, sizeof(void *))) = obj;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

void ObjectRef::Register(lua_State *L)
{
	static const luaL_Reg metamethods[] = {
		{"__gc", gc_object},
		{0, 0}
	};
	registerClass(L, className, methods, metamethods);
}

const char ObjectRef::className[] = "ObjectRef";
luaL_Reg ObjectRef::methods[] = {
	// ServerActiveObject
	luamethod(ObjectRef, remove),

	{0,0}
};
