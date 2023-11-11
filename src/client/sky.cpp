/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
Copyright (C) 2020 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

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

#include <cmath>
#include "sky.h"
#include <ITexture.h>
#include <IVideoDriver.h>
#include <ISceneManager.h>
#include <ICameraSceneNode.h>
#include <S3DVertex.h>
#include "client/tile.h"
#include "noise.h" // easeCurve
#include "profiler.h"
#include "util/numeric.h"
#include "client/renderingengine.h"
#include "settings.h"
#include "camera.h" // CameraModes

using namespace irr::core;

static video::SMaterial baseMaterial()
{
	video::SMaterial mat;
	mat.Lighting = false;
	mat.ZBuffer = video::ECFN_DISABLED;
	mat.ZWriteEnable = video::EZW_OFF;
	mat.AntiAliasing = 0;
	mat.TextureLayers[0].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
	mat.TextureLayers[0].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
	mat.BackfaceCulling = false;
	return mat;
}

static inline void disableTextureFiltering(video::SMaterial &mat)
{
	mat.forEachTexture([] (auto &tex) {
		tex.MinFilter = video::ETMINF_NEAREST_MIPMAP_NEAREST;
		tex.MagFilter = video::ETMAGF_NEAREST;
		tex.AnisotropicFilter = 0;
	});
}

Sky::Sky(s32 id, RenderingEngine *rendering_engine, ITextureSource *tsrc, IShaderSource *ssrc) :
		scene::ISceneNode(rendering_engine->get_scene_manager()->getRootSceneNode(),
			rendering_engine->get_scene_manager(), id)
{
	m_seed = (u64)myrand() << 32 | myrand();

	setAutomaticCulling(scene::EAC_OFF);
	m_box.MaxEdge.set(0, 0, 0);
	m_box.MinEdge.set(0, 0, 0);

	m_enable_shaders = g_settings->getBool("enable_shaders");

	m_sky_params = SkyboxDefaults::getSkyDefaults();

	// Create materials

	m_materials[0] = baseMaterial();
	m_materials[0].MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
	m_materials[0].Lighting = true;
	m_materials[0].ColorMaterial = video::ECM_NONE;

	m_materials[1] = baseMaterial();
	m_materials[1].MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;

	m_materials[2] = baseMaterial();
	m_materials[2].setTexture(0, tsrc->getTextureForMesh("blank.png"));
	m_materials[2].MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;

	for (int i = 5; i < 11; i++) {
		m_materials[i] = baseMaterial();
		m_materials[i].Lighting = true;
		m_materials[i].MaterialType = video::EMT_SOLID;
	}

	m_sky_params.body_orbit_tilt = g_settings->getFloat("shadow_sky_body_orbit_tilt", -60., 60.);
	m_sky_params.fog_start = rangelim(g_settings->getFloat("fog_start"), 0.0f, 0.99f);
}

void Sky::OnRegisterSceneNode()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this, scene::ESNRP_SKY_BOX);

	scene::ISceneNode::OnRegisterSceneNode();
}

