/////////////////////////////////////////
//
//	@author		: huwenzheng
//	@date		: 2016/01/22
//	@purpose	: the header file of reactor
//
/////////////////////////////////////////

#ifndef __REACTOR_H__
#define __REACTOR_H__

#include "msgdata.h"
#include "thread.h"

class xm_MacServer_CReactor : public xm_MacServer_CThread
{
public:
	enum
	{
		REACTOR_ERR		= -1,
		REACTOR_SUC		= 0,
		REACTOR_EINTR	= 1,
		REACTOR_UNKOWN	= 100,
	};

	static B_bool_t Is_Reactor_Init_OK;
	static B_bool_t Is_Reactor_run_OK;
	static xm_MacServer_CReactor *instance();

	xm_MacServer_CReactor();
	virtual
	~xm_MacServer_CReactor()
	{ this->reactor_stopserver(); };

	S_int32_t
	reactor_repairserver();
	S_int32_t
	reactor_stopserver();
	S_int32_t
	reactor_startserver();

protected:

	//open one thread to do ours work
	virtual V_void_t 
	thread_threadproc();

	S_int32_t
	reactor_init_2sockets();

	S_int32_t
	reactor_start_2select();

	V_void_t
	reactor_close_2sockets();

	S_int32_t
	reactor_recvfrom_device();

	S_int32_t
	reactor_recvfrom_status();

	S_int32_t
	reactor_recvfrom_information();

	S_int32_t
	reactor_statu_online(QueryMultDevNatRsp *p_RspNode);

	S_int32_t
	reactor_statu_offline(QueryMultDevNatRsp *p_RspNode);

	
	S_int32_t
	reactor_set_nonblocking(B_bool_t IsOneSockfd, S_int32_t Index);
	
private:

	static B_bool_t Is_Reactor_Loop;
	static B_bool_t Reactor_nFlag;
};


#endif //__REACTOR_H__

























