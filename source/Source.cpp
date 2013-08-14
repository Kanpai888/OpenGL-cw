#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <stddef.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <istream>

using namespace std;
using namespace glm;

struct Vertex {
    GLfloat x, y, z;
	GLfloat normX, normY, normZ;
	GLfloat U, V;
};

const double PI = 3.141592;
int running = GL_TRUE;

/* These pointers will receive the contents of our shader source code files */
GLchar *vertexsource, *fragmentsource, *illuminatedVertSource, *illuminatedFragSource;
/* These are handles used to reference the shaders */
GLuint vertexshader, fragmentshader, illuminatedVertShader, illuminatedFragShader;
/* This is a handle to the shader program */
GLuint shaderprogram, illuminatedShaderProgram;

bool onTour = false;
bool animationReset = false;
int tourStage = 0;

/* Modified from tutorial 3 code to accept binary files */
char* filetobuf(string infile)
{
    FILE *fptr;
    long length;
    char *buf;

	const char* file = infile.c_str();

    fptr = fopen(infile.c_str(), "rb"); /* Open file for reading */
    if (!fptr){
        fprintf(stderr, "%s not found\n", file);
        exit(0);
    } /* Return NULL on failure */
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length + 1); /* Allocate a buffer for the entire length of the file plus a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */
	printf("%s \n\n", buf);

    return buf; /* Return the buffer */
}

