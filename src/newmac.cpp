//////////////////////////////////////
//
//	@author		: huwenzheng
//	@date		: 2016/01/25
//	@purpose	: the realization of newmac class
//
//////////////////////////////////////

#include "newmac.h"
#include "reactor.h"


extern B_bool_t Is_NeedDebug;
B_bool_t xm_MacSev_CNewmac::Is_NewMac_run_OK;
B_bool_t xm_MacSev_CNewmac::Is_NewMac_Loop;

xm_MacSev_CNewmac *
xm_MacSev_CNewmac::instance()
{
	static xm_MacSev_CNewmac *_instance = NULL;
	if (NULL == _instance)
	{
		_instance = new xm_MacSev_CNewmac();
	}

	return _instance;
}

S_int32_t
xm_MacSev_CNewmac::newmac_startserver()
{	
	if (G_data->m_sockfd[0] == -1)
		return NEWMAC_ERR;
	return thread_createthread();
}

V_void_t 
xm_MacSev_CNewmac::thread_threadproc()
{	
	if (Is_NeedDebug)
		Debug("To start allocate a new mac services.");
	Is_NewMac_run_OK = true;
	newmac_send_newmac();
	Is_NewMac_run_OK = false;
	return;
}

V_void_t
xm_MacSev_CNewmac::newmac_send_newmac()
{
	Is_NewMac_Loop = true;
	sleep(1);
	while (Is_NewMac_Loop)
	{
		P_MacNode *node = NULL;
		G_data->Mutex_NeedChange_Mac_t6.Get_lock();
		if (G_data->Need_Change_Mac.size() > 0)
		{
			node = G_data->Need_Change_Mac.front();
			G_data->Need_Change_Mac.pop();
		}
		G_data->Mutex_NeedChange_Mac_t6.Leave_lock();

		if (node != NULL) {
			if (!newmac_allocation_mac(node)) {
				delete node;
				node = NULL;
			}
		}
		else
			usleep(500000);
	}

	return;
}

B_bool_t
xm_MacSev_CNewmac::newmac_allocation_mac(P_MacNode *node_t)
{	
	if (G_data->m_sockfd[0] == -1) {
		//to tell the Rector the socket err
		xm_MacServer_CReactor::instance()->Is_Reactor_run_OK = false;
		Is_NewMac_Loop = false;
		return false;
	}
	
	//to write into log
	S_int8_t log_t[2*XM_MACSERVER_MED_BUFLEN] = {0};

	sockaddr_in DeviceAddr;
	DeviceAddr.sin_family = AF_INET;
	DeviceAddr.sin_port = htons(node_t->fRemotePort);
	DeviceAddr.sin_addr.s_addr = htonl(node_t->fRemoteAddr);
	
	sprintf(log_t, "serialNo: %s\n %s: IP: %s  port: %d\n old mac: %s new mac: %s\n encrydate: %s, remote encrydate: %s\n\n",
		node_t->fSerialNo,Ctime::instance()->Get_gmt_time(),inet_ntoa(DeviceAddr.sin_addr), node_t->fRemotePort,
		node_t->fMyOldMac,G_data->mac_cur,node_t->fEncryDate, node_t->fRemoteEncrydate);

	S_int8_t filename[XM_MACSERVER_MIN_BUFLEN];
	memset(filename, 0x00, sizeof(filename));
	sprintf(filename, "%s_Modify_log.txt", Ctime::instance()->Get_Ym());

	FILE* p_fileLog = fopen(filename, "a");
	if( p_fileLog == NULL){
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"open Modify_log.txt error.");
		Is_NewMac_Loop = false;
		return false;
	}
	fwrite(log_t, sizeof(char), strlen(log_t), p_fileLog);
	fclose(p_fileLog);
	p_fileLog = NULL;


	//发送模块因为测试暂时注释掉
	//send new mac to device. 
    NewmacInfo InfoPack;
    InfoPack.needChange = '1'; 
    strncpy(InfoPack.pNewMac, G_data->mac_cur, strlen(G_data->mac_cur)+1);	
	
	if( -1 == sendto(G_data->m_sockfd[0], &InfoPack, sizeof(InfoPack), 0,
		 (sockaddr*)&DeviceAddr, sizeof(DeviceAddr)))
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"send new mac to device error.");
		
		//to close the socket if err
		shutdown(G_data->m_sockfd[0], 0x01);
		close(G_data->m_sockfd[0]);
		G_data->m_sockfd[0] = -1;
		
		//to tell the Rector the socket err
		xm_MacServer_CReactor::instance()->Is_Reactor_run_OK = false;

		Is_NewMac_Loop = false;
		return false;
	}

	//to free memory
	delete node_t;
	node_t = NULL;

	//to add mac
	if (strncmp(G_data->mac_cur, G_data->mac_end, 17) == 0)
	{
		G_data->Is_Mac_Finish = true;
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"mac had finish, you must to apply for new mac.");
		Is_NewMac_Loop = false;
		return true;
	}

	//这里因为测试暂时注释掉
	//mac auto add
	G_data->datam_mac_2autoadd();

	//to write into youxiao mac log
	FILE *p_mac = fopen("youxiao_mac.txt", "w");
	if( p_mac == NULL){
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"open youxiao_mac.txt error.");
		Is_NewMac_Loop = false;
		return true;
	}
	
	S_int8_t youxiao_mac[XM_MACSERVER_MIN_BUFLEN*2];
	memset(youxiao_mac, 0x00, sizeof(youxiao_mac));
	sprintf(youxiao_mac, "%s%s", G_data->mac_cur, G_data->mac_end);
	
	fwrite(youxiao_mac, sizeof(char), strlen(youxiao_mac), p_mac);
	fclose(p_mac);
	p_mac = NULL;
	
	return true;
}

S_int32_t
xm_MacSev_CNewmac::newmac_stopserver()
{
	Is_NewMac_Loop = false;

	usleep(500000);
	if (Is_NeedDebug)
		Debug("newmac server is ready to exit out.");
	
	return thread_destroythread();
}




