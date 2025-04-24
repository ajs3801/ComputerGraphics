//*******************************************************************
// Terms of Use and Disclaimer
// Copyright 2011-2040 Sungkil Lee
//
// This software is owned by Sungkil Lee.
// You may use and modify this software for non-commercial and commercial software,
// but re-distribution of this software is strictly prohibited.
//*******************************************************************

#ifndef __CGUT_H__
#define __CGUT_H__

// detect OS and compiler
#if defined(_MSC_VER)
	#define CGUT_MSVC _MSC_VER
	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS
	#endif
#elif defined(__CYGWIN__)
	#if !defined(__MINGW64__) // disable mingw32/ucrt32/msys2
		#error use ucrt64 (or mingw64) toolchain in msys2
	#else
		#define CGUT_MINGW64
	#endif
#elif defined(__linux__)&&defined(__GNUC__)
	#define CGUT_LINUX __GNUC__
#elif defined(__APPLE__)
	#error __APPLE__ is not supported
#endif

// minimum standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// platform-dependent configuration
#if defined(__GNUC__)
	#include <unistd.h>
	static const int MAX_PATH = PATH_MAX;
#elif defined(CGUT_MSVC)
	// recommend using the latest Visual Studio
	#if _MSC_VER<1920
		#pragma message ("Install Visual Studio 2022 or higher rather than legacy Visual Studio.")
	#elif _MSC_VER<1930
		#pragma message ("Install Visual Studio 2022 or higher rather than legacy Visual Studio 2019.")
	#endif
	// disable excessive warnings
	#pragma warning( disable:4819 )  // different code page (949)
	#pragma warning( disable:6011 )	 // dereferencing NULL pointer (problematic even for safe cases)
	#pragma warning( disable:6031 )	 // return value ignored
	#pragma warning( disable:26451 ) // excessive warning on casting narrow value types
	#pragma warning( disable:26495 ) // always initialize a member variable
	#pragma warning( disable:26819 ) // Unannotated fallthrough between switch labels

	// enforce not to use /MD or /MDd flag
	#if defined(_DLL) && !defined(CGUT_DYNAMIC)
		#error Use /MT at Configuration > C/C++ > Code Generation > Run-time Library; Otherwise, define CGUT_DYNAMIC to use /MD
	#endif
	#include <direct.h>
	#include <io.h>
	// suppress warning for deprecated posix access
	#define access _access
	#define strdup _strdup
#endif

#if defined(__has_include)
	// GLFW
	#define GLFW_INCLUDE_NONE
	#if __has_include("gl/glfw/glfw3.h")
		#include "gl/glfw/glfw3.h"		// http://www.glfw.org // OpenGL/GLSL Core Profile 4.6
	#elif __has_include("gles/glfw/glfw3.h")
		#include "gles/glfw/glfw3.h"	// http://www.glfw.org // OpenGL/GLSL ES Profile 3.0 or higher
	#endif
	// GLAD
	#if __has_include("gl/glad/glad.h")		// "glad.h" renamed to avoid conflict with <gl.h>
		#ifndef CGUT_GLAD_NO_IMPL
			#define GLAD_GL_IMPLEMENTATION	// defined only once to avoid generating multiple symbols
		#endif
		#ifdef CGUT_MSVC
			#pragma warning( push )
			#pragma warning( disable: 4551 4505 )	// suppress warning in GLAD
		#endif
		#include "gl/glad/glad.h"				// https://gen.glad.sh/ generate your own glad files
		#ifdef CGUT_MSVC
			#pragma warning( pop )
		#endif
	#endif
	// stb_image.h
	#if __has_include( "stb_image.h" )
		#ifdef CGUT_MSVC
			#pragma warning( push )
			#pragma warning( disable: 6011 6262 6308 28182 ) // L3866, L6677, L6712, L6747, L6726
		#endif
		#ifndef CGUT_STB_IMAGE_NO_IMPL
			#define STB_IMAGE_IMPLEMENTATION
		#endif
		#include "stb_image.h"
		#ifdef CGUT_MSVC
			#pragma warning( pop )
		#endif
	#endif
