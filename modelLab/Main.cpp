// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.


// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Project includes
#include "maths_funcs.h"
#include <SOIL2/SOIL2.h>
#include "camera.h"

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define GLOBE "C:/Users/Caoimhe/Documents/graphics/modelLab/models/globe.dae"
#define STAND "C:/Users/Caoimhe/Documents/graphics/modelLab/models/stand.dae"
#define BEARING "C:/Users/Caoimhe/Documents/graphics/modelLab/models/ball_bearing.dae"
#define BASE "C:/Users/Caoimhe/Documents/graphics/modelLab/models/base.dae"
#define TABLE "C:/Users/Caoimhe/Documents/graphics/modelLab/models/table.dae"
#define CARPET "C:/Users/Caoimhe/Documents/graphics/modelLab/models/carpet.dae"
#define ROOM "C:/Users/Caoimhe/Documents/graphics/modelLab/models/room.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
} ModelData;
#pragma endregion SimpleTypes
std::vector<float> mTextureCoords;

using namespace std;
GLuint shaderProgramID;
GLuint VAO[8];

ModelData mesh_data, mesh_data1, mesh_data2, mesh_data3, mesh_data4, mesh_data5, mesh_data6;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;
mat4 translation = identity_mat4();
mat4 rotation = identity_mat4();
mat4 rotation2 = identity_mat4();
mat4 camera = identity_mat4();

GLuint textures[5];
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f, rotate_y_2 = 0.0f;

//Camera camera;


#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}
	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				mTextureCoords.push_back(vt->x);
				mTextureCoords.push_back(vt->y);
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "C:/Users/Caoimhe/Documents/graphics/modelLab/simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "C:/Users/Caoimhe/Documents/graphics/modelLab/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(ModelData mesh_num) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	//mesh_data = load_mesh(MESH_NAME);
	unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_num.mPointCount * sizeof(vec3), &mesh_num.mVertices[0], GL_STATIC_DRAW);
	unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_num.mPointCount * sizeof(vec3), &mesh_num.mNormals[0], GL_STATIC_DRAW);

	unsigned int vt_vbo = 0;
	glGenBuffers(1, &vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_num.mPointCount * 2 * sizeof(float), &mTextureCoords[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	int texture_num_loc = glGetUniformLocation(shaderProgramID, "texture_num");
	int light_pos_loc = glGetUniformLocation(shaderProgramID, "light_pos_z");

	mat4 model = identity_mat4();
	mat4 view = translate(camera, vec3(0.0, 0.0, -4.0));
	view = view * camera;
	//mat4 view = look_at(vec3(10.0, 10.0, 10.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glUniform1f(light_pos_loc, -30);
	glUniform1i(texture_num_loc, 2);
	glBindVertexArray(VAO[4]);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data4.mPointCount);

	mat4 modelChild = identity_mat4();
	modelChild = modelChild * translation;
	// Apply the root matrix to the child matrix
	mat4 Child1 = model * modelChild;

	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Child1.m);
	glBindVertexArray(VAO[3]);
	glUniform1i(texture_num_loc, 1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data3.mPointCount);

	mat4 modelChild2 = identity_mat4();
	modelChild2 = translate(modelChild2, vec3(0.0, -0.12, 0.0));
	mat4 Child2 = model * modelChild * modelChild2;

	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Child2.m);
	glBindVertexArray(VAO[2]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data2.mPointCount);

	mat4 modelChild3 = identity_mat4();
	modelChild3 = modelChild3 * rotation;
	modelChild3 = translate(modelChild3, vec3(0.0, 0.12, 0.0));
	mat4 Child3 = model * modelChild * modelChild2 * modelChild3;


	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Child3.m);
	glBindVertexArray(VAO[1]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data1.mPointCount);

	mat4 modelChild4 = identity_mat4();
	mat4 Child4 = model * modelChild * modelChild2 * modelChild3 * modelChild4;


	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Child4.m);
	glBindVertexArray(VAO[2]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data2.mPointCount);

	mat4 modelChild5 = identity_mat4();
	modelChild5 = translate(modelChild5, vec3(-0.12, 0.0, -0.12)) * rotate_y_deg(modelChild5, rotate_y) * translate(modelChild5, vec3(0.12, 0.0, 0.12));
	mat4 Child5 = model * modelChild * modelChild2 * modelChild3 * modelChild4 * modelChild5;


	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, Child5.m);
	glBindVertexArray(VAO[0]);
	glUniform1i(texture_num_loc, 0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

	mat4 carpet = identity_mat4();
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, carpet.m);
	glBindVertexArray(VAO[5]);
	glUniform1i(texture_num_loc, 3);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data5.mPointCount);

	mat4 room = identity_mat4();
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, room.m);
	glBindVertexArray(VAO[6]);
	glUniform1i(texture_num_loc, 4);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data6.mPointCount);
	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 10.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);

	rotate_y_2 += 30.0f * delta;
	rotate_y_2 = fmodf(rotate_y_2, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}

