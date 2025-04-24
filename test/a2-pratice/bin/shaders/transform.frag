#version 330 core
#ifdef GL_ES
precision highp float;
#endif

in vec2 tc;
uniform int mode;
out vec4 fragColor;

void main()
{
    // mode 0: (tc.xy, 0, 1)
    // mode 1: (tc.xxx, 1)
    // mode 2: (tc.yyy, 1)
    if(mode == 0)
        fragColor = vec4(tc.xy, 0.0, 1.0);
    else if(mode == 1)
        fragColor = vec4(tc.x, tc.x, tc.x, 1.0);
    else if(mode == 2)
        fragColor = vec4(tc.y, tc.y, tc.y, 1.0);
}
