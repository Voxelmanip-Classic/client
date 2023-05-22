/*
Minetest
Copyright (C) 2010-2017 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include <algorithm>
#include "serverenvironment.h"
#include "settings.h"
#include "log.h"
#include "mapblock.h"
#include "map.h"
#include "raycast.h"
#include "remoteplayer.h"
#include "scripting_server.h"
#include "server.h"
#include "util/basic_macros.h"
#include "util/pointedthing.h"
#include "filesys.h"
#include "gameparams.h"
#include "database/database-dummy.h"
#include "database/database-files.h"
#include "server/luaentity_sao.h"
#include "server/player_sao.h"

// A number that is much smaller than the timeout for particle spawners should/could ever be
#define PARTICLE_SPAWNER_NO_EXPIRY -1024.f

/*
	OnMapblocksChangedReceiver
*/

void OnMapblocksChangedReceiver::onMapEditEvent(const MapEditEvent &event)
{
	assert(receiving);
	for (const v3s16 &p : event.modified_blocks) {
		modified_blocks.insert(p);
	}
}

/*
	ServerEnvironment
*/

// Random device to seed pseudo random generators.
static std::random_device seed;

ServerEnvironment::ServerEnvironment(ServerMap *map,
	ServerScripting *script_iface, Server *server):
	Environment(server),
	m_map(map),
	m_script(script_iface),
	m_server(server)
{

}

void ServerEnvironment::init()
{

}

ServerEnvironment::~ServerEnvironment()
{

}

Map & ServerEnvironment::getMap()
{
	return *m_map;
}

RemotePlayer *ServerEnvironment::getPlayer(const session_t peer_id)
{
	for (RemotePlayer *player : m_players) {
		if (player->getPeerId() == peer_id)
			return player;
	}
	return NULL;
}

RemotePlayer *ServerEnvironment::getPlayer(const char* name)
{
	for (RemotePlayer *player : m_players) {
		if (strcmp(player->getName(), name) == 0)
			return player;
	}
	return NULL;
}

void ServerEnvironment::step(float dtime)
{

}
