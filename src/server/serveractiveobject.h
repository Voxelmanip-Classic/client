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

#pragma once

#include <cassert>
#include <unordered_set>
#include "irrlichttypes_bloated.h"
#include "activeobject.h"
#include "itemgroup.h"
#include "util/container.h"

/*

Some planning
-------------

* Server environment adds an active object, which gets the id 1
* The active object list is scanned for each client once in a while,
  and it finds out what objects have been added that are not known
  by the client yet. This scan is initiated by the Server class and
  the result ends up directly to the server.
* A network packet is created with the info and sent to the client.
* Environment converts objects to static data and static data to
  objects, based on how close players are to them.

*/

class ServerEnvironment;
struct ItemStack;
struct ToolCapabilities;
struct ObjectProperties;
struct PlayerHPChangeReason;
class Inventory;
struct InventoryLocation;

class ServerActiveObject : public ActiveObject
{
public:
	/*
		NOTE: m_env can be NULL, but step() isn't called if it is.
		Prototypes are used that way.
	*/
	ServerActiveObject(ServerEnvironment *env, v3f pos);
	virtual ~ServerActiveObject() = default;

	// Returns true if object's deletion is the job of the
	// environment
	virtual bool environmentDeletes() const
	{ return true; }

	// Create a certain type of ServerActiveObject
	static ServerActiveObject* create(ActiveObjectType type,
			ServerEnvironment *env, u16 id, v3f pos,
			const std::string &data);

	/*
		Some simple getters/setters
	*/
	v3f getBasePosition() const { return m_base_position; }
	ServerEnvironment* getEnv(){ return m_env; }

	/*
		Step object in time.
		Messages added to messages are sent to client over network.

		send_recommended:
			True at around 5-10 times a second, same for all objects.
			This is used to let objects send most of the data at the
			same time so that the data can be combined in a single
			packet.
	*/
	virtual void step(float dtime, bool send_recommended){}

	/*
		The return value of this is passed to the server-side object
		when it is created (converted from static to active - actually
		the data is the static form)
	*/
	virtual void getStaticData(std::string *result) const
	{
		assert(isStaticAllowed());
		*result = "";
	}

	/*
		Return false in here to never save and instead remove object
		on unload. getStaticData() will not be called in that case.
	*/
	virtual bool isStaticAllowed() const
	{return true;}

	virtual ServerActiveObject *getParent() const { return nullptr; }

	// Inventory and wielded item
	virtual Inventory *getInventory() const
	{ return NULL; }
	virtual void setInventoryModified()
	{}
	virtual u16 getWieldIndex() const
	{ return 0; }

	/*
		A getter that unifies the above to answer the question:
		"Can the environment still interact with this object?"
	*/
	inline bool isGone() const
	{ return m_pending_removal || m_pending_deactivation; }

protected:

	ServerEnvironment *m_env;
	v3f m_base_position;

	/*
		Same purpose as m_pending_removal but for deactivation.
		deactvation = save static data in block, remove active object

		If this is set alongside with m_pending_removal, removal takes
		priority.
		Note: Do not assign this directly, use markForDeactivation() instead.
	*/
	bool m_pending_deactivation = false;

	/*
		- Whether this object is to be removed when nobody knows about
		  it anymore.
		- Removal is delayed to preserve the id for the time during which
		  it could be confused to some other object by some client.
		- This is usually set to true by the step() method when the object wants
		  to be deleted but can be set by anything else too.
		Note: Do not assign this directly, use markForRemoval() instead.
	*/
	bool m_pending_removal = false;
};