#endif

// in-code linking to glfw lib
#if defined(_glfw3_h_) && defined(_MSC_VER)
	#if _MSC_VER>=1920 && !defined(_DLL)
		#if defined _M_X64
			#pragma comment( lib, "glfw3.x64.lib" ) // static lib for VC2019 or higher
		#endif
	#else // dynamic lib for other VC version
		#if defined _M_X64
			#pragma comment( lib, "glfw3dll.x64.lib" ) // dynamic lib for VC2019 or higher
		#endif
	#endif
#endif

//*************************************
// OpenGL versions
struct gl_version_t
{
	int major=0;
	int minor=0;
	struct { int major=0, minor=0; } sl;
	static gl_version_t& instance(){ static gl_version_t v; return v; }
	int gl() const { return major*10+minor;}
	int glsl() const { return sl.major*10+sl.minor;}

#ifdef GL_ES_VERSION_2_0
	constexpr bool is_gles(){ return true; }
#else
	constexpr bool is_gles(){ return false; }
#endif

#ifdef GL_ES_VERSION_2_0
	enum default_t { major_default=2, minor_default=0, major_min=2, minor_min=0, };	// minimum for modern GL ES
#elif defined(CGUT_LINUX)
	enum default_t { major_default=4, minor_default=5, major_min=3, minor_min=3, }; // default/minimum for modern GL
#elif defined(CGUT_MINGW64)||defined(CGUT_MSVC)
	enum default_t { major_default=1, minor_default=0, major_min=3, minor_min=3, }; // default/minimum for modern GL
#else
	enum default_t { major_default=1, minor_default=0, major_min=3, minor_min=3, }; // unexpected
#endif
};

//*************************************
// module path
struct module_t
{
	char path[MAX_PATH]={}, dir[MAX_PATH]={}, name[MAX_PATH]={}, ext[MAX_PATH]={};
	module_t()
	{
		strcpy(path,_get_module_path());
		char *s=path,*next; while((next=strpbrk(s+1,"/\\"))) s=next;
		strcpy(name,path==s?path:s+1); if(path!=s) strncpy(dir,path,s-path+1);
		char* dot=strrchr(name,'.'); if(dot){strcpy(ext,dot);*dot='\0';}
	}

	const char* temp_dir( bool b_mkdir=true )
	{
		static char d[MAX_PATH+1]; if(*d) return d; strcpy(d,dir);
	#if defined(CGUT_MSVC)
		GetTempPathA(MAX_PATH,d); if(*d){size_t s=strlen(d);if(d[s-1]!='\\'){d[s]='\\';d[s+1]='\0';}}
		strcpy(d+strlen(d),".cgbase\\"); if(b_mkdir&&access(d,0)!=0&&_mkdir(d)!=0) printf( "error: failed in mkdir(%s)\n", d );
	#endif
		return d;
	}

	inline const char* conf_path()
	{
		static std::vector<char> f(MAX_PATH*3+1,0);
		if(!f[0]){ sprintf(f.data(),"%s%s%s.conf",temp_dir(),name,ext); }
		return f.data();
	}

#ifdef __argv // MSVC or MinGW
	static const char* _get_module_path(){ static char p[MAX_PATH]={}; if(!*p) strcpy(p,__argv[0]); return p; }
#elif defined(CGUT_LINUX) // linux
	static const char* _get_module_path(){ static char p[MAX_PATH]={}; if(!*p){ char l[MAX_PATH]={}; sprintf(l,"/proc/%d/exe",getpid()); readlink(l,p,MAX_PATH); } return p; }
#endif
};

