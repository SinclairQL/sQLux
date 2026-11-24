#version 450
/*
    crt-pi - A Raspberry Pi friendly CRT shader.

    Copyright (C) 2015-2016 davej
	Modified 2022 for sQLux
	Modified for colour modes / colour-only
	Ported to the SDL3 GPU renderer: the modes are now uniforms

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.


Notes:

SCANLINES enables scanlines. You'll almost certainly want to use it with MULTISAMPLE to reduce moire effects. SCANLINE_WEIGHT defines how wide scanlines are (it is an inverse value so a higher number = thinner lines). SCANLINE_GAP_BRIGHTNESS defines how dark the gaps between the scan lines are. Darker gaps between scan lines make moire effects more likely.

GAMMA enables gamma correction using the values in INPUT_GAMMA and OUTPUT_GAMMA. FAKE_GAMMA causes it to ignore the values in INPUT_GAMMA and OUTPUT_GAMMA and approximate gamma correction in a way which is faster than true gamma whilst still looking better than having none. You must have GAMMA defined to enable FAKE_GAMMA.

By default the shader uses linear blending horizontally. If you find this too blury, enable SHARPER.

BLOOM_FACTOR controls the increase in width for bright scanlines.

MASK_TYPE defines what, if any, shadow mask to use. MASK_BRIGHTNESS defines how much the mask type darkens the screen.

This is the fragment shader of the SDL3 GPU renderer (SDL 3.4 or later),
applied when the QL screen texture is drawn. After changing it, run
make-shaders.sh to regenerate the compiled shaders.

Set by sQLux from the SHADER option (see docs/shaders.md):

    Curvature   barrel distortion in x and y, (0, 0) for a flat screen
    MonoMode    0 = normal colour, 1 = grey, 2 = amber, 3 = green
    ColourOnly  1 = colour change only: no scanlines, no mask, no curvature
*/

// MASK_TYPE: 0 = none, 1 = green/magenta, 2 = trinitron(ish)
#define MASK_TYPE 1

// Haven't put these as parameters as it would slow the code down.
#define SCANLINES
#define MULTISAMPLE
#define GAMMA
//#define FAKE_GAMMA
//#define SHARPER

#define MASK_BRIGHTNESS 0.70
#define SCANLINE_WEIGHT 5.0
#define SCANLINE_GAP_BRIGHTNESS 0.60
#define BLOOM_FACTOR 1.5
#define INPUT_GAMMA 2.4
#define OUTPUT_GAMMA 2.2

// From the vertex shader of the SDL GPU renderer
layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_uv;

layout(location = 0) out vec4 fragColor;

layout(set = 2, binding = 0) uniform sampler2D tex;

// Must match struct crt_uniforms in SDL3shader.c
layout(set = 3, binding = 0) uniform Context {
	vec2 TextureSize;	// the QL screen, in pixels
	vec2 OutputSize;	// the size it is drawn at, in window pixels
	vec2 Curvature;
	float MonoMode;
	float ColourOnly;
};

// Luminance + phosphor tint
vec3 mono(vec3 c)
{
	if (MonoMode < 0.5)
		return c;

	float l = dot(c, vec3(0.299, 0.587, 0.114));

	if (MonoMode < 1.5)
		return vec3(l);
	else if (MonoMode < 2.5)
		return l * vec3(1.00, 0.69, 0.00);
	return l * vec3(0.20, 1.00, 0.30);
}

vec2 Distort(vec2 coord)
{
	// Barrel distortion shrinks the display area a bit, this will allow us to counteract that.
	vec2 barrelScale = 1.0 - (0.23 * Curvature);
	coord -= vec2(0.5);
	float rsq = coord.x * coord.x + coord.y * coord.y;
	coord += coord * (Curvature * rsq);
	coord *= barrelScale;
	if (abs(coord.x) >= 0.5 || abs(coord.y) >= 0.5)
		coord = vec2(-1.0);		// If out of bounds, return an invalid value.
	else
	{
		coord += vec2(0.5);
	}

	return coord;
}

