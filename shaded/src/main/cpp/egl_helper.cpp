#include "egl_helper.h"
#include <stdexcept>

EGLHelper::EGLHelper(int width, int height)
        : display_(EGL_NO_DISPLAY), context_(EGL_NO_CONTEXT), surface_(EGL_NO_SURFACE) {
    initialize(width, height);
}

EGLHelper::~EGLHelper() {
    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (context_ != EGL_NO_CONTEXT)
            eglDestroyContext(display_, context_);

        if (surface_ != EGL_NO_SURFACE)
            eglDestroySurface(display_, surface_);

        eglTerminate(display_);
    }
}

void EGLHelper::makeCurrent() {
    if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
        throw std::runtime_error("eglMakeCurrent failed.");
    }
}

void EGLHelper::initialize(int width, int height) {
    display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display_ == EGL_NO_DISPLAY)
        throw std::runtime_error("eglGetDisplay failed.");

    if (!eglInitialize(display_, nullptr, nullptr))
        throw std::runtime_error("eglInitialize failed.");

    const EGLint configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(display_, configAttribs, &config, 1, &numConfigs) || numConfigs == 0)
        throw std::runtime_error("eglChooseConfig failed.");

    const EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, contextAttribs);
    if (context_ == EGL_NO_CONTEXT)
        throw std::runtime_error("eglCreateContext failed.");

    const EGLint pbufferAttribs[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
    };
    surface_ = eglCreatePbufferSurface(display_, config, pbufferAttribs);
    if (surface_ == EGL_NO_SURFACE)
        throw std::runtime_error("eglCreatePbufferSurface failed.");

    makeCurrent();
}
