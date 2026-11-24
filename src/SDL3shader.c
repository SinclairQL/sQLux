/*
 * CRT shader: a fragment shader of the SDL3 GPU renderer (SDL 3.4 or later),
 * applied when the QL screen texture is drawn. Source and compiled versions
 * are in src/shaders.
 *
 * SHADER option: bits 0-1 shape (1 flat, 2 curved, 0 colour change only),
 * bits 6-7 colour (64 grey, 128 amber, 192 green).
 */

#include <SDL3/SDL.h>
#include <stdio.h>

#include "debug.h"
#include "SDL3shader.h"

#define CURVATURE_X 0.06f
#define CURVATURE_Y 0.12f

static float curve_x, curve_y;  // 0 for a flat screen

#if SDL_VERSION_ATLEAST(3, 4, 0)

#include "shaders/crt-pi.frag.spv.h"
#include "shaders/crt-pi.frag.msl.h"
#if __has_include("shaders/crt-pi.frag.dxil.h")
#include "shaders/crt-pi.frag.dxil.h"
#define HAVE_DXIL 1
#endif

/* Must match the Context block in src/shaders/crt-pi.frag */
struct crt_uniforms {
	float texture_size[2];
	float output_size[2];
	float curvature[2];
	float mono_mode;
	float colour_only;
};

static SDL_GPUDevice *device;
static SDL_GPUShader *shader;
static SDL_GPURenderState *state;
static struct crt_uniforms uniforms;

static bool QLShaderCreateState(SDL_Renderer *renderer)
{
	SDL_GPUShaderCreateInfo info;
	SDL_GPURenderStateCreateInfo createinfo;
	SDL_GPUShaderFormat formats;

	device = SDL_GetGPURendererDevice(renderer);
	if (!device)
		return false;

	formats = SDL_GetGPUShaderFormats(device);

	SDL_zero(info);
	if (formats & SDL_GPU_SHADERFORMAT_SPIRV) {
		info.format = SDL_GPU_SHADERFORMAT_SPIRV;
		info.code = crt_pi_frag_spv;
		info.code_size = crt_pi_frag_spv_len;
		info.entrypoint = "main";
#ifdef HAVE_DXIL
	} else if (formats & SDL_GPU_SHADERFORMAT_DXIL) {
		info.format = SDL_GPU_SHADERFORMAT_DXIL;
		info.code = crt_pi_frag_dxil;
		info.code_size = crt_pi_frag_dxil_len;
		info.entrypoint = "main";
#endif
	} else if (formats & SDL_GPU_SHADERFORMAT_MSL) {
		info.format = SDL_GPU_SHADERFORMAT_MSL;
		info.code = crt_pi_frag_msl;
		info.code_size = crt_pi_frag_msl_len;
		info.entrypoint = "main0";
	} else {
		SDL_SetError("no supported shader format");
		return false;
	}
	info.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	info.num_samplers = 1;
	info.num_uniform_buffers = 1;

	shader = SDL_CreateGPUShader(device, &info);
	if (!shader)
		return false;

	SDL_zero(createinfo);
	createinfo.fragment_shader = shader;
	state = SDL_CreateGPURenderState(renderer, &createinfo);

	return state != NULL;
}

SDL_Renderer *QLShaderCreateRenderer(SDL_Window *window, int option)
{
	SDL_Renderer *renderer;
	SDL_PropertiesID props;
	int shape = option & 3;
	int colour = (option >> 6) & 3;

	if (!shape && !colour)
		return NULL;

	props = SDL_CreateProperties();
	SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING,
			      SDL_GPU_RENDERER);
	SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER,
			       window);
	SDL_SetBooleanProperty(
		props, SDL_PROP_RENDERER_CREATE_GPU_SHADERS_SPIRV_BOOLEAN, true);
	SDL_SetBooleanProperty(
		props, SDL_PROP_RENDERER_CREATE_GPU_SHADERS_MSL_BOOLEAN, true);
#ifdef HAVE_DXIL
	SDL_SetBooleanProperty(
		props, SDL_PROP_RENDERER_CREATE_GPU_SHADERS_DXIL_BOOLEAN, true);
#endif
	renderer = SDL_CreateRendererWithProperties(props);
	SDL_DestroyProperties(props);

	if (renderer && !QLShaderCreateState(renderer)) {
		printf("CRT shader not available: %s\n", SDL_GetError());
		QLShaderQuit();
		SDL_DestroyRenderer(renderer);
		return NULL;
	}
	if (!renderer) {
		printf("GPU renderer for the CRT shader not available: %s\n",
		       SDL_GetError());
		return NULL;
	}

	if (shape == 2) {
		curve_x = CURVATURE_X;
		curve_y = CURVATURE_Y;
	}
	uniforms.curvature[0] = curve_x;
	uniforms.curvature[1] = curve_y;
	uniforms.mono_mode = (float)colour;
	uniforms.colour_only = (shape == 0) ? 1.0f : 0.0f;

	if (V1)
		printf("Using the CRT shader (%s)\n", SDL_GetGPUDeviceDriver(device));

	return renderer;
}

void QLShaderBegin(SDL_Renderer *renderer, float tex_w, float tex_h)
{
	SDL_FRect out;

	if (!state)
		return;

	// The size the screen is drawn at, in window pixels
	SDL_GetRenderLogicalPresentationRect(renderer, &out);
	uniforms.texture_size[0] = tex_w;
	uniforms.texture_size[1] = tex_h;
	uniforms.output_size[0] = out.w;
	uniforms.output_size[1] = out.h;
	SDL_SetGPURenderStateFragmentUniforms(state, 0, &uniforms,
					      sizeof(uniforms));
	SDL_SetGPURenderState(renderer, state);
}

void QLShaderEnd(SDL_Renderer *renderer)
{
	if (state)
		SDL_SetGPURenderState(renderer, NULL);
}

void QLShaderQuit(void)
{
	if (state)
		SDL_DestroyGPURenderState(state);
	if (shader)
		SDL_ReleaseGPUShader(device, shader);
	state = NULL;
	shader = NULL;
	device = NULL;
}

#else

SDL_Renderer *QLShaderCreateRenderer(SDL_Window *window, int option)
{
	if (option)
		printf("SHADER needs sQLux built with SDL 3.4 or later\n");
	return NULL;
}

void QLShaderBegin(SDL_Renderer *renderer, float tex_w, float tex_h)
{
}

void QLShaderEnd(SDL_Renderer *renderer)
{
}

void QLShaderQuit(void)
{
}

#endif

/*
 * The distortion of the curved screen, as in the shader: from a position
 * in the window (0 to 1) to the position in the QL screen. Used for the
 * mouse.
 */
void QLShaderDistort(float *x, float *y)
{
	float cx, cy, rsq;

	if (curve_x == 0.0f && curve_y == 0.0f)
		return;

	cx = *x - 0.5f;
	cy = *y - 0.5f;
	rsq = cx * cx + cy * cy;
	cx += cx * curve_x * rsq;
	cy += cy * curve_y * rsq;
	cx *= 1.0f - 0.23f * curve_x;
	cy *= 1.0f - 0.23f * curve_y;
	*x = cx + 0.5f;
	*y = cy + 0.5f;
}
