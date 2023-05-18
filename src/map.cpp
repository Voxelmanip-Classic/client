/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "map.h"
#include "mapsector.h"
#include "mapblock.h"
#include "filesys.h"
#include "voxel.h"
#include "voxelalgorithms.h"
#include "porting.h"
#include "serialization.h"
#include "nodemetadata.h"
#include "settings.h"
#include "log.h"
#include "profiler.h"
#include "nodedef.h"
#include "gamedef.h"
#include "util/directiontables.h"
#include "util/basic_macros.h"
#include "environment.h"
#include "emerge.h"
#include "config.h"
#include "server.h"
#include "database/database.h"
#include "database/database-dummy.h"
#include "script/scripting_server.h"
#include <deque>
#include <queue>

/*
	Map
*/

Map::Map(IGameDef *gamedef):
	m_gamedef(gamedef),
	m_nodedef(gamedef->ndef())
{
}

Map::~Map()
{
	/*
		Free all MapSectors
	*/
	for (auto &sector : m_sectors) {
		delete sector.second;
	}
}

void Map::addEventReceiver(MapEventReceiver *event_receiver)
{
	m_event_receivers.insert(event_receiver);
}

void Map::removeEventReceiver(MapEventReceiver *event_receiver)
{
	m_event_receivers.erase(event_receiver);
}

void Map::dispatchEvent(const MapEditEvent &event)
{
	for (MapEventReceiver *event_receiver : m_event_receivers) {
		event_receiver->onMapEditEvent(event);
	}
}

MapSector * Map::getSectorNoGenerateNoLock(v2s16 p)
{
	if(m_sector_cache != NULL && p == m_sector_cache_p){
		MapSector * sector = m_sector_cache;
		return sector;
	}

	auto n = m_sectors.find(p);

	if (n == m_sectors.end())
		return NULL;

	MapSector *sector = n->second;

	// Cache the last result
	m_sector_cache_p = p;
	m_sector_cache = sector;

	return sector;
}

MapSector * Map::getSectorNoGenerate(v2s16 p)
{
	return getSectorNoGenerateNoLock(p);
}

MapBlock * Map::getBlockNoCreateNoEx(v3s16 p3d)
{
	v2s16 p2d(p3d.X, p3d.Z);
	MapSector * sector = getSectorNoGenerate(p2d);
	if(sector == NULL)
		return NULL;
	MapBlock *block = sector->getBlockNoCreateNoEx(p3d.Y);
	return block;
}

MapBlock * Map::getBlockNoCreate(v3s16 p3d)
{
	MapBlock *block = getBlockNoCreateNoEx(p3d);
	if(block == NULL)
		throw InvalidPositionException();
	return block;
}

bool Map::isValidPosition(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	return (block != NULL);
}

// Returns a CONTENT_IGNORE node if not found
MapNode Map::getNode(v3s16 p, bool *is_valid_position)
{
	v3s16 blockpos = getNodeBlockPos(p);
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if (block == NULL) {
		if (is_valid_position != NULL)
			*is_valid_position = false;
		return {CONTENT_IGNORE};
	}

	v3s16 relpos = p - blockpos*MAP_BLOCKSIZE;
	MapNode node = block->getNodeNoCheck(relpos);
	if (is_valid_position != NULL)
		*is_valid_position = true;
	return node;
}

static void set_node_in_block(MapBlock *block, v3s16 relpos, MapNode n)
{
	// Never allow placing CONTENT_IGNORE, it causes problems
	if(n.getContent() == CONTENT_IGNORE){
		const NodeDefManager *nodedef = block->getParent()->getNodeDefManager();
		v3s16 blockpos = block->getPos();
		v3s16 p = blockpos * MAP_BLOCKSIZE + relpos;
		errorstream<<"Not allowing to place CONTENT_IGNORE"
				<<" while trying to replace \""
				<<nodedef->get(block->getNodeNoCheck(relpos)).name
				<<"\" at "<<PP(p)<<" (block "<<PP(blockpos)<<")"<<std::endl;
		return;
	}
	block->setNodeNoCheck(relpos, n);
}

// throws InvalidPositionException if not found
void Map::setNode(v3s16 p, MapNode n)
{
	v3s16 blockpos = getNodeBlockPos(p);
	MapBlock *block = getBlockNoCreate(blockpos);
	v3s16 relpos = p - blockpos*MAP_BLOCKSIZE;
	set_node_in_block(block, relpos, n);
}

