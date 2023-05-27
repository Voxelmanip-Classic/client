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

void ServerEnvironment::step(float dtime)
{

}
