/*
Minetest
Copyright (C) 2010-2014 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include <sstream>
#include "clientiface.h"
#include "network/connection.h"
#include "network/serveropcodes.h"
#include "remoteplayer.h"
#include "settings.h"
#include "mapblock.h"
#include "serverenvironment.h"
#include "map.h"
#include "emerge.h"
#include "server/luaentity_sao.h"
#include "server/player_sao.h"
#include "log.h"
#include "util/srp.h"
#include "face_position_cache.h"

const char *ClientInterface::statenames[] = {
	"Invalid",
	"Disconnecting",
	"Denied",
	"Created",
	"AwaitingInit2",
	"HelloSent",
	"InitDone",
	"DefinitionsSent",
	"Active",
	"SudoMode",
};



std::string ClientInterface::state2Name(ClientState state)
{
	return statenames[state];
}

RemoteClient::RemoteClient() :
	m_max_simul_sends(g_settings->getU16("max_simultaneous_block_sends_per_client")),
	m_min_time_from_building(
		g_settings->getFloat("full_block_send_enable_min_time_from_building")),
	m_max_send_distance(g_settings->getS16("max_block_send_distance")),
	m_max_gen_distance(g_settings->getS16("max_block_generate_distance"))
{
}

void RemoteClient::ResendBlockIfOnWire(v3s16 p)
{
	// if this block is on wire, mark it for sending again as soon as possible
	if (m_blocks_sending.find(p) != m_blocks_sending.end()) {
		SetBlockNotSent(p);
	}
}

LuaEntitySAO *getAttachedObject(PlayerSAO *sao, ServerEnvironment *env)
{
	if (!sao->isAttached())
		return nullptr;

	int id;
	std::string bone;
	v3f dummy;
	bool force_visible;
	sao->getAttachment(&id, &bone, &dummy, &dummy, &force_visible);
	ServerActiveObject *ao = env->getActiveObject(id);
	while (id && ao) {
		ao->getAttachment(&id, &bone, &dummy, &dummy, &force_visible);
		if (id)
			ao = env->getActiveObject(id);
	}
	return dynamic_cast<LuaEntitySAO *>(ao);
}

void RemoteClient::GetNextBlocks (
		ServerEnvironment *env,
		EmergeManager * emerge,
		float dtime,
		std::vector<PrioritySortedBlockTransfer> &dest)
{

}

void RemoteClient::GotBlock(v3s16 p)
{
	if (m_blocks_sending.find(p) != m_blocks_sending.end()) {
		m_blocks_sending.erase(p);
		// only add to sent blocks if it actually was sending
		// (it might have been modified since)
		m_blocks_sent.insert(p);
	} else {
		m_excess_gotblocks++;
	}
}

void RemoteClient::SentBlock(v3s16 p)
{
	if (m_blocks_sending.find(p) == m_blocks_sending.end())
		m_blocks_sending[p] = 0.0f;
	else
		infostream<<"RemoteClient::SentBlock(): Sent block"
				" already in m_blocks_sending"<<std::endl;
}

void RemoteClient::SetBlockNotSent(v3s16 p)
{
	m_nothing_to_send_pause_timer = 0;

	// remove the block from sending and sent sets,
	// and mark as modified if found
	if (m_blocks_sending.erase(p) + m_blocks_sent.erase(p) > 0)
		m_blocks_modified.insert(p);
}

void RemoteClient::SetBlocksNotSent(std::map<v3s16, MapBlock*> &blocks)
{
	m_nothing_to_send_pause_timer = 0;

	for (auto &block : blocks) {
		v3s16 p = block.first;
		// remove the block from sending and sent sets,
		// and mark as modified if found
		if (m_blocks_sending.erase(p) + m_blocks_sent.erase(p) > 0)
			m_blocks_modified.insert(p);
	}
}

void RemoteClient::notifyEvent(ClientStateEvent event)
{
	std::ostringstream myerror;
	switch (m_state)
	{
	case CS_Invalid:
		//intentionally do nothing
		break;
	case CS_Created:
		switch (event) {
		case CSE_Hello:
			m_state = CS_HelloSent;
			break;
		case CSE_Disconnect:
			m_state = CS_Disconnecting;
			break;
		case CSE_SetDenied:
			m_state = CS_Denied;
			break;
		/* GotInit2 SetDefinitionsSent SetMediaSent */
		default:
			myerror << "Created: Invalid client state transition! " << event;
			throw ClientStateError(myerror.str());
		}
		break;
	case CS_Denied:
		/* don't do anything if in denied state */
		break;
	case CS_HelloSent:
		switch(event)
		{
		case CSE_AuthAccept:
			m_state = CS_AwaitingInit2;
			resetChosenMech();
			break;
		case CSE_Disconnect:
			m_state = CS_Disconnecting;
			break;
		case CSE_SetDenied:
			m_state = CS_Denied;
			resetChosenMech();
			break;
		default:
			myerror << "HelloSent: Invalid client state transition! " << event;
			throw ClientStateError(myerror.str());
		}
		break;
	case CS_AwaitingInit2:
		switch(event)
		{
		case CSE_GotInit2:
			confirmSerializationVersion();
			m_state = CS_InitDone;
			break;
		case CSE_Disconnect:
			m_state = CS_Disconnecting;
			break;
		case CSE_SetDenied:
			m_state = CS_Denied;
			break;

		/* Init SetDefinitionsSent SetMediaSent */
		default:
			myerror << "InitSent: Invalid client state transition! " << event;
			throw ClientStateError(myerror.str());
		}
		break;

	case CS_InitDone:
		switch(event)
		{
		case CSE_SetDefinitionsSent:
			m_state = CS_DefinitionsSent;
			break;
		case CSE_Disconnect:
			m_state = CS_Disconnecting;
			break;
		case CSE_SetDenied:
			m_state = CS_Denied;
			break;

		/* Init GotInit2 SetMediaSent */
		default:
			myerror << "InitDone: Invalid client state transition! " << event;
			throw ClientStateError(myerror.str());
		}
		break;
	case CS_DefinitionsSent:
		switch(event)
		{
		case CSE_SetClientReady:
			m_state = CS_Active;
			break;
		case CSE_Disconnect:
			m_state = CS_Disconnecting;
			break;
		case CSE_SetDenied:
			m_state = CS_Denied;
			break;
		/* Init GotInit2 SetDefinitionsSent */
		default:
			myerror << "DefinitionsSent: Invalid client state transition! " << event;
			throw ClientStateError(myerror.str());
		}
		break;
	case CS_Active:
		switch(event)
		{
		case CSE_SetDenied:
			m_state = CS_Denied;
			break;
		case CSE_Disconnect:
			m_state = CS_Disconnecting;
			break;
		case CSE_SudoSuccess:
			m_state = CS_SudoMode;
			resetChosenMech();
			break;
		/* Init GotInit2 SetDefinitionsSent SetMediaSent SetDenied */
		default:
			myerror << "Active: Invalid client state transition! " << event;
			throw ClientStateError(myerror.str());
			break;
		}
		break;
	case CS_SudoMode:
		switch(event)
		{
		case CSE_SetDenied:
			m_state = CS_Denied;
			break;
		case CSE_Disconnect:
			m_state = CS_Disconnecting;
			break;
		case CSE_SudoLeave:
			m_state = CS_Active;
			break;
		default:
			myerror << "Active: Invalid client state transition! " << event;
			throw ClientStateError(myerror.str());
			break;
		}
		break;
	case CS_Disconnecting:
		/* we are already disconnecting */
		break;
	}
}

