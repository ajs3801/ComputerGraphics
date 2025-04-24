#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility

//*************************************
// global constants
static const char*	window_name	= "cgbase - Hello triangle!";
static const char*	vert_shader_path = "shaders/hello.vert";
static const char*	frag_shader_path = "shaders/hello.frag";

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2(512,512);

//*************************************
// OpenGL objects
GLuint		program	= 0;		// ID holder for GPU program
GLuint		vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;		// index of rendering frames
vec4    solid_color = vec4( 1.0f, 0.5f, 0.5f, 1.0f );
bool    b_solid_color = false;

//*************************************
void update()
{
	// update simulation factor by time
	float theta = float(glfwGetTime())*0.5f;

	// speed up rotation
	// float theta = 10.0f * float(glfwGetTime())*0.5f;
	// slow down rotation
	// float theta = 0.2f * float(glfwGetTime())*0.5f;
	// update uniform variables in vertex/fragment shaders
	// theta, b_solid_color, solid_color -> vertex shader program에 있음
	// Shader Files/hello.vert

	// glGetUniformLocation이 실패하면 -1을 반환함. 
	// uloc이 성공하면 glUniform1f를 통해 실제 값을 가져옴
	GLint uloc;
	uloc = glGetUniformLocation( program, "theta" );			if(uloc>-1) glUniform1f( uloc, theta );				
	uloc = glGetUniformLocation( program, "b_solid_color" );	if(uloc>-1) glUniform1i( uloc, b_solid_color );
	uloc = glGetUniformLocation( program, "solid_color" );		if(uloc>-1) glUniform4fv( uloc, 1, solid_color );	// pointer version
}

void render()
{
	// frame 2정도까지는 잘 보일텐데, 그 이후로는 지우지 않으면 잔상이 생김
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex array object
	glBindVertexArray( vertex_array );

	// render vertices: trigger shader programs to process vertex data
	glDrawArrays( GL_TRIANGLES, 0, 3 ); // (topology, start offset, no. vertices)
	
	// 다른 삼각형도 같이 그리기 (다른 속도로 돌게 만듦)
	//int uloc = glGetUniformLocation(program, "theta");
	//float theta = float(glfwGetTime()) * 0.2f;
	//if (uloc > -1) glUniform1f(uloc, theta * 2.0f);

	//glDrawArrays(GL_TRIANGLES, 0, 3); // (topology, start offset, no. vertices)

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'd' to toggle between solid color and vertex colors\n" );
	printf( "\n" );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if(key==GLFW_KEY_D)
		{
			b_solid_color = !b_solid_color;
			printf( "> using %s\n", b_solid_color ? "solid color" : "texture coordinates as color" );
		}
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// enable backface culling
	glEnable( GL_DEPTH_TEST );								// enable depth test

	// create a vertex array for triangles in default view volume: [-1~1, -1~1, -1~1]
	vertex vertices[] =
	{
		{ vec3(0.433f,-0.25f,0), vec3(1,0,0), vec2(0.0f) },  // {position, red color, ...}
		{ vec3(0.0f,0.5f,0), vec3(0,1,0), vec2(0.0f) },      // {position, green color, ...}
		{ vec3(-0.433f,-0.25f,0), vec3(0,0,1), vec2(0.0f) }  // {position, blue color, ...}
	};
	
	// create and update vertex buffer
	GLuint vertex_buffer;
	// generate -> bind -> set
	glGenBuffers( 1, &vertex_buffer ); // generate a buffer object
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer ); // notify GL to use the buffer
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW ); // copy data to GPU

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	if(vertex_array) glDeleteVertexArrays(1,&vertex_array);	// if vertex array already exist
	vertex_array = cg_create_vertex_array( vertex_buffer );
	if(!vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return false; }

	return true;
}

void user_finalize()
{
	// clean-up code put here
	if(vertex_array) glDeleteVertexArrays(1,&vertex_array);
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
