-- Minetest: builtin/client/init.lua
local scriptpath = core.get_builtin_path()
local clientpath = scriptpath.."client"..DIR_DELIM
local commonpath = scriptpath.."common"..DIR_DELIM

dofile(clientpath .. "register.lua")
dofile(commonpath .. "after.lua")
dofile(commonpath .. "mod_storage.lua")
dofile(commonpath .. "chatcommands.lua")
dofile(clientpath .. "chatcommands.lua")
dofile(clientpath .. "death_formspec.lua")
dofile(clientpath .. "misc.lua")
assert(loadfile(commonpath .. "item_s.lua"))({}) -- Just for push/read node functions

dofile(clientpath .. "tab_playerlist.lua")
dofile(clientpath .. "esc.lua")

print(dump(_G))

settingtypes_txt = dofile(core.get_builtin_path().."/settingtypes.txt")
print(lol)

local settingspath = scriptpath.."client"..DIR_DELIM.."settings"..DIR_DELIM
dofile(settingspath .. "settingtypes.lua")
dofile(settingspath .. "dlg_settings.lua")
