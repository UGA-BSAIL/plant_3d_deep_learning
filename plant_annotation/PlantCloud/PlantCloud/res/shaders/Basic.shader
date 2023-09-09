#shader vertex
#version 410 core

layout(location = 0) in vec4 position; 
layout(location = 1) in vec4 pointcolor;

//layout(location = 1) in vec2 texCoords;

//out vec2 v_TexCoords;

out vec4 v_pointcolor;

uniform mat4 u_MVP;
//uniform vec4 u_Color;


void main() {

gl_Position = u_MVP * position;
gl_PointSize = 2;
v_pointcolor = pointcolor;
//v_TexCoords = texCoords;
};

#shader fragment
#version 410 core 

layout(location = 0) out vec4 color; 

//in vec2 v_TexCoords;
in vec4 v_pointcolor;


//uniform sampler2D u_Texture;


void main() {

	//vec4 texColor = texture(u_Texture, v_TexCoords);

	color =  v_pointcolor; // u_Color;// texColor; // u_Color;

};