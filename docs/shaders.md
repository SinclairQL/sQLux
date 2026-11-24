# sQLux Shader support

## Introduction
sQLux can draw the QL screen through a GPU shader that introduces display artifacts similar to those seen when using a BBQL with a 1980s CRT monitor. Artifacts that can be emulated include:
+ Scan lines
+ Shadow mask
+ Bloom
+ Screen curvature
+ Grey, amber and green monitors

The shader is a fragment shader of the SDL3 GPU renderer, so it needs sQLux built with SDL 3.4 or later and a GPU supported by SDL: Metal on macOS, Vulkan on Linux and Windows. No other libraries are needed and nothing has to be enabled at build time.
If the GPU renderer cannot be used, sQLux falls back to the normal display without the shader.
As shaders run on the GPU, the speed of the emulator is not impacted.

# Shader parameters in sqlux.ini
`SHADER`  Selects the shader. 0 disables it (default). The value is a bit field:

| Bits | Meaning |
|---|---|
| 0-1 | Shape: 1 = "flat" shader, 2 = shader including emulated barrel distortion, 0 = colour change only (no scanlines, mask or curvature) |
| 6-7 | Colour: 0 = normal, 64 = grey, 128 = amber, 192 = green |

For example, 66 is a curved shader with a grey display.

```
SHADER = 2
```

# Changing the shader
The shader is `src/shaders/crt-pi.frag`, written in Vulkan GLSL (version 450). It is compiled into SPIR-V (Vulkan) and MSL (Metal), which are included in sQLux as C headers: after editing it, run `src/shaders/make-shaders.sh` and rebuild sQLux. The script needs `glslang` and `spirv-cross`; if `shadercross` (from [SDL_shadercross](https://github.com/libsdl-org/SDL_shadercross)) is installed it also writes DXIL, so that Direct3D 12 can be used on Windows.

The following attributes can be modified in the shader file:
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

The amount of curvature, `CURVATURE_X` (0.06) and `CURVATURE_Y` (0.12), is set in `src/SDL3shader.c`, which also uses it to adjust the mouse position to the curved screen.

## Variables sQLux makes available within the shader
The shader receives from the vertex shader of the SDL GPU renderer the colour (`location = 0`) and the texture coordinate (`location = 1`). The QL screen is the sampler at `set = 2, binding = 0`. These values are in the uniform block at `set = 3, binding = 0`:
### `vec2 TextureSize`
The size of the input texture (i.e. the original size in pixels of the QL display)
### `vec2 OutputSize`
The size, in window pixels, at which the QL display is drawn
### `vec2 Curvature`
The barrel distortion in x and y; 0 for a flat screen
### `float MonoMode`
0 = normal colour, 1 = grey, 2 = amber, 3 = green
### `float ColourOnly`
1 if only the colour has to be changed (no scanlines, mask or curvature)
