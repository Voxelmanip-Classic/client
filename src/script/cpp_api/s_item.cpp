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

#include "cpp_api/s_item.h"
#include "cpp_api/s_internal.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "lua_api/l_item.h"
#include "lua_api/l_inventory.h"
#include "server.h"
#include "log.h"
#include "util/pointedthing.h"
#include "inventory.h"
#include "inventorymanager.h"

#define WRAP_LUAERROR(e, detail) \
	LuaError(std::string(__FUNCTION__) + ": " + (e).what() + ". " detail)

bool ScriptApiItem::item_OnDrop(ItemStack &item,
		ServerActiveObject *dropper, v3f pos)
{
	SCRIPTAPI_PRECHECKHEADER

	int error_handler = PUSH_ERROR_HANDLER(L);

	// Push callback function on stack
	if (!getItemCallback(item.name.c_str(), "on_drop"))
		return false;

	// Call function
	LuaItemStack::create(L, item);
	objectrefGetOrCreate(L, dropper);
	pushFloatPos(L, pos);
	PCALL_RES(lua_pcall(L, 3, 1, error_handler));
	if (!lua_isnil(L, -1)) {
		try {
			item = read_item(L, -1, getServer()->idef());
		} catch (LuaError &e) {
			throw WRAP_LUAERROR(e, "item=" + item.name);
		}
	}
	lua_pop(L, 2);  // Pop item and error handler
	return true;
}

// Retrieves core.registered_items[name][callbackname]
// If that is nil or on error, return false and stack is unchanged
// If that is a function, returns true and pushes the
// function onto the stack
// If core.registered_items[name] doesn't exist, core.nodedef_default
// is tried instead so unknown items can still be manipulated to some degree
bool ScriptApiItem::getItemCallback(const char *name, const char *callbackname,
		const v3s16 *p)
{
	lua_State* L = getStack();

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_items");
	lua_remove(L, -2); // Remove core
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_getfield(L, -1, name);
	lua_remove(L, -2); // Remove registered_items
	// Should be a table
	if (lua_type(L, -1) != LUA_TTABLE) {
		// Report error and clean up
		errorstream << "Item \"" << name << "\" not defined";
		if (p)
			errorstream << " at position " << PP(*p);
		errorstream << std::endl;
		lua_pop(L, 1);

		// Try core.nodedef_default instead
		lua_getglobal(L, "core");
		lua_getfield(L, -1, "nodedef_default");
		lua_remove(L, -2);
		luaL_checktype(L, -1, LUA_TTABLE);
	}

	setOriginFromTable(-1);

	lua_getfield(L, -1, callbackname);
	lua_remove(L, -2); // Remove item def
	// Should be a function or nil
	if (lua_type(L, -1) == LUA_TFUNCTION) {
		return true;
	}

	if (!lua_isnil(L, -1)) {
		errorstream << "Item \"" << name << "\" callback \""
			<< callbackname << "\" is not a function" << std::endl;
	}
	lua_pop(L, 1);
	return false;
}
