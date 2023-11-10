/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "irrlichttypes.h" // must be included before anything irrlicht, see comment in the file
#include "irrlicht.h" // createDevice
#include "irrlichttypes_extrabloated.h"
#include "chat_interface.h"
#include "debug.h"
#include "server.h"
#include "filesys.h"
#include "version.h"
#include "client/game.h"
#include "defaultsettings.h"
#include "gettext.h"
#include "log.h"
#include "httpfetch.h"
#include "gameparams.h"
#include "database/database.h"
#include "config.h"
#include "porting.h"
#include "network/socket.h"
#include "serialization.h"

#ifndef SERVER
#include "gui/guiMainMenu.h"
#include "client/clientlauncher.h"
#include "gui/guiEngine.h"
#include "gui/mainmenumanager.h"
#endif
#ifdef HAVE_TOUCHSCREENGUI
	#include "gui/touchscreengui.h"
#endif

// for version information only
extern "C" {
#if USE_LUAJIT
	#include <luajit.h>
#else
	#include <lua.h>
#endif
}

#if !defined(__cpp_rtti) || !defined(__cpp_exceptions)
#error Minetest cannot be built without exceptions or RTTI
#endif

#if defined(__MINGW32__) && !defined(__MINGW64__) && !defined(__clang__) && \
	(__GNUC__ < 11 || (__GNUC__ == 11 && __GNUC_MINOR__ < 1))
// see e.g. https://github.com/minetest/minetest/issues/10137
#warning ==================================
#warning 32-bit MinGW gcc before 11.1 has known issues with crashes on thread exit, you should upgrade.
#warning ==================================
#endif

#define DEBUGFILE "debug.txt"
#define DEFAULT_SERVER_PORT 30000

#define ENV_NO_COLOR "NO_COLOR"
#define ENV_CLICOLOR "CLICOLOR"
#define ENV_CLICOLOR_FORCE "CLICOLOR_FORCE"

typedef std::map<std::string, ValueSpec> OptionList;

/**********************************************************************
 * Private functions
 **********************************************************************/

static void get_env_opts(Settings &args);
static bool get_cmdline_opts(int argc, char *argv[], Settings *cmd_args);
static void set_allowed_options(OptionList *allowed_options);

static void print_help(const OptionList &allowed_options);
static void print_allowed_options(const OptionList &allowed_options);
static void print_version();

static bool setup_log_params(const Settings &cmd_args);
static bool create_userdata_path();
static bool init_common(const Settings &cmd_args, int argc, char *argv[]);
static void uninit_common();
static void startup_message();
static bool read_config_file(const Settings &cmd_args);
static void init_log_streams(const Settings &cmd_args);

static bool game_configure(GameParams *game_params, const Settings &cmd_args);

/**********************************************************************/


FileLogOutput file_log_output;

static OptionList allowed_options;

int main(int argc, char *argv[])
{
	int retval;
	debug_set_exception_handler();

	g_logger.registerThread("Main");
	g_logger.addOutputMaxLevel(&stderr_output, LL_ACTION);

	Settings cmd_args;
	get_env_opts(cmd_args);
	bool cmd_args_ok = get_cmdline_opts(argc, argv, &cmd_args);
	if (!cmd_args_ok
			|| cmd_args.getFlag("help")
			|| cmd_args.exists("nonopt1")) {
		porting::attachOrCreateConsole();
		print_help(allowed_options);
		return cmd_args_ok ? 0 : 1;
	}
	if (cmd_args.getFlag("console"))
		porting::attachOrCreateConsole();

	if (cmd_args.getFlag("version")) {
		porting::attachOrCreateConsole();
		print_version();
		return 0;
	}

	if (!setup_log_params(cmd_args))
		return 1;

	porting::signal_handler_init();

#ifdef __ANDROID__
	porting::initAndroid();
	porting::initializePathsAndroid();
#else
	porting::initializePaths();
#endif

	if (!create_userdata_path()) {
		errorstream << "Cannot create user data directory" << std::endl;
		return 1;
	}

	// Debug handler
	BEGIN_DEBUG_EXCEPTION_HANDLER

	if (!init_common(cmd_args, argc, argv))
		return 1;

	GameStartData game_params;

	if (!game_configure(&game_params, cmd_args))
		return 1;

	retval = ClientLauncher().run(game_params, cmd_args) ? 0 : 1;

	// Update configuration file
	if (!g_settings_path.empty())
		g_settings->updateConfigFile(g_settings_path.c_str());

	END_DEBUG_EXCEPTION_HANDLER

	return retval;
}


/*****************************************************************************
 * Startup / Init
 *****************************************************************************/


