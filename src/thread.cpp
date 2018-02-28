///////////////////////////////////////
//
//	author		:	huwenzheng
//	date		:	2016/01/20
//	purpose		:	the realization of Cthread
//
///////////////////////////////////////

#include "thread.h"
#include <time.h>
#include <sys/time.h>
#include <assert.h>

extern B_bool_t Is_NeedDebug;
S_int64_t xm_MacServer_CThread::sInitialMsec;
S_int64_t xm_MacServer_CThread::sMsecSince1970;

xm_MacServer_CThread::xm_MacServer_CThread()
	:fThreadID(0),fThreadLoop(false),fThreadJoined(false)
{
	//it is static
	sInitialMsec = 0;
	sMsecSince1970 = 0;
}

xm_MacServer_CThread::~xm_MacServer_CThread()
{
	this->thread_destroythread();
}

V_void_t*
xm_MacServer_CThread::thread_ThreadProc(V_void_t *arg)
{
	xm_MacServer_CThread *TheBase = (xm_MacServer_CThread*)arg;
	//to save his ID
	TheBase->fThreadID = pthread_self();
	//to do child class work
	TheBase->thread_threadproc();

	return NULL;
}

S_int32_t 
xm_MacServer_CThread::thread_createthread()
{
	if (thread_get_threadID() != 0 && thread_get_threadloop() != false)
	{
		return THR_ERR;
	}
	fThreadLoop = true;

	return pthread_create(&fThreadID, NULL, thread_ThreadProc, this);
}

S_int32_t 
xm_MacServer_CThread::thread_destroythread()
{
	fThreadLoop = false;

	if (!fThreadJoined)
		thread_threadjoin();

	return THR_SUC;
}

V_void_t
xm_MacServer_CThread::thread_threadjoin()
{
	if (fThreadJoined)
		return;

	V_void_t *retVal;

	fThreadJoined = true;
	pthread_join(fThreadID, &retVal);
}

V_void_t
xm_MacServer_CThread::thread_initialize()
{
	sInitialMsec = thread_milliseconds();

	sMsecSince1970 = time(NULL);
	sMsecSince1970 *= 1000;
}

S_int64_t
xm_MacServer_CThread::thread_milliseconds()
{
	struct timeval tv;
	S_int32_t theErr = gettimeofday(&tv, NULL);
	assert(theErr == 0);

	S_int64_t curTime;
	//sec->msec
	curTime = tv.tv_sec;
	curTime *= 1000;
	//usec->msec
	curTime += tv.tv_usec / 1000;

	return (curTime - sInitialMsec) + sMsecSince1970;
}











































