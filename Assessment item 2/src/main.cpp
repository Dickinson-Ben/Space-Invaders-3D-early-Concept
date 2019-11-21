#include <iostream>
#include <vector>

// GLEW - OpenGL Extension Wrangler - http://glew.sourceforge.net/
// NOTE: include before SDL.h
#include <GL/glew.h> //Looks for the glew headder file in a folder named GL (The dependancies set up the file paths)

// SDL - Simple DirectMedia Layer - https://www.libsdl.org/
#include "SDL.h" //Includes SDL in the same way
#include "SDL_image.h" //includes SDL in the same way





// - OpenGL Mathematics - https://glm.g-truc.net/
#define GLM_FORCE_RADIANS // force glm to use radians (Uses radians by default, mostly paranoia)
// NOTE: must do before including GLM headers
// NOTE: GLSL uses radians, so will do the same, for consistency
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//More including libraries


//vertex shader
GLchar const *vShader[] = {
	"#version 440 core\n"

	"layout(location=0) in vec3 position;\n"
	"layout (location = 1) in vec2 texCoord;\n"

	"out vec2 TexCoord;\n"
	"out vec3 textureDir;\n"
	
	//"uniform mat4 trans;\n" //this is for transformation i.e. moving the object drawn to the screen!

	"uniform mat4 modelMatrix;\n"
	"uniform mat4 viewMatrix;\n"
	"uniform mat4 projectionMatrix;\n"

	"void main()\n"
	"{\n"
	//"gl_Position = trans * vec4(position.x, position.y, position.z, 1.0);\n" //more transformation stuff
	"gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);\n" 
	"TexCoord = vec2(texCoord.x, 1.0f - texCoord.y);\n"
	"}"
};
GLchar const *fShader[] = {
	"#version 440 core\n"

	"in vec2 TexCoord;\n"
//	"in vec3 textureDir;\n"

	"out vec4 color;\n"
//	"out vec4 cubeColor;\n"

	"uniform sampler2D ourTexture;\n"
//	"uniform samplerCube cubemap;\n"


	"void main()\n"
	"{\n"
	"color = texture(ourTexture, TexCoord);\n" //calculates the final colour of each pixel using texture function.
	//"cubeColor = texture(cubemap, textureDir);\n" // messes with final output I don't know if it's supposed to be here
	"}"
};

//skybox Vertex shader
GLchar const *SBVshader[] = {
	"#version 440 core\n"
	"layout (location = 0) in vec3 position;\n"
	"out vec3 TexCoord; \n"

	"uniform mat4 projectionMatrix;\n"
	"uniform mat4 viewMatrix;\n"

	"void main()\n"
	"{\n"
	"gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);\n"
	"TexCoord = position;\n"
	"}\n"

};

GLchar const *SBFshader[] = {
	"#version 440 core\n"
	"in vec3 TexCoord;\n"
	"out vec4 color;\n"

	"uniform samplerCube skyBox;\n"

	"void main()\n"
	"{\n"

	"color = texture(skyBox, TexCoord);\n"
	"}\n"
};

GLuint loadCubemap(std::vector<const GLchar*> faces);


//void setupStuff(GLuint& VAO, GLuint& EBO, std::vector<GLfloat>& vertices, std::vector<GLuint>& indices) //TODO pass by reference
//{
//moved into main due to scope errors! Fix later?
//}
//*********************************************************************************************
void ErrorCheck()
{
	GLenum error(glGetError());
	while (error != GL_NO_ERROR) { //whilst error is not equal to any GL error the loop will exit
		std::string err;

		switch (error) {
		case GL_INVALID_OPERATION: err = "INVALID_OPERATION"; break;
		case GL_INVALID_ENUM: err = "INVALID_ENUM;"; break;
		case GL_INVALID_VALUE: err = "INVALID_VALUE"; break;
		case GL_OUT_OF_MEMORY: err = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: err = "INVALID_FRAMEBUFFER_OPERATION"; break;



		}
		std::cout << glGetError() << std::endl;
		err = glGetError();
	}

}

