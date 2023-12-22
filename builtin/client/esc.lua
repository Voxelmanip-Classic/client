
core.register_on_esc(function()
	local fs = {[[
		formspec_version[4]
		size[6,7]
		position[0.5,0.5]
		padding[0,0]
		bgcolor[;true;#000000b0]
	]]}

	local btns = {
		{ "btn_continue", "Continue" },
		{ "btn_change_password", "Change Password" }
	}

	if PLATFORM ~= "Android" then
		table.insert(btns, { "btn_key_config", "Change Keys" })
	end

	table.insert(btns, { "btn_exit_menu", "Exit to Menu" })
	table.insert(btns, { "btn_exit_os", "Exit to OS" })

	for i = 1, #btns, 1 do
		table.insert(fs, ("button[1,%s;4,1;%s;%s]"):format((i-1)*1.5, btns[i][1], btns[i][2]))
	end

	core.show_formspec("builtin:esc", table.concat(fs))
end)

core.register_on_formspec_input(function(formname, fields)
	if formname ~= "builtin:esc" then return end

	if fields.btn_change_password then
		dlg_change_password()
		return
	end

	core.close_formspec()

	if fields.btn_key_config then
		core.show_keys_menu()
	end

	if fields.btn_exit_menu then
		core.disconnect()
	end
end)
