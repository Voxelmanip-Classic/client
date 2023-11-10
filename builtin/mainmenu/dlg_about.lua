
local credit_list = {
	"Voxelmanip Classic is made possible by the Minetest",
	"engine, which is free software licensed under LGPLv2.1.",
	"",
	"For the source code of this engine fork, see",
	"github.com/rollerozxa/voxelmanip-classic-mt",
	"",
	core.colorize("#888", "Dedication"),
	"The Minetest 5.7.0 release was dedicated to the memory of",
	"Minetest developer Jude Melton-Houghton (TurkeyMcMac)",
	"who died on February 1, 2023.",
	"Our thoughts are with his family and friends.",
	"",
	core.colorize("#ff0", "Minetest Core Developers"),
	"Perttu Ahola (celeron55) <celeron55@gmail.com> [Project founder]",
	"sfan5 <sfan5@live.de>",
	"ShadowNinja <shadowninja@minetest.net>",
	"Nathanaëlle Courant (Nore/Ekdohibs) <nore@mesecons.net>",
	"Loic Blot (nerzhul/nrz) <loic.blot@unix-experience.fr>",
	"Andrew Ward (rubenwardy) <rw@rubenwardy.com>",
	"Krock/SmallJoker <mk939@ymail.com>",
	"Lars Hofhansl <larsh@apache.org>",
	"v-rob <robinsonvincent89@gmail.com>",
	"Hugues Ross <hugues.ross@gmail.com>",
	"Dmitry Kostenko (x2048) <codeforsmile@gmail.com>",
	"Desour",
	"",
	core.colorize("#ff0", "Minetest Core Team"),
	"Zughy [Issue triager]",
	"wsor [Issue triager]",
	"Hugo Locurcio (Calinou) [Issue triager]",
	"",
	core.colorize("#ff0", "Minetest Active Contributors"),
	"Wuzzy [Features, translations, devtest]",
	"Lars Müller [Bugfixes and entity features]",
	"paradust7 [Bugfixes]",
	"ROllerozxa [Bugfixes, Android]",
	"srifqi [Android, translations]",
	"Lexi Hale [Particlespawner animation]",
	"savilli [Bugfixes]",
	"fluxionary [Bugfixes]",
	"Gregor Parzefall [Bugfixes]",
	"Abdou-31 [Documentation]",
	"pecksin [Bouncy physics]",
	"Daroc Alden [Fixes]",
	"",
	core.colorize("#ff0", "Minetest Previous Core Developers"),
	"BlockMen",
	"Maciej Kasatkin (RealBadAngel) [RIP]",
	"Lisa Milne (darkrose) <lisa@ltmnet.com>",
	"proller",
	"Ilya Zhuravlev (xyz) <xyz@minetest.net>",
	"PilzAdam <pilzadam@minetest.net>",
	"est31 <MTest31@outlook.com>",
	"kahrl <kahrl@gmx.net>",
	"Ryan Kwolek (kwolekr) <kwolekr@minetest.net>",
	"sapier",
	"Zeno",
	"Auke Kok (sofar) <sofar@foo-projects.org>",
	"Aaron Suen <warr1024@gmail.com>",
	"paramat",
	"Pierre-Yves Rollo <dev@pyrollo.com>",
	"hecks",
	"Jude Melton-Houghton (TurkeyMcMac) [RIP]",
	"",
	core.colorize("#ff0", "Minetest Previous Contributors"),
	"Nils Dagsson Moskopp (erlehmann) <nils@dieweltistgarnichtso.net> [Minetest logo]",
	"red-001 <red-001@outlook.ie>",
	"Giuseppe Bilotta",
	"numzero",
	"HybridDog",
	"ClobberXD",
	"Dániel Juhász (juhdanad) <juhdanad@gmail.com>",
	"MirceaKitsune <mirceakitsune@gmail.com>",
	"Jean-Patrick Guerrero (kilbith)",
	"MoNTE48",
	"Constantin Wenger (SpeedProg)",
	"Ciaran Gultnieks (CiaranG)",
	"Paul Ouellette (pauloue)",
	"stujones11",
	"Rogier <rogier777@gmail.com>",
	"Gregory Currie (gregorycu)",
	"JacobF",
	"Jeija <jeija@mesecons.net>",
}

local function build_hacky_list(items, spacing)
	spacing = spacing or 0.5
	local y = spacing / 2
	local ret = {}
	for _, item in ipairs(items) do
		if item ~= "" then
			ret[#ret+1] = ("label[0,%f;%s]"):format(y, core.formspec_escape(item))
		end
		y = y + spacing
	end
	return table.concat(ret, ""), y
end

local function get_formspec(dialogdata)
	header_show()

	local credit_fs, scroll_height = build_hacky_list(credit_list)
	-- account for the visible portion
	scroll_height = math.max(0, scroll_height - 6.5)

	local share_debug = ''
	if PLATFORM == "Android" then
		share_debug = "button[6.5,0.5;3.5,1;share_debug;Share debug log]"
	end

	return table.concat{
		"formspec_version[6]",
		"size[15,9,false]",
		"position[0.5,0.55]",
		formspec_styling,

		"image[0.25,0.25;1.5,1.5;",
		core.formspec_escape(defaulttexturedir.."logo.png"), "]",

		"style_type[label;font=bold]",
		"label[2,0.75;Voxelmanip Classic]",
		"style_type[label;font=regular]",
		"label[2,1.35;v", core.get_version().string, "]",

		"button[10.5,0.5;4.25,1;homepage;classic.voxelmanip.se]",
		share_debug,

		"box[0.25,2;13.9,6.7;#000]",
		"scroll_container[0.35,2.1;13.8,6.5;scroll_credits;vertical;",
		tostring(scroll_height / 1000), "]", credit_fs,
		"scroll_container_end[]",
		"scrollbar[14.15,2;0.6,6.7;vertical;scroll_credits;0]",

		"style[btn_back;noclip=true]",
		"button[0,8.99;4,1;btn_back;Back]"
	}
end

local function buttonhandler(this, fields)
	if fields.homepage then
		core.open_url("https://classic.voxelmanip.se")
	end

	if fields.privacy then
		core.open_url("https://rollertest.voxelmanip.se/privacy.html")
	end

	if fields.share_debug then
		local path = core.get_user_path() .. "/debug.txt"
		core.share_file(path)
	end

	if fields.btn_back then
		this:delete()
		return true
	end
end

function create_about_dialog()
	return dialog_create("dlg_about", get_formspec, buttonhandler, nil)
end
