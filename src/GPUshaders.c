/*
 * Copyright (c) 2020-2022 Graeme Gregory, Ian Jordan
 *
 */

#include "GPUshaders.h"

#ifdef ENABLE_SHADERS
#include <string.h>
#include <SDL_gpu.h>
#include "SDL2screen.h"
#include "QL_screen.h"
#include "debug.h"

/* Structure used in mouse pointer calculation */
typedef struct {
	float x;
	float y;
} vec2;

static uint32_t* screen_buffer = NULL;
static GPU_Image* image = 0;
static GPU_Target* screen = 0;

static SDL_Rect screen_rect;		// Used for mouse calculations
static GPU_Rect frect;			// Used for blit calculations

static double pixel_ratio = 1.0;	// Ratio of x to y pixels
					// e.g. 512x256 = 2.0; 800x600 = 1.333

static Uint32 shader;
static GPU_ShaderBlock shader_block;
static int res_texture_size;
static int res_screen_size;
static bool curve = false;
static float curve_x;
static float curve_y;

static void UpdateDisplay(GPU_Target* screen);
static void setViewPort(int w, int h);
static void CreateImage(void);
static void CreatePalette(void);

static Uint32 LoadShader(GPU_ShaderEnum shader_type, const char* data,
        		int data_size, const char* prepend);
static bool LoadShaderProgram(GPU_ShaderBlock* shader, Uint32* p, const char* shader_file,
                              	const char* prepend, bool curve, float* curve_x, float* curve_y);
static void UpdateShader(float x, float y, float a, float b);
static void FreeShader(Uint32 p);
static void Distort(float* x, float* y);
static void ReadCurve(char* data, float* x, float* y);

/* Initialise the display and set up the shaders */
bool QLGPUCreateDisplay(int w , int h, int ly, uint32_t* id,
			const char* name, uint32_t sdl_window_mode,
			int shader_type, const char* shader_path)
{
	bool ret = false;
	screen = GPU_Init(w, h, GPU_DEFAULT_INIT_FLAGS | sdl_window_mode);

	pixel_ratio = (double)qlscreen.xres / (double)qlscreen.yres;
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
			prepend = "#define CURVATURE\n";
			curve = true;
		}

		ret = LoadShaderProgram(&shader_block, &shader,
					shader_path, prepend,
					curve, &curve_x, &curve_y);

		if (ret) {
			res_texture_size = GPU_GetUniformLocation(shader, "TextureSize");
			res_screen_size = GPU_GetUniformLocation(shader, "u_resolution");
		}
		// Set initial image size
		setViewPort(w, h);
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

	// Render to screen, using the active shader
	GPU_Clear(screen);
	GPU_ActivateShaderProgram(shader, &shader_block);
	UpdateShader((float)qlscreen.xres, (float)qlscreen.yres,
		(float)frect.w, (float)frect.h);
	GPU_BlitRect(image, NULL, screen, &frect);
	GPU_ActivateShaderProgram(0, NULL);
	GPU_Flip(screen);
}

/* sdl_gpu requires the window resolution set after a change in window size */
void QLGPUSetSize(int w, int h)
{
	GPU_SetWindowResolution(w, h);
	// Ensure the aspect ratio of the display is maintained
	setViewPort(w, h);
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
		QLGPUSetSize(screen->w, screen->h);
	}
}

/*
 * Internal functions
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
static void setViewPort(int w, int h)
{
	static int width = -1;
	static int height = -1;

	if ((width != w) || (height != h)) {
		width = w;
		height = h;

#ifdef INTEGER_SCALING
		/*
		   This option is to improve the definition of scan lines when using a
		   BBQL size display in full screen mode
		*/
		if ((ql_fullscreen) && (qlscreen.yres == 256)) {
			// Largest integer pixel height that is divisable by 256
			int max_height = (int)((float)width * ql_screen_ratio / pixel_ratio);

			screen_rect.h = (max_height < height) ? max_height : height;
			screen_rect.h = (screen_rect.h / 256) * 256;
			screen_rect.w = (int)((pixel_ratio * (float)screen_rect.h) / ql_screen_ratio);
			screen_rect.x = (width - screen_rect.w) / 2;
			screen_rect.y = (height - screen_rect.h) / 2;
			//printf("x: %i y: %i w %i h:%i\n",
			//	screen_rect.x, screen_rect.y,
			//	screen_rect.w, screen_rect.h);

		}
		else
