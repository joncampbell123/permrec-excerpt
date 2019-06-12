
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
#include <map>

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

#define us_time_t_INVALID_TIME (0xFFFFFFFFFFFFFFFFULL)

typedef uint64_t us_time_t;

typedef uint64_t ns_time_t;

ns_time_t monotonic_clock_ns(void) {
	struct timespec tv;

	if (clock_gettime(CLOCK_MONOTONIC,&tv))
		return us_time_t_INVALID_TIME;

	return (ns_time_t(tv.tv_sec) * 1000000000ULL) + ns_time_t(tv.tv_nsec);
}

us_time_t monotonic_clock_us(void) {
	return us_time_t(monotonic_clock_ns() / 1000ULL);
}

int                 want_audio_rate = -1;
int                 want_audio_channels = -1;

SDL_Rect            display_region = {0,0,0,0};
SDL_Rect            video_region = {0,0,0,0};

SDL_AudioSpec       audio_spec;
bool                audio_open = false;

SDL_Window*         mainWindow = NULL;
SDL_Surface*        mainSurface = NULL;
bool                quitting_app = false;

void RedrawVideoFrame(void);

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
        UpdateDisplayRect();
        RedrawVideoFrame();
    }
}

bool is_playing(void);
void do_play(void);
void do_stop(void);

bool GUI_Idle(void) {
    SDL_Event event;

    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quitting_app = true;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            GUI_OnWindowEvent(event.window);
        }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                quitting_app = true;
            }
            else if (event.key.keysym.sym == SDLK_SPACE) {
                if (is_playing())
                    do_stop();
                else
                    do_play();
            }
        }
    }

    return !(quitting_app);
}

static constexpr size_t sdl_audio_queue_size = 64 * 1024;
int16_t     sdl_audio_queue[sdl_audio_queue_size];
size_t      sdl_audio_queue_in = 0,sdl_audio_queue_out = 0;

unsigned int audio_queue_delay_samples_nolock(void);

void audio_callback(void *userdata,Uint8* stream,int len) {
    if (len < 0 || stream == NULL)
        return;

    int16_t *dst = reinterpret_cast<int16_t*>(stream);
    unsigned int samples = static_cast<unsigned int>(len / (sizeof(int16_t) * audio_spec.channels));

    if (is_playing()) {
        unsigned int do_samples = std::min(samples,audio_queue_delay_samples_nolock());
        unsigned int do_count = do_samples * audio_spec.channels;

        samples -= do_samples;
        while (do_count > 0 && sdl_audio_queue_out != sdl_audio_queue_in) {
            if (sdl_audio_queue_out == sdl_audio_queue_size)
                sdl_audio_queue_out = 0;

            assert(sdl_audio_queue_out != sdl_audio_queue_in);

            do_count--;
            *dst++ = sdl_audio_queue[sdl_audio_queue_out++];
            if (sdl_audio_queue_out == sdl_audio_queue_size)
                sdl_audio_queue_out = 0;
        }
    }

    /* zero fill the rest */
    while (samples > 0) {
        for (size_t i=0;i < audio_spec.channels;i++) *dst++ = 0;
        samples--;
    }
}

class InputFile {
public:
    class Stream {
    public:
        Stream() {
        }
        ~Stream() {
            close();
        }
        void close(void) {
            av_frame_free(&frame);
        }
        bool open(void) {
            if (frame == NULL) {
                frame = av_frame_alloc();
                if (frame == NULL) return false;
            }

            return true;
        }
    public:
        bool            codec_open = false;
        AVFrame*        frame = NULL;
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

