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

#include "server.h"
#include <iostream>
#include "network/connection.h"
#include "network/networkprotocol.h"
#include "constants.h"
#include "filesys.h"
#include "settings.h"
#include "scripting_server.h"
#include "nodedef.h"
#include "itemdef.h"
#include "modchannels.h"
#include "util/string.h"
#include "util/serialize.h"
#include "server/mods.h"
#include "chatmessage.h"
#include "chat_interface.h"
#include "remoteplayer.h"
#include "server/player_sao.h"
#include "server/serverinventorymgr.h"
#include "database/database-files.h"
#include "database/database-dummy.h"

class ClientNotFoundException : public BaseException
{
public:
	ClientNotFoundException(const char *s):
		BaseException(s)
	{}
};

class ServerThread : public Thread
{
public:

	ServerThread(Server *server):
		Thread("Server"),
		m_server(server)
	{}

	void *run();

private:
	Server *m_server;
};

void *ServerThread::run()
{

	return nullptr;
}

/*
	Server
*/

Server::Server(
		const std::string &path_world,
		const SubgameSpec &gamespec,
		bool simple_singleplayer_mode,
		Address bind_addr,
		bool dedicated,
		ChatInterface *iface,
		std::string *on_shutdown_errmsg
	):
	m_bind_addr(bind_addr),
	m_path_world(path_world),
	m_gamespec(gamespec),
	m_simple_singleplayer_mode(simple_singleplayer_mode),
	m_dedicated(dedicated),
	m_async_fatal_error(""),
	m_con(std::make_shared<con::Connection>(PROTOCOL_ID,
			512,
			CONNECTION_TIMEOUT,
			m_bind_addr.isIPv6(),
			this)),
	m_itemdef(createItemDefManager()),
	m_nodedef(createNodeDefManager()),
	m_thread(new ServerThread(this)),
	m_clients(m_con),
	m_admin_chat(iface),
	m_on_shutdown_errmsg(on_shutdown_errmsg),
	m_modchannel_mgr(new ModChannelMgr())
{

}

Server::~Server()
{

}

void Server::init()
{

}

void Server::start()
{

}

void Server::stop()
{

}

void Server::step(float dtime)
{

}

void Server::AsyncRunStep(bool initial_step)
{


}

void Server::Receive()
{

}

void Server::ProcessData(NetworkPacket *pkt)
{

}

void Server::setTimeOfDay(u32 time)
{

}

void Server::onMapEditEvent(const MapEditEvent &event)
{

}

void Server::peerAdded(con::Peer *peer)
{

}

void Server::deletingPeer(con::Peer *peer, bool timeout)
{

}

bool Server::getClientConInfo(session_t peer_id, con::rtt_stat_type type, float* retval)
{
	return 0;
}

bool Server::getClientInfo(session_t peer_id, ClientInfo &ret)
{
	return true;
}

const ClientDynamicInfo *Server::getClientDynamicInfo(session_t peer_id)
{
	ClientInterface::AutoLock clientlock(m_clients);
	RemoteClient *client = m_clients.lockedGetClientNoEx(peer_id, CS_Invalid);

	if (!client)
		return nullptr;

	return &client->getDynamicInfo();
}

void Server::handlePeerChanges()
{

}

void Server::Send(NetworkPacket *pkt)
{

}

void Server::Send(session_t peer_id, NetworkPacket *pkt)
{

}

void Server::SendMovement(session_t peer_id)
{

}

void Server::HandlePlayerHPChange(PlayerSAO *playersao, const PlayerHPChangeReason &reason)
{

}

void Server::SendPlayerHP(PlayerSAO *playersao, bool effect)
{

}

void Server::SendHP(session_t peer_id, u16 hp, bool effect)
{

}

void Server::SendBreath(session_t peer_id, u16 breath)
{

}

void Server::SendAccessDenied(session_t peer_id, AccessDeniedCode reason,
		const std::string &custom_reason, bool reconnect)
{

}

