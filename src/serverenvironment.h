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
#include "server/activeobjectmgr.h"
#include "util/numeric.h"
#include <set>
#include <random>

struct GameParams;
class RemotePlayer;
class PlayerSAO;
class ServerEnvironment;
struct StaticObject;
class ServerActiveObject;
class Server;
class ServerScripting;
enum AccessDeniedCode : u8;
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
	ServerEnvironment(ServerMap *map, ServerScripting *script_iface,
		Server *server, const std::string &path_world);
	~ServerEnvironment();

	void init();

	Map & getMap();

	//TODO find way to remove this fct!
	ServerScripting* getScriptIface()
	{ return m_script; }

	Server *getGameDef()
	{ return m_server; }

	float getSendRecommendedInterval()
	{ return m_recommended_send_interval; }

	// Save players
	void savePlayer(RemotePlayer *player);

	void removePlayer(RemotePlayer *player);

	u32 addParticleSpawner(float exptime);
	u32 addParticleSpawner(float exptime, u16 attached_id);
	void deleteParticleSpawner(u32 id, bool remove_from_object = true);

	/*
		External ActiveObject interface
		-------------------------------------------
	*/

	ServerActiveObject* getActiveObject(u16 id)
	{
		return m_ao_manager.getActiveObject(id);
	}

	/*
		Add an active object to the environment.
		Environment handles deletion of object.
		Object may be deleted by environment immediately.
		If id of object is 0, assigns a free id to it.
		Returns the id of the object.
		Returns 0 if not added and thus deleted.
	*/
	u16 addActiveObject(ServerActiveObject *object);

	/*
		Add an active object as a static object to the corresponding
		MapBlock.
		Caller allocates memory, ServerEnvironment frees memory.
		Return value: true if succeeded, false if failed.
		(note:  not used, pending removal from engine)
	*/
	//bool addActiveObjectAsStatic(ServerActiveObject *object);

	/*
		Find out what new objects have been added to
		inside a radius around a position
	*/
	void getAddedActiveObjects(PlayerSAO *playersao, s16 radius,
		s16 player_radius,
		std::set<u16> &current_objects,
		std::queue<u16> &added_objects);

	virtual void getSelectedActiveObjects(
		const core::line3d<f32> &shootline_on_map,
		std::vector<PointedThing> &objects
	);

	/*
		Other stuff
		-------------------------------------------
	*/

	// Script-aware node setters
	bool setNode(v3s16 p, const MapNode &n);
	bool removeNode(v3s16 p);
	bool swapNode(v3s16 p, const MapNode &n);

	// Find the daylight value at pos with a Depth First Search
	u8 findSunlight(v3s16 pos) const;

	// Find all active objects inside a radius around a point
	void getObjectsInsideRadius(std::vector<ServerActiveObject *> &objects, const v3f &pos, float radius,
			std::function<bool(ServerActiveObject *obj)> include_obj_cb)
	{
		return m_ao_manager.getObjectsInsideRadius(pos, radius, objects, include_obj_cb);
	}

	// Find all active objects inside a box
	void getObjectsInArea(std::vector<ServerActiveObject *> &objects, const aabb3f &box,
			std::function<bool(ServerActiveObject *obj)> include_obj_cb)
	{
		return m_ao_manager.getObjectsInArea(box, objects, include_obj_cb);
	}

	// This makes stuff happen
	void step(f32 dtime);

	u32 getGameTime() const { return m_game_time; }

	void reportMaxLagEstimate(float f) { m_max_lag_estimate = f; }
	float getMaxLagEstimate() { return m_max_lag_estimate; }

	RemotePlayer *getPlayer(const session_t peer_id);
	RemotePlayer *getPlayer(const char* name);
	const std::vector<RemotePlayer *> getPlayers() const { return m_players; }
	u32 getPlayerCount() const { return m_players.size(); }

private:

	/*
		Internal ActiveObject interface
		-------------------------------------------
	*/

	/*
		Add an active object to the environment.

		Called by addActiveObject.

		Object may be deleted by environment immediately.
		If id of object is 0, assigns a free id to it.
		Returns the id of the object.
		Returns 0 if not added and thus deleted.
	*/
	u16 addActiveObjectRaw(ServerActiveObject *object, bool set_changed, u32 dtime_s);

	/*
		Member variables
	*/

	// The map
	ServerMap *m_map;
	// Lua state
	ServerScripting* m_script;
	// Server definition
	Server *m_server;
	// Active Object Manager
	server::ActiveObjectMgr m_ao_manager;
	// on_mapblocks_changed map event receiver
	OnMapblocksChangedReceiver m_on_mapblocks_changed_receiver;
	// World path
	const std::string m_path_world;
	// Outgoing network message buffer for active objects
	std::queue<ActiveObjectMessage> m_active_object_messages;
	// Some timers
	float m_send_recommended_timer = 0.0f;
	IntervalLimiter m_object_management_interval;
	// List of active blocks

	int m_fast_active_block_divider = 1;
	IntervalLimiter m_active_block_modifier_interval;
	// Whether the variables below have been read from file yet
	bool m_meta_loaded = false;
	// Time from the beginning of the game in seconds.
	// Incremented in step().
	u32 m_game_time = 0;
	// A helper variable for incrementing the latter
	float m_game_time_fraction_counter = 0.0f;
	// Time of last clearObjects call (game time).
	// When a mapblock older than this is loaded, its objects are cleared.
	u32 m_last_clear_objects_time = 0;


	// An interval for generally sending object positions and stuff
	float m_recommended_send_interval = 0.1f;
	// Estimate for general maximum lag as determined by server.
	// Can raise to high values like 15s with eg. map generation mods.
	float m_max_lag_estimate = 0.1f;

	// peer_ids in here should be unique, except that there may be many 0s
	std::vector<RemotePlayer*> m_players;

	// Pseudo random generator for shuffling, etc.
	std::mt19937 m_rgen;

	// Particles
	IntervalLimiter m_particle_management_interval;
	std::unordered_map<u32, float> m_particle_spawners;
	std::unordered_map<u32, u16> m_particle_spawner_attachments;

	ServerActiveObject* createSAO(ActiveObjectType type, v3f pos, const std::string &data);
};