// build/canonicalize a full path from the module path
inline const char* absolute_path( const char* file_path )
{
	if(!file_path||!*file_path) return file_path; // null path
#if defined(CGUT_MSVC)||defined(CGUT_MINGW64)
	if(strchr(file_path,':')) return file_path; // is already absolute path
	char t[MAX_PATH]; sprintf_s( t, "%s%s", module_t().dir, file_path ); // build absolute path
	for(auto& c:t) if(c=='/') c='\\'; // slash to backslash in Windows
	static char f[MAX_PATH]; return _fullpath( f, t, MAX_PATH ); // canonicalize the path
#else
	if(*file_path=='/') return file_path; // is already absolute path
	static char f[MAX_PATH]; sprintf( f, "%s%s", module_t().dir, file_path ); // build absolute path
	for(auto& c:f) if(c=='\\') c='/'; // backslash to slash in Linux
	return f;
#endif
}

//*************************************
// common structures
struct mem_t
{
	char*	ptr = nullptr;
	size_t	size = 0;
};

struct vertex // will be used for all the course examples
{
	vec3 pos;	// position
	vec3 norm;	// normal vector; we will use this for vertex color for this example
	vec2 tex;	// texture coordinate; ignore this for the moment
};

struct image
{
	unsigned char*	ptr = nullptr; // image content
	union { struct { int width, height; }; ivec2 size={}; };
	int		channels = 0;

	image() : width(0), height(0){}
	~image(){ if(ptr) free(ptr); }
	GLint internal_format(){ return channels==1?GL_R8:channels==2?GL_RG8:channels==3?GL_RGB8:GL_RGBA8; }
	GLint format(){ return channels==1?GL_RED:channels==2?GL_RG:channels==3?GL_RGB:GL_RGBA; }
};

struct mesh
{
	std::vector<vertex>	vertex_list;
	std::vector<uint>	index_list;
	GLuint	vertex_buffer = 0;
	GLuint	index_buffer = 0;
	GLuint	vertex_array = 0;
	GLuint	texture = 0;

	~mesh()
	{
		if(vertex_buffer) glDeleteBuffers(1, &vertex_buffer);
		if(index_buffer) glDeleteBuffers(1, &index_buffer);
		if(vertex_array) glDeleteVertexArrays(1,&vertex_array);
	}
};

//*************************************
// utility functions
inline mem_t cg_read_binary( const char* file_path )
{
	mem_t m;
	FILE* fp = fopen( file_path, "rb" ); if(fp==nullptr){ printf( "%s(): unable to open %s\n", __func__, file_path ); return mem_t(); }
	fseek( fp, 0L, SEEK_END);
	m.size = size_t(ftell(fp));
	m.ptr = (char*) malloc(m.size+1);		// +1 for string
	fseek( fp, 0L, SEEK_SET );				// reset file pointer
	if(m.ptr)
	{
		fread( m.ptr, m.size, 1, fp );
		memset(m.ptr+m.size,0,sizeof(char));	// for string
	}
	fclose(fp);
	return m;
}

inline char* cg_read_shader( const char* file_path )
{
	file_path = absolute_path(file_path); // get the full path of a shader file
	if(access(file_path,0)!=0){ printf( "%s(): %s not exists\n", __func__, file_path ); return nullptr; }
	return cg_read_binary( file_path ).ptr;
}

inline bool cg_validate_shader( GLuint shaderID, const char* shaderName )
{
	const int MAX_LOG_LENGTH=4096;
	static char msg[MAX_LOG_LENGTH] = {0};
	GLint shaderInfoLogLength;

	glGetShaderInfoLog( shaderID, MAX_LOG_LENGTH, &shaderInfoLogLength, msg );
	if( shaderInfoLogLength>1 && shaderInfoLogLength<=MAX_LOG_LENGTH )
		printf( "[Shader: %s]\n%s\n", shaderName, msg );

	GLint shaderCompileStatus; glGetShaderiv( shaderID, GL_COMPILE_STATUS, &shaderCompileStatus);
	if(shaderCompileStatus==GL_TRUE) return true;

	glDeleteShader( shaderID );
	return false;
}

