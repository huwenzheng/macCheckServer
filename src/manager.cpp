//////////////////////////////////////
//
//	@author		:	huwenzheng
//	@date		: 	2016/01/22
//	@purpose	:	the manager class realization file
//
//////////////////////////////////////

#include "manager.h"

xm_MacSev_CManager *
xm_MacSev_CManager::instance()
{
	static xm_MacSev_CManager *_instance = NULL;
	if (_instance == NULL) 
	{
		_instance = new xm_MacSev_CManager();
	}

	return _instance;
}

V_void_t
xm_MacSev_CManager::manager_startserver()
{
	Is_Start_MacServer = true;
	
	//start data manager server
	xm_MacSev_CDataManager::instance()->datam_startserver();

	//start reactor server
    xm_MacServer_CReactor::instance()->reactor_startserver();

	//start parse server
	xm_MacSev_CParseMsg::instance()->parse_startserver();

	//start new mac server
	xm_MacSev_CNewmac::instance()->newmac_startserver();


	//to sleep 3 seconds to wait for services initialization
	sleep(3);
	printf("--------services start over!----------\n");

	//to make sure we had init success 
	if (xm_MacSev_CDataManager::instance()->Is_Initdata_Failed)
	{
		manager_stopserver();
		return;
	}

	//to monitoring the servers
	while (Is_Start_MacServer)
	{	

		//to contrl wheter the mac address has been used up.
		if (xm_MacSev_CDataManager::instance()->Is_Mac_Finish)
		{
			manager_stopserver();
			break;
		}

		//to contrl wheter the reactor server has been err
		if (!xm_MacServer_CReactor::instance()->Is_Reactor_run_OK)
		{
			xm_MacServer_CReactor::instance()->reactor_repairserver();
		}

		//to contrl wheter the parsemsg server has been die
		if (!xm_MacSev_CParseMsg::instance()->Is_ParseMsg_run_OK)
		{
			xm_MacSev_CParseMsg::instance()->parse_stopserver();
			xm_MacSev_CParseMsg::instance()->parse_startserver();
		}

		//to contrl wheter the newmac server has been die
		if (!xm_MacSev_CNewmac::instance()->Is_NewMac_run_OK)
		{
			xm_MacSev_CNewmac::instance()->newmac_stopserver();
			xm_MacSev_CNewmac::instance()->newmac_startserver();
		}

		sleep(1);
	}

	return;
}

V_void_t
xm_MacSev_CManager::manager_stopserver()
{
	//to exit the contrl while loop
	Is_Start_MacServer = false;
	sleep(1);

	//to close the servers
	xm_MacServer_CReactor::instance()->reactor_stopserver();
	xm_MacSev_CParseMsg::instance()->parse_stopserver();
	xm_MacSev_CNewmac::instance()->newmac_stopserver();
	xm_MacSev_CDataManager::instance()->datam_stopserver();

	//to free the memory
	xm_MacSev_CDataManager::instance()->datam_destroy_4data();

	return;
}


