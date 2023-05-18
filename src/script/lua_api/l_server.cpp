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

#include "lua_api/l_server.h"
#include "lua_api/l_internal.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "common/c_packer.h"
#include "cpp_api/s_base.h"
#include "cpp_api/s_security.h"
#include "scripting_server.h"
#include "server.h"
#include "environment.h"
#include "remoteplayer.h"
#include "log.h"
#include <algorithm>

// request_shutdown()
int ModApiServer::l_request_shutdown(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *msg = lua_tolstring(L, 1, NULL);
	bool reconnect = readParam<bool>(L, 2);
	float seconds_before_shutdown = lua_tonumber(L, 3);
	getServer(L)->requestShutdown(msg ? msg : "", reconnect, seconds_before_shutdown);
	return 0;
}

// get_server_status()
int ModApiServer::l_get_server_status(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	lua_pushstring(L, getServer(L)->getStatusString().c_str());
	return 1;
}

// get_server_uptime()
int ModApiServer::l_get_server_uptime(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	lua_pushnumber(L, getServer(L)->getUptime());
	return 1;
}

// get_server_max_lag()
int ModApiServer::l_get_server_max_lag(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	GET_ENV_PTR;
	lua_pushnumber(L, env->getMaxLagEstimate());
	return 1;
}

// print(text)
int ModApiServer::l_print(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string text;
	text = luaL_checkstring(L, 1);
	getServer(L)->printToConsoleOnly(text);
	return 0;
}

// chat_send_all(text)
int ModApiServer::l_chat_send_all(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *text = luaL_checkstring(L, 1);
	// Get server from registry
	Server *server = getServer(L);
	// Send
	try {
		server->notifyPlayers(utf8_to_wide(text));
	} catch (PacketError &e) {
		warningstream << "Exception caught: " << e.what() << std::endl
			<< script_get_backtrace(L) << std::endl;
		server->notifyPlayers(utf8_to_wide(std::string("Internal error: ") + e.what()));
	}

	return 0;
}

// chat_send_player(name, text)
int ModApiServer::l_chat_send_player(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *name = luaL_checkstring(L, 1);
	const char *text = luaL_checkstring(L, 2);

	// Get server from registry
	Server *server = getServer(L);
	// Send
	try {
		server->notifyPlayer(name, utf8_to_wide(text));
	} catch (PacketError &e) {
		warningstream << "Exception caught: " << e.what() << std::endl
			<< script_get_backtrace(L) << std::endl;
		server->notifyPlayer(name, utf8_to_wide(std::string("Internal error: ") + e.what()));
	}
	return 0;
}

// disconnect_player(name, [reason]) -> success
int ModApiServer::l_disconnect_player(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	if (!getEnv(L))
		throw LuaError("Can't kick player before server has started up");

	const char *name = luaL_checkstring(L, 1);
	std::string message;
	if (lua_isstring(L, 2))
		message.append(readParam<std::string>(L, 2));
	else
		message.append("Disconnected.");

	Server *server = getServer(L);

	RemotePlayer *player = server->getEnv().getPlayer(name);
	if (!player) {
		lua_pushboolean(L, false); // No such player
		return 1;
	}

	server->DenyAccess(player->getPeerId(), SERVER_ACCESSDENIED_CUSTOM_STRING, message);
	lua_pushboolean(L, true);
	return 1;
}

int ModApiServer::l_remove_player(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string name = luaL_checkstring(L, 1);
	ServerEnvironment *s_env = dynamic_cast<ServerEnvironment *>(getEnv(L));
	if (!s_env)
		throw LuaError("Can't remove player before server has started up");

	RemotePlayer *player = s_env->getPlayer(name.c_str());
	if (!player)
		lua_pushinteger(L, s_env->removePlayerFromDatabase(name) ? 0 : 1);
	else
		lua_pushinteger(L, 2);

	return 1;
}

// show_formspec(playername,formname,formspec)
int ModApiServer::l_show_formspec(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const char *playername = luaL_checkstring(L, 1);
	const char *formname = luaL_checkstring(L, 2);
	const char *formspec = luaL_checkstring(L, 3);

	if(getServer(L)->showFormspec(playername,formspec,formname))
	{
		lua_pushboolean(L, true);
	}else{
		lua_pushboolean(L, false);
	}
	return 1;
}

// get_current_modname()
int ModApiServer::l_get_current_modname(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_CURRENT_MOD_NAME);
	return 1;
}

// get_modpath(modname)
int ModApiServer::l_get_modpath(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string modname = luaL_checkstring(L, 1);
	const ModSpec *mod = getGameDef(L)->getModSpec(modname);
	if (!mod)
		lua_pushnil(L);
	else
		lua_pushstring(L, mod->path.c_str());
	return 1;
}

// get_modnames()
// the returned list is sorted alphabetically for you
int ModApiServer::l_get_modnames(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	// Get a list of mods
	std::vector<std::string> modlist;
	for (auto &it : getGameDef(L)->getMods())
		modlist.emplace_back(it.name);

	std::sort(modlist.begin(), modlist.end());

	// Package them up for Lua
	lua_createtable(L, modlist.size(), 0);
	auto iter = modlist.begin();
	for (u16 i = 0; iter != modlist.end(); ++iter) {
		lua_pushstring(L, iter->c_str());
		lua_rawseti(L, -2, ++i);
	}
	return 1;
}

