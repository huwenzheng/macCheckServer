//////////////////////////////////////
//
//	@author		:	huwenzheng
//	@date		: 	2016/01/22
//	@purpose	:	the manager class header file
//
//////////////////////////////////////

#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "msgdata.h"
#include "reactor.h"
#include "parsemsg.h"
#include "newmac.h"

class xm_MacSev_CManager
{
public:
	static xm_MacSev_CManager *instance();

	xm_MacSev_CManager() : Is_Start_MacServer(false){};
	
	~xm_MacSev_CManager()
	{ this->manager_stopserver(); };

	V_void_t
	manager_startserver();

	V_void_t
	manager_stopserver();
protected:
private:
	B_bool_t Is_Start_MacServer;
};

#endif //__MANAGER_H__
