//Modiefied from tutorial 3 code
void SetupShaders(void){
	char error[1000];
	int InfoLogLength;
	GLint Result = GL_FALSE;

    /* Read our shaders into the appropriate buffers */
    vertexsource = filetobuf("shader.vert");
    fragmentsource = filetobuf("shader.frag");
	
    /* Assign our handles a "name" to new shader objects */
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	
    /* Associate the source code buffers with each handle */
    glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
    glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	
    /* Compile our shader objects */
    glCompileShader(vertexshader);
    glCompileShader(fragmentshader);

	// Check Vertex Shader
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vertexshader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    glGetShaderInfoLog(vertexshader, InfoLogLength, NULL, error);
	if (InfoLogLength > 0) {
		fprintf(stdout, "%s\n", error);
	}

	 // Check Fragment Shader
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(fragmentshader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    glGetShaderInfoLog(fragmentshader, InfoLogLength, NULL, error);
    if (InfoLogLength > 0) {
		fprintf(stdout, "%s\n", error);
	}

    /* Assign our program handle a "name" */
    shaderprogram = glCreateProgram();
    glAttachShader(shaderprogram, vertexshader);/* Attach our shaders to our program */
    glAttachShader(shaderprogram, fragmentshader);
    glBindAttribLocation(shaderprogram, 0, "in_position"); /* Bind attribute 0 (coordinates) to in_Position */
    glLinkProgram(shaderprogram);/* Link our program, and set it as being actively used */


    //Initilisation of illuminated shader
	illuminatedVertSource = filetobuf("illuminated.vert");
	illuminatedFragSource = filetobuf("illuminated.frag");

	illuminatedVertShader = glCreateShader(GL_VERTEX_SHADER);
	illuminatedFragShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(illuminatedVertShader, 1, (const GLchar**)&illuminatedVertSource, 0);
	glShaderSource(illuminatedFragShader, 1, (const GLchar**)&illuminatedFragSource, 0);

	glCompileShader(illuminatedVertShader);
	glCompileShader(illuminatedFragShader);

	// Check Vertex Shader
    glGetShaderiv(illuminatedVertShader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(illuminatedVertShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    glGetShaderInfoLog(illuminatedVertShader, InfoLogLength, NULL, error);
	if (InfoLogLength > 0) {
		fprintf(stdout, "%s\n", error);
	}

	 // Check Fragment Shader
    glGetShaderiv(illuminatedFragShader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(illuminatedFragShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    glGetShaderInfoLog(illuminatedFragShader, InfoLogLength, NULL, error);
    if (InfoLogLength > 0) {
		fprintf(stdout, "%s\n", error);
	}

	illuminatedShaderProgram = glCreateProgram();
	glAttachShader(illuminatedShaderProgram, illuminatedVertShader);
	glAttachShader(illuminatedShaderProgram, illuminatedFragShader);
	glBindAttribLocation(illuminatedShaderProgram, 0, "in_position");
	glBindAttribLocation(illuminatedShaderProgram, 1, "in_normal");
	glLinkProgram(illuminatedShaderProgram);

	glGetProgramInfoLog(illuminatedShaderProgram, 1000, &InfoLogLength, error);
    if(InfoLogLength>0)
        fprintf(stderr, "%s\n", error);
}

//Adapted from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
bool readInObj(const char* filename, vector < Vertex > &outVertices){

		vector< glm::vec3 > tempVertices;
		vector< glm::vec3 > tempNormals;
		vector< glm::vec2 > tempUVS;
		vector< int > vertexIndices, normalIndices, uvIndices;
		unsigned int vertexIndex[3], normalIndex[3], uvIndex[3];

		FILE *file = fopen(filename, "rb");
		if( file == NULL){
			printf("A obj file could not be load");
			return false;
		}

		while(1){
			char lineHeader[128];
			//read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if(res == EOF){
				break; //End of file reached
			}else{
				if(strcmp (lineHeader, "v" ) == 0) { //Detecting Vertecies
					glm::vec3 vertex;
					fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
					//printf("Vertex: %f  %f  %f  \n", vertex.x, vertex.y, vertex.z);
					tempVertices.push_back(vertex);
				}else if(strcmp (lineHeader, "vn" ) == 0) { //Detecting Normals
					glm::vec3 normal;
					fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
					tempNormals.push_back(normal);
				}else if(strcmp (lineHeader, "vt" ) == 0) { //Detecting UV
					glm::vec2 uv;
					fscanf(file, "%f %f\n", &uv.x, &uv.y);
					tempUVS.push_back(uv);
				}else if (strcmp (lineHeader, "f" ) == 0){
					std::string vertex1, vertex2, vertex3;
					
					int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0],&uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
					if (matches != 9){
						printf("File read error \n");
						return false;
					}
					vertexIndices.push_back(vertexIndex[0]);
					vertexIndices.push_back(vertexIndex[1]);
					vertexIndices.push_back(vertexIndex[2]);
					normalIndices.push_back(normalIndex[0]);
					normalIndices.push_back(normalIndex[1]);
					normalIndices.push_back(normalIndex[2]);
					uvIndices.push_back(uvIndex[0]);
					uvIndices.push_back(uvIndex[1]);
					uvIndices.push_back(uvIndex[2]);
				}
			}
		}

		for(size_t i = 0; i < vertexIndices.size(); i++){

			glm::vec3 myPoints = tempVertices[ vertexIndices[i]-1 ];
			glm::vec3 myNormals = tempNormals[ normalIndices[i]-1 ];
			glm::vec2 myUVs = tempUVS[ uvIndices[i] -1 ];

			Vertex point = { myPoints.x, myPoints.y, myPoints.z, myNormals.x, myNormals.y, myNormals.z, myUVs.x, myUVs.y};
			outVertices.push_back(point);
		}
		return true;
}

//Adapted from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/#top
GLuint loadTGA_glfw(const char * tgaPath){
 
    GLuint textureSampleID;
    glGenTextures(1, &textureSampleID);
    glBindTexture(GL_TEXTURE_2D, textureSampleID);
    glfwLoadTexture2D(tgaPath, 0);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
 
    return textureSampleID;
}

struct BoundingBox{
	vec3 max, min;
};

vector<BoundingBox> allObjectBounds;

void createBoundingBox(vec3 inMin, vec3 inMax, vec3 scale, vec3 translation){
	printf("Min x: %f,  y: %f,  z: %f \n", inMin.x, inMin.y ,inMin.z);
	printf("Min x: %f,  y: %f,  z: %f \n", inMax.x, inMax.y, inMax.z);

	//Gives a bigger boundry so that camera will not look through object
	//Since we are only detecting camera to object collision it is acceptable
	float margin = 0.5;
	BoundingBox temp;
	temp.min.x = (inMin.x * scale.x) + translation.x - margin;
	temp.min.y = (inMin.y * scale.y) + translation.y;
	temp.min.z = (inMin.z * scale.z) + translation.z - margin;

	temp.max.x = (inMax.x * scale.x) + translation.x + margin;
	temp.max.y = (inMax.y * scale.y) + translation.y;
	temp.max.z = (inMax.z * scale.z) + translation.z + margin;

	printf("Min x: %f,  y: %f,  z: %f \n", temp.min.x, temp.min.y, temp.min.z);
	printf("Min x: %f,  y: %f,  z: %f \n", temp.max.x, temp.max.y, temp.max.z);
	allObjectBounds.push_back(temp);
}

bool hasCollided(vec3 inPos){
	for(size_t i=0; i < allObjectBounds.size(); i++){
		if(allObjectBounds[i].min.x < inPos.x && allObjectBounds[i].max.x > inPos.x &&
			allObjectBounds[i].min.y < inPos.y && allObjectBounds[i].max.y > inPos.y &&
			allObjectBounds[i].min.z < inPos.z && allObjectBounds[i].max.z > inPos.z)
		{
			return true;
		}

	}
	return false;
}

//Adapted from http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
class Camera{
	protected:
		glm::vec3 position; //position
		float horizontalAngle; //horizontal angle towards -Z
		float verticalAngle; //verticle angle 0, look at horizon
		float initialFoV; //initial field of view
		float speed;//units per second
		glm::mat4 ProjectionMatrix, ViewMatrix;
		float deltaX, deltaY, deltaZ, deltaHoriz, deltaVerti;
	public:
		Camera(){
			position = glm::vec3( -11.899700f, 2.018843f , 19.671196f );
			horizontalAngle = 2.605997f;
			verticalAngle = 0.0f;
			initialFoV = 45.0f;
			speed = 0.0f; 
		}

		void computeMatricesFromInputes(float deltaTime){

			glm::vec3 direction;
			if(!onTour){
				// Increase Speed
				if (glfwGetKey( GLFW_KEY_UP ) == GLFW_PRESS){
					if(speed <= 60.0f){
						speed += 1.0f;
					}	
				}
				// Decrease Speed
				if (glfwGetKey( GLFW_KEY_DOWN ) == GLFW_PRESS){
					if(speed >= 1.0f){
						speed -= 1.0f;
					}
				}
				// Rotate Right
				if (glfwGetKey( GLFW_KEY_RIGHT ) == GLFW_PRESS){
					horizontalAngle -= 0.003f;
				}
				// Rotate Left
				if (glfwGetKey( GLFW_KEY_LEFT ) == GLFW_PRESS){
					horizontalAngle += 0.003f;
				}
				if (glfwGetKey( GLFW_KEY_PAGEUP ) == GLFW_PRESS){
					if(verticalAngle < 1.0f){
						verticalAngle += 0.003f;
					}
				}
				if (glfwGetKey( GLFW_KEY_PAGEDOWN ) == GLFW_PRESS){
					if(verticalAngle > -1.0f){
						verticalAngle -= 0.003f;
					}
				}
			}

			//Compute Vector that represents in Word Space the direction in which we are facing
			 direction = vec3(
				cos(verticalAngle) * sin(horizontalAngle),
				sin(verticalAngle),
				cos(verticalAngle) * cos(horizontalAngle)
			);

			//Right vector
			glm::vec3 right = glm::vec3(
				sin(horizontalAngle - 3.14f/2.0f),
				0,
				cos(horizontalAngle - 3.14f/2.0f)
			);

			//Up vector : perpendicular to both direction and right
			glm::vec3 up = glm::cross( right, direction );

			ProjectionMatrix = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);

			if(!hasCollided(position + direction * deltaTime * speed)){
					position += direction * deltaTime * speed;
			}

			// Camera matrix
			ViewMatrix       = glm::lookAt(
				position,				 // Camera is here
				position+direction,		 // and looks here : at the same position, plus "direction"
				up						 // Head is up (set to 0,-1,0 to look upside-down)
			);	
		}

		void setViewMatrix(glm::vec3 inPos, float inHorizontalAngle, float inVerticalAngle){
			position = inPos;
			horizontalAngle = inHorizontalAngle;
			verticalAngle = inVerticalAngle;
		}

		
		void interpolateBetweenTwoPoints(vec3 startPos, vec2 startDir, vec3 endPos, vec2 endDir, float duration, float startTime){

			float percent = (float)(glfwGetTime() - startTime)/duration;

			if(percent >= 1){
				percent = 1;
			}
			deltaX = std::abs(endPos.x - startPos.x);
			deltaY = std::abs(endPos.y - startPos.y);
			deltaZ = std::abs(endPos.z - startPos.z);
			deltaHoriz = std::abs(endDir.x - startDir.x);
			deltaVerti = std::abs(endDir.y - startDir.y);

			if(startPos.x > endPos.x){position.x = startPos.x - (deltaX * percent) ;}
			else{position.x = (deltaX * percent) + startPos.x;}
			
			if(startPos.y > endPos.y){position.y = startPos.y - (deltaY * percent) ;}
			else{position.y = (deltaY * percent) + startPos.y;}

			if(startPos.z > endPos.z){position.z = startPos.z - (deltaZ * percent) ;}
			else{position.z = (deltaZ * percent) + startPos.z;}

			if(startDir.x > endDir.x){horizontalAngle = startDir.x - (deltaHoriz * percent) ;}
			else{horizontalAngle = (deltaHoriz * percent) + startDir.x;}

			if(startDir.y > endDir.y){verticalAngle = startDir.y - (deltaVerti * percent) ;}
			else{verticalAngle = (deltaVerti * percent) + startDir.y;}
			
		}

		glm::vec3 getPosition(){
			return position;
		}

		glm::mat4 getProjectionMatrix(){
			return ProjectionMatrix;
		}

		glm::mat4 getViewMatrix(){
			return ViewMatrix;
		}

		void printInfo(){
			printf("%ff, %ff , %ff,  horzi: %f,   verti: %f  \n", position.x, position.y, position.z, horizontalAngle, verticalAngle);
		}

		void resetSpeed(){
			speed = 0.0f;
		}
};

Camera myCamera;



class ColoredObject {
	protected:
		GLuint vao, vbo[3]; 
		vector<Vertex> VertexPoints;
		GLuint texture;
		vec3 max, min;
	public:
		ColoredObject(const char* filename, const char* texturePath){
			//Generating the Vertex Array Objects and Buffers
			glGenVertexArrays(1, &vao);
			glGenBuffers(3, vbo);

			bool isRead = readInObj(filename, VertexPoints);
			if(isRead){

				glBindVertexArray(vao);
				//Vertex
				glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
				glBufferData ( GL_ARRAY_BUFFER, VertexPoints.size() * sizeof ( struct Vertex ), &VertexPoints[0], GL_STATIC_DRAW );
				glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE,  sizeof(Vertex), (const GLvoid*)offsetof(Vertex, x) );

				//Normal
				glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
				glBufferData ( GL_ARRAY_BUFFER, VertexPoints.size() * sizeof ( struct Vertex ), &VertexPoints[0], GL_STATIC_DRAW );
				glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE,  sizeof(Vertex), (const GLvoid*)offsetof(Vertex, normX) );

				//UV
				glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
				glBufferData ( GL_ARRAY_BUFFER, VertexPoints.size() * sizeof ( struct Vertex ), &VertexPoints[0], GL_STATIC_DRAW );
				glVertexAttribPointer ( 2, 2, GL_FLOAT, GL_FALSE,  sizeof(Vertex), (const GLvoid*)offsetof(Vertex, U) );

				//Remember to enable the attrib arrays!!!!
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);

				//Cleanup 
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				//Loading in texture file
				texture = loadTGA_glfw(texturePath);

				//Assumes that the center point of a object in 0,0,0
				max = vec3(0.0f, 0.0f, 0.0f);
				min = vec3(0.0f, 0.0f, 0.0f);
				for(size_t i=0; i<VertexPoints.size(); i++){
					if(VertexPoints[i].x < min.x){
						min.x = VertexPoints[i].x;
					}
					if(VertexPoints[i].x > max.x){
						max.x = VertexPoints[i].x;
					}
					if(VertexPoints[i].y < min.y){
						min.y = VertexPoints[i].y;
					}
					if(VertexPoints[i].y > max.y){
						max.y = VertexPoints[i].y;
					}
					if(VertexPoints[i].z < min.z){
						min.z = VertexPoints[i].z;
					}
					if(VertexPoints[i].z > max.z){
						max.z = VertexPoints[i].z;
					}
				}

			}else{
				printf("Obj file has failed to read \n");
			}
		}

		void render(glm::vec3 scale, glm::vec3 rotationAxis, float angle, glm::vec3 translation){

			// Get a handle for our "myTextureSampler" uniform
			glUseProgram(illuminatedShaderProgram);
			glBindVertexArray(vao);			

			GLuint textureID  = glGetUniformLocation(illuminatedShaderProgram, "myTextureSampler");
			GLuint modelMatrixID = glGetUniformLocation(illuminatedShaderProgram, "M");
			GLuint MatrixID = glGetUniformLocation(illuminatedShaderProgram, "MVP");
			GLuint LightPosID = glGetUniformLocation(illuminatedShaderProgram, "lightPos_worldspace");
			GLuint viewMatrixID = glGetUniformLocation(illuminatedShaderProgram, "V");

			glm::mat4 Model = glm::mat4(1.0f); //Centre point of model
			Model = glm::translate(Model, translation);
			Model = glm::rotate(Model, angle, rotationAxis); 
			Model = glm::scale(Model, scale); 
			
			glm::vec3 lightPos = glm::vec3(0,15,10);	
			//glm::vec3 lightPos = myCamera.getPosition();
			glm::mat4 MVP = myCamera.getProjectionMatrix() * myCamera.getViewMatrix() * Model;	

			// Bind  texture in texture unit 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			glUniform1i(textureID, 0);
			glUniform3f(LightPosID, lightPos.x, lightPos.y, lightPos.z);
			glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &myCamera.getViewMatrix()[0][0]);
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &Model[0][0]);
			
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(MVP));

			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glDrawArrays(GL_TRIANGLES, 0, VertexPoints.size());
		}

		void renderWireframe(glm::vec3 scale, glm::vec3 rotationAxis, float angle, glm::vec3 translation, glm::vec3 fragColor, bool isPoints){
			glUseProgram(shaderprogram);
			glBindVertexArray(vao);		

			GLuint MatrixID = glGetUniformLocation(shaderprogram, "MVP");
			GLuint colorID = glGetUniformLocation(shaderprogram, "fragColor");

			glm::mat4 Model = glm::mat4(1.0f); //Centre point of model
			Model = glm::translate(Model, translation);
			Model = glm::rotate(Model, angle, rotationAxis); 
			Model = glm::scale(Model, scale); 
						

			glm::mat4 MVP = myCamera.getProjectionMatrix() * myCamera.getViewMatrix() * Model;

			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(MVP));
			glUniform3f(colorID, fragColor.x, fragColor.y, fragColor.z);

			if(isPoints){
				glPolygonMode( GL_FRONT_AND_BACK, GL_POINT);
			}else{
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			}
			glDrawArrays(GL_TRIANGLES, 0, VertexPoints.size());
		}

		vec3 getMax(){
			return max;
		}

		vec3 getMin(){
			return min;
		}
};

 
vec3 getInterpolatedPoint(vec3 startPos, vec3 endPos,  float duration, float startTime){

	float percent = (float)(glfwGetTime() - startTime)/duration;

	if(percent >= 1.0f){percent = 1.0f;};
			
	float deltaX = std::abs(endPos.x - startPos.x);
	float deltaY = std::abs(endPos.y - startPos.y);
	float deltaZ = std::abs(endPos.z - startPos.z);

	vec3 out;

	if(startPos.x > endPos.x){out.x = startPos.x - (deltaX * percent) ;}
	else{out.x = (deltaX * percent) + startPos.x;}
			
	if(startPos.y > endPos.y){out.y = startPos.y - (deltaY * percent) ;}
	else{out.y = (deltaY * percent) + startPos.y;}

	if(startPos.z > endPos.z){out.z = startPos.z - (deltaZ * percent) ;}
	else{out.z = (deltaZ * percent) + startPos.z;}

	return out;
}

