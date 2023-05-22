/*
Minetest
Copyright (C) 2018 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

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

#include "mods.h"
#include "filesys.h"
#include "log.h"
#include "scripting_server.h"
#include "content/subgames.h"
#include "porting.h"

/**
 * Manage server mods
 *
 * All new calls to this class must be tested in test_servermodmanager.cpp
 */

/**
 * Creates a ServerModManager which targets worldpath
 * @param worldpath
 */
ServerModManager::ServerModManager(const std::string &worldpath):
	configuration()
{
	SubgameSpec gamespec = findWorldSubgame(worldpath);

	// Add all game mods and all world mods
	configuration.addGameMods(gamespec);
	configuration.addModsInPath(worldpath + DIR_DELIM + "worldmods", "worldmods");

	// Load normal mods
	std::string worldmt = worldpath + DIR_DELIM + "world.mt";
	configuration.addModsFromConfig(worldmt, gamespec.addon_mods_paths);
	configuration.checkConflictsAndDeps();
}

// clang-format on
const ModSpec *ServerModManager::getModSpec(const std::string &modname) const
{
	for (const auto &mod : configuration.getMods()) {
		if (mod.name == modname)
			return &mod;
	}

	return nullptr;
}