inline bool cg_validate_program( GLuint programID, const char* programName )
{
	const int MAX_LOG_LENGTH=4096;
	static char msg[MAX_LOG_LENGTH] = {0};
	GLint programInfoLogLength;

	glGetProgramInfoLog( programID, MAX_LOG_LENGTH, &programInfoLogLength, msg );
	if( programInfoLogLength>1 && programInfoLogLength<=MAX_LOG_LENGTH )
		printf( "[Program: %s]\n%s\n", programName, msg );

	GLint programLinkStatus; glGetProgramiv( programID, GL_LINK_STATUS, &programLinkStatus);
	if(programLinkStatus!=GL_TRUE){ glDeleteProgram(programID); return false; }

	glValidateProgram( programID );
	glGetProgramInfoLog( programID, MAX_LOG_LENGTH, &programInfoLogLength, msg );
	if( programInfoLogLength>1 && programInfoLogLength<=MAX_LOG_LENGTH )
		printf( "[Program: %s]\n%s\n", programName, msg );

	GLint programValidateStatus; glGetProgramiv( programID, GL_VALIDATE_STATUS, &programValidateStatus);
	if( programValidateStatus!=GL_TRUE ){ glDeleteProgram(programID); return false; }

	return true;
}

//*************************************
// monitor/screen-related
inline ivec4 cg_monitor()
{
	static ivec4 m={0,0,0,0}; if(m.z) return m;
	GLFWmonitor* mon=glfwGetPrimaryMonitor(); if(!mon) return ivec4(0);
	glfwGetMonitorPos(mon,&m.x,&m.y);
	auto* mode=glfwGetVideoMode(mon); if(!mode) return ivec4(0);
	m.z=mode->width; m.w=mode->height;
	return m;
}

inline ivec2 cg_load_window_pos( int window_width, int window_height )
{
	ivec4 m=cg_monitor();
	ivec2 p0=ivec2((m.z-window_width)/2,(m.w-window_height)/2), p=p0;
#ifdef CGUT_MSVC
	FILE* fp = fopen( module_t().conf_path(), "r" ); if(fp){ ivec2 r; if(2==fscanf( fp, "[window]\nx = %d\ny = %d\n", &r.x, &r.y )) p=r; fclose(fp); }
	if(p.x<0||p.x>=m.z||p.y<0||p.y>=m.w) p=p0;
#endif
	return p+ivec2(m.x,m.y);
}

inline void cg_save_window_pos( GLFWwindow*& window )
{
#ifdef CGUT_MSVC
	if(!window||!glfwGetWindowAttrib(window,GLFW_VISIBLE)) return;
	int x,y; glfwGetWindowPos(window,&x,&y); ivec4 m=cg_monitor(); x-=m.x;y-=m.y;
	FILE* fp=fopen(module_t().conf_path(),"w"); if(fp){ fprintf( fp, "[window]\nx = %d\ny = %d\n",x,y); fclose(fp); }
#endif
}

inline const ivec2 cg_default_window_size()
{
#ifdef GL_ES_VERSION_2_0
	return ivec2( 576, 1024 );	// initial window size similarly to smartphone
#else
	return ivec2( 720, 480 );	// initial window size
#endif
}

