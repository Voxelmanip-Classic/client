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

#include "inventorymanager.h"
#include "debug.h"
#include "log.h"
#include "serverenvironment.h"
#include "scripting_server.h"
#include "server/serveractiveobject.h"
#include "settings.h"
#include "util/strfnd.h"
#include "util/basic_macros.h"
#include "inventory.h"

#define PLAYER_TO_SA(p)   p->getEnv()->getScriptIface()

/*
	InventoryLocation
*/

std::string InventoryLocation::dump() const
{
	std::ostringstream os(std::ios::binary);
	serialize(os);
	return os.str();
}

void InventoryLocation::serialize(std::ostream &os) const
{
	switch (type) {
	case InventoryLocation::UNDEFINED:
		os<<"undefined";
		break;
	case InventoryLocation::CURRENT_PLAYER:
		os<<"current_player";
		break;
	case InventoryLocation::PLAYER:
		os<<"player:"<<name;
		break;
	case InventoryLocation::NODEMETA:
		os<<"nodemeta:"<<p.X<<","<<p.Y<<","<<p.Z;
		break;
	case InventoryLocation::DETACHED:
		os<<"detached:"<<name;
		break;
	default:
		FATAL_ERROR("Unhandled inventory location type");
	}
}

void InventoryLocation::deSerialize(std::istream &is)
{
	std::string tname;
	std::getline(is, tname, ':');
	if (tname == "undefined") {
		type = InventoryLocation::UNDEFINED;
	} else if (tname == "current_player") {
		type = InventoryLocation::CURRENT_PLAYER;
	} else if (tname == "player") {
		type = InventoryLocation::PLAYER;
		std::getline(is, name, '\n');
	} else if (tname == "nodemeta") {
		type = InventoryLocation::NODEMETA;
		std::string pos;
		std::getline(is, pos, '\n');
		Strfnd fn(pos);
		p.X = stoi(fn.next(","));
		p.Y = stoi(fn.next(","));
		p.Z = stoi(fn.next(","));
	} else if (tname == "detached") {
		type = InventoryLocation::DETACHED;
		std::getline(is, name, '\n');
	} else {
		infostream<<"Unknown InventoryLocation type=\""<<tname<<"\""<<std::endl;
		throw SerializationError("Unknown InventoryLocation type");
	}
}

void InventoryLocation::deSerialize(const std::string &s)
{
	std::istringstream is(s, std::ios::binary);
	deSerialize(is);
}

/*
	InventoryAction
*/

InventoryAction *InventoryAction::deSerialize(std::istream &is)
{
	std::string type;
	std::getline(is, type, ' ');

	InventoryAction *a = nullptr;

	if (type == "Move") {
		a = new IMoveAction(is, false);
	} else if (type == "MoveSomewhere") {
		a = new IMoveAction(is, true);
	} else if (type == "Drop") {
		a = new IDropAction(is);
	}

	return a;
}

/*
	IMoveAction
*/

IMoveAction::IMoveAction(std::istream &is, bool somewhere) :
		move_somewhere(somewhere)
{
	std::string ts;

	std::getline(is, ts, ' ');
	count = stoi(ts);

	std::getline(is, ts, ' ');
	from_inv.deSerialize(ts);

	std::getline(is, from_list, ' ');

	std::getline(is, ts, ' ');
	from_i = stoi(ts);

	std::getline(is, ts, ' ');
	to_inv.deSerialize(ts);

	std::getline(is, to_list, ' ');

	if (!somewhere) {
		std::getline(is, ts, ' ');
		to_i = stoi(ts);
	}
}

void IMoveAction::swapDirections()
{
	std::swap(from_inv, to_inv);
	std::swap(from_list, to_list);
	std::swap(from_i, to_i);
}

void IMoveAction::onPutAndOnTake(const ItemStack &src_item, ServerActiveObject *player) const
{
	ServerScripting *sa = PLAYER_TO_SA(player);
	if (to_inv.type == InventoryLocation::DETACHED)
		sa->detached_inventory_OnPut(*this, src_item, player);
	else if (to_inv.type == InventoryLocation::NODEMETA)
		sa->nodemeta_inventory_OnPut(*this, src_item, player);
	else if (to_inv.type == InventoryLocation::PLAYER)
		sa->player_inventory_OnPut(*this, src_item, player);
	else
		assert(false);

	if (from_inv.type == InventoryLocation::DETACHED)
		sa->detached_inventory_OnTake(*this, src_item, player);
	else if (from_inv.type == InventoryLocation::NODEMETA)
		sa->nodemeta_inventory_OnTake(*this, src_item, player);
	else if (from_inv.type == InventoryLocation::PLAYER)
		sa->player_inventory_OnTake(*this, src_item, player);
	else
		assert(false);
}

