#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 WorldPos;
    vec2 TexCoord;
    mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler2D displacementmap;
uniform float displacement_coefficient;

void main()
{
    vs_out.TexCoord = aTexCoord;
    
    // TODO HW8 - 0_displacement_normal | calculate TBN
    mat3 normalMatrix = mat3(model); //transpose(inverse(mat3(model)));

    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);

    // TODO HW8 - 0_displacement_normal | calculate displacement

    //float lambda = displacement_coefficient * (2 * texture(displacementmap, aTexCoord).r - 1);
    float lambda = displacement_coefficient * texture(displacementmap, aTexCoord).r;

    vec4 displacement = lambda * vec4(N, 0.0);

    //∑®œÚÃ˘Õº
    //vec4 worldPos = model * vec4(aPos, 1.0);
    //÷√ªªÃ˘Õº
    vec4 worldPos = model * vec4(aPos, 1.0) + displacement;


    vs_out.WorldPos = worldPos.xyz / worldPos.w;  // worldPos.w == 1

    gl_Position = projection * view * worldPos;
}
