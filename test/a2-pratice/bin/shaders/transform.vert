#version 330 core
// vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

// transformation uniforms
uniform mat4 view_projection_matrix;
uniform mat4 model_matrix;

// output to fragment shader: texture coordinate
out vec2 tc;

void main()
{
    vec4 wpos = model_matrix * vec4(position, 1.0);
    gl_Position = view_projection_matrix * wpos;
    tc = texcoord;
}
