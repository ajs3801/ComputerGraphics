// vertex attributes
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

// matrices
uniform mat4 model_matrix;
uniform mat4 view_projection_matrix;

out vec2 tc;

void main()
{
	vec4 wpos = model_matrix * vec4(position, 1);
	gl_Position = view_projection_matrix * wpos;

	tc = texcoord;
}
