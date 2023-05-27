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
#include "common/c_content.h"
#include "common/c_converter.h"
#include "common/c_types.h"
#include "nodedef.h"
#include "object_properties.h"
#include "collision.h"
#include "cpp_api/s_node.h"
#include "lua_api/l_object.h"
#include "lua_api/l_item.h"
#include "common/c_internal.h"
#include "server.h"
#include "log.h"
#include "tool.h"
#include "porting.h"
#include "noise.h"
#include "util/pointedthing.h"
#include "debug.h" // For FATAL_ERROR
#include <json/json.h>

struct EnumString es_TileAnimationType[] =
{
	{TAT_NONE, "none"},
	{TAT_VERTICAL_FRAMES, "vertical_frames"},
	{TAT_SHEET_2D, "sheet_2d"},
	{0, nullptr},
};

/******************************************************************************/
void push_item_definition(lua_State *L, const ItemDefinition &i)
{
	lua_newtable(L);
	lua_pushstring(L, i.name.c_str());
	lua_setfield(L, -2, "name");
	lua_pushstring(L, i.description.c_str());
	lua_setfield(L, -2, "description");
}

void push_item_definition_full(lua_State *L, const ItemDefinition &i)
{
	std::string type(es_ItemType[(int)i.type].str);

	lua_newtable(L);
	lua_pushstring(L, i.name.c_str());
	lua_setfield(L, -2, "name");
	lua_pushstring(L, i.description.c_str());
	lua_setfield(L, -2, "description");
	if (!i.short_description.empty()) {
		lua_pushstring(L, i.short_description.c_str());
		lua_setfield(L, -2, "short_description");
	}
	lua_pushstring(L, type.c_str());
	lua_setfield(L, -2, "type");
	lua_pushstring(L, i.inventory_image.c_str());
	lua_setfield(L, -2, "inventory_image");
	lua_pushstring(L, i.inventory_overlay.c_str());
	lua_setfield(L, -2, "inventory_overlay");
	lua_pushstring(L, i.wield_image.c_str());
	lua_setfield(L, -2, "wield_image");
	lua_pushstring(L, i.wield_overlay.c_str());
	lua_setfield(L, -2, "wield_overlay");
	lua_pushstring(L, i.palette_image.c_str());
	lua_setfield(L, -2, "palette_image");
	push_ARGB8(L, i.color);
	lua_setfield(L, -2, "color");
	push_v3f(L, i.wield_scale);
	lua_setfield(L, -2, "wield_scale");
	lua_pushinteger(L, i.stack_max);
	lua_setfield(L, -2, "stack_max");
	lua_pushboolean(L, i.usable);
	lua_setfield(L, -2, "usable");
	lua_pushboolean(L, i.liquids_pointable);
	lua_setfield(L, -2, "liquids_pointable");
	if (i.tool_capabilities) {
		push_tool_capabilities(L, *i.tool_capabilities);
		lua_setfield(L, -2, "tool_capabilities");
	}
	push_groups(L, i.groups);
	lua_setfield(L, -2, "groups");
	push_soundspec(L, i.sound_place);
	lua_setfield(L, -2, "sound_place");
	push_soundspec(L, i.sound_place_failed);
	lua_setfield(L, -2, "sound_place_failed");
	lua_pushstring(L, i.node_placement_prediction.c_str());
	lua_setfield(L, -2, "node_placement_prediction");
}

