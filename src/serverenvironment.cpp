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
	ServerScripting *script_iface, Server *server,
	const std::string &path_world, MetricsBackend *mb):
	Environment(server),
	m_map(map),
	m_script(script_iface),
	m_server(server),
	m_path_world(path_world),
	m_rgen(seed())
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

void ServerEnvironment::addPlayer(RemotePlayer *player)
{
	/*
		Check that peer_ids are unique.
		Also check that names are unique.
		Exception: there can be multiple players with peer_id=0
	*/
	// If peer id is non-zero, it has to be unique.
	if (player->getPeerId() != PEER_ID_INEXISTENT)
		FATAL_ERROR_IF(getPlayer(player->getPeerId()) != NULL, "Peer id not unique");
	// Name has to be unique.
	FATAL_ERROR_IF(getPlayer(player->getName()) != NULL, "Player name not unique");
	// Add.
	m_players.push_back(player);
}

void ServerEnvironment::removePlayer(RemotePlayer *player)
{
	for (std::vector<RemotePlayer *>::iterator it = m_players.begin();
		it != m_players.end(); ++it) {
		if ((*it) == player) {
			delete *it;
			m_players.erase(it);
			return;
		}
	}
}

bool ServerEnvironment::removePlayerFromDatabase(const std::string &name)
{
	return m_player_database->removePlayer(name);
}

void ServerEnvironment::kickAllPlayers(AccessDeniedCode reason,
	const std::string &str_reason, bool reconnect)
{
	for (RemotePlayer *player : m_players)
		m_server->DenyAccess(player->getPeerId(), reason, str_reason, reconnect);
}

void ServerEnvironment::saveLoadedPlayers(bool force)
{
	for (RemotePlayer *player : m_players) {
		if (force || player->checkModified() || (player->getPlayerSAO() &&
				player->getPlayerSAO()->getMeta().isModified())) {
			try {
				m_player_database->savePlayer(player);
			} catch (DatabaseException &e) {
				errorstream << "Failed to save player " << player->getName() << " exception: "
					<< e.what() << std::endl;
				throw;
			}
		}
	}
}

void ServerEnvironment::savePlayer(RemotePlayer *player)
{
	try {
		m_player_database->savePlayer(player);
	} catch (DatabaseException &e) {
		errorstream << "Failed to save player " << player->getName() << " exception: "
			<< e.what() << std::endl;
		throw;
	}
}

PlayerSAO *ServerEnvironment::loadPlayer(RemotePlayer *player, bool *new_player,
	session_t peer_id, bool is_singleplayer)
{
	PlayerSAO *playersao = new PlayerSAO(this, player, peer_id, is_singleplayer);
	// Create player if it doesn't exist
	if (!m_player_database->loadPlayer(player, playersao)) {
		*new_player = true;
		// Set player position
		infostream << "Server: Finding spawn place for player \""
			<< player->getName() << "\"" << std::endl;
		playersao->setBasePosition(m_server->findSpawnPos());

		// Make sure the player is saved
		player->setModified(true);
	} else {
		// If the player exists, ensure that they respawn inside legal bounds
		// This fixes an assert crash when the player can't be added
		// to the environment
		if (objectpos_over_limit(playersao->getBasePosition())) {
			actionstream << "Respawn position for player \""
				<< player->getName() << "\" outside limits, resetting" << std::endl;
			playersao->setBasePosition(m_server->findSpawnPos());
		}
	}

	// Add player to environment
	addPlayer(player);

	/* Clean up old HUD elements from previous sessions */
	player->clearHud();

	/* Add object to environment */
	addActiveObject(playersao);

	// Update active blocks quickly for a bit so objects in those blocks appear on the client
	m_fast_active_block_divider = 10;

	return playersao;
}


bool ServerEnvironment::setNode(v3s16 p, const MapNode &n)
{

	return true;
}

bool ServerEnvironment::removeNode(v3s16 p)
{

	return true;
}

bool ServerEnvironment::swapNode(v3s16 p, const MapNode &n)
{

	return true;
}

u8 ServerEnvironment::findSunlight(v3s16 pos) const
{
	return 0;
}

void ServerEnvironment::step(float dtime)
{

}

ServerEnvironment::BlockStatus ServerEnvironment::getBlockStatus(v3s16 blockpos)
{
	return BS_UNKNOWN;
}

u32 ServerEnvironment::addParticleSpawner(float exptime)
{
	// Timers with lifetime 0 do not expire
	float time = exptime > 0.f ? exptime : PARTICLE_SPAWNER_NO_EXPIRY;

	u32 id = 0;
	for (;;) { // look for unused particlespawner id
		id++;
		std::unordered_map<u32, float>::iterator f = m_particle_spawners.find(id);
		if (f == m_particle_spawners.end()) {
			m_particle_spawners[id] = time;
			break;
		}
	}
	return id;
}

u32 ServerEnvironment::addParticleSpawner(float exptime, u16 attached_id)
{
	u32 id = addParticleSpawner(exptime);
	m_particle_spawner_attachments[id] = attached_id;
	if (ServerActiveObject *obj = getActiveObject(attached_id)) {
		obj->attachParticleSpawner(id);
	}
	return id;
}