void Sky::render()
{
	video::IVideoDriver *driver = SceneManager->getVideoDriver();
	scene::ICameraSceneNode *camera = SceneManager->getActiveCamera();

	if (!camera || !driver)
		return;

	ScopeProfiler sp(g_profiler, "Sky::render()", SPT_AVG);

	// Draw perspective skybox

	core::matrix4 translate(AbsoluteTransformation);
	translate.setTranslation(camera->getAbsolutePosition());

	// Draw the sky box between the near and far clip plane
	const f32 viewDistance = (camera->getNearValue() + camera->getFarValue()) * 0.5f;
	core::matrix4 scale;
	scale.setScale(core::vector3df(viewDistance, viewDistance, viewDistance));

	driver->setTransform(video::ETS_WORLD, translate * scale);

	if (m_sunlight_seen) {

		const f32 t = 1.0f;
		const f32 o = 0.0f;
		static const u16 indices[6] = {0, 1, 2, 0, 2, 3};
		video::S3DVertex vertices[4];

		driver->setMaterial(m_materials[1]);

		video::SColor cloudyfogcolor = m_bgcolor;

		// Abort rendering if we're in the clouds.
		// Stops rendering a pure white hole in the bottom of the skybox.
		if (m_in_clouds)
			return;

		// Draw the six sided skybox,
		if (m_sky_params.textures.size() == 6) {
			for (u32 j = 5; j < 11; j++) {
				video::SColor c(255, 255, 255, 255);
				driver->setMaterial(m_materials[j]);
				// Use 1.05 rather than 1.0 to avoid colliding with the
				// sun, moon and stars, as this is a background skybox.
				vertices[0] = video::S3DVertex(-1.05, -1.05, -1.05, 0, 0, 1, c, t, t);
				vertices[1] = video::S3DVertex( 1.05, -1.05, -1.05, 0, 0, 1, c, o, t);
				vertices[2] = video::S3DVertex( 1.05,  1.05, -1.05, 0, 0, 1, c, o, o);
				vertices[3] = video::S3DVertex(-1.05,  1.05, -1.05, 0, 0, 1, c, t, o);
				for (video::S3DVertex &vertex : vertices) {
					if (j == 5) { // Top texture
						vertex.Pos.rotateYZBy(90);
						vertex.Pos.rotateXZBy(90);
					} else if (j == 6) { // Bottom texture
						vertex.Pos.rotateYZBy(-90);
						vertex.Pos.rotateXZBy(90);
					} else if (j == 7) { // Left texture
						vertex.Pos.rotateXZBy(90);
					} else if (j == 8) { // Right texture
						vertex.Pos.rotateXZBy(-90);
					} else if (j == 9) { // Front texture, do nothing
						// Irrlicht doesn't like it when vertexes are left
						// alone and not rotated for some reason.
						vertex.Pos.rotateXZBy(0);
					} else {// Back texture
						vertex.Pos.rotateXZBy(180);
					}
				}
				driver->drawIndexedTriangleList(&vertices[0], 4, indices, 2);
			}
		}

		// Draw far cloudy fog thing blended with skycolor
		if (m_visible) {
			driver->setMaterial(m_materials[1]);
			for (u32 j = 0; j < 4; j++) {
				vertices[0] = video::S3DVertex(-1, -0.02, -1, 0, 0, 1, m_bgcolor, t, t);
				vertices[1] = video::S3DVertex( 1, -0.02, -1, 0, 0, 1, m_bgcolor, o, t);
				vertices[2] = video::S3DVertex( 1, 0.45, -1, 0, 0, 1, m_skycolor, o, o);
				vertices[3] = video::S3DVertex(-1, 0.45, -1, 0, 0, 1, m_skycolor, t, o);
				for (video::S3DVertex &vertex : vertices) {
					if (j == 0)
						// Don't switch
						{}
					else if (j == 1)
						// Switch from -Z (south) to +X (east)
						vertex.Pos.rotateXZBy(90);
					else if (j == 2)
						// Switch from -Z (south) to -X (west)
						vertex.Pos.rotateXZBy(-90);
					else
						// Switch from -Z (south) to +Z (north)
						vertex.Pos.rotateXZBy(-180);
				}
				driver->drawIndexedTriangleList(&vertices[0], 4, indices, 2);
			}
		}

		// Draw far cloudy fog thing below all horizons in front of sun, moon
		// and stars.
		if (m_visible) {
			driver->setMaterial(m_materials[1]);

			for (u32 j = 0; j < 4; j++) {
				video::SColor c = cloudyfogcolor;
				vertices[0] = video::S3DVertex(-1, -1.0,  -1, 0, 0, 1, c, t, t);
				vertices[1] = video::S3DVertex( 1, -1.0,  -1, 0, 0, 1, c, o, t);
				vertices[2] = video::S3DVertex( 1, -0.02, -1, 0, 0, 1, c, o, o);
				vertices[3] = video::S3DVertex(-1, -0.02, -1, 0, 0, 1, c, t, o);
				for (video::S3DVertex &vertex : vertices) {
					if (j == 0)
						// Don't switch
						{}
					else if (j == 1)
						// Switch from -Z (south) to +X (east)
						vertex.Pos.rotateXZBy(90);
					else if (j == 2)
						// Switch from -Z (south) to -X (west)
						vertex.Pos.rotateXZBy(-90);
					else
						// Switch from -Z (south) to +Z (north)
						vertex.Pos.rotateXZBy(-180);
				}
				driver->drawIndexedTriangleList(&vertices[0], 4, indices, 2);
			}

			// Draw bottom far cloudy fog thing in front of sun, moon and stars
			video::SColor c = cloudyfogcolor;
			vertices[0] = video::S3DVertex(-1, -1.0, -1, 0, 1, 0, c, t, t);
			vertices[1] = video::S3DVertex( 1, -1.0, -1, 0, 1, 0, c, o, t);
			vertices[2] = video::S3DVertex( 1, -1.0, 1, 0, 1, 0, c, o, o);
			vertices[3] = video::S3DVertex(-1, -1.0, 1, 0, 1, 0, c, t, o);
			driver->drawIndexedTriangleList(&vertices[0], 4, indices, 2);
		}
	}
}

