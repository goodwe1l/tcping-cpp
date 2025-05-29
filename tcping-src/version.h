// Ok.  "version.h" is in the specific project file instead of in the generic tcping-src, so I can have a compiled_in
// value for what bit version it is (in case it gets renamed or whatever.   Had to add...
// $(MSBuildProjectDirectory);
// .. into the include path so visual studio doesn't get confused.

// 定义当前版本和日期（这些值会在main.cpp中被引用）
#ifndef TCPING_VERSION_STR
#define TCPING_VERSION_STR "0.40"
#endif

#ifndef TCPING_DATE_STR
#define TCPING_DATE_STR "May 20 2025"
#endif

#ifdef __x86_64__
const int compiled_bitversion = 64;
#else
const int compiled_bitversion = 32;
#endif