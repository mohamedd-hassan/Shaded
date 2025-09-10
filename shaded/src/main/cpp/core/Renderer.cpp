#include "Renderer.h"
#include <android/log.h>


void GlClearError() {
    while (!glGetError());
}

bool GlLogCall(const char *function, const char *file, int line) {
    while (GLenum error = glGetError()) {
        __android_log_print(ANDROID_LOG_DEBUG, "OpenGlError",
                            "%d in function %s at file %s, on line %d", error, function, file,
                            line);
        return false;
    }
    return true;
}