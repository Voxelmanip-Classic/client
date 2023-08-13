/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "common/c_types.h"
#include "content/subgames.h"
#include "gamedef.h"
#include "irrTypes.h"
#include "map.h"
#include "network/address.h"
#include "network/peerhandler.h"
#include "serverenvironment.h"
#include "util/basic_macros.h"
#include "util/thread.h"

class IItemDefManager;
class IWritableItemDefManager;
class ModChannel;
class ModChannelMgr;
class NetworkPacket;
class NodeDefManager;
class ServerModManager;
class ServerScripting;
namespace con { class Connection; }
namespace con { class Peer; }
struct ChatInterface;
struct ModSpec;
struct PackedValue;

class Server : public con::PeerHandler, public MapEventReceiver,
		public IGameDef
{
public:
	/*
		NOTE: Every public method should be thread-safe
	*/

	Server(
		const SubgameSpec &gamespec,
		bool simple_singleplayer_mode,
		Address bind_addr,
		bool dedicated,
		ChatInterface *iface = nullptr,
		std::string *shutdown_errmsg = nullptr
	);
	~Server();
	DISABLE_CLASS_COPY(Server);

	void start();
	void stop();
	// This is mainly a way to pass the time to the server.
	// Actual processing is done in another thread.
	void step(float dtime);

	/*
		Shall be called with the environment locked.
		This is accessed by the map, which is inside the environment,
		so it shouldn't be a problem.
	*/
	void onMapEditEvent(const MapEditEvent &event);

	// IGameDef interface
	// Under envlock
	virtual IItemDefManager* getItemDefManager();
	virtual const NodeDefManager* getNodeDefManager();
	virtual u16 allocateUnknownNodeId(const std::string &name);

	virtual const std::vector<ModSpec> &getMods() const;
	virtual const ModSpec* getModSpec(const std::string &modname) const;
	virtual const SubgameSpec* getGameSpec() const { return &m_gamespec; }
	static std::string getBuiltinLuaPath();

	inline void setAsyncFatalError(const std::string &error)
			{ m_async_fatal_error.set(error); }
	inline void setAsyncFatalError(const LuaError &e)
	{
		setAsyncFatalError(std::string("Lua: ") + e.what());
	}

	ServerEnvironment & getEnv() { return *m_env; }

	bool joinModChannel(const std::string &channel);
	bool leaveModChannel(const std::string &channel);
	bool sendModChannelMessage(const std::string &channel, const std::string &message);
	ModChannel *getModChannel(const std::string &channel);

	// Lua files registered for init of async env, pair of modname + path
	std::vector<std::pair<std::string, std::string>> m_async_init_files;

	// Data transferred into other Lua envs at init time
	std::unique_ptr<PackedValue> m_lua_globals_data;

private:

	void init();

	/*
		Variables
	*/
	// Subgame specification
	SubgameSpec m_gamespec;

	// Thread can set; step() will throw as ServerError
	MutexedVariable<std::string> m_async_fatal_error;

	// Environment
	ServerEnvironment *m_env = nullptr;

	// Item definition manager
	IWritableItemDefManager *m_itemdef;

	// Node definition manager
	NodeDefManager *m_nodedef;

	// Mods
	std::unique_ptr<ServerModManager> m_modmgr;

	// ModChannel manager
	std::unique_ptr<ModChannelMgr> m_modchannel_mgr;
};
