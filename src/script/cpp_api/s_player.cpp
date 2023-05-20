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

#include "cpp_api/s_player.h"
#include "cpp_api/s_internal.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "debug.h"
#include "inventorymanager.h"
#include "lua_api/l_inventory.h"
#include "lua_api/l_item.h"
#include "util/string.h"

void ScriptApiPlayer::pushMoveArguments(
		const MoveAction &ma, int count,
		ServerActiveObject *player)
{
	lua_State *L = getStack();
	objectrefGetOrCreate(L, player); // player
	lua_pushstring(L, "move");       // action
	InvRef::create(L, ma.from_inv);  // inventory
	lua_newtable(L);
	{
		// Table containing the action information
		lua_pushstring(L, ma.from_list.c_str());
		lua_setfield(L, -2, "from_list");
		lua_pushstring(L, ma.to_list.c_str());
		lua_setfield(L, -2, "to_list");

		lua_pushinteger(L, ma.from_i + 1);
		lua_setfield(L, -2, "from_index");
		lua_pushinteger(L, ma.to_i + 1);
		lua_setfield(L, -2, "to_index");

		lua_pushinteger(L, count);
		lua_setfield(L, -2, "count");
	}
}

void ScriptApiPlayer::pushPutTakeArguments(
		const char *method, const InventoryLocation &loc,
		const std::string &listname, int index, const ItemStack &stack,
		ServerActiveObject *player)
{
	lua_State *L = getStack();
	objectrefGetOrCreate(L, player); // player
	lua_pushstring(L, method);       // action
	InvRef::create(L, loc);          // inventory
	lua_newtable(L);
	{
		// Table containing the action information
		lua_pushstring(L, listname.c_str());
		lua_setfield(L, -2, "listname");

		lua_pushinteger(L, index + 1);
		lua_setfield(L, -2, "index");

		LuaItemStack::create(L, stack);
		lua_setfield(L, -2, "stack");
	}
}

// Return number of accepted items to be moved
int ScriptApiPlayer::player_inventory_AllowMove(
		const MoveAction &ma, int count,
		ServerActiveObject *player)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_allow_player_inventory_actions");
	pushMoveArguments(ma, count, player);
	runCallbacks(4, RUN_CALLBACKS_MODE_OR_SC);

	return lua_type(L, -1) == LUA_TNUMBER ? lua_tonumber(L, -1) : count;
}

// Return number of accepted items to be put
int ScriptApiPlayer::player_inventory_AllowPut(
		const MoveAction &ma, const ItemStack &stack,
		ServerActiveObject *player)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_allow_player_inventory_actions");
	pushPutTakeArguments("put", ma.to_inv, ma.to_list, ma.to_i, stack, player);
	runCallbacks(4, RUN_CALLBACKS_MODE_OR_SC);

	return lua_type(L, -1) == LUA_TNUMBER ? lua_tonumber(L, -1) : stack.count;
}

// Return number of accepted items to be taken
int ScriptApiPlayer::player_inventory_AllowTake(
		const MoveAction &ma, const ItemStack &stack,
		ServerActiveObject *player)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_allow_player_inventory_actions");
	pushPutTakeArguments("take", ma.from_inv, ma.from_list, ma.from_i, stack, player);
	runCallbacks(4, RUN_CALLBACKS_MODE_OR_SC);

	return lua_type(L, -1) == LUA_TNUMBER ? lua_tonumber(L, -1) : stack.count;
}

// Report moved items
void ScriptApiPlayer::player_inventory_OnMove(
		const MoveAction &ma, int count,
		ServerActiveObject *player)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_player_inventory_actions");
	pushMoveArguments(ma, count, player);
	runCallbacks(4, RUN_CALLBACKS_MODE_FIRST);
}

// Report put items
void ScriptApiPlayer::player_inventory_OnPut(
		const MoveAction &ma, const ItemStack &stack,
		ServerActiveObject *player)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_player_inventory_actions");
	pushPutTakeArguments("put", ma.to_inv, ma.to_list, ma.to_i, stack, player);
	runCallbacks(4, RUN_CALLBACKS_MODE_FIRST);
}

// Report taken items
void ScriptApiPlayer::player_inventory_OnTake(
		const MoveAction &ma, const ItemStack &stack,
		ServerActiveObject *player)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_on_player_inventory_actions");
	pushPutTakeArguments("take", ma.from_inv, ma.from_list, ma.from_i, stack, player);
	runCallbacks(4, RUN_CALLBACKS_MODE_FIRST);
}
