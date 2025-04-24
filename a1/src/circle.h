#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__

static const float VELOCITY_SCALE = 2.5f;			// SCALE for velocity

// This is class, all public
// _t: declare type name
struct circle_t
{
	// default circle value
	vec2	pos=vec2(0);				// 2D position for translation
	vec2    velocity = vec2(0);			// velocity
	float	radius=1.0f;				// radius
	float	theta=0.0f;					// rotation angle
	vec4	color;						// RGBA color in [0,1]
	mat4	model_matrix;				// modeling transformation
	float	lvoc = 0.0;					// level of collision

	// public functions
	void	update( float t, float dt, float x_bound, float y_bound, std::vector<circle_t>& circles);
	float   collide(const circle_t& other);
	float	collide(const std::vector<circle_t>& circles);
};

// why inline? 
// header에서 정의하면 inline으로 정의
// 한번만 정의하기 위해서
// no discard?
// Don't discard output result
// create circles
inline [[nodiscard]] std::vector<circle_t> create_circles( uint count, float x_bound, float y_bound )
{	
	// define circles vector
	std::vector<circle_t> circles;

	// loop over count
	for (uint k = 0,  kn = 1024, n=0; k < kn && n<count; k++) {
		// define circle
		circle_t c;

		// generate random instance of a circle ( see formula in HW1 pdf file )
		c.radius = randf(0.05f, 0.2f)*4 / float(sqrt(count));

		// random position according to x bound and y bound
		c.pos = vec2(
			randf(-x_bound + c.radius, x_bound - c.radius),
			randf(-y_bound + c.radius, y_bound - c.radius)
		);

		// test if any collision exists
		bool collision_exists = false;
		for (auto& d : circles )
			if (d.collide(c) > 0) { collision_exists = true; break; }

		if (collision_exists) continue; // collision exist
		
		// emplace circles now, since it avoids collision
		c.color = vec4(randf3(0.0f, 1.0f), 1.0f);
		c.velocity = randf2(-1.0f, 1.0f) * VELOCITY_SCALE;

		// push to circles
		circles.emplace_back(c);
		n++;
	}

	return circles;
}

// check single collision
inline float circle_t::collide(const circle_t& other)
{	
	if (&other == this) return 0;
	
	return (length(pos - other.pos) < radius + other.radius);
}

// check circles collision
inline float circle_t::collide( const std::vector<circle_t>& circles )
{
	float lvoc_max = 0;
	for (auto& c : circles)
	{
		if (&c == this) continue;	// skip current circle
		// check collide
		float l = this->collide(c); if (l <= 0) continue;

		lvoc_max = std::max(l, lvoc_max);
	}

	return lvoc_max;
}

// how to update radius? this is class update function
inline void circle_t::update( float t, float dt , float x_bound, float y_bound, std::vector<circle_t>& circles)
{
	// suppose t as a current time
	theta	= t;
	float c	= cos(theta), s=sin(theta);

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
		1, 0, 0, pos.x,
		0, 1, 0, pos.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	// circle movement
	pos += velocity * dt * VELOCITY_SCALE;

	// avoid collision with walls
	if (pos.x - radius < -x_bound) {		// left wall collision
		pos.x = -x_bound + radius;
		velocity.x = -velocity.x;
	}
	else if (pos.x + radius > x_bound) {	// right wall collision
		pos.x = x_bound - radius;
		velocity.x = -velocity.x;
	}
	if (pos.y - radius < -y_bound) {		// below wall collision
		pos.y = -y_bound + radius;
		velocity.y = -velocity.y;
	}
	else if (pos.y + radius > y_bound) {	// above wall collision
		pos.y = y_bound - radius;
		velocity.y = -velocity.y;
	}

	// avoid collision with other circles
	float collision_impact = 0.0f;

	for (auto& d : circles) {
		if (&d == this) continue; // skip current circle

		// calculate collision impact
		collision_impact = collide(d);

		if (collision_impact > 0) { // collision
			// check distance of center
			float d_center = length(pos - d.pos);
			if (d_center == 0.0f) continue;

			// two center's normal vector
			vec2 normal = (pos - d.pos) / d_center;

			// calculate relative Velocity between circles
			vec2 relativeVelocity = velocity - d.velocity;

			// change to scalar value
			float scalar = dot(relativeVelocity, normal);

			// two circles is going to collision
			if (scalar < 0) {
				// elastic collision
				velocity -= scalar * normal;
				d.velocity += scalar * normal;
			}
		}
	}

	// M = TRS
	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

#endif