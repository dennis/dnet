#ifndef DEBUG
#define DEBUG
#endif

#ifdef DEBUG
#include <dnet/time.hpp>
 #define LOG(type,...) { printf("%-10ud " type " %s:%d ", dnet::dnettime(), __FILE__, __LINE__ ); printf(__VA_ARGS__); }
 #define LOG_DIST(...) LOG("dist", __VA_ARGS__)
// #define LOG_NET(...) LOG("net ", __VA_ARGS__)
 #define LOG_SYNC(...) LOG("sync", __VA_ARGS__)
 #define LOG_BUF(...) LOG("buf ", __VA_ARGS__)
// #define LOG_PACK(...) LOG("pack", __VA_ARGS__)
#endif

#ifndef DEBUG
#define NDEBUG 1
#endif

#ifndef LOG
#define LOG(...)
#endif

#ifndef LOG_DIST
#define LOG_DIST(...)
#endif

#ifndef LOG_NET
#define LOG_NET(...)
#endif

#ifndef LOG_BUF
#define LOG_BUF(...)
#endif

#ifndef LOG_PACK
#define  LOG_PACK(...)
#endif

#ifndef LOG_SYNC
#define LOG_SYNC(...)
#endif