inline GLFWwindow* cg_create_window( const char* name, int& width, int& height, int version_major=gl_version_t::major_default, int version_minor=gl_version_t::minor_default, bool show_window=true )
{
	// register error callback for GLFW
	static auto cg_glfw_error = []( int error_code, const char* desc ){ printf( "[glfw] error(%d): %s\n", error_code, desc ); };
	glfwSetErrorCallback(cg_glfw_error);

	// initialization
	if(GLFW_TRUE!=glfwInit()){ printf( "%s(): failed in glfwInit()\n", __func__ ); return nullptr; }

	// give GLFW hints for window and OpenGL context
	gl_version_t& v = gl_version_t::instance();
	glfwWindowHint( GLFW_RESIZABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE );
	glfwWindowHint( GLFW_SCALE_TO_MONITOR, GLFW_TRUE );				// high-dpi support
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, version_major );	// minimum requirement
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, version_minor );	// minimum requirement
	if(v.is_gles())
	{
		glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
	}
	else if(version_major>=3&&version_minor>=3)
	{
		glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );				// core profile (>=3.3) allow only forward-compatible profile
		glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );	// create core profile; all legacy deprecated
	}
	
	// create a windowed mode window and its OpenGL context
	GLFWwindow* win = glfwCreateWindow( width, height, name, nullptr, nullptr ); if(!win){ printf( "Failed to create a GLFW window.\n" ); glfwTerminate(); return nullptr; }
	glfwGetWindowSize( win, &width, &height ); // update window size to dpi-aware size

	// get monitor size and locate window in the center
	ivec2 win_pos = cg_load_window_pos(width,height);
	glfwSetWindowPos( win, win_pos.x, win_pos.y );
	printf( "Window placement: %dx%d at (%d,%d)\n", width, height, win_pos.x, win_pos.y );

	// make context and show window
	glfwMakeContextCurrent(win);
	if(show_window) glfwShowWindow( win );

	return win;
}

inline void cg_destroy_window( GLFWwindow*& window )
{
	if(!window) return; // bypass invalid window

	cg_save_window_pos( window );
	glfwDestroyWindow(window); window = nullptr;
	glfwTerminate();
}

