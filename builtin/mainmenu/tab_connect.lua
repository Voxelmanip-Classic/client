--Minetest
--Copyright (C) 2023 ROllerozxa
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

local function get_formspec(tabview, name, tabdata)
	header_show()

	local retval = formspec_styling..[[
		label[1,0.75;Log into your Voxelmanip Classic server account.]

		container[0.7,1.75]
		label[0,0.5;Name]
		field[2.25,0;6,1;username;;ROllerozxa]
		label[0,2;Password]
		pwdfield[2.25,1.5;6,1;password;]
		button[2.25,3.25;4,1.25;btn_login;Login]
		container_end[]

		box[9.75,0;0.15,7.1;#555]
		style[lbl_register;bgimg=;bgimg_hovered=;bgimg_pressed=;font_size=+0]
		button[10.7,0.8;4,0.9;lbl_register;No account?]
		button[10.7,2.2;4,1.25;btn_register;Register]

		box[9.9,4;5.6,0.15;#555]
		button[10.7,5;4,1.25;btn_settings;Settings]

	]]

	return retval, "size[15.5,7.1,false]position[0.5,0.55]padding[0,0]real_coordinates[true]"
end


local function main_button_handler(tabview, fields, name, tabdata)
	if fields.username then
		core.settings:set('name', fields.username)
	end

	if fields.btn_login or fields.key_enter then
		if fields.username == '' or fields.password == '' then
			return false
		end

		gamedata.playername     = fields.username
		gamedata.password       = fields.password
		gamedata.address        = serverdata.address
		gamedata.port           = serverdata.port
		gamedata.selected_world = 0
		gamedata.singleplayer   = false

		core.start()

		return true
	end

	if fields.btn_register then
		local dlg = create_register_dialog(serverdata.address, serverdata.port, nil)
		dlg:set_parent(tabview)
		tabview:hide()
		dlg:show()
		return true
	end

	if fields.btn_settings then
		local dlg = create_settings_dlg()
		dlg:set_parent(tabview)
		tabview:hide()
		dlg:show()
		return true
	end

	return false
end

return {
	name = "connect",
	caption = fgettext("Connect"),
	cbf_formspec = get_formspec,
	cbf_button_handler = main_button_handler
}
