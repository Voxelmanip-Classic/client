-- Minetest: builtin/common/chatcommands.lua

-- For server-side translations (if INIT == "game")
-- Otherwise, use core.gettext
local S = core.get_translator("__builtin")

core.registered_chatcommands = {}

-- Interpret the parameters of a command, separating options and arguments.
-- Input: command, param
--   command: name of command
--   param: parameters of command
-- Returns: opts, args
--   opts is a string of option letters, or false on error
--   args is an array with the non-option arguments in order, or an error message
-- Example: for this command line:
--      /command a b -cd e f -g
-- the function would receive:
--      a b -cd e f -g
-- and it would return:
--	"cdg", {"a", "b", "e", "f"}
-- Negative numbers are taken as arguments. Long options (--option) are
-- currently rejected as reserved.
local function getopts(command, param)
	local opts = ""
	local args = {}
	for match in param:gmatch("%S+") do
		if match:byte(1) == 45 then -- 45 = '-'
			local second = match:byte(2)
			if second == 45 then
				return false, S("Invalid parameters (see /help @1).", command)
			elseif second and (second < 48 or second > 57) then -- 48 = '0', 57 = '9'
				opts = opts .. match:sub(2)
			else
				-- numeric, add it to args
				args[#args + 1] = match
			end
		else
			args[#args + 1] = match
		end
	end
	return opts, args
end

function core.register_chatcommand(cmd, def)
	def = def or {}
	def.params = def.params or ""
	def.description = def.description or ""
	def.privs = def.privs or {}
	def.mod_origin = core.get_current_modname() or "??"
	core.registered_chatcommands[cmd] = def
end

function core.unregister_chatcommand(name)
	if core.registered_chatcommands[name] then
		core.registered_chatcommands[name] = nil
	else
		core.log("warning", "Not unregistering chatcommand " ..name..
			" because it doesn't exist.")
	end
end

function core.override_chatcommand(name, redefinition)
	local chatcommand = core.registered_chatcommands[name]
	assert(chatcommand, "Attempt to override non-existent chatcommand "..name)
	for k, v in pairs(redefinition) do
		rawset(chatcommand, k, v)
	end
	core.registered_chatcommands[name] = chatcommand
end

local function format_help_line(cmd, def)
	local cmd_marker = INIT == "client" and "." or "/"
	local msg = core.colorize("#00ffff", cmd_marker .. cmd)
	if def.params and def.params ~= "" then
		msg = msg .. " " .. def.params
	end
	if def.description and def.description ~= "" then
		msg = msg .. ": " .. def.description
	end
	return msg
end

local function do_help_cmd(name, param)
	local opts, args = getopts("help", param)
	if not opts then
		return false, args
	end
	if #args > 1 then
		return false, S("Too many arguments, try using just /help <command>")
	end

	if #args == 0 then
		local cmds = {}
		for cmd, def in pairs(core.registered_chatcommands) do
			if INIT == "client" or core.check_player_privs(name, def.privs) then
				cmds[#cmds + 1] = cmd
			end
		end
		table.sort(cmds)
		local msg
		msg = core.gettext("Available commands: ")
			.. table.concat(cmds, " ") .. "\n"
			.. core.gettext("Use '.help <cmd>' to get more "
			.. "information, or '.help all' to list "
			.. "everything.")
		return true, msg
	elseif args[1] == "all" then
		local cmds = {}
		for cmd, def in pairs(core.registered_chatcommands) do
			if INIT == "client" or core.check_player_privs(name, def.privs) then
				cmds[#cmds + 1] = format_help_line(cmd, def)
			end
		end
		table.sort(cmds)
		local msg = core.gettext("Available commands:")
		return true, msg.."\n"..table.concat(cmds, "\n")
	else
		local cmd = args[1]
		local def = core.registered_chatcommands[cmd]
		if not def then
			local msg = core.gettext("Command not available: ") .. cmd
			return false, msg
		else
			return true, format_help_line(cmd, def)
		end
	end
end

core.register_chatcommand("help", {
	params = core.gettext("[all | <cmd>]"),
	description = core.gettext("Get help for commands"),
	func = function(param)
		return do_help_cmd(nil, param)
	end,
})