void push_content_features(lua_State *L, const ContentFeatures &c)
{
	std::string paramtype(ScriptApiNode::es_ContentParamType[(int)c.param_type].str);
	std::string paramtype2(ScriptApiNode::es_ContentParamType2[(int)c.param_type_2].str);
	std::string drawtype(ScriptApiNode::es_DrawType[(int)c.drawtype].str);
	std::string liquid_type(ScriptApiNode::es_LiquidType[(int)c.liquid_type].str);

	/* Missing "tiles" because I don't see a usecase (at least not yet). */

	lua_newtable(L);
	lua_pushboolean(L, c.has_on_construct);
	lua_setfield(L, -2, "has_on_construct");
	lua_pushboolean(L, c.has_on_destruct);
	lua_setfield(L, -2, "has_on_destruct");
	lua_pushboolean(L, c.has_after_destruct);
	lua_setfield(L, -2, "has_after_destruct");
	lua_pushstring(L, c.name.c_str());
	lua_setfield(L, -2, "name");
	push_groups(L, c.groups);
	lua_setfield(L, -2, "groups");
	lua_pushstring(L, paramtype.c_str());
	lua_setfield(L, -2, "paramtype");
	lua_pushstring(L, paramtype2.c_str());
	lua_setfield(L, -2, "paramtype2");
	lua_pushstring(L, drawtype.c_str());
	lua_setfield(L, -2, "drawtype");
	if (!c.mesh.empty()) {
		lua_pushstring(L, c.mesh.c_str());
		lua_setfield(L, -2, "mesh");
	}
#ifndef SERVER
	push_ARGB8(L, c.minimap_color);       // I know this is not set-able w/ register_node,
	lua_setfield(L, -2, "minimap_color"); // but the people need to know!
#endif
	lua_pushnumber(L, c.visual_scale);
	lua_setfield(L, -2, "visual_scale");
	lua_pushnumber(L, c.alpha);
	lua_setfield(L, -2, "alpha");
	if (!c.palette_name.empty()) {
		push_ARGB8(L, c.color);
		lua_setfield(L, -2, "color");

		lua_pushstring(L, c.palette_name.c_str());
		lua_setfield(L, -2, "palette_name");

		push_palette(L, c.palette);
		lua_setfield(L, -2, "palette");
	}
	lua_pushnumber(L, c.waving);
	lua_setfield(L, -2, "waving");
	lua_pushnumber(L, c.connect_sides);
	lua_setfield(L, -2, "connect_sides");

	lua_createtable(L, c.connects_to.size(), 0);
	u16 i = 1;
	for (const std::string &it : c.connects_to) {
		lua_pushlstring(L, it.c_str(), it.size());
		lua_rawseti(L, -2, i++);
	}
	lua_setfield(L, -2, "connects_to");

	push_ARGB8(L, c.post_effect_color);
	lua_setfield(L, -2, "post_effect_color");
	lua_pushnumber(L, c.leveled);
	lua_setfield(L, -2, "leveled");
	lua_pushnumber(L, c.leveled_max);
	lua_setfield(L, -2, "leveled_max");
	lua_pushboolean(L, c.sunlight_propagates);
	lua_setfield(L, -2, "sunlight_propagates");
	lua_pushnumber(L, c.light_source);
	lua_setfield(L, -2, "light_source");
	lua_pushboolean(L, c.is_ground_content);
	lua_setfield(L, -2, "is_ground_content");
	lua_pushboolean(L, c.walkable);
	lua_setfield(L, -2, "walkable");
	lua_pushboolean(L, c.pointable);
	lua_setfield(L, -2, "pointable");
	lua_pushboolean(L, c.diggable);
	lua_setfield(L, -2, "diggable");
	lua_pushboolean(L, c.climbable);
	lua_setfield(L, -2, "climbable");
	lua_pushboolean(L, c.buildable_to);
	lua_setfield(L, -2, "buildable_to");
	lua_pushboolean(L, c.rightclickable);
	lua_setfield(L, -2, "rightclickable");
	lua_pushnumber(L, c.damage_per_second);
	lua_setfield(L, -2, "damage_per_second");
	if (c.isLiquid()) {
		lua_pushstring(L, liquid_type.c_str());
		lua_setfield(L, -2, "liquid_type");
		lua_pushstring(L, c.liquid_alternative_flowing.c_str());
		lua_setfield(L, -2, "liquid_alternative_flowing");
		lua_pushstring(L, c.liquid_alternative_source.c_str());
		lua_setfield(L, -2, "liquid_alternative_source");
		lua_pushnumber(L, c.liquid_viscosity);
		lua_setfield(L, -2, "liquid_viscosity");
		lua_pushboolean(L, c.liquid_renewable);
		lua_setfield(L, -2, "liquid_renewable");
		lua_pushnumber(L, c.liquid_range);
		lua_setfield(L, -2, "liquid_range");
	}
	lua_pushnumber(L, c.drowning);
	lua_setfield(L, -2, "drowning");
	lua_pushboolean(L, c.floodable);
	lua_setfield(L, -2, "floodable");
	push_nodebox(L, c.node_box);
	lua_setfield(L, -2, "node_box");
	push_nodebox(L, c.selection_box);
	lua_setfield(L, -2, "selection_box");
	push_nodebox(L, c.collision_box);
	lua_setfield(L, -2, "collision_box");
	lua_newtable(L);
	push_soundspec(L, c.sound_footstep);
	lua_setfield(L, -2, "sound_footstep");
	push_soundspec(L, c.sound_dig);
	lua_setfield(L, -2, "sound_dig");
	push_soundspec(L, c.sound_dug);
	lua_setfield(L, -2, "sound_dug");
	lua_setfield(L, -2, "sounds");
	lua_pushboolean(L, c.legacy_facedir_simple);
	lua_setfield(L, -2, "legacy_facedir_simple");
	lua_pushboolean(L, c.legacy_wallmounted);
	lua_setfield(L, -2, "legacy_wallmounted");
	lua_pushstring(L, c.node_dig_prediction.c_str());
	lua_setfield(L, -2, "node_dig_prediction");
	lua_pushnumber(L, c.move_resistance);
	lua_setfield(L, -2, "move_resistance");
	lua_pushboolean(L, c.liquid_move_physics);
	lua_setfield(L, -2, "liquid_move_physics");
}

/******************************************************************************/
void push_nodebox(lua_State *L, const NodeBox &box)
{
	lua_newtable(L);
	switch (box.type)
	{
		case NODEBOX_REGULAR:
			lua_pushstring(L, "regular");
			lua_setfield(L, -2, "type");
			break;
		case NODEBOX_LEVELED:
		case NODEBOX_FIXED:
			lua_pushstring(L, "fixed");
			lua_setfield(L, -2, "type");
			push_box(L, box.fixed);
			lua_setfield(L, -2, "fixed");
			break;
		case NODEBOX_WALLMOUNTED:
			lua_pushstring(L, "wallmounted");
			lua_setfield(L, -2, "type");
			push_aabb3f(L, box.wall_top);
			lua_setfield(L, -2, "wall_top");
			push_aabb3f(L, box.wall_bottom);
			lua_setfield(L, -2, "wall_bottom");
			push_aabb3f(L, box.wall_side);
			lua_setfield(L, -2, "wall_side");
			break;
		case NODEBOX_CONNECTED: {
			lua_pushstring(L, "connected");
			lua_setfield(L, -2, "type");
			const auto &c = box.getConnected();
			push_box(L, c.connect_top);
			lua_setfield(L, -2, "connect_top");
			push_box(L, c.connect_bottom);
			lua_setfield(L, -2, "connect_bottom");
			push_box(L, c.connect_front);
			lua_setfield(L, -2, "connect_front");
			push_box(L, c.connect_back);
			lua_setfield(L, -2, "connect_back");
			push_box(L, c.connect_left);
			lua_setfield(L, -2, "connect_left");
			push_box(L, c.connect_right);
			lua_setfield(L, -2, "connect_right");
			// half the boxes are missing here?
			break;
		}
		default:
			FATAL_ERROR("Invalid box.type");
			break;
	}
}

