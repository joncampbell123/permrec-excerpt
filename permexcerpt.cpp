
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


/**
* @file
* @brief encoded font 14x256
**/

unsigned char vga_8x14_font[14*256] = {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x7e,0x81,0xa5,0x81,0x81,0xbd,0x99,0x81,0x7e,0x00,0x00,
	0x00,0x00,0x00,0x7e,0xff,0xdb,0xff,0xff,0xc3,0xe7,0xff,0x7e,0x00,0x00,
	0x00,0x00,0x00,0x00,0x6c,0xfe,0xfe,0xfe,0xfe,0x7c,0x38,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x10,0x38,0x7c,0xfe,0x7c,0x38,0x10,0x00,0x00,0x00,
	0x00,0x00,0x00,0x18,0x3c,0x3c,0xe7,0xe7,0xe7,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x00,0x18,0x3c,0x7e,0xff,0xff,0x7e,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x3c,0x3c,0x18,0x00,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xff,0xe7,0xc3,0xc3,0xe7,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x3c,0x66,0x42,0x42,0x66,0x3c,0x00,0x00,0x00,
	0xff,0xff,0xff,0xff,0xff,0xc3,0x99,0xbd,0xbd,0x99,0xc3,0xff,0xff,0xff,
	0x00,0x00,0x00,0x1e,0x0e,0x1a,0x32,0x78,0xcc,0xcc,0xcc,0x78,0x00,0x00,
	0x00,0x00,0x00,0x3c,0x66,0x66,0x66,0x3c,0x18,0x7e,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x3f,0x33,0x3f,0x30,0x30,0x30,0x70,0xf0,0xe0,0x00,0x00,
	0x00,0x00,0x00,0x7f,0x63,0x7f,0x63,0x63,0x63,0x67,0xe7,0xe6,0xc0,0x00,
	0x00,0x00,0x00,0x18,0x18,0xdb,0x3c,0xe7,0x3c,0xdb,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x80,0xc0,0xe0,0xf8,0xfe,0xf8,0xe0,0xc0,0x80,0x00,0x00,
	0x00,0x00,0x00,0x02,0x06,0x0e,0x3e,0xfe,0x3e,0x0e,0x06,0x02,0x00,0x00,
	0x00,0x00,0x00,0x18,0x3c,0x7e,0x18,0x18,0x18,0x7e,0x3c,0x18,0x00,0x00,
	0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x00,0x66,0x66,0x00,0x00,
	0x00,0x00,0x00,0x7f,0xdb,0xdb,0xdb,0x7b,0x1b,0x1b,0x1b,0x1b,0x00,0x00,
	0x00,0x00,0x7c,0xc6,0x60,0x38,0x6c,0xc6,0xc6,0x6c,0x38,0x0c,0xc6,0x7c,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xfe,0xfe,0x00,0x00,
	0x00,0x00,0x00,0x18,0x3c,0x7e,0x18,0x18,0x18,0x7e,0x3c,0x18,0x7e,0x00,
	0x00,0x00,0x00,0x18,0x3c,0x7e,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x7e,0x3c,0x18,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x18,0x0c,0xfe,0x0c,0x18,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x30,0x60,0xfe,0x60,0x30,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0xc0,0xc0,0xfe,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x28,0x6c,0xfe,0x6c,0x28,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x10,0x38,0x38,0x7c,0x7c,0xfe,0xfe,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xfe,0xfe,0x7c,0x7c,0x38,0x38,0x10,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x18,0x3c,0x3c,0x3c,0x18,0x18,0x00,0x18,0x18,0x00,0x00,
	0x00,0x66,0x66,0x66,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x6c,0x6c,0xfe,0x6c,0x6c,0x6c,0xfe,0x6c,0x6c,0x00,0x00,
	0x00,0x18,0x18,0x7c,0xc6,0xc2,0xc0,0x7c,0x06,0x86,0xc6,0x7c,0x18,0x18,
	0x00,0x00,0x00,0x00,0x00,0xc2,0xc6,0x0c,0x18,0x30,0x66,0xc6,0x00,0x00,
	0x00,0x00,0x00,0x38,0x6c,0x6c,0x38,0x76,0xdc,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x18,0x18,0x18,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x0c,0x18,0x30,0x30,0x30,0x30,0x30,0x18,0x0c,0x00,0x00,
	0x00,0x00,0x00,0x30,0x18,0x0c,0x0c,0x0c,0x0c,0x0c,0x18,0x30,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x66,0x3c,0xff,0x3c,0x66,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x7e,0x18,0x18,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x18,0x30,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x02,0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x00,0x00,0x00,
	0x00,0x00,0x00,0x38,0x6c,0xc6,0xc6,0xd6,0xc6,0xc6,0x6c,0x38,0x00,0x00,
	0x00,0x00,0x00,0x18,0x38,0x78,0x18,0x18,0x18,0x18,0x18,0x7e,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0x06,0x0c,0x18,0x30,0x60,0xc6,0xfe,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0x06,0x06,0x3c,0x06,0x06,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x0c,0x1c,0x3c,0x6c,0xcc,0xfe,0x0c,0x0c,0x1e,0x00,0x00,
	0x00,0x00,0x00,0xfe,0xc0,0xc0,0xc0,0xfc,0x06,0x06,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x38,0x60,0xc0,0xc0,0xfc,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0xfe,0xc6,0x06,0x0c,0x18,0x30,0x30,0x30,0x30,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0x7c,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0x7e,0x06,0x06,0x0c,0x78,0x00,0x00,
	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x18,0x18,0x30,0x00,0x00,
	0x00,0x00,0x00,0x0c,0x18,0x30,0x60,0xc0,0x60,0x30,0x18,0x0c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0x7e,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x60,0x30,0x18,0x0c,0x06,0x0c,0x18,0x30,0x60,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0xc6,0x0c,0x18,0x18,0x00,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0xc6,0xde,0xde,0xde,0xdc,0xc0,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x10,0x38,0x6c,0xc6,0xc6,0xfe,0xc6,0xc6,0xc6,0x00,0x00,
	0x00,0x00,0x00,0xfc,0x66,0x66,0x66,0x7c,0x66,0x66,0x66,0xfc,0x00,0x00,
	0x00,0x00,0x00,0x3c,0x66,0xc2,0xc0,0xc0,0xc0,0xc2,0x66,0x3c,0x00,0x00,
	0x00,0x00,0x00,0xf8,0x6c,0x66,0x66,0x66,0x66,0x66,0x6c,0xf8,0x00,0x00,
	0x00,0x00,0x00,0xfe,0x66,0x62,0x68,0x78,0x68,0x62,0x66,0xfe,0x00,0x00,
	0x00,0x00,0x00,0xfe,0x66,0x62,0x68,0x78,0x68,0x60,0x60,0xf0,0x00,0x00,
	0x00,0x00,0x00,0x3c,0x66,0xc2,0xc0,0xc0,0xde,0xc6,0x66,0x3a,0x00,0x00,
	0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xfe,0xc6,0xc6,0xc6,0xc6,0x00,0x00,
	0x00,0x00,0x00,0x3c,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x00,0x1e,0x0c,0x0c,0x0c,0x0c,0x0c,0xcc,0xcc,0x78,0x00,0x00,
	0x00,0x00,0x00,0xe6,0x66,0x6c,0x6c,0x78,0x6c,0x6c,0x66,0xe6,0x00,0x00,
	0x00,0x00,0x00,0xf0,0x60,0x60,0x60,0x60,0x60,0x62,0x66,0xfe,0x00,0x00,
	0x00,0x00,0x00,0xc6,0xee,0xfe,0xd6,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,
	0x00,0x00,0x00,0xc6,0xe6,0xf6,0xfe,0xde,0xce,0xc6,0xc6,0xc6,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0xfc,0x66,0x66,0x66,0x7c,0x60,0x60,0x60,0xf0,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0xd6,0xde,0x7c,0x0e,0x00,
	0x00,0x00,0x00,0xfc,0x66,0x66,0x66,0x7c,0x6c,0x66,0x66,0xe6,0x00,0x00,
	0x00,0x00,0x00,0x7c,0xc6,0xc6,0x60,0x38,0x0c,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x7e,0x7e,0x5a,0x18,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x6c,0x38,0x10,0x00,0x00,
	0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0xd6,0xd6,0xfe,0x6c,0x6c,0x00,0x00,
	0x00,0x00,0x00,0xc6,0xc6,0xc6,0x7c,0x38,0x7c,0xc6,0xc6,0xc6,0x00,0x00,
	0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x3c,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x00,0xfe,0xc6,0x8c,0x18,0x30,0x60,0xc2,0xc6,0xfe,0x00,0x00,
	0x00,0x00,0x00,0x3c,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x3c,0x00,0x00,
	0x00,0x00,0x00,0x80,0xc0,0xe0,0x70,0x38,0x1c,0x0e,0x06,0x02,0x00,0x00,
	0x00,0x00,0x00,0x3c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x3c,0x00,0x00,
	0x10,0x38,0x6c,0xc6,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0x00,0x30,0x18,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x0c,0x7c,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x00,0xe0,0x60,0x60,0x78,0x6c,0x66,0x66,0x66,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0xc0,0xc0,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x1c,0x0c,0x0c,0x3c,0x6c,0xcc,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0xfe,0xc0,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x1c,0x36,0x32,0x30,0x7c,0x30,0x30,0x30,0x78,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x76,0xcc,0xcc,0xcc,0x7c,0x0c,0xcc,0x78,
	0x00,0x00,0x00,0xe0,0x60,0x60,0x6c,0x76,0x66,0x66,0x66,0xe6,0x00,0x00,
	0x00,0x00,0x00,0x18,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x00,0x06,0x06,0x00,0x0e,0x06,0x06,0x06,0x06,0x66,0x66,0x3c,
	0x00,0x00,0x00,0xe0,0x60,0x60,0x66,0x6c,0x78,0x6c,0x66,0xe6,0x00,0x00,
	0x00,0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xec,0xfe,0xd6,0xd6,0xd6,0xd6,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xdc,0x66,0x66,0x66,0x66,0x66,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xdc,0x66,0x66,0x66,0x7c,0x60,0x60,0xf0,
	0x00,0x00,0x00,0x00,0x00,0x00,0x76,0xcc,0xcc,0xcc,0x7c,0x0c,0x0c,0x1e,
	0x00,0x00,0x00,0x00,0x00,0x00,0xdc,0x76,0x66,0x60,0x60,0xf0,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0x70,0x1c,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x10,0x30,0x30,0xfc,0x30,0x30,0x30,0x36,0x1c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xcc,0xcc,0xcc,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0xc6,0x6c,0x38,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0xd6,0xd6,0xfe,0x6c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0x6c,0x38,0x38,0x6c,0xc6,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0x0c,0x78,
	0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xcc,0x18,0x30,0x66,0xfe,0x00,0x00,
	0x00,0x00,0x00,0x0e,0x18,0x18,0x18,0x70,0x18,0x18,0x18,0x0e,0x00,0x00,
	0x00,0x00,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x70,0x18,0x18,0x18,0x0e,0x18,0x18,0x18,0x70,0x00,0x00,
	0x00,0x76,0xdc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x10,0x38,0x6c,0xc6,0xc6,0xfe,0x00,0x00,0x00,
	0x00,0x00,0x00,0x3c,0x66,0xc2,0xc0,0xc0,0xc0,0xc2,0x66,0x3c,0x0c,0x78,
	0x00,0x00,0x00,0xcc,0x00,0x00,0xcc,0xcc,0xcc,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x0c,0x18,0x30,0x00,0x7c,0xc6,0xfe,0xc0,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x10,0x38,0x6c,0x00,0x78,0x0c,0x7c,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x00,0xc6,0x00,0x00,0x78,0x0c,0x7c,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x60,0x30,0x18,0x00,0x78,0x0c,0x7c,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x38,0x6c,0x38,0x00,0x78,0x0c,0x7c,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0xc6,0xc0,0xc0,0xc6,0x7c,0x0c,0x78,
	0x00,0x00,0x10,0x38,0x6c,0x00,0x7c,0xc6,0xfe,0xc0,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0xc6,0x00,0x00,0x7c,0xc6,0xfe,0xc0,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x60,0x30,0x18,0x00,0x7c,0xc6,0xfe,0xc0,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x66,0x00,0x00,0x38,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x18,0x3c,0x66,0x00,0x38,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x60,0x30,0x18,0x00,0x38,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0xc6,0x00,0x10,0x38,0x6c,0xc6,0xc6,0xfe,0xc6,0xc6,0xc6,0x00,0x00,
	0x38,0x6c,0x38,0x10,0x38,0x6c,0xc6,0xc6,0xfe,0xc6,0xc6,0xc6,0x00,0x00,
	0x0c,0x18,0x00,0xfe,0x66,0x62,0x68,0x78,0x68,0x62,0x66,0xfe,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xec,0x36,0x76,0xdc,0xd8,0x6e,0x00,0x00,
	0x00,0x00,0x00,0x3e,0x6c,0xcc,0xcc,0xfe,0xcc,0xcc,0xcc,0xce,0x00,0x00,
	0x00,0x00,0x10,0x38,0x6c,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0xc6,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x60,0x30,0x18,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x30,0x78,0xcc,0x00,0xcc,0xcc,0xcc,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x60,0x30,0x18,0x00,0xcc,0xcc,0xcc,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x00,0xc6,0x00,0x00,0xc6,0xc6,0xc6,0xc6,0x7e,0x06,0x0c,0x78,
	0x00,0xc6,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0xc6,0x00,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x18,0x18,0x7c,0xc6,0xc0,0xc0,0xc6,0x7c,0x18,0x18,0x00,0x00,
	0x00,0x00,0x38,0x6c,0x64,0x60,0xf0,0x60,0x60,0x60,0xe6,0xfc,0x00,0x00,
	0x00,0x00,0x00,0x66,0x66,0x3c,0x18,0x7e,0x18,0x7e,0x18,0x18,0x00,0x00,
	0x00,0x00,0xfc,0x66,0x66,0x7c,0x62,0x66,0x6f,0x66,0x66,0xf3,0x00,0x00,
	0x00,0x00,0x0e,0x1b,0x18,0x18,0x18,0x7e,0x18,0x18,0x18,0xd8,0x70,0x00,
	0x00,0x00,0x0c,0x18,0x30,0x00,0x78,0x0c,0x7c,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x0c,0x18,0x30,0x00,0x38,0x18,0x18,0x18,0x18,0x3c,0x00,0x00,
	0x00,0x00,0x0c,0x18,0x30,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x0c,0x18,0x30,0x00,0xcc,0xcc,0xcc,0xcc,0xcc,0x76,0x00,0x00,
	0x00,0x00,0x00,0x76,0xdc,0x00,0xdc,0x66,0x66,0x66,0x66,0x66,0x00,0x00,
	0x76,0xdc,0x00,0xc6,0xe6,0xf6,0xfe,0xde,0xce,0xc6,0xc6,0xc6,0x00,0x00,
	0x00,0x00,0x3c,0x6c,0x6c,0x3e,0x00,0x7e,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x38,0x6c,0x6c,0x38,0x00,0x7c,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x60,0xc6,0xc6,0x7c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xc0,0xc0,0xc0,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x06,0x06,0x06,0x00,0x00,0x00,
	0x00,0x00,0x60,0xe0,0x63,0x66,0x6c,0x18,0x30,0x6e,0xc3,0x06,0x0c,0x1f,
	0x00,0x00,0x60,0xe0,0x63,0x66,0x6c,0x1a,0x36,0x6e,0xda,0x3f,0x06,0x06,
	0x00,0x00,0x00,0x18,0x18,0x00,0x18,0x18,0x3c,0x3c,0x3c,0x18,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x36,0x6c,0xd8,0x6c,0x36,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xd8,0x6c,0x36,0x6c,0xd8,0x00,0x00,0x00,0x00,
	0x11,0x44,0x11,0x44,0x11,0x44,0x11,0x44,0x11,0x44,0x11,0x44,0x11,0x44,
	0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,
	0xdd,0x77,0xdd,0x77,0xdd,0x77,0xdd,0x77,0xdd,0x77,0xdd,0x77,0xdd,0x77,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xf8,0x18,0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,0xf8,0x18,0xf8,0x18,0x18,0x18,0x18,0x18,0x18,
	0x36,0x36,0x36,0x36,0x36,0x36,0x36,0xf6,0x36,0x36,0x36,0x36,0x36,0x36,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x36,0x36,0x36,0x36,0x36,0x36,
	0x00,0x00,0x00,0x00,0x00,0xf8,0x18,0xf8,0x18,0x18,0x18,0x18,0x18,0x18,
	0x36,0x36,0x36,0x36,0x36,0xf6,0x06,0xf6,0x36,0x36,0x36,0x36,0x36,0x36,
	0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
	0x00,0x00,0x00,0x00,0x00,0xfe,0x06,0xf6,0x36,0x36,0x36,0x36,0x36,0x36,
	0x36,0x36,0x36,0x36,0x36,0xf6,0x06,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,
	0x36,0x36,0x36,0x36,0x36,0x36,0x36,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,
	0x18,0x18,0x18,0x18,0x18,0xf8,0x18,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf8,0x18,0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x18,0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x1f,0x18,0x18,0x18,0x18,0x18,0x18,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xff,0x18,0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,0x1f,0x18,0x1f,0x18,0x18,0x18,0x18,0x18,0x18,
	0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x37,0x36,0x36,0x36,0x36,0x36,0x36,
	0x36,0x36,0x36,0x36,0x36,0x37,0x30,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x3f,0x30,0x37,0x36,0x36,0x36,0x36,0x36,0x36,
	0x36,0x36,0x36,0x36,0x36,0xf7,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xf7,0x36,0x36,0x36,0x36,0x36,0x36,
	0x36,0x36,0x36,0x36,0x36,0x37,0x30,0x37,0x36,0x36,0x36,0x36,0x36,0x36,
	0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
	0x36,0x36,0x36,0x36,0x36,0xf7,0x00,0xf7,0x36,0x36,0x36,0x36,0x36,0x36,
	0x18,0x18,0x18,0x18,0x18,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
	0x36,0x36,0x36,0x36,0x36,0x36,0x36,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xff,0x18,0x18,0x18,0x18,0x18,0x18,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x36,0x36,0x36,0x36,0x36,0x36,
	0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,
	0x18,0x18,0x18,0x18,0x18,0x1f,0x18,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x1f,0x18,0x1f,0x18,0x18,0x18,0x18,0x18,0x18,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0x36,0x36,0x36,0x36,0x36,0x36,
	0x36,0x36,0x36,0x36,0x36,0x36,0x36,0xff,0x36,0x36,0x36,0x36,0x36,0x36,
	0x18,0x18,0x18,0x18,0x18,0xff,0x18,0xff,0x18,0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xf8,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x18,0x18,0x18,0x18,0x18,0x18,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
	0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x76,0xdc,0xd8,0xd8,0xdc,0x76,0x00,0x00,
	0x00,0x00,0x00,0x78,0xcc,0xcc,0xcc,0xd8,0xcc,0xc6,0xc6,0xcc,0x00,0x00,
	0x00,0x00,0x00,0xfe,0xc6,0xc6,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0x6c,0x6c,0x6c,0x6c,0x6c,0x00,0x00,
	0x00,0x00,0x00,0xfe,0xc6,0x60,0x30,0x18,0x30,0x60,0xc6,0xfe,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0xd8,0xd8,0xd8,0xd8,0x70,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x66,0x66,0x66,0x66,0x66,0x7c,0x60,0xc0,
	0x00,0x00,0x00,0x00,0x00,0x76,0xdc,0x18,0x18,0x18,0x18,0x18,0x00,0x00,
	0x00,0x00,0x00,0x7e,0x18,0x3c,0x66,0x66,0x66,0x3c,0x18,0x7e,0x00,0x00,
	0x00,0x00,0x00,0x38,0x6c,0xc6,0xc6,0xfe,0xc6,0xc6,0x6c,0x38,0x00,0x00,
	0x00,0x00,0x00,0x38,0x6c,0xc6,0xc6,0xc6,0x6c,0x6c,0x6c,0xee,0x00,0x00,
	0x00,0x00,0x00,0x1e,0x30,0x18,0x0c,0x3e,0x66,0x66,0x66,0x3c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0xdb,0xdb,0x7e,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x03,0x06,0x7e,0xdb,0xdb,0xf3,0x7e,0x60,0xc0,0x00,0x00,
	0x00,0x00,0x00,0x1e,0x30,0x60,0x60,0x7e,0x60,0x60,0x30,0x1e,0x00,0x00,
	0x00,0x00,0x00,0x00,0x7c,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0xc6,0x00,0x00,
	0x00,0x00,0x00,0x00,0xfe,0x00,0x00,0xfe,0x00,0x00,0xfe,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x18,0x18,0x7e,0x18,0x18,0x00,0x00,0x7e,0x00,0x00,
	0x00,0x00,0x00,0x30,0x18,0x0c,0x06,0x0c,0x18,0x30,0x00,0x7e,0x00,0x00,
	0x00,0x00,0x00,0x0c,0x18,0x30,0x60,0x30,0x18,0x0c,0x00,0x7e,0x00,0x00,
	0x00,0x00,0x0e,0x1b,0x1b,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0xd8,0xd8,0x70,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x7e,0x00,0x18,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x76,0xdc,0x00,0x76,0xdc,0x00,0x00,0x00,0x00,
	0x00,0x38,0x6c,0x6c,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x0f,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0xec,0x6c,0x3c,0x1c,0x00,0x00,
	0x00,0x6c,0x36,0x36,0x36,0x36,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x3c,0x66,0x0c,0x18,0x32,0x7e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x7e,0x7e,0x7e,0x7e,0x7e,0x7e,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

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

us_time_t playing_base = 0;
double play_duration = 0;
double play_in_base = 0;
double play_in_time = 0;
bool playing = false;

double                      in_point = -1;
double                      out_point = -1;

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

SDL_Rect            playpos_bar = {0,0,0,0};
SDL_Rect            playpos_time = {0,0,0,0};
SDL_Rect            playpos_thumb = {0,0,0,0};
SDL_Rect            playpos_region = {0,0,0,0};
SDL_Rect            display_region = {0,0,0,0};
SDL_Rect            video_region = {0,0,0,0};

SDL_AudioSpec       audio_spec;
bool                audio_open = false;

SDL_Window*         mainWindow = NULL;
SDL_Surface*        mainSurface = NULL;
bool                quitting_app = false;

bool                gui_redraw = true;

double              gui_redraw_at_play_time = -1;

void RedrawVideoFrame(void);

void PlayposBarRecomputeThumb(void) {
    playpos_thumb = {0,0,0,0};
    playpos_time = {0,0,0,0};

    if (playpos_bar.w > 0) {
        int pos_x = int((play_in_time * playpos_bar.w) / std::max(play_duration,0.1));
        if (pos_x < 0) pos_x = 0;
        if (pos_x > playpos_bar.w) pos_x = playpos_bar.w;

        playpos_time.x = 8;
        playpos_time.w = playpos_bar.x - playpos_time.x;
        playpos_time.h = 14;
        playpos_time.y = ((playpos_bar.h - playpos_time.h) / 2) + playpos_bar.y;

        playpos_thumb.x = pos_x + playpos_bar.x - 2;
        playpos_thumb.y = playpos_bar.y - ((playpos_region.h * 1) / 3);
        playpos_thumb.w = 4;
        playpos_thumb.h = 2 + (((playpos_region.h * 1) / 3) * 2);
    }
}

void UpdateDisplayRect(void) {
    playpos_region = {0,0,0,0};
    playpos_thumb = {0,0,0,0};
    playpos_bar = {0,0,0,0};

    if (mainSurface) {
        display_region.x = 0;
        display_region.y = 0;
        display_region.w = mainSurface->w;
        display_region.h = mainSurface->h;

        if (display_region.h >= 60 && display_region.w >= 120) {
            display_region.h -= 20;
            playpos_region.x = 0;
            playpos_region.w = display_region.w;
            playpos_region.y = display_region.h;
            playpos_region.h = 20;

            int begin_x = (1+1+1+2+1+2+1+2+1)*8;    //  H:MM:SS.CC
                                                    // 1112-12-12-1
            int end_x = playpos_region.w - 8;

            assert((begin_x + 8) < end_x);

            playpos_bar.x = begin_x;
            playpos_bar.h = 4;
            playpos_bar.y = playpos_region.y + ((playpos_region.h - playpos_bar.h) / 2);
            playpos_bar.w = end_x - begin_x;

            PlayposBarRecomputeThumb();
        }
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
        gui_redraw = true;
    }
}

void mark_in(void);
void mark_out(void);
void clear_cut_points(void);
void do_seek_rel(double dt);
void next_video_stream(void);
void next_audio_stream(void);
void DrawVideoFrame(void);
void do_export_ui(void);
void DrawPlayPos(void);
bool is_playing(void);
void do_play(void);
void do_stop(void);

int mouse_drag = -1;

enum {
    MOUSE_DRAG_THUMB=1
};

void gui_redraw_do_locked(void) {
    memset(mainSurface->pixels, 0, static_cast<unsigned int>(mainSurface->pitch) * static_cast<unsigned int>(mainSurface->h));
    DrawVideoFrame();
    DrawPlayPos();
}

void do_gui_check_redraw(void) {
    if (gui_redraw) {
        SDL_LockSurface(mainSurface);
        gui_redraw_do_locked();
        SDL_UnlockSurface(mainSurface);
        SDL_UpdateWindowSurface(mainWindow);
        gui_redraw = false;

        gui_redraw_at_play_time = play_in_time + 0.1;
    }
}

void MouseDragThumb(int x,int y) {
    (void)y;

    if (playpos_thumb.w == 0)
        return;
    if (playpos_bar.w == 0)
        return;

    double np = double(x) - double(playpos_bar.x);
    np /= double(playpos_bar.w);
    if (np < 0) np = 0;
    if (np > 1) np = 1;
    np *= play_duration;

    double dt = np - play_in_time;
    do_seek_rel(dt);
}

bool GUI_Idle(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quitting_app = true;
        }
        else if (event.type == SDL_WINDOWEVENT) {
            GUI_OnWindowEvent(event.window);
        }
        else if (event.type == SDL_MOUSEMOTION) {
            if (mouse_drag == MOUSE_DRAG_THUMB) {
                MouseDragThumb(event.motion.x,event.motion.y);
                gui_redraw = true;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (playpos_thumb.w > 0 && playpos_thumb.h > 0) {
                SDL_Point p;
                SDL_Rect tmp;
                tmp = playpos_thumb;
                tmp.x -= 8;
                tmp.w += 8*2;
                tmp.y -= 4;
                tmp.h += 4*2;
                p.x = event.button.x;
                p.y = event.button.y;
                if (SDL_PointInRect(&p,&tmp)) {
                    SDL_CaptureMouse(SDL_TRUE);
                    mouse_drag = MOUSE_DRAG_THUMB;
                    MouseDragThumb(event.button.x,event.button.y);
                    gui_redraw = true;
                }
            }
        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            if (mouse_drag >= 0) {
                mouse_drag = -1;
                gui_redraw = true;
            }

            SDL_CaptureMouse(SDL_FALSE);
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
            else if (event.key.keysym.sym == SDLK_i) {
                mark_in();
            }
            else if (event.key.keysym.sym == SDLK_o) {
                mark_out();
            }
            else if (event.key.keysym.sym == SDLK_c) {
                clear_cut_points();
            }
            else if (event.key.keysym.sym == SDLK_v) {
                next_video_stream();
            }
            else if (event.key.keysym.sym == SDLK_a) {
                next_audio_stream();
            }
            else if (event.key.keysym.sym == SDLK_x) {
                do_export_ui();
            }
            else if (event.key.keysym.sym == SDLK_LEFTBRACKET) {
                if (in_point >= 0)
                    do_seek_rel(in_point - play_in_time);
            }
            else if (event.key.keysym.sym == SDLK_RIGHTBRACKET) {
                if (out_point >= 0)
                    do_seek_rel(out_point - play_in_time);
            }
            else if (event.key.keysym.sym == SDLK_LEFT) {
                if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT))
                    do_seek_rel(-60);
                else
                    do_seek_rel(-5);
            }
            else if (event.key.keysym.sym == SDLK_RIGHT) {
                if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT))
                    do_seek_rel(60);
                else
                    do_seek_rel(5);
            }
        }
    }

    if (is_playing() && gui_redraw_at_play_time >= 0.0 && play_in_time >= gui_redraw_at_play_time)
        gui_redraw = true;

    do_gui_check_redraw();
    SDL_Delay(is_playing() ? 1 : (1000/100));

    return !(quitting_app);
}

