//////////////////////////////////////
//
//	@author		:	huwenzheng
//	@date		:	2016/01/20
//	@purpose	:	the header file of thread.h
//
//////////////////////////////////////

#ifndef __CTHREAD_H__
#define __CTHREAD_H__

#include "commUint.h"
#include "attachment.h"

class xm_MacServer_CThread
{
public:
	enum
	{
		THR_ERR 	= -1,
		THR_SUC 	= 0,
		THR_UKOWN	= 100,
	};

	xm_MacServer_CThread();
	virtual 
	~xm_MacServer_CThread();
	
	virtual V_void_t 
	thread_threadproc() = 0;

	S_int32_t 
	thread_createthread();

	S_int32_t 
    thread_destroythread();

	V_void_t
	thread_threadjoin();

	static V_void_t
	thread_initialize();
	
	
	S_int32_t 
	thread_get_threadID()
	{ return fThreadID; };

	B_bool_t 
	thread_get_threadloop()
	{ return fThreadLoop; };

	V_void_t 
	thread_set_threadloop(B_bool_t bLoop)
	{ fThreadLoop = bLoop; return; };

	static S_int32_t 
	thread_get_errno()
	{ return errno; };

	static pthread_t 
	thread_get_currentthreadID()
	{ return ::pthread_self(); };
	
	
protected:

	static V_void_t*
	thread_ThreadProc(V_void_t *arg);

	static S_int64_t
	thread_milliseconds();
	
private:
	pthread_t	fThreadID;

	B_bool_t	fThreadLoop;

	B_bool_t	fThreadJoined;

	static S_int64_t	sInitialMsec;

	static S_int64_t	sMsecSince1970;
};



#endif //__CTHREAD_H__


































