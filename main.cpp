#include "C:/Users/Caoimhe/Documents/graphics/modelLab/modelLab/all_includes.h"

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define GLOBE "C:/Users/Caoimhe/Documents/graphics/modelLab/globe.obj"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#include "Light.cpp"

float last_x = 0.0f, last_y = 0.0f;

float delta;
int width = 800;
int height = 600;

Light light;

void display() {

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	int light_location = glGetUniformLocation(shaderProgramID, "Light");
	int view_location = glGetUniformLocation(shaderProgramID, "view_pos");

	mat4 view = look_at(vec3(10.0, 10.0, 10.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
	mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

	light.draw(shader, matrix_location);
}

void updateScene() {
	// cap at 60 fps
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f) delta = 0.03f;
	last_time = curr_time;
	// Draw the next frame
	glutPostRedisplay();
}

void keypress(unsigned char key, int x, int y) {
}

void init() {
	shader = Shader((GLchar*)File::getAbsoluteShaderPath("simpleShader.vert").c_str(),
		(GLchar*)File::getAbsoluteShaderPath("simpleShader.frag").c_str());
}

int main(int argc, char** argv) {
	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("room view");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	//glutMouseFunc(mouseMove);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

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