void Map::addNodeAndUpdate(v3s16 p, MapNode n,
		std::map<v3s16, MapBlock*> &modified_blocks,
		bool remove_metadata)
{
	v3s16 blockpos = getNodeBlockPos(p);
	MapBlock *block = getBlockNoCreate(blockpos);
	v3s16 relpos = p - blockpos * MAP_BLOCKSIZE;

	// This is needed for updating the lighting
	MapNode oldnode = block->getNodeNoCheck(relpos);

	// Remove node metadata
	if (remove_metadata) {
		removeNodeMetadata(p);
	}

	// Set the node on the map
	ContentLightingFlags f = m_nodedef->getLightingFlags(n);
	ContentLightingFlags oldf = m_nodedef->getLightingFlags(oldnode);
	if (f == oldf) {
		// No light update needed, just copy over the old light.
		n.setLight(LIGHTBANK_DAY, oldnode.getLightRaw(LIGHTBANK_DAY, oldf), f);
		n.setLight(LIGHTBANK_NIGHT, oldnode.getLightRaw(LIGHTBANK_NIGHT, oldf), f);
		set_node_in_block(block, relpos, n);

		modified_blocks[blockpos] = block;
	} else {
		// Ignore light (because calling voxalgo::update_lighting_nodes)
		n.setLight(LIGHTBANK_DAY, 0, f);
		n.setLight(LIGHTBANK_NIGHT, 0, f);
		set_node_in_block(block, relpos, n);

		// Update lighting
		std::vector<std::pair<v3s16, MapNode> > oldnodes;
		oldnodes.emplace_back(p, oldnode);
		voxalgo::update_lighting_nodes(this, oldnodes, modified_blocks);

		for (auto &modified_block : modified_blocks) {
			modified_block.second->expireDayNightDiff();
		}
	}
}

void Map::removeNodeAndUpdate(v3s16 p,
		std::map<v3s16, MapBlock*> &modified_blocks)
{
	addNodeAndUpdate(p, MapNode(CONTENT_AIR), modified_blocks, true);
}

bool Map::addNodeWithEvent(v3s16 p, MapNode n, bool remove_metadata)
{
	MapEditEvent event;
	event.type = remove_metadata ? MEET_ADDNODE : MEET_SWAPNODE;
	event.p = p;
	event.n = n;

	bool succeeded = true;
	try{
		std::map<v3s16, MapBlock*> modified_blocks;
		addNodeAndUpdate(p, n, modified_blocks, remove_metadata);

		event.setModifiedBlocks(modified_blocks);
	}
	catch(InvalidPositionException &e){
		succeeded = false;
	}

	dispatchEvent(event);

	return succeeded;
}

bool Map::removeNodeWithEvent(v3s16 p)
{
	MapEditEvent event;
	event.type = MEET_REMOVENODE;
	event.p = p;

	bool succeeded = true;
	try{
		std::map<v3s16, MapBlock*> modified_blocks;
		removeNodeAndUpdate(p, modified_blocks);

		event.setModifiedBlocks(modified_blocks);
	}
	catch(InvalidPositionException &e){
		succeeded = false;
	}

	dispatchEvent(event);

	return succeeded;
}

struct TimeOrderedMapBlock {
	MapSector *sect;
	MapBlock *block;

	TimeOrderedMapBlock(MapSector *sect, MapBlock *block) :
		sect(sect),
		block(block)
	{}

	bool operator<(const TimeOrderedMapBlock &b) const
	{
		return block->getUsageTimer() < b.block->getUsageTimer();
	};
};

