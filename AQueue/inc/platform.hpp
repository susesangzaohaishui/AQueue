

#ifndef __AQUEUE_PLATFORM_HPP_INCLUDED__
#define __AQUEUE_PLATFORM_HPP_INCLUDED__

//通过宏实现支持不同平台（Win/Linux）

#ifdef WIN32
#define AQUEUE_HAVE_WINDOWS
#else
#define AQUEUE_HAVE_LINUX
#endif

#endif
