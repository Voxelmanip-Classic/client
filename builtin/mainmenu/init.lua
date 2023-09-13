--Minetest
--Copyright (C) 2014 sapier
--
--This program is free software; you can redistribute it and/or modify
--it under the terms of the GNU Lesser General Public License as published by
--the Free Software Foundation; either version 2.1 of the License, or
--(at your option) any later version.
--
--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU Lesser General Public License for more details.
--
--You should have received a copy of the GNU Lesser General Public License along
--with this program; if not, write to the Free Software Foundation, Inc.,
--51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

local menupath = core.get_mainmenu_path()
local basepath = core.get_builtin_path()
defaulttexturedir = core.get_texturepath_share() .. "/base/pack/"

function core.sound_stop(handle, ...)
	return handle:stop(...)
end

dofile(basepath .. "fstk/dialog.lua")
dofile(basepath .. "fstk/tabview.lua")
dofile(basepath .. "fstk/ui.lua")
dofile(menupath .. "/async_event.lua")
menudata = {}
dofile(menupath .. "/pkgmgr.lua")

dofile(menupath .. "/dlg_about.lua")
dofile(menupath .. "/dlg_register.lua")

local tabs = {}

dofile(menupath .. "/settings/init.lua")
tabs.connect = dofile(menupath .. "/tab_connect.lua")

function header_show()
	core.set_background('header', defaulttexturedir .. "menu_header.png")
end

function header_hide()
	core.set_background('header', "")
end

local function main_event_handler(tabview, event)
	if event == "MenuQuit" then
		core.close()
	end
	return true
end

local function init_globals()

	core.set_background('background', defaulttexturedir .. "menu_bg.png")
	core.set_clouds(false)

	-- Create main tabview
	local tv_main = tabview_create("maintab", {x = 15.5, y = 7.1})

	tv_main:add(tabs.connect)

	tv_main:set_global_event_handler(main_event_handler)

	ui.set_default("maintab")
	tv_main:show()
	ui.update()
end

init_globals()
