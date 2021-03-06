#if defined(__APPLE__)

#define PLATFORM_POSIX

#include "core.c"
#include "descriptor.c"
#include "hotplug.c"
#include "io.c"
#include "strerror.c"
#include "sync.c"

#include "os/darwin_usb.c"
#include "os/events_posix.c"
#include "os/threads_posix.c"

#else
// On linux, we link directly to the system library installed through
// apt-get / pacman or any other package manages
#endif
