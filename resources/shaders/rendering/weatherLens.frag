#version 430 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

layout(binding = 0) uniform sampler2D u_color;
layout(binding = 1) uniform sampler2D u_depth;

uniform float u_wetness;
uniform vec2 u_screenSize;

float wetCurve(float d)
{
    return smoothstep(0.0, 1.0, d) * 0.15;
}

void main()
{
    vec2 uv = v_texCoords;
    vec3 color = texture(u_color, uv).rgb;

    float wet = u_wetness;
    if (wet < 0.1)
    {
        outColor = vec4(color, 1.0);
        return;
    }

    vec2 dir = uv - 0.5;
    float dist = length(dir);
    vec2 distorted = uv + dir * wet * 0.03 * (1.0 - dist) * 0.15;
    color = texture(u_color, distorted).rgb;

    vec2 bump = texture(u_depth, distorted).rg;
    color += bump.x * wet * 0.05;

    outColor = vec4(color, 1.0);
}