void push_box(lua_State *L, const std::vector<aabb3f> &box)
{
	lua_createtable(L, box.size(), 0);
	u8 i = 1;
	for (const aabb3f &it : box) {
		push_aabb3f(L, it);
		lua_rawseti(L, -2, i++);
	}
}

/******************************************************************************/
void push_palette(lua_State *L, const std::vector<video::SColor> *palette)
{
	lua_createtable(L, palette->size(), 0);
	int newTable = lua_gettop(L);
	int index = 1;
	std::vector<video::SColor>::const_iterator iter;
	for (iter = palette->begin(); iter != palette->end(); ++iter) {
		push_ARGB8(L, (*iter));
		lua_rawseti(L, newTable, index);
		index++;
	}
}

/******************************************************************************/
void read_soundspec(lua_State *L, int index, SimpleSoundSpec &spec)
{
	if(index < 0)
		index = lua_gettop(L) + 1 + index;
	if (lua_isnil(L, index))
		return;

	if (lua_istable(L, index)) {
		getstringfield(L, index, "name", spec.name);
		getfloatfield(L, index, "gain", spec.gain);
		getfloatfield(L, index, "fade", spec.fade);
		getfloatfield(L, index, "pitch", spec.pitch);
	} else if (lua_isstring(L, index)) {
		spec.name = lua_tostring(L, index);
	}
}

void push_soundspec(lua_State *L, const SimpleSoundSpec &spec)
{
	lua_createtable(L, 0, 3);
	lua_pushstring(L, spec.name.c_str());
	lua_setfield(L, -2, "name");
	lua_pushnumber(L, spec.gain);
	lua_setfield(L, -2, "gain");
	lua_pushnumber(L, spec.fade);
	lua_setfield(L, -2, "fade");
	lua_pushnumber(L, spec.pitch);
	lua_setfield(L, -2, "pitch");
}

/******************************************************************************/
NodeBox read_nodebox(lua_State *L, int index)
{
	NodeBox nodebox;
	if (lua_isnil(L, -1))
		return nodebox;

	luaL_checktype(L, -1, LUA_TTABLE);

	nodebox.type = (NodeBoxType)getenumfield(L, index, "type",
			ScriptApiNode::es_NodeBoxType, NODEBOX_REGULAR);

#define NODEBOXREAD(n, s){ \
		lua_getfield(L, index, (s)); \
		if (lua_istable(L, -1)) \
			(n) = read_aabb3f(L, -1, BS); \
		lua_pop(L, 1); \
	}

#define NODEBOXREADVEC(n, s) \
	lua_getfield(L, index, (s)); \
	if (lua_istable(L, -1)) \
		(n) = read_aabb3f_vector(L, -1, BS); \
	lua_pop(L, 1);

	NODEBOXREADVEC(nodebox.fixed, "fixed");
	NODEBOXREAD(nodebox.wall_top, "wall_top");
	NODEBOXREAD(nodebox.wall_bottom, "wall_bottom");
	NODEBOXREAD(nodebox.wall_side, "wall_side");

	if (nodebox.type == NODEBOX_CONNECTED) {
		auto &c = nodebox.getConnected();
		NODEBOXREADVEC(c.connect_top, "connect_top");
		NODEBOXREADVEC(c.connect_bottom, "connect_bottom");
		NODEBOXREADVEC(c.connect_front, "connect_front");
		NODEBOXREADVEC(c.connect_left, "connect_left");
		NODEBOXREADVEC(c.connect_back, "connect_back");
		NODEBOXREADVEC(c.connect_right, "connect_right");
		NODEBOXREADVEC(c.disconnected_top, "disconnected_top");
		NODEBOXREADVEC(c.disconnected_bottom, "disconnected_bottom");
		NODEBOXREADVEC(c.disconnected_front, "disconnected_front");
		NODEBOXREADVEC(c.disconnected_left, "disconnected_left");
		NODEBOXREADVEC(c.disconnected_back, "disconnected_back");
		NODEBOXREADVEC(c.disconnected_right, "disconnected_right");
		NODEBOXREADVEC(c.disconnected, "disconnected");
		NODEBOXREADVEC(c.disconnected_sides, "disconnected_sides");
	}

	return nodebox;
}

/******************************************************************************/
MapNode readnode(lua_State *L, int index)
{
	lua_pushvalue(L, index);
	lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_READ_NODE);
	lua_insert(L, -2);
	lua_call(L, 1, 3);
	content_t content = lua_tointeger(L, -3);
	u8 param1 = lua_tointeger(L, -2);
	u8 param2 = lua_tointeger(L, -1);
	lua_pop(L, 3);
	return MapNode(content, param1, param2);
}

