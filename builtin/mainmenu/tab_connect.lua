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
	local retval = [[
		label[1,0.75;Log into your Voxelmanip Classic server account.]

		container[0.7,1.75]
		label[0,0.5;Name]
		field[2.25,0;6,1;username;;ROllerozxa]
		label[0,2;Password]
		pwdfield[2.25,1.5;6,1;password;]
		button[2.25,3.25;4,1.25;btn_login;Login]
		container_end[]

		box[9.75,0;0.15,7.1;#555]
		container[0,1]
		label[11.65,0.8;No account?]
		button[10.7,1.7;4,1.25;btn_register;Register]
		container_end[]
	]]

	return retval, "size[15.5,7.1,false]position[0.5,0.65]real_coordinates[true]"
end

--------------------------------------------------------------------------------

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
		gamedata.address        = "voxelmanip.se"
		gamedata.port           = 30001
		gamedata.selected_world = 0
		gamedata.singleplayer   = false

		core.start()

		return true
	end

	if fields.btn_register then
		local dlg = create_register_dialog("voxelmanip.se", 30001, nil)
		dlg:set_parent(tabview)
		tabview:hide()
		dlg:show()
		return true
	end


	return false
end

local function on_change(type, old_tab, new_tab)

end

return {
	name = "connect",
	caption = fgettext("Connect"),
	cbf_formspec = get_formspec,
	cbf_button_handler = main_button_handler,
	on_change = on_change
}
