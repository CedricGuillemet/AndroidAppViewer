#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#include "RendererES3.h"

#include "imgui/imgui.h"
#include "imgui_remote.h"
#define VCANVAS_WIDTH  8192
#define VCANVAS_HEIGHT 8192


// ----------------------------------------------------------------------------

static void ImImpl_RenderDrawLists(ImDrawData* draw_data) {
    // @RemoteImgui begin
    ImGui::RemoteDraw(draw_data->CmdLists, draw_data->CmdListsCount);
}

void LoadFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    //ImFont* my_font1 = io.Fonts->AddFontDefault();
    //ImFont* my_font2 = io.Fonts->AddFontFromFileTTF("extra_fonts/Karla-Regular.ttf", 15.0f);
    //ImFont* my_font3 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyClean.ttf", 13.0f); my_font3->DisplayOffset.y += 1;
    //ImFont* my_font4 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyTiny.ttf", 10.0f); my_font4->DisplayOffset.y += 1;
    //ImFont* my_font5 = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 20.0f, io.Fonts->GetGlyphRangesJapanese());

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    GLuint tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)tex_id;
}


void InitImGui()
{
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f/60.0f;                                    // Time elapsed since last frame, in seconds (in this sample app we'll override this every frame because our time step is variable)
/*    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
*/
    io.RenderDrawListsFn = ImImpl_RenderDrawLists;
//    io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
//    io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;

//    LoadFontsTexture();

    // @RemoteImgui begin
    ImGui::RemoteInit("0.0.0.0", 7002); // local host, local port
    //ImGui::GetStyle().WindowRounding = 0.f; // no rounding uses less bandwidth
    io.DisplaySize = ImVec2((float)VCANVAS_WIDTH, (float)VCANVAS_HEIGHT);
    // @RemoteImgui end
}

void UpdateImGui(int w, int h)
{
    ImGuiIO& io = ImGui::GetIO();
    // Setup resolution (every frame to accommodate for window resizing)

    io.DisplaySize = ImVec2((float)w, (float)h);                                   // Display size, in pixels. For clamping windows positions.

    // @RemoteImgui begin
    ImGui::RemoteUpdate();
    ImGui::RemoteInput input;
    if (ImGui::RemoteGetInput(input))
    {
        ImGuiIO& io = ImGui::GetIO();
        for (int i = 0; i < 256; i++)
            io.KeysDown[i] = input.KeysDown[i];
        io.KeyCtrl = input.KeyCtrl;
        io.KeyShift = input.KeyShift;
        io.MousePos = input.MousePos;
        io.MouseDown[0] = (input.MouseButtons & 1);
        io.MouseDown[1] = (input.MouseButtons & 2) != 0;
        io.MouseWheel += input.MouseWheelDelta;
        // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
        io.KeyMap[ImGuiKey_Tab] = ImGuiKey_Tab;
        io.KeyMap[ImGuiKey_LeftArrow] = ImGuiKey_LeftArrow;
        io.KeyMap[ImGuiKey_RightArrow] = ImGuiKey_RightArrow;
        io.KeyMap[ImGuiKey_UpArrow] = ImGuiKey_UpArrow;
        io.KeyMap[ImGuiKey_DownArrow] = ImGuiKey_DownArrow;
        io.KeyMap[ImGuiKey_Home] = ImGuiKey_Home;
        io.KeyMap[ImGuiKey_End] = ImGuiKey_End;
        io.KeyMap[ImGuiKey_Delete] = ImGuiKey_Delete;
        io.KeyMap[ImGuiKey_Backspace] = ImGuiKey_Backspace;
        io.KeyMap[ImGuiKey_Enter] = 13;
        io.KeyMap[ImGuiKey_Escape] = 27;
        io.KeyMap[ImGuiKey_A] = 'a';
        io.KeyMap[ImGuiKey_C] = 'c';
        io.KeyMap[ImGuiKey_V] = 'v';
        io.KeyMap[ImGuiKey_X] = 'x';
        io.KeyMap[ImGuiKey_Y] = 'y';
        io.KeyMap[ImGuiKey_Z] = 'z';
    }
    ImGui::NewFrame();
}

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
    if (!g_renderer)
    {
        InitImGui();
    }
    delete g_renderer;
    g_renderer = createES3Renderer(window);
    LoadFontsTexture();
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
        UpdateImGui(g_renderer->mWidth, g_renderer->mHeight);
        g_renderer->render();
        bool show_another_window = true;
        ImGui::Begin("Another Window", &show_another_window, ImVec2(200,100));
        ImGui::Text("Hello");
        static float col[3] = {1.f, 1.f, 1.f};
        static float size = 1.f;
        if (ImGui::ColorEdit3("Cube Color", col))
            g_renderer->setCubeColor(col[0], col[1], col[2]);
        if (ImGui::SliderFloat("Size", &size, 0.01f, 2.f))
            g_renderer->setCubeSize(size);
        ImGui::End();

        ImGui::Render();
        g_renderer->present();
    }
}

JNIEXPORT void JNICALL
Java_com_android_appviewer_AndroidViewAppActivity_setMouseDelta(JNIEnv* env, jobject obj, jfloat dx, jfloat dy) {
    if (g_renderer) {
        g_renderer->setMouseDelta(dx, dy);
    }
}