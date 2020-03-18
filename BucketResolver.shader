#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#define GID gl_GlobalInvocationID.x
//define needed buffers
layout(std140, binding = 4) buffer Pos
{
	vec4 Positions[];
};
layout(std140, binding = 5) buffer Vel
{
	vec4 Velocities[];
};

layout(std430, binding = 8) buffer Buck
{
	uint Buckets[];
};

uniform float xSizeB;
uniform float ySizeB;
uniform float zSizeB;
uniform float spheresPerCell;
uniform float cellSizeB;
uniform float deltaTime;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;




//dot product of two 3d vectors
float dotP(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void main()
{



	//for every sphere check it against others for collision as they are in the same general area of space and could be in collision
	for (unsigned int i = 0; i < uint(spheresPerCell); i++)
	{
		if (Buckets[i + GID * uint(spheresPerCell)] != 0)
		{
			for (unsigned int j = 0; j < uint(spheresPerCell); j++)
			{
				//if the sphere is valid to check collision ie it is not the same sphere and it is not a null/0 sphere that doesnt exist
				if (Buckets[j + GID * uint(spheresPerCell)] != 0 && Buckets[j + GID * uint(spheresPerCell)] != Buckets[i + GID * uint(spheresPerCell)])
				{
					//vector pointing from sphere i to sphere j
					vec3 cv = Positions[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz - Positions[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz;

					//find the square of the distance between the spheres
					float sqr = cv.x * cv.x + cv.y * cv.y + cv.z * cv.z;
					//check if they collide radius squared
					if (sqr < 4.0000)
					{
						//calculate how far the spheres must move to no longer collider
						vec3 mov = (normalize(Positions[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz - Positions[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz)) * (2 - sqrt(sqr)) * 0.5000;


						//move the spheres away from each other
						Positions[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz = Positions[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz + mov;
						Positions[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz = Positions[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz - mov;

						//find the direction of impact normal to the surfaces of both spheres and intersecting the point they touch
						vec3 vN = normalize(Positions[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz - Positions[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz);
						//transfer their velocities in the direction of the normal sicne they have the same mass
						vec3 prime = ((dot(Velocities[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz, vN)) - (dot(Velocities[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz, vN))) * vN;
						
						Velocities[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz -= 0.97 * prime;//newVel;//-1.0f * Velocities[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz;//newVel;
						Velocities[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz += 0.97 * prime;// newVel; //-1.0f * Velocities[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz;// newVel;

						//friction of collision, slow them down cuz they hit
						Velocities[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz -= (Velocities[Buckets[j + GID * uint(spheresPerCell)] - 1].xyz) * deltaTime * length(mov) * 55.5;
						Velocities[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz -= (Velocities[Buckets[i + GID * uint(spheresPerCell)] - 1].xyz) * deltaTime * length(mov) * 55.5;
					}
				}
			}
			//clear memory as buckets change from frame to frame
			Buckets[i + GID * uint(spheresPerCell)] = 0;
		}
		else
		{

			// stop checking for collisions if you are out of spheres to check
			break;
		}
	}

}