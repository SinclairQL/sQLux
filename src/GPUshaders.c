#include "GPUshaders.h"

#ifdef ENABLE_SHADERS
#include <SDL_gpu.h>
#include "SDL2screen.h"
#include "QL_screen.h"

/* Structure used in mouse pointer calculation */
typedef struct {
	float x;
	float y;
} vec2;

/* Curvature for screen */
#define CURVATURE_X 0.05
#define CURVATURE_Y 0.12

static uint32_t* screen_buffer = NULL;
static GPU_Image* image = 0;
static GPU_Target* screen = 0;

static SDL_Rect screen_rect;

static Uint32 shader;
static GPU_ShaderBlock shader_block;
static int res_texture_size;
static int res_screen_size;
static bool curve = false;

static void UpdateDisplay(GPU_Target* screen);
static void setViewPort(void);
static void CreateImage(void);
static void CreatePalette(void);

static Uint32 LoadShader(GPU_ShaderEnum shader_type, const char* filename, const char* prepend);
static bool LoadShaderProgram(GPU_ShaderBlock* shader, Uint32* p, const char* vertex_shader_file,
                                const char* fragment_shader_file, const char* prepend);
static void UpdateShader(float x, float y, float a, float b);
static void FreeShader(Uint32 p);
static void Distort(float* x, float* y);

/* Initialise the display and set up the shaders */
bool QLGPUCreateDisplay(int w , int h, int ly, uint32_t* id,
			const char* name, uint32_t sdl_window_mode,
			int shader_type, const char* shader_path)
{
	bool ret = false;
	screen = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS | sdl_window_mode);

	if (screen != NULL) {
		// Obtain the window id
		*id = screen->context->windowID;
		SDL_Window* window = SDL_GetWindowFromID(*id);

		// Set the windows title
		SDL_SetWindowTitle(window, name);

		// Set the icon
		QLSDLCreateIcon(window);
		CreateImage();
		CreatePalette();

		// Configure the shaders
		char* prepend = NULL;

		if (shader_type == 2) {
			prepend = malloc(100);
			if (prepend) {
				sprintf(prepend,
				        "#define CURVATURE\n#define CURVATURE_X %4.2f\n#define CURVATURE_Y %4.2f\n",
					CURVATURE_X, CURVATURE_Y);
				printf("%s", prepend);
				curve = true;
			}
		}

		ret = LoadShaderProgram(&shader_block, &shader, shader_path,
					shader_path, prepend);

		if (prepend)
			free(prepend);

		if (ret) {
			res_texture_size = GPU_GetUniformLocation(shader, "u_tex0Resolution");
			res_screen_size = GPU_GetUniformLocation(shader, "u_resolution");
		}
	}
	return ret;
}

/* Tidy up the memory and resources at shut down */
void QLGPUClean(void) {

	if (image)
	        GPU_FreeImage(image);

	free(screen_buffer);
	GPU_Quit();
}

/* Update the display using sdl_gpu */
void QLGPUUpdateDisplay(void)
{
	// Update the display memory
    	QLSDLWritePixels(screen_buffer);

	// Update the image using the updated memory buffer
    	GPU_UpdateImageBytes(image, NULL, (unsigned char*)screen_buffer, qlscreen.xres * 4);

	// Ensure the aspect ratio of the display is maintained
    	setViewPort();

    	// Render to screen, using the active shader
	GPU_Clear(screen);
	GPU_ActivateShaderProgram(shader, &shader_block);
	UpdateShader((float)qlscreen.xres, (float)qlscreen.yres, (float)screen->base_w, (float)screen->base_h);
	GPU_BlitRect(image, NULL, screen, NULL);
	GPU_ActivateShaderProgram(0, NULL);
	GPU_Flip(screen);
}

/* sdl_gpu requires the window resolution set after a change in window size */
void QLGPUSetSize(int w, int h)
{
	GPU_SetWindowResolution(w, h);
}

/* Convert the mouse coordinates into QL screen coordinates */
void QLGPUProcessMouse(int* qlx, int* qly, int x, int y)
{
	*qlx = x - screen_rect.x;
	*qly = y - screen_rect.y;
	*qlx = (*qlx * qlscreen.xres + 0.5) / screen_rect.w;
	*qly = (*qly * qlscreen.yres + 0.5) / screen_rect.h;

	if (curve) {
		// Convert to range 0 to 1
		float fx = (float)(*qlx) / (float)(qlscreen.xres);
		float fy = (float)(*qly) / (float)(qlscreen.yres);

		Distort(&fx, &fy);

		*qlx = fx * qlscreen.xres;
		*qly = fy * qlscreen.yres;
	}

	// Make sure in range
	*qlx = (*qlx > 0) ? *qlx : 0;
	*qlx = (*qlx < qlscreen.xres) ? *qlx : qlscreen.xres - 1;
	*qly = (*qly > 0) ? *qly : 0;
	*qly = (*qly < qlscreen.yres) ? *qly : qlscreen.yres - 1;

	//printf("x: %i y: %i qx: %i qy: %i\n", x, y, *qlx, *qly);
}

