/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "mapblock.h"

#include <sstream>
#include "map.h"
#include "light.h"
#include "nodedef.h"
#include "nodemetadata.h"
#include "gamedef.h"
#include "log.h"
#include "nameidmapping.h"
#include "serialization.h"
#ifndef SERVER
#include "client/mapblock_mesh.h"
#endif
#include "porting.h"
#include "util/string.h"
#include "util/serialize.h"
#include "util/basic_macros.h"

static const char *modified_reason_strings[] = {
	"initial",
	"reallocate",
	"setIsUnderground",
	"setLightingExpired",
	"setGenerated",
	"setNode",
	"setNodeNoCheck",
	"setTimestamp",
	"NodeMetaRef::reportMetadataChange",
	"clearAllObjects",
	"Timestamp expired (step)",
	"addActiveObjectRaw",
	"removeRemovedObjects/remove",
	"removeRemovedObjects/deactivate",
	"Stored list cleared in activateObjects due to overflow",
	"deactivateFarObjects: Static data moved in",
	"deactivateFarObjects: Static data moved out",
	"deactivateFarObjects: Static data changed considerably",
	"finishBlockMake: expireDayNightDiff",
	"unknown",
};

/*
	MapBlock
*/

MapBlock::MapBlock(Map *parent, v3s16 pos, IGameDef *gamedef):
		m_parent(parent),
		m_pos(pos),
		m_pos_relative(pos * MAP_BLOCKSIZE),
		m_gamedef(gamedef)
{
	reallocate();
}

MapBlock::~MapBlock()
{
#ifndef SERVER
	{
		delete mesh;
		mesh = nullptr;
	}
#endif
}

// This method is only for Server, don't call it on client
void MapBlock::step(float dtime, const std::function<bool(v3s16, MapNode, f32)> &on_timer_cb)
{
	// we hab no server
}

bool MapBlock::isValidPositionParent(v3s16 p)
{
	if (isValidPosition(p)) {
		return true;
	}

	return m_parent->isValidPosition(getPosRelative() + p);
}

MapNode MapBlock::getNodeParent(v3s16 p, bool *is_valid_position)
{
	if (!isValidPosition(p))
		return m_parent->getNode(getPosRelative() + p, is_valid_position);

	if (is_valid_position)
		*is_valid_position = true;
	return data[p.Z * zstride + p.Y * ystride + p.X];
}

std::string MapBlock::getModifiedReasonString()
{
	std::string reason;

	const u32 ubound = MYMIN(sizeof(m_modified_reason) * CHAR_BIT,
		ARRLEN(modified_reason_strings));

	for (u32 i = 0; i != ubound; i++) {
		if ((m_modified_reason & (1 << i)) == 0)
			continue;

		reason += modified_reason_strings[i];
		reason += ", ";
	}

	if (reason.length() > 2)
		reason.resize(reason.length() - 2);

	return reason;
}


void MapBlock::copyTo(VoxelManipulator &dst)
{
	v3s16 data_size(MAP_BLOCKSIZE, MAP_BLOCKSIZE, MAP_BLOCKSIZE);
	VoxelArea data_area(v3s16(0,0,0), data_size - v3s16(1,1,1));

	// Copy from data to VoxelManipulator
	dst.copyFrom(data, data_area, v3s16(0,0,0),
			getPosRelative(), data_size);
}

void MapBlock::copyFrom(VoxelManipulator &dst)
{
	v3s16 data_size(MAP_BLOCKSIZE, MAP_BLOCKSIZE, MAP_BLOCKSIZE);
	VoxelArea data_area(v3s16(0,0,0), data_size - v3s16(1,1,1));

	// Copy from VoxelManipulator to data
	dst.copyTo(data, data_area, v3s16(0,0,0),
			getPosRelative(), data_size);
}

void MapBlock::actuallyUpdateDayNightDiff()
{
	const NodeDefManager *nodemgr = m_gamedef->ndef();

	// Running this function un-expires m_day_night_differs
	m_day_night_differs_expired = false;

	bool differs = false;

	/*
		Check if any lighting value differs
	*/

	MapNode previous_n(CONTENT_IGNORE);
	for (u32 i = 0; i < nodecount; i++) {
		MapNode n = data[i];

		// If node is identical to previous node, don't verify if it differs
		if (n == previous_n)
			continue;

		differs = !n.isLightDayNightEq(nodemgr->getLightingFlags(n));
		if (differs)
			break;
		previous_n = n;
	}

	/*
		If some lighting values differ, check if the whole thing is
		just air. If it is just air, differs = false
	*/
	if (differs) {
		bool only_air = true;
		for (u32 i = 0; i < nodecount; i++) {
			MapNode &n = data[i];
			if (n.getContent() != CONTENT_AIR) {
				only_air = false;
				break;
			}
		}
		if (only_air)
			differs = false;
	}

	// Set member variable
	m_day_night_differs = differs;
}

void MapBlock::expireDayNightDiff()
{
	m_day_night_differs_expired = true;
}

