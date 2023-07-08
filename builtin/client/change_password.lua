
local formspec = [[
	formspec_version[6]
	size[11.5,8]
	no_prepend[]

	container[0.75,0.75]
	label[0,0.5;Old Password:]
	pwdfield[4,0;6,1;chpwd_old;]

	label[0,2;New Password:]
	pwdfield[4,1.5;6,1;chpwd_new;]

	label[0,3.5;Confirm Password:]
	pwdfield[4,3;6,1;chpwd_confirm;]

	button[0,5.5;3.5,1;chpwd_change;Change]
	button[6.5,5.5;3.5,1;chpwd_cancel;Cancel]
	container_end[]

	field_close_on_enter[chpwd_old;false]
	field_close_on_enter[chpwd_new;false]
	field_close_on_enter[chpwd_confirm;false]
]]

function dlg_change_password(message)
	local msg = message and "label[5,5.5;"..message.."]" or ""

	core.show_formspec('builtin:chpwd', formspec..msg)
end

core.register_on_formspec_input(function(formname, fields)
	if formname ~= "builtin:chpwd" then return end

	if fields.chpwd_cancel then
		core.close_formspec()
	end

	if fields.chpwd_change then
		if fields.chpwd_new == '' or fields.chpwd_confirm == '' then
			dlg_change_password("Please enter a new password.")
		elseif fields.chpwd_new == fields.chpwd_confirm then
			core.send_change_password(fields.chpwd_old, fields.chpwd_new)
			core.close_formspec()
		else
			dlg_change_password("Passwords do not match!")
		end
	end
end)
