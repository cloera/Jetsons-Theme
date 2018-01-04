//-----------------------------------------------------------------------------
// Copyright Ed Keenan 2016
// Optimized C++
//----------------------------------------------------------------------------- 

#ifndef WINDOWS_WRAPPER_H
#define WINDOWS_WRAPPER_H

// Needs to be included because Window.h sucks for advanced warnings
#pragma warning( push )
#pragma warning( disable : 4820 )
#pragma warning( disable : 4668 )
#include <Windows.h>
// min funcition must be undefined or else it cannot be
// recognized in main()
#undef min
#pragma warning( pop ) 

#endif
