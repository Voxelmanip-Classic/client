/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "settings.h"
#include "porting.h"
#include "filesys.h"
#include "config.h"
#include "constants.h"
#include "porting.h"
#include "util/string.h"

void set_default_settings()
{
	Settings *settings = Settings::createLayer(SL_DEFAULTS);

	// Client and server
	settings->setDefault("language", "");
	settings->setDefault("name", "");
	settings->setDefault("bind_address", "");

	// Client
	settings->setDefault("address", "");
	settings->setDefault("sound_volume", "0.8");
	settings->setDefault("mute_sound", "false");
	settings->setDefault("mesh_generation_interval", "0");
	settings->setDefault("mesh_generation_threads", "0");
	settings->setDefault("meshgen_block_cache_size", "20");
	settings->setDefault("free_move", "false");
	settings->setDefault("fast_move", "false");
	settings->setDefault("noclip", "false");
	settings->setDefault("client_unload_unused_data_timeout", "600");
	settings->setDefault("client_mapblock_limit", "7500");
	settings->setDefault("enable_build_where_you_stand", "false");
	settings->setDefault("max_out_chat_queue_size", "20");
	settings->setDefault("pause_on_lost_focus", "false");
	settings->setDefault("occlusion_culler", "bfs");
	settings->setDefault("enable_raytraced_culling", "true");

	// Keymap
	settings->setDefault("remote_port", "30000");
	settings->setDefault("keymap_forward", "KEY_KEY_W");
	settings->setDefault("keymap_backward", "KEY_KEY_S");
	settings->setDefault("keymap_left", "KEY_KEY_A");
	settings->setDefault("keymap_right", "KEY_KEY_D");
	settings->setDefault("keymap_jump", "KEY_SPACE");
	settings->setDefault("keymap_sneak", "KEY_LSHIFT");
	settings->setDefault("keymap_dig", "KEY_LBUTTON");
	settings->setDefault("keymap_place", "KEY_RBUTTON");
	settings->setDefault("keymap_drop", "KEY_KEY_Q");
	settings->setDefault("keymap_zoom", "KEY_RCONTROL");
	settings->setDefault("keymap_inventory", "KEY_KEY_E");
	settings->setDefault("keymap_aux1", "KEY_KEY_F");
	settings->setDefault("keymap_chat", "KEY_KEY_T");
	settings->setDefault("keymap_cmd", "/");
	settings->setDefault("keymap_cmd_local", ".");
	settings->setDefault("keymap_minimap", "KEY_KEY_V");
	settings->setDefault("keymap_console", "KEY_F10");
	settings->setDefault("keymap_rangeselect", "KEY_KEY_R");
	settings->setDefault("keymap_freemove", "KEY_KEY_Z");
	settings->setDefault("keymap_fastmove", "KEY_LCONTROL");
	settings->setDefault("keymap_noclip", "KEY_KEY_X");
	settings->setDefault("keymap_hotbar_next", "KEY_KEY_N");
	settings->setDefault("keymap_hotbar_previous", "KEY_KEY_B");
	settings->setDefault("keymap_mute", "KEY_KEY_M");
	settings->setDefault("keymap_cinematic", "");
	settings->setDefault("keymap_toggle_hud", "KEY_F1");
	settings->setDefault("keymap_screenshot", "KEY_F2");
	settings->setDefault("keymap_toggle_debug", "KEY_F3");

#if DEBUG
	settings->setDefault("keymap_toggle_block_bounds", "");
#else
	settings->setDefault("keymap_toggle_block_bounds", "KEY_F4");
#endif
	settings->setDefault("keymap_camera_mode", "KEY_F5");
	settings->setDefault("keymap_toggle_chat", "KEY_F7");
	settings->setDefault("keymap_toggle_profiler", "");
	settings->setDefault("keymap_toggle_fog", "");
	settings->setDefault("keymap_increase_viewing_range_min", "+");
	settings->setDefault("keymap_decrease_viewing_range_min", "-");
	settings->setDefault("keymap_slot1", "KEY_KEY_1");
	settings->setDefault("keymap_slot2", "KEY_KEY_2");
	settings->setDefault("keymap_slot3", "KEY_KEY_3");
	settings->setDefault("keymap_slot4", "KEY_KEY_4");
	settings->setDefault("keymap_slot5", "KEY_KEY_5");
	settings->setDefault("keymap_slot6", "KEY_KEY_6");
	settings->setDefault("keymap_slot7", "KEY_KEY_7");
	settings->setDefault("keymap_slot8", "KEY_KEY_8");
	settings->setDefault("keymap_slot9", "KEY_KEY_9");
	settings->setDefault("keymap_slot10", "KEY_KEY_0");
	settings->setDefault("keymap_slot11", "");
	settings->setDefault("keymap_slot12", "");
	settings->setDefault("keymap_slot13", "");
	settings->setDefault("keymap_slot14", "");
	settings->setDefault("keymap_slot15", "");
	settings->setDefault("keymap_slot16", "");
	settings->setDefault("keymap_slot17", "");
	settings->setDefault("keymap_slot18", "");
	settings->setDefault("keymap_slot19", "");
	settings->setDefault("keymap_slot20", "");
	settings->setDefault("keymap_slot21", "");
	settings->setDefault("keymap_slot22", "");
	settings->setDefault("keymap_slot23", "");
	settings->setDefault("keymap_slot24", "");
	settings->setDefault("keymap_slot25", "");
	settings->setDefault("keymap_slot26", "");
	settings->setDefault("keymap_slot27", "");
	settings->setDefault("keymap_slot28", "");
	settings->setDefault("keymap_slot29", "");
	settings->setDefault("keymap_slot30", "");
	settings->setDefault("keymap_slot31", "");
	settings->setDefault("keymap_slot32", "");

	settings->setDefault("keymap_tabb", "KEY_TAB");

	// Visuals
#ifdef NDEBUG
	settings->setDefault("show_debug", "false");
#else
	settings->setDefault("show_debug", "true");
#endif
	settings->setDefault("fsaa", "0");
	settings->setDefault("undersampling", "1");
	settings->setDefault("world_aligned_mode", "enable");
	settings->setDefault("autoscale_mode", "disable");
	settings->setDefault("enable_fog", "true");
	settings->setDefault("fog_start", "0.4");
	settings->setDefault("3d_mode", "none");
	settings->setDefault("3d_paralax_strength", "0.025");
	settings->setDefault("tooltip_show_delay", "400");
	settings->setDefault("tooltip_append_itemname", "false");
	settings->setDefault("fps_max", "120");
	settings->setDefault("fps_max_unfocused", "20");
	settings->setDefault("viewing_range", "190");
	settings->setDefault("client_mesh_chunk", "1");
	settings->setDefault("screen_w", "1280");
	settings->setDefault("screen_h", "720");
	settings->setDefault("window_maximized", "false");
	settings->setDefault("autosave_screensize", "false");
	settings->setDefault("fullscreen", "false");
	settings->setDefault("vsync", "true");
	settings->setDefault("fov", "72");
	settings->setDefault("leaves_style", "fancy");
	settings->setDefault("connected_glass", "true");
	settings->setDefault("smooth_lighting", "true");
	settings->setDefault("performance_tradeoffs", "false");
	settings->setDefault("lighting_alpha", "0.0");
	settings->setDefault("lighting_beta", "1.5");
	settings->setDefault("display_gamma", "1.0");
	settings->setDefault("lighting_boost", "0.2");
	settings->setDefault("lighting_boost_center", "0.5");
	settings->setDefault("lighting_boost_spread", "0.2");
	settings->setDefault("texture_path", "");
	settings->setDefault("shader_path", "");
	settings->setDefault("video_driver", "");
	settings->setDefault("cinematic", "false");
	settings->setDefault("camera_smoothing", "0.0");
	settings->setDefault("cinematic_camera_smoothing", "0.7");
	settings->setDefault("enable_clouds", "true");
	settings->setDefault("view_bobbing_amount", "1.0");
	settings->setDefault("enable_3d_clouds", "false");
	settings->setDefault("recent_chat_messages", "6");
	settings->setDefault("hud_scaling", "1.0");
	settings->setDefault("gui_scaling", "1.0");
	settings->setDefault("gui_scaling_filter", "false");
	settings->setDefault("gui_scaling_filter_txr2img", "true");
	settings->setDefault("hud_hotbar_max_width", "1.0");
	settings->setDefault("show_entity_selectionbox", "false");
	settings->setDefault("ambient_occlusion_gamma", "1.8");
	settings->setDefault("enable_shaders", "true");
	settings->setDefault("enable_particles", "true");
	settings->setDefault("show_nametag_backgrounds", "true");
	settings->setDefault("transparency_sorting_distance", "16");

	settings->setDefault("enable_minimap", "true");
	settings->setDefault("minimap_shape_round", "true");
	settings->setDefault("minimap_double_scan_height", "true");

	// Effects
	settings->setDefault("inventory_items_animations", "true");
	settings->setDefault("mip_map", "true");
	settings->setDefault("anisotropic_filter", "true");
	settings->setDefault("tone_mapping", "false");
	settings->setDefault("enable_waving_water", "false");
	settings->setDefault("enable_waving_leaves", "false");
	settings->setDefault("enable_waving_plants", "false");
	settings->setDefault("exposure_compensation", "0.0");
	settings->setDefault("enable_auto_exposure", "false");
	settings->setDefault("antialiasing", "none");
	settings->setDefault("enable_bloom", "false");
	settings->setDefault("enable_bloom_debug", "false");
	settings->setDefault("bloom_strength_factor", "1.0");
	settings->setDefault("bloom_intensity", "0.05");
	settings->setDefault("bloom_radius", "1");

	// Effects Shadows
	settings->setDefault("enable_dynamic_shadows", "false");
	settings->setDefault("shadow_strength_gamma", "1.0");
	settings->setDefault("shadow_map_max_distance", "200.0");
	settings->setDefault("shadow_map_texture_size", "2048");
	settings->setDefault("shadow_map_texture_32bit", "true");
	settings->setDefault("shadow_map_color", "false");
	settings->setDefault("shadow_filters", "1");
	settings->setDefault("shadow_poisson_filter", "true");
	settings->setDefault("shadow_update_frames", "8");
	settings->setDefault("shadow_soft_radius", "5.0");
	settings->setDefault("shadow_sky_body_orbit_tilt", "0.0");

	// Input
	settings->setDefault("invert_mouse", "false");
	settings->setDefault("enable_hotbar_mouse_wheel", "true");
	settings->setDefault("invert_hotbar_mouse_wheel", "false");
	settings->setDefault("mouse_sensitivity", "0.2");
	settings->setDefault("repeat_place_time", "0.25");
	settings->setDefault("safe_dig_and_place", "false");
	settings->setDefault("aux1_descends", "false");
	settings->setDefault("doubletap_jump", "false");
	settings->setDefault("always_fly_fast", "true");
#ifdef HAVE_TOUCHSCREENGUI
	settings->setDefault("autojump", "true");
#else
	settings->setDefault("autojump", "false");
#endif
	settings->setDefault("continuous_forward", "false");
	settings->setDefault("enable_joysticks", "false");
	settings->setDefault("joystick_id", "0");
	settings->setDefault("joystick_type", "auto");
	settings->setDefault("repeat_joystick_button_time", "0.17");
	settings->setDefault("joystick_frustum_sensitivity", "170");
	settings->setDefault("joystick_deadzone", "2048");

	// General font settings
	settings->setDefault("font_path", porting::getDataPath("fonts" DIR_DELIM "Archivo-Regular.ttf"));
	settings->setDefault("font_path_italic", porting::getDataPath("fonts" DIR_DELIM "Archivo-Regular.ttf"));
	settings->setDefault("font_path_bold", porting::getDataPath("fonts" DIR_DELIM "Archivo-Bold.ttf"));
	settings->setDefault("font_path_bold_italic", porting::getDataPath("fonts" DIR_DELIM "Archivo-Bold.ttf"));
	settings->setDefault("font_bold", "false");
	settings->setDefault("font_italic", "false");
	settings->setDefault("font_shadow", "1");
	settings->setDefault("font_shadow_alpha", "255");
	settings->setDefault("font_size_divisible_by", "1");
	settings->setDefault("mono_font_path", porting::getDataPath("fonts" DIR_DELIM "Cousine-Regular.ttf"));
	settings->setDefault("mono_font_path_italic", porting::getDataPath("fonts" DIR_DELIM "Cousine-Regular.ttf"));
	settings->setDefault("mono_font_path_bold", porting::getDataPath("fonts" DIR_DELIM "Cousine-Bold.ttf"));
	settings->setDefault("mono_font_path_bold_italic", porting::getDataPath("fonts" DIR_DELIM "Cousine-Bold.ttf"));
	settings->setDefault("mono_font_size_divisible_by", "1");
	settings->setDefault("fallback_font_path", porting::getDataPath("fonts" DIR_DELIM "Arimo-Regular.ttf"));

	settings->setDefault("font_size", "18");
	settings->setDefault("mono_font_size", "16");
	settings->setDefault("chat_font_size", "0"); // Default "font_size"

	// Network
	settings->setDefault("enable_ipv6", "true");
	settings->setDefault("max_packets_per_iteration","1024");
	settings->setDefault("port", "30000");

	settings->setDefault("deprecated_lua_api_handling", "log");

	settings->setDefault("profiler_print_interval", "0");
	settings->setDefault("debug_log_level", "action");
	settings->setDefault("debug_log_size_max", "50");
	settings->setDefault("chat_log_level", "error");
	settings->setDefault("secure.enable_security", "true");
	settings->setDefault("secure.trusted_mods", "");

	settings->setDefault("screen_dpi", "72");
	settings->setDefault("display_density_factor", "1");

	// Altered settings for macOS
#if defined(__MACH__) && defined(__APPLE__)
	settings->setDefault("keymap_sneak", "KEY_SHIFT");
#endif

#ifdef HAVE_TOUCHSCREENGUI
	settings->setDefault("touchscreen_threshold","20");
	settings->setDefault("touch_use_crosshair", "true");
	settings->setDefault("fixed_virtual_joystick", "false");
	settings->setDefault("virtual_joystick_triggers_aux1", "false");
#endif
	// Altered settings for Android
#ifdef __ANDROID__
	settings->setDefault("fullscreen", "true");
	settings->setDefault("smooth_lighting", "false");
	settings->setDefault("performance_tradeoffs", "true");
	settings->setDefault("enable_3d_clouds", "false");
	settings->setDefault("fps_max_unfocused", "10");
	settings->setDefault("client_mapblock_limit", "1000");
	settings->setDefault("viewing_range", "90");
	settings->setDefault("leaves_style", "simple");
	settings->setDefault("transparency_sorting_distance", "1");

	// Apply settings according to screen size
	float x_inches = (float) porting::getDisplaySize().X /
			(160.f * porting::getDisplayDensity());

	if (x_inches < 3.7f) {
		settings->setDefault("hud_scaling", "0.6");
		settings->setDefault("font_size", "14");
		settings->setDefault("mono_font_size", "14");
	} else if (x_inches < 4.5f) {
		settings->setDefault("hud_scaling", "0.7");
		settings->setDefault("font_size", "14");
		settings->setDefault("mono_font_size", "14");
	} else if (x_inches < 6.0f) {
		settings->setDefault("hud_scaling", "0.85");
		settings->setDefault("font_size", "14");
		settings->setDefault("mono_font_size", "14");
	}
	// Tablets >= 6.0 use non-Android defaults for these settings
#endif
}
