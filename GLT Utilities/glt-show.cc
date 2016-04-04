/** glt-show: Program for displaying GLT files */

// C Includes
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <GL/glew.h>
#include <GL/gl.h>      // For everything GL, except window
#include <GL/glext.h>
#include <GLFW/glfw3.h> // Window

#include "glt/glt.hpp" // GLT utility

int main(int argc, char **argv){
    if(argc <= 1){
        fprintf(stderr, "Usage: %s <file> [options]\n", argv[0]);
        return 3;
    }

    bool nearest = false;
    for(int i = 0; i < argc; ++i){
        if(strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nearest") == 0)
            nearest = true;
    }

    // Open the file.
    glt::file file(argv[1]);

    // Print a warning if the file format is not known.
    if(file.get_texture_header().format != GLT_PIXEL_FORMAT_RGBA &&
       file.get_texture_header().format != GLT_PIXEL_FORMAT_BGRA)
        fprintf(stderr, "Warning: Unknown pixel format \'%d\'",
                        file.get_texture_header().format);

    // Initialize GLFW
    if(!glfwInit()){
        fprintf(stderr, "Could not initialize GLFW.\n");
        return 1;
    }

    // Create a resizable 800x600 window
    GLFWwindow *window = glfwCreateWindow(
                            800,
                            600,
                            argv[1], NULL, NULL);

    if(window == NULL){
        fprintf(stderr, "Could not create a GLFW window.\n");

        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_RESIZABLE, 1);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    
    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Create a GL texture and fill it with the file data.
    glEnable(GL_TEXTURE_2D);

    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, nearest ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, nearest ? GL_NEAREST : GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 file.get_texture_header().gl_format(),
                 file.get_texture_header().width,
                 file.get_texture_header().height,
                 0,
                 file.get_texture_header().gl_format(),
                 GL_UNSIGNED_BYTE,
                 file.get_texture_data());

    // Texture width and height.
    glt::texture_header header = file.get_texture_header();

    // Render loop.
    float scale = 1;

    u64 tw = file.get_texture_header().width;
    u64 th = file.get_texture_header().height;
    while(!glfwWindowShouldClose(window)){
        // Viewport.
        int ww, wh;
        glfwGetFramebufferSize(window, &ww, &wh);

        glViewport(0, 0, ww, wh);

        // Update the scale (Scale up or down)
        if(ww < tw || wh < th){
            for(float i = 1; i <= 100; i += .001f){
                if(ww >= tw / i && wh >= th / i){
                    scale = ((float) 1) / i;

                    break;
                }
            }
        }else if(ww > tw || wh > th){
            for(float i = 1; i <= 100; i += .001f){
                if(ww == floor(tw * i) || wh == floor(th * i)){
                    scale = ((float) 1) * i;

                    break;
                }
            }
        }else{
            scale = 1;
        }

        // Update the projection.
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(-(ww / 2), ww / 2, wh / 2, -(wh / 2), 1, -1);

        glScalef(scale, scale, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the texture.
        glBegin(GL_QUADS);
            glTexCoord2f(0, 0);
            glVertex3f(-((s64)(tw / 2)),
                       -((s64)(th / 2)), 0);

            glTexCoord2f(1, 0);
            glVertex3f( (tw / 2),
                       -((s64)(th / 2)), 0);

            glTexCoord2f(1, 1);
            glVertex3f((tw / 2),
                       (th / 2), 0);

            glTexCoord2f(0, 1);
            glVertex3f(-((s64)(tw / 2)),
                        (th / 2), 0);
        glEnd();

        // Update the screen.
        glfwSwapBuffers(window);

        // Wait for events.
        glfwWaitEvents();
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