void ServerEnvironment::deleteParticleSpawner(u32 id, bool remove_from_object)
{
	m_particle_spawners.erase(id);
	const auto &it = m_particle_spawner_attachments.find(id);
	if (it != m_particle_spawner_attachments.end()) {
		u16 obj_id = it->second;
		ServerActiveObject *sao = getActiveObject(obj_id);
		if (sao != NULL && remove_from_object) {
			sao->detachParticleSpawner(id);
		}
		m_particle_spawner_attachments.erase(id);
	}
}

u16 ServerEnvironment::addActiveObject(ServerActiveObject *object)
{
	assert(object);	// Pre-condition
	m_added_objects++;
	u16 id = addActiveObjectRaw(object, true, 0);
	return id;
}

/*
	Finds out what new objects have been added to
	inside a radius around a position
*/
void ServerEnvironment::getAddedActiveObjects(PlayerSAO *playersao, s16 radius,
	s16 player_radius,
	std::set<u16> &current_objects,
	std::queue<u16> &added_objects)
{
	f32 radius_f = radius * BS;
	f32 player_radius_f = player_radius * BS;

	if (player_radius_f < 0.0f)
		player_radius_f = 0.0f;

	m_ao_manager.getAddedActiveObjectsAroundPos(playersao->getBasePosition(), radius_f,
		player_radius_f, current_objects, added_objects);
}


bool ServerEnvironment::getActiveObjectMessage(ActiveObjectMessage *dest)
{
	if (m_active_object_messages.empty())
		return false;

	*dest = std::move(m_active_object_messages.front());
	m_active_object_messages.pop();
	return true;
}

void ServerEnvironment::getSelectedActiveObjects(
	const core::line3d<f32> &shootline_on_map,
	std::vector<PointedThing> &objects)
{
	std::vector<ServerActiveObject *> objs;
	getObjectsInsideRadius(objs, shootline_on_map.start,
		shootline_on_map.getLength() + 10.0f, nullptr);
	const v3f line_vector = shootline_on_map.getVector();

	for (auto obj : objs) {
		if (obj->isGone())
			continue;
		aabb3f selection_box;
		if (!obj->getSelectionBox(&selection_box))
			continue;

		v3f pos = obj->getBasePosition();
		v3f rel_pos = shootline_on_map.start - pos;

		v3f current_intersection;
		v3f current_normal;
		v3f current_raw_normal;

		ObjectProperties *props = obj->accessObjectProperties();
		bool collision;
		UnitSAO* usao = dynamic_cast<UnitSAO*>(obj);
		if (props->rotate_selectionbox && usao != nullptr) {
			collision = boxLineCollision(selection_box, usao->getTotalRotation(),
				rel_pos, line_vector, &current_intersection, &current_normal, &current_raw_normal);
		} else {
			collision = boxLineCollision(selection_box, rel_pos, line_vector,
				&current_intersection, &current_normal);
			current_raw_normal = current_normal;
		}
		if (collision) {
			current_intersection += pos;
			objects.emplace_back(
				(s16) obj->getId(), current_intersection, current_normal, current_raw_normal,
				(current_intersection - shootline_on_map.start).getLengthSQ());
		}
	}
}

/*
	************ Private methods *************
*/

u16 ServerEnvironment::addActiveObjectRaw(ServerActiveObject *object,
	bool set_changed, u32 dtime_s)
{
	if (!m_ao_manager.registerObject(object)) {
		return 0;
	}

	// Register reference in scripting api (must be done before post-init)
	m_script->addObjectReference(object);
	// Post-initialize object
	object->addedToEnvironment(dtime_s);

	// Add static data to block
	if (object->isStaticAllowed()) {
		// Add static object to active static list of the block
		v3f objectpos = object->getBasePosition();
		StaticObject s_obj(object, objectpos);
		// Add to the block where the object is located in
		v3s16 blockpos = getNodeBlockPos(floatToInt(objectpos, BS));
		MapBlock *block = m_map->emergeBlock(blockpos);
		if (block) {
			block->m_static_objects.setActive(object->getId(), s_obj);
			object->m_static_exists = true;
			object->m_static_block = blockpos;

			if(set_changed)
				block->raiseModified(MOD_STATE_WRITE_NEEDED,
					MOD_REASON_ADD_ACTIVE_OBJECT_RAW);
		} else {
			v3s16 p = floatToInt(objectpos, BS);
			errorstream<<"ServerEnvironment::addActiveObjectRaw(): "
				<<"could not emerge block for storing id="<<object->getId()
				<<" statically (pos="<<PP(p)<<")"<<std::endl;
		}
	}

	return object->getId();
}

ServerActiveObject* ServerEnvironment::createSAO(ActiveObjectType type, v3f pos,
		const std::string &data)
{
	switch (type) {
		case ACTIVEOBJECT_TYPE_LUAENTITY:
			return new LuaEntitySAO(this, pos, data);
		default:
			warningstream << "ServerActiveObject: No factory for type=" << type << std::endl;
	}
	return nullptr;
}
