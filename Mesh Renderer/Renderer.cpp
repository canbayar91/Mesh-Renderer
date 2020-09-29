#include "Angel.h"
#include "Face.h"
#include <string>
#include <vector>
#include <fstream>

using namespace Angel;

typedef vec4 point4;
typedef vec4 color4;

unsigned int mesh_count;
unsigned int vertice_count;
unsigned int face_count;

float zoom_multiplier = 1.6f;
unsigned int current_mesh = 0;
DrawingMode drawing_mode = FACE_MODE;
std::vector<std::string> mesh_list;

GLfloat angle = 0.0f;
GLfloat scale = 1.0f;
point4 location(0.0f, 0.0f, 0.0f, 1.0f);

GLfloat x_max = -100.0f;
GLfloat x_min = 100.0f;
GLfloat y_max = -100.0f;
GLfloat y_min = 100.0f;
GLfloat z_max = -100.0f;
GLfloat z_min = 100.0f;

point4 center;
GLfloat radius;

GLfloat projection_handle;
GLfloat angle_handle;
GLfloat scale_handle;
GLfloat position_handle;

void read_file(std::string filename, point4* &mesh, vec3* &normals) {

	// Create input stream
	std::ifstream file(filename);

	// Read the file format (always OFF)
	std::string format;
	file >> format;

	// Read the first line
	unsigned int ignored;
	file >> vertice_count >> face_count >> ignored;

	// Read each vertex one by one
	point4* vertices = new point4[vertice_count];
	for (unsigned int i = 0; i < vertice_count; i++) {
		float x, y, z;
		file >> x >> y >> z;
		vertices[i] = point4(x, y, z, 1.0);
	}

	// Read each face one by one
	Face* faces = new Face[face_count];
	for (unsigned int i = 0; i < face_count; i++) {
		int count, vertex0, vertex1, vertex2;
		file >> count >> vertex0 >> vertex1 >> vertex2;
		faces[i] = Face(vertex0, vertex1, vertex2);
	}

	// Create the mesh data using vertex and face information
	mesh = new point4[face_count * 3];
	normals = new vec3[face_count * 3];
	for (unsigned int i = 0; i < face_count; i++) {

		// Create a triangle for the face
		Face current = faces[i];
		mesh[i * 3] = vertices[current.get_vertex0()];
		mesh[i * 3 + 1] = vertices[current.get_vertex1()];
		mesh[i * 3 + 2] = vertices[current.get_vertex2()];

		// Calculate the face normal
		vec4 u = vertices[current.get_vertex1()] - vertices[current.get_vertex0()];
		vec4 v = vertices[current.get_vertex1()] - vertices[current.get_vertex2()];
		vec3 normal = normalize(cross(u, v));

		// Pass the normals for each vertex
		normals[i * 3] = normal;
		normals[i * 3 + 1] = normal;
		normals[i * 3 + 2] = normal;
	}

	// Pick 100 random points and find the approximate bounding box of the object
	for (unsigned int i = 0; i < 100; i++) {

		// Pick a random vertex
		int index = rand() % vertice_count;
		point4 current = vertices[index];

		// Decide the maximum and minimum values of x
		if (current.x > x_max) { 
			x_max = current.x; 
		} else if (current.x < x_min) { 
			x_min = current.x; 
		}

		// Decide the maximum and minimum values of y
		if (current.y > y_max) { 
			y_max = current.y; 
		} else if (current.y < y_min) { 
			y_min = current.y; 
		}

		// Decide the maximum and minimum values of z
		if (current.z > z_max) { 
			z_max = current.z; 
		} else if (current.z < z_min) { 
			z_min = current.z; 
		}
	}

	// Close the file
	file.close();

	// Release the resources from the memory
	delete[] vertices;
	delete[] faces;
}

void calculate_bounding_box() {

	// Calculate the middle point of the mesh
	center = point4((x_max + x_min) / 2.0f, (y_max + y_min) / 2.0f, (z_max + z_min) / 2.0f, 1.0f);

	// Calculate the radius of the mesh on the bigger side
	radius = abs(x_max - center.x);
	if (abs(x_min - center.x) > radius) radius = abs(x_min - center.x);
	if (abs(y_max - center.y) > radius) radius = abs(y_max - center.y);
	if (abs(y_min - center.y) > radius) radius = abs(y_min - center.y);
	if (abs(z_max - center.z) > radius) radius = abs(z_max - center.z);
	if (abs(z_min - center.z) > radius) radius = abs(z_min - center.z);
}

void reset_options() {

	// Reset the rendering mode
	drawing_mode = FACE_MODE;

	// Reset the rotation
	angle = 0.0f;

	// Reset the scale
	scale = 1.0f;

	// Reset the position
	location = point4(0.0f, 0.0f, 0.0f, 1.0f);

	// Reset the bounding box
	x_max = -100.0f;
	x_min = 100.0f;
	y_max = -100.0f;
	y_min = 100.0f;
	z_max = -100.0f;
	z_min = 100.0f;
}