void RemoteClient::resetChosenMech()
{
	if (auth_data) {
		srp_verifier_delete((SRPVerifier *) auth_data);
		auth_data = nullptr;
	}
	chosen_mech = AUTH_MECHANISM_NONE;
}

u64 RemoteClient::uptime() const
{
	return porting::getTimeS() - m_connection_time;
}

ClientInterface::ClientInterface(const std::shared_ptr<con::Connection> & con)
:
	m_con(con),
	m_env(NULL),
	m_print_info_timer(0.0f)
{

}
ClientInterface::~ClientInterface()
{
	/*
		Delete clients
	*/
	{
		RecursiveMutexAutoLock clientslock(m_clients_mutex);

		for (auto &client_it : m_clients) {
			// Delete client
			delete client_it.second;
		}
	}
}

std::vector<session_t> ClientInterface::getClientIDs(ClientState min_state)
{
	std::vector<session_t> reply;
	RecursiveMutexAutoLock clientslock(m_clients_mutex);

	for (const auto &m_client : m_clients) {
		if (m_client.second->getState() >= min_state)
			reply.push_back(m_client.second->peer_id);
	}

	return reply;
}

void ClientInterface::markBlockposAsNotSent(const v3s16 &pos)
{
	RecursiveMutexAutoLock clientslock(m_clients_mutex);
	for (const auto &client : m_clients) {
		if (client.second->getState() >= CS_Active)
			client.second->SetBlockNotSent(pos);
	}
}

/**
 * Verify if user limit was reached.
 * User limit count all clients from HelloSent state (MT protocol user) to Active state
 * @return true if user limit was reached
 */
bool ClientInterface::isUserLimitReached()
{
	return getClientIDs(CS_HelloSent).size() >= g_settings->getU16("max_users");
}

void ClientInterface::step(float dtime)
{
	m_print_info_timer += dtime;
	if (m_print_info_timer >= 30.0f) {
		m_print_info_timer = 0.0f;
		UpdatePlayerList();
	}
}

void ClientInterface::UpdatePlayerList()
{
	if (m_env) {
		std::vector<session_t> clients = getClientIDs();
		m_clients_names.clear();

		if (!clients.empty())
			infostream<<"Players:"<<std::endl;

		for (session_t i : clients) {
			RemotePlayer *player = m_env->getPlayer(i);

			if (player == NULL)
				continue;

			infostream << "* " << player->getName() << "\t";

			{
				RecursiveMutexAutoLock clientslock(m_clients_mutex);
				RemoteClient* client = lockedGetClientNoEx(i);
				if (client)
					client->PrintInfo(infostream);
			}

			m_clients_names.emplace_back(player->getName());
		}
	}
}

