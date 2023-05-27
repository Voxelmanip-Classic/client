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

/*
Dummy database class
*/

#include "database-dummy.h"

void Database_Dummy::getModEntries(const std::string &modname, StringMap *storage)
{
	const auto mod_pair = m_mod_storage_database.find(modname);
	if (mod_pair != m_mod_storage_database.cend()) {
		for (const auto &pair : mod_pair->second) {
			(*storage)[pair.first] = pair.second;
		}
	}
}

void Database_Dummy::getModKeys(const std::string &modname, std::vector<std::string> *storage)
{
	const auto mod_pair = m_mod_storage_database.find(modname);
	if (mod_pair != m_mod_storage_database.cend()) {
		storage->reserve(storage->size() + mod_pair->second.size());
		for (const auto &pair : mod_pair->second)
			storage->push_back(pair.first);
	}
}

bool Database_Dummy::getModEntry(const std::string &modname,
	const std::string &key, std::string *value)
{
	auto mod_pair = m_mod_storage_database.find(modname);
	if (mod_pair == m_mod_storage_database.end())
		return false;
	const StringMap &meta = mod_pair->second;

	auto pair = meta.find(key);
	if (pair != meta.end()) {
		*value = pair->second;
		return true;
	}
	return false;
}

bool Database_Dummy::hasModEntry(const std::string &modname, const std::string &key)
{
	auto mod_pair = m_mod_storage_database.find(modname);
	if (mod_pair == m_mod_storage_database.end())
		return false;
	const StringMap &meta = mod_pair->second;

	return meta.find(key) != meta.cend();
}

bool Database_Dummy::setModEntry(const std::string &modname,
	const std::string &key, const std::string &value)
{
	auto mod_pair = m_mod_storage_database.find(modname);
	if (mod_pair == m_mod_storage_database.end()) {
		m_mod_storage_database[modname] = StringMap({{key, value}});
	} else {
		mod_pair->second[key] = value;
	}
	return true;
}

bool Database_Dummy::removeModEntry(const std::string &modname, const std::string &key)
{
	auto mod_pair = m_mod_storage_database.find(modname);
	if (mod_pair != m_mod_storage_database.end())
		return mod_pair->second.erase(key) > 0;
	return false;
}

bool Database_Dummy::removeModEntries(const std::string &modname)
{
	auto mod_pair = m_mod_storage_database.find(modname);
	if (mod_pair != m_mod_storage_database.end() && !mod_pair->second.empty()) {
		mod_pair->second.clear();
		return true;
	}
	return false;
}

void Database_Dummy::listMods(std::vector<std::string> *res)
{
	for (const auto &pair : m_mod_storage_database) {
		res->push_back(pair.first);
	}
}