void Sky::update(float time_of_day, float time_brightness,
	float direct_brightness, bool sunlight_seen,
	CameraMode cam_mode, float yaw, float pitch)
{
	// Stabilize initial brightness and color values by flooding updates
	if (m_first_update) {
		/*dstream<<"First update with time_of_day="<<time_of_day
				<<" time_brightness="<<time_brightness
				<<" direct_brightness="<<direct_brightness
				<<" sunlight_seen="<<sunlight_seen<<std::endl;*/
		m_first_update = false;
		for (u32 i = 0; i < 100; i++) {
			update(time_of_day, time_brightness, direct_brightness,
					sunlight_seen, cam_mode, yaw, pitch);
		}
		return;
	}

	m_time_of_day = time_of_day;
	m_time_brightness = time_brightness;
	m_sunlight_seen = sunlight_seen;
	m_in_clouds = false;

	bool is_dawn = (time_brightness >= 0.20 && time_brightness < 0.35);

	video::SColorf bgcolor_bright_normal_f = m_sky_params.sky_color.day_horizon;
	video::SColorf bgcolor_bright_indoor_f = m_sky_params.sky_color.indoors;
	video::SColorf bgcolor_bright_dawn_f = m_sky_params.sky_color.dawn_horizon;
	video::SColorf bgcolor_bright_night_f = m_sky_params.sky_color.night_horizon;

	video::SColorf skycolor_bright_normal_f = m_sky_params.sky_color.day_sky;
	video::SColorf skycolor_bright_dawn_f = m_sky_params.sky_color.dawn_sky;
	video::SColorf skycolor_bright_night_f = m_sky_params.sky_color.night_sky;

	video::SColorf cloudcolor_bright_normal_f = m_cloudcolor_day_f;
	video::SColorf cloudcolor_bright_dawn_f = m_cloudcolor_dawn_f;

	float cloud_color_change_fraction = 0.95;
	if (sunlight_seen) {
		if (std::fabs(time_brightness - m_brightness) < 0.2f) {
			m_brightness = m_brightness * 0.95 + time_brightness * 0.05;
		} else {
			m_brightness = m_brightness * 0.80 + time_brightness * 0.20;
			cloud_color_change_fraction = 0.0;
		}
	} else {
		if (direct_brightness < m_brightness)
			m_brightness = m_brightness * 0.95 + direct_brightness * 0.05;
		else
			m_brightness = m_brightness * 0.98 + direct_brightness * 0.02;
	}

	m_clouds_visible = true;
	float color_change_fraction = 0.98f;
	if (sunlight_seen) {
		if (is_dawn) { // Dawn
			m_bgcolor_bright_f = m_bgcolor_bright_f.getInterpolated(
				bgcolor_bright_dawn_f, color_change_fraction);
			m_skycolor_bright_f = m_skycolor_bright_f.getInterpolated(
				skycolor_bright_dawn_f, color_change_fraction);
			m_cloudcolor_bright_f = m_cloudcolor_bright_f.getInterpolated(
				cloudcolor_bright_dawn_f, color_change_fraction);
		} else {
			if (time_brightness < 0.13f) { // Night
				m_bgcolor_bright_f = m_bgcolor_bright_f.getInterpolated(
					bgcolor_bright_night_f, color_change_fraction);
				m_skycolor_bright_f = m_skycolor_bright_f.getInterpolated(
					skycolor_bright_night_f, color_change_fraction);
			} else { // Day
				m_bgcolor_bright_f = m_bgcolor_bright_f.getInterpolated(
					bgcolor_bright_normal_f, color_change_fraction);
				m_skycolor_bright_f = m_skycolor_bright_f.getInterpolated(
					skycolor_bright_normal_f, color_change_fraction);
			}

			m_cloudcolor_bright_f = m_cloudcolor_bright_f.getInterpolated(
				cloudcolor_bright_normal_f, color_change_fraction);
		}
	} else {
		m_bgcolor_bright_f = m_bgcolor_bright_f.getInterpolated(
			bgcolor_bright_indoor_f, color_change_fraction);
		m_skycolor_bright_f = m_skycolor_bright_f.getInterpolated(
			bgcolor_bright_indoor_f, color_change_fraction);
		m_cloudcolor_bright_f = m_cloudcolor_bright_f.getInterpolated(
			cloudcolor_bright_normal_f, color_change_fraction);
		m_clouds_visible = false;
	}

	video::SColor bgcolor_bright = m_bgcolor_bright_f.toSColor();
	m_bgcolor = video::SColor(
		255,
		bgcolor_bright.getRed() * m_brightness,
		bgcolor_bright.getGreen() * m_brightness,
		bgcolor_bright.getBlue() * m_brightness
	);

	video::SColor skycolor_bright = m_skycolor_bright_f.toSColor();
	m_skycolor = video::SColor(
		255,
		skycolor_bright.getRed() * m_brightness,
		skycolor_bright.getGreen() * m_brightness,
		skycolor_bright.getBlue() * m_brightness
	);

	// Horizon coloring based on sun and moon direction during sunset and sunrise
	video::SColor pointcolor = video::SColor(m_bgcolor.getAlpha(), 255, 255, 255);


	float cloud_direct_brightness = 0.0f;
	if (sunlight_seen) {
		cloud_direct_brightness = std::fmin(m_horizon_blend() * 0.15f +
			m_time_brightness, 1.0f);
		// Set the same minimum cloud brightness at night
		if (time_brightness < 0.5f)
			cloud_direct_brightness = std::fmax(cloud_direct_brightness,
				time_brightness * 1.3f);
	} else {
		cloud_direct_brightness = direct_brightness;
	}

	m_cloud_brightness = m_cloud_brightness * cloud_color_change_fraction +
		cloud_direct_brightness * (1.0 - cloud_color_change_fraction);
	m_cloudcolor_f = video::SColorf(
		m_cloudcolor_bright_f.r * m_cloud_brightness,
		m_cloudcolor_bright_f.g * m_cloud_brightness,
		m_cloudcolor_bright_f.b * m_cloud_brightness,
		1.0
	);
	m_cloudcolor_f = m_mix_scolorf(m_cloudcolor_f,
		video::SColorf(pointcolor), m_horizon_blend() * 0.25);
}

