#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ErrorCode
{
    RETURN_SUCCESS=0,
    RETURN_ERROR=-1,
    RETURN_BAD_REF=-2,
    RETURN_NULL_PTR=-3
} ErrorCode_t ;

#ifdef __cplusplus
}
#endif

#endif // ERROR_CODE_HPP