double elapse = 0;
void GLFWCALL My_Key_Callback(int key, int action){
	if (key == GLFW_KEY_ESC && action == GLFW_PRESS){
		running = GL_FALSE;
		printf("ESC - Program closing \n");
	}else if (key == 'Q' && action == GLFW_PRESS){
		printf("Q - Program closing \n");
		running = GL_FALSE;
	}else{ 
		if(!onTour){
			if (key == 'T' && action == GLFW_PRESS){
				printf("T - Start Tour, ignore all except E, Q and ESC \n");
				onTour = true;
				animationReset = true;
				tourStage = 0;
			}else if (key == 'P' && action == GLFW_PRESS){
				printf("P - Move to predefined screenshot location \n");
				myCamera.setViewMatrix(glm::vec3(-28.848560f, 10.000000f , 25.864367f), 2.305994f, -0.120000f);	
			}else if (key == 'Y' && action == GLFW_PRESS){
				printf("Y - Switch to alternative view point 1 \n");
				myCamera.setViewMatrix(glm::vec3(0.201208f, 0.987757f , 10.615110f), 2.995999f, 0.210000f);	
			}else if (key == 'U' && action == GLFW_PRESS){
				printf("U - Switch to alternative view point 2 \n");
				myCamera.setViewMatrix(glm::vec3(-23.136635f, 9.297966f , 8.757370f), 2.086993f, -0.186000f);	
			}else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
				//printf("Space - Stop motion \n ");
				//myCamera.printInfo();
				//printf("Time:  %f \n", glfwGetTime());
			}else if (key == 'R' && action == GLFW_PRESS){
				printf("R - Reset all animation \n ");
				animationReset = true;
			}
		}else{
			if (key == 'E' && action == GLFW_PRESS){
				printf("E - Exit tour mode \n");
				myCamera.resetSpeed();
				onTour = false;
			}
		}
	}
}




