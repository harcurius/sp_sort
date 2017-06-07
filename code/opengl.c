#include <GL/gl.h>

static void
DrawQuad(float x, float y, float w, float h, v4 Color)
{
    glBegin(GL_QUADS);
    glColor4f(Color.r, Color.g, Color.b, Color.a);
    glVertex2f(x,y);
    glColor4f(Color.r, Color.g, Color.b, Color.a);
    glVertex2f(x+w,y);
    glColor4f(Color.r, Color.g, Color.b, Color.a);
    glVertex2f(x+w,y+h);
    glColor4f(Color.r, Color.g, Color.b, Color.a);
    glVertex2f(x,y+h);
    glEnd();
}

static void
BeginRender(int Width, int Height)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    float Matrix[] =
    {
        2.0f/Width,0,0,0,
        0,2.0f/Height,0,0,
        0,0,1,0,
        -1,-1,0,1
    };
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(Matrix);
    glViewport(0,0,Width, Height);
}