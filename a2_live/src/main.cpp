#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility

//*************************************
// global constants
static const char*	window_name = "cgbase - transform - Stanford dragon colored by normals";
static const char*	vert_shader_path = "shaders/transform.vert";
static const char*	frag_shader_path = "shaders/transform.frag";
static const char*	mesh_vertex_path = "mesh/dragon.vertex.bin";
static const char*	mesh_index_path	= "mesh/dragon.index.bin";
static const uint	MIN_INSTANCE = 1;	// minimum instances
static const uint	MAX_INSTANCE = 3;	// maximum instances
uint				NUM_INSTANCE = 1;	// initial instances

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
// mesh here! (store sphere mesh in here)
mesh*	p_mesh = nullptr;
// camera
camera	cam;

//*************************************
void update()
{
	float aspect = window_size.x / float(window_size.y);

	// update projection matrix
	cam.aspect = window_size.x/float(window_size.y);
	cam.projection_matrix = mat4::perspective( cam.fovy, cam.aspect, cam.dnear, cam.dfar );

	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "view_matrix" );			if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.view_matrix );		// update the view matrix (covered later in viewing lecture)
	uloc = glGetUniformLocation( program, "projection_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.projection_matrix );	// update the projection matrix (covered later in viewing lecture)

	mat4 aspect_matrix = mat4::scale(std::min(1 / aspect, 1.0f), std::min(aspect, 1.0f), 1.0f);
	mat4 view_projection_matrix = aspect_matrix * mat4{ 0,1,0,0,0,0,1,0,-1,0,0,1,0,0,0,1 };

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
	if(p_mesh&&p_mesh->vertex_array) glBindVertexArray( p_mesh->vertex_array );

	// render vertices: trigger shader programs to process vertex data
	for( int k=0, kn=int(NUM_INSTANCE); k<kn; k++ )
	{
		// configure transformation parameters
		double t = glfwGetTime();
		float theta	= float(t)*((k%2)-0.5f)*float(k+1)*0.5f;
		float move	= ((k%2)-0.5f)*300.0f*float((k+1)/2);

		mat4 scale_matrix = (k == 1) ? mat4::scale(2.0f) : mat4();

		// build the model matrix
		mat4 model_matrix = mat4::translate( move, abs(move), 0.0f ) *
							mat4::translate( cam.at ) *
							mat4::rotate( vec3(0,0,1), theta ) *
							// scale_matrix *	scal
							mat4::translate( -cam.at );

		// update the uniform model matrix and render
		glUniformMatrix4fv( glGetUniformLocation( program, "model_matrix" ), 1, GL_TRUE, model_matrix );
		glUniform1i(glGetUniformLocation(program, "type"), k);
		glDrawElements( GL_TRIANGLES, GLsizei(p_mesh->index_list.size()), GL_UNSIGNED_INT, nullptr );
	}

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
	printf( "- press '+/-' to increase/decrease the number of instances (min=%d, max=%d)\n", MIN_INSTANCE, MAX_INSTANCE );
	printf( "\n" );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if(key==GLFW_KEY_KP_ADD||(key==GLFW_KEY_EQUAL&&(mods&GLFW_MOD_SHIFT))/* + */)
		{
			if(NUM_INSTANCE>=MAX_INSTANCE) return;
			printf( "> NUM_INSTANCE = % -4d\r", ++NUM_INSTANCE );
		}
		else if(key==GLFW_KEY_KP_SUBTRACT||key==GLFW_KEY_MINUS)
		{
			if(NUM_INSTANCE<=MIN_INSTANCE) return;
			printf( "> NUM_INSTANCE = % -4d\r", --NUM_INSTANCE );
		}
		// w key
		else if (key == GLFW_KEY_W || key == GLFW_KEY_MINUS)
		{
			static bool wireframe = false;
			// toggle
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

inline mesh* create_sphere_mesh()
{
	mesh* new_mesh = new mesh();

	// define vertex 
	for (int i = 0; i <= 36; i++) {
		for (int j = 0; j <= 18; j++) {
			float theta = ((j%18) / 18.0f)*PI;
			float phi = ((i%36) / 36.0f)*PI*2.0f;

			vertex v;
			v.pos = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			v.norm = v.pos; // r is 1
			v.tex = vec2(phi/3.14159f, 1-theta/PI);

			new_mesh->vertex_list.emplace_back(v);

			// index for index buffer
		}
	}

	// m + 1 vertex를 정의해야한다 (하나 더)
	// connecting indexes (texture coordinate이 달라야된다?)

	/*vertex v;
	v.pos = vec3(0, 0.5f, -0.5f);
	v.norm = vec3(0, 0.5f, -0.5f);
	v.tex = vec2(0.0f, 0);
	new_mesh->vertex_list.emplace_back(v);

	v.pos = vec3(0, 0, 0.5f);
	new_mesh->vertex_list.emplace_back(v);

	v.pos = vec3(0, -0.5f, -0.5f);
	new_mesh->vertex_list.emplace_back(v);*/

	new_mesh->index_list.emplace_back(0);
	new_mesh->index_list.emplace_back(1);
	new_mesh->index_list.emplace_back(2);

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

	// load the mesh
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
