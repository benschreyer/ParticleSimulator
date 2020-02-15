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

	//vec3 direction = normalize(u_center - u_pointAt);
	//vec3 pt = u_center + distance(u_center, position);

	mat4 modModel;

	modModel[3] = Positions[gl_InstanceID];
	modModel[2] = vec4(u_V[0][2], u_V[1][2], u_V[2][2], 0.0);
	modModel[1] = vec4(u_V[0][1], u_V[1][1], u_V[2][1], 0.0);
	modModel[0] = vec4(u_V[0][0], u_V[1][0], u_V[2][0], 0.0);


	gl_Position = (u_P * u_V * modModel) * vec4(position, 1.0);
	//mvp = projection * view * model2;
//v_TexCoord = texCoord;
	colorIn = Colors[gl_InstanceID];
};
#shader fragment
#version 430 core
layout(location = 0) out vec4 color;
//in vec2 v_TexCoord;
in vec4 colorIn;
//uniform vec4 u_Color;
//uniform sampler2D u_Texture;
void main()
{
	//vec4(0.65, 0.61, 0.43, 1.0); //
	//vec4 texColor = texture(u_Texture, v_TexCoord);
	color = colorIn;//vec4(0.65, 0.61, 0.43, 1.0);vec4(0.2,0.0,0.899,1.0);//colorIn;//
	//color = vec4(1.0, 0.5, 0.0, 1.0);
};