static v3f getSkyBodyPosition(float horizon_position, float day_position, float orbit_tilt)
{
	v3f result = v3f(0, 0, -1);
	result.rotateXZBy(horizon_position);
	result.rotateXYBy(day_position);
	result.rotateYZBy(orbit_tilt);
	return result;
}

v3f Sky::getSunDirection()
{
	return getSkyBodyPosition(90, getWickedTimeOfDay(m_time_of_day) * 360 - 90, m_sky_params.body_orbit_tilt);
}

v3f Sky::getMoonDirection()
{
	return getSkyBodyPosition(270, getWickedTimeOfDay(m_time_of_day) * 360 - 90, m_sky_params.body_orbit_tilt);
}

void Sky::draw_sky_body(std::array<video::S3DVertex, 4> &vertices, float pos_1, float pos_2, const video::SColor &c)
{
	/*
	* Create an array of vertices with the dimensions specified.
	* pos_1, pos_2: position of the body's vertices
	* c: color of the body
	*/

	const f32 t = 1.0f;
	const f32 o = 0.0f;
	vertices[0] = video::S3DVertex(pos_1, pos_1, -1, 0, 0, 1, c, t, t);
	vertices[1] = video::S3DVertex(pos_2, pos_1, -1, 0, 0, 1, c, o, t);
	vertices[2] = video::S3DVertex(pos_2, pos_2, -1, 0, 0, 1, c, o, o);
	vertices[3] = video::S3DVertex(pos_1, pos_2, -1, 0, 0, 1, c, t, o);
}