/******************************************************************************/
void pushnode(lua_State *L, const MapNode &n)
{
	lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_PUSH_NODE);
	lua_pushinteger(L, n.getContent());
	lua_pushinteger(L, n.getParam1());
	lua_pushinteger(L, n.getParam2());
	lua_call(L, 3, 1);
}

/******************************************************************************/
int getenumfield(lua_State *L, int table,
		const char *fieldname, const EnumString *spec, int default_)
{
	int result = default_;
	string_to_enum(spec, result,
			getstringfield_default(L, table, fieldname, ""));
	return result;
}

/******************************************************************************/
bool string_to_enum(const EnumString *spec, int &result,
		const std::string &str)
{
	const EnumString *esp = spec;
	while(esp->str){
		if (!strcmp(str.c_str(), esp->str)) {
			result = esp->num;
			return true;
		}
		esp++;
	}
	return false;
}

/******************************************************************************/
ItemStack read_item(lua_State* L, int index, IItemDefManager *idef)
{
	if(index < 0)
		index = lua_gettop(L) + 1 + index;

	if (lua_isnil(L, index)) {
		return ItemStack();
	}

	if (lua_isuserdata(L, index)) {
		// Convert from LuaItemStack
		LuaItemStack *o = ModApiBase::checkObject<LuaItemStack>(L, index);
		return o->getItem();
	}

	if (lua_isstring(L, index)) {
		// Convert from itemstring
		std::string itemstring = lua_tostring(L, index);
		try
		{
			ItemStack item;
			item.deSerialize(itemstring, idef);
			return item;
		}
		catch(SerializationError &e)
		{
			warningstream<<"unable to create item from itemstring"
					<<": "<<itemstring<<std::endl;
			return ItemStack();
		}
	}
	else if(lua_istable(L, index))
	{
		// Convert from table
		std::string name = getstringfield_default(L, index, "name", "");
		int count = getintfield_default(L, index, "count", 1);
		int wear = getintfield_default(L, index, "wear", 0);

		ItemStack istack(name, count, wear, idef);

		// BACKWARDS COMPATIBLITY
		std::string value = getstringfield_default(L, index, "metadata", "");
		istack.metadata.setString("", value);

		// Get meta
		lua_getfield(L, index, "meta");
		int fieldstable = lua_gettop(L);
		if (lua_istable(L, fieldstable)) {
			lua_pushnil(L);
			while (lua_next(L, fieldstable) != 0) {
				// key at index -2 and value at index -1
				std::string key = lua_tostring(L, -2);
				size_t value_len;
				const char *value_cs = lua_tolstring(L, -1, &value_len);
				std::string value(value_cs, value_len);
				istack.metadata.setString(key, value);
				lua_pop(L, 1); // removes value, keeps key for next iteration
			}
		}

		return istack;
	} else {
		throw LuaError("Expecting itemstack, itemstring, table or nil");
	}
}

/******************************************************************************/
void push_tool_capabilities(lua_State *L,
		const ToolCapabilities &toolcap)
{
	lua_newtable(L);
	setfloatfield(L, -1, "full_punch_interval", toolcap.full_punch_interval);
	setintfield(L, -1, "max_drop_level", toolcap.max_drop_level);
	setintfield(L, -1, "punch_attack_uses", toolcap.punch_attack_uses);
		// Create groupcaps table
		lua_newtable(L);
		// For each groupcap
		for (const auto &gc_it : toolcap.groupcaps) {
			// Create groupcap table
			lua_newtable(L);
			const std::string &name = gc_it.first;
			const ToolGroupCap &groupcap = gc_it.second;
			// Create subtable "times"
			lua_newtable(L);
			for (auto time : groupcap.times) {
				lua_pushinteger(L, time.first);
				lua_pushnumber(L, time.second);
				lua_settable(L, -3);
			}
			// Set subtable "times"
			lua_setfield(L, -2, "times");
			// Set simple parameters
			setintfield(L, -1, "maxlevel", groupcap.maxlevel);
			setintfield(L, -1, "uses", groupcap.uses);
			// Insert groupcap table into groupcaps table
			lua_setfield(L, -2, name.c_str());
		}
		// Set groupcaps table
		lua_setfield(L, -2, "groupcaps");
		//Create damage_groups table
		lua_newtable(L);
		// For each damage group
		for (const auto &damageGroup : toolcap.damageGroups) {
			// Create damage group table
			lua_pushinteger(L, damageGroup.second);
			lua_setfield(L, -2, damageGroup.first.c_str());
		}
		lua_setfield(L, -2, "damage_groups");
}

/******************************************************************************/
void push_inventory_list(lua_State *L, const InventoryList &invlist)
{
	push_items(L, invlist.getItems());
}

/******************************************************************************/
void push_inventory_lists(lua_State *L, const Inventory &inv)
{
	const auto &lists = inv.getLists();
	lua_createtable(L, 0, lists.size());
	for(const InventoryList *list : lists) {
		const std::string &name = list->getName();
		lua_pushlstring(L, name.c_str(), name.size());
		push_inventory_list(L, *list);
		lua_rawset(L, -3);
	}
}