/*
	Updates usage timers
*/
void Map::timerUpdate(float dtime, float unload_timeout, s32 max_loaded_blocks,
		std::vector<v3s16> *unloaded_blocks)
{
	bool save_before_unloading = maySaveBlocks();

	// Profile modified reasons
	Profiler modprofiler;

	std::vector<v2s16> sector_deletion_queue;
	u32 deleted_blocks_count = 0;
	u32 saved_blocks_count = 0;
	u32 block_count_all = 0;
	u32 locked_blocks = 0;

	const auto start_time = porting::getTimeUs();
	beginSave();

	// If there is no practical limit, we spare creation of mapblock_queue
	if (max_loaded_blocks < 0) {
		for (auto &sector_it : m_sectors) {
			MapSector *sector = sector_it.second;

			bool all_blocks_deleted = true;

			MapBlockVect blocks;
			sector->getBlocks(blocks);

			for (MapBlock *block : blocks) {
				block->incrementUsageTimer(dtime);

				if (block->refGet() == 0
						&& block->getUsageTimer() > unload_timeout) {
					v3s16 p = block->getPos();

					// Save if modified
					if (block->getModified() != MOD_STATE_CLEAN
							&& save_before_unloading) {
						modprofiler.add(block->getModifiedReasonString(), 1);
						if (!saveBlock(block))
							continue;
						saved_blocks_count++;
					}

					// Delete from memory
					sector->deleteBlock(block);

					if (unloaded_blocks)
						unloaded_blocks->push_back(p);

					deleted_blocks_count++;
				} else {
					all_blocks_deleted = false;
					block_count_all++;
				}
			}

			// Delete sector if we emptied it
			if (all_blocks_deleted) {
				sector_deletion_queue.push_back(sector_it.first);
			}
		}
	} else {
		std::priority_queue<TimeOrderedMapBlock> mapblock_queue;
		for (auto &sector_it : m_sectors) {
			MapSector *sector = sector_it.second;

			MapBlockVect blocks;
			sector->getBlocks(blocks);

			for (MapBlock *block : blocks) {
				block->incrementUsageTimer(dtime);
				mapblock_queue.push(TimeOrderedMapBlock(sector, block));
			}
		}
		block_count_all = mapblock_queue.size();

		// Delete old blocks, and blocks over the limit from the memory
		while (!mapblock_queue.empty() && ((s32)mapblock_queue.size() > max_loaded_blocks
				|| mapblock_queue.top().block->getUsageTimer() > unload_timeout)) {
			TimeOrderedMapBlock b = mapblock_queue.top();
			mapblock_queue.pop();

			MapBlock *block = b.block;

			if (block->refGet() != 0) {
				locked_blocks++;
				continue;
			}

			v3s16 p = block->getPos();

			// Save if modified
			if (block->getModified() != MOD_STATE_CLEAN && save_before_unloading) {
				modprofiler.add(block->getModifiedReasonString(), 1);
				if (!saveBlock(block))
					continue;
				saved_blocks_count++;
			}

			// Delete from memory
			b.sect->deleteBlock(block);

			if (unloaded_blocks)
				unloaded_blocks->push_back(p);

			deleted_blocks_count++;
			block_count_all--;
		}

		// Delete empty sectors
		for (auto &sector_it : m_sectors) {
			if (sector_it.second->empty()) {
				sector_deletion_queue.push_back(sector_it.first);
			}
		}
	}

	endSave();
	const auto end_time = porting::getTimeUs();

	reportMetrics(end_time - start_time, saved_blocks_count, block_count_all);

	// Finally delete the empty sectors
	deleteSectors(sector_deletion_queue);

	if(deleted_blocks_count != 0)
	{
		PrintInfo(infostream); // ServerMap/ClientMap:
		infostream<<"Unloaded "<<deleted_blocks_count
				<<" blocks from memory";
		if(save_before_unloading)
			infostream<<", of which "<<saved_blocks_count<<" were written";
		infostream<<", "<<block_count_all<<" blocks in memory, " << locked_blocks << " locked";
		infostream<<"."<<std::endl;
		if(saved_blocks_count != 0){
			PrintInfo(infostream); // ServerMap/ClientMap:
			infostream<<"Blocks modified by: "<<std::endl;
			modprofiler.print(infostream);
		}
	}
}

void Map::unloadUnreferencedBlocks(std::vector<v3s16> *unloaded_blocks)
{
	timerUpdate(0.0, -1.0, 0, unloaded_blocks);
}

void Map::deleteSectors(std::vector<v2s16> &sectorList)
{
	for (v2s16 j : sectorList) {
		MapSector *sector = m_sectors[j];
		// If sector is in sector cache, remove it from there
		if(m_sector_cache == sector)
			m_sector_cache = NULL;
		// Remove from map and delete
		m_sectors.erase(j);
		delete sector;
	}
}

void Map::PrintInfo(std::ostream &out)
{
	out<<"Map: ";
}

#define WATER_DROP_BOOST 4

const static v3s16 liquid_6dirs[6] = {
	// order: upper before same level before lower
	v3s16( 0, 1, 0),
	v3s16( 0, 0, 1),
	v3s16( 1, 0, 0),
	v3s16( 0, 0,-1),
	v3s16(-1, 0, 0),
	v3s16( 0,-1, 0)
};

enum NeighborType : u8 {
	NEIGHBOR_UPPER,
	NEIGHBOR_SAME_LEVEL,
	NEIGHBOR_LOWER
};

struct NodeNeighbor {
	MapNode n;
	NeighborType t;
	v3s16 p;

	NodeNeighbor()
		: n(CONTENT_AIR), t(NEIGHBOR_SAME_LEVEL)
	{ }

	NodeNeighbor(const MapNode &node, NeighborType n_type, const v3s16 &pos)
		: n(node),
		  t(n_type),
		  p(pos)
	{ }
};

