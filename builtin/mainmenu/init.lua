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

mt_color_grey  = "#AAAAAA"
mt_color_blue  = "#6389FF"
mt_color_lightblue  = "#99CCFF"
mt_color_green = "#72FF63"
mt_color_dark_green = "#25C191"
mt_color_orange  = "#FF8800"
mt_color_red = "#FF3300"

local menupath = core.get_mainmenu_path()
local basepath = core.get_builtin_path()
defaulttexturedir = core.get_texturepath_share() .. DIR_DELIM .. "base" ..
					DIR_DELIM .. "pack" .. DIR_DELIM

dofile(basepath .. "common" .. DIR_DELIM .. "filterlist.lua")
dofile(basepath .. "fstk" .. DIR_DELIM .. "dialog.lua")
dofile(basepath .. "fstk" .. DIR_DELIM .. "tabview.lua")
dofile(basepath .. "fstk" .. DIR_DELIM .. "ui.lua")
dofile(menupath .. DIR_DELIM .. "async_event.lua")
menudata = {}
dofile(menupath .. DIR_DELIM .. "pkgmgr.lua")

dofile(menupath .. DIR_DELIM .. "dlg_about.lua")
dofile(menupath .. DIR_DELIM .. "dlg_register.lua")

local tabs = {}

dofile(menupath .. DIR_DELIM .. "settings" .. DIR_DELIM .. "init.lua")
tabs.connect = dofile(menupath .. DIR_DELIM .. "tab_connect.lua")

--------------------------------------------------------------------------------

function header_show()
	core.set_background('header', defaulttexturedir .. "menu_header.png")
end

function header_hide()
	core.set_background('header', "")
end

--------------------------------------------------------------------------------
local function main_event_handler(tabview, event)
	if event == "MenuQuit" then
		core.close()
	end
	return true
end

--------------------------------------------------------------------------------
local function init_globals()

	core.set_background('background', defaulttexturedir .. "menu_bg.png")
	core.set_clouds(false)
	core.set_topleft_text("Voxelmanip Client - In development!")

	-- Create main tabview
	local tv_main = tabview_create("maintab", {x = 12, y = 5.4}, {x = 0, y = 0})
	-- note: size would be 15.5,7.1 in real coordinates mode

	tv_main:set_autosave_tab(true)
	tv_main:add(tabs.connect)

	tv_main:set_global_event_handler(main_event_handler)
	tv_main:set_fixed_size(false)

	ui.set_default("maintab")
	tv_main:show()
	ui.update()
end

init_globals()