int main(int argc, char *argv[]) {
	//*********************************************************************************************
	// Declare display mode structure to be filled in.
	SDL_DisplayMode current;
	// SDL initialise and report errors if any
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init Error: %s\n",
			SDL_GetError());
		return 1;
	}

	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) { //cycles through the displays getting the resolution and refresh rate for each.

		int displayCheck = SDL_GetCurrentDisplayMode(0, &current);// assigns the number of displays found to a variable for error checking purposes

		if (displayCheck != 0)
			// Checks for errors and reports if display check returns a zero, meaning no displays were found.
			SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());

		else
			//prints the current number of displays and their resolitions (per loop itteration) to the console
			SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz.", i, current.w, current.h, current.refresh_rate);
	}

	IMG_Init(IMG_INIT_PNG);
	IMG_Init(IMG_INIT_JPG);



	SDL_Log("SDL initialised OK!\n"); //reports success on
	//*********************************************************************************************
	// Window Creation
	// first and second number refer to the position the inner window appears on screen
	//third and fourth number are the windows dimensions
	SDL_Window *win = nullptr;
	win = SDL_CreateWindow("Ben Dickinson ¦ CGP2012M Graphics ¦ DIC11213186", current.w / 4, current.h / 4, current.w / 2, current.h / 2, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI |  SDL_WINDOW_RESIZABLE);
	//window creation error checking
	if (win == nullptr) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"SDL_CreateWindow init error: %s\n", SDL_GetError());
		return 1;
	}


	// Set OpenGL context parameters
	int major = 4, minor = 5;
	SDL_Log("Asking for OpenGL %d.%d context\n", major, minor);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_PROFILE_CORE); //use core profile
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); //default, probably

												 // OpenGL Context Creation
	SDL_GLContext glcontext = SDL_GL_CreateContext(win);
	if (glcontext == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"SDL_GL_CreateContext init error: %s\n", SDL_GetError());
		return 1;
	}
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	SDL_Log("Got an OpenGL %d.%d context\n", major, minor);

	// initialise GLEW - sets up the OpenGL function pointers for the version of OpenGL we're using
	GLenum rev;
	glewExperimental = GL_TRUE; //GLEW isn't perfect - see https://www.opengl.org/wiki/OpenGL_Loading_Library#GLEW
	rev = glewInit();
	if (GLEW_OK != rev) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
			"glewInit error: %s\n", glewGetErrorString(rev));
		return 1;
	}
	SDL_Log("glew initialised OK!\n");

	int WIDTH = current.w, HEIGHT = current.h;
	//Viewport code here!

	

	glViewport(0, 0, current.w/2, current.h/2);
	//glViewport(0, 0, current.w /2 , current.h/2);



	//build and compile shader program

	//vertex shader
	GLuint vertexShader; //creates a variable for the shader
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, vShader, NULL);
	glCompileShader(vertexShader);

	//Check for errors when compiling vertex shader
	GLint sucsess;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &sucsess);
	if (!sucsess)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return 1;
	}
	std::cout << "SHADER::VERTEX::COMPILATION_SUCCESS" << std::endl;

	//skybox vertex shader 
	GLuint skyBoxVShader;
	skyBoxVShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(skyBoxVShader, 1, SBVshader, NULL);
	glCompileShader(skyBoxVShader);

	glGetShaderiv(skyBoxVShader, GL_COMPILE_STATUS, &sucsess);
	if (!sucsess)
	{
		glGetShaderInfoLog(skyBoxVShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::skyBoxVShader::COMPILATION_FAILED\n" << infoLog << std::endl;
		SDL_Delay(5000);
		return 1;
	}
	std::cout << "SHADER::skyBoxVShader::COMPILATION_SUCCESS" << std::endl;
	

	//fragment shader
	GLuint fragmentShader; //creates variable for the fragment shader
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, fShader, NULL);
	glCompileShader(fragmentShader);

	//Check for errors when compiliing fragment shader

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &sucsess);
	if (!sucsess)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED \n" << infoLog << std::endl;
		return 1;
	}
	std::cout << "SHADER::FRAGMENT::COMPILATION_SUCCESS" << std::endl;

	//skyBox fragment shader
	GLuint skyBoxFShader;
	skyBoxFShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(skyBoxFShader, 1, SBFshader, NULL);
	glCompileShader(skyBoxFShader);

	glGetShaderiv(skyBoxFShader, GL_COMPILE_STATUS, &sucsess);
	if (!sucsess)
	{
		glGetShaderInfoLog(skyBoxFShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::skyBoxFShader::COMPILATION_FAILED \n" << infoLog << std::endl;
		return 1;
	}
	std::cout << "SHADER::skyBoxFShader::COMPILATION_SUCCESS" << std::endl;


	//Link shaders

	GLint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader); //attatch the vertexshader to the shader program
	glAttachShader(shaderProgram, fragmentShader); //attatch the fragment shader to the shader program
	glLinkProgram(shaderProgram); //link the two shaders together

								  //check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &sucsess);
	if (!sucsess) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING::FAILED\n" << infoLog << std::endl;
	}
	std::cout << "SHADER::PROGRAM::LINKING::SUCCESS" << std::endl;
	//delete the shaders
	//glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	//repeat for additional shaders e.g. for cubemap / skybox or whatever I'm calling it.

	GLint SkyboxShader = glCreateProgram();
	glAttachShader(SkyboxShader, skyBoxVShader);
	glAttachShader(SkyboxShader, skyBoxFShader);
	glLinkProgram(SkyboxShader);

	glGetProgramiv(SkyboxShader, GL_LINK_STATUS, &sucsess);
	if (!sucsess) {
		glGetProgramInfoLog(SkyboxShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::SkyboxShader::LINKING::FAILED\n" << infoLog << std::endl;
	}
	std::cout << "SHADER::PROGRAM::SkyboxShader::LINKING::SUCCESS" << std::endl;
	
	glDeleteShader(skyBoxFShader);
	glDeleteShader(skyBoxVShader);




 //it's likley I'll have to use another VAO AND VBO because of the stride settings, there's no texture co-ords for this
	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};





	
	
	// player character
	std::vector<GLfloat> playerVertices  = {
		//first triangle vertices
		0.5f,  0.5f, -0.5f,     1.0f, 1.0f,   // Top Right position and texture co-ordinates
		0.5f, -0.2f, -0.5f,     1.0f, 0.0f,   // Bottom Right and texture co-ordinates
		-0.5f, -0.2f, -0.5f,    0.0f, 0.0f, // Bottom Left and texture co-ordinates

		//second triangle
		0.5f,  0.5f, -0.5f,			1.0f, 1.0f, // Top Right
		-0.5f, -0.2f, -0.5f,		 0.0f, 0.0f,  // Bottom Left
		-0.5f,  0.5f, -0.5f,		 0.0f, 1.0f,   // Top Left 



		//third triangle
		-0.02f, 0.08f, -0.05f,		0.0f, 1.0f, //top left
		0.02f, 0.08f, -0.05f,		1.0f, 1.0f, //top right
		-0.02f, 0.05f, -0.05f,		0.0f, 1.0f,//bottom left
		
		//fourth triangle
		0.02f, 0.08f, -0.05f,		1.0f, 1.0f, //top right
		0.02f, 0.05f, -0.05f,		1.0f, 1.0f,  //bottomr right
		-0.02f, 0.05f, -0.05f,		0.0f, 1.0f, //bottom left

//BACKGROUND SQUARE THING

		2.0f,  2.0f, -0.05f,     1.0f, 1.0f,   // Top Right position and texture co-ordinates
		2.0f, -2.0f, -0.05f,     1.0f, 0.0f,   // Bottom Right and texture co-ordinates
		-2.0f, -2.0f, -0.05f,    0.0f, 0.0f, // Bottom Left and texture co-ordinates

//second triangle
		
		-2.0f,  2.0f, -0.05f,    0.0f, 1.0f, // Top Right
	
		2.0f,  2.0f, -0.05f,	 1.0f, 1.0f,   // Top Left 

	   -2.0f, -2.0f, -0.05f,    0.0f, 0.0f,  // Bottom Left

//BROKE AF

//Cube for testing
-0.05f, -0.05f, -0.05f,  0.0f, 0.0f,
0.05f, -0.05f, -0.05f,  1.0f, 0.0f,
0.05f,  0.05f, -0.05f,  1.0f, 1.0f,
0.05f,  0.05f, -0.05f,  1.0f, 1.0f,
-0.05f,  0.05f, -0.05f,  0.0f, 1.0f,
-0.05f, -0.05f, -0.05f,  0.0f, 0.0f,

-0.05f, -0.05f,  0.05f,  0.0f, 0.0f,
0.05f, -0.05f,  0.05f,  1.0f, 0.0f,
0.05f,  0.05f,  0.05f,  1.0f, 1.0f,
0.05f,  0.05f,  0.05f,  1.0f, 1.0f,
-0.05f,  0.05f,  0.05f,  0.0f, 1.0f,
-0.05f, -0.05f,  0.05f,  0.0f, 0.0f,

-0.05f,  0.05f,  0.05f,  1.0f, 0.0f,
-0.05f,  0.05f, -0.05f,  1.0f, 1.0f,
-0.05f, -0.05f, -0.05f,  0.0f, 1.0f,
-0.05f, -0.05f, -0.05f,  0.0f, 1.0f,
-0.05f, -0.05f,  0.05f,  0.0f, 0.0f,
-0.05f,  0.05f,  0.05f,  1.0f, 0.0f,

0.05f,  0.05f,  0.05f,  1.0f, 0.0f,
0.05f,  0.05f, -0.05f,  1.0f, 1.0f,
0.05f, -0.05f, -0.05f,  0.0f, 1.0f,
0.05f, -0.05f, -0.05f,  0.0f, 1.0f,
0.05f, -0.05f,  0.05f,  0.0f, 0.0f,
0.05f,  0.05f,  0.05f,  1.0f, 0.0f,

-0.05f, -0.05f, -0.05f,  0.0f, 1.0f,
0.05f, -0.05f, -0.05f,  1.0f, 1.0f,
0.05f, -0.05f,  0.05f,  1.0f, 0.0f,
0.05f, -0.05f,  0.05f,  1.0f, 0.0f,
-0.05f, -0.05f,  0.05f,  0.0f, 0.0f,
-0.05f, -0.05f, -0.05f,  0.0f, 1.0f,

-0.05f,  0.05f, -0.05f,  0.0f, 1.0f,
0.05f,  0.05f, -0.05f,  1.0f, 1.0f,
0.05f,  0.05f,  0.05f,  1.0f, 0.0f,
0.05f,  0.05f,  0.05f,  1.0f, 0.0f,
-0.05f,  0.05f,  0.05f,  0.0f, 0.0f,
-0.05f,  0.05f, -0.05f,  0.0f, 1.0f,
	};


	//Either add vertices for skybox to this existing list or make a new VBO, probably the former.


	//goes through the array and multiplies the co-ords by half, shrinking them in size

	//for(GLuint y = 60; y < playerVertices.size(); y++){
	//	playerVertices[y] = playerVertices[y] / 2;
	//}
	//
	glm::vec3 barricadePositions[] = {
		//selcts co-ords to be drawn for barricades. 
		glm::vec3(0.4f, -0.2f, 0.0f),
		glm::vec3(0.0f, -0.2f, 0.0f),
		glm::vec3(-0.4f, -0.2f, 0.0f)
	
	};


	glm::vec3 alienPositions[] = {
		
		glm::vec3(0.6f, 0.6f, 0.0f) ,
		glm::vec3(0.2f, 0.6f, 0.0f),
		glm::vec3(0.4f, 0.6f, 0.0f),//Right hand boundary check
		glm::vec3(0.6f, 0.4f, 0.0f),
		glm::vec3(0.2f, 0.4f, 0.0f),
		glm::vec3(0.4f, 0.4f, 0.0f),
		glm::vec3(0.6f, 0.2f, 0.0f),//Left hand boundary check
		glm::vec3(0.2f, 0.2f, 0.0f),
		glm::vec3(0.4f, 0.2f, 0.0f)

			
	};

	glm::vec3 playerPosition[] = {
		glm::vec3(0.0f, -0.4f, 0.0f)
	};

	glm::vec3 bulletPosition[] = {
		glm::vec3(0.0f, -0.4f, 0.0f)
	};


	//Player Vertices VAO. Currently using the vertices in the afore mentioned playerVertices array. 

	//Buffer generation and texture generation
	GLuint VAO, VAO2, VBO, VBO2; //created variables for vertex array object, element buffer object and vertex buffer object
    //Generate vertex array
	glGenVertexArrays(1, &VAO);//player character array
	glGenBuffers(1, &VBO);//generate the vertex buffer objext and the element buffer object

	//assign vertices to VAO etc.
	glBindVertexArray(VAO); //binds the vertex array for player
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //bind the vertex array object, then bind and set the vertext buffers and attribute pointers
	//copy vertices array into buffer for barricades object
	int bytes = sizeof(GLfloat) * playerVertices.size(); //created a variable called bytes the size of the player vertices array
	glBufferData(GL_ARRAY_BUFFER, bytes, playerVertices.data(), GL_STATIC_DRAW);

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//texture attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	//tells the program where to find the texture co-ordinates are in the series of vertecies. Stride is set to 3, meaning skip the first three vertecies in the row
    //where as the 5 before it tells the it where to find the next set of texture co-ordinates.
	glEnableVertexAttribArray(1);
	
	//unbind the things!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//SKYBOX VBO and VAO setup. 

	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);


	//load and create texture 
	GLuint texture; //create variable to hold texture
	glGenTextures(1, &texture); //first argument is how many textures to generate and second argument is an unassigned int array as second argument (Just a single in this case)
	glBindTexture(GL_TEXTURE_2D, texture); //binds the texture so and subsequent texture commans will configure to the currently bound texture
 //texture parameters 
  //sets the image wrap for the 2d object

 //texture wrapping 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//first argument specifies the texture target, second is the option to set for which axis and third is the texture wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//uses nearest neighbour filtering for the minyfying (Scaling down)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//uses linear filtering for magnifying (Scaling up)

	
	//load an image to be used as a texture!
	//issues loading this from outside the project directory, look into getBasePath to fix!
	SDL_Surface* image = IMG_Load("bin/assets/player.png"); //loads image from assets file to be used as a texture

	
  //checks to see if image failed to load and prints to the console in case of an error
	if (image == NULL)
	{
		std::cout << "Failed to load image...." << std::endl;
	}
	else
	{
		std::cout << "Image loaded sucsessfully!" << std::endl;
	}
	//
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
	//in order: first argumentgenerates a texture on the currently bound texture, 
	//specified mipmap level, tell openGL what format to use
	//sets height and width of resulting texture, 7th and 8th specify format and daya type and the last argument is the actual image
	glGenerateMipmap(GL_TEXTURE_2D);

	//free up memory here
	glBindTexture(GL_TEXTURE_2D, 0); //unbinds the current texture

								
	//************								 ************************************
	 //repeat for barricades
	
	GLuint texture2;
	glGenTextures(1, &texture2); //first argument is how many textures to generate and second argument is an unassigned int array as second argument (Just a single in this case)
	glBindTexture(GL_TEXTURE_2D, texture2); //binds the texture so and subsequent texture commans will configure to the currently bound texture



	 //texture parameters 
	//sets the image wrap for the 2d object

										   //texture wrapping 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//first argument specifies the texture target, second is the option to set for which axis and third is the texture wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//uses nearest neighbour filtering for the minyfying (Scaling down)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//uses linear filtering for magnifying (Scaling up)

	glGenTextures(1, &texture2); //first argument is how many textures to generate and second argument is an unassigned int array as second argument (Just a single in this case)
	glBindTexture(GL_TEXTURE_2D, texture2); //binds the texture so and subsequent texture commans will configure to the currently bound texture



										   //texture parameters 
										   //sets the image wrap for the 2d object

										   //texture wrapping 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//first argument specifies the texture target, second is the option to set for which axis and third is the texture wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//uses nearest neighbour filtering for the minyfying (Scaling down)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//uses linear filtering for magnifying (Scaling up)


	//load an image to be used as a texture!
	//issues loading this from outside the project directory, look into getBasePath to fix!
	SDL_Surface* image2 = IMG_Load("bin/assets/wall.png"); //loads image from assets file to be used as a texture
												//checks to see if image failed to load and prints to the console in case of an error
	if (image2 == NULL)
	{
		std::cout << "Failed to load image...." << std::endl;
	}
	else
	{
		std::cout << "Image loaded sucsessfully!" << std::endl;
	}


												//
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image2->w, image2->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image2->pixels);
	//in order: first argumentgenerates a texture on the currently bound texture, 
	//specified mipmap level, tell openGL what format to use
	//sets height and width of resulting texture, 7th and 8th specify format and daya type and the last argument is the actual image
	glGenerateMipmap(GL_TEXTURE_2D);

	//free up memory here
	glBindTexture(GL_TEXTURE_2D, 0); //unbinds the current texture

									 //repeat for aliens



									 //************								 ************************************
									 //repeat for barricades

	GLuint texture3;
	glGenTextures(1, &texture3); //first argument is how many textures to generate and second argument is an unassigned int array as second argument (Just a single in this case)
	glBindTexture(GL_TEXTURE_2D, texture3); //binds the texture so and subsequent texture commans will configure to the currently bound texture

	//texture parameters 
	//sets the image wrap for the 2d object
	//texture wrapping 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//first argument specifies the texture target, second is the option to set for which axis and third is the texture wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//uses nearest neighbour filtering for the minyfying (Scaling down)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//uses linear filtering for magnifying (Scaling up)

	glGenTextures(1, &texture3); //first argument is how many textures to generate and second argument is an unassigned int array as second argument (Just a single in this case)
	glBindTexture(GL_TEXTURE_2D, texture3); //binds the texture so and subsequent texture commans will configure to the currently bound texture

	//texture parameters 
	//sets the image wrap for the 2d object
	//texture wrapping 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//first argument specifies the texture target, second is the option to set for which axis and third is the texture wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//uses nearest neighbour filtering for the minyfying (Scaling down)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//uses linear filtering for magnifying (Scaling up)


	//load an image to be used as a texture 

	//for some reason I cannot change this image to be a different image, it's bizzare.

	SDL_Surface* image3 = IMG_Load("bin/assets/Alien3.png"); //loads image from assets file to be used as a texture
	//checks to see if image failed to load and prints to the console in case of an error
	if (image3 == NULL)
	{
		std::cout << "Failed to load image...." << std::endl;
	}
	else
	{
		std::cout << "Image loaded sucsessfully!" << std::endl;
	}


	//
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image3->w, image3->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image3->pixels);
	//in order: first argumentgenerates a texture on the currently bound texture, 
	//specified mipmap level, tell openGL what format to use
	//sets height and width of resulting texture, 7th and 8th specify format and daya type and the last argument is the actual image
	glGenerateMipmap(GL_TEXTURE_2D);

	//free up memory here
	glBindTexture(GL_TEXTURE_2D, 0); //unbinds the current texture


	GLuint texture4; //create variable to hold texture
	glGenTextures(1, &texture4); //first argument is how many textures to generate and second argument is an unassigned int array as second argument (Just a single in this case)
	glBindTexture(GL_TEXTURE_2D, texture4); //binds the texture so and subsequent texture commans will configure to the currently bound texture
	//texture parameters 
	//sets the image wrap for the 2d object
	//texture wrapping 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//first argument specifies the texture target, second is the option to set for which axis and third is the texture wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//uses nearest neighbour filtering for the minyfying (Scaling down)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//uses linear filtering for magnifying (Scaling up)
	//load an image to be used as a texture!
	//issues loading this from outside the project directory, look into getBasePath to fix!
	SDL_Surface* image4 = IMG_Load("bin/assets/stars6.JPG"); //loads image from assets file to be used as a texture
	//checks to see if image failed to load and prints to the console in case of an error
	if (image4 == NULL)
	{
		std::cout << "Failed to load image...." << std::endl;
	}
	else
	{
		std::cout << "Image loaded sucsessfully!" << std::endl;
	}
	//
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image4->w, image4->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image4->pixels);
	//in order: first argumentgenerates a texture on the currently bound texture, 
	//specified mipmap level, tell openGL what format to use
	//sets height and width of resulting texture, 7th and 8th specify format and daya type and the last argument is the actual image
	glGenerateMipmap(GL_TEXTURE_2D);

	//free up memory here
	glBindTexture(GL_TEXTURE_2D, 0); //unbinds the current texture




	//skybox texture 

	std::vector<const GLchar*> faces;
	faces.push_back("bin/assets/right.jpg");
	faces.push_back("bin/assets/left.jpg");
	faces.push_back("bin/assets/top.jpg");
	faces.push_back("bin/assets/bottom.jpg");
	faces.push_back("bin/assets/back.jpg");
	faces.push_back("bin/assets/front.jpg");


	GLuint cubeMapTexture = loadCubemap(faces);

	//************************************************************************************************





	SDL_Event event;//initialises SDL_Event as an event I guess?
	bool runGame = true; //boolean value used as loop condition

						 //sets up the 4*4 matrix to hold rotation, transformation and scale
	glm::mat4 tRotate; 
	glm::mat4 tScale; 
	glm::mat4 tTranslate;


	//mat 4 matrix variables for projection useage 
	glm::mat4 pModel;
	glm::mat4 pView;
	glm::mat4 pProjection;
	glm::mat4 aModel;
	glm::mat4 bModel;
	glm::mat4 buModel;

	//camera position
	glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 1.0f); //starting position of the camera change
	//camera direction and where it's looking
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(cameraPosition - cameraTarget);
	//right axis
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection)); //takes a cross product of up and direction to get right
	//gets the up vector
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight); // takes a cross product of direction and right (right which was defined by a previous cross product)
	//gets the front vector
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.2f);
	//declares a perspective type projection matrix
	
	//perspective projection
	//floats represent left, right, top and bottom then near and far. These define the clip plane
	pProjection = glm::perspective(45.0f, (GLfloat)current.w / (GLfloat)current.h, 0.1f, 100.0f);
	//translates the model to the middle of the window
	//pModel = glm::translate(pModel, glm::vec3(0.0f, -1.0f, -2.0f));





	//for rotation
	GLfloat camX;
	GLfloat camZ;
	GLfloat radius = 0.6f;
	//GLfloat camY;
	GLfloat angle = 0;
	
	bool fullScreen;
	fullScreen = false;
	bool movingLeft = false;
	bool movingRight = false;
	bool bulletFired = false;


	unsigned int lastTime = 0, currentTime; //sets up variables for time tracking
	//int WIDTH, HEIGHT; //created variables width and height for use in the game loop / window resizing
	float speed = 0.05f;

	//Viewmatrix
	pView = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
	camX = sin(glm::radians(angle));
	camZ = cos(glm::radians(angle));
	pView = glm::lookAt(glm::vec3(camX, 0.0, camZ), cameraPosition + cameraFront, glm::vec3(0.0, 1.0, 0.0)); //changing the first float in the last glm::vec3 will change the rotation on the X axis
	
	for (GLuint i = 0; i < 9; i++)
	{
		alienPositions[i] += cameraPosition + cameraFront;
		
	}
	
	playerPosition[0] += cameraPosition + cameraFront;
	for (GLuint i = 0; i < 3; i++)
	{
		barricadePositions[i] += cameraPosition + cameraFront;
	}
	//pModel = glm::scale(pModel, glm::vec3(2, 2, 2)); //<- this scales the model by the xyz given. Useful for re-using co-ords
	
	bulletPosition[0] += cameraPosition + cameraFront;


	glEnable(GL_DEPTH_TEST); //enabled depth testing to stop texture distortion on 3d objects
	while (runGame)//Start game loop - check if game should end
	{
		


		//draw code
		// Clear the back buffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//skybox
		glDisable(GL_DEPTH_TEST);
	 	glDepthMask(GL_FALSE); //turn depth writing off
		glUseProgram(SkyboxShader);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);


		//get the uniform location
		GLint CmodelLocation = glGetUniformLocation(SkyboxShader, "modelMatrix");
		GLint CviewLocation = glGetUniformLocation(SkyboxShader, "viewMatrix");
		GLint CprojectionLocation = glGetUniformLocation(SkyboxShader, "projectionMatrix");
		glm::mat4 cModel;
		
		
		



		glUniformMatrix4fv(CviewLocation, 1, GL_FALSE, glm::value_ptr(pView));
		glUniformMatrix4fv(CprojectionLocation, 1, GL_FALSE, glm::value_ptr(pProjection));
		glUniformMatrix4fv(CmodelLocation, 1, GL_FALSE, glm::value_ptr(cModel));
		//glBindVertexArray(VAO);


		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);
		glUseProgram(0);
		//activate skybox shader

	




		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST); //enabled depth testing to stop texture distortion on 3d objects
		//Activate the shader
		glUseProgram(shaderProgram);


		//get the uniform location
		GLint modelLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
		GLint viewLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
		GLint projectionLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");

		//sets the modelMatrix uniform in the shader
		
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(pView));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(pProjection));
		glBindVertexArray(VAO);


		//Draw space invaders
		glBindTexture(GL_TEXTURE_2D, texture3);
		for (GLuint i = 0; i < 9; i++)
		{
			//multi-object loop goes here
			glm::mat4 aModel;
			aModel = glm::translate(aModel, alienPositions[i] + cameraPosition + cameraFront);
			//aModel = glm::translate(aModel, glm::vec3(0.0f, 0.0f, 0.6f));
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(aModel));
			glDrawArrays(GL_TRIANGLES, 18, 53); //Number of co-orditnates to render / connect
											  //loop ends here
			currentTime = SDL_GetTicks();//sts current time to the number of ticks that have passed in miliseconds
			if (currentTime > lastTime + 500) //checks to see if 1000 miliseconds (1.5 second) has passed
			{
				
					alienPositions[0].x += speed;
					alienPositions[1].x += speed;
					alienPositions[2].x += speed;
					alienPositions[3].x += speed;
					alienPositions[4].x += speed;
					alienPositions[5].x += speed;
					alienPositions[6].x += speed;
					alienPositions[7].x += speed;
					alienPositions[8].x += speed;

					//runs through the positions array and applies the transformation to each one
					//do check to see if aliens have hit the edge
					if(alienPositions[i].x > 0.9){
						speed = -0.05f;
						for(int b = 0; b < 9; b++)
						{
						alienPositions[b].y -= 0.15f;

						}
					}

					if (alienPositions[i].x < -0.9f) {
						speed = 0.05;
						for(int b = 0; b < 9; b++)
						{
							alienPositions[b].y -= 0.15f;
						}
				}
				lastTime = currentTime;
				//Sets last time to the current time, then when loop restarts compares current time to last time, since last time is the last number of ticks
				//stored in that variable you can check to see if a certian number of tick difference has passed, i.e. if it's been 2 seconds since last time was
				//updated, do an action then update last time and repeat.	

				//do check to see if aliens have reached bottom of the screen

				if (alienPositions[3].y < -0.2f) //ends the game loop when the aliens get to the bottom!! No collision detection needed, but would be nice
				{
					runGame = false;
				}
			}
		};


		//draw barricades

		glBindTexture(GL_TEXTURE_2D, texture2);
		//do a check on time to determin which texture is rendered each loop itteration
		
		for (GLuint i = 0; i < 3; i++)
		{
			//multi-object loop goes here
			glm::mat4 gModel;
			gModel = glm::translate(gModel, barricadePositions[i] + cameraPosition + cameraFront);
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(gModel));
			glDrawArrays(GL_TRIANGLES, 18, 53); 
			//Number of co-orditnates to render / connect
			//loop ends here
		};


		//draw bullets on spacebar press



		//draw player
			
			glBindTexture(GL_TEXTURE_2D, texture);
			glm::mat4 pModel;
			pModel = glm::translate(pModel, playerPosition[0] + cameraPosition + cameraFront);
			modelLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(pModel));
			glDrawArrays(GL_TRIANGLES, 18, 52); 			



			glm::mat4 buModel;
			buModel = glm::translate(buModel, bulletPosition[0] + cameraPosition + cameraFront);
			buModel = glm::scale(buModel, glm::vec3(0.5, 0.4, 0.4));


			modelLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(buModel));
			glDrawArrays(GL_TRIANGLES, 18, 52);
			

			////draw bullet 
			//glm::mat4 buModel;
			//buModel = glm::translate(buModel, playerPosition[0]).
			//
			//glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(pModel));
			//modelLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
			//glDrawArrays(GL_TRIANGLES, 18, 52);

			if (bulletFired == true)
			{
				bulletPosition[0].y += 0.10f;
			}

			//unbind the vertex buffer
			glBindVertexArray(0); 
			// Present the back buffer
		SDL_GL_SwapWindow(win);
		//input handling goes here. I.e. keypress for transformations etc.

		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN)// && !event.key.repeat)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE: runGame = false;	 //sets rungame to false if escape is pressed	  
					break;

				case SDLK_a:
					
				if(playerPosition[0].x >= -0.9f)
				{
				//	cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * (speed);
					//check to see if player position is >= desired left boundary
					playerPosition[0].x -= 0.10f;
					if (bulletFired == false)
					{
						bulletPosition[0].x -= 0.1f;
					}
					movingLeft = true;
				}
					break;
				case SDLK_RIGHT: 
					angle += 30;
					camX = sin(glm::radians(angle));
					camZ = cos(glm::radians(angle));
					pView = glm::lookAt(glm::vec3(camX , 0.0, camZ), cameraPosition + cameraFront, glm::vec3(0.0, 1.0, 0.0));
					break;
				case SDLK_LEFT: 
					angle -= 30;
					camX = sin(glm::radians(angle));
					camZ = cos(glm::radians(angle));
					pView = glm::lookAt(glm::vec3(camX, 0.0, camZ), cameraPosition + cameraFront, glm::vec3(0.0, 1.0, 0.0));
					break;

				case SDLK_d:
					
					if (playerPosition[0].x <= 0.9f) 
					{
				//	cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * (speed);
					//check to see if player position is >= desired right boundary
					playerPosition[0].x += 0.10f;
					if (bulletFired == false)
					{
						bulletPosition[0].x += 0.10f;
					}
					movingRight = true;
					}
					break;
				case SDLK_KP_PLUS: if (fullScreen == false)
				{
					SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN);
					SDL_GetCurrentDisplayMode(0, &current);//gets the resolution for the main display and sets them to current.w and current.h (These are inbuilt values)
					WIDTH = current.w; //assign current.w and h to width and hight since they apparently can't be passed in directly
					HEIGHT = current.h;
					glViewport(0, 0, (WIDTH), (HEIGHT));
					fullScreen = true;
				}	
				break;

				case SDLK_KP_MINUS: if (fullScreen = true)
				{
					SDL_SetWindowFullscreen(win, 0); //setting the full screen parameter to 0 disables full screen mode!, who knew?
					SDL_GetCurrentDisplayMode(0, &current);
					WIDTH = current.w;
					HEIGHT = current.h;
					glViewport(0, 0, (WIDTH), (HEIGHT));
					fullScreen = false;
				}
				break;
				case SDLK_SPACE:
					
					bulletFired = true;
					bulletPosition[0] = playerPosition[0];


					break;

				}//end of switch statement
			}

		}
			////uses the quit button on the SDL window to end the applicaiton
			if (event.type == SDL_QUIT)
			{
				runGame = false;
			}

			//window resizing stuff. Pass in monitor resolution for full screen and window size normal resizing
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
				SDL_GetCurrentDisplayMode(0, &current);
				WIDTH = current.w; //assign current.w and h to width and hight since they apparently can't be passed in directly
				HEIGHT = current.h;
				glViewport(0, 0, (WIDTH), (HEIGHT));
				}


			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
				SDL_GetCurrentDisplayMode(0, &current);//gets the resolution for the main display and sets them to current.w and 
				SDL_GetWindowSize(win, &WIDTH, &HEIGHT);
				glViewport(0, 0, WIDTH, HEIGHT);
			}


			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESTORED) {
				SDL_GetCurrentDisplayMode(0, &current);//gets the resolution for the main display and sets them to current.w and 
				SDL_GetWindowSize(win, &WIDTH, &HEIGHT);
				glViewport(0, 0, WIDTH, HEIGHT);
				}
			SDL_Delay(10);

		}

		//int lives;
		//if (lives == 3) {
		//render number of positional objects maybe? Or have a single object rendered and swap the texture out upon colision detection.
		//}




		ErrorCheck(); //rus the error check function as last step.


	


	//de-allocate the recources since they're not in use anymore! 

	glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO); //fixme add into the destructor - when you have a class


	// Clean up
	SDL_Log("Finished. Cleaning up and closing down\n");

	//closes the window 
	SDL_Quit();
	return 0;

}

//issue in here somewhere, just don't know where

GLuint loadCubemap(std::vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	//int width, height;
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		SDL_Surface* SBimage = IMG_Load(faces[i]); //had to convert from soil, this may cause issues if I've not done the conversion right.
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, SBimage->w, SBimage->h, 0, GL_RGB, GL_UNSIGNED_BYTE, SBimage->pixels); //SBimage->pixels also lets program compile
	}


	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}