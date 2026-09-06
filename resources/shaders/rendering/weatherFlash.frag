#version 430 core

layout(location = 0) out vec4 outColor;

noperspective in vec2 v_texCoords;

layout(binding = 0) uniform sampler2D u_color;

uniform float u_flashIntensity;
uniform float u_time;

void main()
{
    vec3 color = texture(u_color, v_texCoords).rgb;
    float flash = u_flashIntensity * (0.5 + 0.5 * sin(u_time * 20.0));
    color += vec3(1.0, 0.95, 0.8) * flash * 0.6;
    outColor = vec4(color, 1.0);
}