        open_flag = true;
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
    AVPacket *read_packet(void) { // caller must not free it, because we will
        if (avfmt != NULL && !eof) {
            avpkt_reset();
            if (av_read_frame(avfmt,&avpkt) >= 0) {
                if (size_t(avpkt.stream_index) < size_t(avfmt->nb_streams))
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
    AVFrame *decode_frame(AVPacket *pkt,unsigned int &ft) {
        int got_frame = 0;
        int rd;

        ft = 0u;
        if (avfmt == NULL || pkt == NULL)
            return NULL;

        if (size_t(pkt->stream_index) >= size_t(avfmt->nb_streams))
            return NULL;

        AVStream *avs = avfmt_stream(size_t(pkt->stream_index));
        if (avs == NULL)
            return NULL;

        Stream &strm = stream(size_t(pkt->stream_index));
        if (!strm.codec_open) {
            if (!open_stream_codec(size_t(pkt->stream_index)))
                return NULL;
        }

        AVCodecContext *avc = avs->codec;
        if (avc == NULL)
            return NULL;

        if (avc->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (!strm.open()) return NULL;

            rd = avcodec_decode_video2(avc,strm.frame,&got_frame,pkt);
            if (rd < 0 || !got_frame || strm.frame->width == 0 || strm.frame->height == 0)
                return NULL;

            AVFrame *fr = av_frame_clone(strm.frame);
            if (fr == NULL)
                return NULL;

            ft = static_cast<unsigned int>(avc->codec_type);
            return fr;
        }
        else if (avc->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (!strm.open()) return NULL;

            rd = avcodec_decode_audio4(avc,strm.frame,&got_frame,pkt);
            if (rd < 0 || !got_frame || strm.frame->nb_samples == 0)
                return NULL;

            AVFrame *fr = av_frame_clone(strm.frame);

            // FIXME: Why does av_frame_clone/av_frame_copy() refuse to clone/copy packed audio?
            if (fr == NULL && !av_sample_fmt_is_planar(AVSampleFormat(strm.frame->format))) {
                AVFrame *nfr = av_frame_alloc();
                if (nfr == NULL)
                    return NULL;

                av_frame_copy_props(nfr,strm.frame);
                nfr->channel_layout = strm.frame->channel_layout;
                nfr->nb_samples = strm.frame->nb_samples;
                nfr->channels = strm.frame->channels;
                nfr->format = strm.frame->format;
                if (av_frame_get_buffer(nfr,64) < 0) {
                    fprintf(stderr,"Cannot copy frame, cannot get buffer\n");
                    av_frame_free(&nfr);
                    return NULL;
                }

                int balign = av_get_bytes_per_sample(AVSampleFormat(nfr->format));
                if (balign <= 0 || balign >= 64) {
                    fprintf(stderr,"Cannot copy frame, bad block align\n");
                    av_frame_free(&nfr);
                    return NULL;
                }

                /* OK DO IT */
                assert(strm.frame->data[0] != NULL);
                assert(nfr->data[0] != NULL);
                assert(strm.frame->nb_samples == nfr->nb_samples);
                memcpy(nfr->data[0],strm.frame->data[0],nfr->nb_samples * nfr->channels * balign);

                fr = nfr;
            }

            if (fr == NULL)
                return NULL;

            ft = static_cast<unsigned int>(avc->codec_type);
            return fr;
        }

        return NULL;
    }
    int find_default_stream(int type) {
        if (avfmt != NULL) {
            for (size_t i=0;i < avfmt_stream_count();i++) {
                AVCodecContext *ctx = avfmt_stream_codec_context(i);
                if (ctx != NULL) {
                    if (ctx->codec_type == type)
                        return int(i);
                }
            }
        }

        return -1;
    }
    int find_default_stream_audio(void) {
        return find_default_stream(AVMEDIA_TYPE_AUDIO);
    }
    int find_default_stream_video(void) {
        return find_default_stream(AVMEDIA_TYPE_VIDEO);
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
int                         in_file_video_stream = -1;
int                         in_file_audio_stream = -1;

InputFile &current_file(void) {
    return in_file;
}

us_time_t playing_base = 0;
double play_in_base = 0;
double play_in_time = 0;
bool playing = false;

double get_play_time_now(void) {
    if (playing) {
        play_in_time  = static_cast<double>(monotonic_clock_us());
        play_in_time -= static_cast<double>(playing_base);
        play_in_time /= 1000000;
        play_in_time += play_in_base;
    }

    return play_in_time;
}

bool is_playing(void) {
    return playing;
}

void do_play(void) {
    if (!playing) {
        fprintf(stderr,"Playing\n");
        playing_base = monotonic_clock_us();
        play_in_base = play_in_time;
        playing = true;
    }
}

void do_stop(void) {
    if (playing) {
        fprintf(stderr,"Stopping\n");
        get_play_time_now();
        playing_base = monotonic_clock_us();
        play_in_base = play_in_time;
        playing = false;
    }
}

struct QueueEntry {
    QueueEntry() {
    }
    QueueEntry(const QueueEntry &ent) = delete;
    QueueEntry(QueueEntry &&ent) {
        free();

        frame = ent.frame; ent.frame = NULL;
        pt = ent.pt; ent.pt = 0;
    }
    QueueEntry& operator=(QueueEntry &ent) = delete;
    QueueEntry& operator=(QueueEntry &&ent) {
        free();

        frame = ent.frame; ent.frame = NULL;
        pt = ent.pt; ent.pt = 0;
        return *this;
    }
    ~QueueEntry() {
        free();
    }
    void free(void) {
        free_frame();
    }
    void free_frame(void) {
        av_frame_free(&frame);
    }
    bool                    update = false;
    bool                    done = false;
    AVFrame*                frame = NULL;
    double                  pt = 0;
    AVRational              container_ar = {0,0};
};

struct ScalerTrack {
    int             sw = -1,sh = -1;
    int             dw = -1,dh = -1;
    AVPixelFormat   sf = AV_PIX_FMT_NONE;
    AVPixelFormat   df = AV_PIX_FMT_NONE;

    void clear(void) {
        sw = -1,sh = -1;
        dw = -1,dh = -1;
        sf = AV_PIX_FMT_NONE;
        df = AV_PIX_FMT_NONE;
    }
};

struct ResamplerTrack {
    struct group {
        int             rate = -1,channels = -1,format = -1;
        uint64_t        channel_layout = 0;
        int             alloc_samples = 0;

        void clear(void) {
            alloc_samples = 0;
            rate = -1,channels = -1;
            channel_layout = 0;
            format = -1;
        }
    };

    group       s,d;

    void clear(void) {
        s.clear();
        d.clear();
    }
};

AVFrame*                        audio_resampler_frame = NULL;
ResamplerTrack                  audio_resampler_trk;
SwrContext*                     audio_resampler = NULL;
ScalerTrack                     video_scaler_trk;
SwsContext*                     video_scaler = NULL;
QueueEntry                      current_video_frame;
std::queue<QueueEntry>          video_queue;
QueueEntry                      current_audio_frame;
std::queue<QueueEntry>          audio_queue;

void RedrawVideoFrame(void) {
    current_video_frame.update = true;
}

void flush_queue(queue<QueueEntry> &m) {
    while (!m.empty()) m.pop();
}

void flush_queue(void) {
    flush_queue(video_queue);
    flush_queue(audio_queue);
}

bool schedule_video_frame(double pt,AVFrame *fr,AVStream *avs) {
    QueueEntry ent;
    if (avs->sample_aspect_ratio.num > 0) ent.container_ar = avs->sample_aspect_ratio;
    ent.frame = fr;
    ent.pt = pt;
    video_queue.push(std::move(ent));
    fprintf(stderr,"Queued video frame pt=%.3f\n",pt);
    return true;
}

bool schedule_audio_frame(double pt,AVFrame *fr) {
    QueueEntry ent;
    ent.frame = fr;
    ent.pt = pt;
    audio_queue.push(std::move(ent));
    fprintf(stderr,"Queued audio frame pt=%.3f\n",pt);
    return true;
}

int64_t video_last_next_pts = AV_NOPTS_VALUE;
bool queue_video_frame(AVFrame *fr,AVPacket *pkt,AVStream *avs) {
    {
        double pt;
        int64_t pts = video_last_next_pts;
        if (fr->pts != AV_NOPTS_VALUE)
            pts = fr->pts;
        if (pts == AV_NOPTS_VALUE && fr->pkt_pts != AV_NOPTS_VALUE)
            pts = fr->pkt_pts;
        if (pts == AV_NOPTS_VALUE && fr->pkt_dts != AV_NOPTS_VALUE)
            pts = fr->pkt_dts;

        if (pts != AV_NOPTS_VALUE)
            pt = (double(pts) * avs->time_base.num) / avs->time_base.den;   // i.e. 1001/30000 for 29.97
        else
            return false;

        video_last_next_pts = pts + fr->pkt_duration;

        if (!schedule_video_frame(pt,fr,avs))
            return false;

        return true;
    }

    return false;
}

int64_t audio_last_next_pts = AV_NOPTS_VALUE;
bool queue_audio_frame(AVFrame *fr,AVPacket *pkt,AVStream *avs) {
    {
        double pt;
        int64_t pts = audio_last_next_pts;
        if (fr->pts != AV_NOPTS_VALUE)
            pts = fr->pts;
        if (pts == AV_NOPTS_VALUE && fr->pkt_dts != AV_NOPTS_VALUE)
            pts = fr->pkt_dts;

        if (pts != AV_NOPTS_VALUE)
            pt = (double(pts) * avs->time_base.num) / avs->time_base.den;
        else
            return false;

        audio_last_next_pts = pts + fr->pkt_duration;

        if (!schedule_audio_frame(pt,fr))
            return false;

        return true;
    }

    return false;
}

void free_audio_resampler(void) {
    av_frame_free(&audio_resampler_frame);
    audio_resampler_trk.clear();
    swr_free(&audio_resampler);
    audio_resampler = NULL;
}

void free_video_scaler(void) {
    video_scaler_trk.clear();
    sws_freeContext(video_scaler);
    video_scaler = NULL;
}

unsigned int sdl_audio_queue_write(const int16_t *audio,unsigned int samples);

void send_audio_frame(QueueEntry &frame) {
    if (frame.frame == NULL)
        return;
    if (frame.frame->sample_rate == 0 || frame.frame->channels == 0 || frame.frame->format < 0)
        return;
    if (frame.frame->nb_samples == 0)
        return;

    if (audio_resampler != NULL) {
        if (audio_resampler_trk.s.rate != frame.frame->sample_rate ||
            audio_resampler_trk.s.channels != frame.frame->channels ||
            audio_resampler_trk.s.format != frame.frame->format ||
            audio_resampler_trk.s.channel_layout != frame.frame->channel_layout ||
            audio_resampler_trk.s.alloc_samples < frame.frame->nb_samples ||
            audio_resampler_trk.d.rate != audio_spec.freq ||
            audio_resampler_trk.d.channels != audio_spec.channels ||
            audio_resampler_trk.d.format != AV_SAMPLE_FMT_S16 ||
            audio_resampler_trk.d.channel_layout != 0) {
            fprintf(stderr,"Audio format changed, freeing resampler\n");
            free_audio_resampler();
        }
    }
    if (audio_resampler == NULL) {
        audio_resampler = swr_alloc();
        if (audio_resampler == NULL) return;

        audio_resampler_trk.s.rate = frame.frame->sample_rate;
        audio_resampler_trk.s.channels = frame.frame->channels;
        audio_resampler_trk.s.format = frame.frame->format;
        audio_resampler_trk.s.channel_layout = frame.frame->channel_layout;
        audio_resampler_trk.s.alloc_samples = frame.frame->nb_samples;
        audio_resampler_trk.d.rate = audio_spec.freq;
        audio_resampler_trk.d.channels = audio_spec.channels;
        audio_resampler_trk.d.format = AV_SAMPLE_FMT_S16;
        audio_resampler_trk.d.channel_layout = 0;

        av_opt_set_int(audio_resampler,         "in_channel_count",     audio_resampler_trk.s.channels, 0); // FIXME: FFMPEG should document this!!
        av_opt_set_int(audio_resampler,         "out_channel_count",    audio_resampler_trk.d.channels, 0); // FIXME: FFMPEG should document this!!
        av_opt_set_int(audio_resampler,         "in_channel_layout",    int64_t(audio_resampler_trk.s.channel_layout), 0);
        av_opt_set_int(audio_resampler,         "out_channel_layout",   int64_t(audio_resampler_trk.d.channel_layout), 0);
        av_opt_set_int(audio_resampler,         "in_sample_rate",       audio_resampler_trk.s.rate, 0);
        av_opt_set_int(audio_resampler,         "out_sample_rate",      audio_resampler_trk.d.rate, 0);
        av_opt_set_sample_fmt(audio_resampler,  "in_sample_fmt",        AVSampleFormat(audio_resampler_trk.s.format), 0);
        av_opt_set_sample_fmt(audio_resampler,  "out_sample_fmt",       AVSampleFormat(audio_resampler_trk.d.format), 0);

        if (swr_init(audio_resampler) < 0) {
            fprintf(stderr,"Unable to init audio resampler\n");
            return;
        }

        audio_resampler_trk.d.alloc_samples = static_cast<int>(av_rescale_rnd(
            swr_get_delay(audio_resampler,frame.frame->sample_rate) + frame.frame->nb_samples,
            audio_spec.freq, frame.frame->sample_rate, AV_ROUND_UP));

        assert(audio_resampler_frame == NULL);
        audio_resampler_frame = av_frame_alloc();
        if (audio_resampler_frame == NULL) {
            fprintf(stderr,"Unable to alloc audio resampler frame\n");
            free_audio_resampler();
            return;
        }
        audio_resampler_frame->nb_samples = audio_resampler_trk.d.alloc_samples;
        audio_resampler_frame->channels = audio_resampler_trk.d.channels;
        audio_resampler_frame->sample_rate = audio_resampler_trk.d.rate;
        audio_resampler_frame->format = audio_resampler_trk.d.format;
        audio_resampler_frame->channel_layout = audio_resampler_trk.d.channel_layout;
        if (av_frame_get_buffer(audio_resampler_frame,64) < 0) {
            fprintf(stderr,"Unable to alloc buffer for audio resampler frame\n");
            free_audio_resampler();
            return;
        }

        fprintf(stderr,"Audio resampler init out samples %d\n",audio_resampler_trk.d.alloc_samples);
    }
    if (audio_resampler != NULL) {
        int out_samp;

        out_samp = swr_convert(audio_resampler,
            /* out */
            audio_resampler_frame->data,
            audio_resampler_frame->nb_samples,
            /* in */
            const_cast<const uint8_t**>(frame.frame->data),
            frame.frame->nb_samples);

        if (out_samp >= 0) {
            assert(audio_resampler_frame->data[0] != NULL);
            if (sdl_audio_queue_write(reinterpret_cast<int16_t*>(audio_resampler_frame->data[0]),static_cast<unsigned int>(out_samp)) != static_cast<unsigned int>(out_samp))
                fprintf(stderr,"Not all samples written\n");
        }
        else {
            fprintf(stderr,"Resampler failed\n");
        }
    }
}

void draw_video_frame(QueueEntry &frame) {
    if (frame.frame == NULL)
        return;
    if (frame.frame->width <= 0 || frame.frame->height <= 0)
        return;
    if (display_region.w <= 0 || display_region.h <= 0)
        return;

    /* compute the frame */
    double sw = frame.frame->width;
    double sh = frame.frame->height;

    if (frame.container_ar.num > 0 && frame.container_ar.den > 0) {
        double ar = double(frame.container_ar.num) / frame.container_ar.den;
        sw *= ar;
    }
    else if (frame.frame->sample_aspect_ratio.num > 0 && frame.frame->sample_aspect_ratio.den > 0) {
        double ar = double(frame.frame->sample_aspect_ratio.num) / frame.frame->sample_aspect_ratio.den;
        sw *= ar;
    }

    /* fit the frame */
    double dh = display_region.h;
    double dw = (dh * sw) / sh;

    if (dw > display_region.w) {
        dh = (dh * display_region.w) / dw;
        dw = display_region.w;
    }

    int ifw = int(floor(dw + 0.5));
    int ifh = int(floor(dh + 0.5));
    if (ifw < 4) return;
    if (ifh < 4) return;

    video_region.w = ifw;
    video_region.h = ifh;
    video_region.x = (display_region.w - ifw) / 2;
    video_region.y = (display_region.h - ifh) / 2;
    assert(video_region.x >= 0);
    assert(video_region.y >= 0);
    assert((video_region.x+video_region.w) <= display_region.w);
    assert((video_region.y+video_region.h) <= display_region.h);

    if (video_scaler != NULL) {
        if (video_scaler_trk.sw != frame.frame->width ||
            video_scaler_trk.sh != frame.frame->height ||
            video_scaler_trk.sf != AVPixelFormat(frame.frame->format) ||
            video_scaler_trk.dw != video_region.w ||
            video_scaler_trk.dh != video_region.h ||
            video_scaler_trk.df != AVPixelFormat(AV_PIX_FMT_BGRA)) {
            fprintf(stderr,"Scaler change\n");
            free_video_scaler();
        }
    }

    if (video_scaler == NULL) {
        video_scaler = sws_getContext(
            // source
            frame.frame->width,
            frame.frame->height,
            AVPixelFormat(frame.frame->format),
            // dest
            video_region.w,
            video_region.h,
            AVPixelFormat(AV_PIX_FMT_BGRA),
            // opt
            SWS_BILINEAR, NULL, NULL, NULL);
        if (video_scaler == NULL) {
            fprintf(stderr,"Failed to create scaler\n");
            return;
        }
        video_scaler_trk.sw = frame.frame->width;
        video_scaler_trk.sh = frame.frame->height;
        video_scaler_trk.sf = AVPixelFormat(frame.frame->format);
        video_scaler_trk.dw = video_region.w;
        video_scaler_trk.dh = video_region.h;
        video_scaler_trk.df = AVPixelFormat(AV_PIX_FMT_BGRA);
        fprintf(stderr,"Scaler init\n");
    }

    if (video_scaler != NULL) {
        SDL_LockSurface(mainSurface);

        if (mainSurface->pixels != NULL) {
            uint8_t *dstptr[4] = {NULL,NULL,NULL,NULL};
            int dstlinesize[4] = {0,0,0,0};

            dstlinesize[0] = mainSurface->pitch;
            dstptr[0] = reinterpret_cast<uint8_t*>(mainSurface->pixels) + (4 * video_region.x) + (mainSurface->pitch * video_region.y);

            if (sws_scale(video_scaler,
                        // source
                        frame.frame->data,
                        frame.frame->linesize,
                        0,
                        frame.frame->height,
                        // dest
                        dstptr,
                        dstlinesize) <= 0) {
                fprintf(stderr,"scaler error\n");
            }
        }

        SDL_UnlockSurface(mainSurface);
        SDL_UpdateWindowSurface(mainWindow);
    }
}

unsigned int audio_queue_delay_samples_nolock(void) {
    ssize_t amt;

    amt = static_cast<ssize_t>(sdl_audio_queue_in);
    amt -= static_cast<ssize_t>(sdl_audio_queue_out);
    if (amt < 0) amt += static_cast<ssize_t>(sdl_audio_queue_size);
    if (amt < 0) amt = 0;
    if (amt > static_cast<ssize_t>(sdl_audio_queue_size)) amt = static_cast<ssize_t>(sdl_audio_queue_size);

    return static_cast<unsigned int>(amt / audio_spec.channels);
}

unsigned int sdl_audio_queue_write(const int16_t *audio,unsigned int samples) {
    unsigned int full,avail;

    SDL_LockAudio();
    full = audio_queue_delay_samples_nolock();
    avail = static_cast<unsigned int>(sdl_audio_queue_size / audio_spec.channels);
    if (avail < full) avail = full; // avoid negative numbers in following line
    avail -= full;
    if (avail != 0) avail--;

    if (samples > avail) samples = avail;

    if (samples != 0) {
        unsigned int count = static_cast<unsigned int>(samples) * audio_spec.channels;

        do {
            if (sdl_audio_queue_in == sdl_audio_queue_size)
                sdl_audio_queue_in = 0;

            sdl_audio_queue[sdl_audio_queue_in++] = *audio++;
            if (sdl_audio_queue_in == sdl_audio_queue_size)
                sdl_audio_queue_in = 0;

            assert(sdl_audio_queue_in != sdl_audio_queue_out);
        } while (--count != 0);
    }

    SDL_UnlockAudio();

    return samples;
}

unsigned int audio_queue_delay_samples(void) {
    unsigned int r;

    SDL_LockAudio();
    r = audio_queue_delay_samples_nolock();
    SDL_UnlockAudio();
    return r;
}

double audio_queue_block_delay(void) {
    return double(audio_spec.samples) / (audio_spec.freq * 2); // half a block
}

double audio_queue_delay(void) {
    return double(audio_queue_delay_samples()) / audio_spec.freq;
}

void Play_Idle(void) {
    unsigned int ft;
    AVFrame *fr = NULL;
    bool notfull = true;
    auto &fp = current_file();

    if (is_playing()) {
        if (video_queue.size() >= 64 || audio_queue.size() >= 256)
            notfull = false;

        if (fp.is_open()) {
            if (notfull) {
                AVPacket *pkt = in_file.read_packet(); // no need to free, invalidated at next call
                if (pkt != NULL) {
                    if (pkt->stream_index == in_file_video_stream) {
                        fr = in_file.decode_frame(pkt,/*&*/ft);
                        if (fr != NULL) {
                            if (!queue_video_frame(fr,pkt,in_file.avfmt_stream(size_t(pkt->stream_index)))) {
                                av_frame_free(&fr);
                            }
                        }
                    }
                    else if (pkt->stream_index == in_file_audio_stream) {
                        fr = in_file.decode_frame(pkt,/*&*/ft);
                        if (fr != NULL) {
                            if (!queue_audio_frame(fr,pkt,in_file.avfmt_stream(size_t(pkt->stream_index)))) {
                                av_frame_free(&fr);
                            }
                        }
                    }
                }
                else if (in_file.is_eof()) {
                    if (video_queue.empty() && audio_queue.empty())
                        do_stop();
                }
            }
        }
        else {
            do_stop();
        }

        get_play_time_now();
        if (!video_queue.empty()) {
            auto &ent = video_queue.front();
            if (play_in_time >= ent.pt) {
                current_video_frame = std::move(ent);
                video_queue.pop();
                current_video_frame.update = true;
            }
        }

        if (!audio_queue.empty()) {
            auto &ent = audio_queue.front();
            if (audio_queue_delay() < 0.5 && play_in_time >= (ent.pt - audio_queue_delay())) {
                current_audio_frame = std::move(ent);
                audio_queue.pop();
                current_audio_frame.update = true;
            }
        }
    }

    if (current_video_frame.update && current_video_frame.frame != NULL) {
        draw_video_frame(current_video_frame);
        current_video_frame.update = false;
    }

    if (current_audio_frame.update && current_audio_frame.frame != NULL) {
        send_audio_frame(current_audio_frame);
        current_audio_frame.update = false;
    }
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

    in_file_audio_stream = in_file.find_default_stream_audio();
    in_file_video_stream = in_file.find_default_stream_video();
    fprintf(stderr,"Chose audio stream %d, video stream %d\n",in_file_audio_stream,in_file_video_stream);

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

    do_play();
    while (!quitting_app) {
        Play_Idle();
        GUI_Idle();
    }
    do_stop();
    flush_queue();
    free_audio_resampler();
    free_video_scaler();

    SDL_DestroyWindow(mainWindow);
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}