static void get_env_opts(Settings &args)
{
	// CLICOLOR is a de-facto standard option for colors <https://bixense.com/clicolors/>
	// CLICOLOR != 0: ANSI colors are supported (auto-detection, this is the default)
	// CLICOLOR == 0: ANSI colors are NOT supported
	const char *clicolor = std::getenv(ENV_CLICOLOR);
	if (clicolor && std::string(clicolor) == "0") {
		args.set("color", "never");
	}
	// NO_COLOR only specifies that no color is allowed.
	// Implemented according to <http://no-color.org/>
	const char *no_color = std::getenv(ENV_NO_COLOR);
	if (no_color && no_color[0]) {
		args.set("color", "never");
	}
	// CLICOLOR_FORCE is another option, which should turn on colors "no matter what".
	const char *clicolor_force = std::getenv(ENV_CLICOLOR_FORCE);
	if (clicolor_force && std::string(clicolor_force) != "0") {
		// should ALWAYS have colors, so we ignore tty (no "auto")
		args.set("color", "always");
	}
}

static bool get_cmdline_opts(int argc, char *argv[], Settings *cmd_args)
{
	set_allowed_options(&allowed_options);

	return cmd_args->parseCommandLine(argc, argv, allowed_options);
}

static void set_allowed_options(OptionList *allowed_options)
{
	allowed_options->clear();

	allowed_options->insert(std::make_pair("help", ValueSpec(VALUETYPE_FLAG,
			_("Show allowed options"))));
	allowed_options->insert(std::make_pair("version", ValueSpec(VALUETYPE_FLAG,
			_("Show version information"))));
	allowed_options->insert(std::make_pair("config", ValueSpec(VALUETYPE_STRING,
			_("Load configuration from specified file"))));
	allowed_options->insert(std::make_pair("port", ValueSpec(VALUETYPE_STRING,
			_("Set network port (UDP)"))));
	allowed_options->insert(std::make_pair("quiet", ValueSpec(VALUETYPE_FLAG,
			_("Print to console errors only"))));
	allowed_options->insert(std::make_pair("color", ValueSpec(VALUETYPE_STRING,
			_("Coloured logs ('always', 'never' or 'auto'), defaults to 'auto'"
			))));
	allowed_options->insert(std::make_pair("info", ValueSpec(VALUETYPE_FLAG,
			_("Print more information to console"))));
	allowed_options->insert(std::make_pair("verbose",  ValueSpec(VALUETYPE_FLAG,
			_("Print even more information to console"))));
	allowed_options->insert(std::make_pair("trace", ValueSpec(VALUETYPE_FLAG,
			_("Print enormous amounts of information to log and console"))));
	allowed_options->insert(std::make_pair("debugger", ValueSpec(VALUETYPE_FLAG,
			_("Try to automatically attach a debugger before starting (convenience option)"))));
	allowed_options->insert(std::make_pair("logfile", ValueSpec(VALUETYPE_STRING,
			_("Set logfile path ('' = no logging)"))));

	allowed_options->insert(std::make_pair("address", ValueSpec(VALUETYPE_STRING,
			_("Address to connect to."))));
	allowed_options->insert(std::make_pair("name", ValueSpec(VALUETYPE_STRING,
			_("Set player name"))));
	allowed_options->insert(std::make_pair("password", ValueSpec(VALUETYPE_STRING,
			_("Set password"))));
	allowed_options->insert(std::make_pair("password-file", ValueSpec(VALUETYPE_STRING,
			_("Set password from contents of file"))));
	allowed_options->insert(std::make_pair("go", ValueSpec(VALUETYPE_FLAG,
			_("Disable main menu"))));

}

static void print_help(const OptionList &allowed_options)
{
	std::cout << _("Allowed options:") << std::endl;
	print_allowed_options(allowed_options);
}

static void print_allowed_options(const OptionList &allowed_options)
{
	for (const auto &allowed_option : allowed_options) {
		std::string opt = "  --" + allowed_option.first;
		if (allowed_option.second.type != VALUETYPE_FLAG)
			opt += _(" <value>");

		std::string opt_padded = padStringRight(opt, 30);
		std::cout << opt_padded;
		if (opt == opt_padded) // Line is too long to pad
			std::cout << std::endl << padStringRight("", 30);

		if (allowed_option.second.help)
			std::cout << allowed_option.second.help;

		std::cout << std::endl;
	}
}

static void print_version()
{
	std::cout << PROJECT_NAME_C " " << g_version_hash
		<< " (" << porting::getPlatformName() << ")" << std::endl;
#ifndef SERVER
	std::cout << "Using Irrlicht " IRRLICHT_SDK_VERSION << std::endl;
#endif
#if USE_LUAJIT
	std::cout << "Using " << LUAJIT_VERSION << std::endl;
#else
	std::cout << "Using " << LUA_RELEASE << std::endl;
#endif
	std::cout << g_build_info << std::endl;
}

