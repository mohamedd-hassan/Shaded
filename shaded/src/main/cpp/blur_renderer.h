#ifndef BLUR_RENDERER_H
#define BLUR_RENDERER_H

#include <GLES2/gl2.h>
#include "egl_helper.h"
// Include your EGL helper header
// #include "egl_helper.h"

class BlurRenderer {
public:
    BlurRenderer(int width, int height);
    ~BlurRenderer() = default;

    void initialize();
    GLuint uploadBitmapAsTexture(unsigned char* pixels, int width, int height);
    void render(GLuint textureId, int width, int height, float radius);
    void readFBO(unsigned char* pixels, int width, int height);

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
    GLint uniformDirection_;
    EGLHelper eglHelper_;  // Offscreen EGL context manager
    // Framebuffers for two-pass rendering
    GLuint framebuffer1_;  // For horizontal pass output
    GLuint framebuffer2_;  // For vertical pass output
    GLuint fboTexture1_;   // Intermediate texture
    GLuint fboTexture2_;   // Final output texture

    // Current framebuffer dimensions
    int currentWidth_;
    int currentHeight_;

    bool initialized_;

    // Helper methods
    void setupFullscreenQuad();
    void setupFramebuffers();
    void resizeFramebuffers(int width, int height);
    void compileShaders();
    GLuint compileShader(GLenum type, const char* source);

    // Rendering passes
    void renderHorizontalPass(GLuint textureId, int width, int height, float radius);
    void renderVerticalPass(int width, int height, float radius);
    void renderDirect(GLuint textureId, int width, int height);
    void drawQuad(GLuint shaderProgram);
};

#endif // BLUR_RENDERER_H

