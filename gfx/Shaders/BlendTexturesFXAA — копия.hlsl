// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

texture TextureTransparent;
sampler SamplerTransparent = sampler_state
{
	Texture = <TextureTransparent>;
	/*MinFilter = Bilinear;
	MagFilter = Bilinear;*/
};	// sampler SamplerTransparent

texture TextureSolid;
sampler SamplerSolid = sampler_state
{
	Texture = <TextureSolid>;
	/*MinFilter = Bilinear;
	MagFilter = Bilinear;*/
};	// sampler SamplerSolid

static const float2 TexCoordOffset = 1.0f / float2(1440.0f * 0.75f, 900.0f);
//static const float2 TexCoordOffset = 1.0f / float2(1920.0f * 0.75f, 1080.0f);

float4 Tex2DSolid(float2 texCoords)
{
	return tex2D(SamplerSolid, texCoords);
}

float4 Tex2DTransparent(float2 texCoords)
{
	return tex2D(SamplerTransparent, texCoords);
}

/* Performs AA for our render solid target. */
float4 AntiAliasBlendedSolidRenderTargets(float2 texCoords)
{
	static const float4 luminance = float4(0.299f, 0.587f, 0.114f, 0.0f);

	/* Computing luminance in the surrounding texels.. */
	const float luminanceCenter = dot(luminance, Tex2DSolid(texCoords + TexCoordOffset));
	const float luminanceTopLeft = dot(luminance, Tex2DSolid(texCoords + float2(-1.0f, -1.0f) * TexCoordOffset));
	const float luminanceTopRight = dot(luminance, Tex2DSolid(texCoords + float2(+1.0f, -1.0f) * TexCoordOffset));
	const float luminanceBottomLeft = dot(luminance, Tex2DSolid(texCoords + float2(-1.0f, +1.0f) * TexCoordOffset));
	const float luminanceBottomRight = dot(luminance, Tex2DSolid(texCoords + float2(+1.0f, +1.0f) * TexCoordOffset));

	const float minLuminance = min(luminanceCenter, min(luminanceTopLeft, min(luminanceTopRight, min(luminanceBottomLeft, luminanceBottomRight))));
	const float maxLuminance = max(luminanceCenter, max(luminanceTopLeft, max(luminanceTopRight, max(luminanceBottomLeft, luminanceBottomRight))));

	static const float dirReduceMul = 1.0f / 4.0f;
	static const float dirReduceMin = 1.0f / 128.0f;
	static const float2 dirSpanMax = float2(8.0f, 8.0f);

	/* Computing the blur vector.. */
	float2 blurDirection = float2((luminanceTopLeft - luminanceTopRight) + (luminanceBottomLeft - luminanceBottomRight)
		, (luminanceTopLeft - luminanceBottomLeft) - (luminanceTopRight - luminanceBottomRight));

	const float blurDirectionReduce = max((luminanceTopLeft + luminanceTopRight + luminanceBottomLeft + luminanceBottomRight) * dirReduceMul, dirReduceMin);
	const float blurInverseDirectionAdjustment = 1.0f / (blurDirectionReduce + min(abs(blurDirection.x), abs(blurDirection.y)));
	blurDirection = clamp(blurInverseDirectionAdjustment * blurDirection, -dirSpanMax, dirSpanMax) * TexCoordOffset;
	
	/* Applying blur.. */
	const float4 oneStepBlur = 0.5f * (
		Tex2DSolid(texCoords + blurDirection * (1.0f / 3.0f - 0.5f)) +
		Tex2DSolid(texCoords + blurDirection * (2.0f / 3.0f - 0.5f)));
	const float4 twoStepBlur = 0.5f * oneStepBlur + 0.25f * (
		Tex2DSolid(texCoords + blurDirection * (0.0f / 3.0f - 0.5f)) +
		Tex2DSolid(texCoords + blurDirection * (3.0f / 3.0f - 0.5f)));
	
	/* If only we have power to run this.. */
	const float luminanceTwoStep = dot(luminance, twoStepBlur);
	if (minLuminance < luminanceTwoStep && luminanceTwoStep < maxLuminance)
	{
		return twoStepBlur;
	}
	return oneStepBlur;
}

/* Blends two textures from our render targets. */
float4 BlendRenderTargets(float2 texCoords)
{
	const float4 transparentCenter = Tex2DTransparent(texCoords);
	const float4 transparentTopLeft = Tex2DTransparent(texCoords + float2(-1.5f, -1.5f) * TexCoordOffset);
	const float4 transparentTopRight = Tex2DTransparent(texCoords + float2(+1.5f, -1.5f) * TexCoordOffset);
//	const float4 transparentBottomLeft = Tex2DTransparent(texCoords + float2(-1.0f, +1.0f) * TexCoordOffset);
//	const float4 transparentBottomRight = Tex2DTransparent(texCoords + float2(+1.0f, +1.0f) * TexCoordOffset);
	const float4 transparent = 1.0f / 3.0f * (transparentCenter + transparentTopLeft + transparentTopRight /*+ transparentBottomLeft + transparentBottomRight*/);
	
//	const float4 transparent = tex2D(SamplerTransparent, texCoords);
	const float4 solid = AntiAliasBlendedSolidRenderTargets(texCoords);

	const float4 sourceBlendColor = solid;
	const float4 destBlendColor = solid * transparent;
	return lerp(sourceBlendColor, destBlendColor, transparent.a);
}

float4 main(float2 texCoords : TEXCOORD0) : COLOR
{
	return BlendRenderTargets(texCoords);
}
