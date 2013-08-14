#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 vertexUV;

out vec3 pos_worldspace;
out vec3 eyeDirection_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 normal_cameraspace;
out vec2 UV;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos_worldspace;

//http://www.opengl-tutorial.org/beginners-tutorials/tutorial-8-basic-shading/#The_Specular_component
void main(void) {
	vec4 v = vec4(in_position, 1.0);   
	gl_Position =  MVP * v;

	//Position of the vertex in worldspace
	pos_worldspace = (M * vec4(in_position, 1)).xyz;

	//Vector from the Vertex to the camera, in camera space.
	vec3 vertex_pos_cameraspace = ( V * M * vec4(in_position, 1)).xyz;
	eyeDirection_cameraspace = vec3(0,0,0) - vertex_pos_cameraspace;

	//Vector that goes from the vertex to the light source in camera space.
	vec3 lightPosition_cameraspace = (V * vec4(lightPos_worldspace, 1)).xyz;
	lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace;

	//Normal of the vertex in camera space
	normal_cameraspace = ( V * M * vec4(in_normal, 0)).xyz;

	UV = vertexUV;
}