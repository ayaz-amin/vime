#version 450

layout(location=0) in vec3 gPosition;
layout(location=1) in vec3 gColor;
layout(location=2) in vec3 gNormal;

layout(location=0) out vec4 frag_color;

layout(push_constant) uniform constants
{
    vec4 data;
    mat4 render_matrix;
} PushConstants;

void main()
{
    gl_Position = PushConstants.render_matrix * vec4(gPosition, 1.0f);
    frag_color = vec4(gColor, 1.0f);
}