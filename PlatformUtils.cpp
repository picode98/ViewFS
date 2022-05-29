//
// Created by sdk on 5/28/2022.
//

#include "PlatformUtils.h"

#ifdef _MSC_VER
    #include "windows/Subprocess.cpp"
    #include "windows/NamedPipe.cpp"
#else
#ifdef __linux__
        #include "linux/Subprocess.cpp"
        #include "linux/NamedPipe.cpp"
    #else
        #error "No implementation of platform-specific code is available for this platform."
    #endif
#endif