void ServerMap::transforming_liquid_add(v3s16 p) {
        m_transforming_liquid.push_back(p);
}

std::vector<v3s16> Map::findNodesWithMetadata(v3s16 p1, v3s16 p2)
{
	std::vector<v3s16> positions_with_meta;

	sortBoxVerticies(p1, p2);
	v3s16 bpmin = getNodeBlockPos(p1);
	v3s16 bpmax = getNodeBlockPos(p2);

	VoxelArea area(p1, p2);

	for (s16 z = bpmin.Z; z <= bpmax.Z; z++)
	for (s16 y = bpmin.Y; y <= bpmax.Y; y++)
	for (s16 x = bpmin.X; x <= bpmax.X; x++) {
		v3s16 blockpos(x, y, z);

		MapBlock *block = getBlockNoCreateNoEx(blockpos);
		if (!block) {
			verbosestream << "Map::getNodeMetadata(): Need to emerge "
				<< PP(blockpos) << std::endl;
			block = emergeBlock(blockpos, false);
		}
		if (!block) {
			infostream << "WARNING: Map::getNodeMetadata(): Block not found"
				<< std::endl;
			continue;
		}

		v3s16 p_base = blockpos * MAP_BLOCKSIZE;
		std::vector<v3s16> keys = block->m_node_metadata.getAllKeys();
		for (size_t i = 0; i != keys.size(); i++) {
			v3s16 p(keys[i] + p_base);
			if (!area.contains(p))
				continue;

			positions_with_meta.push_back(p);
		}
	}

	return positions_with_meta;
}

NodeMetadata *Map::getNodeMetadata(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if(!block){
		infostream<<"Map::getNodeMetadata(): Need to emerge "
				<<PP(blockpos)<<std::endl;
		block = emergeBlock(blockpos, false);
	}
	if(!block){
		warningstream<<"Map::getNodeMetadata(): Block not found"
				<<std::endl;
		return NULL;
	}
	NodeMetadata *meta = block->m_node_metadata.get(p_rel);
	return meta;
}

bool Map::setNodeMetadata(v3s16 p, NodeMetadata *meta)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if(!block){
		infostream<<"Map::setNodeMetadata(): Need to emerge "
				<<PP(blockpos)<<std::endl;
		block = emergeBlock(blockpos, false);
	}
	if(!block){
		warningstream<<"Map::setNodeMetadata(): Block not found"
				<<std::endl;
		return false;
	}
	block->m_node_metadata.set(p_rel, meta);
	return true;
}

void Map::removeNodeMetadata(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if(block == NULL)
	{
		warningstream<<"Map::removeNodeMetadata(): Block not found"
				<<std::endl;
		return;
	}
	block->m_node_metadata.remove(p_rel);
}

NodeTimer Map::getNodeTimer(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if(!block){
		infostream<<"Map::getNodeTimer(): Need to emerge "
				<<PP(blockpos)<<std::endl;
		block = emergeBlock(blockpos, false);
	}
	if(!block){
		warningstream<<"Map::getNodeTimer(): Block not found"
				<<std::endl;
		return NodeTimer();
	}
	NodeTimer t = block->getNodeTimer(p_rel);
	NodeTimer nt(t.timeout, t.elapsed, p);
	return nt;
}

void Map::setNodeTimer(const NodeTimer &t)
{
	v3s16 p = t.position;
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if(!block){
		infostream<<"Map::setNodeTimer(): Need to emerge "
				<<PP(blockpos)<<std::endl;
		block = emergeBlock(blockpos, false);
	}
	if(!block){
		warningstream<<"Map::setNodeTimer(): Block not found"
				<<std::endl;
		return;
	}
	NodeTimer nt(t.timeout, t.elapsed, p_rel);
	block->setNodeTimer(nt);
}

void Map::removeNodeTimer(v3s16 p)
{
	v3s16 blockpos = getNodeBlockPos(p);
	v3s16 p_rel = p - blockpos*MAP_BLOCKSIZE;
	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if(block == NULL)
	{
		warningstream<<"Map::removeNodeTimer(): Block not found"
				<<std::endl;
		return;
	}
	block->removeNodeTimer(p_rel);
}

