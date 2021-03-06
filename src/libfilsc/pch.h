// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <map>
#include <vector>
#include <set>
#include <memory>

#include <string>
#include <string.h>
#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include <fstream>

#include <functional>
#include <algorithm>

#include <filesystem>

#include <assert.h>
#include <stdarg.h>
#include <math.h>
#include <sys/stat.h>

#include <direct.h>

#if defined(WIN32) || defined(WIN64)
    // Copied from linux libc sys/stat.h:
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