void initialize(std::string filename) {

	// Reset the options each time a mesh is reloaded
	reset_options();

	// Initialize the mesh by reading the input file
	point4* mesh;
	vec3* normals;
	read_file(filename, mesh, normals);

	// Find the center and the radius of the bounding box
	calculate_bounding_box();

	// Create and bind a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a buffer object in GPU and place the data into the buffer
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point4) + sizeof(vec3)) * face_count * 3, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * face_count * 3, mesh);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * face_count * 3, sizeof(vec3) * face_count * 3, normals);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Initialize the vertex position attribute from the vertex shader
	GLuint location = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// Initialize the normal attribute from the vertex shader
	GLuint normal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(normal);
	glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4) * face_count * 3));

	// Generate random material color
	float red = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
	float green = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
	float blue = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
	color4 material(red, green, blue, 1.0);

	// Specify the position of light source
	point4 light_position(0.0, 0.0, -20.0, 0.0);

	// Specify the components of light source
	color4 light_ambient(0.2, 0.2, 0.2, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);

	// Specify the shininess coefficient for specular reflectivity
	float material_shininess = 100.0;

	// Calculate the products of lighting components
	color4 ambient_product = light_ambient * material;
	color4 diffuse_product = light_diffuse * material;
	color4 specular_product = light_specular * material;

	// Get the light values to the shader
	glUniform4fv(glGetUniformLocation(program, "ambientProduct"), 1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "diffuseProduct"), 1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "specularProduct"), 1, specular_product);
	glUniform4fv(glGetUniformLocation(program, "lightPosition"), 1, light_position);
	glUniform1f(glGetUniformLocation(program, "shininess"), material_shininess);

	// Get the uniform variables from the shader
	projection_handle = glGetUniformLocation(program, "projection");
	angle_handle = glGetUniformLocation(program, "theta");
	scale_handle = glGetUniformLocation(program, "scale");
	position_handle = glGetUniformLocation(program, "location");

	// Enable z-buffer algorithm for hidden surface removal
	glEnable(GL_DEPTH_TEST);

	// Use flat shading
	glShadeModel(GL_FLAT);

	// White background
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Release the mesh
	delete[] mesh;
	delete[] normals;
}

void display() {

	// Clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Load current angle and position to the vertex shader
	glUniform1f(angle_handle, angle);
	glUniform1f(scale_handle, scale);
	glUniform4fv(position_handle, 1, location);

	// Calculate the zoom amount
	GLfloat zoom = radius * zoom_multiplier;

	// Set the projection matrix and color in shaders
	mat4 projection_matrix = Ortho(center.x - zoom, center.x + zoom, center.y - zoom, center.y + zoom, center.z - zoom, center.z + zoom);
	glUniformMatrix4fv(projection_handle, 1, GL_TRUE, projection_matrix);

	// Render the points in GPU
	if (drawing_mode == VERTEX_MODE) {
		glDrawArrays(GL_POINTS, 0, face_count * 3);
	} else if (drawing_mode == EDGE_MODE) {
		for (unsigned int i = 0; i < face_count; i++) glDrawArrays(GL_LINE_LOOP, i * 3, 3);
	} else if (drawing_mode == FACE_MODE) {
		glDrawArrays(GL_TRIANGLES, 0, face_count * 3);
	}

	// Double buffering
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {

	// Keyboard mapping
	switch (key) {
	case 'v':
		drawing_mode = VERTEX_MODE;
		break;
	case 'e':
		drawing_mode = EDGE_MODE;
		break;
	case 'f':
		drawing_mode = FACE_MODE;
		break;
	case 'n':
		current_mesh == mesh_count - 1 ? current_mesh = 0 : current_mesh++;
		initialize(mesh_list[current_mesh]);
		break;
	case 'p':
		current_mesh == 0 ? current_mesh = mesh_count - 1 : current_mesh--;
		initialize(mesh_list[current_mesh]);
		break;
	case 'x':
		location.x += 0.1f;
		break;
	case 'X':
		location.x -= 0.1f;
		break;
	case 'y':
		location.y += 0.1f;
		break;
	case 'Y':
		location.y -= 0.1f;
		break;
	case 'z':
		location.z += 0.1f;
		break;
	case 'Z':
		location.z -= 0.1f;
		break;
	case 's':
		scale /= 2;
		break;
	case 'S':
		scale *= 2;
		break;
	case 'r':
		reset_options();
		break;
	}

	// Render the scene again
	glutPostRedisplay();
}

void arrow(int key, int x, int y) {

	// Arrow keys
	switch (key) {
	case GLUT_KEY_UP:
		zoom_multiplier -= 0.2f;
		break;
	case GLUT_KEY_DOWN:
		zoom_multiplier += 0.2f;
		break;
	case GLUT_KEY_LEFT:
		angle -= 2.0f;
		break;
	case GLUT_KEY_RIGHT:
		angle += 2.0f;
		break;
	}

	// Render the scene again
	glutPostRedisplay();
}

int main(int argc, char **argv) {

	// Read the command line inputs
	mesh_count = argc - 1;
	for (int i = 0; i < mesh_count; i++) {
		mesh_list.push_back(argv[i + 1]);
	}

	// Initialize window using GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(0, 0);

	// The lines below are used to check the version in case freeglut is used
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Create the window
	glutCreateWindow("Mesh Renderer");

	// Initialize GLEW
	glewInit();

	// Initialize the application with the first mesh
	initialize(mesh_list[0]);

	// Create a display function callback
	glutDisplayFunc(display);

	// Create a keyboard event listener
	glutKeyboardFunc(keyboard);

	// Create a special event listener for arrow keys
	glutSpecialFunc(arrow);

	// Keep the window open
	glutMainLoop();

	return 0;
}