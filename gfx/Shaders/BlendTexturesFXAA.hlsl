// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

texture TextureTransparent;
sampler SamplerTransparent = sampler_state
{
	Texture = <TextureTransparent>;
	MinFilter = Bilinear;
	MagFilter = Bilinear;
};	// sampler SamplerTransparent

texture TextureOpaque;
sampler SamplerOpaque = sampler_state
{
	Texture = <TextureOpaque>;
	MinFilter = Bilinear;
	MagFilter = Bilinear;
};	// sampler SamplerOpaque

static const float4 g_ColorCorrection = float4(0.7, 0.7, 0.7, 1.0);

/* Blends two textures from our render targets. */
float4 BlendRenderTargets(float2 texCoords)
{
	const float4 transparent = tex2D(SamplerTransparent, texCoords);
	const float4 opaque = g_ColorCorrection * tex2D(SamplerOpaque, texCoords);

	const float4 sourceBlendColor = opaque;
	const float4 destBlendColor = transparent;
	return sourceBlendColor + destBlendColor;

//	const float4 sourceBlendColor = opaque;
//	const float4 destBlendColor = opaque * transparent;
//	return lerp(sourceBlendColor, destBlendColor, transparent.a);
}

static const float2 g_TexCoordOffset = 1.0f / float2(1440.0f * 0.75f, 900.0f); // 1.0f / float2(1920.0f * 0.75f, 1080.0f)
static const float4 g_Luminance = float4(0.299f, 0.587f, 0.114f, 0.0f);

static const float2 g_DirectionSpanMax = float2(8.0f, 8.0f);
static const float  g_DirectionReduceMul = 1.0f / 4.0f;
static const float  g_DirectionReduceMin = 1.0f / 128.0f;

/* Performs AA for our blended targets. */
float4 AntiAliasBlendedRenderTargets(float2 texCoords)
{
	/* Computing g_Luminance in the surrounding texels.. */
	const float luminanceCenter = dot(g_Luminance, BlendRenderTargets(texCoords + g_TexCoordOffset));
	const float luminanceTopLeft = dot(g_Luminance, BlendRenderTargets(texCoords + float2(-1.0f, -1.0f) * g_TexCoordOffset));
	const float luminanceTopRight = dot(g_Luminance, BlendRenderTargets(texCoords + float2(+1.0f, -1.0f) * g_TexCoordOffset));
	const float luminanceBottomLeft = dot(g_Luminance, BlendRenderTargets(texCoords + float2(-1.0f, +1.0f) * g_TexCoordOffset));
	const float luminanceBottomRight = dot(g_Luminance, BlendRenderTargets(texCoords + float2(+1.0f, +1.0f) * g_TexCoordOffset));

	const float minLuminance = min(luminanceCenter, min(luminanceTopLeft, min(luminanceTopRight, min(luminanceBottomLeft, luminanceBottomRight))));
	const float maxLuminance = max(luminanceCenter, max(luminanceTopLeft, max(luminanceTopRight, max(luminanceBottomLeft, luminanceBottomRight))));

	/* Computing the blur vector.. */
	float2 blurDirection = float2(
		(luminanceTopLeft - luminanceTopRight) + (luminanceBottomLeft - luminanceBottomRight),
		(luminanceTopLeft - luminanceBottomLeft) - (luminanceTopRight - luminanceBottomRight));

	const float blurDirectionReduce = max((luminanceTopLeft + luminanceTopRight + luminanceBottomLeft + luminanceBottomRight) * g_DirectionReduceMul, g_DirectionReduceMin);
	const float blurInverseDirectionAdjustment = 1.0f / (blurDirectionReduce + min(abs(blurDirection.x), abs(blurDirection.y)));
	blurDirection = clamp(blurInverseDirectionAdjustment * blurDirection, -g_DirectionSpanMax, g_DirectionSpanMax) * g_TexCoordOffset;
	
	/* Applying blur.. */
	const float4 oneStepBlur = 0.5f * (
		BlendRenderTargets(texCoords + blurDirection * (1.0f / 3.0f - 0.5f)) +
		BlendRenderTargets(texCoords + blurDirection * (2.0f / 3.0f - 0.5f)));
	const float4 twoStepBlur = 0.5f * oneStepBlur + 0.25f * (
		BlendRenderTargets(texCoords + blurDirection * (0.0f / 3.0f - 0.5f)) +
		BlendRenderTargets(texCoords + blurDirection * (3.0f / 3.0f - 0.5f)));
	
	const float luminanceTwoStep = dot(g_Luminance, twoStepBlur);
	if (minLuminance < luminanceTwoStep && luminanceTwoStep < maxLuminance)
	{
		return twoStepBlur;
	}
	return oneStepBlur;
}

float4 main(float2 texCoords : TEXCOORD0) : COLOR
{
	return AntiAliasBlendedRenderTargets(texCoords);
}
