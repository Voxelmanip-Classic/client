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

#pragma once

#include "activeobject.h"
#include "environment.h"
#include "map.h"
#include "settings.h"
#include "server/serveractiveobject.h"
#include "util/numeric.h"
#include <set>
#include <random>

struct GameParams;
class ServerEnvironment;
class ServerActiveObject;
class Server;
class ServerScripting;
typedef u16 session_t;



/*
	ServerEnvironment::m_on_mapblocks_changed_receiver
*/
struct OnMapblocksChangedReceiver : public MapEventReceiver {
	std::unordered_set<v3s16> modified_blocks;
	bool receiving = false;

	void onMapEditEvent(const MapEditEvent &event) override;
};

class ServerEnvironment final : public Environment
{
public:
	ServerEnvironment(ServerMap *map,
		Server *server);
	~ServerEnvironment();

	void init();

	Map & getMap();

	Server *getGameDef()
	{ return m_server; }

	/*
		Other stuff
		-------------------------------------------
	*/

	// This makes stuff happen
	void step(f32 dtime);

private:

	/*
		Internal ActiveObject interface
		-------------------------------------------
	*/

	/*
		Member variables
	*/

	// The map
	ServerMap *m_map;
	// Server definition
	Server *m_server;

};
