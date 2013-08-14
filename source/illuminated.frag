#version 330 core

precision highp float;

in vec3 pos_worldspace; 
in vec3 eyeDirection_cameraspace;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
in vec2 UV;

out vec3 color;

//uniform vec3 fragColor;
uniform vec3 lightPos_worldspace;
uniform sampler2D myTextureSampler;

//Adapted from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-8-basic-shading/#The_Specular_component
void main(void) {

	vec3 materialDiffuseColor = texture2D( myTextureSampler, UV ).rgb;
	vec3 materialAmbientColor = vec3(0.1,0.1,0.1) * materialDiffuseColor;
	vec3 materialSpecularColor = vec3(0.3,0.3,0.3);

	vec3 lightColor = vec3(1.0f,0.9f,1.0f);
	float lightPower = 300.0f;

	//Normal of the computed fragment
	vec3 n = normalize( normal_cameraspace );
	// Direction fragment to the light source
	vec3 l = normalize( lightDirection_cameraspace);

	float cosTheta = clamp( dot(n, l), 0, 1);
	float distance = length( lightPos_worldspace - pos_worldspace);

	//Eye Vector toward the camera
	vec3 E = normalize(eyeDirection_cameraspace);
	//Direction in which the triangle reflects the light
	vec3 R = reflect(-l, n);
	float cosAlpha = clamp( dot( E, R), 0, 1);

    color = 
		//Ambient lighting
		materialAmbientColor * materialAmbientColor +
		//Diffused lighting
		materialDiffuseColor * lightColor * lightPower * cosTheta / (distance * distance) +
		//Specular lighting
		materialSpecularColor * lightColor * lightPower * pow(cosAlpha, 5) / (distance*distance);

}