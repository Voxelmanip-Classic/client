/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2013-2020 Minetest core developers & community

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

#include "player_sao.h"
#include "constants.h"
#include "debug.h"
#include "remoteplayer.h"
#include "server/unit_sao.h"
#include "serverenvironment.h"
class Inventory;

PlayerSAO::PlayerSAO(ServerEnvironment *env_, RemotePlayer *player_, session_t peer_id_,
		bool is_singleplayer):
	UnitSAO(env_, v3f(0,0,0)),
	m_player(player_),
	m_peer_id(peer_id_)
{

}

void PlayerSAO::getStaticData(std::string * result) const
{
	FATAL_ERROR("This function shall not be called for PlayerSAO");
}

void PlayerSAO::step(float dtime, bool send_recommended)
{

}


Inventory *PlayerSAO::getInventory() const
{
	return m_player ? &m_player->inventory : nullptr;
}
