/*
Minetest
Copyright (C) 2010-2018 nerzhul, Loic BLOT <loic.blot@unix-experience.fr>

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

#include <log.h>
#include "mapblock.h"
#include "profiler.h"
#include "activeobjectmgr.h"

namespace server
{

void ActiveObjectMgr::clear(const std::function<bool(ServerActiveObject *, u16)> &cb)
{
	// make a defensive copy in case the
	// passed callback changes the set of active objects
	auto cloned_map(m_active_objects);

	for (auto &it : cloned_map) {
		if (cb(it.second, it.first)) {
			// Remove reference from m_active_objects
			m_active_objects.erase(it.first);
		}
	}
}

void ActiveObjectMgr::step(
		float dtime, const std::function<void(ServerActiveObject *)> &f)
{
	g_profiler->avg("ActiveObjectMgr: SAO count [#]", m_active_objects.size());
	for (auto &ao_it : m_active_objects) {
		f(ao_it.second);
	}
}

// clang-format off
bool ActiveObjectMgr::registerObject(ServerActiveObject *obj)
{
	assert(obj); // Pre-condition
	if (obj->getId() == 0) {
		u16 new_id = getFreeId();
		if (new_id == 0) {
			errorstream << "Server::ActiveObjectMgr::addActiveObjectRaw(): "
					<< "no free id available" << std::endl;
			if (obj->environmentDeletes())
				delete obj;
			return false;
		}
		obj->setId(new_id);
	} else {
		verbosestream << "Server::ActiveObjectMgr::addActiveObjectRaw(): "
				<< "supplied with id " << obj->getId() << std::endl;
	}

	if (!isFreeId(obj->getId())) {
		errorstream << "Server::ActiveObjectMgr::addActiveObjectRaw(): "
				<< "id is not free (" << obj->getId() << ")" << std::endl;
		if (obj->environmentDeletes())
			delete obj;
		return false;
	}

	if (objectpos_over_limit(obj->getBasePosition())) {
		v3f p = obj->getBasePosition();
		warningstream << "Server::ActiveObjectMgr::addActiveObjectRaw(): "
				<< "object position (" << p.X << "," << p.Y << "," << p.Z
				<< ") outside maximum range" << std::endl;
		if (obj->environmentDeletes())
			delete obj;
		return false;
	}

	m_active_objects[obj->getId()] = obj;

	verbosestream << "Server::ActiveObjectMgr::addActiveObjectRaw(): "
			<< "Added id=" << obj->getId() << "; there are now "
			<< m_active_objects.size() << " active objects." << std::endl;
	return true;
}

void ActiveObjectMgr::removeObject(u16 id)
{
	verbosestream << "Server::ActiveObjectMgr::removeObject(): "
			<< "id=" << id << std::endl;
	ServerActiveObject *obj = getActiveObject(id);
	if (!obj) {
		infostream << "Server::ActiveObjectMgr::removeObject(): "
				<< "id=" << id << " not found" << std::endl;
		return;
	}

	m_active_objects.erase(id);
	delete obj;
}

} // namespace server