float CalcScanLineWeight(float dist)
{
	return max(1.0-dist*dist*SCANLINE_WEIGHT, SCANLINE_GAP_BRIGHTNESS);
}

float CalcScanLine(float dy, float filterWidth)
{
	float scanLineWeight = CalcScanLineWeight(dy);
#if defined(MULTISAMPLE)
	scanLineWeight += CalcScanLineWeight(dy-filterWidth);
	scanLineWeight += CalcScanLineWeight(dy+filterWidth);
	scanLineWeight *= 0.3333333;
#endif
	return scanLineWeight;
}

void main()
{
	vec2 TEX0 = v_uv * 1.0001;
	float filterWidth = (TextureSize.y / OutputSize.y) / 3.0;

	if (ColourOnly > 0.5) {
		// Colour change only: no scanlines, mask or curvature
		vec4 c = texture(tex, v_uv) * v_color;
		fragColor = vec4(mono(c.rgb), c.a);
		return;
	}

	vec2 texcoord = TEX0;
	if (Curvature.x > 0.0 || Curvature.y > 0.0) {
		texcoord = Distort(TEX0);
		if (texcoord.x < 0.0) {
			fragColor = vec4(0.0, 0.0, 0.0, 1.0);
			return;
		}
	}

	vec2 texcoordInPixels = texcoord * TextureSize;
#if defined(SHARPER)
	vec2 tempCoord = floor(texcoordInPixels) + 0.5;
	vec2 coord = tempCoord / TextureSize;
	vec2 deltas = texcoordInPixels - tempCoord;
	float scanLineWeight = CalcScanLine(deltas.y, filterWidth);
	vec2 signs = sign(deltas);
	deltas.x *= 2.0;
	deltas = deltas * deltas;
	deltas.y = deltas.y * deltas.y;
	deltas.x *= 0.5;
	deltas.y *= 8.0;
	deltas /= TextureSize;
	deltas *= signs;
	vec2 tc = coord + deltas;
#else
	float tempY = floor(texcoordInPixels.y) + 0.5;
	float yCoord = tempY / TextureSize.y;

	float dy = texcoordInPixels.y - tempY;
	float scanLineWeight = CalcScanLine(dy, filterWidth);
	float signY = sign(dy);
	dy = dy * dy;
	dy = dy * dy;
	dy *= 8.0;
	dy /= TextureSize.y;
	dy *= signY;
	vec2 tc = vec2(texcoord.x, yCoord + dy);
#endif

	vec3 colour = (texture(tex, tc) * v_color).rgb;

#if defined(SCANLINES)
#if defined(GAMMA)
#if defined(FAKE_GAMMA)
	colour = colour * colour;
#else
	colour = pow(colour, vec3(INPUT_GAMMA));
#endif
#endif
	scanLineWeight *= BLOOM_FACTOR;
	colour *= scanLineWeight;

#if defined(GAMMA)
#if defined(FAKE_GAMMA)
	colour = sqrt(colour);
#else
	colour = pow(colour, vec3(1.0/OUTPUT_GAMMA));
#endif
#endif
#endif

	colour = mono(colour);

#if MASK_TYPE == 0
	fragColor = vec4(colour, 1.0);
#else
#if MASK_TYPE == 1
	float whichMask = fract((gl_FragCoord.x*1.0001) * 0.5);
	vec3 mask;
	if (whichMask < 0.5)
		mask = vec3(MASK_BRIGHTNESS, 1.0, MASK_BRIGHTNESS);
	else
		mask = vec3(1.0, MASK_BRIGHTNESS, 1.0);
#elif MASK_TYPE == 2
	float whichMask = fract((gl_FragCoord.x*1.0001) * 0.3333333);
	vec3 mask = vec3(MASK_BRIGHTNESS, MASK_BRIGHTNESS, MASK_BRIGHTNESS);
	if (whichMask < 0.3333333)
		mask.x = 1.0;
	else if (whichMask < 0.6666666)
		mask.y = 1.0;
	else
		mask.z = 1.0;
#endif
	fragColor = vec4(colour * mask, 1.0);
#endif
}