static bool setup_log_params(const Settings &cmd_args)
{
	// Quiet mode, print errors only
	if (cmd_args.getFlag("quiet")) {
		g_logger.removeOutput(&stderr_output);
		g_logger.addOutputMaxLevel(&stderr_output, LL_ERROR);
	}

	// Coloured log messages (see log.h)
	std::string color_mode;
	if (cmd_args.exists("color")) {
		color_mode = cmd_args.get("color");
#if !defined(_WIN32)
	} else {
		char *color_mode_env = getenv("MT_LOGCOLOR");
		if (color_mode_env)
			color_mode = color_mode_env;
#endif
	}
	if (!color_mode.empty()) {
		if (color_mode == "auto") {
			Logger::color_mode = LOG_COLOR_AUTO;
		} else if (color_mode == "always") {
			Logger::color_mode = LOG_COLOR_ALWAYS;
		} else if (color_mode == "never") {
			Logger::color_mode = LOG_COLOR_NEVER;
		} else {
			errorstream << "Invalid color mode: " << color_mode << std::endl;
			return false;
		}
	}

	// In certain cases, output info level on stderr
	if (cmd_args.getFlag("info") || cmd_args.getFlag("verbose") ||
			cmd_args.getFlag("trace"))
		g_logger.addOutput(&stderr_output, LL_INFO);

	// In certain cases, output verbose level on stderr
	if (cmd_args.getFlag("verbose") || cmd_args.getFlag("trace"))
		g_logger.addOutput(&stderr_output, LL_VERBOSE);

	if (cmd_args.getFlag("trace")) {
		dstream << _("Enabling trace level debug output") << std::endl;
		g_logger.addOutput(&stderr_output, LL_TRACE);
		socket_enable_debug_output = true;
	}

	return true;
}

static bool create_userdata_path()
{
	bool success;

#ifdef __ANDROID__
	if (!fs::PathExists(porting::path_user)) {
		success = fs::CreateDir(porting::path_user);
	} else {
		success = true;
	}
#else
	// Create user data directory
	success = fs::CreateAllDirs(porting::path_user);
#endif

	return success;
}

static bool init_common(const Settings &cmd_args, int argc, char *argv[])
{
	startup_message();
	set_default_settings();

	sockets_init();

	// Initialize g_settings
	Settings::createLayer(SL_GLOBAL);

	// Set cleanup callback(s) to run at process exit
	atexit(uninit_common);

	if (!read_config_file(cmd_args))
		return false;

	init_log_streams(cmd_args);

	// Initialize random seed
	srand(time(0));
	mysrand(time(0));

	// Initialize HTTP fetcher
	httpfetch_init(10);

	init_gettext(porting::path_locale.c_str(),
		g_settings->get("language"), argc, argv);

	return true;
}

static void uninit_common()
{
	httpfetch_cleanup();

	sockets_cleanup();

	// It'd actually be okay to leak these but we want to please valgrind...
	for (int i = 0; i < (int)SL_TOTAL_COUNT; i++)
		delete Settings::getLayer((SettingsLayer)i);
}

static void startup_message()
{
	infostream << PROJECT_NAME << " " << _("with")
	           << " SER_FMT_VER_HIGHEST_READ="
               << (int)SER_FMT_VER_HIGHEST_READ << ", "
               << g_build_info << std::endl;
}

static bool read_config_file(const Settings &cmd_args)
{
	// Path of configuration file in use
	sanity_check(g_settings_path.empty());	// Sanity check

	if (cmd_args.exists("config")) {
		bool r = g_settings->readConfigFile(cmd_args.get("config").c_str());
		if (!r) {
			errorstream << "Could not read configuration from \""
			            << cmd_args.get("config") << "\"" << std::endl;
			return false;
		}
		g_settings_path = cmd_args.get("config");
	} else {
		std::vector<std::string> filenames;
		filenames.push_back(porting::path_user + DIR_DELIM + "voxelmanip_classic.conf");
		// Legacy configuration file location
		filenames.push_back(porting::path_user +
				DIR_DELIM + ".." + DIR_DELIM + "voxelmanip_classic.conf");

		for (const std::string &filename : filenames) {
			bool r = g_settings->readConfigFile(filename.c_str());
			if (r) {
				g_settings_path = filename;
				break;
			}
		}

		// If no path found, use the first one (menu creates the file)
		if (g_settings_path.empty())
			g_settings_path = filenames[0];
	}

	return true;
}

static void init_log_streams(const Settings &cmd_args)
{
	std::string log_filename = porting::path_user + DIR_DELIM + DEBUGFILE;

	if (cmd_args.exists("logfile"))
		log_filename = cmd_args.get("logfile");

	g_logger.removeOutput(&file_log_output);
	std::string conf_loglev = g_settings->get("debug_log_level");

	if (log_filename.empty() || conf_loglev.empty())  // No logging
		return;

	LogLevel log_level = Logger::stringToLevel(conf_loglev);
	if (log_level == LL_MAX) {
		warningstream << "Supplied unrecognized debug_log_level; "
			"using maximum." << std::endl;
	}

	file_log_output.setFile(log_filename,
		g_settings->getU64("debug_log_size_max") * 1000000);
	g_logger.addOutputMaxLevel(&file_log_output, log_level);
}

static bool game_configure(GameParams *game_params, const Settings &cmd_args)
{
	if (cmd_args.exists("port"))
		game_params->socket_port = cmd_args.getU16("port");
	else
		game_params->socket_port = 30000;

	return true;
}
