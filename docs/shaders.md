# sQLux Shader support

## Introduction
sQLux optionally supports OPenGL Shader Language (GLSL) shaders. These shaders run on the GPU and generate display artifacts similar to those seen when using a BBQL with a 1980s monitor. Artifacts that can be emulated include:
+ Scan lines
+ Shadow mask
+ Bloom
+ Screen curvature

Support for shaders has to be enabled at run time. OpenGL or OpenGL ES libraries must to be installed. Shader support is realised through the use of [sdl-gpu](https://github.com/grimfang4/sdl-gpu) which is statically linked into sQLux. The code for sdl-gpu is included in the sQLux respository.  
As shaders run on the GPU, the speed of the emulator is not impacted.

---
# Building
In all cases, first follow the steps to build sQLux without shader support, as described in the [README](../README.md). Then update submodules

```
git submodule init
git submodule update
```

## Building linux
```
cd linux  
cmake .. -DSUPPORT_SHADERS=TRUE
make
```  
## Building MinGW on Linux
```
cd mingw
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-x86_64.cmake -DCMAKE_PREFIX_PATH=/usr/local/x86_64-w64-mingw32 -DSUPPORT_SHADERS=TRUE ..
```
## Building MinGW on Windows
```
cd mingw
cmake.exe -G "MinGW Makefiles" -DSUPPORT_SHADERS=TRUE ..
mingw32-make
```
# Shader parameters in sqlux.ini
`SHADER`  Selects shader support. 0 disables shaders. 1 enables a "flat" shader. 2 enables a shader including emulated barrel disortion. Disabled by default.  


```
SHADER = 2
```

`SHADER_FILE`
The path to the GPU shader (written in OpenGL Shading Language) that is loaded when `SHADER` is set to 1 or 2.

```
SHADER_FILE = ../shaders/crt-pi.glsl
```
# Creating your own shader
Examples of shaders which can be modified to use within sQLux can be found at e.g. [libretro shaders](https://github.com/libretro/glsl-shaders/tree/master/crt/shaders).  
A single file is used for both the vertex and fragment shader. It is compiled twice to generate the two shader types, once with `#define VERTEX` prepended to the start of the file, and once with `#define FRAGMENT`  
The following variables are used to pass information to the shader. 
## Variables sQLux makes available within a shader
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
The size of the window to display the result in 

## Support for curvature
If `SHADER` is set to 2 in `sqlux.ini` then `#define CURVATURE` is prepended to the file prior to compilation.
### Mouse support
The mouse position has to be adjusted to reflect simulated curvature. Within the slader file it is assumed that the amount of curvature is defeined by   
```
#define CURVATURE_X a.aa  
#define CURVATURE_Y b.bb  
```
where a.aa and b.bb are floats defining the curvature in the x and y direction. sQLux parses these values and uses them to adjust the reported mouse position.