bool Map::determineAdditionalOcclusionCheck(const v3s16 &pos_camera,
	const core::aabbox3d<s16> &block_bounds, v3s16 &check)
{
	/*
		This functions determines the node inside the target block that is
		closest to the camera position. This increases the occlusion culling
		accuracy in straight and diagonal corridors.
		The returned position will be occlusion checked first in addition to the
		others (8 corners + center).
		No position is returned if
		- the closest node is a corner, corners are checked anyway.
		- the camera is inside the target block, it will never be occluded.
	*/
#define CLOSEST_EDGE(pos, bounds, axis) \
	((pos).axis <= (bounds).MinEdge.axis) ? (bounds).MinEdge.axis : \
	(bounds).MaxEdge.axis

	bool x_inside = (block_bounds.MinEdge.X <= pos_camera.X) &&
			(pos_camera.X <= block_bounds.MaxEdge.X);
	bool y_inside = (block_bounds.MinEdge.Y <= pos_camera.Y) &&
			(pos_camera.Y <= block_bounds.MaxEdge.Y);
	bool z_inside = (block_bounds.MinEdge.Z <= pos_camera.Z) &&
			(pos_camera.Z <= block_bounds.MaxEdge.Z);

	if (x_inside && y_inside && z_inside)
		return false; // Camera inside target mapblock

	// straight
	if (x_inside && y_inside) {
		check = v3s16(pos_camera.X, pos_camera.Y, 0);
		check.Z = CLOSEST_EDGE(pos_camera, block_bounds, Z);
		return true;
	} else if (y_inside && z_inside) {
		check = v3s16(0, pos_camera.Y, pos_camera.Z);
		check.X = CLOSEST_EDGE(pos_camera, block_bounds, X);
		return true;
	} else if (x_inside && z_inside) {
		check = v3s16(pos_camera.X, 0, pos_camera.Z);
		check.Y = CLOSEST_EDGE(pos_camera, block_bounds, Y);
		return true;
	}

	// diagonal
	if (x_inside) {
		check = v3s16(pos_camera.X, 0, 0);
		check.Y = CLOSEST_EDGE(pos_camera, block_bounds, Y);
		check.Z = CLOSEST_EDGE(pos_camera, block_bounds, Z);
		return true;
	} else if (y_inside) {
		check = v3s16(0, pos_camera.Y, 0);
		check.X = CLOSEST_EDGE(pos_camera, block_bounds, X);
		check.Z = CLOSEST_EDGE(pos_camera, block_bounds, Z);
		return true;
	} else if (z_inside) {
		check = v3s16(0, 0, pos_camera.Z);
		check.X = CLOSEST_EDGE(pos_camera, block_bounds, X);
		check.Y = CLOSEST_EDGE(pos_camera, block_bounds, Y);
		return true;
	}

	// Closest node would be a corner, none returned
	return false;
}

bool Map::isOccluded(const v3s16 &pos_camera, const v3s16 &pos_target,
	float step, float stepfac, float offset, float end_offset, u32 needed_count)
{
	v3f direction = intToFloat(pos_target - pos_camera, BS);
	float distance = direction.getLength();

	// Normalize direction vector
	if (distance > 0.0f)
		direction /= distance;

	v3f pos_origin_f = intToFloat(pos_camera, BS);
	u32 count = 0;
	bool is_valid_position;

	for (; offset < distance + end_offset; offset += step) {
		v3f pos_node_f = pos_origin_f + direction * offset;
		v3s16 pos_node = floatToInt(pos_node_f, BS);

		MapNode node = getNode(pos_node, &is_valid_position);

		if (is_valid_position &&
				!m_nodedef->getLightingFlags(node).light_propagates) {
			// Cannot see through light-blocking nodes --> occluded
			count++;
			if (count >= needed_count)
				return true;
		}
		step *= stepfac;
	}
	return false;
}

bool Map::isBlockOccluded(MapBlock *block, v3s16 cam_pos_nodes)
{
	// Check occlusion for center and all 8 corners of the mapblock
	// Overshoot a little for less flickering
	static const s16 bs2 = MAP_BLOCKSIZE / 2 + 1;
	static const v3s16 dir9[9] = {
		v3s16( 0,  0,  0),
		v3s16( 1,  1,  1) * bs2,
		v3s16( 1,  1, -1) * bs2,
		v3s16( 1, -1,  1) * bs2,
		v3s16( 1, -1, -1) * bs2,
		v3s16(-1,  1,  1) * bs2,
		v3s16(-1,  1, -1) * bs2,
		v3s16(-1, -1,  1) * bs2,
		v3s16(-1, -1, -1) * bs2,
	};

	v3s16 pos_blockcenter = block->getPosRelative() + (MAP_BLOCKSIZE / 2);

	// Starting step size, value between 1m and sqrt(3)m
	float step = BS * 1.2f;
	// Multiply step by each iteraction by 'stepfac' to reduce checks in distance
	float stepfac = 1.05f;

	float start_offset = BS * 1.0f;

	// The occlusion search of 'isOccluded()' must stop short of the target
	// point by distance 'end_offset' to not enter the target mapblock.
	// For the 8 mapblock corners 'end_offset' must therefore be the maximum
	// diagonal of a mapblock, because we must consider all view angles.
	// sqrt(1^2 + 1^2 + 1^2) = 1.732
	float end_offset = -BS * MAP_BLOCKSIZE * 1.732f;

	// to reduce the likelihood of falsely occluded blocks
	// require at least two solid blocks
	// this is a HACK, we should think of a more precise algorithm
	u32 needed_count = 2;

	// Additional occlusion check, see comments in that function
	v3s16 check;
	if (determineAdditionalOcclusionCheck(cam_pos_nodes, block->getBox(), check)) {
		// node is always on a side facing the camera, end_offset can be lower
		if (!isOccluded(cam_pos_nodes, check, step, stepfac, start_offset,
				-1.0f, needed_count))
			return false;
	}

	for (const v3s16 &dir : dir9) {
		if (!isOccluded(cam_pos_nodes, pos_blockcenter + dir, step, stepfac,
				start_offset, end_offset, needed_count))
			return false;
	}
	return true;
}

