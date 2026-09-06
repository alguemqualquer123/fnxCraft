#version 430 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

layout(binding = 0) uniform sampler2D u_color;
layout(binding = 1) uniform sampler2D u_depth;

uniform float u_wetness;
uniform float u_time;
uniform vec2 u_screenSize;

void main()
{
    vec2 uv = v_texCoords;
    vec3 color = texture(u_color, uv).rgb;
    float depth = texture(u_depth, uv).r;

    vec2 wind = vec2(sin(u_time * 0.5) * 0.02, cos(u_time * 0.3) * 0.01);
    vec2 rainUV = uv + wind + vec2(0, u_time * 0.3);
    float streak = sin(rainUV.y * 200.0) * 0.5 + 0.5;
    float alpha = streak * u_wetness * 0.3;

    vec3 rainColor = vec3(0.6, 0.7, 0.9) * alpha;
    color += rainColor;

    outColor = vec4(color, 1.0);
}
