/////////////////////////////////////////
//
//	@author		: huwenzheng
//	@date		: 2016/01/22
//	@purpose	: the header file of parsemsg
//
/////////////////////////////////////////

#ifndef __PARSEMSG_H__
#define __PARSEMSG_H__

#include "msgdata.h"

class xm_MacSev_CParseMsg : public xm_MacServer_CThread
{
public:
	enum
	{
		PARSE_ERR		= -1,
		PARSE_SUC   	= 0,
		PARSE_UNKOWN 	= 100,
	};

	static B_bool_t Is_ParseMsg_run_OK;
	static xm_MacSev_CParseMsg *instance();

	S_int32_t
	parse_startserver();

	S_int32_t
	parse_stopserver();
	
protected:

	//open one thread to do ours work
	virtual V_void_t 
	thread_threadproc();

	V_void_t
	parse_start_procmsg();

	V_void_t
	parse_proc_node(P_MacNode *node);

	B_bool_t
	parse_send_2statusev(P_MacNode *Pnode);
	
private:

	static B_bool_t Is_ParseMsg_Loop;
	
};

#endif //__PARSEMSG_H__
























