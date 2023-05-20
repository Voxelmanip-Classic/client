/*
Minetest
Copyright (C) 2010-2016 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2014-2016 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

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

#include "player.h"
#include "skyparams.h"
#include "lighting.h"

class PlayerSAO;

/*
	Player on the server
*/
class RemotePlayer : public Player
{

public:
	RemotePlayer(const char *name, IItemDefManager *idef);
	virtual ~RemotePlayer() = default;

	PlayerSAO *getPlayerSAO() { return m_sao; }
	void setPlayerSAO(PlayerSAO *sao) { m_sao = sao; }

	inline void setModified(const bool x) { m_dirty = x; }

	void setDirty(bool dirty) { m_dirty = true; }

	u16 protocol_version = 0;
	u16 formspec_version = 0;

	session_t getPeerId() const { return m_peer_id; }

	void setPeerId(session_t peer_id) { m_peer_id = peer_id; }

private:
	PlayerSAO *m_sao = nullptr;
	bool m_dirty = false;

	static bool m_setting_cache_loaded;

	Lighting m_lighting;

	session_t m_peer_id = PEER_ID_INEXISTENT;
};