/*
	Serialization
*/

void MapBlock::serialize(std::ostream &os_compressed, u8 version, bool disk, int compression_level)
{
	if(!ser_ver_supported(version))
		throw VersionMismatchException("ERROR: MapBlock format not supported");

	FATAL_ERROR_IF(version < SER_FMT_VER_LOWEST_WRITE, "Serialization version error");

	std::ostringstream os_raw(std::ios_base::binary);
	std::ostream &os = version >= 29 ? os_raw : os_compressed;

	// First byte
	u8 flags = 0;
	if(is_underground)
		flags |= 0x01;
	if(getDayNightDiff())
		flags |= 0x02;
	if (!m_generated)
		flags |= 0x08;
	writeU8(os, flags);
	if (version >= 27) {
		writeU16(os, m_lighting_complete);
	}

	/*
		Bulk node data
	*/
	SharedBuffer<u8> buf;
	const u8 content_width = 2;
	const u8 params_width = 2;

	buf = MapNode::serializeBulk(version, data, nodecount,
			content_width, params_width);

	writeU8(os, content_width);
	writeU8(os, params_width);
	if (version >= 29) {
		os.write(reinterpret_cast<char*>(*buf), buf.getSize());
	} else {
		// prior to 29 node data was compressed individually
		compress(buf, os, version, compression_level);
	}

	/*
		Node metadata
	*/
	if (version >= 29) {
		m_node_metadata.serialize(os, version, disk);
	} else {
		// use os_raw from above to avoid allocating another stream object
		m_node_metadata.serialize(os_raw, version, disk);
		// prior to 29 node data was compressed individually
		compress(os_raw.str(), os, version, compression_level);
	}

	if (version >= 29) {
		// now compress the whole thing
		compress(os_raw.str(), os_compressed, version, compression_level);
	}
}

void MapBlock::serializeNetworkSpecific(std::ostream &os)
{
	writeU8(os, 2); // version
}

void MapBlock::deSerialize(std::istream &in_compressed, u8 version, bool disk)
{
	if(!ser_ver_supported(version))
		throw VersionMismatchException("ERROR: MapBlock format not supported");

	TRACESTREAM(<<"MapBlock::deSerialize "<<PP(getPos())<<std::endl);

	m_day_night_differs_expired = false;

	// Decompress the whole block (version >= 29)
	std::stringstream in_raw(std::ios_base::binary | std::ios_base::in | std::ios_base::out);
	if (version >= 29)
		decompress(in_compressed, in_raw, version);
	std::istream &is = version >= 29 ? in_raw : in_compressed;

	u8 flags = readU8(is);
	is_underground = (flags & 0x01) != 0;
	m_day_night_differs = (flags & 0x02) != 0;
	if (version < 27)
		m_lighting_complete = 0xFFFF;
	else
		m_lighting_complete = readU16(is);
	m_generated = (flags & 0x08) == 0;

	TRACESTREAM(<<"MapBlock::deSerialize "<<PP(getPos())
			<<": Bulk node data"<<std::endl);
	u8 content_width = readU8(is);
	u8 params_width = readU8(is);
	if(content_width != 1 && content_width != 2)
		throw SerializationError("MapBlock::deSerialize(): invalid content_width");
	if(params_width != 2)
		throw SerializationError("MapBlock::deSerialize(): invalid params_width");

	/*
		Bulk node data
	*/
	if (version >= 29) {
		MapNode::deSerializeBulk(is, version, data, nodecount,
			content_width, params_width);
	} else {
		// use in_raw from above to avoid allocating another stream object
		decompress(is, in_raw, version);
		MapNode::deSerializeBulk(in_raw, version, data, nodecount,
			content_width, params_width);
	}

	/*
		NodeMetadata
	*/
	TRACESTREAM(<<"MapBlock::deSerialize "<<PP(getPos())
			<<": Node metadata"<<std::endl);
	if (version >= 29) {
		m_node_metadata.deSerialize(is, m_gamedef->idef());
	} else {
		try {
			// reuse in_raw
			in_raw.str("");
			in_raw.clear();
			decompress(is, in_raw, version);

			m_node_metadata.deSerialize(in_raw, m_gamedef->idef());
		} catch(SerializationError &e) {
			warningstream<<"MapBlock::deSerialize(): Ignoring an error"
					<<" while deserializing node metadata at ("
					<<PP(getPos())<<": "<<e.what()<<std::endl;
		}
	}

	TRACESTREAM(<<"MapBlock::deSerialize "<<PP(getPos())
			<<": Done."<<std::endl);
}

void MapBlock::deSerializeNetworkSpecific(std::istream &is)
{
	try {
		readU8(is);
		//const u8 version = readU8(is);
		//if (version != 1)
			//throw SerializationError("unsupported MapBlock version");

	} catch(SerializationError &e) {
		warningstream<<"MapBlock::deSerializeNetworkSpecific(): Ignoring an error"
				<<": "<<e.what()<<std::endl;
	}
}

//END
