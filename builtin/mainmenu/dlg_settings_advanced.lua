--Minetest
--Copyright (C) 2015 PilzAdam
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

local FILENAME = "settingtypes.txt"

local CHAR_CLASSES = {
	SPACE = "[%s]",
	VARIABLE = "[%w_%-%.]",
	INTEGER = "[+-]?[%d]",
	FLOAT = "[+-]?[%d%.]",
	FLAGS = "[%w_%-%.,]",
}

local function flags_to_table(flags)
	return flags:gsub("%s+", ""):split(",", true) -- Remove all spaces and split
end

-- returns error message, or nil
local function parse_setting_line(settings, line, read_all, base_level, allow_secure)

	-- strip carriage returns (CR, /r)
	line = line:gsub("\r", "")

	-- comment
	local comment = line:match("^#" .. CHAR_CLASSES.SPACE .. "*(.*)$")
	if comment then
		if settings.current_comment == "" then
			settings.current_comment = comment
		else
			settings.current_comment = settings.current_comment .. "\n" .. comment
		end
		return
	end

	-- clear current_comment so only comments directly above a setting are bound to it
	-- but keep a local reference to it for variables in the current line
	local current_comment = settings.current_comment
	settings.current_comment = ""

	-- empty lines
	if line:match("^" .. CHAR_CLASSES.SPACE .. "*$") then
		return
	end

	-- category
	local stars, category = line:match("^%[([%*]*)([^%]]+)%]$")
	if category then
		table.insert(settings, {
			name = category,
			level = stars:len() + base_level,
			type = "category",
		})
		return
	end

	-- settings
	local first_part, name, readable_name, setting_type = line:match("^"
			-- this first capture group matches the whole first part,
			--  so we can later strip it from the rest of the line
			.. "("
				.. "([" .. CHAR_CLASSES.VARIABLE .. "+)" -- variable name
				.. CHAR_CLASSES.SPACE .. "*"
				.. "%(([^%)]*)%)"  -- readable name
				.. CHAR_CLASSES.SPACE .. "*"
				.. "(" .. CHAR_CLASSES.VARIABLE .. "+)" -- type
				.. CHAR_CLASSES.SPACE .. "*"
			.. ")")

	if not first_part then
		return "Invalid line"
	end

	if name:match("secure%.[.]*") and not allow_secure then
		return "Tried to add \"secure.\" setting"
	end

	if readable_name == "" then
		readable_name = nil
	end
	local remaining_line = line:sub(first_part:len() + 1)

	if setting_type == "int" then
		local default, min, max = remaining_line:match("^"
				-- first int is required, the last 2 are optional
				.. "(" .. CHAR_CLASSES.INTEGER .. "+)" .. CHAR_CLASSES.SPACE .. "*"
				.. "(" .. CHAR_CLASSES.INTEGER .. "*)" .. CHAR_CLASSES.SPACE .. "*"
				.. "(" .. CHAR_CLASSES.INTEGER .. "*)"
				.. "$")

		if not default or not tonumber(default) then
			return "Invalid integer setting"
		end

		min = tonumber(min)
		max = tonumber(max)
		table.insert(settings, {
			name = name,
			readable_name = readable_name,
			type = "int",
			default = default,
			min = min,
			max = max,
			comment = current_comment,
		})
		return
	end

	if setting_type == "string"
			or setting_type == "key" or setting_type == "v3f" then
		local default = remaining_line:match("^(.*)$")

		if not default then
			return "Invalid string setting"
		end
		if setting_type == "key" and not read_all then
			-- ignore key type if read_all is false
			return
		end

		table.insert(settings, {
			name = name,
			readable_name = readable_name,
			type = setting_type,
			default = default,
			comment = current_comment,
		})
		return
	end

	if setting_type == "bool" then
		if remaining_line ~= "false" and remaining_line ~= "true" then
			return "Invalid boolean setting"
		end

		table.insert(settings, {
			name = name,
			readable_name = readable_name,
			type = "bool",
			default = remaining_line,
			comment = current_comment,
		})
		return
	end

	if setting_type == "float" then
		local default, min, max = remaining_line:match("^"
				-- first float is required, the last 2 are optional
				.. "(" .. CHAR_CLASSES.FLOAT .. "+)" .. CHAR_CLASSES.SPACE .. "*"
				.. "(" .. CHAR_CLASSES.FLOAT .. "*)" .. CHAR_CLASSES.SPACE .. "*"
				.. "(" .. CHAR_CLASSES.FLOAT .. "*)"
				.."$")

		if not default or not tonumber(default) then
			return "Invalid float setting"
		end

		min = tonumber(min)
		max = tonumber(max)
		table.insert(settings, {
			name = name,
			readable_name = readable_name,
			type = "float",
			default = default,
			min = min,
			max = max,
			comment = current_comment,
		})
		return
	end

	if setting_type == "enum" then
		local default, values = remaining_line:match("^"
				-- first value (default) may be empty (i.e. is optional)
				.. "(" .. CHAR_CLASSES.VARIABLE .. "*)" .. CHAR_CLASSES.SPACE .. "*"
				.. "(" .. CHAR_CLASSES.FLAGS .. "+)"
				.. "$")

		if not default or values == "" then
			return "Invalid enum setting"
		end

		table.insert(settings, {
			name = name,
			readable_name = readable_name,
			type = "enum",
			default = default,
			values = values:split(",", true),
			comment = current_comment,
		})
		return
	end

	if setting_type == "path" or setting_type == "filepath" then
		local default = remaining_line:match("^(.*)$")

		if not default then
			return "Invalid path setting"
		end

		table.insert(settings, {
			name = name,
			readable_name = readable_name,
			type = setting_type,
			default = default,
			comment = current_comment,
		})
		return
	end

	return "Invalid setting type \"" .. setting_type .. "\""