void loadTextures(GLuint texture, const char* filepath, int active_arg, const GLchar* texString, int texNum) {
	int x, y, n;
	int force_channels = 4;
	unsigned char *image_data = stbi_load(filepath, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", filepath);

	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
			filepath);
	}

	glActiveTexture(active_arg);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		image_data);
	glUniform1i(glGetUniformLocation(shaderProgramID, texString), texNum);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

}


void init()
{
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	glGenVertexArrays(7, VAO);
	glBindVertexArray(VAO[0]);
	mesh_data = load_mesh(GLOBE);
	generateObjectBufferMesh(mesh_data);

	glBindVertexArray(VAO[1]);
	mesh_data1 = load_mesh(STAND);
	generateObjectBufferMesh(mesh_data1);

	glBindVertexArray(VAO[2]);
	mesh_data2 = load_mesh(BEARING);
	generateObjectBufferMesh(mesh_data2);

	glBindVertexArray(VAO[3]);
	mesh_data3 = load_mesh(BASE);
	generateObjectBufferMesh(mesh_data3);

	glBindVertexArray(VAO[4]);
	mesh_data4 = load_mesh(TABLE);
	generateObjectBufferMesh(mesh_data4);

	glBindVertexArray(VAO[5]);
	mesh_data5 = load_mesh(CARPET);
	generateObjectBufferMesh(mesh_data5);

	glBindVertexArray(VAO[6]);
	mesh_data6 = load_mesh(ROOM);
	generateObjectBufferMesh(mesh_data6);


	glGenTextures(5, textures);

	loadTextures(textures[0], "C:/Users/Caoimhe/Documents/graphics/modelLab/models/world_map.jpg", GL_TEXTURE0, "map", 0);

	loadTextures(textures[1], "C:/Users/Caoimhe/Documents/graphics/modelLab/models/stone2.jpg", GL_TEXTURE1, "stone", 1);

	loadTextures(textures[2], "C:/Users/Caoimhe/Documents/graphics/modelLab/models/wood.jpg", GL_TEXTURE2, "wood", 2);

	loadTextures(textures[3], "C:/Users/Caoimhe/Documents/graphics/modelLab/models/carpet.jpg", GL_TEXTURE3, "carpet", 3);

	loadTextures(textures[4], "C:/Users/Caoimhe/Documents/graphics/modelLab/models/paint.jpg", GL_TEXTURE4, "paint", 4);
	// load mesh into a vertex buffer array
	//generateObjectBufferMesh();*/

}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	if (key == 'x') {
		//Translate the base, etc.
		translation = translate(translation, vec3(1.0f, 0.0f, 0.0f));
	}

	if (key == 'z') {
		//Translate the base, etc.
		rotation = rotate_y_deg(rotation,0.3);

	}

	if (key == 'w')
	{
		camera = translate(camera, vec3(0.0, 0.0, 0.1));
	}

	if (key == 's')
	{
		camera = translate(camera, vec3(0.0, 0.0, -0.1));
	}

	if (key == 'a')
	{
		camera = translate(camera, vec3(0.1, 0.0, 0.0));
	}

	if (key == 'd')
	{
		camera = translate(camera, vec3(-0.1, 0.0, 0.0));
	}

	if (key == 'q')
	{
		camera = translate(camera, vec3(0.0, 0.1, 0.0));
	}

	if (key == 'e')
	{
		camera = translate(camera, vec3(0.0, -0.1, 0.0));
	}
	if (key == 'r')
	{
		camera = rotate_x_deg(camera, 10);
	}
	if (key == 't')
	{
		camera = rotate_x_deg(camera, -10);
	}
	if (key == 'f')
	{
		camera = rotate_y_deg(camera, 10);
	}
	if (key == 'g')
	{
		camera = rotate_y_deg(camera, -10);
	}
	if (key == 'y')
	{
		camera = rotate_z_deg(camera, 10);
	}
	if (key == 'u')
	{
		camera = rotate_z_deg(camera, -10);
	}
	
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("SCENE");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
