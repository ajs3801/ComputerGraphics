#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility

//*************************************
// global constants
static const char*	window_name = "sphere";
static const char*	vert_shader_path = "shaders/transform.vert";
static const char*	frag_shader_path = "shaders/transform.frag";

//*************************************
// common structures
struct camera
{
	vec3	eye = vec3( 0, -300, 30 );
	vec3	at = vec3( 0, 0, 0 );
	vec3	up = vec3( 0, 0, 1 );
	mat4	view_matrix = mat4::look_at( eye, at, up );
		
	float	fovy = PI/4.0f; // must be in radian
	float	aspect;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix;
};

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program

//*************************************
// global variables
int		frame = 0;		// index of rendering frames

//*************************************
// scene objects
mesh*	p_mesh = nullptr;
camera	cam;

//*************************************
// global variables for Assignment 2
double	t = 0.0;				// time variable
bool	is_rotate = false;		// flag for rotating
int		texture_mode = 0;		// flag for texture mode
bool	b_wireframe = false;	// flag for wireframe
float	angle = 0.0;			// rotation angle variable
	
//*************************************
void update()
{
	// update time
	t = glfwGetTime() * 0.4;
	float aspect = window_size.x / float(window_size.y);

	// update projection matrix
	cam.aspect = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect, cam.dnear, cam.dfar);
	
	// code from Assignment2 pdf
	mat4 aspect_matrix = mat4::scale(std::min(1 / aspect, 1.0f), std::min(aspect, 1.0f), 1.0f);
	mat4 view_projection_matrix = aspect_matrix * mat4{ 0,1,0,0,0,0,1,0,-1,0,0,1,0,0,0,1 };

	GLint uloc;
	uloc = glGetUniformLocation(program, "view_projection_matrix");
	glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);
}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );
	
	// bind vertex array object
	if (p_mesh && p_mesh->vertex_array) glBindVertexArray(p_mesh->vertex_array);

	// build the model matrix 
	mat4 model_matrix = mat4::translate(cam.at) *
						mat4::rotate(vec3(0, 0, 1), angle) *
						mat4::translate(-cam.at);

	// update the uniform model matrix and render
	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
	// give texture mode to shader as mode
	glUniform1i(glGetUniformLocation(program, "mode"), texture_mode);

	// render
	glDrawElements(GL_TRIANGLES, GLsizei(p_mesh->index_list.size()), GL_UNSIGNED_INT, nullptr);

	// [Assignment 2 function] rotate using time and angle
	static double t0 = 0;			// still alive static
	float dt = float(t - t0);

	// if rotation is true
	if (is_rotate)
		angle += dt;

	t0 = t;							// update static time to current time

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
	printf("- press 'w' to toggle wireframe\n");
	printf("- press 'd' to toggle (tc.xy,0) > (tc.xxx) > (tc.yyy)\n");
	printf("- press 'r' to rotate the sphere\n");
	printf( "\n" );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_D)
		{
			// update texture mode
			texture_mode = (texture_mode + 1) % 3;

			// print color mode log
			if (texture_mode == 0) {
				printf("> using (texcoord.xxx) as color\n");
			}
			else if (texture_mode == 1) {
				printf("> using (texcoord.yyy) as color\n");
			}
			else if (texture_mode == 2) {
				printf("> using (texcoord.xy, 0) as color\n");
			}
		}
		else if (key == GLFW_KEY_R)
		{
			// change rotation flag
			is_rotate = !is_rotate;
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

// Assignment 2
// create sphere mesh
inline mesh* create_sphere_mesh() 
{
	// define mesh
	mesh* new_mesh = new mesh();

	// 1. make sphere vertex (36 latitude and 72 longitude)
	// loop over latitude
	for (int latitude = 0; latitude <= 36; latitude++) {
		// loop over longitude
		for (int longitude = 0; longitude <= 72; longitude++) {
			// define theta and phi
			float theta = (float(latitude) / 36.0f) * PI;
			float phi = (float(longitude) / 72.0f) * 2.0f * PI;

			// define vertex
			vertex v;

			// N(x,y,z): formula from a2 pdf | pos and norm are same
			v.pos = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			v.norm = v.pos;	// doesn't need to normalize (already normalized)
			// T(x,y): formula from a2 pdf 
			v.tex = vec2(phi / (2.0f * PI), 1.0f - theta / PI);

			// push vertex to mesh vertex list
			new_mesh->vertex_list.push_back(v);
		}
	}

	// 2. make index list
	// loop over latitude
	for (int latitude = 0; latitude < 36; latitude++) {
		// loop over longitude
		for (int longitude = 0; longitude < 72; longitude++) {
			int point1 = latitude * (72 + 1) + longitude;
			int point2 = point1 + 72 + 1;

			// triangle 1
			new_mesh->index_list.push_back(point1);
			new_mesh->index_list.push_back(point2);
			new_mesh->index_list.push_back(point1 + 1);

			// triangle 2
			new_mesh->index_list.push_back(point2);
			new_mesh->index_list.push_back(point2 + 1);
			new_mesh->index_list.push_back(point1 + 1);
		}
	}

	// code from cg_load_mesh()
	// create a vertex buffer
	glGenBuffers(1, &new_mesh->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, new_mesh->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * new_mesh->vertex_list.size(), &new_mesh->vertex_list[0], GL_STATIC_DRAW);

	// create a index buffer
	glGenBuffers(1, &new_mesh->index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_mesh->index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * new_mesh->index_list.size(), &new_mesh->index_list[0], GL_STATIC_DRAW);

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	new_mesh->vertex_array = cg_create_vertex_array(new_mesh->vertex_buffer, new_mesh->index_buffer);
	if (!new_mesh->vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return nullptr; }

	return new_mesh;
}
bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests

	// load the mesh (in this assignment load sphere)
	p_mesh = create_sphere_mesh();

	if(p_mesh==nullptr){ printf( "Unable to load mesh\n" ); return false; }

	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movement

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
