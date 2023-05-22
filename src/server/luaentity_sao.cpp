/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2013-2020 Minetest core developers & community

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

#include "luaentity_sao.h"
#include "collision.h"
#include "constants.h"
#include "inventory.h"
#include "player_sao.h"
#include "scripting_server.h"
#include "server.h"
#include "serverenvironment.h"

LuaEntitySAO::LuaEntitySAO(ServerEnvironment *env, v3f pos, const std::string &data)
	: UnitSAO(env, pos)
{

}

LuaEntitySAO::~LuaEntitySAO()
{

}

void LuaEntitySAO::step(float dtime, bool send_recommended)
{

}

void LuaEntitySAO::getStaticData(std::string *result) const
{

}

std::string LuaEntitySAO::getName()
{
	return m_init_name;
}

bool LuaEntitySAO::collideWithObjects() const
{
	return m_prop.collideWithObjects;
}