end

local function parse_single_file(file, filepath, read_all, result, base_level, allow_secure)
	-- store this helper variable in the table so it's easier to pass to parse_setting_line()
	result.current_comment = ""

	local line = file:read("*line")
	while line do
		local error_msg = parse_setting_line(result, line, read_all, base_level, allow_secure)
		if error_msg then
			core.log("error", error_msg .. " in " .. filepath .. " \"" .. line .. "\"")
		end
		line = file:read("*line")
	end

	result.current_comment = nil
end

-- read_all: whether to ignore certain setting types for GUI or not
-- parse_mods: whether to parse settingtypes.txt in mods and games
local function parse_config_file(read_all, parse_mods)
	local settings = {}

	do
		local builtin_path = core.get_builtin_path() .. FILENAME
		local file = io.open(builtin_path, "r")
		if not file then
			core.log("error", "Can't load " .. FILENAME)
			return settings
		end

		parse_single_file(file, builtin_path, read_all, settings, 0, true)

		file:close()
	end

	if parse_mods then
		-- Parse games
		local games_category_initialized = false
		for _, game in ipairs(pkgmgr.games) do
			local path = game.path .. DIR_DELIM .. FILENAME
			local file = io.open(path, "r")
			if file then
				if not games_category_initialized then
					fgettext_ne("Content: Games") -- not used, but needed for xgettext
					table.insert(settings, {
						name = "Content: Games",
						level = 0,
						type = "category",
					})
					games_category_initialized = true
				end

				table.insert(settings, {
					name = game.name,
					level = 1,
					type = "category",
				})

				parse_single_file(file, path, read_all, settings, 2, false)

				file:close()
			end
		end

		-- Parse client mods
		local clientmods_category_initialized = false
		local clientmods = {}
		pkgmgr.get_mods(core.get_clientmodpath(), "clientmods", clientmods)
		for _, mod in ipairs(clientmods) do
			local path = mod.path .. DIR_DELIM .. FILENAME
			local file = io.open(path, "r")
			if file then
				if not clientmods_category_initialized then
					fgettext_ne("Client Mods") -- not used, but needed for xgettext
					table.insert(settings, {
						name = "Client Mods",
						level = 0,
						type = "category",
					})
					clientmods_category_initialized = true
				end

				table.insert(settings, {
					name = mod.name,
					level = 1,
					type = "category",
				})

				parse_single_file(file, path, read_all, settings, 2, false)

				file:close()
			end
		end
	end

	return settings
end

