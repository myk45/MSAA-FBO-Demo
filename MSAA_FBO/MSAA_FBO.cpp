/**
 *  A simple demo of using MSAA on FBOs.
 */


#include <windows.h>
#include "FBO.h"
#include <gl/glew.h>
#include <gl/glut.h>

 
#define TEXTURE_WIDTH        256
#define TEXTURE_HEIGHT       256
 
///////////////////////////////////////////////////////////
//					Global Section						 //
///////////////////////////////////////////////////////////
GLuint fboID;
GLuint textureID;
GLuint rboID;
double ang;
bool isMSAA = false;
GLuint depthID;
GLuint frameBufID;
GLuint fbo2ID;
GLuint colorBuffer, depthBuffer, mfbo, texture1, fbo;
int width = 500, height = 500;
///////////////////////////////////////////////////////////

int render_option = 1;
 
void createFBO();
void drawFBO(int option);
void drawScene();
void drawQuad();

void renderToTexture()
{
	// This part renders a cube onto a texture. After the scene is rendered to  texture, we bind the frame buffer
	// back to 0 so that we can draw back into the default OpenGL frame buffer.
	{		
		// Draw into FBO
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		glPushAttrib(GL_VIEWPORT_BIT);
    
		glViewport(0, 0, 256, 256);
 
		// clear buffer
		//glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//glRotatef(ang += 1, 1, 1, 1);
		drawScene();
		
		// back to normal window-system-provided framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER  , 0); // unbind
		glPopAttrib();
	}

	// Now draw the actual scene.
	{
		// clear buffer
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//glRotatef(ang += 1, 0, 1, 0);

		glEnable(GL_TEXTURE_2D);		
		glBindTexture(GL_TEXTURE_2D, textureID);		
		drawQuad();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}



void drawScene()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  
	glDisable(GL_TEXTURE_2D);
 
    float amb[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
 
    float diff[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
 
    float spec[] = {0.0, 1.0, 0.0, 0.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, spec);
 
    const GLfloat blue[4] = { 0.0f, 0.3f, 1.f, 1.f };
    glMaterialfv( GL_FRONT, GL_DIFFUSE, blue );
    glMaterialfv( GL_FRONT, GL_SPECULAR, blue );
    glMaterialf( GL_FRONT, GL_SHININESS, 120.0);
 
    float position[] = {15, 15, 15, 1.0};
    //glLightfv(GL_LIGHT0, GL_POSITION, position);
 
    glColor3f(0.0, 1.0, 0.0);
    glutSolidTeapot(4.0);   
}
 
void drawQuad()
{	
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2d(-8, -8);
        glTexCoord2f(0, 1); glVertex2d(-8, 8);
        glTexCoord2f(1, 1); glVertex2d(8, 8);
        glTexCoord2f(1, 0); glVertex2d(8, -8);
    glEnd();
}
 

void idle()
{
    glutPostRedisplay();
    Sleep(10);
}
 
void display()
{
     drawFBO(render_option);
	 glutSwapBuffers();
}
 

void drawFBO(int option)
{    
	if (option == 1) {
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, mfbo);
		glPushAttrib(GL_VIEWPORT_BIT);
    
		glViewport(0, 0, width, height);
    
		// Draw scene.
		glClearColor(1.0, 1.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(ang += 1, 1, 1, 1);

		drawScene();

		glPopAttrib();
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mfbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
		// you have to unbind all fbos before you can render to the main window
		glBindFramebuffer(GL_FRAMEBUFFER, 0);    
			
		glBindTexture(GL_TEXTURE_2D, texture1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		drawQuad();		
	} else {
		glClearColor(1.0, 1.0, 0.0, 0.0);		
		glClear(GL_COLOR_BUFFER_BIT);

		renderToTexture();	

		glutSwapBuffers();
	}
    
}

/**
 *	Here, we create two FBOs. 
 *	1) One with a multi-sampled color and depth buffer.
 *	2) Another FBo, to whose color attachment point, we add a texture. -(A)
 *
 *	So, this is how we do it: 
 *		- Bind the Multisampled FBO and then render whatever you want into it.
 *		- Bind the Multisampled FBO as the READ_FRAMEBUFFER and the other FBO as DRAW_FRAMEBUFFER.
 *		- Do a blit. 
 *		- Bind the texture (A).
 */

void createFBO(int option)
{
	if (option == 1) {
		// multi sampled color buffer
		glGenRenderbuffers(1, &colorBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4 /* 4 samples */, GL_RGBA8, width, height);

		// multi sampled depth buffer
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4 /* 4 samples */, GL_DEPTH_COMPONENT, width, height);

		// create fbo for multi sampled content and attach depth and color buffers to it
		glGenFramebuffers(1, &mfbo);
		glBindFramebuffer(GL_FRAMEBUFFER, mfbo);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		// create texture
		glGenTextures(1, &texture1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);    
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    

		// create final fbo and attach texture to it
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);  
	} else {
		// create a texture object
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);    
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap generation included in OpenGL v1.4
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // No data.
		glBindTexture(GL_TEXTURE_2D, 0); // Not yet bound to anything.

		// create a framebuffer object, you need to delete them when program exits.
		glGenFramebuffers(1, &fboID);
		glBindFramebuffer  (GL_FRAMEBUFFER  , fboID);

		// Create a render-buffer object
		glGenRenderbuffers(1, &rboID);
		glBindRenderbuffer(GL_RENDERBUFFER  , rboID);
		glRenderbufferStorage(GL_RENDERBUFFER  , GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT); // Depth component needed for Depth Test
		glBindRenderbuffer(GL_RENDERBUFFER  , 0); // Attach to nothing as of now.

		// At the FBO color attachment point, attach a TEXTURE.
		glFramebufferTexture2D  (GL_FRAMEBUFFER  , GL_COLOR_ATTACHMENT0  , GL_TEXTURE_2D, textureID, 0);

		// At the FBO depth attachment point, attach a RENDER BUFFER OBJECT.
		glFramebufferRenderbuffer  (GL_FRAMEBUFFER  , GL_DEPTH_ATTACHMENT  , GL_RENDERBUFFER  , rboID);

		glBindFramebuffer(GL_FRAMEBUFFER  , 0);		
	}
}
 
void init()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10, 10, -10, 10, -10, 10);
 
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
 
    //createTexture();
    createFBO(render_option);
}
 
void keyboard(unsigned char c, int x, int y)
{
        // TODO: Enable/Disable MSAA on FBOs here?
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
 
    glutInitWindowSize(500, 500);
    glutCreateWindow("MSAA FBO demo");
 
    glewInit();
    init();
 
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
 
    glutMainLoop();
 
    return 0;
}
