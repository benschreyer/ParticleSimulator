#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#define GID gl_GlobalInvocationID.x
//EULER METHOD PROPOGATE PARTICLES
layout(std140, binding = 4) buffer Pos
{
	vec4 Positions[];
};
layout(std140, binding = 5) buffer Vel
{
	vec4 Velocities[];
};
layout(std140, binding = 6) buffer Col
{
	vec4 Colors[];
};

layout(std430, binding = 8) buffer Buck
{
	uint Buckets[];
};

uniform float deltaTime;
//uniform float sideLength;
uniform float xSizeB;
uniform float ySizeB;
uniform float zSizeB;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
	//Euler's method for position 
	Positions[GID].xyz = Positions[GID].xyz + Velocities[GID].xyz * deltaTime;
	//collision check with walls and reverse direction and apply friction and correct position if it collided
	if (Positions[GID].x < 1)
	{
		Positions[GID].x = 1.0067656;
		Velocities[GID].x = Velocities[GID].x * -1.0;
		Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 45.0;
	}
	if (Positions[GID].x > xSizeB - 1)
	{
		Positions[GID].x = xSizeB - 1.00874222;
		Velocities[GID].x = Velocities[GID].x * -1.0;
		Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime *45.0;
	}
	if (Positions[GID].y < 1)
	{
		Positions[GID].y = 1.012467;
		Velocities[GID].y = Velocities[GID].y * -1.0;
		Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 45.0;
	}

	if (Positions[GID].y > ySizeB - 1)
	{
		Positions[GID].y = ySizeB - 1.00452;
		Velocities[GID].y = Velocities[GID].y * -1.0;
		Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 45.01;
	}
	if (Positions[GID].z < 1)
	{
		Positions[GID].z = 1.008123;
		Velocities[GID].z = Velocities[GID].z * -1.0;
		Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 35.01;
	}
	if (Positions[GID].z > zSizeB - 1)
	{
		Positions[GID].z = zSizeB - 1.00431111243;
		Velocities[GID].z = Velocities[GID].z * -1.0;
		Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 35.01;
	}
	//Euler's method gravity if it is not touching the floor
	if(Positions[GID].y > 1.001f)
	{
	
		Velocities[GID].y -= 9.8 * deltaTime;

    }
	
}	
