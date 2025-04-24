#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// input from vertex shader
in vec2 texture_coord;
// get mode
uniform int mode;

// the only output variable
out vec4 fragColor;

void main()
{
	if(mode == 0)
        fragColor = vec4(texture_coord.xy, 0.0, 1.0);
    else if(mode == 1)
        fragColor = vec4(texture_coord.x, texture_coord.x, texture_coord.x, 1.0);
    else if(mode == 2)
        fragColor = vec4(texture_coord.y, texture_coord.y, texture_coord.y, 1.0);
}
