#ifndef UNBOUNDED_BLUR_RENDERER_H
#define UNBOUNDED_BLUR_RENDERER_H

#include <GLES2/gl2.h>
#include "egl_helper.h"

class UnboundedBlurRenderer {
public:
    explicit UnboundedBlurRenderer(int maxWidth, int maxHeight);

    GLuint uploadBitmapAsTexture(unsigned char* pixels, int width, int height);

    void render(GLuint textureId, int inputWidth, int inputHeight, float radius, 
                int& outputWidth, int& outputHeight);

    void readFBO(unsigned char* pixels, int width, int height);

private:
    void initialize();
    void compileShaders();
    GLuint compileShader(GLenum type, const char* source);
    void setupFullscreenQuad();
    void setupFramebuffer();
    void resizeFramebuffer(int width, int height);
    int calculateOutputSize(int inputSize, float radius);

    EGLHelper eglHelper_;

    GLuint quadVBO_;
    GLuint shaderProgram_;
    GLint attrPos_;
    GLint attrTexCoord_;
    GLint uniformTexture_;
    GLint uniformRadius_;
    GLint uniformTextureSize_;
    GLint uniformInputSize_;
    GLint uniformOutputSize_;

    GLuint framebuffer_;
    GLuint fboTexture_;
    int currentFBOWidth_;
    int currentFBOHeight_;

    bool initialized_;
};

#endif // UNBOUNDED_BLUR_RENDERER_H