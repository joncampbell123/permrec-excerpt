
#if defined(WIN32)
# include <windows.h>
# include <io.h>
# if !defined(__MINGW32__) /* MinGW does not have these headers */
#  include <shcore.h>
#  include <shellscalingapi.h>
# endif
typedef int ssize_t;
# define htole32(x) (x)
# define le32toh(x) (x)
# define le16toh(x) (x)
# pragma warning(disable : 4996) /* I don't care if you prefer I use your weird underscore posix functions. Shut up. */
#elif defined(__APPLE__)
 /* This is a simple compatibility shim to convert
 * BSD/Linux endian macros to the Mac OS X equivalents. */
#include <libkern/OSByteOrder.h>
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)
# include <unistd.h>
#else
# include <endian.h>
# include <unistd.h>
#endif

#if defined(WIN32) && defined(__MINGW32__) /* MinGW does not have this */
typedef enum PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE             = 0,
    PROCESS_SYSTEM_DPI_AWARE        = 1,
    PROCESS_PER_MONITOR_DPI_AWARE   = 2
} PROCESS_DPI_AWARENESS;
#endif

#include <SDL.h>
#include <SDL_main.h>

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <istream>
#include <fstream>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <array>
#include <queue>

#ifndef O_BINARY
#define O_BINARY (0)
#endif

#if defined(WIN32)
// NTS: I intend to add code that not only indicates High DPI awareness but also queries the monitor DPI
//      and then factor the DPI into DOSBox's scaler and UI decisions.
void Windows_DPI_Awareness_Init() {
    // turn off DPI scaling so DOSBox-X doesn't look so blurry on Windows 8 & Windows 10.
    // use GetProcAddress and LoadLibrary so that these functions are not hard dependencies that prevent us from
    // running under Windows 7 or XP.
    HRESULT (WINAPI *__SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS) = NULL; // windows 8.1
    BOOL (WINAPI *__SetProcessDPIAware)(void) = NULL; // vista/7/8/10
    HMODULE __user32;
    HMODULE __shcore;

    __user32 = GetModuleHandle("USER32.DLL");
    __shcore = GetModuleHandle("SHCORE.DLL");

    if (__user32)
        __SetProcessDPIAware = (BOOL(WINAPI *)(void))GetProcAddress(__user32, "SetProcessDPIAware");
    if (__shcore)
        __SetProcessDpiAwareness = (HRESULT (WINAPI *)(PROCESS_DPI_AWARENESS))GetProcAddress(__shcore, "SetProcessDpiAwareness");

    if (__SetProcessDpiAwareness)
        __SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    if (__SetProcessDPIAware)
        __SetProcessDPIAware();
}
#endif

using namespace std;

int                 want_audio_rate = -1;
int                 want_audio_channels = -1;

SDL_AudioSpec       audio_spec;
bool                audio_open = false;

SDL_Window*         mainWindow = NULL;
SDL_Surface*        mainSurface = NULL;
bool                quitting_app = false;

void GUI_OnWindowEvent(SDL_WindowEvent &wevent) {
    if (wevent.event == SDL_WINDOWEVENT_RESIZED) {
        mainSurface = SDL_GetWindowSurface(mainWindow);
        assert(mainSurface != NULL);
    }
}

bool GUI_Idle(void) {
    SDL_Event event;

    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quitting_app = true;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            GUI_OnWindowEvent(event.window);
        }
    }

    return !(quitting_app);
}

void audio_callback(void *userdata,Uint8* stream,int len) {
}

int main(int argc,char **argv) {
    (void)argc;
    (void)argv;

#if defined(WIN32)
    Windows_DPI_Awareness_Init();
#endif

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_AUDIO) != 0) {
        fprintf(stderr,"Unable to init SDL2\n");
        return 1;
    }

    memset(&audio_spec,0,sizeof(audio_spec));
    audio_spec.format = AUDIO_S16SYS;

    if (want_audio_rate >= 1000)
        audio_spec.freq = want_audio_rate;
    else
        audio_spec.freq = 48000;

    if (want_audio_channels > 0)
        audio_spec.channels = static_cast<unsigned char>(want_audio_channels);
    else
        audio_spec.channels = 2;

    audio_spec.samples = static_cast<uint16_t>(audio_spec.freq / 30);
    audio_spec.callback = audio_callback;

    if (SDL_OpenAudio(&audio_spec,NULL)) {
        fprintf(stderr,"Unable to open audio\n");
        return 1;
    }

    if (audio_spec.format != AUDIO_S16SYS) {
        fprintf(stderr,"Didn't get the audio format I wanted\n");
        return 1;
    }

    fprintf(stderr,"Audio spec result: %uHz %u-channel %u-samples/cb\n",
        audio_spec.freq,
        audio_spec.channels,
        audio_spec.samples);

    SDL_PauseAudio(0);

    if ((mainWindow=SDL_CreateWindow("CVEDIT",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_RESIZABLE)) == NULL) {
        fprintf(stderr,"Unable to create window\n");
        return 1;
    }
    mainSurface = SDL_GetWindowSurface(mainWindow);
    assert(mainSurface != NULL);

    while (!quitting_app) {
        GUI_Idle();
    }

    SDL_DestroyWindow(mainWindow);
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}
