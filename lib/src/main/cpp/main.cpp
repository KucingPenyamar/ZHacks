#include "main.h"
#include "lib_dump.h"
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <cstring>
#include <thread>
#include <dlfcn.h>
#include <cinttypes>
#include "zygisk.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
// ImG
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
// EGL
#include <EGL/egl.h>
#include <GLES3/gl3.h>
// Dobby
#include "dobby.h"
#include "logger.h"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "MyModule", __VA_ARGS__)

// -- ZYGISK MODULE
class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        flog("\nonLoad()");
    }
    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Use JNI to fetch our process name
        const char *processName = env->GetStringUTFChars(args->nice_name, nullptr);
        const char *appDataDir = env->GetStringUTFChars(args->app_data_dir, nullptr);
        preSpecialize(processName, appDataDir);
        env->ReleaseStringUTFChars(args->nice_name, processName);
        env->ReleaseStringUTFChars(args->app_data_dir, appDataDir);
        flog("\preAppSpecialize()");
    }
    void postAppSpecialize(const AppSpecializeArgs *) override {
        flog("\npostAppSpecialize()");
        if (createThread) {
            std::thread mainThread(inject, gameDataDir, data, length);
            mainThread.detach();
            LOGD("Thread created");
        }   
    }
private:
    Api *api;
    JNIEnv *env;
    bool createThread = false;
    char *gameDataDir;
    void *data;
    size_t length;

    void preSpecialize(const char *processName, const char *appDataDir) {
        flog("\npreSpecialize()");
        // Check is the current process is match with targeted process
        if (strcmp(processName, targetProcessName) == 0) {
            LOGD("Success, setup a thread");
            createThread = true;
            gameDataDir = new char[strlen(appDataDir) + 1];
            strcpy(gameDataDir, appDataDir);
            #if defined(__x86_64__)
                auto path = "zygisk/arm64-v8a.so";
            #endif 
            #if defined(__i386__)
                auto path = "zygisk/armeabi-v7a.so";
            #endif
            #if defined(__i386__) || defined(__x86_64__)
                int dirfd = api->getModuleDir();
                int fd = openat(dirfd, path, O_RDONLY);
                if (fd != -1) {
                    struct stat sb{};
                    fstat(fd, &sb);
                    length = sb.st_size;
                    data = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
                     close(fd);
                } 
            #endif
        } else {
            LOGD("Skip, process unknown");
        }
    }
};

// Register our module class
REGISTER_ZYGISK_MODULE(MyModule)
// -- END ZYGISK

// Draw ImGui Menu
using namespace ImGui;
bool setupGui;
bool exampleCheckbox;
void MainMenu() {
    flog("Menu is Called!");
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
    	Begin("Mod Menu");
    	ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_FittingPolicyResizeDown;
    	if (BeginTabBar("Menu", tabBarFlags)) {
    		if (BeginTabItem("Player")) {
    			Checkbox("This is checkbox", &exampleCheckbox);
    			EndTabItem();
    		}
    		EndTabItem();
    	}
        End();
    }
}
void drawImGui() {
    flog("\ndrawImGui() called!");
	IMGUI_CHECKVERSION();
	CreateContext();
	ImGuiIO &io = GetIO();
	io.DisplaySize = ImVec2((float) width, (float) height);
	ImGui_ImplOpenGL3_Init("#version 300");
	StyleColorsDark(); // Default
	GetStyle().ScaleAllSizes(10.0f);
}

// -- HOOK IMGUI

// INPUT HANDLER
/* android::InputConsumer::initializeMotionEvent(android::MotionEvent*, android::InputMessage const*)
 * void InputConsumer::initializeMotionEvent(MotionEvent* event, const InputMessage* msg)
 */
void (*inputOrig)(void* thiz, void* event, void* msg);
void inputHook(void *thiz, void *event, void *msg) {
    inputOrig(thiz, event, msg);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
} 
// EGLSWAPBUFFER HANDLER
EGLBoolean (*eglSwapBufferOrig)(EGLDisplay display, EGLSurface surface);
EGLBoolean eglSwapBufferHook(EGLDisplay display, EGLSurface surface) {
    flog("\nEntering eglSwapBuffer");
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
	if (!setupGui) {
		drawImGui();
		setupGui = true;
	}
	ImGuiIO &io = GetIO();
	ImGui_ImplOpenGL3_NewFrame();
	NewFrame();
	MainMenu();
	EndFrame();
	Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return eglSwapBufferOrig(display, surface);	
}

// INJECT OUR MENU
void inject(const char *gameDataDir, void *data, size_t length) {
    logger(gameDataDir);
    flog("\n\nMain Thread Called!\n\n");
    // start_dump(gameDataDir);
    /* Dobby api
    int DobbyHook(void *address, dobby_dummy_func_t replace_func, dobby_dummy_func_t *origin_func);
    DobbyHook(sym_addr, (dobby_dummy_func_t)fake_##name, (dobby_dummy_func_t *)&orig_##name);            
     */
    // HOOK INPUT SYMBOL
    auto input_handler = DobbySymbolResolver("/system/lib/libinput.so", "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE");
    if (NULL != input_handler) {
        flog("\nlibinput.so found!");
    } else {
        flog("\nlibinput.so not found!");
    }
    DobbyHook((void *)input_handler, (void *)inputHook, (void**)inputOrig);
    // HOOK EGLSWAPBUFFER
    auto egl_handler = DobbySymbolResolver("libEGL.so", "eglSwapBuffers");
    if (NULL != egl_handler) {
        flog("\nlibEGL.so success!");
    } else {
        flog("\nlibEGL.so error!");
    }
    DobbyHook((void *)egl_handler, (void *)eglSwapBufferHook, (void **)eglSwapBufferOrig);
}
// -- END INJECT