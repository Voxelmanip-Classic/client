--Minetest
--Copyright (C) 2022 rubenwardy
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

--------------------------------------------------------------------------------

local function register_formspec(dialogdata)
	local buttons_y = 5.5 + 0.5 + 0.8

	local retval = {
		"formspec_version[4]",
		"size[10,", tostring(buttons_y + 1.2), "]",
		"position[0.5,0.55]",
		formspec_styling,
		"set_focus[", (dialogdata.name ~= "" and "password" or "name"), "]",
		"label[0.375,0.8;", fgettext("Register"), "]",

		"label[0.375,2;Name]",
		"field[3.6,1.5;6,1;name;;", core.formspec_escape(dialogdata.name), "]",
		"label[0.375,3.5;Password]",
		"pwdfield[3.6,3;6,1;password;]",
		"label[0.375,5;Confirm Password]",
		"pwdfield[3.6,4.5;6,1;password_2;]",

		"container[0.375,", tostring(buttons_y), "]",
		"button[0,0;3,1;dlg_register_confirm;", fgettext("Register"), "]",
		"button[6.25,0;3,1;dlg_register_cancel;", fgettext("Cancel"), "]",
		"container_end[]",
	}

	if dialogdata.error then
		table.insert_all(retval, {
		"box[0.375,", tostring(buttons_y - 0.9), ";9.25,0.6;darkred]",
		"label[0.625,", tostring(buttons_y - 0.6), ";", core.formspec_escape(dialogdata.error or ""), "]",
		})
	end

	return table.concat(retval)
end

--------------------------------------------------------------------------------
local function register_buttonhandler(this, fields)
	this.data.name = fields.name
	this.data.error = nil

	if fields.dlg_register_confirm or fields.key_enter then
		if fields.name == "" then
			this.data.error = "Missing name"
		elseif fields.password ~= fields.password_2 then
			this.data.error = "Passwords do not match"
		elseif fields.password == "" then
			this.data.error = "Please enter a password"
		elseif string.len(fields.password) < 6 then
			this.data.error = "Password needs to be longer than 6 characters"
		end

		if this.data.error then return true end

		gamedata.playername = fields.name
		gamedata.password   = fields.password
		gamedata.address    = this.data.address
		gamedata.port       = this.data.port
		gamedata.allow_login_or_register = "register"
		gamedata.selected_world = 0

		assert(gamedata.address and gamedata.port)

		core.settings:set("name", fields.name)

		core.start()
	end

	if fields["dlg_register_cancel"] then
		this:delete()
		return true
	end

	return false
end

--------------------------------------------------------------------------------
function create_register_dialog(address, port)
	assert(address)
	assert(type(port) == "number")

	local retval = dialog_create("dlg_register",
			register_formspec,
			register_buttonhandler,
			nil)
	retval.data.address = address
	retval.data.port = port
	retval.data.name = core.settings:get("name") or ""
	return retval
end
