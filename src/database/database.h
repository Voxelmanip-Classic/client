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

#pragma once

#include <set>
#include <string>
#include <vector>
#include "irr_v3d.h"
#include "irrlichttypes.h"
#include "util/basic_macros.h"
#include "util/string.h"

class Database
{
public:
	virtual void beginSave() = 0;
	virtual void endSave() = 0;
	virtual bool initialized() const { return true; }
};

class MapDatabase : public Database
{
public:
	virtual ~MapDatabase() = default;

	virtual bool saveBlock(const v3s16 &pos, const std::string &data) = 0;
	virtual void loadBlock(const v3s16 &pos, std::string *block) = 0;
	virtual bool deleteBlock(const v3s16 &pos) = 0;
};


class ModStorageDatabase : public Database
{
public:
	virtual ~ModStorageDatabase() = default;

	virtual void getModEntries(const std::string &modname, StringMap *storage) = 0;
	virtual void getModKeys(const std::string &modname, std::vector<std::string> *storage) = 0;
	virtual bool hasModEntry(const std::string &modname, const std::string &key) = 0;
	virtual bool getModEntry(const std::string &modname,
		const std::string &key, std::string *value) = 0;
	virtual bool setModEntry(const std::string &modname,
		const std::string &key, const std::string &value) = 0;
	virtual bool removeModEntry(const std::string &modname, const std::string &key) = 0;
	virtual bool removeModEntries(const std::string &modname) = 0;
	virtual void listMods(std::vector<std::string> *res) = 0;
};
