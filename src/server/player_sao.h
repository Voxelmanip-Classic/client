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

	// Data should not be sent at player initialization
	f32 getFov() const { return m_fov; }

	/*
		Inventory interface
	*/
	Inventory *getInventory() const override;
	void setInventoryModified() override {}
	u16 getWieldIndex() const override;

	/*
		PlayerSAO-specific
	*/

	RemotePlayer *getPlayer() { return m_player; }
	session_t getPeerID() const { return m_peer_id; }

	// Other

	bool collideWithObjects() const override { return true; }

	inline SimpleMetadata &getMeta() { return m_meta; }

private:
	RemotePlayer *m_player = nullptr;
	session_t m_peer_id = 0;

	u16 m_breath = 10;
	f32 m_pitch = 0.0f;
	f32 m_fov = 0.0f;
	s16 m_wanted_range = 0.0f;

	SimpleMetadata m_meta;

public:
	bool m_physics_override_sent = false;
};

struct PlayerHPChangeReason
{
	enum Type : u8
	{
		SET_HP,
		SET_HP_MAX, // internal type to allow distinguishing hp reset and damage (for effects)
		PLAYER_PUNCH,
		FALL,
		NODE_DAMAGE,
		DROWNING,
		RESPAWN
	};

	Type type = SET_HP;
	bool from_mod = false;
	int lua_reference = -1;

	// For PLAYER_PUNCH
	ServerActiveObject *object = nullptr;
	// For NODE_DAMAGE
	std::string node;
    v3s16 node_pos;

	PlayerHPChangeReason(Type type) : type(type) {}

	PlayerHPChangeReason(Type type, ServerActiveObject *object) :
			type(type), object(object)
	{
	}

	PlayerHPChangeReason(Type type, std::string node, v3s16 node_pos) : type(type), node(node), node_pos(node_pos) {}
};