local function filter_settings(settings, searchstring)
	if not searchstring or searchstring == "" then
		return settings, -1
	end

	-- Setup the keyword list
	local keywords = {}
	for word in searchstring:lower():gmatch("%S+") do
		table.insert(keywords, word)
	end

	local result = {}
	local category_stack = {}
	local current_level = 0
	local best_setting = nil
	for _, entry in pairs(settings) do
		if entry.type == "category" then
			-- Remove all settingless categories
			while #category_stack > 0 and entry.level <= current_level do
				table.remove(category_stack, #category_stack)
				if #category_stack > 0 then
					current_level = category_stack[#category_stack].level
				else
					current_level = 0
				end
			end

			-- Push category onto stack
			category_stack[#category_stack + 1] = entry
			current_level = entry.level
		else
			-- See if setting matches keywords
			local setting_score = 0
			for k = 1, #keywords do
				local keyword = keywords[k]

				if string.find(entry.name:lower(), keyword, 1, true) then
					setting_score = setting_score + 1
				end

				if entry.readable_name and
						string.find(fgettext(entry.readable_name):lower(), keyword, 1, true) then
					setting_score = setting_score + 1
				end

				if entry.comment and
						string.find(fgettext_ne(entry.comment):lower(), keyword, 1, true) then
					setting_score = setting_score + 1
				end
			end

			-- Add setting to results if match
			if setting_score > 0 then
				-- Add parent categories
				for _, category in pairs(category_stack) do
					result[#result + 1] = category
				end
				category_stack = {}

				-- Add setting
				result[#result + 1] = entry
				entry.score = setting_score

				if not best_setting or
						setting_score > result[best_setting].score then
					best_setting = #result
				end
			end
		end
	end
	return result, best_setting or -1
end

local full_settings = parse_config_file(false, true)
local search_string = ""
local settings = full_settings
local selected_setting = 1

local function get_current_value(setting)
	local value = core.settings:get(setting.name)
	if value == nil then
		value = setting.default
	end
	return value
end

local checkboxes = {} -- handle checkboxes events

local function create_change_setting_formspec(dialogdata)
	local setting = settings[selected_setting]
	-- Final formspec will be created at the end of this function
	-- Default values below, may be changed depending on setting type
	local width = 10
	local height = 3.5
	local description_height = 3
	local formspec = ""

	-- Setting-specific formspec elements
	if setting.type == "bool" then
		local selected_index = 1
		if core.is_yes(get_current_value(setting)) then
			selected_index = 2
		end
		formspec = "dropdown[3," .. height .. ";4,1;dd_setting_value;"
				.. fgettext("Disabled") .. "," .. fgettext("Enabled") .. ";"
				.. selected_index .. "]"
		height = height + 1.25

	elseif setting.type == "enum" then
		local selected_index = 0
		formspec = "dropdown[3," .. height .. ";4,1;dd_setting_value;"
		for index, value in ipairs(setting.values) do
			-- translating value is not possible, since it's the value
			--  that we set the setting to
			formspec = formspec ..  core.formspec_escape(value) .. ","
			if get_current_value(setting) == value then
				selected_index = index
			end
		end
		if #setting.values > 0 then
			formspec = formspec:sub(1, -2) -- remove trailing comma
		end
		formspec = formspec .. ";" .. selected_index .. "]"
		height = height + 1.25

	else
		-- TODO: fancy input for float, int
		local text = get_current_value(setting)
		if dialogdata.error_message and dialogdata.entered_text then
			text = dialogdata.entered_text
		end
		formspec = "field[0.28," .. height + 0.15 .. ";" .. width .. ",1;te_setting_value;;"
				.. core.formspec_escape(text) .. "]"
		height = height + 1.15
	end

	-- Box good, textarea bad. Calculate textarea size from box.
	local function create_textfield(size, label, text, bg_color)
		local textarea = {
			x = size.x + 0.3,
			y = size.y,
			w = size.w + 0.25,
			h = size.h * 1.16 + 0.12
		}
		return ("box[%f,%f;%f,%f;%s]textarea[%f,%f;%f,%f;;%s;%s]"):format(
			size.x, size.y, size.w, size.h, bg_color or "#000",
			textarea.x, textarea.y, textarea.w, textarea.h,
			core.formspec_escape(label), core.formspec_escape(text)
		)

	end

	-- When there's an error: Shrink description textarea and add error below
	if dialogdata.error_message then
		local error_box = {
			x = 0,
			y = description_height - 0.4,
			w = width - 0.25,
			h = 0.5
		}
		formspec = formspec ..
			create_textfield(error_box, "", dialogdata.error_message, "#600")
		description_height = description_height - 0.75
	end

	-- Get description field
	local description_box = {
		x = 0,
		y = 0.2,
		w = width - 0.25,
		h = description_height
	}

	local setting_name = setting.name
	if setting.readable_name then
		setting_name = fgettext_ne(setting.readable_name) ..
			" (" .. setting.name .. ")"
	end

	local comment_text
	if setting.comment == "" then
		comment_text = fgettext_ne("(No description of setting given)")
	else
		comment_text = fgettext_ne(setting.comment)
	end

	return (
		"size[" .. width .. "," .. height + 0.25 .. ",true]" ..
		create_textfield(description_box, setting_name, comment_text) ..
		formspec ..
		"button[" .. width / 2 - 2.5 .. "," .. height - 0.4 .. ";2.5,1;btn_done;" ..
			fgettext("Save") .. "]" ..
		"button[" .. width / 2 .. "," .. height - 0.4 .. ";2.5,1;btn_cancel;" ..
			fgettext("Cancel") .. "]"
	)
end

local function handle_change_setting_buttons(this, fields)
	local setting = settings[selected_setting]
	if fields["btn_done"] or fields["key_enter"] then
		if setting.type == "bool" then
			local new_value = fields["dd_setting_value"]
			-- Note: new_value is the actual (translated) value shown in the dropdown
			core.settings:set_bool(setting.name, new_value == fgettext("Enabled"))

		elseif setting.type == "enum" then
			local new_value = fields["dd_setting_value"]
			core.settings:set(setting.name, new_value)

		elseif setting.type == "int" then
			local new_value = tonumber(fields["te_setting_value"])
			if not new_value or math.floor(new_value) ~= new_value then
				this.data.error_message = fgettext_ne("Please enter a valid integer.")
				this.data.entered_text = fields["te_setting_value"]
				core.update_formspec(this:get_formspec())
				return true
			end
			if setting.min and new_value < setting.min then
				this.data.error_message = fgettext_ne("The value must be at least $1.", setting.min)
				this.data.entered_text = fields["te_setting_value"]
				core.update_formspec(this:get_formspec())
				return true
			end
			if setting.max and new_value > setting.max then
				this.data.error_message = fgettext_ne("The value must not be larger than $1.", setting.max)
				this.data.entered_text = fields["te_setting_value"]
				core.update_formspec(this:get_formspec())
				return true
			end
			core.settings:set(setting.name, new_value)

		elseif setting.type == "float" then
			local new_value = tonumber(fields["te_setting_value"])
			if not new_value then
				this.data.error_message = fgettext_ne("Please enter a valid number.")
				this.data.entered_text = fields["te_setting_value"]
				core.update_formspec(this:get_formspec())
				return true
			end
			if setting.min and new_value < setting.min then
				this.data.error_message = fgettext_ne("The value must be at least $1.", setting.min)
				this.data.entered_text = fields["te_setting_value"]
				core.update_formspec(this:get_formspec())
				return true
			end
			if setting.max and new_value > setting.max then
				this.data.error_message = fgettext_ne("The value must not be larger than $1.", setting.max)
				this.data.entered_text = fields["te_setting_value"]
				core.update_formspec(this:get_formspec())
				return true
			end
			core.settings:set(setting.name, new_value)
		else
			local new_value = fields["te_setting_value"]
			core.settings:set(setting.name, new_value)
		end
		core.settings:write()
		this:delete()
		return true
	end

	if fields["btn_cancel"] then
		this:delete()
		return true
	end

	return false
end

local function create_settings_formspec(tabview, _, tabdata)
	local formspec = "size[14,8.4;true]" ..
			"position[0.5,0.55]" ..
			"tablecolumns[color;tree;text,width=28;text]" ..
			"tableoptions[background=#00000000;border=false]" ..
			"field[0.3,0.1;12.2,1;search_string;;" .. core.formspec_escape(search_string) .. "]" ..
			"field_close_on_enter[search_string;false]" ..
			"button[12.2,-0.2;2,1;search;" .. fgettext("Search") .. "]" ..
			"table[0,0.8;14,6.5;list_settings;"

	local current_level = 0
	for _, entry in ipairs(settings) do
		local name
		if not core.settings:get_bool("show_technical_names") and entry.readable_name then
			name = fgettext_ne(entry.readable_name)
		else
			name = entry.name
		end

		if entry.type == "category" then
			current_level = entry.level
			formspec = formspec .. "#FFFF00," .. current_level .. "," .. fgettext(name) .. ",,"

		elseif entry.type == "bool" then
			local value = get_current_value(entry)
			if core.is_yes(value) then
				value = fgettext("Enabled")
			else
				value = fgettext("Disabled")
			end
			formspec = formspec .. "," .. (current_level + 1) .. "," .. core.formspec_escape(name) .. ","
					.. value .. ","

		elseif entry.type == "key" then --luacheck: ignore
			-- ignore key settings, since we have a special dialog for them

		else
			formspec = formspec .. "," .. (current_level + 1) .. "," .. core.formspec_escape(name) .. ","
					.. core.formspec_escape(get_current_value(entry)) .. ","
		end
	end

	if #settings > 0 then
		formspec = formspec:sub(1, -2) -- remove trailing comma
	end
	formspec = formspec .. ";" .. selected_setting .. "]" ..
			"button[0,7.9;4,1;btn_back;".. fgettext("< Back to Settings page") .. "]" ..
			"button[12,7.9;2,1;btn_edit;" .. fgettext("Edit") .. "]" ..
			"button[9,7.9;3,1;btn_restore;" .. fgettext("Restore Default") .. "]" ..
			"checkbox[0,7.3;cb_tech_settings;" .. fgettext("Show technical names") .. ";"
					.. dump(core.settings:get_bool("show_technical_names")) .. "]"

	return formspec
end

local function handle_settings_buttons(this, fields, tabname, tabdata)
	local list_enter = false
	if fields["list_settings"] then
		selected_setting = core.get_table_index("list_settings")
		if core.explode_table_event(fields["list_settings"]).type == "DCL" then
			-- Directly toggle booleans
			local setting = settings[selected_setting]
			if setting and setting.type == "bool" then
				local current_value = get_current_value(setting)
				core.settings:set_bool(setting.name, not core.is_yes(current_value))
				core.settings:write()
				return true
			else
				list_enter = true
			end
		else
			return true
		end
	end

	if fields.search or fields.key_enter_field == "search_string" then
		if search_string == fields.search_string then
			if selected_setting > 0 then
				-- Go to next result on enter press
				local i = selected_setting + 1
				local looped = false
				while i > #settings or settings[i].type == "category" do
					i = i + 1
					if i > #settings then
						-- Stop infinte looping
						if looped then
							return false
						end
						i = 1
						looped = true
					end
				end
				selected_setting = i
				core.update_formspec(this:get_formspec())
				return true
			end
		else
			-- Search for setting
			search_string = fields.search_string
			settings, selected_setting = filter_settings(full_settings, search_string)
			core.update_formspec(this:get_formspec())
		end
		return true
	end

	if fields["btn_edit"] or list_enter then
		local setting = settings[selected_setting]
		if setting and setting.type ~= "category" then
			local edit_dialog = dialog_create("change_setting",
					create_change_setting_formspec, handle_change_setting_buttons)
			edit_dialog:set_parent(this)
			this:hide()
			edit_dialog:show()
		end
		return true
	end

	if fields["btn_restore"] then
		local setting = settings[selected_setting]
		if setting and setting.type ~= "category" then
			core.settings:remove(setting.name)
			core.settings:write()
			core.update_formspec(this:get_formspec())
		end
		return true
	end

	if fields["btn_back"] then
		this:delete()
		return true
	end

	if fields["cb_tech_settings"] then
		core.settings:set("show_technical_names", fields["cb_tech_settings"])
		core.settings:write()
		core.update_formspec(this:get_formspec())
		return true
	end

	return false
end

function create_adv_settings_dlg()
	local dlg = dialog_create("settings_advanced",
				create_settings_formspec,
				handle_settings_buttons,
				nil)

				return dlg
end