/*
	ServerMap
*/
ServerMap::ServerMap(const std::string &savedir, IGameDef *gamedef,
		EmergeManager *emerge, MetricsBackend *mb):
	Map(gamedef),
	m_emerge(emerge)
{
	verbosestream<<FUNCTION_NAME<<std::endl;


}

ServerMap::~ServerMap()
{
	verbosestream<<FUNCTION_NAME<<std::endl;

	try
	{
		if (m_map_saving_enabled) {
			// Save only changed parts
			save(MOD_STATE_WRITE_AT_UNLOAD);
			infostream << "ServerMap: Saved map to " << m_savedir << std::endl;
		} else {
			infostream << "ServerMap: Map not saved" << std::endl;
		}
	}
	catch(std::exception &e)
	{
		infostream<<"ServerMap: Failed to save map to "<<m_savedir
				<<", exception: "<<e.what()<<std::endl;
	}

	/*
		Close database if it was opened
	*/
	delete dbase;
	delete dbase_ro;

	deleteDetachedBlocks();
}

bool ServerMap::initBlockMake(v3s16 blockpos, BlockMakeData *data)
{
	return true;
}

void ServerMap::finishBlockMake(BlockMakeData *data,
	std::map<v3s16, MapBlock*> *changed_blocks)
{
	v3s16 bpmin = data->blockpos_min;
	v3s16 bpmax = data->blockpos_max;

	/*
		Blit generated stuff to map
		NOTE: blitBackAll adds nearly everything to changed_blocks
	*/
	data->vmanip->blitBackAll(changed_blocks);

	/*
		Copy transforming liquid information
	*/
	while (data->transforming_liquid.size()) {
		m_transforming_liquid.push_back(data->transforming_liquid.front());
		data->transforming_liquid.pop_front();
	}

	for (auto &changed_block : *changed_blocks) {
		MapBlock *block = changed_block.second;
		if (!block)
			continue;
		/*
			Update day/night difference cache of the MapBlocks
		*/
		block->expireDayNightDiff();
		/*
			Set block as modified
		*/
		block->raiseModified(MOD_STATE_WRITE_NEEDED,
			MOD_REASON_EXPIRE_DAYNIGHTDIFF);
	}

	/*
		Set central blocks as generated
	*/
	for (s16 x = bpmin.X; x <= bpmax.X; x++)
	for (s16 z = bpmin.Z; z <= bpmax.Z; z++)
	for (s16 y = bpmin.Y; y <= bpmax.Y; y++) {
		MapBlock *block = getBlockNoCreateNoEx(v3s16(x, y, z));
		if (!block)
			continue;

		block->setGenerated(true);
	}

	/*
		Save changed parts of map
		NOTE: Will be saved later.
	*/
	//save(MOD_STATE_WRITE_AT_UNLOAD);
	m_chunks_in_progress.erase(bpmin);
}

MapSector *ServerMap::createSector(v2s16 p2d)
{
	/*
		Check if it exists already in memory
	*/
	MapSector *sector = getSectorNoGenerate(p2d);
	if (sector)
		return sector;

	/*
		Do not create over max mapgen limit
	*/
	if (blockpos_over_max_limit(v3s16(p2d.X, 0, p2d.Y)))
		throw InvalidPositionException("createSector(): pos. over max mapgen limit");

	/*
		Generate blank sector
	*/

	sector = new MapSector(this, p2d, m_gamedef);

	/*
		Insert to container
	*/
	m_sectors[p2d] = sector;

	return sector;
}

