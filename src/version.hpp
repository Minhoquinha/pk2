#pragma once

#define PK2_NAME         "Pekka Kana 2"
#define PK2_VERSION      "1.3.2"


#define PK2_VERSION "1.3.2"
#define PK2_VERNUM 0x0302
#define PK2_VER_MAJOR 1
#define PK2_VER_MINOR 3
#define PK2_VER_REVISION 2

#define PK2_VERSION_NAME "(r3)"

#ifdef NO_ZIP
#define PK2_ZIP "(no-zip)"
#else
#define PK2_ZIP "(zip)"
#endif

#ifdef __ANDROID__
#define PK2_PLATFORM "(Android)"
#elif _WIN32
#define PK2_PLATFORM "(Windows)"
#else
#define PK2_PLATFORM "(Linux)"
#endif

#define PK2_VERSION_STR PK2_VERSION " " PK2_VERSION_NAME " " PK2_PLATFORM " " PK2_ZIP