void IMoveAction::onMove(int count, ServerActiveObject *player) const
{
	ServerScripting *sa = PLAYER_TO_SA(player);
	if (from_inv.type == InventoryLocation::DETACHED)
		sa->detached_inventory_OnMove(*this, count, player);
	else if (from_inv.type == InventoryLocation::NODEMETA)
		sa->nodemeta_inventory_OnMove(*this, count, player);
	else if (from_inv.type == InventoryLocation::PLAYER)
		sa->player_inventory_OnMove(*this, count, player);
	else
		assert(false);
}

int IMoveAction::allowPut(const ItemStack &dst_item, ServerActiveObject *player) const
{
	ServerScripting *sa = PLAYER_TO_SA(player);
	int dst_can_put_count = 0xffff;
	if (to_inv.type == InventoryLocation::DETACHED)
		dst_can_put_count = sa->detached_inventory_AllowPut(*this, dst_item, player);
	else if (to_inv.type == InventoryLocation::NODEMETA)
		dst_can_put_count = sa->nodemeta_inventory_AllowPut(*this, dst_item, player);
	else if (to_inv.type == InventoryLocation::PLAYER)
		dst_can_put_count = sa->player_inventory_AllowPut(*this, dst_item, player);
	else
		assert(false);
	return dst_can_put_count;
}

int IMoveAction::allowTake(const ItemStack &src_item, ServerActiveObject *player) const
{
	ServerScripting *sa = PLAYER_TO_SA(player);
	int src_can_take_count = 0xffff;
	if (from_inv.type == InventoryLocation::DETACHED)
		src_can_take_count = sa->detached_inventory_AllowTake(*this, src_item, player);
	else if (from_inv.type == InventoryLocation::NODEMETA)
		src_can_take_count = sa->nodemeta_inventory_AllowTake(*this, src_item, player);
	else if (from_inv.type == InventoryLocation::PLAYER)
		src_can_take_count = sa->player_inventory_AllowTake(*this, src_item, player);
	else
		assert(false);
	return src_can_take_count;
}

int IMoveAction::allowMove(int try_take_count, ServerActiveObject *player) const
{
	ServerScripting *sa = PLAYER_TO_SA(player);
	int src_can_take_count = 0xffff;
	if (from_inv.type == InventoryLocation::DETACHED)
		src_can_take_count = sa->detached_inventory_AllowMove(*this, try_take_count, player);
	else if (from_inv.type == InventoryLocation::NODEMETA)
		src_can_take_count = sa->nodemeta_inventory_AllowMove(*this, try_take_count, player);
	else if (from_inv.type == InventoryLocation::PLAYER)
		src_can_take_count = sa->player_inventory_AllowMove(*this, try_take_count, player);
	else
		assert(false);
	return src_can_take_count;
}

void IMoveAction::clientApply(InventoryManager *mgr, IGameDef *gamedef)
{
	// Optional InventoryAction operation that is run on the client
	// to make lag less apparent.

	Inventory *inv_from = mgr->getInventory(from_inv);
	Inventory *inv_to = mgr->getInventory(to_inv);
	if (!inv_from || !inv_to)
		return;

	InventoryLocation current_player;
	current_player.setCurrentPlayer();
	Inventory *inv_player = mgr->getInventory(current_player);
	if (inv_from != inv_player || inv_to != inv_player)
		return;

	InventoryList *list_from = inv_from->getList(from_list);
	InventoryList *list_to = inv_to->getList(to_list);
	if (!list_from || !list_to)
		return;

	if (!move_somewhere)
		list_from->moveItem(from_i, list_to, to_i, count);
	else
		list_from->moveItemSomewhere(from_i, list_to, count);

	mgr->setInventoryModified(from_inv);
	if (inv_from != inv_to)
		mgr->setInventoryModified(to_inv);
}

/*
	IDropAction
*/

IDropAction::IDropAction(std::istream &is)
{
	std::string ts;

	std::getline(is, ts, ' ');
	count = stoi(ts);

	std::getline(is, ts, ' ');
	from_inv.deSerialize(ts);

	std::getline(is, from_list, ' ');

	std::getline(is, ts, ' ');
	from_i = stoi(ts);
}

void IDropAction::clientApply(InventoryManager *mgr, IGameDef *gamedef)
{
	// Optional InventoryAction operation that is run on the client
	// to make lag less apparent.

	Inventory *inv_from = mgr->getInventory(from_inv);
	if (!inv_from)
		return;

	InventoryLocation current_player;
	current_player.setCurrentPlayer();
	Inventory *inv_player = mgr->getInventory(current_player);
	if (inv_from != inv_player)
		return;

	InventoryList *list_from = inv_from->getList(from_list);
	if (!list_from)
		return;

	if (count == 0)
		list_from->changeItem(from_i, ItemStack());
	else
		list_from->takeItem(from_i, count);

	mgr->setInventoryModified(from_inv);
}