/******************************************************************************/
void read_inventory_list(lua_State *L, int tableindex,
		Inventory *inv, const char *name, IGameDef *gdef, int forcesize)
{
	if(tableindex < 0)
		tableindex = lua_gettop(L) + 1 + tableindex;

	// If nil, delete list
	if(lua_isnil(L, tableindex)){
		inv->deleteList(name);
		return;
	}

	// Get Lua-specified items to insert into the list
	std::vector<ItemStack> items = read_items(L, tableindex, gdef);
	size_t listsize = (forcesize >= 0) ? forcesize : items.size();

	// Create or resize/clear list
	InventoryList *invlist = inv->addList(name, listsize);
	if (!invlist) {
		luaL_error(L, "inventory list: cannot create list named '%s'", name);
		return;
	}

	for (size_t i = 0; i < items.size(); ++i) {
		if (i == listsize)
			break; // Truncate provided list of items
		invlist->changeItem(i, items[i]);
	}
}

/******************************************************************************/
struct TileAnimationParams read_animation_definition(lua_State *L, int index)
{
	if(index < 0)
		index = lua_gettop(L) + 1 + index;

	struct TileAnimationParams anim;
	anim.type = TAT_NONE;
	if (!lua_istable(L, index))
		return anim;

	anim.type = (TileAnimationType)
		getenumfield(L, index, "type", es_TileAnimationType,
		TAT_NONE);
	if (anim.type == TAT_VERTICAL_FRAMES) {
		// {type="vertical_frames", aspect_w=16, aspect_h=16, length=2.0}
		anim.vertical_frames.aspect_w =
			getintfield_default(L, index, "aspect_w", 16);
		anim.vertical_frames.aspect_h =
			getintfield_default(L, index, "aspect_h", 16);
		anim.vertical_frames.length =
			getfloatfield_default(L, index, "length", 1.0);
	} else if (anim.type == TAT_SHEET_2D) {
		// {type="sheet_2d", frames_w=5, frames_h=3, frame_length=0.5}
		getintfield(L, index, "frames_w",
			anim.sheet_2d.frames_w);
		getintfield(L, index, "frames_h",
			anim.sheet_2d.frames_h);
		getfloatfield(L, index, "frame_length",
			anim.sheet_2d.frame_length);
	}

	return anim;
}

