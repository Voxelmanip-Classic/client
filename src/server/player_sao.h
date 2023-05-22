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

#pragma once

#include "metadata.h"
#include "network/networkprotocol.h"
#include "unit_sao.h"
#include "util/numeric.h"

/*
	PlayerSAO needs some internals exposed.
*/

class RemotePlayer;

class PlayerSAO : public UnitSAO
{
public:
	PlayerSAO(ServerEnvironment *env_, RemotePlayer *player_, session_t peer_id_,
			bool is_singleplayer);

	ActiveObjectType getType() const override { return ACTIVEOBJECT_TYPE_PLAYER; }

	/*
		Active object <-> environment interface
	*/

	bool isStaticAllowed() const override { return false; }
	void getStaticData(std::string *result) const override;
	void step(float dtime, bool send_recommended) override;

	/*
		Inventory interface
	*/
	Inventory *getInventory() const override;

	/*
		PlayerSAO-specific
	*/

	RemotePlayer *getPlayer() { return m_player; }

	// Other

	bool collideWithObjects() const override { return true; }

private:
	RemotePlayer *m_player = nullptr;
	session_t m_peer_id = 0;
};