void Server::SendDeathscreen(session_t peer_id, bool set_camera_point_target,
		v3f camera_point_target)
{

}

void Server::SendItemDef(session_t peer_id,
		IItemDefManager *itemdef, u16 protocol_version)
{

}

void Server::SendNodeDef(session_t peer_id,
	const NodeDefManager *nodedef, u16 protocol_version)
{

}

/*
	Non-static send methods
*/

void Server::SendInventory(PlayerSAO *sao, bool incremental)
{

}

void Server::SendChatMessage(session_t peer_id, const ChatMessage &message)
{

}

void Server::SendShowFormspecMessage(session_t peer_id, const std::string &formspec,
	const std::string &formname)
{

}

// Spawns a particle on peer with peer_id
void Server::SendSpawnParticle(session_t peer_id, u16 protocol_version,
	const ParticleParameters &p)
{

}

// Adds a ParticleSpawner on peer with peer_id
void Server::SendAddParticleSpawner(session_t peer_id, u16 protocol_version,
	const ParticleSpawnerParameters &p, u16 attached_id, u32 id)
{

}

void Server::SendDeleteParticleSpawner(session_t peer_id, u32 id)
{

}

void Server::SendHUDAdd(session_t peer_id, u32 id, HudElement *form)
{

}

void Server::SendHUDRemove(session_t peer_id, u32 id)
{

}

void Server::SendHUDChange(session_t peer_id, u32 id, HudElementStat stat, void *value)
{

}

void Server::SendHUDSetFlags(session_t peer_id, u32 flags, u32 mask)
{

}

void Server::SendHUDSetParam(session_t peer_id, u16 param, const std::string &value)
{

}

void Server::SendSetSky(session_t peer_id, const SkyboxParams &params)
{

}

void Server::SendSetSun(session_t peer_id, const SunParams &params)
{

}
void Server::SendSetMoon(session_t peer_id, const MoonParams &params)
{

}
void Server::SendSetStars(session_t peer_id, const StarParams &params)
{

}

void Server::SendCloudParams(session_t peer_id, const CloudParams &params)
{

}

void Server::SendOverrideDayNightRatio(session_t peer_id, bool do_override,
		float ratio)
{

}

void Server::SendSetLighting(session_t peer_id, const Lighting &lighting)
{

}

void Server::SendTimeOfDay(session_t peer_id, u16 time, f32 time_speed)
{

}

void Server::SendPlayerBreath(PlayerSAO *sao)
{

}

void Server::SendMovePlayer(session_t peer_id)
{

}

void Server::SendPlayerFov(session_t peer_id)
{

}

void Server::SendLocalPlayerAnimations(session_t peer_id, v2s32 animation_frames[4],
		f32 animation_speed)
{

}

void Server::SendEyeOffset(session_t peer_id, v3f first, v3f third)
{

}

void Server::SendPlayerPrivileges(session_t peer_id)
{

}

void Server::SendPlayerInventoryFormspec(session_t peer_id)
{

}

void Server::SendPlayerFormspecPrepend(session_t peer_id)
{

}

void Server::SendActiveObjectMessages(session_t peer_id, const std::string &datas,
		bool reliable)
{

}


void Server::SendPlayerSpeed(session_t peer_id, const v3f &added_vel)
{

}

void Server::stopSound(s32 handle)
{

}

void Server::fadeSound(s32 handle, float step, float gain)
{

}

void Server::sendRemoveNode(v3s16 p, std::unordered_set<u16> *far_players,
		float far_d_nodes)
{

}

void Server::sendAddNode(v3s16 p, MapNode n, std::unordered_set<u16> *far_players,
		float far_d_nodes, bool remove_metadata)
{

}

void Server::sendNodeChangePkt(NetworkPacket &pkt, v3s16 block_pos,
		v3f p, float far_d_nodes, std::unordered_set<u16> *far_players)
{

}

void Server::sendMetadataChanged(const std::unordered_set<v3s16> &positions, float far_d_nodes)
{

}

