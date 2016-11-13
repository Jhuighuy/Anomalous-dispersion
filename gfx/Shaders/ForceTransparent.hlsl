// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

/* Assuming that we have a single texture per object. */
texture Texture0;
sampler Sampler0 = sampler_state
{
	Texture = <Texture0>;
	MinFilter = Bilinear;
	MagFilter = Bilinear;
	AddressU = Clamp;
	AddressV = Clamp;
};	// sampler Sampler0

float4 main(float4 diffuseColor : COLOR0, float2 texCoord : TEXCOORD0) : COLOR
{
	return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
