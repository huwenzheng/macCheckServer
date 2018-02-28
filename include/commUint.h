///////////////////////////////////
//
//	@author		:	huwenzheng
//	@date		:	2016/01/20
//	@purpose	:	public header file
//
///////////////////////////////////

#ifndef __COMMUINT_H__
#define __COMMUINT_H__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>
#include <errno.h>
#include <map>
#include <list>
#include <queue>
#include <string>
#include <signal.h>

typedef signed long long	S_int64_t;
typedef unsigned long long 	U_int64_t;
typedef signed int			S_int32_t;
typedef unsigned int 		U_int32_t;
typedef signed short		S_int16_t;
typedef unsigned short		U_int16_t;
typedef char				S_int8_t;
typedef unsigned char		U_int8_t;
typedef bool				B_bool_t;
typedef void				V_void_t;


//define debug function
#define Debug(format, ...) printf("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)


//define alignment rule
#define XM_DEAFULT_ALIGN(size) ((size & 0xfffffffc) + ((size & 3) ? 4 : 0))

//define max min
#define XM_DEAFULT_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define XM_DEAFULT_MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif //__COMMUINT_H__


































