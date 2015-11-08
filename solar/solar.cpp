/*
Lab3start
Creates a cube and a sphere
Includes rotation, scaling, translation, view and perspective transformations,
and is setup for you to add lighting because the objects have normals and colours defined
for all vertices.
Feel free to either use this example as a start to lab3 or extract bits and add to
an example of your own to practice implementing per-vertex lighting.
Iain Martin October 2014
*/


/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")


/* Include the header to the GLFW wrapper class which
also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <vector>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

/* Define buffer object indices */
GLuint positionBufferObject, colourObject, normalsBufferObject;
GLuint sphereBufferObject, sphereNormals, sphereColours;
GLuint elementbuffer;

GLuint program;		/* Identifier for the shader program */
GLuint vao;			/* Vertex array (Container) object. This is the index of the VAO that will be the container for
					our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					I've included this to show you how to pass in an unsigned integer into
					your vertex shader. */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scale, z, y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numLats, numLongs;	//Define the resolution of the sun object

/* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/
GLuint numSphereVertices;

/* Function prototypes */
/* Note that a better design would be to make a sphere class. I've suggested that as one of the
extras to do in the lab for this week. */
void makeUnitSphere(GLfloat *pVertices, GLuint numlats, GLuint numlongs);
GLuint makeSphereVBO(GLuint numlats, GLuint numlongs);
void drawSphere();

