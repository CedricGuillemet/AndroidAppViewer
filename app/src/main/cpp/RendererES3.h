#pragma once

#include <android/log.h>
#include <math.h>

#if DYNAMIC_ES3
#include "gl3stub.h"
#else
// Include the latest possible header file( GL version header )
#if __ANDROID_API__ >= 24
#include <GLES3/gl32.h>
#elif __ANDROID_API__ >= 21
#include <GLES3/gl31.h>
#include <android/native_window.h>
#include <EGL/egl.h>

#else
#include <GLES3/gl3.h>
#endif

#endif

#define DEBUG 1

#define LOG_TAG "GLES3JNI"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

// Simple renderer that draws a chamfered cube

class Renderer {
public:

    virtual ~Renderer();
    void resize(int w, int h);
    void render();
    void setMouseDelta(float dx, float dy) { mMouseDeltaX = dx; mMouseDeltaY = dy; }

protected:
    Renderer();

    bool init();
    bool initSurface(ANativeWindow* nativeWindow);
    float mMouseDeltaX, mMouseDeltaY;
    float mMouseX, mMouseY;

    int mWidth, mHeight;

    EGLDisplay mEglDisplay;
    EGLSurface mEglSurface;
    EGLContext mEglContext;
    EGLConfig mEglConfig;

    GLuint mProgram;
    GLuint fsVA;
    GLuint mGLFullScreenVertexArrayName;

    friend Renderer* createES3Renderer(ANativeWindow* nativeWindow);
};

extern Renderer* createES3Renderer(ANativeWindow* nativeWindow);