static constexpr size_t sdl_audio_queue_size = 128 * 1024 * 6;
int16_t     sdl_audio_queue[sdl_audio_queue_size];
size_t      sdl_audio_queue_in = 0,sdl_audio_queue_out = 0;

unsigned int audio_queue_delay_samples_nolock(void);

void sdl_audio_queue_flush(void) {
    SDL_LockAudio();
    sdl_audio_queue_in = 0;
    sdl_audio_queue_out = 0;
    SDL_UnlockAudio();
}

void audio_callback(void *userdata,Uint8* stream,int len) {
    (void)userdata;

    if (len < 0 || stream == NULL)
        return;

    int16_t *dst = reinterpret_cast<int16_t*>(stream);
    unsigned int samples = static_cast<unsigned int>(len) / (static_cast<unsigned int>(sizeof(int16_t)) * static_cast<unsigned int>(audio_spec.channels));

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

// This function is necessary because FFMPEG av_frame_clone() and av_frame_copy() will not
// copy packed audio or float planar audio. So we do it ourself. >:(
AVFrame *copy_the_audio_frame_because_ffmpeg_wont_do_it_for_some_stupid_reason(AVFrame *sf) {
    AVFrame *nfr = av_frame_alloc();
    if (nfr == NULL)
        return NULL;

    av_frame_copy_props(nfr,sf);
    nfr->channel_layout = sf->channel_layout;
    nfr->nb_samples = sf->nb_samples;
    nfr->channels = sf->channels;
    nfr->format = sf->format;
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

    assert(sf->nb_samples == nfr->nb_samples);

    int planes = av_sample_fmt_is_planar(AVSampleFormat(nfr->format)) ? nfr->channels : 1;
    for (int p=0;p < planes;p++) {
        assert(sf->data[p] != NULL);
        assert(nfr->data[p] != NULL);

        if (planes != 1) {
            memcpy(nfr->data[p],sf->data[p],
                static_cast<unsigned int>(nfr->nb_samples) *
                static_cast<unsigned int>(balign));
        }
        else {
            memcpy(nfr->data[p],sf->data[p],
                static_cast<unsigned int>(nfr->nb_samples) *
                static_cast<unsigned int>(nfr->channels) *
                static_cast<unsigned int>(balign));
        }
    }

    return nfr;
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
        int64_t         ts_adj = 0;
    };
public:
    InputFile() {
    }
    ~InputFile() {
        close();
    }
public:
    double get_duration(void) {
        double d = 0;

        if (!is_open())
            return 0;

        if (avfmt != NULL) {
            d = double(avfmt->duration) / AV_TIME_BASE;
        }

        return d;
    }
    double stream_start_time(const size_t i) {
        if (i < avfmt_stream_count()) {
            AVStream *s = avfmt_stream(i);
            if (s != NULL && s->start_time != AV_NOPTS_VALUE)
                return (double(s->start_time) * s->time_base.num) / s->time_base.den;
        }

        return 0;
    }
    void set_adj(double min_t) {
        for (size_t i=0;i < avfmt_stream_count();i++) {
            AVStream *s = avfmt_stream(i);
            Stream &si = stream(i);
            si.ts_adj = -int64_t((min_t * s->time_base.den) / s->time_base.num);
        }
    }
    void set_stream_adj(double min_t,const size_t i) {
        AVStream *s = avfmt_stream(i);
        if (s != NULL) {
            Stream &si = stream(i);
            si.ts_adj = -int64_t((min_t * s->time_base.den) / s->time_base.num);
        }
    }
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

        for (size_t i=0;i < avfmt_stream_count();i++)
            stream(i).ts_adj = 0;

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

            if (s->duration != AV_NOPTS_VALUE)
                fprintf(stderr," duration=%lld",static_cast<signed long long>(s->duration));
            else
                fprintf(stderr," duration=NOPTS");
 
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
    bool seek_to(double t) {
        if (avfmt == NULL)
            return false;

        eof = false;
        if (t < 0) t = 0;
        int64_t ts = int64_t(t * AV_TIME_BASE);
        if (avfmt->start_time != AV_NOPTS_VALUE)
            ts += avfmt->start_time;

        if (av_seek_frame(avfmt, -1, ts, AVSEEK_FLAG_BACKWARD) < 0) {
            fprintf(stderr,"avseek failed\n");
            return false;
        }

        return true;
    }
    AVPacket *read_packet(void) { // caller must not free it, because we will
        if (avfmt != NULL && !eof) {
            avpkt_reset();
            if (av_read_frame(avfmt,&avpkt) >= 0) {
                if (size_t(avpkt.stream_index) < size_t(avfmt->nb_streams)) {
                    Stream &s = stream(size_t(avpkt.stream_index));
                    if (avpkt.pts != AV_NOPTS_VALUE)
                        avpkt.pts += s.ts_adj;
                    if (avpkt.dts != AV_NOPTS_VALUE)
                        avpkt.dts += s.ts_adj;
                    return &avpkt;
                }
                else {
                    return NULL;
                }
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
        if (pkt->size == 0 || pkt->data == NULL)
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

            if (rd > pkt->size) rd = pkt->size;
            pkt->data += rd;
            pkt->size -= rd;

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

            if (rd > pkt->size) rd = pkt->size;
            pkt->data += rd;
            pkt->size -= rd;

            AVFrame *fr = av_frame_clone(strm.frame);
            if (fr == NULL)
                fr = copy_the_audio_frame_because_ffmpeg_wont_do_it_for_some_stupid_reason(strm.frame);

            if (fr == NULL)
                return NULL;

            ft = static_cast<unsigned int>(avc->codec_type);
            return fr;
        }

        return NULL;
    }
    void reset_codec(const size_t i) {
        AVStream *avs = avfmt_stream(i);
        if (avs == NULL)
            return;

        Stream &strm = stream(i);
        if (!strm.codec_open)
            return;

        AVCodecContext *avc = avs->codec;
        if (avc == NULL)
            return;

        avcodec_flush_buffers(avc);
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

void mark_in(void) {
    in_point = play_in_time;
    fprintf(stderr,"Mark in: %.3f\n",in_point);
    gui_redraw = true;
}

void mark_out(void) {
    out_point = play_in_time;
    fprintf(stderr,"Mark out: %.3f\n",out_point);
    gui_redraw = true;
}

void clear_cut_points(void) {
    fprintf(stderr,"Clearing cut points\n");
    in_point = -1;
    out_point = -1;
    gui_redraw = true;
}

void clear_in_point(void) {
    in_point = -1;
    gui_redraw = true;
}

void clear_out_point(void) {
    out_point = -1;
    gui_redraw = true;
}

InputFile &current_file(void) {
    return in_file;
}

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

void flush_queue(void);
void clear_current_av(void);

void do_seek_rel(double dt) {
    play_in_time += dt;
    if (play_in_time < 0) play_in_time = 0;
    if (play_in_time > play_duration) play_in_time = play_duration;
    play_in_base = play_in_time;
    playing_base = monotonic_clock_us();

    auto &fp = current_file();
    fp.reset_codec(size_t(in_file_audio_stream));
    fp.reset_codec(size_t(in_file_video_stream));
    fp.seek_to(play_in_time);
    sdl_audio_queue_flush();
    clear_current_av();
    flush_queue();

    gui_redraw = true;
}

void do_play(void) {
    if (!playing) {
        fprintf(stderr,"Playing\n");
        playing_base = monotonic_clock_us();
        play_in_base = play_in_time;
        gui_redraw = true;
        playing = true;
    }
}

void do_stop(void) {
    if (playing) {
        fprintf(stderr,"Stopping\n");
        get_play_time_now();
        playing_base = monotonic_clock_us();
        play_in_base = play_in_time;
        gui_redraw = true;
        playing = false;
    }
}

void Play_Idle(void);
void vga_print_font(int x,int y,const char *str);

bool do_prompt(std::string &str,const std::string &title) {
    size_t cursor_pos = str.length();
    int title_x = 0,title_y = 0;
    int text_x = 0,text_y = 0;
    SDL_Rect cursor_box;
    SDL_Rect prompt_box;
    SDL_Rect title_box;
    SDL_Event event;
    bool running = true;
    bool answer = false;
    bool sizeme = true;

    while (running) {
        Play_Idle();

        if (SDL_PollEvent(&event)) {
            char insert_char = 0;

            if (event.type == SDL_QUIT) {
                quitting_app = true;
                running = false;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                GUI_OnWindowEvent(event.window);

                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    sizeme = true;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    answer = false;
                    break;
                }
                else if (event.key.keysym.sym == SDLK_RETURN) {
                    answer = true;
                    break;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    if (cursor_pos > str.length())
                        cursor_pos = str.length();

                    if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT)) {
                        cursor_pos = 0;
                        str.clear();
                    }
                    else {
                        if (cursor_pos > 0) {
                            std::string last = str.substr(cursor_pos);
                            cursor_pos--;
                            std::string first = str.substr(0,cursor_pos);
                            str = first + last;
                            gui_redraw = true;
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_DELETE) {
                    if (cursor_pos < str.length()) {
                        std::string last = str.substr(cursor_pos+1);
                        std::string first = str.substr(0,cursor_pos);
                        str = first + last;
                        gui_redraw = true;
                    }
                }
                else if (event.key.keysym.sym == SDLK_MINUS) {
                    insert_char = '-';
                }
                else if (event.key.keysym.sym == SDLK_PERIOD) {
                    insert_char = '.';
                }
                else if (event.key.keysym.sym == SDLK_COMMA) {
                    insert_char = ',';
                }
                else if (event.key.keysym.sym == SDLK_HOME) {
                    cursor_pos = 0;
                    gui_redraw = true;
                }
                else if (event.key.keysym.sym == SDLK_END) {
                    cursor_pos = str.length();
                    gui_redraw = true;
                }
                else if (event.key.keysym.sym == SDLK_LEFT) {
                    gui_redraw = true;
                    if (cursor_pos)
                        cursor_pos--;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    gui_redraw = true;
                    if (cursor_pos < str.length())
                        cursor_pos++;
                }
                else if (event.key.keysym.sym == SDLK_SPACE) {
                    insert_char = ' ';
                }
                else if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z) {
                    if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT))
                        insert_char = char('A' + int(event.key.keysym.sym - SDLK_a));
                    else
                        insert_char = char('a' + int(event.key.keysym.sym - SDLK_a));
                }
                else if (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9) {
                    insert_char = char('0' + int(event.key.keysym.sym - SDLK_0));
                }
            }

            if (insert_char != 0) {
                gui_redraw = true;
                std::string last = str.substr(cursor_pos);
                std::string first = str.substr(0,cursor_pos);
                str  = first;
                str += insert_char;
                str += last;
                cursor_pos++;
            }
        }

        if (sizeme) {
            if (mainSurface->w < 400 || mainSurface->h < 128) {
                answer = false;
                break;
            }

            prompt_box.x = 8;
            prompt_box.w = std::max(mainSurface->w - 8 - prompt_box.x,0);
            prompt_box.y = 8;
            prompt_box.h = (1 + 14 + 1) + (1 + 14 + 1);

            title_box.x = prompt_box.x + 1;
            title_box.w = prompt_box.w - 1 - 1;
            title_box.y = prompt_box.y + 1;
            title_box.h = 14;

            title_x = int(((size_t(title_box.w) - (title.length() * size_t(8))) / size_t(2)) + size_t(title_box.x));
            title_y = title_box.y;

            text_x = prompt_box.x + 1;
            text_y = title_box.y + (1 + 14 + 1);

            gui_redraw = true;
            sizeme = false;
        }

        if (gui_redraw) {
            SDL_LockSurface(mainSurface);
            gui_redraw_do_locked();

            cursor_box.x = int(size_t(text_x) + (cursor_pos * size_t(8)));
            cursor_box.y = text_y;
            cursor_box.w = 2;
            cursor_box.h = 14;

            SDL_FillRect(mainSurface, &prompt_box,  SDL_MapRGB(mainSurface->format,15,15,15));
            SDL_FillRect(mainSurface, &title_box,   SDL_MapRGB(mainSurface->format,31,31,31));
            vga_print_font(title_x,title_y,title.c_str());
            vga_print_font(text_x,text_y,str.c_str());
            SDL_FillRect(mainSurface, &cursor_box,  SDL_MapRGB(mainSurface->format,127,191,191));

            SDL_UnlockSurface(mainSurface);
            SDL_UpdateWindowSurface(mainWindow);
            gui_redraw = false;

            gui_redraw_at_play_time = play_in_time + 0.1;
        }

        SDL_Delay(is_playing() ? 1 : (1000/100));
    }

    gui_redraw = true;
    return answer;
}

