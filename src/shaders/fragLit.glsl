#version 140

varying vec2 vr_TexCoord;
varying vec3 vr_NormalWS;
varying vec4 vr_Color;

vec4 litWithDirectionalLight(vec3 lightDirectionWS)
{
    float ambientComponent = 0.1;
    float diffuseComponent = max(0.0, dot(vr_NormalWS, lightDirectionWS));

    return vec4(ambientComponent + diffuseComponent, ambientComponent + diffuseComponent, ambientComponent + diffuseComponent, 1.0);
}

void main(void)
{
    vec3 lightDirectionWS = vec3(-0.1f, -0.3f, 1.0f);
    gl_FragColor = vr_Color * litWithDirectionalLight(normalize(lightDirectionWS));
}