void Sky::place_sky_body(
	std::array<video::S3DVertex, 4> &vertices, float horizon_position, float day_position)
	/*
	* Place body in the sky.
	* vertices: The body as a rectangle of 4 vertices
	* horizon_position: turn the body around the Y axis
	* day_position: turn the body around the Z axis, to place it depending of the time of the day
	*/
{
	v3f centrum = getSkyBodyPosition(horizon_position, day_position, m_sky_params.body_orbit_tilt);
	v3f untilted_centrum = getSkyBodyPosition(horizon_position, day_position, 0.f);
	for (video::S3DVertex &vertex : vertices) {
		// Body is directed to -Z (south) by default
		vertex.Pos.rotateXZBy(horizon_position);
		vertex.Pos.rotateXYBy(day_position);
		vertex.Pos += centrum - untilted_centrum;
	}
}

void Sky::setSkyColors(const SkyColor &sky_color)
{
	m_sky_params.sky_color = sky_color;
}

void Sky::setHorizonTint(video::SColor sun_tint, video::SColor moon_tint,
	const std::string &use_sun_tint)
{
	// Change sun and moon tinting:
	m_sky_params.fog_sun_tint = sun_tint;
	m_sky_params.fog_moon_tint = moon_tint;
	// Faster than comparing strings every rendering frame
	if (use_sun_tint == "default")
		m_default_tint = true;
	else if (use_sun_tint == "custom")
		m_default_tint = false;
	else
		m_default_tint = true;
}

void Sky::addTextureToSkybox(const std::string &texture, int material_id,
		ITextureSource *tsrc)
{
	// Sanity check for more than six textures.
	if (material_id + 5 >= SKY_MATERIAL_COUNT)
		return;
	// Keep a list of texture names handy.
	m_sky_params.textures.emplace_back(texture);
	video::ITexture *result = tsrc->getTextureForMesh(texture);
	m_materials[material_id+5] = baseMaterial();
	m_materials[material_id+5].setTexture(0, result);
	m_materials[material_id+5].MaterialType = video::EMT_SOLID;
}

float getWickedTimeOfDay(float time_of_day)
{
	float nightlength = 0.415f;
	float wn = nightlength / 2;
	float wicked_time_of_day = 0;
	if (time_of_day > wn && time_of_day < 1.0f - wn)
		wicked_time_of_day = (time_of_day - wn) / (1.0f - wn * 2) * 0.5f + 0.25f;
	else if (time_of_day < 0.5f)
		wicked_time_of_day = time_of_day / wn * 0.25f;
	else
		wicked_time_of_day = 1.0f - ((1.0f - time_of_day) / wn * 0.25f);
	return wicked_time_of_day;
}
