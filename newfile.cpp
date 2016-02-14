#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <bits/stdc++.h>
#include <ao/ao.h>
#include <mpg123.h>
#include <thread>


#define ll long long
#define mp(x,y) make_pair(x,y)
#define pr pair<int,int>
#define F first
#define S second
#define pb push_back

#define BITS 8

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void* play_audio(string audioFile){   
  mpg123_handle *mh;
  unsigned char *buffer;
  size_t buffer_size;
  size_t done;
  int err;
  int driver;
  ao_device *dev;
  ao_sample_format format;
  int channels, encoding;
  long rate;    /* initializations */
  ao_initialize();
  driver = ao_default_driver_id();
  mpg123_init();
  mh = mpg123_new(NULL, &err);
  buffer_size = mpg123_outblock(mh);
  buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
  mpg123_open(mh, &audioFile[0]);   mpg123_getformat(mh, &rate, &channels, &encoding);
  format.bits = mpg123_encsize(encoding) * 8;
  format.rate = rate;
  format.channels = channels;
  format.byte_format = AO_FMT_NATIVE;
  format.matrix = 0;
  dev = ao_open_live(driver, &format, NULL);
  char *p =(char *)buffer;
  while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
  ao_play(dev, p, done);    /* clean up */  
  free(buffer);   
  ao_close(dev);  
  mpg123_close(mh);   
  mpg123_delete(mh);
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float cube_size = 0.2;
float ho_t=0;
float vo_t=0;
float fall=0;
bool arrow_work=0;
bool x_turn =0;
bool z_turn =1;
int no_of_walks =0;
float obstacle;
int x=40,y=0,z=0,x1=10,z1=0;
float rotatebuilding=0;
float rotatebuilding1=0;
float player_height = 9;
bool only_player=0;
bool top_view = 0;
bool rotate_build=1,player_eye=0,dont_show=0,dont_show1=0,ind=1,inw=0,ina=0,ins=0;
int bigradius=40;
double xmousePos,ymousePos,xmousePos1,ymousePos1;
int shiftx = 0,shifty=0;
float horizontal_position=0,vertical_position=0,angle_thrown=M_PI/2.5,initial_velocity=7.7,time_travel=0,z_position=0;
bool jump_initiated = 0;
int toaddh=1,toaddv=-1;
float board_position = 2.8,dire=1.0,forboardmovement=0,storeinitialposition=0;
bool onboard=0,work=0;

int const test[10][10] = {{9,9,9,7,9,7,9,9,9,9},
                {9,9,5,9,9,9,1,9,9,9},
                {9,9,9,5,9,9,9,9,9,9},
                {5,9,9,12,9,7,9,7,9,1},
                {5,9,9,9,1,9,9,9,5,9},
                {5,9,9,12,9,9,9,9,9,9},
                {5,5,9,9,9,9,1,12,9,9},
                {9,9,1,9,9,2,9,9,9,1},
                {9,9,9,9,1,9,5,9,9,9},
                {9,9,1,9,3,9,9,9,9,9}};

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_LEFT:
                  ho_t-=0.2;
                  x_turn=1;
                  z_turn=0;
                  ho_t = floor(ho_t*10);
                  ho_t=ho_t/10;
                  no_of_walks=1;
                  if(player_eye==1)
                  {
                    dont_show=1;
                    dont_show1=0;
                  }
                  ind=0;
                  ina=1;
                  inw=0;
                  ins=0;
                  // thread(play_audio,"Mario - Jump.mp3").detach();
                  if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9 && player_height==9)
                    ho_t+=0.2;
                  break;
            case GLFW_KEY_RIGHT:
                x_turn=1;
                z_turn=0;
                // if(arrow_work==0)
                ho_t+=0.2;
                ho_t = floor(ho_t*10);
                ho_t=ho_t/10;
                no_of_walks=1;
                if(player_eye==1)
                {
                  dont_show=1;
                  dont_show1=0;
                }
                cout << vo_t << " " << ho_t << " ---" << endl;

                // thread(play_audio,"Mario - Jump.mp3").detach();
               ind=1;
               ina=0;
               inw=0;
               ins=0;
               if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9 && player_height==9)
               {
                // if(int(10*vo_t)%4==0 && int(10*ho_t)%4==0)
                  ho_t-=0.2;
               }
               break;
            case GLFW_KEY_UP:
                x_turn=0;
          z_turn=1;
          // temp=vo_t;
          // if(arrow_work==0)
            vo_t-=0.2;
          vo_t = floor(vo_t*10);
          cout << vo_t << endl;
          vo_t=vo_t/10;
          cout << ":::" << vo_t << endl;
          no_of_walks=1;
          if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9 && player_height==9)
          {
            // if(int(10*vo_t)%4==0 && int(10*ho_t)%4==0)
              vo_t+=0.2;
          }
          // cout << vo_t << " " << ho_t << endl;
          if(player_eye==1)
          {
            dont_show1=1;
            dont_show=0;
          }
          // thread(play_audio,"Mario - Jump.mp3").detach(); 
          ind=0;
          ina=0;
          inw=1;
          ins=0;
          break;
          case GLFW_KEY_DOWN:
            x_turn=0;
          z_turn=1;
          // if(arrow_work==0)
            vo_t+=0.2;
          // cout << vo_t*10 << endl;
          vo_t = floor(vo_t*10);
          if(int(-1*vo_t)%2==1)
          {
            vo_t+=1;
            // cout << "----" << endl;
          }
          
          vo_t=vo_t/10;
          cout << vo_t << endl;
          no_of_walks=1;
          if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9  && player_height==9)
            vo_t-=0.2;
          if(player_eye==1)
          {
            dont_show1=1;
            dont_show=0;
          }
          ind=0;
          ina=0;
          inw=0;
          ins=1;
          // thread(play_audio,"Mario - Jump.mp3").detach();
          break;    
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
        case 'a':
        // case 37:
        	// if(arrow_work==0)
        		ho_t-=0.2;
        	x_turn=1;
        	z_turn=0;
        	ho_t = floor(ho_t*10);
        	ho_t=ho_t/10;
          no_of_walks=1;
          if(player_eye==1)
          {
            dont_show=1;
            dont_show1=0;
          }
          ind=0;
          ina=1;
          inw=0;
          ins=0;
          // thread(play_audio,"Mario - Jump.mp3").detach();
          // play_audio("jump_01.mp3");
          // cout << ho_t << " " << vo_t << endl;
          if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9 && player_height==9)
            ho_t+=0.2;
          if(test[-1*int(vo_t*10)/4+1][int(ho_t*10)/4]>9 && player_height==9 && int(vo_t*10)%4!=0)
          {
            ho_t-=0.2;
          }
          // cout << ho_t << " " << vo_t << endl;
        	break;
        case 'd':
        	x_turn=1;
        	z_turn=0;
        	// if(arrow_work==0)
        		ho_t+=0.2;
        	ho_t = floor(ho_t*10);
        	ho_t=ho_t/10;
          no_of_walks=1;
          if(player_eye==1)
          {
            dont_show=1;
            dont_show1=0;
          }
          cout << vo_t << " " << ho_t << " ---" << endl;
          // thread(play_audio,"Mario - Jump.mp3").detach();
          ind=1;
          ina=0;
          inw=0;
          ins=0;
          if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9 && player_height==9)
          {
            // if(int(10*vo_t)%4==0 && int(10*ho_t)%4==0)
              ho_t-=0.2;
          }
          if(test[-1*int(vo_t*10)/4+1][int(ho_t*10)/4]>9 && player_height==9 && int(vo_t*10)%4!=0)
          {
            ho_t-=0.2;
          }
        	break;
        case 'w':
        	x_turn=0;
        	z_turn=1;
          // temp=vo_t;
        	// if(arrow_work==0)
        		vo_t-=0.2;
        	vo_t = floor(vo_t*10);
          cout << vo_t << endl;
        	vo_t=vo_t/10;
          cout << ":::" << vo_t << endl;
          no_of_walks=1;
          if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9 && player_height==9)
          {
            // if(int(10*vo_t)%4==0 && int(10*ho_t)%4==0)
              vo_t+=0.2;
          }
          // cout << vo_t << " " << ho_t << endl;
          if(player_eye==1)
          {
            dont_show1=1;
            dont_show=0;
          }
          // thread(play_audio,"Mario - Jump.mp3").detach(); 
          ind=0;
          ina=0;
          inw=1;
          ins=0;
        	break;
        case 's':
        	x_turn=0;
        	z_turn=1;
        	// if(arrow_work==0)
        		vo_t+=0.2;
          // cout << vo_t*10 << endl;
        	vo_t = floor(vo_t*10);
          if(int(-1*vo_t)%2==1)
          {
            vo_t+=1;
            // cout << "----" << endl;
          }
          
        	vo_t=vo_t/10;
          cout << vo_t << endl;
          no_of_walks=1;
          if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9  && player_height==9)
            vo_t-=0.2;
          if(player_eye==1)
          {
            dont_show1=1;
            dont_show=0;
          }
          ind=0;
          ina=0;
          inw=0;
          ins=1;
          // thread(play_audio,"Mario - Jump.mp3").detach();
        	break;
        case 'r':
          // x++;
          rotate_build = 1;
          // only_player = 0;
          rotatebuilding += M_PI/30;
          rotatebuilding1 += M_PI/30;
          x = bigradius*cos(rotatebuilding);
          z = bigradius*sin(rotatebuilding);
          x1 = 10*cos(rotatebuilding);
          z1 = 10*sin(rotatebuilding);
          // z--;
          break;
        case 'X':
          x--;
          break;
        case 'x':
          x++;
          break;
        case 'y':
          y++;
          break;
        case 'Y':
          y--;
          break;
        case 'z':
          z++;
          break;
        case 'Z':
          z--;
        case 'o':
          only_player = 1;
          top_view = 0;
          dont_show=0;
          dont_show1=0;
          break;
        case 'O':
          only_player = 0;
          dont_show=0;
          dont_show1=0;
          break;
        case 't':
          top_view = 1;
          only_player = 0;
          player_eye = 0;
          break;
        case 'T':
          top_view = 0;
          break;
        case 'p':
          player_eye =1;
          top_view = 0;
          only_player = 0;
          break;
        case 'P':
          player_eye =0;
          dont_show=0;
          dont_show1=0;
          break;
        case ' ':
          jump_initiated =1;
          if(onboard==1)
          {
            storeinitialposition=forboardmovement;
            if((board_position-4.7)<-1.8)
            {
              work=1;
            }
            cout << board_position-4.7 << "  @@@@@@@@@"  << endl;
          }

		default:
			break;
	}
}

