#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <GLES/egl.h>
#include <GLES/gl.h>

NativeWindowType createNativeWindow(void) {
    int fd = open("/dev/fb0", O_RDWR);
    return (NativeWindowType)fd;
}

static EGLint const attribute_list[] = {
    EGL_RED_SIZE, 1,
    EGL_GREEN_SIZE, 1,
    EGL_BLUE_SIZE, 1,
    EGL_NONE
};

int main(int argc, char ** argv)
{
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    EGLSurface surface;
    NativeWindowType native_window;
    EGLint num_config;

    /* get an EGL display connection */
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    /* initialize the EGL display
     * connection */
    eglInitialize(display, NULL, NULL);

    /* get an appropriate EGL frame
     * buffer configuration */
    eglChooseConfig(display, attribute_list, &config, 1, &num_config);

    /* create an EGL rendering
     * context */
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);

    /* create a native window
     * */
    native_window = createNativeWindow();

    /* create an EGL window
     * surface */
    surface = eglCreateWindowSurface(display, config, native_window, NULL);

    /* connect the
     * context to the
     * surface */
    eglMakeCurrent(display, surface, surface, context);

    /* clear the
     * color buffer
     * */
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    /* draw a triangle */
    /*
    GLfloat glverts[9];
    glVertexPointer(3, GL_FLOAT, 0, glverts);
    glEnableClientState(GL_VERTEX_ARRAY);

    int i;
    for (i = 0; i < 3; i++) {
	glverts[i*3]   = i;
	glverts[i*3+1] = i*i;
	glverts[i*3+2] = 0.0;
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
    */


    glFlush();
    eglSwapBuffers(display, surface);


    int i;
    for(i=0; i<1000; i++) {
	sleep(1);
	glClearColor(random()*1.0/RAND_MAX, random()*1.0/RAND_MAX, random()*1.0/RAND_MAX, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	eglSwapBuffers(display, surface);
    }

    return EXIT_SUCCESS;
}
