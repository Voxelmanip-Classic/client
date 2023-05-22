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
class PlayerSAO;
class ServerInventoryManager;
class ServerModManager;
class ServerScripting;
class ServerThread;
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
		const std::string &path_world,
		const SubgameSpec &gamespec,
		bool simple_singleplayer_mode,
		Address bind_addr,
		bool dedicated,
		ChatInterface *iface = nullptr,
		std::string *on_shutdown_errmsg = nullptr
	);
	~Server();
	DISABLE_CLASS_COPY(Server);

	void start();
	void stop();
	// This is mainly a way to pass the time to the server.
	// Actual processing is done in another thread.
	void step(float dtime);

	void Receive();

	/*
	 * Command Handlers
	 */

	void Send(NetworkPacket *pkt);
	void Send(session_t peer_id, NetworkPacket *pkt);

	// Both setter and getter need no envlock,
	// can be called freely from threads
	void setTimeOfDay(u32 time);

	/*
		Shall be called with the environment locked.
		This is accessed by the map, which is inside the environment,
		so it shouldn't be a problem.
	*/
	void onMapEditEvent(const MapEditEvent &event);

	// Returns -1 if failed, sound handle on success
	// Envlock
	void stopSound(s32 handle);
	void fadeSound(s32 handle, float step, float gain);

	ServerInventoryManager *getInventoryMgr() const { return m_inventory_mgr.get(); }

	// Envlock and conlock should be locked when using scriptapi
	ServerScripting *getScriptIface(){ return m_script; }

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

	Map & getMap() { return m_env->getMap(); }
	ServerEnvironment & getEnv() { return *m_env; }

	/* con::PeerHandler implementation. */
	void peerAdded(con::Peer *peer);
	void deletingPeer(con::Peer *peer, bool timeout);

	bool joinModChannel(const std::string &channel);
	bool leaveModChannel(const std::string &channel);
	bool sendModChannelMessage(const std::string &channel, const std::string &message);
	ModChannel *getModChannel(const std::string &channel);

	// Lua files registered for init of async env, pair of modname + path
	std::vector<std::pair<std::string, std::string>> m_async_init_files;

	// Data transferred into async envs at init time
	std::unique_ptr<PackedValue> m_async_globals_data;

	// Bind address
	Address m_bind_addr;

private:

	void init();

	/*
		Something random
	*/

	// When called, environment mutex should be locked
	PlayerSAO *getPlayerSAO(session_t peer_id);

	/*
		Variables
	*/
	// World directory
	std::string m_path_world;
	// Subgame specification
	SubgameSpec m_gamespec;

	// Thread can set; step() will throw as ServerError
	MutexedVariable<std::string> m_async_fatal_error;

	// Environment
	ServerEnvironment *m_env = nullptr;

	// server connection
	std::shared_ptr<con::Connection> m_con;

	// Scripting
	// Envlock and conlock should be locked when using Lua
	ServerScripting *m_script = nullptr;

	// Item definition manager
	IWritableItemDefManager *m_itemdef;

	// Node definition manager
	NodeDefManager *m_nodedef;

	// Mods
	std::unique_ptr<ServerModManager> m_modmgr;

	// The server mainly operates in this thread
	ServerThread *m_thread = nullptr;

	// ModChannel manager
	std::unique_ptr<ModChannelMgr> m_modchannel_mgr;

	// Inventory manager
	std::unique_ptr<ServerInventoryManager> m_inventory_mgr;
};
