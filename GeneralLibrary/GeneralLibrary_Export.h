#pragma once


#ifdef GENERALLIBRARY_EXPORTS
#define GENERALLIBRARY_EXPORTS_API __declspec(dllexport) 
#else
#define GENERALLIBRARY_EXPORTS_API __declspec(dllimport)
#endif // GENERALLIBRARY_EXPORTS

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif


#define GeneralLibrary_Zero 1e-14
