#ifndef HTTPTEXT_OUT_H_
#define HTTPTEXT_OUT_H_

#include "../IHTTPData.h"

#ifdef __HTTPTEXT_OUT_C__

#ifdef __cplusplus
extern "C" {
#endif

char* HTTPTextOut_GetData(void);

#ifdef __cplusplus
}
#endif

#else
extern IHTTPDataOut gHTTPDataOut;
#endif
#endif /* HTTPTEXT_H_ */
