# sQLux Shader support

## Introduction
sQLux optionally supports OpenGL Shader Language (GLSL) shaders. These shaders run on the GPU and introduce display artifacts similar to those seen when using a BBQL with a 1980s CRT monitor. Artifacts that can be emulated include:
+ Scan lines
+ Shadow mask
+ Bloom
+ Screen curvature

Support for shaders has to be enabled at run time. OpenGL or OpenGL ES libraries must to be installed. Shader support is realised through the use of [sdl-gpu](https://github.com/grimfang4/sdl-gpu) which is statically linked into sQLux. The code for sdl-gpu is included in the sQLux repository.  
As shaders run on the GPU, the speed of the emulator is not impacted.

---
# Building
In all cases, first follow the steps to build sQLux without shader support, as described in the [README](../README.md). Then update submodules

```
git submodule update --init --recursive
```

## Building linux
```
cd build
cmake -DSUPPORT_SHADERS=TRUE ..
make
```
## Building MinGW on Linux
```
cd mingw
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32 -DSUPPORT_SHADERS=TRUE ..
make
```
## Building MinGW on Windows
```
cd mingw
cmake.exe -G "MinGW Makefiles" -DSUPPORT_SHADERS=TRUE ..
mingw32-make
```
# Shader parameters in sqlux.ini
`SHADER`  Selects shader support. 0 disables shaders. 1 enables a "flat" shader. 2 enables a shader including emulated barrel distortion. Disabled by default.

```
SHADER = 2
```

`SHADER_FILE`
The path to the GPU shader (written in OpenGL Shading Language) that is loaded when `SHADER` is set to 1 or 2.

```
SHADER_FILE = ../shaders/crt-pi.glsl
```
# Configuration
## The supplied shader
The supplied shader `crt-pi.glsl` works "out of the box". However, optionally, it can be edited to modify the following attributes:
### `MASK_TYPE`
The type of emulated CRT shadow mask.  
0 = none, 1 = standard mask, 2 = trinitron like. Default = 1
### `MASK_BRIGHTNESS`
The relative brightness of the emulated shadow mask. Range 0.0 to 1.0. Smaller number = darker. Default = 0.7.  
A darker setting, with curvature enabled, may produce Moiré effects.
### `SCANLINE_WEIGHT`
How wide scanlines are (it is an inverse value, higher number = thinner lines). Default = 5.0
### `SCANLINE_GAP_BRIGHTNESS`
How dark the gaps between the scan lines are. Range 0.0 to 1.0. Smaller number = darker. Default = 0.6  
Darker gaps between scan lines make Moiré effects more likely.
### `BLOOM_FACTOR`
The increase in width for bright scanlines. Default = 1.5
### `INPUT_GAMMA`
Input value for gamma correction. Applied prior to calculating other effects. Default = 2.4
### `OUTPUT_GAMMA`
Output value for gamma correction. Inverse applied after calculating other effects. Default = 2.2
### `SHARPER`
By default the shader uses linear blending horizontally. If this is too blurry, enable `SHARPER`
### `CURVATURE_X`
Simulated curvature to be applied in the x dimension, if `SHADER = 2` in `sqlux.ini`. Default = 0.06
### `CURVATURE_Y`
Simulated curvature to be applied in the y dimension, if `SHADER = 2` in `sqlux.ini`. Default = 0.12
## Older Raspberry Pis
By default, when using the Raspberry Pi OS Bullseye edition, hardware graphics acceleration may not be enabled for Pi0 to Pi3. This can impact shader performance.

To enable hardware acceleration, run the configuration tool:

`sudo raspi-config`

Select the options to enable glamor (A8) and GL Driver (A2), under the advanced options sub-menu, and reboot.

**Note:** Hardware graphics acceleration *should* be enabled by default for the Pi4, so this change should *not* be needed for the Pi4.

# Creating your own shader
Examples of shaders which can be modified to use within sQLux can be found at e.g. [libretro shaders](https://github.com/libretro/glsl-shaders/tree/master/crt/shaders).  
A single file is used for both the vertex and fragment shader. It is compiled twice to generate the two shader types, once with `#define VERTEX` prepended to the start of the file, and once with `#define FRAGMENT`  
MacOS no longer supports versions of shader files prior to GLSL version 1.5. (OpenGL 3.2). Many Raspberry Pi models do not support versions of shader files later then GLSL version 1.2 (or the equivalent GLSL ES version 1.0). Therefore compatibility `#defines` are needed to ensure one shader file can support multiple targets. See `crt-pi.glsl` for an example.  
sQLux prepends the version required to the file before it is compiled.
## Variables sQLux makes available within a shader
The following variables are used to pass information to the shader.
### `attribute vec2 VertexCoord`
Vertex coordinate
### `attribute vec2 TexCoord`
Coordinate of pixel in texture
### `gl_Color`
See OpenGL description.
### `uniform mat4 MVPMatrix`
The combined model, view and projection matrix
### `uniform sampler2D TEX0`
The input texture, i.e. the image of the QL screen to display.
### `uniform vec2 TextureSize`
The size of the input texture (i.e. the original size in pixels of the QL display)
### `uniform vec2 u_resolution`
The output size of the image to be displayed
## Support for curvature
If `SHADER` is set to 2 in `sqlux.ini` then `#define CURVATURE` is prepended to the file prior to compilation.
### Mouse support
The mouse position has to be adjusted to reflect simulated curvature. Within the shader file it is assumed that the amount of curvature is defined by
```
#define CURVATURE_X a.aa
#define CURVATURE_Y b.bb
```
where a.aa and b.bb are floats defining the curvature in the x and y direction. sQLux parses these values and uses them to adjust the reported mouse position.
