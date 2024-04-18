#include "timeutils.h"

#ifdef _WIN32

#include <Windows.h>

uint32_t GetUnpreciseEpoch() {
    FILETIME ft;
    ULONGLONG epoch_time_ms;
    GetSystemTimePreciseAsFileTime(&ft);
    epoch_time_ms = ((ULONGLONG)ft.dwHighDateTime << 32 | ft.dwLowDateTime) / 10000ULL;
    return epoch_time_ms - 0xffffffff;
}

#elif __linux__

#include <time.h>

uint32_t GetUnpreciseEpoch() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t epoch_time_ms = (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
	return epoch_time_ms - 0xffffffff;
}

#else
#error "Unknown operating system!"
#endif