inline bool cg_init_extensions( GLFWwindow* window )
{
	glfwMakeContextCurrent(window);	// make sure the current context again

#ifdef GL_ES_VERSION_2_0
	if(!gladLoadGLES2(glfwGetProcAddress)){ printf( "%s(): Failed in gladLoadGLES2()\n", __func__ ); glfwTerminate(); return false; }
#else
	if(!gladLoadGL(glfwGetProcAddress)){ printf( "%s(): Failed in gladLoadGL()\n", __func__ ); glfwTerminate(); return false; }
#endif

	// print GL/GLSL versions
	gl_version_t& v = gl_version_t::instance();
	printf( "OpenGL %s on %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER) );
	glGetIntegerv(GL_MAJOR_VERSION, &v.major );
	glGetIntegerv(GL_MINOR_VERSION, &v.minor ); while(v.minor>10) v.minor/=10;
	const char* strGLSLver = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	if( v.gl()<20 ) printf( "Warning: OpenGL version may be too low to run modern-style OpenGL\n" );

	// warning for lower version
#ifdef GL_ES_VERSION_2_0
	if(strGLSLver){ sscanf( strGLSLver, "OpenGL ES GLSL ES %d.%d", &v.sl.major, &v.sl.minor ); while(v.sl.minor>=10) v.sl.minor/=10; }
#else
	if(strGLSLver){ ivec2 t; if(2==sscanf( strGLSLver, "%d.%d", &t.x, &t.y )){ v.sl.major = t.x; v.sl.minor = t.y; while(v.sl.minor>=10) v.sl.minor/=10; } }
#endif
	if(v.gl()!=v.glsl()) printf( "%s%s\n", v.is_gles()?"":"OpenGL GLSL ", strGLSLver );

	// load and check extensions
	std::set<std::string> ext_set;
#ifdef GL_ES_VERSION_2_0
	size_t ext_len = strlen((const char*)glGetString(GL_EXTENSIONS));
	std::vector<char> ext((char*)glGetString(GL_EXTENSIONS),(char*)glGetString(GL_EXTENSIONS)+ext_len+2);
	for( char* t=strtok( &ext[0], " \t\n" ); t; t=strtok( nullptr, " \t\n" ) ) ext_set.insert(t);
#else
	GLint next; glGetIntegerv(GL_NUM_EXTENSIONS,&next);
	for( GLint k=0; k<next; k++) ext_set.insert((const char*)glGetStringi(GL_EXTENSIONS,k));
	#define CHECK_GL_EXT(ext) if(ext_set.find( "GL_ARB_"#ext )==ext_set.end() ) printf( "Warning: GL_ARB_" #ext " not supported.\n" );
		CHECK_GL_EXT( vertex_buffer_object );	// BindBuffers, DeleteBuffers, GenBuffers, IsBuffer, BufferData, BufferSubData, GenBufferSubData, ...
		CHECK_GL_EXT( vertex_array_object );	// BindBuffers, DeleteBuffers, GenBuffers, IsBuffer, BufferData, BufferSubData, GenBufferSubData, ...
		CHECK_GL_EXT( vertex_shader );			// functions related to vertex shaders
		CHECK_GL_EXT( fragment_shader );		// functions related to fragment shaders
		CHECK_GL_EXT( shader_objects );			// functions related to program and shaders
	#undef CHECK_GL_EXT
#endif

	return true;
}

inline const char* shader_type_name( GLenum shader_type )
{
	if( shader_type==0x8B31 ) return "vertex shader";
	if( shader_type==0x8B30 ) return "fragment shader";
	if( shader_type==0x8DD9 ) return "geometry shader";
	if( shader_type==0x8E88 ) return "tess control shader";
	if( shader_type==0x8E87 ) return "tess evaluation shader";
	if( shader_type==0x91B9 ) return "compute shader";
	return "unknown shader_type";
}

inline bool strstr( const char* src, std::initializer_list<const char*> substr )
{
	for( auto* s : substr ) if(strstr(src,s)) return true;
	return false;
}

inline GLuint cg_create_shader( const char* shader_source, GLenum shader_type, std::string& log )
{
	if(!shader_source){ printf( "%s(): shader_source == nullptr\n", __func__ ); return 0; }

	std::string stn = std::string("[")+shader_type_name(shader_type)+"]";
	std::string src = shader_source;
	std::string macro;

	if(!strstr(shader_source,"#version"))
	{
		gl_version_t& v = gl_version_t::instance();
		char sver[1024]; sprintf( sver, "#version %d%s", v.glsl()*10, v.is_gles()?" es":"" ); macro += std::string(sver)+"\n";
		printf( "%-18s '%s' added automatically.\n", stn.c_str(), sver );
	}

	std::vector<const char*> src_list;
	std::vector<GLint> src_size_list;
	if(!macro.empty()){ src_list.push_back(macro.c_str()); src_size_list.push_back(GLint(macro.size())); }
	src_list.push_back(shader_source); src_size_list.push_back(GLint(strlen(shader_source)));

	GLuint shader = glCreateShader( shader_type );
	glShaderSource( shader, GLsizei(src_list.size()), &src_list[0], &src_size_list[0] );
	glCompileShader( shader );
	if(!cg_validate_shader( shader, shader_type_name(shader_type))){ printf( "Unable to compile %s\n", shader_type_name(shader_type) ); return 0; }

	return shader;
}

inline GLuint cg_create_program_from_string( const char* vertex_shader_source, const char* fragment_shader_source )
{
	// try to create a program
	GLuint program = glCreateProgram();
	glUseProgram( program );

	// create shaders
	std::string log;
	GLuint vertex_shader = cg_create_shader( vertex_shader_source, GL_VERTEX_SHADER, log );
	GLuint fragment_shader = cg_create_shader( fragment_shader_source, GL_FRAGMENT_SHADER, log );
	if(!log.empty()) printf( "%s\n", log.c_str() );
	if(!vertex_shader||!fragment_shader) return 0;

	// attach vertex/fragments shaders
	glAttachShader( program, vertex_shader );
	glAttachShader( program, fragment_shader );

	// try to link program
	glLinkProgram( program );
	if(!cg_validate_program( program, "program" )){ printf( "Unable to link program\n" ); return 0; }

	return program;
}

inline GLuint cg_create_program( const char* vert_path, const char* frag_path )
{
	const char* vertex_shader_source = cg_read_shader( vert_path ); if(vertex_shader_source==NULL) return 0;
	const char* fragment_shader_source = cg_read_shader( frag_path ); if(fragment_shader_source==NULL) return 0;

	// try to create a program
	GLuint program = cg_create_program_from_string( vertex_shader_source, fragment_shader_source );

	// deallocate string
	free((void*)vertex_shader_source);
	free((void*)fragment_shader_source);
	return program;
}

inline uint cg_create_vertex_array( uint vertex_buffer, uint index_buffer=0 )
{
	if(!vertex_buffer){ printf("%s(): vertex_buffer == 0\n", __func__ ); return 0; }

	// create and bind a vertex array object
	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	// bind vertex/index buffer
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
	if(index_buffer) glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );

	// bind vertex attributes by interpreting the organization of struct vertex
	static const size_t attrib_sizes[] = { sizeof(vertex::pos), sizeof(vertex::norm), sizeof(vertex::tex) };
	for( size_t k=0, byte_offset=0; k<std::extent<decltype(attrib_sizes)>::value; byte_offset+=attrib_sizes[k], k++ )
	{
		// We need to indicate the explicit binding of vertex attributes in vertex shader:
		// e.g., layout(location=0) in vec3 position;
		glEnableVertexAttribArray( GLuint(k) );
		glVertexAttribPointer( GLuint(k), GLint(attrib_sizes[k]/sizeof(GLfloat)), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*) byte_offset );
	}

	// unbind vao and return
	glBindVertexArray( 0 );
	return vao;
}