void Server::SendBlockNoLock(session_t peer_id, MapBlock *block, u8 ver,
		u16 net_proto_version, SerializedBlockCache *cache)
{

}

void Server::SendBlocks(float dtime)
{

}

bool Server::SendBlock(session_t peer_id, const v3s16 &blockpos)
{
	return true;
}

void Server::fillMediaCache()
{

}

void Server::sendMediaAnnouncement(session_t peer_id, const std::string &lang_code)
{

}

struct SendableMedia
{
	std::string name;
	std::string path;
	std::string data;

	SendableMedia(const std::string &name, const std::string &path,
			std::string &&data):
		name(name), path(path), data(std::move(data))
	{}
};

void Server::sendRequestedMedia(session_t peer_id,
		const std::vector<std::string> &tosend)
{

}



void Server::sendDetachedInventory(Inventory *inventory, const std::string &name, session_t peer_id)
{

}

void Server::sendDetachedInventories(session_t peer_id, bool incremental)
{

}

/*
	Something random
*/

void Server::HandlePlayerDeath(PlayerSAO *playersao, const PlayerHPChangeReason &reason)
{

}

void Server::RespawnPlayer(session_t peer_id)
{

}


void Server::DenySudoAccess(session_t peer_id)
{

}


void Server::DenyAccess(session_t peer_id, AccessDeniedCode reason,
		const std::string &custom_reason, bool reconnect)
{

}

void Server::DisconnectPeer(session_t peer_id)
{

}

void Server::acceptAuth(session_t peer_id, bool forSudoMode)
{

}

void Server::DeleteClient(session_t peer_id, ClientDeletionReason reason)
{

}

RemoteClient *Server::getClient(session_t peer_id, ClientState state_min)
{
	RemoteClient *client = getClientNoEx(peer_id,state_min);
	if(!client)
		throw ClientNotFoundException("Client not found");

	return client;
}
RemoteClient *Server::getClientNoEx(session_t peer_id, ClientState state_min)
{
	return m_clients.getClientNoEx(peer_id, state_min);
}

std::string Server::getPlayerName(session_t peer_id)
{
	RemotePlayer *player = m_env->getPlayer(peer_id);
	if (!player)
		return "[id="+itos(peer_id)+"]";
	return player->getName();
}

PlayerSAO *Server::getPlayerSAO(session_t peer_id)
{
	RemotePlayer *player = m_env->getPlayer(peer_id);
	if (!player)
		return NULL;
	return player->getPlayerSAO();
}

std::set<std::string> Server::getPlayerEffectivePrivs(const std::string &name)
{
	std::set<std::string> privs;
	m_script->getAuth(name, NULL, &privs);
	return privs;
}

bool Server::checkPriv(const std::string &name, const std::string &priv)
{

}

void Server::reportInventoryFormspecModified(const std::string &name)
{

}

void Server::reportFormspecPrependModified(const std::string &name)
{

}

void Server::notifyPlayer(const char *name, const std::wstring &msg)
{

}

bool Server::showFormspec(const char *playername, const std::string &formspec,
	const std::string &formname)
{

	return true;
}

u32 Server::hudAdd(RemotePlayer *player, HudElement *form)
{
	if (!player)
		return -1;

	u32 id = player->addHud(form);

	SendHUDAdd(player->getPeerId(), id, form);

	return id;
}

bool Server::hudRemove(RemotePlayer *player, u32 id) {
	return true;
}

bool Server::hudChange(RemotePlayer *player, u32 id, HudElementStat stat, void *data)
{

	return true;
}

bool Server::hudSetFlags(RemotePlayer *player, u32 flags, u32 mask)
{

	return true;
}

bool Server::hudSetHotbarItemcount(RemotePlayer *player, s32 hotbar_itemcount)
{

	return true;
}

void Server::hudSetHotbarImage(RemotePlayer *player, const std::string &name)
{

}

void Server::hudSetHotbarSelectedImage(RemotePlayer *player, const std::string &name)
{

}

