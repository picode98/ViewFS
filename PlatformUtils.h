//
// Created by sdk on 5/28/2022.
//

#ifndef VIEWFS_PLATFORMUTILS_H
#define VIEWFS_PLATFORMUTILS_H

#ifdef _MSC_VER
    #include "windows/Subprocess.h"
    #include "windows/NamedPipe.h"
#else
    #ifdef __linux__
        #include "linux/Subprocess.h"
        #include "linux/NamedPipe.h"
    #else
        #error "No implementation of platform-specific code is available for this platform."
    #endif
#endif

#endif //VIEWFS_PLATFORMUTILS_H