void ClientInterface::send(session_t peer_id, u8 channelnum,
		NetworkPacket *pkt, bool reliable)
{
	m_con->Send(peer_id, channelnum, pkt, reliable);
}

void ClientInterface::sendToAll(NetworkPacket *pkt)
{
	RecursiveMutexAutoLock clientslock(m_clients_mutex);
	for (auto &client_it : m_clients) {
		RemoteClient *client = client_it.second;

		if (client->net_proto_version != 0) {
			m_con->Send(client->peer_id,
					clientCommandFactoryTable[pkt->getCommand()].channel, pkt,
					clientCommandFactoryTable[pkt->getCommand()].reliable);
		}
	}
}

RemoteClient* ClientInterface::getClientNoEx(session_t peer_id, ClientState state_min)
{
	RecursiveMutexAutoLock clientslock(m_clients_mutex);
	RemoteClientMap::const_iterator n = m_clients.find(peer_id);
	// The client may not exist; clients are immediately removed if their
	// access is denied, and this event occurs later then.
	if (n == m_clients.end())
		return NULL;

	if (n->second->getState() >= state_min)
		return n->second;

	return NULL;
}

RemoteClient* ClientInterface::lockedGetClientNoEx(session_t peer_id, ClientState state_min)
{
	RemoteClientMap::const_iterator n = m_clients.find(peer_id);
	// The client may not exist; clients are immediately removed if their
	// access is denied, and this event occurs later then.
	if (n == m_clients.end())
		return NULL;

	if (n->second->getState() >= state_min)
		return n->second;

	return NULL;
}

ClientState ClientInterface::getClientState(session_t peer_id)
{
	RecursiveMutexAutoLock clientslock(m_clients_mutex);
	RemoteClientMap::const_iterator n = m_clients.find(peer_id);
	// The client may not exist; clients are immediately removed if their
	// access is denied, and this event occurs later then.
	if (n == m_clients.end())
		return CS_Invalid;

	return n->second->getState();
}

void ClientInterface::setPlayerName(session_t peer_id, const std::string &name)
{
	RecursiveMutexAutoLock clientslock(m_clients_mutex);
	RemoteClientMap::iterator n = m_clients.find(peer_id);
	// The client may not exist; clients are immediately removed if their
	// access is denied, and this event occurs later then.
	if (n != m_clients.end())
		n->second->setName(name);
}

void ClientInterface::DeleteClient(session_t peer_id)
{
	RecursiveMutexAutoLock conlock(m_clients_mutex);

	// Error check
	RemoteClientMap::iterator n = m_clients.find(peer_id);
	// The client may not exist; clients are immediately removed if their
	// access is denied, and this event occurs later then.
	if (n == m_clients.end())
		return;

	/*
		Mark objects to be not known by the client
	*/
	//TODO this should be done by client destructor!!!
	RemoteClient *client = n->second;
	// Handle objects
	for (u16 id : client->m_known_objects) {
		// Get object
		ServerActiveObject* obj = m_env->getActiveObject(id);

		if(obj && obj->m_known_by_count > 0)
			obj->m_known_by_count--;
	}

	// Delete client
	delete m_clients[peer_id];
	m_clients.erase(peer_id);
}

void ClientInterface::CreateClient(session_t peer_id)
{
	RecursiveMutexAutoLock conlock(m_clients_mutex);

	// Error check
	RemoteClientMap::iterator n = m_clients.find(peer_id);
	// The client shouldn't already exist
	if (n != m_clients.end()) return;

	// Create client
	RemoteClient *client = new RemoteClient();
	client->peer_id = peer_id;
	m_clients[client->peer_id] = client;
}

void ClientInterface::event(session_t peer_id, ClientStateEvent event)
{
	{
		RecursiveMutexAutoLock clientlock(m_clients_mutex);

		// Error check
		RemoteClientMap::iterator n = m_clients.find(peer_id);

		// No client to deliver event
		if (n == m_clients.end())
			return;
		n->second->notifyEvent(event);
	}

	if ((event == CSE_SetClientReady) ||
		(event == CSE_Disconnect)     ||
		(event == CSE_SetDenied))
	{
		UpdatePlayerList();
	}
}

u16 ClientInterface::getProtocolVersion(session_t peer_id)
{
	RecursiveMutexAutoLock conlock(m_clients_mutex);

	// Error check
	RemoteClientMap::iterator n = m_clients.find(peer_id);

	// No client to get version
	if (n == m_clients.end())
		return 0;

	return n->second->net_proto_version;
}

void ClientInterface::setClientVersion(session_t peer_id, u8 major, u8 minor, u8 patch,
		const std::string &full)
{
	RecursiveMutexAutoLock conlock(m_clients_mutex);

	// Error check
	RemoteClientMap::iterator n = m_clients.find(peer_id);

	// No client to set versions
	if (n == m_clients.end())
		return;

	n->second->setVersionInfo(major, minor, patch, full);
}
