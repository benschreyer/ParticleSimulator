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
layout(std140, binding = 7) buffer PosOld
{
	vec4 PositionsOld[];
};
layout(std430, binding = 8) buffer Buck
{
	uint Buckets[];
};

uniform float deltaTime;
uniform float sideLength;
layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uint hashDim = uint(sideLength) / 10;
	//gl_GlobalInvocationID.x;
	PositionsOld[GID] = Positions[GID];
	Positions[GID].xyz = Positions[GID].xyz + Velocities[GID].xyz * deltaTime;
	if (abs(Velocities[GID].x) < 0.01)
		Velocities[GID].x = 0.0000;
	if (abs(Velocities[GID].y) < 0.01)
		Velocities[GID].y = 0.0000;
	if (abs(Velocities[GID].z) < 0.01)
		Velocities[GID].z = 0.0000;
	if (Positions[GID].x < 1)
	{
		Positions[GID].x = 1;
		//Velocities[GID].x = Velocities[GID].x * 0.9;
		//Velocities[GID].y = Velocities[GID].y * 0.9;
		Velocities[GID].x = Velocities[GID].x * -1.0;
		//Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 5.0;
	}
	if (Positions[GID].x > sideLength - 1)
	{
		Positions[GID].x = sideLength - 1;

		Velocities[GID].x = Velocities[GID].x * -1.0;
		//Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 5.0;
	}
	if (Positions[GID].y < 1)
	{
		Positions[GID].y = 1;
		//Velocities[GID].x = Velocities[GID].x * 0.9;
		//Velocities[GID].y = Velocities[GID].y * 0.9;
		Velocities[GID].y = Velocities[GID].y * -1.0;
		//Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 5.0;
	}
	if (Positions[GID].y > sideLength - 1)
	{
		Positions[GID].y = sideLength - 1;

		Velocities[GID].y = Velocities[GID].y * -1.0;
		//Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 5.01;
	}
	if(Positions[GID].z < 1 )
	{
		Positions[GID].z = 1;
		//Velocities[GID].x = Velocities[GID].x * 0.9;
		//Velocities[GID].y = Velocities[GID].y * 0.9;
		Velocities[GID].z = Velocities[GID].z * -1.0;
		//Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 5.01;
	}
	if (Positions[GID].z > sideLength - 1)
	{
		Positions[GID].z = sideLength - 1;

		Velocities[GID].z = Velocities[GID].z * -1.0;
		//Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 5.01;
	}
//	uint hash = hashDim * hashDim * (uint(Positions[GID].x) / hashDim) + hashDim * (uint(Positions[GID].y) / hashDim) + (uint(Positions[GID].z) / hashDim);
//	uint secondaryHash = uint(float(uint(Positions[GID].x) % 10) / 0.5) * 20 * 20 + uint(float(uint(Positions[GID].y) % 10) / 0.5) * 20 + uint(float(uint(Positions[GID].z) % 10) / 0.5);
	
	/*
		colors[i].r = 0.0; //(hash / (cubeSide * cubeSide)) / (float)cubeSide;
		colors[i].g = 0.0; //((hash % cubeSide) / (cubeSide)) / (float)cubeSide;
		colors[i].b = hash / (float)(cubeSide * cubeSide * cubeSide);//((hash % (cubeSide * cubeSide))) / (float)cubeSide;
		colors[i].a = 1.0;*/
	//Colors[GID].x = (hash / (hashDim * hashDim)) / float(hashDim);
	//Colors[GID].y = (hash % hashDim)  / float(hashDim);
	//Colors[GID].z = ((hash % (hashDim)) % hashDim) / float(hashDim);
	
	//Buckets[hash * 20 * 20 * 20 + secondaryHash] = GID + 1;
	if(Positions[GID].y > 1.01f)
	{
		//grav disabled
		Velocities[GID].xyz = Velocities[GID].xyz + vec3(0.0, -9.8, 0.0) * deltaTime * 0.0;
	}
	Colors[GID] = vec4(length(Velocities[GID]) * length(Velocities[GID]) * 0.00006 + 0.23, 0.0, 0.8 - (length(Velocities[GID]) * length(Velocities[GID]) * 0.00006 + 0.23), 1.0);
	//Velocities[GID].xyz -= Velocities[GID].xyz * deltaTime * 0.000001;
}	