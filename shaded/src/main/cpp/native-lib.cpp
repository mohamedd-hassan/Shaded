#include <jni.h>
#include <GLES2/gl2.h>
#include <android/bitmap.h>
#include "blur_renderer.h"
#include "unbounded_blur.h"

static BlurRenderer rectangleRenderer(200, 200);
static UnboundedBlurRenderer unboundedRenderer(200, 200);

extern "C"
JNIEXPORT jobject JNICALL
Java_io_sifr_shaded_blurProcessor_BlurNative_blurBitmap(JNIEnv* env, jobject thiz,
                                                        jobject inputBitmap, jfloat radius) {
    // Keep the original rectangle blur implementation unchanged
    AndroidBitmapInfo info;
    void* pixels;

    if (AndroidBitmap_getInfo(env, inputBitmap, &info) < 0) {
        return nullptr;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return nullptr;
    }

    if (AndroidBitmap_lockPixels(env, inputBitmap, &pixels) < 0) {
        return nullptr;
    }

    GLuint texId = rectangleRenderer.uploadBitmapAsTexture(reinterpret_cast<unsigned char*>(pixels),
                                                           info.width, info.height);

    rectangleRenderer.render(texId, info.width, info.height, radius);
    rectangleRenderer.readFBO(reinterpret_cast<unsigned char*>(pixels), info.width, info.height);

    AndroidBitmap_unlockPixels(env, inputBitmap);

    return inputBitmap;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_io_sifr_shaded_blurProcessor_BlurNative_blurBitmapUnbounded(JNIEnv* env, jobject thiz,
                                                                 jobject inputBitmap, jfloat radius) {
    AndroidBitmapInfo info;
    void* pixels;

    if (AndroidBitmap_getInfo(env, inputBitmap, &info) < 0) {
        return nullptr;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        return nullptr;
    }

    if (AndroidBitmap_lockPixels(env, inputBitmap, &pixels) < 0) {
        return nullptr;
    }

    // Upload input bitmap as texture
    GLuint texId = unboundedRenderer.uploadBitmapAsTexture(reinterpret_cast<unsigned char*>(pixels),
                                                           info.width, info.height);

    // Calculate output dimensions
    int outputWidth, outputHeight;
    unboundedRenderer.render(texId, info.width, info.height, radius, outputWidth, outputHeight);

    AndroidBitmap_unlockPixels(env, inputBitmap);

    // Create output bitmap with expanded dimensions
    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMethod = env->GetStaticMethodID(bitmapClass, "createBitmap",
                                                          "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

    jclass configClass = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID argb8888Field = env->GetStaticFieldID(configClass, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
    jobject argb8888Config = env->GetStaticObjectField(configClass, argb8888Field);

    jobject outputBitmap = env->CallStaticObjectMethod(bitmapClass, createBitmapMethod,
                                                       outputWidth, outputHeight, argb8888Config);

    // Lock output bitmap pixels
    void* outputPixels;
    if (AndroidBitmap_lockPixels(env, outputBitmap, &outputPixels) < 0) {
        return nullptr;
    }

    // Read the blurred result
    unboundedRenderer.readFBO(reinterpret_cast<unsigned char*>(outputPixels), outputWidth, outputHeight);

    AndroidBitmap_unlockPixels(env, outputBitmap);

    // Clean up texture
    glDeleteTextures(1, &texId);

    return outputBitmap;
}