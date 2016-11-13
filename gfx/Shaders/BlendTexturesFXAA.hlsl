// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

texture TextureTransparent;
sampler SamplerTransparent = sampler_state
{
	Texture = <TextureTransparent>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = Linear;
	MagFilter = Linear;
};	// sampler SamplerTransparent

texture TextureSolid;
sampler SamplerSolid = sampler_state
{
	Texture = <TextureSolid>;
	AddressU = Clamp;
	AddressV = Clamp;
	MinFilter = Linear;
	MagFilter = Linear;
};	// sampler SamplerSolid

float3 SolidTex2D(float2 texCoords)
{
	return tex2D(SamplerSolid, texCoords);
}

/* Performs AA for our render solid target. */
float4 SolidAntiAlias(float2 texCoords)
{
	static const float2 texCoordOffset = 1.0f / float2(1440.0f, 1080.0f);
	static const float3 luminance = float3(0.299f, 0.587f, 0.114f);

	/* Computing luminance in the surrounding texels.. */
	const float luminanceCenter = dot(luminance, SolidTex2D(texCoords + texCoordOffset));
	const float luminanceTopLeft = dot(luminance, SolidTex2D(texCoords + float2(-1.0f, +1.0f) * texCoordOffset));
	const float luminanceTopRight = dot(luminance, SolidTex2D(texCoords + float2(+1.0f, +1.0f) * texCoordOffset));
	const float luminanceBottomLeft = dot(luminance, SolidTex2D(texCoords + float2(-1.0f, -1.0f) * texCoordOffset));
	const float luminanceBottomRight = dot(luminance, SolidTex2D(texCoords + float2(-1.0f, +1.0f) * texCoordOffset));

	const float minLuminance = min(luminanceCenter, min(luminanceTopLeft, min(luminanceTopRight, min(luminanceBottomLeft, luminanceBottomRight))));
	const float maxLuminance = max(luminanceCenter, max(luminanceTopLeft, max(luminanceTopRight, max(luminanceBottomLeft, luminanceBottomRight))));

	static const float dirReduceMul = 1.0f;
	static const float dirReduceMin = 1.0f / 256.0f;
	static const float2 dirSpanMax = float2(16.0f, 16.0f);

	/* Computing the blur vector.. */
	float2 blurDirection = float2((luminanceTopLeft + luminanceTopRight) - (luminanceBottomLeft + luminanceBottomRight)
		, (luminanceTopLeft + luminanceBottomLeft) - (luminanceTopRight + luminanceBottomRight));

	const float blurDirectionReduce = max((luminanceTopLeft + luminanceTopRight + luminanceBottomLeft + luminanceBottomRight) * dirReduceMul * 0.25f, dirReduceMin);
	const float blurInverseDirectionAdjustment = 1.0f / (blurDirectionReduce + min(abs(blurDirection.x), abs(blurDirection.y)));
	blurDirection = clamp(blurDirection, -dirSpanMax, dirSpanMax);
	blurDirection *= step(float2(1.0f, 1.0f), abs(blurDirection));

	/* Applying blur.. */
	const float3 oneStepBlur = 0.5f * (
		SolidTex2D(texCoords + blurDirection * (1.0f / 3.0f - 0.5f)) +
		SolidTex2D(texCoords + blurDirection * (2.0f / 3.0f - 0.5f)));
	const float3 twoStepBlur = 0.5f * oneStepBlur + 0.25f * (
		SolidTex2D(texCoords + blurDirection * (0.0f / 3.0f - 0.5f)) +
		SolidTex2D(texCoords + blurDirection * (3.0f / 3.0f - 0.5f)));
	
	const float luminanceTwoStep = dot(luminance, twoStepBlur);
	if (minLuminance < luminanceTwoStep && luminanceTwoStep < maxLuminance)
	{
		return float4(twoStepBlur, 1.0f);
	}
	return float4(oneStepBlur, 1.0f);
}

/* Blends two textures from our render targets. */
float4 BlendTextures(float2 texCoords)
{
	const float4 transparent = tex2D(SamplerTransparent, texCoords);
	const float4 solid = SolidAntiAlias(texCoords);

	const float4 sourceBlendColor = solid;
	const float4 destBlendColor = solid * transparent;
	return lerp(sourceBlendColor, destBlendColor, transparent.a);
}

float4 main(float2 texCoords : TEXCOORD0) : COLOR
{
	return BlendTextures(texCoords);
}