MapBlock * ServerMap::createBlock(v3s16 p)
{
	/*
		Do not create over max mapgen limit
	*/
	if (blockpos_over_max_limit(p))
		throw InvalidPositionException("createBlock(): pos. over max mapgen limit");

	v2s16 p2d(p.X, p.Z);
	s16 block_y = p.Y;
	/*
		This will create or load a sector if not found in memory.
		If block exists on disk, it will be loaded.

		NOTE: On old save formats, this will be slow, as it generates
		      lighting on blocks for them.
	*/
	MapSector *sector;
	try {
		sector = createSector(p2d);
	} catch (InvalidPositionException &e) {
		infostream<<"createBlock: createSector() failed"<<std::endl;
		throw e;
	}

	/*
		Try to get a block from the sector
	*/

	MapBlock *block = sector->getBlockNoCreateNoEx(block_y);
	if (block) {
		return block;
	}
	// Create blank
	block = sector->createBlankBlock(block_y);

	return block;
}

MapBlock * ServerMap::emergeBlock(v3s16 p, bool create_blank)
{
	{
		MapBlock *block = getBlockNoCreateNoEx(p);
		if (block)
			return block;
	}

	{
		MapBlock *block = loadBlock(p);
		if(block)
			return block;
	}

	if (create_blank) {
		MapSector *sector = createSector(v2s16(p.X, p.Z));
		MapBlock *block = sector->createBlankBlock(p.Y);

		return block;
	}

	return NULL;
}

bool ServerMap::isBlockInQueue(v3s16 pos)
{
	return true;
}

void ServerMap::addNodeAndUpdate(v3s16 p, MapNode n,
		std::map<v3s16, MapBlock*> &modified_blocks,
		bool remove_metadata)
{

}

// N.B.  This requires no synchronization, since data will not be modified unless
// the VoxelManipulator being updated belongs to the same thread.
void ServerMap::updateVManip(v3s16 pos)
{

}

void ServerMap::reportMetrics(u64 save_time_us, u32 saved_blocks, u32 all_blocks)
{

}

void ServerMap::save(ModifiedState save_level)
{

}

void ServerMap::listAllLoadableBlocks(std::vector<v3s16> &dst)
{

}

void ServerMap::listAllLoadedBlocks(std::vector<v3s16> &dst)
{

}

MapDatabase *ServerMap::createDatabase(
	const std::string &name,
	const std::string &savedir,
	Settings &conf)
{
	if (name == "dummy")
		return new Database_Dummy();

	throw BaseException(std::string("Database backend ") + name + " not supported.");
}

void ServerMap::beginSave()
{

}

void ServerMap::endSave()
{

}

bool ServerMap::saveBlock(MapBlock *block)
{
	return true;
}

bool ServerMap::saveBlock(MapBlock *block, MapDatabase *db, int compression_level)
{
	return true;
}

void ServerMap::loadBlock(std::string *blob, v3s16 p3d, MapSector *sector, bool save_after_load)
{

}

MapBlock* ServerMap::loadBlock(v3s16 blockpos)
{
	bool created_new = (getBlockNoCreateNoEx(blockpos) == NULL);

	v2s16 p2d(blockpos.X, blockpos.Z);

	std::string ret;
	dbase->loadBlock(blockpos, &ret);
	if (!ret.empty()) {
		loadBlock(&ret, blockpos, createSector(p2d), false);
	} else if (dbase_ro) {
		dbase_ro->loadBlock(blockpos, &ret);
		if (!ret.empty()) {
			loadBlock(&ret, blockpos, createSector(p2d), false);
		}
	} else {
		return NULL;
	}

	MapBlock *block = getBlockNoCreateNoEx(blockpos);
	if (created_new && (block != NULL)) {
		std::map<v3s16, MapBlock*> modified_blocks;
		// Fix lighting if necessary
		voxalgo::update_block_border_lighting(this, block, modified_blocks);
		if (!modified_blocks.empty()) {
			//Modified lighting, send event
			MapEditEvent event;
			event.type = MEET_OTHER;
			event.setModifiedBlocks(modified_blocks);
			dispatchEvent(event);
		}
	}
	return block;
}

bool ServerMap::deleteBlock(v3s16 blockpos)
{

	return true;
}

void ServerMap::deleteDetachedBlocks()
{

}

void ServerMap::step()
{

}

void ServerMap::PrintInfo(std::ostream &out)
{

}

bool ServerMap::repairBlockLight(v3s16 blockpos,
	std::map<v3s16, MapBlock *> *modified_blocks)
{
	return true;
}

MMVManip::MMVManip(Map *map):
		VoxelManipulator(),
		m_map(map)
{
	assert(map);
}