// get_game_info()
int ModApiServer::l_get_game_info(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const SubgameSpec *game_spec = getGameDef(L)->getGameSpec();
	assert(game_spec);
	lua_newtable(L);
	setstringfield(L, -1, "id", game_spec->id);
	setstringfield(L, -1, "title", game_spec->title);
	setstringfield(L, -1, "author", game_spec->author);
	setstringfield(L, -1, "path", game_spec->path);
	return 1;
}

// get_worldpath()
int ModApiServer::l_get_worldpath(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const Server *srv = getServer(L);
	lua_pushstring(L, srv->getWorldPath().c_str());
	return 1;
}

// sound_play(spec, parameters, [ephemeral])
int ModApiServer::l_sound_play(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	ServerPlayingSound params;
	read_soundspec(L, 1, params.spec);
	read_server_sound_params(L, 2, params);
	bool ephemeral = lua_gettop(L) > 2 && readParam<bool>(L, 3);
	if (ephemeral) {
		getServer(L)->playSound(params, true);
		lua_pushnil(L);
	} else {
		s32 handle = getServer(L)->playSound(params);
		lua_pushinteger(L, handle);
	}
	return 1;
}

// sound_stop(handle)
int ModApiServer::l_sound_stop(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	s32 handle = luaL_checkinteger(L, 1);
	getServer(L)->stopSound(handle);
	return 0;
}

int ModApiServer::l_sound_fade(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	s32 handle = luaL_checkinteger(L, 1);
	float step = readParam<float>(L, 2);
	float gain = readParam<float>(L, 3);
	getServer(L)->fadeSound(handle, step, gain);
	return 0;
}

// is_singleplayer()
int ModApiServer::l_is_singleplayer(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	const Server *srv = getServer(L);
	lua_pushboolean(L, srv->isSingleplayer());
	return 1;
}

// notify_authentication_modified(name)
int ModApiServer::l_notify_authentication_modified(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	std::string name;
	if(lua_isstring(L, 1))
		name = readParam<std::string>(L, 1);
	getServer(L)->reportPrivsModified(name);
	return 0;
}

// do_async_callback(func, params, mod_origin)
int ModApiServer::l_do_async_callback(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;
	ServerScripting *script = getScriptApi<ServerScripting>(L);

	luaL_checktype(L, 1, LUA_TFUNCTION);
	luaL_checktype(L, 2, LUA_TTABLE);
	luaL_checktype(L, 3, LUA_TSTRING);

	call_string_dump(L, 1);
	size_t func_length;
	const char *serialized_func_raw = lua_tolstring(L, -1, &func_length);

	PackedValue *param = script_pack(L, 2);

	std::string mod_origin = readParam<std::string>(L, 3);

	u32 jobId = script->queueAsync(
		std::string(serialized_func_raw, func_length),
		param, mod_origin);

	lua_settop(L, 0);
	lua_pushinteger(L, jobId);
	return 1;
}

// register_async_dofile(path)
int ModApiServer::l_register_async_dofile(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	std::string path = readParam<std::string>(L, 1);
	CHECK_SECURE_PATH(L, path.c_str(), false);

	// Find currently running mod name (only at init time)
	lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_CURRENT_MOD_NAME);
	if (!lua_isstring(L, -1))
		return 0;
	std::string modname = readParam<std::string>(L, -1);

	getServer(L)->m_async_init_files.emplace_back(modname, path);
	lua_pushboolean(L, true);
	return 1;
}

// serialize_roundtrip(value)
// Meant for unit testing the packer from Lua
int ModApiServer::l_serialize_roundtrip(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	int top = lua_gettop(L);
	auto *pv = script_pack(L, 1);
	if (top != lua_gettop(L))
		throw LuaError("stack values leaked");

#ifndef NDEBUG
	script_dump_packed(pv);
#endif

	top = lua_gettop(L);
	script_unpack(L, pv);
	delete pv;
	if (top + 1 != lua_gettop(L))
		throw LuaError("stack values leaked");

	return 1;
}

void ModApiServer::Initialize(lua_State *L, int top)
{
	API_FCT(request_shutdown);
	API_FCT(get_server_status);
	API_FCT(get_server_uptime);
	API_FCT(get_server_max_lag);
	API_FCT(get_worldpath);
	API_FCT(is_singleplayer);

	API_FCT(get_current_modname);
	API_FCT(get_modpath);
	API_FCT(get_modnames);
	API_FCT(get_game_info);

	API_FCT(print);

	API_FCT(chat_send_all);
	API_FCT(chat_send_player);
	API_FCT(show_formspec);
	API_FCT(sound_play);
	API_FCT(sound_stop);
	API_FCT(sound_fade);

	API_FCT(disconnect_player);
	API_FCT(remove_player);
	API_FCT(notify_authentication_modified);

	API_FCT(do_async_callback);
	API_FCT(register_async_dofile);
	API_FCT(serialize_roundtrip);
}

void ModApiServer::InitializeAsync(lua_State *L, int top)
{
	API_FCT(get_worldpath);
	API_FCT(is_singleplayer);

	API_FCT(get_current_modname);
	API_FCT(get_modpath);
	API_FCT(get_modnames);
	API_FCT(get_game_info);
}
