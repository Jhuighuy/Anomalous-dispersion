#version 120

varying vec4 vr_Color;
varying vec4 vr_VertexCoordWS;
varying vec3 vr_NormalWS;

uniform vec3 un_CameraPositionWS;
uniform samplerCube un_Environment;

void main(void)
{
    const float eta = 1.0f / 1.52f;
    const float fresnelPower = 5.0f;
    const float f = (1.0f - eta) * (1.0f - eta) / (1.0f + eta) / (1.0f + eta);

    vec3 direction = normalize(vec3(vr_VertexCoordWS) - un_CameraPositionWS);
    vec3 refracted = refract(direction, vr_NormalWS, eta);
    vec3 reflected = reflect(direction, vr_NormalWS);
    float ratio = f + (1.0f - f) * pow(1.0f - dot(-direction, vr_NormalWS), fresnelPower);

    vec4 refractedColor = textureCube(un_Environment, refracted);
    vec4 reflectedColor = textureCube(un_Environment, reflected);
    vec4 color = mix(refractedColor, reflectedColor, ratio);
    gl_FragColor = color * vr_Color;
}
