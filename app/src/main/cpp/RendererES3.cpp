#include "RendererES3.h"
#include <EGL/egl.h>
#include <malloc.h>

// returns true if a GL error occurred
extern bool checkGlError(const char* funcName);
extern GLuint createShader(GLenum shaderType, const char* src);
extern GLuint createProgram(const char* vtxSrc, const char* fragSrc);

static const char VERTEX_SHADER[] =
    "#version 300 es\n"
    "layout(location = 0) in vec2 inUV;\n"
    "out vec4 vUV;\n"
    "void main() {\n"
    "gl_Position = vec4(inUV.xy*2.0-1.0,0.5,1.0);"
    "vUV = vec4(inUV, 0., 0.);"
    "}\n";

static const char FRAGMENT_SHADER[] =
    "#version 300 es\n"
    "precision mediump float;\n"
    "uniform vec2 rotation;"
    "in vec4 vUV;\n"
    "out vec4 outColor;\n"
"#define MAX_RAYMARCH_ITER 32\n"
"#define MIN_RAYMARCH_DELTA 0.01\n"
"float map(vec3 p) {"
"    float d = length(p) - 0.3;"
"    d = min(d, length(p-vec3(0.6, -0.4, 0.)) - 0.45);"
"    d = min(d, length(p-vec3(-0.4, 0.4, -0.2)) - 0.65);"
"    d = min(d, length(p-vec3(-0.3, -0.8, 0.)) - 0.3);"
"    d = min(d, length(p-vec3(0.7, 0.8, 0.)) - 0.4);"
"    d = min(d, p.z-0.);"
"d = min(d, p.x+1.0);"
"d = min(d, -p.x+1.0);"
"d = min(d, p.y+1.0);"
"d = min(d, -p.y+1.0);"

"    return d;"
"}"
"float raymarch(vec3 ray_start, vec3 ray_dir){"
"    float dist = 0.0;"
"    for (int i = 0; i <= MAX_RAYMARCH_ITER; i++)"
"    {"
"        vec3 p = ray_start + ray_dir * dist;"
"        float d = map(p);"
"        if (d < MIN_RAYMARCH_DELTA)"
"        {"
"           return dist;"
"        }"
"        dist += d;"
"    }"
"    return -1.;"
"}"
"void main() {"
"    vec2 uv = vUV.xy;"
"    vec2 ng = rotation;"
"    vec3 offset = vec3(ng.xy, -length(ng)*0.1) * 2.;"
"    vec3 camPos = vec3(0., 0., 1.5) + offset;"
"    vec3 camTarget = vec3((uv-0.5) *2.0, 0.49);"
"    vec3 camDir = normalize(camTarget - camPos);"
"    float d = raymarch(camPos, camDir);"
"    vec3 cl = camPos + camDir * d;"
"    vec3 p1 = floor(cl * vec3(2.));"
"    vec3 p2 = floor(cl * vec3(4.));"
"    vec3 p3 = floor(cl * vec3(8.));"
"    vec3 col = vec3(mod((p1.x + p1.y + p1.z), 2.) * (cl.z*2. +0.3 ) + 0.1);"
"    col += vec3(abs(mod((p2.x + p2.y + p2.z), 2.) * 0.04));"
"    col += vec3(abs(mod((p3.x + p3.y + p3.z), 2.) * 0.02));"
"    outColor = vec4(col,1.0);"
//"outColor = vec4(1.0,0.0,1.0,1.0);"
"}\n";


