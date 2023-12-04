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

#include "object_properties.h"
#include "irrlicht_changes/printing.h"
#include "irrlichttypes_bloated.h"
#include "exceptions.h"
#include "util/serialize.h"
#include <sstream>

static const video::SColor NULL_BGCOLOR{0, 1, 1, 1};

ObjectProperties::ObjectProperties()
{
	textures.emplace_back("no_texture.png");
	colors.emplace_back(255,255,255,255);
}

bool ObjectProperties::validate()
{
	const char *func = "ObjectProperties::validate(): ";
	bool ret = true;

	// cf. where serializeString16 is used below
	for (u32 i = 0; i < textures.size(); i++) {
		if (textures[i].size() > U16_MAX) {
			warningstream << func << "texture " << (i+1) << " has excessive length, "
				"clearing it." << std::endl;
			textures[i].clear();
			ret = false;
		}
	}
	if (nametag.length() > U16_MAX) {
		warningstream << func << "nametag has excessive length, clearing it." << std::endl;
		nametag.clear();
		ret = false;
	}
	if (infotext.length() > U16_MAX) {
		warningstream << func << "infotext has excessive length, clearing it." << std::endl;
		infotext.clear();
		ret = false;
	}
	if (wield_item.length() > U16_MAX) {
		warningstream << func << "wield_item has excessive length, clearing it." << std::endl;
		wield_item.clear();
		ret = false;
	}

	return ret;
}

void ObjectProperties::deSerialize(std::istream &is)
{
	int version = readU8(is);
	if (version != 4)
		throw SerializationError("unsupported ObjectProperties version");

	hp_max = readU16(is);
	physical = readU8(is);
	readU32(is); // removed property (weight)
	collisionbox.MinEdge = readV3F32(is);
	collisionbox.MaxEdge = readV3F32(is);
	selectionbox.MinEdge = readV3F32(is);
	selectionbox.MaxEdge = readV3F32(is);
	pointable = readU8(is);
	visual = deSerializeString16(is);
	visual_size = readV3F32(is);
	textures.clear();
	u32 texture_count = readU16(is);
	for (u32 i = 0; i < texture_count; i++){
		textures.push_back(deSerializeString16(is));
	}
	spritediv = readV2S16(is);
	initial_sprite_basepos = readV2S16(is);
	is_visible = readU8(is);
	makes_footstep_sound = readU8(is);
	automatic_rotate = readF32(is);
	mesh = deSerializeString16(is);
	colors.clear();
	u32 color_count = readU16(is);
	for (u32 i = 0; i < color_count; i++){
		colors.push_back(readARGB8(is));
	}
	collideWithObjects = readU8(is);
	stepheight = readF32(is);
	automatic_face_movement_dir = readU8(is);
	automatic_face_movement_dir_offset = readF32(is);
	backface_culling = readU8(is);
	nametag = deSerializeString16(is);
	nametag_color = readARGB8(is);
	automatic_face_movement_max_rotation_per_sec = readF32(is);
	infotext = deSerializeString16(is);
	wield_item = deSerializeString16(is);
	glow = readS8(is);
	breath_max = readU16(is);
	eye_height = readF32(is);
	zoom_fov = readF32(is);
	use_texture_alpha = readU8(is);
	try {
		damage_texture_modifier = deSerializeString16(is);
		u8 tmp = readU8(is);
		if (is.eof())
			return;
		shaded = tmp;
		tmp = readU8(is);
		if (is.eof())
			return;
		show_on_minimap = tmp;

		auto bgcolor = readARGB8(is);
		if (bgcolor != NULL_BGCOLOR)
			nametag_bgcolor = bgcolor;
		else
			nametag_bgcolor = std::nullopt;

		tmp = readU8(is);
		if (is.eof())
			return;
		rotate_selectionbox = tmp;
	} catch (SerializationError &e) {}
}