inline bool cg_load_vertices( const char* vert_binary_path, std::vector<vertex>* p_out_vertices )
{
	vert_binary_path = absolute_path(vert_binary_path);
	if(access(vert_binary_path,0)!=0){ printf( "%s(): %s not exists\n", __func__, vert_binary_path ); return false; }
	mem_t v = cg_read_binary(vert_binary_path); if(!v.ptr){ printf( "%s(): failed to read %s\n", __func__, vert_binary_path ); return false; }
	if(v.size%sizeof(vertex)){ printf( "%s(): %s is not a valid vertex binary file\n", __func__, vert_binary_path ); return false; }
	if(!p_out_vertices){ printf( "%s(): p_out_vertices == nullptr\n", __func__ ); return false; }
	p_out_vertices->resize( v.size/sizeof(vertex) );
	memcpy( (void*)p_out_vertices->data(), v.ptr, v.size );
	free(v.ptr);
	return true;
}

inline bool cg_load_indices( const char* index_binary_path, std::vector<uint>* p_out_indices )
{
	index_binary_path = absolute_path(index_binary_path);
	if(access(index_binary_path,0)!=0){ printf( "%s(): %s not exists\n", __func__, index_binary_path ); return false; }
	mem_t i = cg_read_binary(index_binary_path); if(!i.ptr){ printf( "%s(): failed to read %s\n", __func__, index_binary_path ); return false; }
	if(i.size%sizeof(uint)){ printf( "%s(): %s is not a valid index binary file\n", __func__, index_binary_path ); return false; }
	if(!p_out_indices){ printf( "%s(): p_out_indices == nullptr\n", __func__ ); return false; }
	p_out_indices->resize( i.size/sizeof(uint) );
	memcpy( p_out_indices->data(), i.ptr, i.size );
	free(i.ptr);
	return true;
}

inline mesh* cg_load_mesh( const char* vert_binary_path, const char* index_binary_path )
{
	mesh* new_mesh = new mesh();

	// load vertex/index buffers
	if(!cg_load_vertices( vert_binary_path, &new_mesh->vertex_list )) return nullptr;
	if(!cg_load_indices( index_binary_path, &new_mesh->index_list )) return nullptr;

	// create a vertex buffer
	glGenBuffers( 1, &new_mesh->vertex_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, new_mesh->vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*new_mesh->vertex_list.size(), &new_mesh->vertex_list[0], GL_STATIC_DRAW );

	// create a index buffer
	glGenBuffers( 1, &new_mesh->index_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, new_mesh->index_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*new_mesh->index_list.size(), &new_mesh->index_list[0], GL_STATIC_DRAW );

	// generate vertex array object, which is mandatory for OpenGL 3.3 and higher
	new_mesh->vertex_array = cg_create_vertex_array( new_mesh->vertex_buffer, new_mesh->index_buffer );
	if(!new_mesh->vertex_array){ printf("%s(): failed to create vertex aray\n",__func__); return nullptr; }

	return new_mesh;
}

