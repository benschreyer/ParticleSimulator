#shader vertex
#version 430 core
layout(std140, binding = 4) buffer Pos
{
	vec4 Positions[];
};
layout(std140, binding = 6) buffer Col
{
	vec4 Colors[];
};



layout(location = 0) in vec3 position;
out vec4 colorIn;

//out vec2 v_TexCoord;
//uniform mat4 u_M;
uniform mat4 u_V;
uniform mat4 u_P;


void main()
{



	mat4 modModel;
	// billboard the circle so it faces the camera and appears as a sphere
	modModel[3] = vec4(Positions[gl_InstanceID].xyz,1.0);



	modModel[2] = vec4(u_V[0][2], u_V[1][2], u_V[2][2], 0.0);
	modModel[1] = vec4(u_V[0][1], u_V[1][1], u_V[2][1], 0.0);
	modModel[0] = vec4(u_V[0][0], u_V[1][0], u_V[2][0], 0.0);

	

	
	gl_Position = (u_P * u_V * modModel) * vec4(position, 1.0);
	//change vertex color to make sphere shaded
	colorIn = vec4(Colors[gl_InstanceID].xyz * (1.0 + position.x) * (0.5 + position.y * 0.5) * 2.0, Colors[gl_InstanceID].w);

};
//color pixels
#shader fragment
#version 430 core
layout(location = 0) out vec4 color;

in vec4 colorIn;

void main()
{
	
	color = colorIn;
};