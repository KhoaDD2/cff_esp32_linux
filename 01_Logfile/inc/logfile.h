#ifndef LOGFILE_HPP__
#define LOGFILE_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#if 1
#define LOGD(pre,...) {printf("[D][%s:%04i]: " pre "\r\n",__FILENAME__,__LINE__,##__VA_ARGS__);}
#else
#define LOGD(pre,...)
#endif
#define LOGI(pre,...) {printf("[I][%s:%04i]: " pre "\r\n",__FILENAME__,__LINE__,##__VA_ARGS__);}
#define LOGE(pre,...) {printf("[E][%s:%04i]: " pre "\r\n",__FILENAME__,__LINE__,##__VA_ARGS__);while(1){};}
#define LOGF(pre,...) {printf("[F][%s:%04i]: " pre "\r\n",__FILENAME__,__LINE__,##__VA_ARGS__);}
#define LOGW(pre,...) {printf("[W][%s:%04i]: " pre "\r\n",__FILENAME__,__LINE__,##__VA_ARGS__);}

#ifdef __cplusplus
}
#endif

#endif // LOGFILE_HPP__