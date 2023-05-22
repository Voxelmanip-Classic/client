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

#pragma once

#include "unit_sao.h"

class LuaEntitySAO : public UnitSAO
{
public:
	LuaEntitySAO() = delete;
	// Used by the environment to load SAO
	LuaEntitySAO(ServerEnvironment *env, v3f pos, const std::string &data);
	// Used by the Lua API
	LuaEntitySAO(ServerEnvironment *env, v3f pos, const std::string &name,
			const std::string &state) :
			UnitSAO(env, pos),
			m_init_name(name), m_init_state(state)
	{
	}
	~LuaEntitySAO();

	ActiveObjectType getType() const { return ACTIVEOBJECT_TYPE_LUAENTITY; }
	void step(float dtime, bool send_recommended);

	bool isStaticAllowed() const { return m_prop.static_save; }
	void getStaticData(std::string *result) const;

	std::string getName();
	bool collideWithObjects() const;

private:
	std::string m_init_name;
	std::string m_init_state;
	bool m_registered = false;

	v3f m_velocity;
	v3f m_acceleration;
};