#if defined(STBI_INCLUDE_STB_IMAGE_H)||defined(STBI_VERSION)
// assume stb_image.h included somewhere with STB_IMAGE_IMPLEMENTATION
extern "C" unsigned char* stbi_load(const char*,int*,int*,int*,int);

inline image* cg_load_image( const char* image_path )
{
	// validate path
	image_path = absolute_path(image_path);
	if(access(image_path,0)!=0){ printf("%s(): %s not exists\n", __func__, image_path); return nullptr; }
	if(access(image_path,4)!=0){ printf("%s(): %s does not have a read access\n", __func__, image_path); return nullptr; }

	int w, h, c; unsigned char* i0 = stbi_load( image_path, &w, &h, &c, 0 ); // load as is for channels
	if(i0&&c==1){ free(i0); i0 = stbi_load( image_path, &w, &h, &c, 3 ); c=3; } // load again gray-scale image as RGB
	if(!i0||!w||!h||!c){ printf("%s(): unable to load %s\n", __func__, image_path ); return nullptr; }

	image* i = new image;
	i->width = w;
	i->height = h;
	i->channels = c;
	
	ptrdiff_t stride0=ptrdiff_t(w)*c, stride1=(stride0+3)&(~3); // alignment at 4-byte boundary
	i->ptr = (unsigned char*) malloc( sizeof(unsigned char)*stride1*h );
	for( int y=0; y<h; y++ )
	{
		unsigned char* dst = i->ptr + stride1*(ptrdiff_t(h)-1-y);
		if(dst) memcpy( dst, i0+y*stride0, stride0 ); // copy row with vertical flip
	}
	free(i0); // release the stbi-loaded image; stbi_image_free==free() in regular cases

	return i;
}
#endif

inline GLuint cg_create_texture( image*	i, bool mipmap, GLenum wrap=GL_CLAMP_TO_EDGE, GLenum filter=GL_LINEAR )
{
	if(!i) return 0; // return null texture; 0 is reserved as a null texture
	int w=i->width, h=i->height, c=i->channels;

	// induce internal format and format from image
	GLint	internal_format = c==1?GL_R8:c==2?GL_RG8:c==3?GL_RGB8:GL_RGBA8;
	GLenum	format = c==1?GL_RED:c==2?GL_RG:c==3?GL_RGB:GL_RGBA;

	// create a src texture (lena texture)
	GLuint texture;
	glGenTextures( 1, &texture ); if(texture==0){ printf("%s(): failed in glGenTextures()\n", __func__ ); return 0; }
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexImage2D( GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, i->ptr );
	if(i){ delete i; i=nullptr; } // release image

	// build mipmap
	if( mipmap )
	{
		int mip_levels=0; for( int k=w>h?w:h; k; k>>=1 ) mip_levels++;
		for( int l=1; l < mip_levels; l++ )
			glTexImage2D( GL_TEXTURE_2D, l, internal_format, (w>>l)==0?1:(w>>l), (h>>l)==0?1:(h>>l), 0, format, GL_UNSIGNED_BYTE, nullptr );
		if(glGenerateMipmap) glGenerateMipmap(GL_TEXTURE_2D);
	}

	// set up texture parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, !mipmap?filter:filter==GL_LINEAR?GL_LINEAR_MIPMAP_LINEAR:GL_NEAREST_MIPMAP_NEAREST );

	return texture;
}

#if defined(STBI_INCLUDE_STB_IMAGE_H)||defined(STBI_VERSION)
inline GLuint cg_create_texture( const char* image_path, bool mipmap=true, GLenum wrap=GL_CLAMP_TO_EDGE, GLenum filter=GL_LINEAR )
{
	return cg_create_texture( cg_load_image( image_path ), mipmap, wrap, filter );
}
#endif

#endif // __CGUT_H__