void MMVManip::initialEmerge(v3s16 blockpos_min, v3s16 blockpos_max,
	bool load_if_inexistent)
{
	TimeTaker timer1("initialEmerge", &emerge_time);

	assert(m_map);

	// Units of these are MapBlocks
	v3s16 p_min = blockpos_min;
	v3s16 p_max = blockpos_max;

	VoxelArea block_area_nodes
			(p_min*MAP_BLOCKSIZE, (p_max+1)*MAP_BLOCKSIZE-v3s16(1,1,1));

	u32 size_MB = block_area_nodes.getVolume()*4/1000000;
	if(size_MB >= 1)
	{
		infostream<<"initialEmerge: area: ";
		block_area_nodes.print(infostream);
		infostream<<" ("<<size_MB<<"MB)";
		infostream<<std::endl;
	}

	addArea(block_area_nodes);

	for(s32 z=p_min.Z; z<=p_max.Z; z++)
	for(s32 y=p_min.Y; y<=p_max.Y; y++)
	for(s32 x=p_min.X; x<=p_max.X; x++)
	{
		u8 flags = 0;
		MapBlock *block;
		v3s16 p(x,y,z);
		std::map<v3s16, u8>::iterator n;
		n = m_loaded_blocks.find(p);
		if(n != m_loaded_blocks.end())
			continue;

		bool block_data_inexistent = false;
		{
			TimeTaker timer2("emerge load", &emerge_load_time);

			block = m_map->getBlockNoCreateNoEx(p);
			if (!block)
				block_data_inexistent = true;
			else
				block->copyTo(*this);
		}

		if(block_data_inexistent)
		{

			if (load_if_inexistent && !blockpos_over_max_limit(p)) {
				ServerMap *svrmap = (ServerMap *)m_map;
				block = svrmap->emergeBlock(p, false);
				if (block == NULL)
					block = svrmap->createBlock(p);
				block->copyTo(*this);
			} else {
				flags |= VMANIP_BLOCK_DATA_INEXIST;

				/*
					Mark area inexistent
				*/
				VoxelArea a(p*MAP_BLOCKSIZE, (p+1)*MAP_BLOCKSIZE-v3s16(1,1,1));
				// Fill with VOXELFLAG_NO_DATA
				for(s32 z=a.MinEdge.Z; z<=a.MaxEdge.Z; z++)
				for(s32 y=a.MinEdge.Y; y<=a.MaxEdge.Y; y++)
				{
					s32 i = m_area.index(a.MinEdge.X,y,z);
					memset(&m_flags[i], VOXELFLAG_NO_DATA, MAP_BLOCKSIZE);
				}
			}
		}
		/*else if (block->getNode(0, 0, 0).getContent() == CONTENT_IGNORE)
		{
			// Mark that block was loaded as blank
			flags |= VMANIP_BLOCK_CONTAINS_CIGNORE;
		}*/

		m_loaded_blocks[p] = flags;
	}

	m_is_dirty = false;
}

void MMVManip::blitBackAll(std::map<v3s16, MapBlock*> *modified_blocks,
	bool overwrite_generated)
{
	if(m_area.getExtent() == v3s16(0,0,0))
		return;
	assert(m_map);

	/*
		Copy data of all blocks
	*/
	for (auto &loaded_block : m_loaded_blocks) {
		v3s16 p = loaded_block.first;
		MapBlock *block = m_map->getBlockNoCreateNoEx(p);
		bool existed = !(loaded_block.second & VMANIP_BLOCK_DATA_INEXIST);
		if (!existed || (block == NULL) ||
			(!overwrite_generated && block->isGenerated()))
			continue;

		block->copyFrom(*this);
		block->raiseModified(MOD_STATE_WRITE_NEEDED, MOD_REASON_VMANIP);

		if(modified_blocks)
			(*modified_blocks)[p] = block;
	}
}

MMVManip *MMVManip::clone() const
{
	MMVManip *ret = new MMVManip();

	const s32 size = m_area.getVolume();
	ret->m_area = m_area;
	if (m_data) {
		ret->m_data = new MapNode[size];
		memcpy(ret->m_data, m_data, size * sizeof(MapNode));
	}
	if (m_flags) {
		ret->m_flags = new u8[size];
		memcpy(ret->m_flags, m_flags, size * sizeof(u8));
	}

	ret->m_is_dirty = m_is_dirty;
	// Even if the copy is disconnected from a map object keep the information
	// needed to write it back to one
	ret->m_loaded_blocks = m_loaded_blocks;

	return ret;
}

void MMVManip::reparent(Map *map)
{
	assert(map && !m_map);
	m_map = map;
}

//END