#endif
		{
			if (fabs((float)width - (pixel_ratio * (float)height) / ql_screen_ratio) < 3.0) {
				screen_rect.h = height;
				screen_rect.w = width;
				screen_rect.x = 0;
				screen_rect.y = 0;
			}
			else if ((float)width > (pixel_ratio * (float)height) / ql_screen_ratio) {
				screen_rect.h = height;
				screen_rect.w = (int)((pixel_ratio * (float)height) / ql_screen_ratio);
				screen_rect.x = (width - screen_rect.w) / 2;
				screen_rect.y = 0;
			}
			else {
				screen_rect.w = width;
				screen_rect.h = (int)((float)width * ql_screen_ratio / pixel_ratio);
				screen_rect.x = 0;
				screen_rect.y = (height - screen_rect.h) / 2;
			}
		}
		frect.x = (float)screen_rect.x;
		frect.y = (float)screen_rect.y;
		frect.w = (float)screen_rect.w;
		frect.h = (float)screen_rect.h;
	}
}

/* Mimic the distortion in the shader, to calculate mouse position */
static void Distort(float* x, float* y)
{
	vec2 coord;
	coord.x = *x;
	coord.y = *y;

	vec2 curvature_distortion;
	curvature_distortion.x = curve_x;
	curvature_distortion.y = curve_y;

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

/*
   Loads a shader and prepends version/compatibility info before compiling it.
   This function also prepends version specific information to the shader file.
   This is needed as some shader compilers (older AMD) do not do this correctly
*/
static Uint32 LoadShader(GPU_ShaderEnum shader_type, const char* data,
			int data_size, const char* prepend)
{
	Uint32 shader;
	char* source;
	int header_size, directive_size;
	int prepend_size = 0;
	char header[] = "#version 100\nprecision mediump int;\nprecision mediump float;\n";
	const char* directive = "";
	GPU_Renderer* renderer = GPU_GetCurrentRenderer();

	if (renderer->shader_language == GPU_LANGUAGE_GLSL) {
		if (V2 && shader_type == GPU_VERTEX_SHADER)
			printf("GLS shader\n");
		/* Aim to use GLSL version 120 (OpenGL 2.1), but ensure that a
		   supported version is used */
		if (renderer->min_shader_version >= 120)
			sprintf(header, "#version %i\n", renderer->min_shader_version);
		else if (renderer->max_shader_version >= 120)
			strcpy(header, "#version 120\n");
		else
			strcpy(header, "#version 110\n");
	}
	else if (renderer->shader_language == GPU_LANGUAGE_GLSLES) {
		/* Always use GLSL ES version 100 (OpenGL ES 2.0), which is
		   based on GLS version 120 (OpenGL 2.1) */
		if (V2 && shader_type == GPU_VERTEX_SHADER)
			printf("GLS ES shader\n");
	}

	if (V2 && shader_type == GPU_VERTEX_SHADER) {
		printf("Min shader version: %i, Max shader version: %i\n",
			renderer->min_shader_version,
			renderer->max_shader_version);
		printf("Shader header text: %s", header);
	}

	header_size = (int)strlen(header);

	// Add a preprocessor directive
	if (shader_type == GPU_VERTEX_SHADER) {
		directive = "#define VERTEX\n";
	} else if (shader_type == GPU_FRAGMENT_SHADER) {
		directive = "#define FRAGMENT\n";
	}
	directive_size = (int)strlen(directive);
	if (prepend)
		prepend_size = (int)strlen(prepend);

	int pre_source_size = header_size + directive_size + prepend_size;

	int source_size = (int)(sizeof(char)) * (pre_source_size + data_size + 1);

	// Allocate source buffer
	source = (char*)malloc(source_size);

	if (source == NULL) {
		GPU_LogError("malloc failed\n");
		return 0;
	}

	// Prepend header
	strcpy(source, header);
	strcpy(source + header_size, directive);

	if (prepend_size)
		strcpy(source + header_size + directive_size, prepend);

	// Copy the file contents
	memcpy(source + pre_source_size, data, data_size);
	source[pre_source_size + data_size] = '\0';

	// Compile the shader
	shader = GPU_CompileShader(shader_type, source);

	// Clean up
	free(source);

	return shader;
}

/* Reads curve data from shader program file */
static void ReadCurve(char* data, float* x, float* y) {

	*x = 1e8;
	*y = 1e8;

	// Walk through tokens
	const char delimiters[] = " \t\n";
   	char* token;
	bool define = false;

   	token = strtok(data, delimiters);

   	while( token != NULL ) {
		if (define) {
			if (!strcmp(token, "CURVATURE_X")) {
      			token = strtok(NULL, delimiters);

				if (token)
					*x = strtof(token, NULL);
			}
			else if (!strcmp(token, "CURVATURE_Y"))	{
      			token = strtok(NULL, delimiters);

				if (token)
					*y = strtof(token, NULL);
			}
		}
		define = !strcmp(token, "#define");
		if (token)
      			token = strtok(NULL, delimiters);
   	}

	if (*x == 1e8 || *y == 1e8) {
		printf("Cannot read curve data\n");
		*x = 1.0;
		*y = 1.0;
	}
}

/*
   Loads shader, compiles once with vertex and once with fragment defines.
   Links results and optionally extracts and returns curvature defines
*/
static bool LoadShaderProgram(GPU_ShaderBlock* shader, Uint32* p, const char* shader_file,
                              const char* prepend, bool curve, float* curve_x, float* curve_y)
{
	bool ret = false;
	Uint32 v, f;
	SDL_RWops* rwops;
	char* source;
	int file_size;

	// Allocate memory and read in data
	rwops = SDL_RWFromFile(shader_file, "rb");
	if (rwops == NULL) {
		GPU_LogError("Cannot open shader file %s\n", shader_file);
		return false;
	}

	// Get file size
	file_size = (int)SDL_RWseek(rwops, 0, SEEK_END);
	SDL_RWseek(rwops, 0, SEEK_SET);

	// Allocate source buffer
	source = (char*)malloc(file_size + 1);

	if (source == NULL) {
		GPU_LogError("malloc failed\n");
		return false;
	}

	// Read in source code
	SDL_RWread(rwops, source, 1, file_size);
	source[file_size] = '\0';

	v = LoadShader(GPU_VERTEX_SHADER, source, file_size, prepend);

	if (!v) {
		GPU_LogError("Failed to load vertex shader (%s): %s\n", shader_file, GPU_GetShaderMessage());
		free(source);
		return false;
	}

	f = LoadShader(GPU_FRAGMENT_SHADER, source, file_size, prepend);

	if (!f) {
		GPU_LogError("Failed to load fragment shader (%s): %s\n", shader_file, GPU_GetShaderMessage());
		free(source);
		return false;
	}

	*p = GPU_LinkShaders(v, f);

	if (!*p) {
		GPU_LogError("Failed to link shader program (%s): %s\n", shader_file, GPU_GetShaderMessage());
		free(source);
		return false;
	}

	// Read the curvature variables
	if (curve && curve_x && curve_y) {
		ReadCurve(source, curve_x, curve_y);
	}

	*shader = GPU_LoadShaderBlock(*p, "VertexCoord", "TexCoord", "gl_Color", "MVPMatrix");
	GPU_ActivateShaderProgram(*p, shader);
	free(source);
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
/*
   Empty functions, to allow linkage when ENABLE_SHADERS is not selected.
   In reality these functions will never be called, as a check is always made
   against shaders_selected before calling these functions, and
   shaders_selected will always be false if ENABLE_SHADERS is not defined
*/
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
