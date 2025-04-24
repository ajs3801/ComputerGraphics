// input attributes of vertices
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal; // we are using this for vertex color

// the second output = additional input to fragment shader
out vec3 vertex_color;

// uniform variables
uniform float theta; // rotation angle

void main()
{
	// built-in output variable that must be written
	gl_Position = vec4( position, 1 );

	// rotate the vertices
	float c=cos(theta), s=sin(theta);
	mat2 m = mat2(c,s,-s,c); // column-major rotation matrix
	gl_Position.xy = m * position.xy; // swizzling for easy access
	// gl_Position.xy *= abs(cos(theta * 10.0));		// 시간에 따라 Scaling
	// another output passed via output variable
	vertex_color = normal; // pass the color in norm to the vertex color output
}
