// pch.h: 这是预编译标头文件。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"
#include <mutex>

#endif //PCH_H


#define USE_OPENCV
#define USE_HALCON
#define DEBUG_LOG


#define GeneralLibrary_Zero 1e-14
#define MakeSurePathExists(a)																				\
{																											\
	std::string dirPath(a);																					\
	std::string path;																						\
	for (int i = 0; i < dirPath.length(); ++i)																\
	{																										\
		if (dirPath[i] == '\\' || dirPath[i] == '/')														\
		{																									\
			path = dirPath.substr(0, i);																	\
			if (_access(path.c_str(), 0) != 0)																\
			{																								\
				_mkdir(path.c_str());																		\
			}																								\
		}																									\
	}																										\
}

/// LogError("File:%s, Line:%d, Function:%s.", __FILE__, __LINE__, __func__);
/// #define STRINGIFY(x) "Line:"#x
/// #define TOSTRING(x) STRINGIFY(x)
/// #define STRING_LINE TOSTRING(__LINE__)
#define BEGIN_EXCEPTION() try{
#ifdef DEBUG_LOG
#define END_EXCEPTION()																						\
	}																										\
	catch (HException &except)																				\
	{																										\
		LogError("%s line:%d, ErrorCode = %d.", __func__, __LINE__, except.ErrorCode());					\
		LogError("%s line:%d, ErrorMessage = %s.", __func__, __LINE__, except.ErrorMessage().Text());		\
	}																										\
	catch (const std::exception& err)																		\
	{																										\
		LogError("%s line:%d, std::exception err = %s.", __func__, __LINE__, err.what());					\
	}																										\
	catch (...)																								\
	{																										\
		LogError("%s line:%d, unknown exception.", __func__, __LINE__);										\
	}
#else
#define END_EXCEPTION()																						\
	}																										\
	catch (HException &except)																				\
	{																										\
	}																										\
	catch (const std::exception& err)																		\
	{																										\
	}																										\
	catch (...)																								\
	{																										\
	}
#endif



#ifdef USE_OPENCV

#ifndef CV
#define CV

#include <opencv2/opencv.hpp>

#define CV_VERSION_ID  CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)

#ifdef _DEBUG
#include <opencv2/core/utils/logger.hpp>
#define cvLIB(name) "opencv_" name CV_VERSION_ID "d"
#else
#define cvLIB(name) "opencv_" name CV_VERSION_ID
#endif

#pragma comment(lib, cvLIB("world"))

#endif // !CV


#endif // USE_OPENCV


#ifdef USE_HALCON

#include "HalconCpp.h"
#include "HDevThread.h"

#pragma comment(lib, "halcon.lib")
#pragma comment(lib, "halconcpp.lib")

using namespace HalconCpp;

#endif // !USE_HALCON


#ifdef DEBUG_LOG

#pragma comment(lib, "log.lib")

#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#endif

#define __solution__ "GeneralLibrary"
LOG_API void LogTrace(const char* m, ...);
LOG_API void LogDebug(const char* m, ...);
LOG_API void LogInfo(const char* m, ...);
LOG_API void LogWarn(const char* m, ...);
LOG_API void LogError(const char* m, ...);
LOG_API void LogCritical(const char* m, ...);
/*
enum level_enum
{
	trace = 0,
	debug = 1,
	info = 2,
	warn = 3,
	err = 4,
	critical = 5,
	off = 6,
};
*/
LOG_API void SetLogLevel(int level);
LOG_API void SetLogFileName(const char* file_name);
LOG_API void LogOutputSystemMessage();

#endif // !DEBUG_LOG