void do_export(const std::string &out_filename,double in_point,double out_point) {
    bool keyframe[2] = {false,false};
    int64_t last_next_pts[2] = {0,0};
    AVStream *out_video_stream = NULL;
    AVStream *out_audio_stream = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVDictionary *mp4_dict = NULL;
    AVOutputFormat *ofmt = NULL;
    const char *fmtname = NULL;
    auto &fp = current_file();
    int audio_stream = -1;
    int video_stream = -1;
    int ret;

    if (in_file_video_stream < 0 || in_file_audio_stream < 0)
        return;

    avformat_alloc_output_context2(&ofmt_ctx, NULL, fmtname, out_filename.c_str());
    if (!ofmt_ctx) {
        fprintf(stderr,"Failed to open output context\n");
        return;
    }
    ofmt = ofmt_ctx->oformat;
    assert(ofmt != NULL);

    if (in_file_video_stream >= 0) {
        AVStream *s = fp.avfmt_stream(size_t(in_file_video_stream));
        if (s != NULL) {
            out_video_stream = avformat_new_stream(ofmt_ctx, s->codec->codec);
            if (out_video_stream == NULL) {
                fprintf(stderr,"Failed to create new video stream\n");
                goto fail;
            }

            ret = avcodec_copy_context(out_video_stream->codec, s->codec);
            if (ret < 0) {
                fprintf(stderr,"Failed to copy video context\n");
                goto fail;
            }

            out_video_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            video_stream = out_video_stream->index;
            fprintf(stderr,"Output video stream %d\n",video_stream);
        }
    }

    if (in_file_audio_stream >= 0) {
        AVStream *s = fp.avfmt_stream(size_t(in_file_audio_stream));
        if (s != NULL) {
            out_audio_stream = avformat_new_stream(ofmt_ctx, s->codec->codec);
            if (out_audio_stream == NULL) {
                fprintf(stderr,"Failed to create new audio stream\n");
                goto fail;
            }

            ret = avcodec_copy_context(out_audio_stream->codec, s->codec);
            if (ret < 0) {
                fprintf(stderr,"Failed to copy audio context\n");
                goto fail;
            }

            out_audio_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_audio_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            audio_stream = out_audio_stream->index;
            fprintf(stderr,"Output audio stream %d\n",audio_stream);
        }
    }

    av_dump_format(ofmt_ctx, 0, out_filename.c_str(), 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr,"Failed to open output file\n");
            goto fail;
        }
    }

    ret = avformat_write_header(ofmt_ctx, &mp4_dict);
    if (ret < 0) {
        fprintf(stderr,"Failed to write header\n");
        goto fail;
    }

    fp.seek_to(in_point);

    do {
        AVPacket *pkt = fp.read_packet();
        if (pkt == NULL) break;

        int out_stream = -1;

        if (pkt->stream_index == in_file_video_stream)
            out_stream = video_stream;
        if (pkt->stream_index == in_file_audio_stream)
            out_stream = audio_stream;

        if (out_stream < 0)
            continue;

        assert(out_stream < 2);

        AVStream *avs = fp.avfmt_stream(size_t(pkt->stream_index));
        if (avs == NULL)
            continue;

        AVStream *ovs = NULL;
        if (out_stream == video_stream)
            ovs = out_video_stream;
        if (out_stream == audio_stream)
            ovs = out_audio_stream;
        if (ovs == NULL)
            continue;

        double pt = -1;
        int64_t pts = AV_NOPTS_VALUE;
        if (pts == AV_NOPTS_VALUE && pkt->pts != AV_NOPTS_VALUE)
            pts = pkt->pts;
        if (pts == AV_NOPTS_VALUE && pkt->dts != AV_NOPTS_VALUE)
            pts = pkt->dts;
        if (pts == AV_NOPTS_VALUE)
            pts = last_next_pts[out_stream];

        if (pts != AV_NOPTS_VALUE)
            pt = (double(pts) * avs->time_base.num) / avs->time_base.den;   // i.e. 1001/30000 for 29.97

        if (pt > (out_point + 5.0))
            break; // that's far enough
        if (pt < in_point)
            continue;

        if (keyframe[out_stream] == false) {
            if (!(pkt->flags & AV_PKT_FLAG_KEY))
                continue;
            if (pts == AV_NOPTS_VALUE) // muxers get grumpy without timestamps
                continue;

            fprintf(stderr,"Output stream %d first keyframe\n",out_stream);
            keyframe[out_stream] = true;
        }

        AVPacket* outpkt = av_packet_clone(pkt);
        if (outpkt != NULL) {
            int64_t adj;

            adj = int64_t((in_point * avs->time_base.den) / avs->time_base.num);

            if (pkt->pts != AV_NOPTS_VALUE)
                pkt->pts -= adj;
            if (pkt->dts != AV_NOPTS_VALUE)
                pkt->dts -= adj;

            outpkt->pts = av_rescale_q_rnd(pkt->pts, avs->time_base, ovs->time_base, AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            outpkt->dts = av_rescale_q_rnd(pkt->dts, avs->time_base, ovs->time_base, AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            outpkt->duration = av_rescale_q(pkt->duration, avs->time_base, ovs->time_base);
            outpkt->stream_index = out_stream;
            outpkt->pos = -1;

            ret = av_interleaved_write_frame(ofmt_ctx, outpkt);
            if (ret < 0)
                fprintf(stderr,"Error writing packet\n");

            av_packet_free(&outpkt);
        }

        last_next_pts[out_stream] = pts + pkt->duration;
    } while(1);

fail:
    if (ofmt_ctx != NULL) {
        if (ofmt_ctx->pb != NULL) {
            fprintf(stderr,"Writing trailer\n");
            av_write_trailer(ofmt_ctx);
        }

        if (!(ofmt->flags & AVFMT_NOFILE)) {
            fprintf(stderr,"Closing pb\n");
            avio_closep(&ofmt_ctx->pb);
        }
        fprintf(stderr,"Freeing context\n");
        avformat_free_context(ofmt_ctx);
        fprintf(stderr,"Done\n");
        ofmt_ctx = NULL;
        ofmt = NULL;
    }

    do_seek_rel(0);
}

void do_export_ui(void) {
    auto &fp = current_file();
    auto &name = fp.get_path();

    if (in_point < 0 || out_point < 0)
        return;
    if (in_point >= out_point)
        return;

    if (name.empty()) return;

    /* get file extension including dot */
    std::string ext;
    {
        size_t i = name.find_last_of('.');
        if (i == string::npos) return;
        ext = name.substr(i);
    }

    fprintf(stderr,"from filename = '%s'\n",name.c_str());
    fprintf(stderr,"ext = '%s'\n",ext.c_str());

    /* prompt for file name */
    std::string unamestr = "untitled-";
    {
        char tmp[256];
        sprintf(tmp,"%.2f-%.2f",in_point,out_point);
        unamestr += tmp;
    }
    if (!do_prompt(/*&*/unamestr,"Clip name?")) {
        fprintf(stderr,"User cancelled prompt\n");
        return;
    }
    if (unamestr.empty()) {
        fprintf(stderr,"User did not enter anything\n");
        return;
    }
    fprintf(stderr,"User entered = '%s'\n",unamestr.c_str());

    if (unamestr.find_first_of('/') != string::npos ||
        unamestr.find_first_of('\\') != string::npos) {
        fprintf(stderr,"Name rejected\n");
        return;
    }

    std::string final_name = unamestr + ext;
    fprintf(stderr,"Final name = '%s'\n",final_name.c_str());

    do_export(final_name,in_point,out_point);
}

struct QueueEntry {
    QueueEntry() {
    }
    QueueEntry(const QueueEntry &ent) = delete;
    QueueEntry(QueueEntry &&ent) {
        free();

        packet = ent.packet; ent.packet = NULL;
        frame = ent.frame; ent.frame = NULL;
        pt = ent.pt; ent.pt = 0;
    }
    QueueEntry& operator=(QueueEntry &ent) = delete;
    QueueEntry& operator=(QueueEntry &&ent) {
        free();

        packet = ent.packet; ent.packet = NULL;
        frame = ent.frame; ent.frame = NULL;
        pt = ent.pt; ent.pt = 0;
        return *this;
    }
    ~QueueEntry() {
        free();
    }
    double duration(void) {
        if (frame != NULL) {
            if (frame->sample_rate > 0)
                return double(frame->nb_samples) / frame->sample_rate;
        }

        return 0;
    }
    void free(void) {
        free_frame();
        free_packet();
    }
    void free_packet(void) {
        av_packet_free(&packet);
    }
    void free_frame(void) {
        av_frame_free(&frame);
    }
    bool                    update = false;
    bool                    done = false;
    AVFrame*                frame = NULL;
    AVPacket*               packet = NULL;
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
std::queue<QueueEntry>          video_queue_pkt;
QueueEntry                      current_audio_frame;
std::queue<QueueEntry>          audio_queue;
std::queue<QueueEntry>          audio_queue_pkt;

void clear_current_av(void) {
    current_video_frame.free();
    current_audio_frame.free();
}

void RedrawVideoFrame(void) {
    current_video_frame.update = true;
}

void flush_queue(queue<QueueEntry> &m) {
    while (!m.empty()) m.pop();
}

void flush_queue(void) {
    flush_queue(video_queue);
    flush_queue(audio_queue);
    flush_queue(video_queue_pkt);
    flush_queue(audio_queue_pkt);
}

bool schedule_video_frame(double pt,AVFrame *fr,AVStream *avs) {
    QueueEntry ent;
    if (avs->sample_aspect_ratio.num > 0) ent.container_ar = avs->sample_aspect_ratio;
    ent.frame = fr;
    ent.pt = pt;
    video_queue.push(std::move(ent));
//    fprintf(stderr,"Queued video frame pt=%.3f\n",pt);
    return true;
}

bool schedule_audio_frame(double pt,AVFrame *fr) {
    QueueEntry ent;
    ent.frame = fr;
    ent.pt = pt;
    audio_queue.push(std::move(ent));
//    fprintf(stderr,"Queued audio frame pt=%.3f\n",pt);
    return true;
}

int64_t video_last_next_pts = AV_NOPTS_VALUE;
bool queue_video_frame(AVFrame *fr,AVPacket *pkt,AVStream *avs) {
    (void)pkt;

    {
        double pt;
        int64_t pts = AV_NOPTS_VALUE;
        if (pts == AV_NOPTS_VALUE && fr->pts != AV_NOPTS_VALUE)
            pts = fr->pts;
        if (pts == AV_NOPTS_VALUE && fr->pkt_dts != AV_NOPTS_VALUE)
            pts = fr->pkt_dts;
        if (pts == AV_NOPTS_VALUE)
            pts = video_last_next_pts;

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
    (void)pkt;

    {
        double pt;
        int64_t pts = AV_NOPTS_VALUE;
        if (pts == AV_NOPTS_VALUE && fr->pts != AV_NOPTS_VALUE)
            pts = fr->pts;
        if (pts == AV_NOPTS_VALUE && fr->pkt_dts != AV_NOPTS_VALUE)
            pts = fr->pkt_dts;
        if (pts == AV_NOPTS_VALUE)
            pts = audio_last_next_pts;

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
        audio_resampler_trk.s.alloc_samples = frame.frame->nb_samples * 2;  // in case of ASF files
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
            swr_get_delay(audio_resampler,frame.frame->sample_rate) + audio_resampler_trk.s.alloc_samples,
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

void DrawVideoFrame(void) {
    draw_video_frame(current_video_frame);
}

void vga_print_char(int x,int y,const char c) {
    if (x < 0 || (x+8) > mainSurface->w)
        return;
    if (y < 0 || (y+14) > mainSurface->h)
        return;
    if (mainSurface->pixels == NULL)
        return;

    if (mainSurface->format->BytesPerPixel == 4) {
        unsigned char *fnt = vga_8x14_font + (static_cast<unsigned char>(c) * 14u);
        uint32_t fg = SDL_MapRGB(mainSurface->format,127,127,127);
        for (unsigned int py=0;py < 14;py++) {
            uint32_t *dst = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(mainSurface->pixels) + (4*x) +
                (static_cast<unsigned int>(mainSurface->pitch)*(static_cast<unsigned int>(y)+py)));
            unsigned char bmp = fnt[py];
            for (unsigned int px=0;px < 8;px++) {
                if (bmp & 0x80) *dst = fg;
                dst++;
                bmp = static_cast<unsigned char>(bmp << 1u);
            }
        }
    }
}

void vga_print_font(int x,int y,const char *str) {
    char c;

    while ((c = *str++) != 0) {
        vga_print_char(x,y,c);
        x += 8;
    }
}

void DrawPlayPos(void) {
    char tmp[80];

    if (playpos_region.w == 0 || playpos_region.h == 0)
        return;
    if (mainSurface->pixels == NULL)
        return;

    PlayposBarRecomputeThumb();

    SDL_FillRect(mainSurface, &playpos_region, SDL_MapRGB(mainSurface->format,31,31,31));
    SDL_FillRect(mainSurface, &playpos_bar,    SDL_MapRGB(mainSurface->format,63,63,63));

    if (in_point >= 0 || out_point >= 0) {
        SDL_Rect rct;
        int p1,p2;
        double t;

        t = std::min(in_point,play_duration) / play_duration;
        p1 = playpos_bar.x + static_cast<int>(t * double(playpos_bar.w));

        t = std::min(out_point,play_duration) / play_duration;
        p2 = playpos_bar.x + static_cast<int>(t * double(playpos_bar.w));

        if (in_point >= 0 && out_point >= 0 && in_point < out_point && (p1+1) < (p2-1)) {
            rct = playpos_bar;
            rct.x = p1 + 1;
            rct.w = (p2 - 1) - (p1 + 1);
            SDL_FillRect(mainSurface, &rct, SDL_MapRGB(mainSurface->format,144,191,144));
        }

        if (in_point >= 0) {
            rct = playpos_bar;
            rct.x = p1;
            rct.w = 1;
            SDL_FillRect(mainSurface, &rct, SDL_MapRGB(mainSurface->format,144,191,191));
        }

        if (out_point >= 0 && in_point < out_point && p1 < p2) {
            rct = playpos_bar;
            rct.x = p2 - 1;
            rct.w = 1;
            SDL_FillRect(mainSurface, &rct, SDL_MapRGB(mainSurface->format,144,191,191));
        }
    }

    SDL_FillRect(mainSurface, &playpos_thumb,
        (mouse_drag == MOUSE_DRAG_THUMB ?
            SDL_MapRGB(mainSurface->format,191,255,255) :
            SDL_MapRGB(mainSurface->format,160,160,160)));

    if (playpos_time.w > 0 && playpos_time.h > 0) {
        int cs = int(floor((play_in_time * 100) + 0.01));
        int cent = cs % 100; cs /= 100;
        int sec = cs % 60; cs /= 60;
        int min = cs % 60; cs /= 60;
        int h = cs;

        sprintf(tmp,"%u:%02u:%02u.%02u",h,min,sec,cent);
        vga_print_font(playpos_time.x,playpos_time.y,tmp);
    }
}

void next_stream_of_type(const int type,int &in_file_stream) {
    auto &fp = current_file();

    if (fp.is_open() && in_file_stream >= 0 && fp.avfmt_stream_count() != 0) {
        size_t patience = fp.avfmt_stream_count();

        in_file_stream++;
        if (size_t(in_file_stream) >= fp.avfmt_stream_count())
            in_file_stream = 0;

        do {
            AVStream *s = fp.avfmt_stream(size_t(in_file_stream));
            if (s != NULL) {
                if (s->codec->codec_type == type) {
                    if (type == AVMEDIA_TYPE_VIDEO) {
                        if (s->codec->width > 0 && s->codec->height > 0)
                            break;
                    }
                    else if (type == AVMEDIA_TYPE_AUDIO) {
                        if (s->codec->sample_rate > 0 && s->codec->channels > 0)
                            break;
                    }
                    else {
                        break;
                    }
                }
            }

            if (--patience == 0) {
                in_file_stream = -1;
                break;
            }

            in_file_stream++;
            if (size_t(in_file_stream) >= fp.avfmt_stream_count())
                in_file_stream = 0;
        } while(1);
    }
}

void recompute_start_adj(void) {
    auto &fp = current_file();
    double vs = 0,as = 0;

    if (in_file_video_stream >= 0)
        vs = fp.stream_start_time(size_t(in_file_video_stream));
    if (in_file_audio_stream >= 0)
        as = fp.stream_start_time(size_t(in_file_audio_stream));

    double fas;

    if (fabs(vs-as) < 1.0) {
        fas = std::min(vs,as);
        fp.set_adj(fas);
    }
    else {
        fprintf(stderr,"audio/video start are too far apart\n");
        fas = std::min(vs,as);
        fp.set_adj(fas);
        if (in_file_video_stream >= 0)
            fp.set_stream_adj(vs,size_t(in_file_video_stream));
        if (in_file_audio_stream >= 0)
            fp.set_stream_adj(as,size_t(in_file_audio_stream));
    }

    fprintf(stderr,"Start compute: as=%.3f vs=%.3f min=%.3f\n",as,vs,fas);
}

void next_video_stream(void) {
    auto &fp = current_file();

    next_stream_of_type(AVMEDIA_TYPE_VIDEO,/*&*/in_file_video_stream);
    if (in_file_video_stream == -1)
        in_file_video_stream = fp.find_default_stream_video();
    recompute_start_adj();
    sdl_audio_queue_flush();
    flush_queue(video_queue);
    flush_queue(video_queue_pkt);
    fp.reset_codec(size_t(in_file_video_stream));
    do_seek_rel(0);
}

void next_audio_stream(void) {
    auto &fp = current_file();

    next_stream_of_type(AVMEDIA_TYPE_AUDIO,/*&*/in_file_audio_stream);
    if (in_file_audio_stream == -1)
        in_file_audio_stream = fp.find_default_stream_audio();
    recompute_start_adj();
    sdl_audio_queue_flush();
    flush_queue(audio_queue);
    flush_queue(audio_queue_pkt);
    fp.reset_codec(size_t(in_file_audio_stream));
    do_seek_rel(0);
}

bool paused_need_frame = false;

void process_video_queue(void) {
    while (!video_queue.empty()) {
        auto &ent = video_queue.front();
        if ((is_playing() && play_in_time >= ent.pt) || paused_need_frame) {
            current_video_frame = std::move(ent);
            video_queue.pop();
            current_video_frame.update = true;
            paused_need_frame = false;
        }
        else {
            break;
        }
    }
}

void process_audio_queue(void) {
    while (!audio_queue.empty()) {
        auto &ent = audio_queue.front();
        if (is_playing() && audio_queue_delay() < 0.2 && play_in_time >= (ent.pt - audio_queue_delay())) {
            current_audio_frame = std::move(ent);
            audio_queue.pop();

            /* I'm not gonna take you back to the past,
             * to play stale old audio fragments that have passed. */
            if (current_audio_frame.pt >= (play_in_time-0.005-current_audio_frame.duration()-audio_queue_delay()))
                send_audio_frame(current_audio_frame);
        }

        break;
    }
}

void Play_Idle(void) {
    unsigned int ft;
    AVFrame *fr = NULL;
    auto &fp = current_file();

    if (is_playing() && !fp.is_open())
        do_stop();

    if (fp.is_open()) {
        bool active = false;
        size_t times;

        times = 512;
        while (times-- > 0) {
            if (video_queue_pkt.size() >= 512 || audio_queue_pkt.size() >= 2048)
                break;

            AVPacket *pkt = in_file.read_packet();
            if (pkt == NULL) break;

            if (pkt->stream_index == in_file_video_stream) {
                QueueEntry ent;
                ent.packet = av_packet_clone(pkt);
                video_queue_pkt.push(std::move(ent));
            }
            else if (pkt->stream_index == in_file_audio_stream) {
                QueueEntry ent;
                ent.packet = av_packet_clone(pkt);
                audio_queue_pkt.push(std::move(ent));
            }
        }

        times = 4;
        while (times-- > 0) {
            if (!is_playing() && in_file_video_stream >= 0 && !video_queue.empty() && current_video_frame.frame == NULL)
                paused_need_frame = true;

            if (video_queue.size() < 64 && !video_queue_pkt.empty()) {
                QueueEntry ent = std::move(video_queue_pkt.front());
                video_queue_pkt.pop();
                AVPacket *pkt = ent.packet;
                active = true;
                if (pkt != NULL && pkt->stream_index == in_file_video_stream) {
                    do {
                        fr = in_file.decode_frame(pkt,/*&*/ft);
                        if (fr != NULL) {
                            if (!queue_video_frame(fr,pkt,in_file.avfmt_stream(size_t(pkt->stream_index)))) {
                                av_frame_free(&fr);
                            }
                        }
                        else {
                            break;
                        }
                    } while(1);
                }
            }
            else {
                break;
            }
        }

        times = 16;
        while (times-- > 0) {
            if (audio_queue.size() < 256 && !audio_queue_pkt.empty()) {
                QueueEntry ent = std::move(audio_queue_pkt.front());
                audio_queue_pkt.pop();
                AVPacket *pkt = ent.packet;
                active = true;
                if (pkt != NULL && pkt->stream_index == in_file_audio_stream) {
                    // in some cases (old ASF files) the AVPacket contains multiple audio frames,
                    // and the decoder will only decode ONE.
                    do {
                        fr = in_file.decode_frame(pkt,/*&*/ft);
                        if (fr != NULL) {
                            if (!queue_audio_frame(fr,pkt,in_file.avfmt_stream(size_t(pkt->stream_index)))) {
                                av_frame_free(&fr);
                            }
                        }
                        else {
                            break;
                        }
                    } while(1);
                }
            }
        }

        if (!active) {
            if (in_file.is_eof()) {
                if (video_queue.empty() && audio_queue.empty() && video_queue_pkt.empty() && audio_queue_pkt.empty())
                    do_stop();
            }
        }

        get_play_time_now();
        process_video_queue();
        process_audio_queue();
    }

    if (current_video_frame.update && current_video_frame.frame != NULL) {
        current_video_frame.update = false;
        gui_redraw = true;
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

    play_duration = in_file.get_duration();
    in_file_audio_stream = in_file.find_default_stream_audio();
    in_file_video_stream = in_file.find_default_stream_video();
    fprintf(stderr,"Duration: %.3f\n",play_duration);
    fprintf(stderr,"Chose audio stream %d, video stream %d\n",in_file_audio_stream,in_file_video_stream);

    recompute_start_adj();

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

    if ((mainWindow=SDL_CreateWindow("Permanent Record player/clip maker",
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
