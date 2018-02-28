//////////////////////////////////////////
//
//	@author		: huwenzheng
//	@date		: 2016/01/25
//	@purpose	: the header file of send new mac class
//
//////////////////////////////////////////

#ifndef __NEWMAC_H__
#define __NEWMAC_H__

#include "msgdata.h"
#include "thread.h"

class xm_MacSev_CNewmac : public xm_MacServer_CThread
{
public:

	enum
	{
		NEWMAC_ERR	  = -1,
		NEWMAC_SUC	  = 0,
		NEWMAC_UNKOWN = 100,	
	};

	static B_bool_t Is_NewMac_run_OK;
	static xm_MacSev_CNewmac *instance();

	S_int32_t
	newmac_startserver();

	S_int32_t
	newmac_stopserver();

protected:

	V_void_t 
	thread_threadproc();

	V_void_t
	newmac_send_newmac();

	B_bool_t
	newmac_allocation_mac(P_MacNode *node_t);
	
private:

	static B_bool_t Is_NewMac_Loop;
	
};


#endif //__NEWMAC_H__