//Adapted from tutorial 3
int main( void ) {
   
    if( !glfwInit() ) {
		fprintf(stdout, "GLFW did not initialize \n");
        exit( EXIT_FAILURE );
    }
    if( !glfwOpenWindow( 800,800, 0,0,0,0,0,0, GLFW_WINDOW ) ) {
		fprintf(stdout, "Window could not be opened \n");
        glfwTerminate();
        exit( EXIT_FAILURE );
    }
    if(glewInit() != GLEW_OK){
		glfwTerminate();
		fprintf(stdout, "Glew did not initialize \n");
	}
	glfwSetKeyCallback(My_Key_Callback);
	
	SetupShaders();
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	
	myCamera = Camera();

	ColoredObject map = ColoredObject("map.obj", "map.tga");
	ColoredObject platform = ColoredObject("platform.obj", "red.tga");
	ColoredObject orb = ColoredObject("orb.obj", "red.tga");
	ColoredObject orbTower = ColoredObject("orbTower.obj", "yellow.tga");
	ColoredObject floatText = ColoredObject("text.obj", "orange.tga");
	ColoredObject powerTower = ColoredObject("powerTower.obj", "yellow.tga");
	ColoredObject powerTowerRing = ColoredObject("powerTowerRing.obj", "orange.tga");
	ColoredObject ship = ColoredObject("ship.obj", "orange.tga");
	ColoredObject scannerTower = ColoredObject("scanner.obj", "yellow.tga");
	ColoredObject robot = ColoredObject("robot.obj", "orange.tga");

	//Creating boundry boxes for collision detection
	createBoundingBox(powerTower.getMin(), powerTower.getMax(), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -0.5f, 0.0f)); //Centre tower bounding box
	createBoundingBox(map.getMin(), map.getMax(), glm::vec3(32.0f, 32.0f, 32.0f), glm::vec3(0.0f, -0.6f, 0.0f)); //Map
	createBoundingBox(platform.getMin(), platform.getMax(),glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, -1.2f, 0.0f)); //Platform
	createBoundingBox(orbTower.getMin(), orbTower.getMax(), glm::vec3(0.34f, 0.34f, 0.34f), glm::vec3(15.0f, -0.55f, 0.0f));//East tower
	createBoundingBox(orbTower.getMin(), orbTower.getMax(), glm::vec3(0.34f, 0.34f, 0.34f), glm::vec3(-15.0f, -0.55f, 0.0f));//West tower
	createBoundingBox(robot.getMin(), robot.getMax(), glm::vec3(1.0f, 1.00f, 1.0f), glm::vec3(0.0f, -0.50f, 3.0f)); //Robot 1
	createBoundingBox(robot.getMin(), robot.getMax(), glm::vec3(1.0f, 1.00f, 1.0f), glm::vec3(3.0f, -0.50f, 4.0f)); //Robot 2
	createBoundingBox(robot.getMin(), robot.getMax(), glm::vec3(1.0f, 1.00f, 1.0f),glm::vec3(-3.0f, -0.50f, 0.0f)); //Robot 3

	double lastTime;
	double currentTime;
	double timeDelta;
	
	float angle = 0;
	float starAngle = 0;
	float shipMovement = 0;
	float loading = 5.f;

	bool docking = false;
	bool shipLoaded = false;

	double startTime;
	double shipStart1;
	double shipStart2;
	int shipStage1=0;
	int shipStage2=0;

	vec3 startPos;
	vec3 endPos;
	vec2 startDir;
	vec2 endDir;

	lastTime = glfwGetTime();
    do{
		if(animationReset == true){
			angle = 0;
			starAngle = 0;
			shipMovement = 0;
			loading = 5.f;
			tourStage = 0;
			docking = false;
			shipLoaded = false;
			animationReset = false;
			shipStage1 = 0;
			shipStage2 = 0;
		}
		currentTime = glfwGetTime();
		timeDelta = currentTime - lastTime;

		if(docking){
			loading += 3.00f * (float)timeDelta;			
		}else{
			shipMovement += 15.00f * (float)timeDelta;
		}
		angle += 30.0f * (float)timeDelta;
		starAngle += 5.0f * (float)timeDelta;
		
		if(onTour){
			if(tourStage == 0){
				startTime = glfwGetTime();
				elapse = 0.0f;
				tourStage++;
			}
			if(tourStage == 1){
				if(elapse <= 10.0f){
					elapse +=  timeDelta;
					startPos = vec3(-36.782089f, 61.682175f, 28.978712f);
					startDir = vec2(2.42994f, -1.002003f);

					endPos = vec3(-10.951618f, 0.881441f, 14.037151f);
					endDir = vec2(2.479995f, 0.153000f);

					myCamera.interpolateBetweenTwoPoints(startPos, startDir, endPos, endDir, 10.0f, (float)startTime);
				}else{
					tourStage++;
					startTime = glfwGetTime();
					elapse = 0.0f;
				}
			}else if(tourStage == 2){
				if(elapse <= 10.0f){
					elapse +=  timeDelta;
					startPos =  vec3(-10.951618f, 0.881441f, 14.037151f);
					startDir = vec2(2.479995f, 0.153000f);

					endPos = vec3(15.637814f, 11.962977f, 21.365538f);
					endDir = vec2(3.791004f, -0.219000f);

					myCamera.interpolateBetweenTwoPoints(startPos, startDir, endPos, endDir, 10.0f, (float)startTime);
				}else{
					tourStage++;
					startTime = glfwGetTime();
					elapse = 0.0f;
				}
			}else if(tourStage == 3){
				if(elapse <= 4.0f){
					elapse +=  timeDelta;
					startPos =  vec3(15.637814f, 11.962977f, 21.365538f);
					startDir = vec2(3.791004f, -0.219000f);

					endPos = vec3(15.637814f, 11.962977f, 21.365538f);
					endDir = vec2(3.19700f, -0.21900f);

					myCamera.interpolateBetweenTwoPoints(startPos, startDir, endPos, endDir, 4.0f, (float)startTime);
				}else{
					tourStage++;
					startTime = glfwGetTime();
					elapse = 0.0f;
				}
			}else if(tourStage == 4){
				if(elapse <= 2.0f){
					elapse +=  timeDelta;
					startPos =  vec3(15.637814f, 11.962977f, 21.365538f);
					startDir = vec2(3.19700f, -0.21900f);

					endPos = vec3(15.637814f, 11.962977f, 21.365538f);
					endDir = vec2(3.197000f, 0.153000f);

					myCamera.interpolateBetweenTwoPoints(startPos, startDir, endPos, endDir, 2.0f, (float)startTime);
				}else{
					tourStage++;
					startTime = glfwGetTime();
					elapse = 0.0f;
				}
			}
		}



		myCamera.computeMatricesFromInputes(float(timeDelta));

		//Render objects on screen- scale, rotation axis, angle, translate, color
		map.render(
			glm::vec3(32.0f, 32.0f, 32.0f), 
			glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 
			glm::vec3(0.0f, -0.6f, 0.0f));
		platform.render(
			glm::vec3(5.0f, 5.0f, 5.0f), 
			glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 
			glm::vec3(0.0f, -1.2f, 0.0f));
		
		//Eastern tower
		orb.renderWireframe(
			glm::vec3(1.0f, 1.0f, 1.0f), 
			glm::vec3(1.0f, 1.0f, 1.0f), angle, 
			glm::vec3(15.0f, 7.0f, 0.0f), 
			glm::vec3(0.6f, 0.f, 0.f),
			false);
		orbTower.render(
			glm::vec3(0.34f, 0.34f, 0.34f), 
			glm::vec3(1.0f, 1.0f, 1.0f), 0.f, 
			glm::vec3(15.0f, -0.55f, 0.0f));

		//Western tower
		orb.renderWireframe(
			glm::vec3(1.0f, 1.0f, 1.0f), 
			glm::vec3(1.0f, 1.0f, 1.0f), angle, 
			glm::vec3(-15.0f, 7.0f, 0.0f), 
			glm::vec3(0.6f, 0.f, 0.f), 
			false);
		orbTower.render(
			glm::vec3(0.34f, 0.34f, 0.34f), 
			glm::vec3(1.0f, 1.0f, 1.0f), 0.f, 
			glm::vec3(-15.0f, -0.55f, 0.0f));

		//North tower
		scannerTower.render(
			glm::vec3(1.0f, 1.00f, 1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), angle, 
			glm::vec3(0.0f, -0.55f, -15.0f));

		//South tower
		scannerTower.render(
			glm::vec3(1.0f, 1.00f, 1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), 360-angle, 
			glm::vec3(0.0f, -0.55f, 15.0f));

		//Robots
		robot.render(
			glm::vec3(1.0f, 1.00f, 1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), 30, 
			glm::vec3(0.0f, -0.50f, 3.0f));

		robot.render(
			glm::vec3(1.0f, 1.00f, 1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), -30, 
			glm::vec3(3.0f, -0.50f, 4.0f));

		robot.render(
			glm::vec3(1.0f, 1.00f, 1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), -43, 
			glm::vec3(-3.0f, -0.50f, 0.0f));

		//Stars in the sky
		orb.renderWireframe(
			glm::vec3(50.0f, 50.0f, 50.0f), 
			glm::vec3(1.0f, 1.0f, 1.0f), starAngle, 
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(1.0f, 1.0f, 1.0f), 
			true);

		//Centre Tower
		powerTower.render(
			glm::vec3(1.0f, 1.0f, 1.0f), 
			glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 
			glm::vec3(0.0f, -0.5f, 0.0f));
		powerTowerRing.render(
			glm::vec3(1.0f, 1.0f, 1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), angle, 
			glm::vec3(0.0f, 1.9f, 0.0f));
		powerTowerRing.render(
			glm::vec3(1.0f, 1.0f, 1.0f), 
			glm::vec3(0.0f, 1.0f, 0.0f), 360.0f-angle, 
			glm::vec3(0.0f, 2.4f, 0.0f));
		floatText.render(
			glm::vec3(1.5f, 1.5f, 1.5f), 
			glm::vec3(0.0f, 1.0f, 0.0f), 360.0f-angle, 
			glm::vec3(0.0f, 4.9f, 0.0f));

		if(docking){
			orb.renderWireframe(
				glm::vec3(0.7f, 0.7f, 0.7f), 
				glm::vec3(1.0f, 1.0f, 1.0f), angle, 
				glm::vec3(0.0f, loading, 0.0f), 
				glm::vec3(0.6f, 0.f, 0.f), 
				false);
			ship.render(
				glm::vec3(1.0f, 1.0f, 1.0f), 
				glm::vec3(0.0f, 1.0f, 0.0f), angle, 
				glm::vec3(0.0f, 9.0f, 0.0f));
		}else{
			if(shipMovement >= 50.0f){
				orb.renderWireframe(
					glm::vec3(0.7f, 0.7f, 0.7f), 
					glm::vec3(1.0f, 1.0f, 1.0f), angle, 
					glm::vec3(shipMovement-50.0f, 9.0f, 0.0f), 
					glm::vec3(0.6f, 0.f, 0.f), 
					false);
			}
			ship.render(
				glm::vec3(1.0f, 1.0f, 1.0f), 
				glm::vec3(0.0f, 1.0f, 0.0f), angle, 
				glm::vec3(shipMovement-50.0f, 9.0f, 0.0f));
		}
		
		//Ships in sky
		if(shipStage1 == 0){shipStart1 = glfwGetTime(); shipStage1++;}	
		if(shipStage1 == 1){
			if(glfwGetTime() - shipStart1 < 7.0f){
				vec3 shipPos = getInterpolatedPoint(
						vec3(18.702641f, 18.948092f , -16.510452f),
						vec3(-11.655668f, 16.675585f , -5.951130f),
						7.0f,
						(float)shipStart1
						);
				ship.render(
					glm::vec3(1.0f, 1.0f, 1.0f), 
					glm::vec3(0.0f, 1.0f, 0.0f), angle, 
					shipPos);
				orb.renderWireframe(
					glm::vec3(0.7f, 0.7f, 0.7f), 
					glm::vec3(1.0f, 1.0f, 1.0f), angle, 
					shipPos, 
					glm::vec3(0.6f, 0.f, 0.f), 
					false);
			}else{shipStage1++; shipStart1 = glfwGetTime();}
		}
		if(shipStage1 == 2){
			if(glfwGetTime() - shipStart1 < 5.0f){
				vec3 shipPos = getInterpolatedPoint(
						vec3(-11.655668f, 16.675585f , -5.951130f),
						vec3(8.261437f, 38.611156f , 14.102474f),
						5.0f,
						(float)shipStart1
						);
				ship.render(
					glm::vec3(1.0f, 1.0f, 1.0f), 
					glm::vec3(0.0f, 1.0f, 0.0f), angle, 
					shipPos);
				orb.renderWireframe(
					glm::vec3(0.7f, 0.7f, 0.7f), 
					glm::vec3(1.0f, 1.0f, 1.0f), angle, 
					shipPos, 
					glm::vec3(0.6f, 0.f, 0.f), 
					false);
			}else{shipStage1++; shipStart1 = glfwGetTime();}
		}
		if(shipStage1 == 3){
			if(glfwGetTime() - shipStart1 < 4.0f){
				vec3 shipPos = getInterpolatedPoint(
						vec3(8.261437f, 38.611156f , 14.102474f),
						vec3(18.702641f, 18.948092f , -16.510452f),
						4.0f,
						(float)shipStart1
						);
				ship.render(
					glm::vec3(1.0f, 1.0f, 1.0f), 
					glm::vec3(0.0f, 1.0f, 0.0f), angle, 
					shipPos);
				orb.renderWireframe(
					glm::vec3(0.7f, 0.7f, 0.7f), 
					glm::vec3(1.0f, 1.0f, 1.0f), angle, 
					shipPos, 
					glm::vec3(0.6f, 0.f, 0.f), 
					false);
			}else{shipStage1 = 1; shipStart1 = glfwGetTime();}
		}	
		

		if(shipStage2 == 0){shipStart2 = glfwGetTime(); shipStage2++;}	
		if(shipStage2 == 1){
			if(glfwGetTime() - shipStart2 < 5.0f){
				vec3 shipPos = getInterpolatedPoint(
						vec3(-21.701416f, 8.311575f , -22.523884f),
						vec3(8.246375f, 16.367680f , -7.842208f),
						5.0f,
						(float)shipStart2
						);
				ship.render(
					glm::vec3(1.0f, 1.0f, 1.0f), 
					glm::vec3(0.0f, 1.0f, 0.0f), angle, 
					shipPos);
				orb.renderWireframe(
					glm::vec3(0.7f, 0.7f, 0.7f), 
					glm::vec3(1.0f, 1.0f, 1.0f), angle, 
					shipPos, 
					glm::vec3(0.6f, 0.f, 0.f), 
					false);
			}else{shipStage2++; shipStart2 = glfwGetTime();}
		}
		if(shipStage2 == 2){
			if(glfwGetTime() - shipStart2 < 7.0f){
				vec3 shipPos = getInterpolatedPoint(
						vec3(8.246375f, 16.367680f , -7.842208f),
						vec3(-18.942865f, 57.289284f , -15.025329f),
						7.0f,
						(float)shipStart2
						);
				ship.render(
					glm::vec3(1.0f, 1.0f, 1.0f), 
					glm::vec3(0.0f, 1.0f, 0.0f), angle, 
					shipPos);
				orb.renderWireframe(
					glm::vec3(0.7f, 0.7f, 0.7f), 
					glm::vec3(1.0f, 1.0f, 1.0f), angle, 
					shipPos, 
					glm::vec3(0.6f, 0.f, 0.f), 
					false);
			}else{shipStage2++; shipStart2 = glfwGetTime();}
		}
		if(shipStage2 == 3){
			if(glfwGetTime() - shipStart2 < 6.0f){
				vec3 shipPos = getInterpolatedPoint(
						vec3(-18.942865f, 57.289284f , -15.025329f),
						vec3(-21.701416f, 8.311575f , -22.523884f),
						6.0f,
						(float)shipStart2
						);
				ship.render(
					glm::vec3(1.0f, 1.0f, 1.0f), 
					glm::vec3(0.0f, 1.0f, 0.0f), angle, 
					shipPos);
				orb.renderWireframe(
					glm::vec3(0.7f, 0.7f, 0.7f), 
					glm::vec3(1.0f, 1.0f, 1.0f), angle, 
					shipPos, 
					glm::vec3(0.6f, 0.f, 0.f), 
					false);
			}else{shipStage2 = 1; shipStart2 = glfwGetTime();}
		}	
		
		



		lastTime = glfwGetTime();
		glfwSwapBuffers();	
        
		//Clear screen and Z buffer
		glClearColor(0.0, 0.0, 0.0f, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(angle > 360.0f){
			angle = 0;
		}

		if(starAngle > 360.0f){
			starAngle = 0;
		}

		if(loading >= 9.0f){
			loading = 5.0f;
			docking = false;
			shipLoaded = true;
		}

		if(shipMovement > 50 && !shipLoaded){
			docking = true;
		}

		if(shipMovement >= 100){
			shipMovement = 0;
			shipLoaded = false;
		}
    }while( running == GL_TRUE &&  glfwGetWindowParam( GLFW_OPENED ));
	
    glfwTerminate();
    exit( EXIT_SUCCESS );
}
