#ifndef BLUR_RENDERER_H
#define BLUR_RENDERER_H

#include <GLES2/gl2.h>
#include "egl_helper.h"

class BlurRenderer {
public:
    explicit BlurRenderer(int width, int height);

    GLuint uploadBitmapAsTexture(unsigned char* pixels, int width, int height);

    void render(GLuint textureId, int width, int height, float radius);

    void readFBO(unsigned char* pixels, int width, int height);

private:
    void initialize();
    void compileShaders();
    GLuint compileShader(GLenum type, const char* source);
    void setupFullscreenQuad();
    void setupFramebuffer();

    EGLHelper eglHelper_;  // Offscreen EGL context manager

    GLuint quadVBO_;
    GLuint shaderProgram_;
    GLint attrPos_;
    GLint attrTexCoord_;
    GLint uniformTexture_;
    GLint uniformRadius_;
    GLint uniformTextureSize_;

    GLuint framebuffer_;
    GLuint fboTexture_;

    bool initialized_;
};

#endif // BLUR_RENDERER_H
