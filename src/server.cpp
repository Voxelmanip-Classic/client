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
#include "database/database-files.h"
#include "database/database-dummy.h"

/*
	Server
*/

Server::Server(
		const SubgameSpec &gamespec,
		bool simple_singleplayer_mode,
		Address bind_addr,
		bool dedicated,
		ChatInterface *iface,
		std::string *on_shutdown_errmsg
	):
	m_gamespec(gamespec),
	m_async_fatal_error(""),
	m_itemdef(createItemDefManager()),
	m_nodedef(createNodeDefManager()),
	m_modchannel_mgr(new ModChannelMgr()) {}

Server::~Server() {}

void Server::init() {}

void Server::start() {}

void Server::stop() {}

void Server::step(float dtime) {}

void Server::onMapEditEvent(const MapEditEvent &event) {}

/*
	Something random
*/


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