void cbfun (GLFWwindow* window, double x,double y)
{
  if(y==-1)
  {
    bigradius++;
    x = bigradius*cos(rotatebuilding);
    z = bigradius*sin(rotatebuilding);
  }
  if(y==1)
  {
    bigradius--;
    x = bigradius*cos(rotatebuilding);
    z = bigradius*sin(rotatebuilding);
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
            {
                glfwGetCursorPos(window,&xmousePos1,&ymousePos1);
                ymousePos1 = 600 - ymousePos1;
                shifty +=-1*int(((ymousePos1 - ymousePos)*8)/600);
              shiftx +=int(((xmousePos1 - xmousePos)*8)/600);
            }
            if(action == GLFW_PRESS)
            {
                glfwGetCursorPos(window,&xmousePos,&ymousePos);
                ymousePos = 600 - ymousePos;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                // rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	// GLfloat fov = 90.0f;
    GLfloat fov = 0.2f;
	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    // Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle ,*trans, *forplayer, *body, *body_x, *arrow1, *arrow2, *arrow3, *arrow4 , *small_cube, *board, *plane;

// Creates the triangle object used in this sample code
VAO* createTriangle (float x,float y,float z,float w)
{
  static const GLfloat vertex_buffer_data [] = {
    x,0,z, // vertex 0
    w,0,y, // vertex 1
    -x,0,-z, // vertex 2
  };
  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 0
    1,1,1, // color 1
    1,1,1, // color 2
  };
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

VAO createPlane()
{
  static const GLfloat vertex_buffer_data [] = {
    165,0,0, // vertex 0
    0,0,0, // vertex 1
    0,0,-165, // vertex 2
    165,0,0,
    0,0,-165,
    165,0,-165,
  };
  static const GLfloat color_buffer_data [] = {
    0/255.0,128/255.0,255/255.0, // color 0
    0/255.0,128/255.0,255/255.0,
    0/255.0,128/255.0,255/255.0,
    0/255.0,128/255.0,255/255.0,
    0/255.0,128/255.0,255/255.0,
    0/255.0,128/255.0,255/255.0,
  };
  plane = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL); 
}

// Creates the rectangle object used in this sample code
VAO* createRectangle (float x,float y,float z,GLenum fill_mode=GL_FILL)
{
  // GL3 accepts only Triangles. Quads are not supported
  GLfloat vertex_buffer_data [] = {
    -x,-y,0, // vertex 1
    x,-y,0, // vertex 2
    x, y,0, // vertex 3

    x, y,0, // vertex 3
    -x, y,0, // vertex 4
    -x,-y,0,  // vertex 1

    x,y,0,
    x,-y,0,
    x,-y,2*z,

    x,y,0,
    x,y,2*z,
    x,-y,2*z,

    -x,y,0,
    -x,-y,0,
    -x,y,2*z,

    -x,-y,0,
    -x,-y,2*z,
    -x,y,2*z,

    x,y,2*z,
    -x,y,2*z,
    x,-y,2*z,

    -x,-y,2*z,
    x,-y,2*z,
    -x,y,2*z,

    x,y,2*z,
    -x,y,2*z,
    x,y,0,

    x,y,0,
    -x,y,0,
    -x,y,2*z,

    x,-y,2*z,
    -x,-y,2*z,
    x,-y,0,

    x,-y,0,
    -x,-y,0,
    -x,-y,2*z,    

  };

  GLfloat color_buffer_data [] = {
    152/255.0, 205/255.0, 152/255.0,
    152/255.0, 205/255.0, 152/255.0,
    152/255.0, 205/255.0, 152/255.0,

    152/255.0, 205/255.0, 152/255.0,
    152/255.0, 205/255.0, 152/255.0,
    152/255.0, 205/255.0, 152/255.0,

    0,0,1,
    0,0,1,
    0,0,1,

    0,0,1,
    0,0,1,
    0,0,1,

    1,0,0,
    1,0,0,
    1,0,0,

    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    0,1,0,
    0,1,0,
    0,1,0,

    0,1,0,
    0,1,0,
    0,1,0,

    1,1,1,
    1,1,1,
    1,1,1,

    1,1,1,
    1,1,1,
    1,1,1,

    1,1,0,
    1,0,1,
    1,1,1,

    1,1,0,
    1,0,1,
    1,1,1,
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  return create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data,fill_mode);
}


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */

float jump(float horizontal_position)
{
  horizontal_position += initial_velocity*cos(angle_thrown)*0.005;
  vertical_position += initial_velocity*sin(angle_thrown)*0.005 - (time_travel*time_travel);
  time_travel +=0.01;
  return horizontal_position;
}

void draw_cube(VAO *obj,float x_pos,float y_pos,float z_pos)
{
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP;
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(x_pos, y_pos, z_pos));        // glTranslatef
  // glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(1,1,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(obj);
}

void draw_cuboid(VAO *obj,float x_pos,float y_pos,float z_pos,int flag,int x_walk,int z_walk)
{
	glm::mat4 VP = Matrices.projection * Matrices.view;
 	glm::mat4 MVP;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle = glm::translate (glm::vec3(x_pos, y_pos, z_pos));
  glm::mat4 translateRectangle1 = glm::translate (glm::vec3(0, 0.2, 0));
  glm::mat4 translateRectangle2 = glm::translate (glm::vec3(0, -0.2, 0));
  glm::mat4 rotateRectangle = glm::rotate((float)(flag*rectangle_rotation*M_PI/180.0f), glm::vec3(z_walk,0,x_walk));
  if(no_of_walks!=0)
  {
    Matrices.model *= (translateRectangle * translateRectangle1 * rotateRectangle * translateRectangle2);
  }
  else
  {
      Matrices.model *= (translateRectangle);
  }
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obj);
}