bool checkGlError(const char* funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        ALOGE("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

GLuint createShader(GLenum shaderType, const char* src) {
    GLuint shader = glCreateShader(shaderType);
    if (!shader) {
        checkGlError("glCreateShader");
        return 0;
    }
    glShaderSource(shader, 1, &src, NULL);

    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
                ALOGE("Could not compile %s shader:\n%s\n",
                      shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
                      infoLog);
                free(infoLog);
            }
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createProgram(const char* vtxSrc, const char* fragSrc) {
    GLuint vtxShader = 0;
    GLuint fragShader = 0;
    GLuint program = 0;
    GLint linked = GL_FALSE;

    vtxShader = createShader(GL_VERTEX_SHADER, vtxSrc);
    if (!vtxShader)
        goto exit;

    fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fragShader)
        goto exit;

    program = glCreateProgram();
    if (!program) {
        checkGlError("glCreateProgram");
        goto exit;
    }
    glAttachShader(program, vtxShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        ALOGE("Could not link program");
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            if (infoLog) {
                glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
                ALOGE("Could not link program:\n%s\n", infoLog);
                free(infoLog);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    exit:
    glDeleteShader(vtxShader);
    glDeleteShader(fragShader);
    return program;
}

static void printGlString(const char* name, GLenum s) {
    const char* v = (const char*)glGetString(s);
    ALOGV("GL %s: %s\n", name, v);
}

class RendererES3: public Renderer {
public:
    RendererES3();
    virtual ~RendererES3();
    bool init();

private:
    virtual void draw();

    const EGLContext mEglContext;

};

Renderer* createES3Renderer(ANativeWindow* nativeWindow) {
    Renderer* renderer = new Renderer;
    if (!renderer->initSurface(nativeWindow) || !renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

Renderer::Renderer() : mMouseDeltaX(0)
        , mMouseDeltaY(0)
        , mMouseX(0.f)
        , mMouseY(0.f)
        , mWidth(1)
        , mHeight(1)
        , mProgram(0)
        , fsVA(0)
        , mGLFullScreenVertexArrayName(0)
{
}

Renderer::~Renderer() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
    if (eglGetCurrentContext() != mEglContext)
        return;
    glDeleteVertexArrays(1, &mGLFullScreenVertexArrayName);
    glDeleteBuffers(1, &fsVA);
    glDeleteProgram(mProgram);
}

bool Renderer::initSurface(ANativeWindow* nativeWindow)
{
    ALOGV("NativeEngine: initializing display.");
    mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_FALSE == eglInitialize(mEglDisplay, 0, 0)) {
        ALOGE("NativeEngine: failed to init display, error %d", eglGetError());
        return false;
    }
    ALOGV("NativeEngine: initializing surface.");

    EGLint numConfigs;

    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // request OpenGL ES 2.0
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
    };

    // since this is a simple sample, we have a trivial selection process. We pick
    // the first EGLConfig that matches:
    eglChooseConfig(mEglDisplay, attribs, &mEglConfig, 1, &numConfigs);

    // create EGL surface
    mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, nativeWindow, NULL);
    if (mEglSurface == EGL_NO_SURFACE) {
        ALOGE("Failed to create EGL surface, EGL error %d", eglGetError());
        return false;
    }

    ALOGV("NativeEngine: successfully initialized surface.");

    EGLint attribList[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE }; // OpenGL 2.0
    ALOGV("NativeEngine: initializing context.");

    // create EGL context
    mEglContext = eglCreateContext(mEglDisplay, mEglConfig, NULL, attribList);
    if (mEglContext == EGL_NO_CONTEXT) {
        ALOGE("Failed to create EGL context, EGL error %d", eglGetError());
        return false;
    }

    ALOGV("NativeEngine: successfull initialized context.");

    if (EGL_FALSE == eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
        ALOGE("NativeEngine: eglMakeCurrent failed, EGL error %d", eglGetError());
        return false;
    }
    return true;
}

bool Renderer::init()
{
    mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!mProgram)
        return false;

    static const float fsVts[] = { 0.f,0.f, 2.f,0.f, 0.f,2.f };
    glGenBuffers(1, &fsVA);
    glBindBuffer(GL_ARRAY_BUFFER, fsVA);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * 2, fsVts, GL_STATIC_DRAW);

    glGenVertexArrays(1, &mGLFullScreenVertexArrayName);
    glBindVertexArray(mGLFullScreenVertexArrayName);
    glBindBuffer(GL_ARRAY_BUFFER, fsVA);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

void Renderer::resize(int w, int h)
{
    mWidth = w;
    mHeight = h;
    glViewport(0, 0, w, h);
}

void Renderer::render()
{
    // whole screen is rendered. no need to erase it
    //glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
    glClear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT);

    mMouseX += mMouseDeltaX;
    mMouseY += mMouseDeltaY;
    mMouseDeltaX *= 0.9f;
    mMouseDeltaY *= 0.9f;

    glUseProgram(mProgram);
    glUniform2f(glGetUniformLocation(mProgram, "rotation"), -mMouseDeltaY*0.2f, mMouseDeltaX*0.2f);
    glUniform1f(glGetUniformLocation(mProgram, "ratio"), float(mWidth)/float(mHeight));
    glBindVertexArray(mGLFullScreenVertexArrayName);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    checkGlError("Renderer::render");

    eglSwapBuffers(mEglDisplay, mEglSurface);
}
