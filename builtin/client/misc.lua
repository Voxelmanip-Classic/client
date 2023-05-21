function core.setting_get_pos(name)
	local value = core.settings:get(name)
	if not value then
		return nil
	end
	return core.string_to_pos(value)
end

-- Close any and all formspecs that are currently open clientside
function core.close_formspec()
	return core.show_formspec("", "")
end