/* Move to and from fullscreen mode */
void QLGPUSetFullscreen(void)
{
	GPU_SetFullscreen(ql_fullscreen, true);

	if (!ql_fullscreen) {
		// Need to set the window resolution after
		// return from fullscreen
		GPU_SetWindowResolution(screen->w, screen->h);
	}
}

/*
 * Static functions
 */

/* create a surface and memory buffer for the display */
static void CreateImage(void)
{
    	// Create an image and surface
    	if (image)
        	GPU_FreeImage(image);
	if (!screen_buffer)
        	screen_buffer = (uint32_t*)malloc(qlscreen.xres * qlscreen.yres * 4);

	image = GPU_CreateImage(qlscreen.xres, qlscreen.yres, GPU_FORMAT_RGBA);
}

/* Create the colour or grey palette of QL colours */
static void CreatePalette(void)
{
	SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
	QLSDLCreatePalette(format);
	SDL_FreeFormat(format);
}

/* Ensure that the screen aspect ratio is preserved */
static void setViewPort(void)
{
	static int width = -1;
	static int height = -1;
	int w,h;
	GPU_Rect frect;

	h = screen->base_h;
	w = screen->base_w;

	if ((w != width) || (height != h)) {
		width = w;
		height = h;

		printf("Width: %i Height: %i Ratio: 6.3f %s\n", w, h,
		        ql_screen_ratio, ql_fullscreen ? "fullscreen" : "not fullscreen");

		if (ql_fullscreen) {
			printf("setViewPort: Full screen\n");
			// Largest integer pixel height
			// note deliberately ignore larger screen heights
			screen_rect.h = (h / 256) * 256;
			screen_rect.w = (int)(ql_screen_ratio * (float)screen_rect.h);
			screen_rect.x = (w - screen_rect.w) / 2;
			screen_rect.y = (h - screen_rect.h) / 2;
		}
		else {
			if (fabs((float)w - (2.0 * (float)h) / ql_screen_ratio) < 3.0) {
				printf("setViewPort: fit\n");
				screen_rect.h = h;
				screen_rect.w = w;
				screen_rect.x = 0;
				screen_rect.y = 0;
			}
			else if ((2.0 * (float)h) / ql_screen_ratio < (float)w) {
				printf("setViewPort: scale to h\n");
				screen_rect.h = h;
				screen_rect.w = (int)((2.0 * (float)h) / ql_screen_ratio);
				screen_rect.x = (w - screen_rect.w) / 2;
				screen_rect.y = 0;
			}
			else {
				printf("setViewPort: scale to w\n");
				screen_rect.w = w;
				screen_rect.h = (int)((float)w * ql_screen_ratio / 2.0);
				screen_rect.x = 0;
				screen_rect.y = (h - screen_rect.h) / 2;
			}
		}
		frect.x = (float)screen_rect.x;
		frect.y = (float)screen_rect.y;
		frect.w = (float)screen_rect.w;
		frect.h = (float)screen_rect.h;

		printf("x:%6.1f y:%6.1f w:%6.1f h:%6.1f\n",frect.x, frect.y, frect.w, frect.h);
		GPU_SetViewport(screen, frect);
	}
}

/* Mimic the distortion in the shader, to calculate mouse position */
static void Distort(float* x, float* y)
{
	vec2 coord;
	coord.x = *x;
	coord.y = *y;

	vec2 curvature_distortion;
	curvature_distortion.x = CURVATURE_X;
	curvature_distortion.y = CURVATURE_Y;

	vec2 barrelScale;
	barrelScale.x = 1.0 - 0.23 * curvature_distortion.x;
	barrelScale.y = 1.0 - 0.23 * curvature_distortion.y;

	coord.x -= 0.5;
	coord.y -= 0.5;

	float rsq = coord.x * coord.x + coord.y * coord.y;

	coord.x += coord.x * curvature_distortion.x * rsq;
	coord.y += coord.y * curvature_distortion.y * rsq;

	coord.x *= barrelScale.x;
	coord.y *= barrelScale.y;

	coord.x += 0.5;
	coord.y += 0.5;

	*x = coord.x;
	*y = coord.y;
}