/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0.05f;
	y = 0;
	z = 0;
	angle_x = angle_z = 0;
	angle_y = 1;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	scale = 1.f;
	aspect_ratio = 1024.f / 768.f;	// Initial aspect ratio from window size (variables would be better!)
	colourmode = 0;
	numLats = 10;		// Number of latitudes in our sphere
	numLongs = 10;		// Number of longitudes in our sphere

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);
	// Create the vertex array object and make it current
	glBindVertexArray(vao);
	/* create the sphere object */
	numSphereVertices = makeSphereVBO(numLats, numLongs);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("lab3start.vert", "lab3start.frag");
	}
	catch (std::exception &e)
	{
		std::cout << "Caught exception: " << e.what() << std::endl;
		std::cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program);

	/* Bind cube vertices. Note that this is in attribute index 0 */
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index 1 */
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube normals. Note that this is in attribute index 2 */
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Define the model transformations for the cube
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(x + 0.5, y, z));
	model = glm::scale(model, glm::vec3(scale, scale, scale));//scale equally in all axis
	model = glm::rotate(model, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = glm::rotate(model, -angle_z, glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(30.0f, aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 3, 8), // Camera is at (0,0,4), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
	glUniform1ui(colourmodeID, colourmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &Projection[0][0]);

	/* Define the model transformations for our planet */
	model = glm::mat4(1.0f);
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-0.5 - 0.5, 0, 0));
	model = glm::scale(model, glm::vec3(scale / 9.f, scale / 9.f, scale / 9.f));//scale equally in all axis
	model = glm::rotate(model, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = glm::rotate(model, -angle_z, glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our planet */
	drawSphere();

	/* Define the model transformations for our moon */
	model = glm::mat4(1.0f);
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0)); // Rotate it again around the sun so it appears to follow the planet
	model = glm::translate(model, glm::vec3(-0.5 - 0.5, 0, 0)); // Third move it out by the same distance as the planet was moved
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0)); // Second rotate the moon around the origin
	model = glm::translate(model, glm::vec3(-0.3, 0, 0)); // First move the moon out by its distance from the planet
	model = glm::scale(model, glm::vec3(scale / 22.f, scale / 22.f, scale / 22.f));//scale equally in all axis
	model = glm::rotate(model, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = glm::rotate(model, -angle_z, glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our moon */
	drawSphere();

	/* Define the model transformations for our sun */
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(scale / 3.f, scale / 3.f, scale / 3.f));//scale equally in all axis
	model = glm::rotate(model, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, (-angle_y/3), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = glm::rotate(model, -angle_z, glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our sun */
	drawSphere();

	/* Define the model transformations for our planet2 */
	model = glm::mat4(1.0f);
	model = glm::rotate(model, -angle_y, glm::vec3(0, 1, 0));
	model = glm::translate(model, glm::vec3(-0.5, 0, 2));
	model = glm::scale(model, glm::vec3(scale / 7.f, scale / 7.f, scale / 7.f));//scale equally in all axis
	model = glm::rotate(model, -angle_x, glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = glm::rotate(model, (-angle_y/2), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = glm::rotate(model, -angle_z, glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our planet2 */
	drawSphere();

	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
	
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = float(w) / float(h);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	/*
	if (key == 'Q') angle_inc_x -= 0.05f;
	if (key == 'W') angle_inc_x += 0.05f;
	*/
	if (key == 'E') angle_inc_y -= 0.05f;
	if (key == 'R') angle_inc_y += 0.05f;
	
	/*
	if (key == 'T') angle_inc_z -= 0.05f;
	if (key == 'Y') angle_inc_z += 0.05f;
	if (key == 'A') scale -= 0.02f;
	if (key == 'S') scale += 0.02f;
	if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;
	*/

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		std::cout << "colourmode=" << colourmode << std::endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}

}



/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Lab2: Hello 3D");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

/* Make a sphere from two triangle fans (one at each pole) and triangle strips along latitudes */
/* This version uses indexed vertex buffers for both the fans at the poles and the latitude strips */
/* Please not that a better structure would be to make a seperate sphere class and create an instance
of this in our lab3start.cpp program. */
GLuint makeSphereVBO(GLuint numlats, GLuint numlongs)
{
	GLuint i, j;
	/* Calculate the number of vertices required in sphere */
	GLuint numvertices = 2 + ((numlats - 1) * numlongs);
	GLfloat* pVertices = new GLfloat[numvertices * 3];
	GLfloat* pColours = new GLfloat[numvertices * 4];
	makeUnitSphere(pVertices, numlats, numlongs);

	/* Define colours as the x,y,z components of the sphere vertices */
	for (i = 0; i < numvertices; i++)
	{
		pColours[i * 4] = pVertices[i * 3];
		pColours[i * 4 + 1] = pVertices[i * 3 + 1];
		pColours[i * 4 + 2] = pVertices[i * 3 + 2];
		pColours[i * 4 + 3] = 1.f;
	}

	/* Generate the vertex buffer object */
	glGenBuffers(1, &sphereBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, sphereBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numvertices * 3, pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the normals in a buffer object */
	glGenBuffers(1, &sphereNormals);
	glBindBuffer(GL_ARRAY_BUFFER, sphereNormals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numvertices * 3, pVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Store the colours in a buffer object */
	glGenBuffers(1, &sphereColours);
	glBindBuffer(GL_ARRAY_BUFFER, sphereColours);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* numvertices * 4, pColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Calculate the number of indices in our index array and allocate memory for it */
	GLuint numindices = ((numlongs * 2) + 2) * (numlats - 1) + ((numlongs + 2) * 2);
	GLuint* pindices = new GLuint[numindices];

	// fill "indices" to define triangle strips
	GLuint index = 0;		// Current index

	// Define indices for the first triangle fan for one pole
	for (i = 0; i < numlongs + 1; i++)
	{
		pindices[index++] = i;
	}
	pindices[index++] = 1;	// Join last triangle in the triangle fan

	GLuint start = 1;		// Start index for each latitude row
	for (j = 0; j < numlats - 2; j++)
	{
		for (i = 0; i < numlongs; i++)
		{
			pindices[index++] = start + i;
			pindices[index++] = start + i + numlongs;
		}
		pindices[index++] = start; // close the triangle strip loop by going back to the first vertex in the loop
		pindices[index++] = start + numlongs; // close the triangle strip loop by going back to the first vertex in the loop

		start += numlongs;
	}

	// Define indices for the last triangle fan for the south pole region
	for (i = numvertices - 1; i > (numvertices - numlongs - 2); i--)
	{
		pindices[index++] = i;
	}
	pindices[index] = numvertices - 2;	// Tie up last triangle in fan

	// Generate a buffer for the indices
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numindices * sizeof(GLuint), pindices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete pindices;
	delete pColours;
	delete pVertices;
	return numvertices;
}


/* Define the vertex positions for a sphere. The array of vertices must have previosuly
been created.
*/
void makeUnitSphere(GLfloat *pVertices, GLuint numlats, GLuint numlongs)
{
	GLfloat DEG_TO_RADIANS = 3.141592f / 180.f;
	GLuint vnum = 0;
	GLfloat x, y, z, lat_radians, lon_radians;

	/* Define north pole */
	pVertices[0] = 0; pVertices[1] = 0; pVertices[2] = 1.f;
	vnum++;

	GLfloat latstep = 180.f / numlats;
	GLfloat longstep = 360.f / numlongs;

	/* Define vertices along latitude lines */
	for (GLfloat lat = 90.f - latstep; lat > -90.f; lat -= latstep)
	{
		lat_radians = lat * DEG_TO_RADIANS;
		for (GLfloat lon = -180.f; lon < 180.f; lon += longstep)
		{
			lon_radians = lon * DEG_TO_RADIANS;

			x = cos(lat_radians) * cos(lon_radians);
			y = cos(lat_radians) * sin(lon_radians);
			z = sin(lat_radians);

			/* Define the vertex */
			pVertices[vnum * 3] = x; pVertices[vnum * 3 + 1] = y; pVertices[vnum * 3 + 2] = z;
			vnum++;
		}
	}
	/* Define south pole */
	pVertices[vnum * 3] = 0; pVertices[vnum * 3 + 1] = 0; pVertices[vnum * 3 + 2] = -1.f;
}

/* Draws the sphere form the previously defined vertex and index buffers */
void drawSphere()
{
	GLuint i;

	/* Draw the vertices as GL_POINTS */
	glBindBuffer(GL_ARRAY_BUFFER, sphereBufferObject);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	/* Bind the sphere normals */
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, sphereNormals);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	/* Bind the sphere colours */
	glBindBuffer(GL_ARRAY_BUFFER, sphereColours);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glPointSize(3.f);

	// Enable this line to show model in wireframe
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, numSphereVertices);
	}
	else
	{
		/* Bind the indexed vertex buffer */
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		/* Draw the north pole regions as a triangle  */
		glDrawElements(GL_TRIANGLE_FAN, numLongs + 2, GL_UNSIGNED_INT, (GLvoid*)(0));

		/* Calculate offsets into the indexed array. Note that we multiply offsets by 4
		because it is a memory offset the indices are type GLuint which is 4-bytes */
		GLuint lat_offset_jump = (numLongs * 2) + 2;
		GLuint lat_offset_start = numLongs + 2;
		GLuint lat_offset_current = lat_offset_start * 4;

		/* Draw the triangle strips of latitudes */
		for (i = 0; i < numLats - 2; i++)
		{
			glDrawElements(GL_TRIANGLE_STRIP, numLongs * 2 + 2, GL_UNSIGNED_INT, (GLvoid*)(lat_offset_current));
			lat_offset_current += (lat_offset_jump * 4);
		}
		/* Draw the south pole as a triangle fan */
		glDrawElements(GL_TRIANGLE_FAN, numLongs + 2, GL_UNSIGNED_INT, (GLvoid*)(lat_offset_current));
	}
}