Address Server::getPeerAddress(session_t peer_id)
{
	// Note that this is only set after Init was received in Server::handleCommand_Init
	return getClient(peer_id, CS_Invalid)->getAddress();
}

void Server::setLocalPlayerAnimations(RemotePlayer *player,
		v2s32 animation_frames[4], f32 frame_speed)
{

}

void Server::setPlayerEyeOffset(RemotePlayer *player, const v3f &first, const v3f &third)
{

}

void Server::setSky(RemotePlayer *player, const SkyboxParams &params)
{

}

void Server::setSun(RemotePlayer *player, const SunParams &params)
{

}

void Server::setMoon(RemotePlayer *player, const MoonParams &params)
{

}

void Server::setStars(RemotePlayer *player, const StarParams &params)
{

}

void Server::setClouds(RemotePlayer *player, const CloudParams &params)
{

}

void Server::overrideDayNightRatio(RemotePlayer *player, bool do_override,
	float ratio)
{

}

void Server::setLighting(RemotePlayer *player, const Lighting &lighting)
{

}

void Server::notifyPlayers(const std::wstring &msg)
{

}

void Server::spawnParticle(const std::string &playername,
	const ParticleParameters &p)
{

}

u32 Server::addParticleSpawner(const ParticleSpawnerParameters &p,
	ServerActiveObject *attached, const std::string &playername)
{
	return -1;

}

void Server::deleteParticleSpawner(const std::string &playername, u32 id)
{

}

// IGameDef interface
// Under envlock
IItemDefManager *Server::getItemDefManager()
{
	return m_itemdef;
}

const NodeDefManager *Server::getNodeDefManager()
{
	return m_nodedef;
}

u16 Server::allocateUnknownNodeId(const std::string &name)
{
	return m_nodedef->allocateDummy(name);
}

IWritableItemDefManager *Server::getWritableItemDefManager()
{
	return m_itemdef;
}

NodeDefManager *Server::getWritableNodeDefManager()
{
	return m_nodedef;
}

const std::vector<ModSpec> & Server::getMods() const
{
	return m_modmgr->getMods();
}

const ModSpec *Server::getModSpec(const std::string &modname) const
{
	return m_modmgr->getModSpec(modname);
}

std::string Server::getBuiltinLuaPath()
{
	return porting::path_share + DIR_DELIM + "builtin";
}

v3f Server::findSpawnPos()
{
	return v3f(0.0f, 0.0f, 0.0f);
}

PlayerSAO* Server::emergePlayer(const char *name, session_t peer_id, u16 proto_version)
{
	return NULL;
}

/*
 * Mod channels
 */


bool Server::joinModChannel(const std::string &channel)
{
	return true;
}

bool Server::leaveModChannel(const std::string &channel)
{
	return true;
}

bool Server::sendModChannelMessage(const std::string &channel, const std::string &message)
{

	return true;
}

ModChannel* Server::getModChannel(const std::string &channel)
{
	return m_modchannel_mgr->getModChannel(channel);
}

void Server::broadcastModChannelMessage(const std::string &channel,
		const std::string &message, session_t from_peer)
{

}

ModStorageDatabase *Server::openModStorageDatabase(const std::string &world_path)
{
	std::string world_mt_path = world_path + DIR_DELIM + "world.mt";
	Settings world_mt;
	if (!world_mt.readConfigFile(world_mt_path.c_str()))
		throw BaseException("Cannot read world.mt!");

	std::string backend = world_mt.exists("mod_storage_backend") ?
		world_mt.get("mod_storage_backend") : "files";

	return openModStorageDatabase(backend, world_path, world_mt);
}

ModStorageDatabase *Server::openModStorageDatabase(const std::string &backend,
		const std::string &world_path, const Settings &world_mt)
{
	if (backend == "files")
		return new ModStorageDatabaseFiles(world_path);

	if (backend == "dummy")
		return new Database_Dummy();

	throw BaseException("Mod storage database backend " + backend + " not supported");
}
