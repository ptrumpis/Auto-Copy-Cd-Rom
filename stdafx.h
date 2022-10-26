#pragma once

// Windows-Header
#include <windows.h>

// C RunTime-Header
#include <cstdlib>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cctype>
#include <string>

#include <winbase.h>
#include <winioctl.h>
#include <shellapi.h>
#include <dbt.h>
#include <direct.h>


#ifdef UNICODE
#define _strstr wcsstr
#else
#define _strstr strstr
#endif // !UNICODE
