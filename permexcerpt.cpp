
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

#include <sys/types.h>
#include <sys/stat.h>

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

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
#include <libavutil/pixfmt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/samplefmt.h>
#include <libavutil/pixelutils.h>

#include <libavcodec/avcodec.h>
#include <libavcodec/version.h>

#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavformat/version.h>

#include <libswscale/swscale.h>
#include <libswscale/version.h>

#include <libswresample/swresample.h>
#include <libswresample/version.h>
}

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

SDL_Rect            display_region;

SDL_AudioSpec       audio_spec;
bool                audio_open = false;

SDL_Window*         mainWindow = NULL;
SDL_Surface*        mainSurface = NULL;
bool                quitting_app = false;

void UpdateDisplayRect(void) {
    if (mainSurface) {
        display_region.x = 0;
        display_region.y = 0;
        display_region.w = mainSurface->w;
        display_region.h = mainSurface->h;
    }
    else {
        display_region.x = 0;
        display_region.y = 0;
        display_region.w = 32;
        display_region.h = 32;
    }
}

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
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                quitting_app = true;
            }
        }
    }

    return !(quitting_app);
}

void audio_callback(void *userdata,Uint8* stream,int len) {
}

class InputFile {
public:
    class tracking {
    public:
        int             width,height,linesize,channels,sample_rate;
        uint64_t        channel_layout;
        AVPixelFormat   format;
    public:
        tracking() {
            clear();
        }
    public:
        void clear(void) {
            width = -1;
            height = -1;
            linesize = -1;
            channels = -1;
            sample_rate = -1;
            channel_layout = 0;
            format = AV_PIX_FMT_NONE;
        }
        bool operator==(const tracking &o) {
            return  width ==            o.width &&
                    height ==           o.height &&
                    linesize ==         o.linesize &&
                    channels ==         o.channels &&
                    sample_rate ==      o.sample_rate &&
                    channel_layout ==   o.channel_layout &&
                    format ==           o.format;
        }
        bool operator!=(const tracking &o) {
            return !(*this == o);
        }
    };
    class Stream {
    public:
        Stream() {
        }
        ~Stream() {
            close();
        }
        void close(void) {
            close_src_frame();
            close_dst_frame();
            close_converter();
        }
        void close_src_frame(void) {
            close_src_video();
            close_src_audio();
        }
        void close_dst_frame(void) {
            close_dst_video();
            close_dst_audio();
        }
        void close_dst_video(void) {
            av_frame_free(&dst_video_frame);
        }
        void close_dst_audio(void) {
            av_frame_free(&dst_audio_frame);
        }
        void close_src_video(void) {
            av_frame_free(&src_video_frame);
        }
        void close_src_audio(void) {
            av_frame_free(&src_audio_frame);
        }
        void close_swr_audio(void) {
            swr_free(&swr_audio);
        }
        void close_sws_video(void) {
            if (sws_video != NULL) {
                sws_freeContext(sws_video);
                sws_video = NULL;
            }
        }
        void close_converter(void) {
            close_swr_audio();
            close_sws_video();
        }
    public:
        bool            codec_open = false;
        bool            is_video = false;
        bool            is_audio = false;
        tracking        src_trk;
        AVFrame*        src_video_frame = NULL;
        AVFrame*        src_audio_frame = NULL;
        tracking        dst_trk;
        AVFrame*        dst_video_frame = NULL;
        AVFrame*        dst_audio_frame = NULL;
        SwrContext*     swr_audio = NULL;
        SwsContext*     sws_video = NULL;
    };
public:
    InputFile() {
    }
    ~InputFile() {
        close();
    }
public:
    bool open(const std::string &path) {
        if (is_open())
            close();
        if (path.empty())
            return false;

        file_path = path;
        assert(avfmt == NULL);
        if (avformat_open_input(&avfmt,file_path.c_str(),NULL,NULL) < 0) {
            fprintf(stderr,"avformat: failed to open input file %s\n",file_path.c_str());
            close();
            return false;
        }

        if (avformat_find_stream_info(avfmt,NULL) < 0)
            fprintf(stderr,"avformat: Did not find stream info for %s\n",file_path.c_str());

        av_dump_format(avfmt, 0, file_path.c_str(), 0);
        streams.resize(avfmt_stream_count());

        print_fmt_debug();
        open_stream_codecs();

        return true;
    }
    const std::string &get_path(void) const {
        return file_path;
    }
    bool is_open(void) const {
        return open_flag;
    }
    void close(void) {
        avpkt_free();
        close_avformat();
        open_flag = false;
        file_path.clear();
    }
    void print_fmt_debug(void) {
        if (avfmt != NULL) {
            fprintf(stderr,"Format: %d streams, start_time=%lld/%lld duration=%lld/%lld bitrate=%lld packet_size=%u\n",
                static_cast<int>(avfmt->nb_streams),
                static_cast<signed long long>(avfmt->start_time),
                static_cast<signed long long>(AV_TIME_BASE),
                static_cast<signed long long>(avfmt->duration),
                static_cast<signed long long>(AV_TIME_BASE),
                static_cast<signed long long>(avfmt->bit_rate),
                avfmt->packet_size);
        }
    }
    void print_stream_debug(const size_t i) {
        AVStream *s = avfmt_stream(i);
        if (s != NULL) {
            fprintf(stderr,"Stream %zu: Stream time_base=%llu/%llu",i,
                static_cast<unsigned long long>(s->time_base.num),static_cast<unsigned long long>(s->time_base.den));
            if (s->start_time != AV_NOPTS_VALUE)
                fprintf(stderr," start=%lld",static_cast<signed long long>(s->start_time));
            else
                fprintf(stderr," start=NOPTS");
            fprintf(stderr,"\n");
        }

        AVCodecContext *ctx = avfmt_stream_codec_context(i);
        if (ctx != NULL) {
            fprintf(stderr,"Stream %zu: Codec '%s' (%s)",i,
                ctx->codec ? ctx->codec->name : NULL,
                ctx->codec ? ctx->codec->long_name : NULL);
            if (ctx->codec_type == AVMEDIA_TYPE_VIDEO)
                fprintf(stderr," type=Video");
            else if (ctx->codec_type == AVMEDIA_TYPE_AUDIO)
                fprintf(stderr," type=Audio");
            else
                fprintf(stderr," type=?");
            fprintf(stderr," time_base=%llu/%llu ticks/frame=%d delay=%d %dx%d coded=%dx%d hasb=%d\n",
                static_cast<unsigned long long>(ctx->time_base.num),static_cast<unsigned long long>(ctx->time_base.den),
                ctx->ticks_per_frame,ctx->delay,ctx->width,ctx->height,ctx->coded_width,ctx->coded_height,ctx->has_b_frames);
            fprintf(stderr," rate=%d channels=%d framesize=%d blockalign=%d",
                ctx->sample_rate,ctx->channels,ctx->frame_size,ctx->block_align);
            fprintf(stderr,"\n");
        }
    }
    bool open_stream_codec(const size_t i) {
        AVCodecContext *ctx = avfmt_stream_codec_context(i);
        if (ctx != NULL) {
            Stream &strm = stream(i);
            if (!strm.codec_open) {
                if (avcodec_open2(ctx,avcodec_find_decoder(ctx->codec_id),NULL) >= 0) {
                    fprintf(stderr,"Stream %zu codec opened\n",i);
                    print_stream_debug(i);
                    strm.codec_open = true;
                }
                else {
                    fprintf(stderr,"Stream %zu failed to open codec\n",i);
                }
            }

            return strm.codec_open;
        }

        return false;
    }
    size_t open_stream_codecs(void) {
        size_t ok = 0;

        for (size_t i=0;i < avfmt_stream_count();i++) {
            if (open_stream_codec(i))
                ok++;
        }

        return ok;
    }
    void close_stream_codec(const size_t i) {
        AVCodecContext *ctx = avfmt_stream_codec_context(i);
        if (ctx != NULL) {
            Stream &strm = stream(i);
            if (strm.codec_open) {
                fprintf(stderr,"Stream %zu closing codec\n",i);
                avcodec_close(ctx);
                strm.codec_open = false;
            }
        }
    }
    void close_stream_codecs(void) {
        if (avfmt != NULL) {
            for (size_t i=0;i < avfmt_stream_count();i++)
                close_stream_codec(i);
        }
    }
    void close_avformat(void) {
        close_stream_codecs();
        if (avfmt != NULL) {
            fprintf(stderr,"Closing avformat %s\n",file_path.c_str());
            avformat_close_input(&avfmt);
            avfmt = NULL;
        }
        streams.clear();
        eof = false;
    }
    size_t avfmt_stream_count(void) const {
        return (avfmt != NULL) ? size_t(avfmt->nb_streams) : size_t(0);
    }
    AVStream *avfmt_stream(const size_t i) {
        if (avfmt != NULL) {
            if (i < size_t(avfmt->nb_streams))
                return avfmt->streams[i];
        }

        return NULL;
    }
    AVCodecContext *avfmt_stream_codec_context(const size_t i) {
        AVStream *s = avfmt_stream(i);
        if (s != NULL) return s->codec;
        return NULL;
    }
    Stream &stream(const size_t i) {
        return streams.at(i); // throw C++ exception if out of bounds
    }
    void avpkt_reset(void) {
        avpkt_free();
        av_init_packet(&avpkt);
        avpkt_valid = true;
    }
    void avpkt_free(void) {
        if (avpkt_valid) {
            av_packet_unref(&avpkt);
            avpkt_valid = false;
        }
    }
    AVPacket *read_packet(void) {
        if (avfmt != NULL && !eof) {
            avpkt_reset();
            if (av_read_frame(avfmt,&avpkt) >= 0) {
                if (avpkt.stream_index < avfmt->nb_streams)
                    return &avpkt;
                else
                    return NULL;
            }

            eof = true;
        }

        return NULL;
    }
    bool is_eof(void) const {
        return eof;
    }
protected:
    AVPacket                avpkt;
    std::vector<Stream>     streams;
    std::string             file_path;
    AVFormatContext*        avfmt = NULL;
    bool                    open_flag = false;
    bool                    avpkt_valid = false;
    bool                    eof = false;
};

InputFile                   in_file;

InputFile &current_file(void) {
    return in_file;
}

int main(int argc,char **argv) {
    std::string open_file;

    (void)argc;
    (void)argv;

    /* FIXME: This is simple, dumb. Will do more later. */
    if (argc < 2) {
        fprintf(stderr,"Must specify file\n");
        return 1;
    }
    open_file = argv[1];

    {
        struct stat st;

        if (::stat(open_file.c_str(),&st)) {
            fprintf(stderr,"Cannot stat file, %s\n",strerror(errno));
            return 1;
        }
        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr,"%s is not a file\n",open_file.c_str());
            return 1;
        }
    }

	av_register_all();
	avformat_network_init();
	avcodec_register_all();

#if defined(WIN32)
    Windows_DPI_Awareness_Init();
#endif

    if (!in_file.open(open_file)) {
        fprintf(stderr,"Failed to open file %s\n",open_file.c_str());
        return 1;
    }

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
    UpdateDisplayRect();

    while (!quitting_app) {
        GUI_Idle();
    }

    SDL_DestroyWindow(mainWindow);
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}
