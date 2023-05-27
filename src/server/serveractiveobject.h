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
class Inventory;
struct InventoryLocation;

class ServerActiveObject : public ActiveObject
{
public:

	ServerActiveObject();
	virtual ~ServerActiveObject() = default;

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

	/*
		A getter that unifies the above to answer the question:
		"Can the environment still interact with this object?"
	*/
	inline bool isGone() const
	{ return m_pending_removal || m_pending_deactivation; }

protected:

	bool m_pending_deactivation = false;

	bool m_pending_removal = false;
};
