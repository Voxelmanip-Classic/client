/*
Minetest
Copyright (C) 2010-2020 Minetest core development team

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

#include "serverinventorymgr.h"
#include "map.h"
#include "nodemetadata.h"
#include "player_sao.h"
#include "remoteplayer.h"
#include "server.h"
#include "serverenvironment.h"

ServerInventoryManager::ServerInventoryManager() : InventoryManager()
{
}

ServerInventoryManager::~ServerInventoryManager()
{
	// Delete detached inventories
	for (auto &detached_inventory : m_detached_inventories) {
		delete detached_inventory.second.inventory;
	}
}

Inventory *ServerInventoryManager::getInventory(const InventoryLocation &loc)
{
	// No m_env check here: allow creation and modification of detached inventories

	switch (loc.type) {
	case InventoryLocation::UNDEFINED:
	case InventoryLocation::CURRENT_PLAYER:
		break;
	case InventoryLocation::PLAYER: {
		if (!m_env)
			return nullptr;

		RemotePlayer *player = m_env->getPlayer(loc.name.c_str());
		if (!player)
			return NULL;

		PlayerSAO *playersao = player->getPlayerSAO();
		return playersao ? playersao->getInventory() : nullptr;
	} break;
	case InventoryLocation::NODEMETA: {
		if (!m_env)
			return nullptr;

		NodeMetadata *meta = m_env->getMap().getNodeMetadata(loc.p);
		return meta ? meta->getInventory() : nullptr;
	} break;
	case InventoryLocation::DETACHED: {
		auto it = m_detached_inventories.find(loc.name);
		if (it == m_detached_inventories.end())
			return nullptr;
		return it->second.inventory;
	} break;
	default:
		sanity_check(false); // abort
		break;
	}
	return NULL;
}

void ServerInventoryManager::setInventoryModified(const InventoryLocation &loc)
{
	switch (loc.type) {
	case InventoryLocation::UNDEFINED:
		break;
	case InventoryLocation::PLAYER: {

		RemotePlayer *player = m_env->getPlayer(loc.name.c_str());

		if (!player)
			return;

		player->setModified(true);
		player->inventory.setModified(true);
		// Updates are sent in ServerEnvironment::step()
	} break;
	case InventoryLocation::NODEMETA: {
		MapEditEvent event;
		event.type = MEET_BLOCK_NODE_METADATA_CHANGED;
		event.setPositionModified(loc.p);
		m_env->getMap().dispatchEvent(event);
	} break;
	case InventoryLocation::DETACHED: {
		// Updates are sent in ServerEnvironment::step()
	} break;
	default:
		sanity_check(false); // abort
		break;
	}
}
