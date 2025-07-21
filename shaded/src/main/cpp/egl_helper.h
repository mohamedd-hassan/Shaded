#ifndef EGL_HELPER_H
#define EGL_HELPER_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>

class EGLHelper {
public:
    EGLHelper(int width, int height);
    ~EGLHelper();

    void makeCurrent();

private:
    void initialize(int width, int height);

    EGLDisplay display_;
    EGLContext context_;
    EGLSurface surface_;
};

#endif // EGL_HELPER_H
