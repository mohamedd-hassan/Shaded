#ifndef UNBOUNDED_BLUR_H
#define UNBOUNDED_BLUR_H

#include <GLES2/gl2.h>
#include "egl_helper.h"
// Include your EGL helper header
// #include "egl_helper.h"

class UnboundedBlurRenderer {
public:
    UnboundedBlurRenderer(int maxWidth, int maxHeight);
    ~UnboundedBlurRenderer() = default;

    void initialize();
    GLuint uploadBitmapAsTexture(unsigned char* pixels, int width, int height);
    void render(GLuint textureId, int inputWidth, int inputHeight,
                float radius, int& outputWidth, int& outputHeight);
    void readFBO(unsigned char* pixels, int width, int height);

    static int calculateOutputSize(int inputSize, float radius);

private:
    // EGL and OpenGL setup
    // EGLHelper eglHelper_;  // Replace with your actual EGL helper class

    // OpenGL resources
    GLuint quadVBO_;
    GLuint horizontalShaderProgram_;
    GLuint verticalShaderProgram_;

    // Shader attributes and uniforms
    GLint attrPos_;
    GLint attrTexCoord_;
    GLint uniformTexture_;
    GLint uniformRadius_;
    GLint uniformTextureSize_;
    GLint uniformInputSize_;
    GLint uniformOutputSize_;

    EGLHelper eglHelper_;  // Offscreen EGL context manager


    // Framebuffers for two-pass rendering
    GLuint framebuffer1_;  // For horizontal pass output
    GLuint framebuffer2_;  // For vertical pass output
    GLuint fboTexture1_;   // Intermediate texture
    GLuint fboTexture2_;   // Final output texture

    // Current framebuffer dimensions
    int currentFBOWidth_;
    int currentFBOHeight_;

    bool initialized_;

    // Helper methods
    void setupFullscreenQuad();
    void setupFramebuffers();
    void resizeFramebuffers(int width, int height);
    void compileShaders();
    GLuint compileShader(GLenum type, const char* source);

    // Rendering passes
    void renderHorizontalPass(GLuint textureId, int inputWidth, int inputHeight,
                              int outputWidth, int outputHeight, float radius);
    void renderVerticalPass(int inputWidth, int inputHeight,
                            int outputWidth, int outputHeight, float radius);
    void renderDirect(GLuint textureId, int inputWidth, int inputHeight,
                      int outputWidth, int outputHeight);
    void drawQuad(GLuint shaderProgram);
};

#endif // UNBOUNDED_BLUR_H