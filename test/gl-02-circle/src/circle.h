#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__

// class, all public
// _t: declare type name
struct circle_t
{
	// default circle value
	vec2	center=vec2(0);		// 2D position for translation
	float	radius=1.0f;		// radius
	float	theta=0.0f;			// rotation angle
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation
	
	// public functions
	void	update( float t );
};

// why inline? 
// header에서 정의하면 inline으로 정의
// 한번만 정의하기 위해서
// already defined
//inline float randf(float _min=0, float _max=1)
//{
//	// normalize random number 0 ~ 1
//	return rand() / float(RAND_MAX);
//}

// create circles
// nodiscard??
// return 값을 버리지 말것??
inline [[nodiscard]] std::vector<circle_t> create_circles( uint count )
{	
	// outcome을 regenerate하기 위해서 random number 고정
	// srand(0);

	std::vector<circle_t> circles;

	for (uint k = 0; k < count; k++) {
		// define circle
		circle_t c;

		// -1 ~ 1, -1 ~ 1
		// [rmin, rmax] ∝
		/*1√N*/
		c.radius = randf(0.05f, 0.2f)*4/float(sqrt(count));

		c.center = vec2(randf2(-0.8f, 0.8f));
		// c.color = vec4(randf(0.0f, 1.0f), randf(0.0f, 1.0f), randf(0.0f, 1.0f), randf(0.0f, 1.0f));
		c.color = vec4(randf3(0.0f, 1.0f), 1.0f);

		// c = { vec2(-0.5f,0),1.0f,0.0f,vec4(1.0f,0.5f,0.5f,1.0f) };

		// push to circles
		circles.emplace_back(c);
	}

	printf("circles = %zd\n", circles.size());

	return circles;
	
	/*c = {vec2(-0.5f,0),1.0f,0.0f,vec4(1.0f,0.5f,0.5f,1.0f)};
	circles.emplace_back(c);

	c = {vec2(+0.5f,0),1.0f,0.0f,vec4(0.5f,1.0f,1.0f,1.0f)};
	circles.emplace_back(c);*/
}

inline void circle_t::update( float t )
{
	// radius	= 0.35f+cos(t)*0.1f;		// simple animation
	theta	= t;
	float c	= cos(theta), s=sin(theta);

	// radius 어떻게 업데이트?
	// class update function임
	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		c,-s, 0, 0,
		s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

#endif