double GetFloatPrecision(double value, double precision)
{
    return (floor((value * pow(10, precision) + 0.5)) / pow(10, precision)); 
}


void draw ()
{
	
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  //glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  if(only_player==0 && top_view==0 && player_eye==0)
    Matrices.view = glm::lookAt(glm::vec3(0+x+shiftx,20+y+shifty,0+z), glm::vec3(-1,3+0,-1.8), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  else if(only_player==1)
    Matrices.view = glm::lookAt(glm::vec3(0+x1+shiftx,y+shifty,z1), glm::vec3(-2.9+ho_t-0.1+(horizontal_position*toaddh),5-((9-player_height)*0.4)+vertical_position,vo_t+0.8-0.6+(z_position*toaddv)), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  else if(top_view==1)
    Matrices.view = glm::lookAt(glm::vec3(0,30,0), glm::vec3(-1,3+0,-1.8), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  else
  {
    if(inw==1)
      Matrices.view = glm::lookAt(glm::vec3(-2.9+ho_t-0.1,5-((9-player_height)*0.4)-0.1,vo_t+0.2), glm::vec3(-2,-2+y,-80), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    if(ins==1)
      Matrices.view = glm::lookAt(glm::vec3(-2.9+ho_t-0.1,5-((9-player_height)*0.4)-0.1,vo_t+0.3), glm::vec3(-2,-2+y,80), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    if(ind==1)
      Matrices.view = glm::lookAt(glm::vec3(-2.9+ho_t-0.1-1,5-((9-player_height)*0.4),vo_t+0.8-0.6-0.1), glm::vec3(40,y+5-((9-player_height)*0.4)-3,1), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    if(ina==1)
      Matrices.view = glm::lookAt(glm::vec3(-2.9+ho_t-0.1-1,2+5-((9-player_height)*0.4),vo_t+0.8-0.6-0.1), glm::vec3(-40,y+5-((9-player_height)*0.4)-3,1), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    // dont_show=1;
  }
  // 200,5+y,-00

  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model
// cout << dont_show1 << dont_show << endl;
if(z_turn==1)
{
  // draw_cuboid(forplayer,-3+ho_t,2+fall-0.3,vo_t+0.8,1,0,1);
  // draw_cuboid(forplayer,-2.8+ho_t,2+fall,vo_t+0.8,-1,0,1);
  if(onboard==0)
  {
    forboardmovement = vo_t+0.8-0.6+(toaddv*z_position);
  }
  else
  {
    if(jump_initiated==0)
      forboardmovement = board_position-4.55+(toaddv*z_position);
    else
      forboardmovement = storeinitialposition + (toaddv*z_position);
  }
  draw_cube(body,-2.9+ho_t-0.1+(horizontal_position*toaddh),5-((9-player_height)*0.4)+vertical_position,forboardmovement);
}
// cout << -2.9+ho_t-0.1 << " " << -2.9+ho_t-0.1+horizontal_position <<  " " << horizontal_position << "(((" << endl;

if(x_turn==1 && dont_show==0)
{
  // draw_cuboid(forplayer,-3+ho_t,2+fall,vo_t+0.8,1,1,0);
  // draw_cuboid(forplayer,-3+ho_t,2+fall,vo_t+0.8,-1,1,0);
  draw_cube(body_x,-3+ho_t-0.1+(horizontal_position*toaddh),5-((9-player_height)*0.4)+vertical_position,vo_t+0.8-0.8+(toaddv*z_position));      
}
if(jump_initiated==1)
{
  if(ind==1 || ina==1)
  {
    horizontal_position = jump(horizontal_position);
  }
  else
  {
    z_position = jump(z_position);
  }
  if(ind==1)
  {
    toaddh = 1;
  }
  if(ina==1)
    toaddh = -1;
  if(inw==1)
    toaddv = -1;
  if(ins==1)
    toaddv = 1;
  if(vertical_position<0)
  {
    jump_initiated=0;
    horizontal_position=0;
    z_position=0;
    vertical_position=0;
    if(onboard==1 && work==1)
    {
      onboard=0;
      vo_t -= 1.0;
      work;
    }
    time_travel=0;
    if(ina==1)
      ho_t -= 0.4;
    if(ind==1)
      ho_t += 0.4;
    if(inw==1)
      vo_t -= 0.4;
    if(ins==1)
      vo_t += 0.4;
  }

}
// draw_cube(small_cube,1,5,3);
draw_cube(board,-3,4.75,board_position-4.7);
if(board_position>2.3 && board_position<3.5)
{
  board_position+=(0.02*dire);
  board_position = GetFloatPrecision(board_position,2);
  // cout << "in adding " << endl; 
}
else if(board_position>=3.5)
{
  dire*=-1;
  board_position+=(0.05*dire);
  board_position = GetFloatPrecision(board_position,2);
}
else if(board_position<=2.3)
{
  dire*=-1;
  board_position+=(0.05*dire);
  board_position = GetFloatPrecision(board_position,2);
}
draw_cube(plane,-68,-10,60);
for(int i=0;i<10;i++)
{
  for(int j=0;j<10;j++)
  {
      for(int k=0;k<test[i][j];k++)
      {
        if(k%2==0 && k<=9)
          draw_cube(rectangle,-3+j*0.4,-2+k*0.4+3.4,-i*0.4);
        else
          draw_cube(trans,-3+j*0.4,-2+k*0.4+3.4,-i*0.4);
      }
  }
}


// cout << int(ho_t*10)/4 << " " <<  -1*int(vo_t*10)/4 << endl;
if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]<player_height && jump_initiated==0)
{
  if(board_position-4.3>-1.3 && (5-((9-player_height)*0.4)+vertical_position)>4.8 && (-2.9+ho_t-0.1+(horizontal_position*toaddh)<=-2.8))
  {
    // cout << "1---" << endl;
    onboard=1;
  }
  // cout << int(-1*vo_t)%4 <<
  if(int(10*vo_t)%4==0 && int(10*ho_t)%4==0 && onboard==0)
  {
    player_height -= 0.04;
  }
  else if(test[(-1*int(vo_t*10)/4)+1][int(ho_t*10)/4]<player_height && onboard==0)
  {
    player_height -=0.04;
  }
  // cout << vo_t+0.8-0.6+(toaddv*z_position) << endl;
	// if(fall>-3.6)
	// 	fall-=0.06;
	// else
	// 	arrow_work =1;
}

if(( (int(ho_t*10)/4)<0 || ((-1*int(vo_t*10)/4)<0)) && player_height>0)
{
  player_height -=0.04;
  cout << player_height << endl;
}

// cout << vo_t+0.8-0.6+(toaddv*z_position) << endl;
// cout  << test[-1*int(vo_t*10)/4+1][int(ho_t*10)/4] << "***"<< int(vo_t*10)/4 << " &&&&&" << vo_t <<endl;

// cout << -2.9+ho_t-0.1+(horizontal_position*toaddh) << endl;

if((-1*int(vo_t*10)/4)==9 && (int(ho_t*10)/4)==9)
{
  cout << "You Win" << endl;
}



// cout << test[-1*int(vo_t*10)/4][int(ho_t*10)/4] << " " << -1*int(vo_t*10)/4 << " " << int(ho_t*10)/4 <<  endl;

// if(test[-1*int(vo_t*10)/4][int(ho_t*10)/4]>9)
// {
//   obstacle=1;
//   if(x_walk==1)
//   {

//   }
// }
// cout << obstacle << endl;

  // Increment angles
float increments = 1;
if((rectangle_rotation>25 || rectangle_rotation<-25) && no_of_walks>=0)
{
  rectangle_rot_dir *=-1;
  // cout << "{{{{{{{{{{" << no_of_walks <<  endl;
  if(no_of_walks>0)
    no_of_walks--;
  // cout << "<<<<<<" << no_of_walks <<  endl;
}

  // cout << x_turn << " " << z_turn << endl;
  // camera_rotation_angle++; // Simulating camera rotation
  // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback (window,cbfun);
    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	// createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	rectangle = createRectangle (0.2,0.2,0.2,GL_FILL);
  trans = createRectangle(0.2,0.2,0.2,GL_LINE);
  forplayer = createRectangle(0.05,0.2,0.05,GL_FILL);
  body = createRectangle(0.2,0.2,0.05,GL_FILL);
  body_x = createRectangle(0.05,0.2,0.2,GL_FILL);
  arrow2 = createTriangle(0.4,0.3,0,0);
  small_cube = createRectangle(0.05,0.05,0.05,GL_FILL);
  board = createRectangle(0.2,0.05,0.2,GL_FILL);
  createPlane();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
