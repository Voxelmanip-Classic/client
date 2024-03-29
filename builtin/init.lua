--
-- This file contains built-in stuff in Minetest implemented in Lua.
--
-- It is always loaded and executed after registration of the C API,
-- before loading and running any mods.
--

-- Initialize some very basic things
function core.error_handler(err, level)
	return debug.traceback(tostring(err), level)
end
do
	local function concat_args(...)
		local n, t = select("#", ...), {...}
		for i = 1, n do
			t[i] = tostring(t[i])
		end
		return table.concat(t, "\t")
	end
	function core.debug(...) core.log(concat_args(...)) end
	if core.print then
		local core_print = core.print
		-- Override native print and use
		-- terminal if that's turned on
		function print(...) core_print(concat_args(...)) end
		core.print = nil -- don't pollute our namespace
	end
end
math.randomseed(os.time())
minetest = core

-- Load other files
local scriptdir = core.get_builtin_path()
local gamepath = scriptdir .. "game/"
local clientpath = scriptdir .. "client/"
local commonpath = scriptdir .. "common/"
local asyncpath = scriptdir .. "async/"

dofile(commonpath .. "vector.lua")
dofile(commonpath .. "strict.lua")
dofile(commonpath .. "serialize.lua")
dofile(commonpath .. "misc_helpers.lua")

if INIT == "game" then
	dofile(gamepath .. "init.lua")
	assert(not core.get_http_api)
elseif INIT == "mainmenu" then
	dofile(core.get_mainmenu_path() .. "/init.lua")
elseif INIT == "async"  then
	dofile(asyncpath .. "mainmenu.lua")
elseif INIT == "client" then
	dofile(clientpath .. "init.lua")
else
	error(("Unrecognized builtin initialization type %s!"):format(tostring(INIT)))
end
