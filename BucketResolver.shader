#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable
#define GID gl_GlobalInvocationID.x
//COLLISION RESOLVING
layout(std140, binding = 4) buffer Pos
{
	vec4 Positions[];
};
layout(std140, binding = 5) buffer Vel
{
	vec4 Velocities[];
};
layout(std140, binding = 7) buffer PosOld
{
	vec4 PositionsOld[];
};
layout(std430, binding = 8) buffer Buck
{
	uint Buckets[];
};
layout(local_size_x = 100, local_size_y = 1, local_size_z = 1) in;

uniform float deltaTime;
uniform float sideLength;
float dotP(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

void main()
{

	
		
	
		for (unsigned int i = 0; i < 200; i++)
		{
			if(Buckets[i + GID * 200] != 0)
			{ 
				for (unsigned int j = i; j < 200; j++)
				{
					if (Buckets[j + GID * 200] != 0 && Buckets[j + GID * 200]!= Buckets[i + GID * 200])
					{
						vec3 cv = Positions[Buckets[j + GID * 200] - 1].xyz - Positions[Buckets[i + GID * 200] - 1].xyz;
						float sqr = cv.x * cv.x + cv.y * cv.y + cv.z * cv.z;
						if (sqr < 4.0000)
						{
							float l = sqrt(sqr);
							//vec3 newVel = 0.50 * (Velocities[Buckets[j + GID * 200] - 1].xyz + Velocities[Buckets[i + GID * 200] - 1].xyz);
							vec3 mov = (normalize(Positions[Buckets[j + GID * 200] - 1].xyz - Positions[Buckets[i + GID * 200] - 1].xyz)) * (2 - l) * 0.5001;
							
							if ( Positions[Buckets[j + GID * 200] - 1].x < 1.1 || Positions[Buckets[j + GID * 200] - 1].x > sideLength || Positions[Buckets[j + GID * 200] - 1].y < 1.1 || Positions[Buckets[j + GID * 200] - 1].z > sideLength || Positions[Buckets[j + GID * 200] - 1].z < 1.1 || Positions[Buckets[j + GID * 200] - 1].z > sideLength)
							{
								Positions[Buckets[i + GID * 200] - 1].xyz = Positions[Buckets[i + GID * 200] - 1].xyz - 2.0 * mov;
							}
							else if ( Positions[Buckets[i + GID * 200] - 1].x < 1.1 || Positions[Buckets[i + GID * 200] - 1].x > sideLength-1.1 || Positions[Buckets[i + GID * 200] - 1].y < 1.1 || Positions[Buckets[i + GID * 200] - 1].z > sideLength || Positions[Buckets[i + GID * 200] - 1].z < 1.1 || Positions[Buckets[i + GID * 200] - 1].z > sideLength)
							{
								Positions[Buckets[j + GID * 200] - 1].xyz = Positions[Buckets[j + GID * 200] - 1].xyz +2.0 * mov;
							}
							else 
							{

								Positions[Buckets[j + GID * 200] - 1].xyz = Positions[Buckets[j + GID * 200] - 1].xyz + mov;
								Positions[Buckets[i + GID * 200] - 1].xyz = Positions[Buckets[i + GID * 200] - 1].xyz - mov;
							}
							vec3 vN = normalize(Positions[Buckets[j + GID * 200] - 1].xyz - Positions[Buckets[i + GID * 200] - 1].xyz);
							vec3 vPrimei = Velocities[Buckets[i + GID * 200] - 1].xyz - (dot(Velocities[Buckets[i + GID * 200] - 1].xyz, vN)) * vN + (dot(Velocities[Buckets[j + GID * 200] - 1].xyz, vN)) * vN;
							vec3 vPrimej = Velocities[Buckets[j + GID * 200] - 1].xyz + (dot(Velocities[Buckets[i + GID * 200] - 1].xyz, vN)) * vN - (dot(Velocities[Buckets[j + GID * 200] - 1].xyz, vN)) * vN;
							Velocities[Buckets[i + GID * 200] - 1].xyz = vPrimei;//newVel;//-1.0f * Velocities[Buckets[i + GID * 200] - 1].xyz;//newVel;
							Velocities[Buckets[j + GID * 200] - 1].xyz = vPrimej;// newVel; //-1.0f * Velocities[Buckets[j + GID * 200] - 1].xyz;// newVel;
							
						//	Velocities[Buckets[j + GID * 200] - 1].xyz -= Velocities[Buckets[j + GID * 200] - 1].xyz  * deltaTime * 3.5;
							//Velocities[Buckets[i + GID * 200] - 1].xyz -= Velocities[Buckets[i + GID * 200] - 1].xyz  * deltaTime * 3.5;
						}
					}
				}
				Buckets[i + GID * 200] = 0;
			}
			else
			{
				break;
			}
		}
	
}