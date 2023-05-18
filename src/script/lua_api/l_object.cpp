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
#include "lua_api/l_inventory.h"
#include "lua_api/l_item.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "log.h"
#include "tool.h"
#include "remoteplayer.h"
#include "server.h"
#include "hud.h"
#include "scripting_server.h"
#include "server/luaentity_sao.h"
#include "server/player_sao.h"
#include "server/serverinventorymgr.h"

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

LuaEntitySAO* ObjectRef::getluaobject(ObjectRef *ref)
{
	ServerActiveObject *sao = getobject(ref);
	if (sao == nullptr)
		return nullptr;
	if (sao->getType() != ACTIVEOBJECT_TYPE_LUAENTITY)
		return nullptr;
	return (LuaEntitySAO*)sao;
}

PlayerSAO* ObjectRef::getplayersao(ObjectRef *ref)
{
	ServerActiveObject *sao = getobject(ref);
	if (sao == nullptr)
		return nullptr;
	if (sao->getType() != ACTIVEOBJECT_TYPE_PLAYER)
		return nullptr;
	return (PlayerSAO*)sao;
}

RemotePlayer *ObjectRef::getplayer(ObjectRef *ref)
{
	PlayerSAO *playersao = getplayersao(ref);
	if (playersao == nullptr)
		return nullptr;
	return playersao->getPlayer();
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

	ObjectRef *ref = checkObject<ObjectRef>(L, 1);
	ServerActiveObject *sao = getobject(ref);
	if (sao == nullptr)
		return 0;
	if (sao->getType() == ACTIVEOBJECT_TYPE_PLAYER)
		return 0;

	sao->clearChildAttachments();
	sao->clearParentAttachment();

	verbosestream << "ObjectRef::l_remove(): id=" << sao->getId() << std::endl;
	sao->markForRemoval();
	return 0;
}

// get_pos(self)
int ObjectRef::l_get_pos(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	ObjectRef *ref = checkObject<ObjectRef>(L, 1);
	ServerActiveObject *sao = getobject(ref);
	if (sao == nullptr)
		return 0;

	push_v3f(L, sao->getBasePosition() / BS);
	return 1;
}

// set_pos(self, pos)
int ObjectRef::l_set_pos(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	ObjectRef *ref = checkObject<ObjectRef>(L, 1);
	ServerActiveObject *sao = getobject(ref);
	if (sao == nullptr)
		return 0;

	v3f pos = checkFloatPos(L, 2);

	sao->setPos(pos);
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

void ObjectRef::set_null(lua_State *L)
{
	ObjectRef *obj = checkObject<ObjectRef>(L, -1);
	obj->m_object = nullptr;
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
	luamethod_aliased(ObjectRef, get_pos, getpos),
	luamethod_aliased(ObjectRef, set_pos, setpos),

	{0,0}
};
