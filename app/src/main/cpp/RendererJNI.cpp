#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#include "RendererES3.h"

// ----------------------------------------------------------------------------

static Renderer* g_renderer = NULL;

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_appviewer_AndroidViewAppActivity_init(JNIEnv* env, jobject obj, jobject surface);
    JNIEXPORT void JNICALL Java_com_android_appviewer_AndroidViewAppActivity_resize(JNIEnv* env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_appviewer_AndroidViewAppActivity_setMouseDelta(JNIEnv* env, jobject obj, jfloat dx, jfloat dy);
    JNIEXPORT void JNICALL Java_com_android_appviewer_AndroidViewAppActivity_step(JNIEnv* env, jobject obj);
};

JNIEXPORT void JNICALL
Java_com_android_appviewer_AndroidViewAppActivity_init(JNIEnv* env, jobject obj, jobject surface) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    delete g_renderer;
    g_renderer = createES3Renderer(window);
}

JNIEXPORT void JNICALL
Java_com_android_appviewer_AndroidViewAppActivity_resize(JNIEnv* env, jobject obj, jint width, jint height) {
    if (g_renderer) {
        g_renderer->resize(width, height);
    }
}

JNIEXPORT void JNICALL
Java_com_android_appviewer_AndroidViewAppActivity_step(JNIEnv* env, jobject obj) {
    if (g_renderer) {
        g_renderer->render();
    }
}

JNIEXPORT void JNICALL
Java_com_android_appviewer_AndroidViewAppActivity_setMouseDelta(JNIEnv* env, jobject obj, jfloat dx, jfloat dy) {
    if (g_renderer) {
        float maxv = 2.f;
        if (dx < -maxv) dx = -maxv;
        if (dx > maxv) dx = maxv;

        if (dy < -maxv) dy = -maxv;
        if (dy > maxv) dy = maxv;

        g_renderer->setMouseDelta(dx, dy);
    }
}