/******************************************************************************/
ToolCapabilities read_tool_capabilities(
		lua_State *L, int table)
{
	ToolCapabilities toolcap;
	getfloatfield(L, table, "full_punch_interval", toolcap.full_punch_interval);
	getintfield(L, table, "max_drop_level", toolcap.max_drop_level);
	getintfield(L, table, "punch_attack_uses", toolcap.punch_attack_uses);
	lua_getfield(L, table, "groupcaps");
	if(lua_istable(L, -1)){
		int table_groupcaps = lua_gettop(L);
		lua_pushnil(L);
		while(lua_next(L, table_groupcaps) != 0){
			// key at index -2 and value at index -1
			std::string groupname = luaL_checkstring(L, -2);
			if(lua_istable(L, -1)){
				int table_groupcap = lua_gettop(L);
				// This will be created
				ToolGroupCap groupcap;
				// Read simple parameters
				getintfield(L, table_groupcap, "maxlevel", groupcap.maxlevel);
				getintfield(L, table_groupcap, "uses", groupcap.uses);
				// DEPRECATED: maxwear
				float maxwear = 0;
				if (getfloatfield(L, table_groupcap, "maxwear", maxwear)){
					if (maxwear != 0)
						groupcap.uses = 1.0/maxwear;
					else
						groupcap.uses = 0;
					warningstream << "Field \"maxwear\" is deprecated; "
							<< "replace with uses=1/maxwear" << std::endl;
					infostream << script_get_backtrace(L) << std::endl;
				}
				// Read "times" table
				lua_getfield(L, table_groupcap, "times");
				if(lua_istable(L, -1)){
					int table_times = lua_gettop(L);
					lua_pushnil(L);
					while(lua_next(L, table_times) != 0){
						// key at index -2 and value at index -1
						int rating = luaL_checkinteger(L, -2);
						float time = luaL_checknumber(L, -1);
						groupcap.times[rating] = time;
						// removes value, keeps key for next iteration
						lua_pop(L, 1);
					}
				}
				lua_pop(L, 1);
				// Insert groupcap into toolcap
				toolcap.groupcaps[groupname] = groupcap;
			}
			// removes value, keeps key for next iteration
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	lua_getfield(L, table, "damage_groups");
	if(lua_istable(L, -1)){
		int table_damage_groups = lua_gettop(L);
		lua_pushnil(L);
		while(lua_next(L, table_damage_groups) != 0){
			// key at index -2 and value at index -1
			std::string groupname = luaL_checkstring(L, -2);
			u16 value = luaL_checkinteger(L, -1);
			toolcap.damageGroups[groupname] = value;
			// removes value, keeps key for next iteration
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	return toolcap;
}

/******************************************************************************/
/* Lua Stored data!                                                           */
/******************************************************************************/

/******************************************************************************/
void push_groups(lua_State *L, const ItemGroupList &groups)
{
	lua_createtable(L, 0, groups.size());
	for (const auto &group : groups) {
		lua_pushinteger(L, group.second);
		lua_setfield(L, -2, group.first.c_str());
	}
}

/******************************************************************************/
void push_items(lua_State *L, const std::vector<ItemStack> &items)
{
	lua_createtable(L, items.size(), 0);
	for (u32 i = 0; i != items.size(); i++) {
		LuaItemStack::create(L, items[i]);
		lua_rawseti(L, -2, i + 1);
	}
}

/******************************************************************************/
std::vector<ItemStack> read_items(lua_State *L, int index, IGameDef *gdef)
{
	if(index < 0)
		index = lua_gettop(L) + 1 + index;

	std::vector<ItemStack> items;
	luaL_checktype(L, index, LUA_TTABLE);
	lua_pushnil(L);
	while (lua_next(L, index)) {
		s32 key = luaL_checkinteger(L, -2);
		if (key < 1) {
			throw LuaError("Invalid inventory list index");
		}
		if (items.size() < (u32) key) {
			items.resize(key);
		}
		items[key - 1] = read_item(L, -1, gdef->idef());
		lua_pop(L, 1);
	}
	return items;
}

/******************************************************************************/
void luaentity_get(lua_State *L, u16 id)
{
	// Get luaentities[i]
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "luaentities");
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_pushinteger(L, id);
	lua_gettable(L, -2);
	lua_remove(L, -2); // Remove luaentities
	lua_remove(L, -2); // Remove core
}

/******************************************************************************/
// Returns depth of json value tree
static int push_json_value_getdepth(const Json::Value &value)
{
	if (!value.isArray() && !value.isObject())
		return 1;

	int maxdepth = 0;
	for (const auto &it : value) {
		int elemdepth = push_json_value_getdepth(it);
		if (elemdepth > maxdepth)
			maxdepth = elemdepth;
	}
	return maxdepth + 1;
}
// Recursive function to convert JSON --> Lua table
static bool push_json_value_helper(lua_State *L, const Json::Value &value,
		int nullindex)
{
	switch(value.type()) {
		case Json::nullValue:
		default:
			lua_pushvalue(L, nullindex);
			break;
		case Json::intValue:
			lua_pushinteger(L, value.asLargestInt());
			break;
		case Json::uintValue:
			lua_pushinteger(L, value.asLargestUInt());
			break;
		case Json::realValue:
			lua_pushnumber(L, value.asDouble());
			break;
		case Json::stringValue:
			{
				const char *str = value.asCString();
				lua_pushstring(L, str ? str : "");
			}
			break;
		case Json::booleanValue:
			lua_pushboolean(L, value.asInt());
			break;
		case Json::arrayValue:
			lua_createtable(L, value.size(), 0);
			for (Json::Value::const_iterator it = value.begin();
					it != value.end(); ++it) {
				push_json_value_helper(L, *it, nullindex);
				lua_rawseti(L, -2, it.index() + 1);
			}
			break;
		case Json::objectValue:
			lua_createtable(L, 0, value.size());
			for (Json::Value::const_iterator it = value.begin();
					it != value.end(); ++it) {
#if !defined(JSONCPP_STRING) && (JSONCPP_VERSION_MAJOR < 1 || JSONCPP_VERSION_MINOR < 9)
				const char *str = it.memberName();
				lua_pushstring(L, str ? str : "");
#else
				std::string str = it.name();
				lua_pushstring(L, str.c_str());
#endif
				push_json_value_helper(L, *it, nullindex);
				lua_rawset(L, -3);
			}
			break;
	}
	return true;
}
// converts JSON --> Lua table; returns false if lua stack limit exceeded
// nullindex: Lua stack index of value to use in place of JSON null
bool push_json_value(lua_State *L, const Json::Value &value, int nullindex)
{
	if(nullindex < 0)
		nullindex = lua_gettop(L) + 1 + nullindex;

	int depth = push_json_value_getdepth(value);

	// The maximum number of Lua stack slots used at each recursion level
	// of push_json_value_helper is 2, so make sure there a depth * 2 slots
	if (lua_checkstack(L, depth * 2))
		return push_json_value_helper(L, value, nullindex);

	return false;
}

// Converts Lua table --> JSON
void read_json_value(lua_State *L, Json::Value &root, int index, u8 recursion)
{
	if (recursion > 16) {
		throw SerializationError("Maximum recursion depth exceeded");
	}
	int type = lua_type(L, index);
	if (type == LUA_TBOOLEAN) {
		root = (bool) lua_toboolean(L, index);
	} else if (type == LUA_TNUMBER) {
		root = lua_tonumber(L, index);
	} else if (type == LUA_TSTRING) {
		size_t len;
		const char *str = lua_tolstring(L, index, &len);
		root = std::string(str, len);
	} else if (type == LUA_TTABLE) {
		lua_pushnil(L);
		while (lua_next(L, index)) {
			// Key is at -2 and value is at -1
			Json::Value value;
			read_json_value(L, value, lua_gettop(L), recursion + 1);

			Json::ValueType roottype = root.type();
			int keytype = lua_type(L, -1);
			if (keytype == LUA_TNUMBER) {
				lua_Number key = lua_tonumber(L, -1);
				if (roottype != Json::nullValue && roottype != Json::arrayValue) {
					throw SerializationError("Can't mix array and object values in JSON");
				} else if (key < 1) {
					throw SerializationError("Can't use zero-based or negative indexes in JSON");
				} else if (floor(key) != key) {
					throw SerializationError("Can't use indexes with a fractional part in JSON");
				}
				root[(Json::ArrayIndex) key - 1] = value;
			} else if (keytype == LUA_TSTRING) {
				if (roottype != Json::nullValue && roottype != Json::objectValue) {
					throw SerializationError("Can't mix array and object values in JSON");
				}
				root[lua_tostring(L, -1)] = value;
			} else {
				throw SerializationError("Lua key to convert to JSON is not a string or number");
			}
		}
	} else if (type == LUA_TNIL) {
		root = Json::nullValue;
	} else {
		throw SerializationError("Can only store booleans, numbers, strings, objects, arrays, and null in JSON");
	}
	lua_pop(L, 1); // Pop value
}

void push_pointed_thing(lua_State *L, const PointedThing &pointed, bool csm,
	bool hitpoint)
{
	lua_newtable(L);
	if (pointed.type == POINTEDTHING_NODE) {
		lua_pushstring(L, "node");
		lua_setfield(L, -2, "type");
		push_v3s16(L, pointed.node_undersurface);
		lua_setfield(L, -2, "under");
		push_v3s16(L, pointed.node_abovesurface);
		lua_setfield(L, -2, "above");
	} else if (pointed.type == POINTEDTHING_OBJECT) {
		lua_pushstring(L, "object");
		lua_setfield(L, -2, "type");

		if (csm) {
			lua_pushinteger(L, pointed.object_id);
			lua_setfield(L, -2, "id");
		} else {
			push_objectRef(L, pointed.object_id);
			lua_setfield(L, -2, "ref");
		}
	} else {
		lua_pushstring(L, "nothing");
		lua_setfield(L, -2, "type");
	}
	if (hitpoint && (pointed.type != POINTEDTHING_NOTHING)) {
		push_v3f(L, pointed.intersection_point / BS); // convert to node coords
		lua_setfield(L, -2, "intersection_point");
		push_v3f(L, pointed.intersection_normal);
		lua_setfield(L, -2, "intersection_normal");
		lua_pushinteger(L, pointed.box_id + 1); // change to Lua array index
		lua_setfield(L, -2, "box_id");
	}
}

void push_objectRef(lua_State *L, const u16 id)
{
	// Get core.object_refs[i]
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "object_refs");
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_pushinteger(L, id);
	lua_gettable(L, -2);
	lua_remove(L, -2); // object_refs
	lua_remove(L, -2); // core
}

void read_hud_element(lua_State *L, HudElement *elem)
{
	elem->type = (HudElementType)getenumfield(L, 2, "hud_elem_type",
									es_HudElementType, HUD_ELEM_TEXT);

	lua_getfield(L, 2, "position");
	elem->pos = lua_istable(L, -1) ? read_v2f(L, -1) : v2f();
	lua_pop(L, 1);

	lua_getfield(L, 2, "scale");
	elem->scale = lua_istable(L, -1) ? read_v2f(L, -1) : v2f();
	lua_pop(L, 1);

	lua_getfield(L, 2, "size");
	elem->size = lua_istable(L, -1) ? read_v2s32(L, -1) : v2s32();
	lua_pop(L, 1);

	elem->name    = getstringfield_default(L, 2, "name", "");
	elem->text    = getstringfield_default(L, 2, "text", "");
	elem->number  = getintfield_default(L, 2, "number", 0);
	if (elem->type == HUD_ELEM_WAYPOINT)
		// waypoints reuse the item field to store precision, item = precision + 1
		elem->item = getintfield_default(L, 2, "precision", -1) + 1;
	else
		elem->item = getintfield_default(L, 2, "item", 0);
	elem->dir     = getintfield_default(L, 2, "direction", 0);
	elem->z_index = MYMAX(S16_MIN, MYMIN(S16_MAX,
			getintfield_default(L, 2, "z_index", 0)));
	elem->text2   = getstringfield_default(L, 2, "text2", "");

	// Deprecated, only for compatibility's sake
	if (elem->dir == 0)
		elem->dir = getintfield_default(L, 2, "dir", 0);

	lua_getfield(L, 2, "alignment");
	elem->align = lua_istable(L, -1) ? read_v2f(L, -1) : v2f();
	lua_pop(L, 1);

	lua_getfield(L, 2, "offset");
	elem->offset = lua_istable(L, -1) ? read_v2f(L, -1) : v2f();
	lua_pop(L, 1);

	lua_getfield(L, 2, "world_pos");
	elem->world_pos = lua_istable(L, -1) ? read_v3f(L, -1) : v3f();
	lua_pop(L, 1);

	elem->style = getintfield_default(L, 2, "style", 0);

	/* check for known deprecated element usage */
	if ((elem->type  == HUD_ELEM_STATBAR) && (elem->size == v2s32()))
		log_deprecated(L,"Deprecated usage of statbar without size!");
}

void push_hud_element(lua_State *L, HudElement *elem)
{
	lua_newtable(L);

	lua_pushstring(L, es_HudElementType[(u8)elem->type].str);
	lua_setfield(L, -2, "type");

	push_v2f(L, elem->pos);
	lua_setfield(L, -2, "position");

	lua_pushstring(L, elem->name.c_str());
	lua_setfield(L, -2, "name");

	push_v2f(L, elem->scale);
	lua_setfield(L, -2, "scale");

	lua_pushstring(L, elem->text.c_str());
	lua_setfield(L, -2, "text");

	lua_pushnumber(L, elem->number);
	lua_setfield(L, -2, "number");

	if (elem->type == HUD_ELEM_WAYPOINT) {
		// waypoints reuse the item field to store precision, precision = item - 1
		lua_pushnumber(L, elem->item - 1);
		lua_setfield(L, -2, "precision");
	}
	// push the item field for waypoints as well for backwards compatibility
	lua_pushnumber(L, elem->item);
	lua_setfield(L, -2, "item");

	lua_pushnumber(L, elem->dir);
	lua_setfield(L, -2, "direction");

	push_v2f(L, elem->offset);
	lua_setfield(L, -2, "offset");

	push_v2f(L, elem->align);
	lua_setfield(L, -2, "alignment");

	push_v2s32(L, elem->size);
	lua_setfield(L, -2, "size");

	// Deprecated, only for compatibility's sake
	lua_pushnumber(L, elem->dir);
	lua_setfield(L, -2, "dir");

	push_v3f(L, elem->world_pos);
	lua_setfield(L, -2, "world_pos");

	lua_pushnumber(L, elem->z_index);
	lua_setfield(L, -2, "z_index");

	lua_pushstring(L, elem->text2.c_str());
	lua_setfield(L, -2, "text2");

	lua_pushinteger(L, elem->style);
	lua_setfield(L, -2, "style");
}

bool read_hud_change(lua_State *L, HudElementStat &stat, HudElement *elem, void **value)
{
	std::string statstr = lua_tostring(L, 3);
	{
		int statint;
		if (!string_to_enum(es_HudElementStat, statint, statstr)) {
			script_log_unique(L, "Unknown HUD stat type: " + statstr, warningstream);
			return false;
		}

		stat = (HudElementStat)statint;
	}

	switch (stat) {
		case HUD_STAT_POS:
			elem->pos = read_v2f(L, 4);
			*value = &elem->pos;
			break;
		case HUD_STAT_NAME:
			elem->name = luaL_checkstring(L, 4);
			*value = &elem->name;
			break;
		case HUD_STAT_SCALE:
			elem->scale = read_v2f(L, 4);
			*value = &elem->scale;
			break;
		case HUD_STAT_TEXT:
			elem->text = luaL_checkstring(L, 4);
			*value = &elem->text;
			break;
		case HUD_STAT_NUMBER:
			elem->number = luaL_checknumber(L, 4);
			*value = &elem->number;
			break;
		case HUD_STAT_ITEM:
			elem->item = luaL_checknumber(L, 4);
			if (elem->type == HUD_ELEM_WAYPOINT && statstr == "precision")
				elem->item++;
			*value = &elem->item;
			break;
		case HUD_STAT_DIR:
			elem->dir = luaL_checknumber(L, 4);
			*value = &elem->dir;
			break;
		case HUD_STAT_ALIGN:
			elem->align = read_v2f(L, 4);
			*value = &elem->align;
			break;
		case HUD_STAT_OFFSET:
			elem->offset = read_v2f(L, 4);
			*value = &elem->offset;
			break;
		case HUD_STAT_WORLD_POS:
			elem->world_pos = read_v3f(L, 4);
			*value = &elem->world_pos;
			break;
		case HUD_STAT_SIZE:
			elem->size = read_v2s32(L, 4);
			*value = &elem->size;
			break;
		case HUD_STAT_Z_INDEX:
			elem->z_index = MYMAX(S16_MIN, MYMIN(S16_MAX, luaL_checknumber(L, 4)));
			*value = &elem->z_index;
			break;
		case HUD_STAT_TEXT2:
			elem->text2 = luaL_checkstring(L, 4);
			*value = &elem->text2;
			break;
		case HUD_STAT_STYLE:
			elem->style = luaL_checknumber(L, 4);
			*value = &elem->style;
			break;
	}

	return true;
}

/******************************************************************************/

// Indices must match values in `enum CollisionType` exactly!!
static const char *collision_type_str[] = {
	"node",
	"object",
};

// Indices must match values in `enum CollisionAxis` exactly!!
static const char *collision_axis_str[] = {
	"x",
	"y",
	"z",
};

void push_collision_move_result(lua_State *L, const collisionMoveResult &res)
{
	lua_createtable(L, 0, 4);

	setboolfield(L, -1, "touching_ground", res.touching_ground);
	setboolfield(L, -1, "collides", res.collides);
	setboolfield(L, -1, "standing_on_object", res.standing_on_object);

	/* collisions */
	lua_createtable(L, res.collisions.size(), 0);
	int i = 1;
	for (const auto &c : res.collisions) {
		lua_createtable(L, 0, 5);

		lua_pushstring(L, collision_type_str[c.type]);
		lua_setfield(L, -2, "type");

		assert(c.axis != COLLISION_AXIS_NONE);
		lua_pushstring(L, collision_axis_str[c.axis]);
		lua_setfield(L, -2, "axis");

		if (c.type == COLLISION_NODE) {
			push_v3s16(L, c.node_p);
			lua_setfield(L, -2, "node_pos");
		} else if (c.type == COLLISION_OBJECT) {
			push_objectRef(L, c.object->getId());
			lua_setfield(L, -2, "object");
		}

		push_v3f(L, c.old_speed / BS);
		lua_setfield(L, -2, "old_velocity");

		push_v3f(L, c.new_speed / BS);
		lua_setfield(L, -2, "new_velocity");

		lua_rawseti(L, -2, i++);
	}
	lua_setfield(L, -2, "collisions");
	/**/
}

