/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2015 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

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

#include "serveropcodes.h"

const static ToServerCommandHandler null_command_handler = { "TOSERVER_NULL", TOSERVER_STATE_ALL, &Server::handleCommand_Null };

const static ClientCommandFactory null_command_factory = { "TOCLIENT_NULL", 0, false };

/*
	Channels used for Server -> Client communication
	2: Bulk data (mapblocks, media, ...)
	1: HUD packets
	0: everything else

	Packet order is only guaranteed inside a channel, so packets that operate on
	the same objects are *required* to be in the same channel.
*/

const ClientCommandFactory clientCommandFactoryTable[TOCLIENT_NUM_MSG_TYPES] =
{
	null_command_factory, // 0x00
	null_command_factory, // 0x01
	{ "TOCLIENT_HELLO",                    0, true }, // 0x02
	{ "TOCLIENT_AUTH_ACCEPT",              0, true }, // 0x03
	{ "TOCLIENT_ACCEPT_SUDO_MODE",         0, true }, // 0x04
	{ "TOCLIENT_DENY_SUDO_MODE",           0, true }, // 0x05
	null_command_factory, // 0x06
	null_command_factory, // 0x07
	null_command_factory, // 0x08
	null_command_factory, // 0x09
	{ "TOCLIENT_ACCESS_DENIED",            0, true }, // 0x0A
	null_command_factory, // 0x0B
	null_command_factory, // 0x0C
	null_command_factory, // 0x0D
	null_command_factory, // 0x0E
	null_command_factory, // 0x0F
	{ "TOCLIENT_INIT",                     0, true }, // 0x10
	null_command_factory, // 0x11
	null_command_factory, // 0x12
	null_command_factory, // 0x13
	null_command_factory, // 0x14
	null_command_factory, // 0x15
	null_command_factory, // 0x16
	null_command_factory, // 0x17
	null_command_factory, // 0x18
	null_command_factory, // 0x19
	null_command_factory, // 0x1A
	null_command_factory, // 0x1B
	null_command_factory, // 0x1C
	null_command_factory, // 0x1D
	null_command_factory, // 0x1E
	null_command_factory, // 0x1F
	{ "TOCLIENT_BLOCKDATA",                2, true }, // 0x20
	{ "TOCLIENT_ADDNODE",                  0, true }, // 0x21
	{ "TOCLIENT_REMOVENODE",               0, true }, // 0x22
	null_command_factory, // 0x23
	null_command_factory, // 0x24
	null_command_factory, // 0x25
	null_command_factory, // 0x26
	{ "TOCLIENT_INVENTORY",                0, true }, // 0x27
	null_command_factory, // 0x28
	{ "TOCLIENT_TIME_OF_DAY",              0, true }, // 0x29
	{ "TOCLIENT_CSM_RESTRICTION_FLAGS",    0, true }, // 0x2A
	{ "TOCLIENT_PLAYER_SPEED",             0, true }, // 0x2B
	{ "TOCLIENT_MEDIA_PUSH",               0, true }, // 0x2C (sent over channel 1 too if legacy)
	null_command_factory, // 0x2D
	null_command_factory, // 0x2E
	{ "TOCLIENT_CHAT_MESSAGE",             0, true }, // 0x2F
	null_command_factory, // 0x30
	{ "TOCLIENT_ACTIVE_OBJECT_REMOVE_ADD", 0, true }, // 0x31
	{ "TOCLIENT_ACTIVE_OBJECT_MESSAGES",   0, true }, // 0x32 (may be sent as unrel over channel 1 too)
	{ "TOCLIENT_HP",                       0, true }, // 0x33
	{ "TOCLIENT_MOVE_PLAYER",              0, true }, // 0x34
	null_command_factory, // 0x35
	{ "TOCLIENT_FOV",                      0, true }, // 0x36
	{ "TOCLIENT_DEATHSCREEN",              0, true }, // 0x37
	{ "TOCLIENT_MEDIA",                    2, true }, // 0x38
	null_command_factory, // 0x39
	{ "TOCLIENT_NODEDEF",                  0, true }, // 0x3A
	null_command_factory, // 0x3B
	{ "TOCLIENT_ANNOUNCE_MEDIA",           0, true }, // 0x3C
	{ "TOCLIENT_ITEMDEF",                  0, true }, // 0x3D
	null_command_factory, // 0x3E
	{ "TOCLIENT_PLAY_SOUND",               0, true }, // 0x3f (may be sent as unrel too)
	{ "TOCLIENT_STOP_SOUND",               0, true }, // 0x40
	{ "TOCLIENT_PRIVILEGES",               0, true }, // 0x41
	{ "TOCLIENT_INVENTORY_FORMSPEC",       0, true }, // 0x42
	{ "TOCLIENT_DETACHED_INVENTORY",       0, true }, // 0x43
	{ "TOCLIENT_SHOW_FORMSPEC",            0, true }, // 0x44
	{ "TOCLIENT_MOVEMENT",                 0, true }, // 0x45
	{ "TOCLIENT_SPAWN_PARTICLE",           0, true }, // 0x46
	{ "TOCLIENT_ADD_PARTICLESPAWNER",      0, true }, // 0x47
	null_command_factory, // 0x48
	{ "TOCLIENT_HUDADD",                   1, true }, // 0x49
	{ "TOCLIENT_HUDRM",                    1, true }, // 0x4a
	{ "TOCLIENT_HUDCHANGE",                1, true }, // 0x4b
	{ "TOCLIENT_HUD_SET_FLAGS",            1, true }, // 0x4c
	{ "TOCLIENT_HUD_SET_PARAM",            1, true }, // 0x4d
	{ "TOCLIENT_BREATH",                   0, true }, // 0x4e
	{ "TOCLIENT_SET_SKY",                  0, true }, // 0x4f
	{ "TOCLIENT_OVERRIDE_DAY_NIGHT_RATIO", 0, true }, // 0x50
	{ "TOCLIENT_LOCAL_PLAYER_ANIMATIONS",  0, true }, // 0x51
	{ "TOCLIENT_EYE_OFFSET",               0, true }, // 0x52
	{ "TOCLIENT_DELETE_PARTICLESPAWNER",   0, true }, // 0x53
	{ "TOCLIENT_CLOUD_PARAMS",             0, true }, // 0x54
	{ "TOCLIENT_FADE_SOUND",               0, true }, // 0x55
	{ "TOCLIENT_UPDATE_PLAYER_LIST",       0, true }, // 0x56
	{ "TOCLIENT_MODCHANNEL_MSG",           0, true }, // 0x57
	{ "TOCLIENT_MODCHANNEL_SIGNAL",        0, true }, // 0x58
	{ "TOCLIENT_NODEMETA_CHANGED",         0, true }, // 0x59
	{ "TOCLIENT_SET_SUN",                  0, true }, // 0x5a
	{ "TOCLIENT_SET_MOON",                 0, true }, // 0x5b
	{ "TOCLIENT_SET_STARS",                0, true }, // 0x5c
	null_command_factory, // 0x5d
	null_command_factory, // 0x5e
	null_command_factory, // 0x5f
	{ "TOCLIENT_SRP_BYTES_S_B",            0, true }, // 0x60
	{ "TOCLIENT_FORMSPEC_PREPEND",         0, true }, // 0x61
	{ "TOCLIENT_MINIMAP_MODES",            0, true }, // 0x62
	{ "TOCLIENT_SET_LIGHTING",             0, true }, // 0x63
};