// Loads a shader and prepends version/compatibility info before compiling it.
// Normally, GPU_LoadShader() can be used for shader source files and GPU_CompileShader() for strings.
// However, some hardware (certain ATI/AMD cards) does not support  non-#version preprocessing at the top of the file.
// This prepends the version info so that both GLSL and GLSLES can be supported with one shader file.
static Uint32 LoadShader(GPU_ShaderEnum shader_type, const char* filename, const char* prepend)
{
	SDL_RWops* rwops;
	Uint32 shader;
	char* source;
	int header_size, directive_size, file_size;
	int prepend_size = 0;
	const char* header = "";
	const char* directive = "";
	GPU_Renderer* renderer = GPU_GetCurrentRenderer();

	// Open file
	rwops = SDL_RWFromFile(filename, "rb");
	if (rwops == NULL) {
		return 0;
	}

	// Get file size
	file_size = (int)SDL_RWseek(rwops, 0, SEEK_END);
	SDL_RWseek(rwops, 0, SEEK_SET);

	// Get size from header
	if (renderer->shader_language == GPU_LANGUAGE_GLSL) {
		printf("Shader Language: GPU_LANGUAGE_GLSL\n");
		if (renderer->max_shader_version >= 120)
			header = "#version 120\n";
		else
			header = "#version 110\n";
	}
	else if (renderer->shader_language == GPU_LANGUAGE_GLSLES) {
		printf("Shader Language: GPU_LANGUAGE_GLSLES\n");
		header = "#version 100\nprecision mediump int;\nprecision mediump float;\n";
	}
	else {
		printf("Shader Language Enum: %i\n", renderer->shader_language);
	}

	printf("Shader Version %i\n", renderer->max_shader_version);

	header_size = (int)strlen(header);

	// Add a preprocessor directive
	if (shader_type == GPU_VERTEX_SHADER) {
		directive = "#define VERTEX\n";
	} else if (shader_type == GPU_FRAGMENT_SHADER) {
		directive = "#define FRAGMENT\n";
	}
	directive_size = (int)strlen(directive);

	if (prepend) {
		prepend_size = (int)strlen(prepend);
	}

	int pre_source_size = header_size + directive_size + prepend_size;

	int source_size = (int)(sizeof(char)) * (pre_source_size + file_size + 1);

	// Allocate source buffer
	source = (char*)malloc(source_size);

	if (source == NULL) {
		GPU_LogError("malloc failed\n");
		return 0;
	}

	// Prepend header
	strcpy(source, header);
	strcpy(source + header_size, directive);

	if (prepend_size) {
		strcpy(source + header_size + directive_size, prepend);
	}

	// Read in source code
	SDL_RWread(rwops, source + pre_source_size, 1, file_size);
	source[pre_source_size + file_size] = '\0';

	// Compile the shader
	shader = GPU_CompileShader(shader_type, source);

	// Clean up
	free(source);
	SDL_RWclose(rwops);

	return shader;
}

static bool LoadShaderProgram(GPU_ShaderBlock* shader, Uint32* p, const char* vertex_shader_file,
                                const char* fragment_shader_file, const char* prepend)
{
	bool ret = false;
	Uint32 v, f;
	v = LoadShader(GPU_VERTEX_SHADER, vertex_shader_file, prepend);

	if (!v) {
		GPU_LogError("Failed to load vertex shader (%s): %s\n", vertex_shader_file, GPU_GetShaderMessage());
		return false;
	}

	f = LoadShader(GPU_FRAGMENT_SHADER, fragment_shader_file, prepend);

	if (!f) {
		GPU_LogError("Failed to load fragment shader (%s): %s\n", fragment_shader_file, GPU_GetShaderMessage());
		return false;
	}

	*p = GPU_LinkShaders(v, f);

	if (!*p) {
		GPU_LogError("Failed to link shader program (%s + %s): %s\n", vertex_shader_file, fragment_shader_file, GPU_GetShaderMessage());
		return false;
	}

	*shader = GPU_LoadShaderBlock(*p, "gpu_Vertex", "gpu_TexCoord", "gpu_Color", "gpu_ModelViewProjectionMatrix");
	GPU_ActivateShaderProgram(*p, shader);
	return true;
}

static void FreeShader(Uint32 p)
{
	GPU_FreeShaderProgram(p);
}

static void UpdateShader(float x, float y, float a, float b)
{
	float fres[2] = { x, y };
	GPU_SetUniformfv(res_texture_size, 2, 1, fres);
	fres[0] = a;
	fres[1] = b;
	GPU_SetUniformfv(res_screen_size, 2, 1, fres);
}

#else
/* Empry functions, for when shaders not enabled */
bool QLGPUCreateDisplay(int w , int h, int ly, uint32_t* id,
			const char* name, uint32_t sdl_window_mode,
			int shader_type, const char* shader_path) {
	return true;
}
void QLGPUUpdateDisplay(void) {}
void QLGPUSetFullscreen(void) {}
void QLGPUSetSize(int w, int h) {}
void QLGPUProcessMouse(int* qlx, int* qly, int x, int y) {}
void QLGPUClean(void) {}
#endif
