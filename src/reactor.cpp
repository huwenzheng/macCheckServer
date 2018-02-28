////////////////////////////////////
//
//	@author		: huwenzheng
//	@date		: 2016/10/22
//	@purpose	: the realization of reactor
//
////////////////////////////////////

#include "reactor.h"

extern B_bool_t Is_NeedDebug;
B_bool_t xm_MacServer_CReactor::Is_Reactor_Loop;
B_bool_t xm_MacServer_CReactor::Is_Reactor_Init_OK;
B_bool_t xm_MacServer_CReactor::Is_Reactor_run_OK;
B_bool_t xm_MacServer_CReactor::Reactor_nFlag = false;

xm_MacServer_CReactor *
xm_MacServer_CReactor::instance()
{
	static xm_MacServer_CReactor *_instance = NULL;
	if (NULL == _instance)
		_instance = new xm_MacServer_CReactor();
	
	return _instance;
}

S_int32_t
xm_MacServer_CReactor::reactor_startserver()
{
	return thread_createthread();
}

xm_MacServer_CReactor::xm_MacServer_CReactor()
{
	Is_Reactor_run_OK = true;
	Reactor_nFlag = false;
};

S_int32_t
xm_MacServer_CReactor::reactor_stopserver()
{
	Is_Reactor_Loop = false;

	usleep(1000000);
	reactor_close_2sockets();

	if (Is_NeedDebug)
		Debug("===>>reactor server is ready to exit out.");

	//to destroy this class
	return thread_destroythread();
}

S_int32_t
xm_MacServer_CReactor::reactor_repairserver()
{
	for (int i = 0; i < XM_MACSERVER_THR_BUFLEN; i++) {
		if (G_data->m_sockfd[i] != -1)
			continue;

		//create sockets
		G_data->m_sockfd[i] = socket(AF_INET, SOCK_DGRAM, 0);
		
		//to set sockets reuse
		S_int32_t sock_on = 1;
		if (setsockopt(G_data->m_sockfd[i], SOL_SOCKET, 
			SO_REUSEADDR, &sock_on, sizeof(sock_on)) == -1)
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"we call setsockopt() err.");
			close(G_data->m_sockfd[i]);
			G_data->m_sockfd[i] = -1;
			return REACTOR_ERR;
		}

		if (i == 0) 
		{
			//to bind mac server
			if (bind(G_data->m_sockfd[i], (sockaddr*)&(G_data->m_addr[0]), sizeof(G_data->m_addr[0])) 
				== -1)
			{
				Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
						(S_int8_t*)"we call bind() err.");
				close(G_data->m_sockfd[i]);
				G_data->m_sockfd[i] = -1;
				return REACTOR_ERR;
			}
		}
		
		//to set non-blocking
		if (reactor_set_nonblocking(true, i) == REACTOR_ERR)
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
					(S_int8_t*)"set nonblocking err.");
			close(G_data->m_sockfd[i]);
			G_data->m_sockfd[i] = -1;
			return REACTOR_ERR;
		}
		
	}
	Is_Reactor_run_OK = true;
	return REACTOR_SUC;
}

S_int32_t
xm_MacServer_CReactor::reactor_recvfrom_device()
{
	sockaddr_in rcvAddr;
	MacInfo rcvBuf;

	memset(&rcvBuf, 0x00, sizeof(rcvBuf));
	memset(&rcvAddr, 0x00, sizeof(rcvAddr));
	socklen_t len = sizeof(sockaddr_in);

	if (G_data->m_sockfd[0] == -1) {
		Is_Reactor_run_OK = false;
		return REACTOR_ERR;
	}
	ssize_t nReady = recvfrom(G_data->m_sockfd[0], &rcvBuf, sizeof(rcvBuf), 0, (sockaddr*)&rcvAddr, &len);
	if (nReady < 0)
	{
		if (errno == EINTR || errno == EAGAIN)
			return REACTOR_EINTR;
		else
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"recvfrom msg from device error, port is 5556.");

			//to close if socketfd err
			shutdown(G_data->m_sockfd[0], 0x01);
			close(G_data->m_sockfd[0]);
			G_data->m_sockfd[0] = -1;

			Is_Reactor_run_OK = false;
			return REACTOR_ERR;
		}
	}
	

	P_MacNode *node = new P_MacNode;

	memset(node->fMyOldMac, 0x00, sizeof(node->fMyOldMac));
	memset(node->fSerialNo, 0x00, sizeof(node->fSerialNo));
	memset(node->fEncryDate, 0x00, sizeof(node->fEncryDate));
	memset(node->fMyNewMac, 0x00, sizeof(node->fMyNewMac));

	node->fRemoteAddr = ntohl(rcvAddr.sin_addr.s_addr);
	node->fRemotePort = ntohs(rcvAddr.sin_port);
	strncpy(node->fSerialNo, rcvBuf.pSerialNo, 16);
	strncpy(node->fMyOldMac, rcvBuf.pMyMac, 17);
	strncpy(node->fEncryDate, rcvBuf.pBurnDate, 10);

	if (Is_NeedDebug)
		Debug("===>Recv from Device 序列号:%s, 烧加密时间: %s", 
			node->fSerialNo, node->fEncryDate);
		
	//put in queue.
	G_data->Mutex_MacInfo_queue_t1.Get_lock();
	G_data->MacInfo_queue_t1.push(node);
	G_data->Mutex_MacInfo_queue_t1.Leave_lock();
	
	return REACTOR_SUC;
}

S_int32_t
xm_MacServer_CReactor::reactor_recvfrom_status()
{
	S_int8_t RcvBuf[2*XM_MACSERVER_MED_BUFLEN];
	sockaddr_in	rcvAddr;
	
	memset(&rcvAddr, 0x00, sizeof(rcvAddr));
	memset(RcvBuf, 0x00, sizeof(RcvBuf));
	socklen_t len = sizeof(sockaddr_in);

	if (G_data->m_sockfd[1] == -1) {
		Is_Reactor_run_OK = false;
		return REACTOR_ERR;
	}
	ssize_t nReady = recvfrom(G_data->m_sockfd[1], RcvBuf, sizeof(RcvBuf), 0, (sockaddr*)&rcvAddr, &len);
	if (nReady < 0) 
	{
		if (errno == EINTR || errno == EAGAIN)
			return REACTOR_EINTR;
		else
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"recvfrom statu server error, port is 8777.");

			//to close if socket err
			shutdown(G_data->m_sockfd[1], 0x01);
			close(G_data->m_sockfd[1]);
			G_data->m_sockfd[1] = -1;

			Is_Reactor_run_OK = false;
			return REACTOR_ERR;
		}	
	}

	S_int32_t nRetCode = REACTOR_SUC;
	S_int32_t msgID = *((S_int32_t*)RcvBuf);
	if (msgID == (S_int32_t)QUERY_MULT_DEV_NAT_RSP)
	{
		//it is the mssage we need.
		QueryMultDevNatRsp *pDevNatRsp = (QueryMultDevNatRsp *)(RcvBuf+4);
		if (pDevNatRsp->Ret == AUTH_HELPER_ONLINE)
		{
			//to debug
			if (Is_NeedDebug)
				Debug("===>Recv from 8777 (OnLine)序列号:%s, 设备所在服务器地址:%s", 
					pDevNatRsp->Uuid, pDevNatRsp->ServerAddr);

			//it is online
			nRetCode = reactor_statu_online(pDevNatRsp);
		}
		else
		{
			//to debug
			if (Is_NeedDebug)
				Debug("===>Recv from 8777 (OffLine)序列号:%s", pDevNatRsp->Uuid);

			//it is offline
			nRetCode = reactor_statu_offline(pDevNatRsp);
		}
	}
	
	return nRetCode;
}

S_int32_t
xm_MacServer_CReactor::reactor_statu_online(QueryMultDevNatRsp *p_RspNode)
{
	//if it is online, we will get it's infomation.
	p_RspNode->Uuid[16] = '\0';

	//to make send package.
	struct sockaddr_in SrvIpAddr;
	SrvIpAddr.sin_family = AF_INET;
	SrvIpAddr.sin_port = htons((U_int16_t)p_RspNode->Port);
	SrvIpAddr.sin_addr.s_addr = inet_addr(p_RspNode->ServerAddr);

	//to make package and send it to information server.
	S_int8_t SendBuf[XM_MACSERVER_MIN_BUFLEN];
	memset(SendBuf, 0x00, sizeof(SendBuf));

	*((S_int32_t*)(SendBuf)) = ASK_EXTINFO_REQ;
	strncpy(SendBuf+4, p_RspNode->Uuid, 16);

	if (G_data->m_sockfd[2] == -1) {
		Is_Reactor_run_OK = false;
		return REACTOR_ERR;
	}
	if (sendto(G_data->m_sockfd[2], SendBuf, strlen(SendBuf)+1, 0, 
		(sockaddr*)&SrvIpAddr, sizeof(struct sockaddr_in)) < 0)
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"sendto infomation server error, (8765 or 8000).");

		//to close socket if err
		shutdown(G_data->m_sockfd[2], 0x01);
		close(G_data->m_sockfd[2]);
		G_data->m_sockfd[2] = -1;

		Is_Reactor_run_OK = false;
		return REACTOR_ERR;
	}

	//to output statu map talbe, and add to information map talbe.
	std::map<std::string, P_MacNode*>::iterator iter;
	std::string str_uuid(p_RspNode->Uuid);
	
	G_data->Mutex_Sendto_8777_t4.Get_lock();
	iter = G_data->Sendto_8777_4status.find(str_uuid);
	if (iter != G_data->Sendto_8777_4status.end()) 
	{
		P_MacNode *p_node = iter->second;
		G_data->Sendto_8777_4status.erase(iter);
		G_data->Mutex_Sendto_8777_t4.Leave_lock();

		//add into information map talbe
		G_data->Mutex_Sendto_tosev_t5.Get_lock();
		G_data->Sendto_tosev_4information[str_uuid] = p_node;
		G_data->Mutex_Sendto_tosev_t5.Leave_lock();
	}
	G_data->Mutex_Sendto_8777_t4.Leave_lock();

	return REACTOR_SUC;
}

S_int32_t
xm_MacServer_CReactor::reactor_statu_offline(QueryMultDevNatRsp *p_RspNode)
{
	//if it is offline, we will get it's information
	p_RspNode->Uuid[16] = '\0';

	std::map<std::string, P_MacNode*>::iterator iter;
	std::string str_uuid(p_RspNode->Uuid);

	G_data->Mutex_Sendto_8777_t4.Get_lock();
	iter = G_data->Sendto_8777_4status.find(str_uuid);
	if (iter != G_data->Sendto_8777_4status.end())
	{
		P_MacNode *p_node = iter->second;
		G_data->Sendto_8777_4status.erase(iter);
		G_data->Mutex_Sendto_8777_t4.Leave_lock();

		//send reply to device
		NewmacInfo InfoPack;
		InfoPack.needChange = '0';
		memset(InfoPack.pNewMac, 0x00, sizeof(InfoPack.pNewMac));

		sockaddr_in DeviceAddr;
		DeviceAddr.sin_family = AF_INET;
		DeviceAddr.sin_port = htons(p_node->fRemotePort);
		DeviceAddr.sin_addr.s_addr = htonl(p_node->fRemoteAddr);

		//to free memory.
		delete p_node;
		p_node = NULL;

		//to reply
		if (sendto(G_data->m_sockfd[0], &InfoPack, sizeof(InfoPack), 0, 
		(sockaddr*)&DeviceAddr, sizeof(struct sockaddr_in)) < 0)
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"sendto device error, port 5556.");
			return REACTOR_ERR;
		}
	}
	G_data->Mutex_Sendto_8777_t4.Leave_lock();

	return REACTOR_SUC;
}

S_int32_t
xm_MacServer_CReactor::reactor_recvfrom_information()
{	
	//to check wheter the socket is valid
	if (G_data->m_sockfd[2] == -1) {
		Is_Reactor_run_OK = false;
		return REACTOR_ERR;
	}
	
	S_int8_t RcvBuf[2*XM_MACSERVER_MED_BUFLEN];
	sockaddr_in	rcvAddr;
	
	memset(&rcvAddr, 0x00, sizeof(rcvAddr));
	memset(RcvBuf, 0x00, sizeof(RcvBuf));
	socklen_t len = sizeof(sockaddr_in);

	ssize_t nReady = recvfrom(G_data->m_sockfd[2], RcvBuf, sizeof(RcvBuf), 0, (sockaddr*)&rcvAddr, &len);
	if (nReady < 0)
	{
		if (errno == EINTR || errno == EAGAIN)
			return REACTOR_EINTR;
		else
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"recvfrom information server error, (port is 8000 or 8765).");

			//to close socket if err
			shutdown(G_data->m_sockfd[2], 0x01);
			close(G_data->m_sockfd[2]);
			G_data->m_sockfd[2] = -1;

			Is_Reactor_run_OK = false;
			return REACTOR_ERR;
		}	
	}

	S_int32_t msgID = *((S_int32_t *)RcvBuf);
	if (msgID == ASK_EXTINFO_RSP)
	{
		S_int8_t p_uuid[XM_MACSERVER_MIN_BUFLEN];
		S_int8_t p_info[2*XM_MACSERVER_MED_BUFLEN];

		memset(p_uuid, 0x00, sizeof(p_uuid));
		memset(p_info, 0x00, sizeof(p_info));

		//get uuid
		memcpy(p_uuid, RcvBuf+4, 16);

		//to judge wheter it is online
		S_int32_t Is_OnLine = *((S_int32_t*)(RcvBuf+104));

		//get information for the device.
		S_int32_t extraLength = *((S_int32_t*)(RcvBuf+144));
		
		//to get export information
		memcpy(p_info, RcvBuf+148, extraLength);
		ExInfo *info = (ExInfo*)p_info;

		//to debug
		if (Is_NeedDebug)
			Debug("===>Recv from statu 序列号:%s, 烧加密时间:%02d-%02d-%02d", 
				info->SerialNo, info->encryptionDate.year, info->encryptionDate.month, info->encryptionDate.day);

		//to compare some information
		std::map<std::string, P_MacNode*>::iterator iter;
		std::string str_uuid(p_uuid);
		
		G_data->Mutex_Sendto_tosev_t5.Get_lock();
		iter = G_data->Sendto_tosev_4information.find(str_uuid);
		if (iter != G_data->Sendto_tosev_4information.end())
		{
			P_MacNode *p_oldnode = iter->second;
			G_data->Sendto_tosev_4information.erase(iter);
			G_data->Mutex_Sendto_tosev_t5.Leave_lock();

			S_int8_t p_data[XM_MACSERVER_MIN_BUFLEN];

			memset(p_data, 0x00, sizeof(p_data));
			sprintf(p_data, "%04d-%02d-%02d", info->encryptionDate.year,info->encryptionDate.month,info->encryptionDate.day);

			//满足在线、信息有效、烧加密时间不等才改mac
			if( Is_OnLine == 1 && extraLength != 0 && strncmp(p_data, p_oldnode->fEncryDate, 10) != 0 )
			{

				strncpy(p_oldnode->fRemoteEncrydate, p_data, 11);
				//Inspection found that the MAC is the same,
				//burning time, conflict is considered to be the MAC.
				G_data->Mutex_NeedChange_Mac_t6.Get_lock();
				G_data->Need_Change_Mac.push(p_oldnode);
				G_data->Mutex_NeedChange_Mac_t6.Leave_lock();
				
				if (Is_NeedDebug)
				{

					printf("Is_Online===>>%d, changdu : %d\n", Is_OnLine,extraLength);

					printf("----------------cloud results-----------------\n");
					printf("uuid: [%s] is online.\n",p_uuid);
					printf("device mac: %s\n", (char*)info->mac);
					printf("date: %d.%d.%d %d:%d:%d\n",info->encryptionDate.year,info->encryptionDate.month,info->encryptionDate.day,info->encryptionDate.hour,info->encryptionDate.minute,info->encryptionDate.second);

					printf("-----------------device results------------------\n");
					printf("device mac: %s\n", p_oldnode->fMyOldMac);
					printf("device date: %s\n", p_oldnode->fEncryDate);
					printf("---------------------------------------------\n");
					
					printf("\n=======================================================\n");
					Debug("info authorizeStat: %d", info->authorizeStat);
					Debug("info info->buildDate.day: %d", info->buildDate.day);
					Debug("info info->buildDate.hour: %d", info->buildDate.hour);
					Debug("info info->buildDate.minute: %d", info->buildDate.minute);
					Debug("info info->buildDate.month: %d", info->buildDate.month);
					Debug("info info->buildDate.second: %d", info->buildDate.second);
					Debug("info info->hostip: %d", info->hostip);
					Debug("info info->httpport: %d", info->httpport);
					Debug("info info->mac: %s", info->mac);
					Debug("info info->encryptionDate.day: %d", info->encryptionDate.day);
					Debug("info info->encryptionDate.hour: %d", info->encryptionDate.hour);
					Debug("info info->encryptionDate.isdst: %d", info->encryptionDate.isdst);
					Debug("info info->encryptionDate.minute: %d", info->encryptionDate.minute);
					Debug("info info->encryptionDate.month: %d", info->encryptionDate.month);
					Debug("info info->encryptionDate.second: %d", info->encryptionDate.second);
					Debug("info info->encryptionDate.wday: %d", info->encryptionDate.wday);
					Debug("info info->encryptionDate.year: %d", info->encryptionDate.year);
					Debug("info info->tcpport: %d", info->tcpport);
					Debug("info info->udpport: %d", info->udpport);
					Debug("info info->version: %s", info->version);
					Debug("info info->msgID: %d", info->msgID);
					printf("=====================================================\n");
					
				}
	
			}
			else
			{
				//examination revealed the same Mac
				//burn time is also same, is considered to be the same machine!
				//send reply to device
				NewmacInfo InfoPack;
				InfoPack.needChange = '0';
				memset(InfoPack.pNewMac, 0x00, sizeof(InfoPack.pNewMac));

				sockaddr_in DeviceAddr;
				DeviceAddr.sin_family = AF_INET;
				DeviceAddr.sin_port = htons(p_oldnode->fRemotePort);
				DeviceAddr.sin_addr.s_addr = htonl(p_oldnode->fRemoteAddr);

				//to free memory.
				delete p_oldnode;
				p_oldnode = NULL;

				//to reply
				if (sendto(G_data->m_sockfd[0], &InfoPack, sizeof(InfoPack), 0, 
				(sockaddr*)&DeviceAddr, sizeof(struct sockaddr_in)) < 0)
				{
					Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
						(S_int8_t*)"sendto device error, port 5556.");
					return REACTOR_ERR;
				}
			}

		}	

		G_data->Mutex_Sendto_tosev_t5.Leave_lock();

	}	

	return REACTOR_SUC;
}

S_int32_t
xm_MacServer_CReactor::reactor_start_2select()
{
	//set select timeout
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 200;
	Is_Reactor_Loop = true;
	while (Is_Reactor_Loop) 
	{
		
		//select setup
		fd_set fds;
		FD_ZERO(&fds);

		for (S_int32_t i = 0; i < 3; i++) 
		{
			if (G_data->m_sockfd[i] != -1)
				FD_SET(G_data->m_sockfd[i], &fds);
		}
		S_int32_t sockfd_max = XM_DEAFULT_MAX(XM_DEAFULT_MAX(G_data->m_sockfd[0],G_data->m_sockfd[1]),G_data->m_sockfd[2]);

		//wait for datas
		S_int32_t nReady = select(sockfd_max + 1, &fds, (fd_set*)0, (fd_set*)0, &tv);
		if (nReady < 0) 
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"select err. nRead < 0.");
			Is_Reactor_Loop = false;
		}
		else if (nReady == 0)
		{
			//to set timeout again.
			tv.tv_sec = 0;
			tv.tv_usec = 200;
			continue;
		}
		else if (FD_ISSET(G_data->m_sockfd[0], &fds))
		{
			//to recv data from device
			S_int32_t nRetCode = reactor_recvfrom_device();
			if (nRetCode == REACTOR_EINTR)
				continue;
		}
		else if (FD_ISSET(G_data->m_sockfd[1], &fds))
		{
			//to recv data from statu server
			S_int32_t nRetCode = reactor_recvfrom_status();
			if (nRetCode == REACTOR_EINTR)
				continue;
		}
		else if (FD_ISSET(G_data->m_sockfd[2], &fds))
		{
			//to recv data from infomation server
			S_int32_t nRetCode = reactor_recvfrom_information();
			if (nRetCode == REACTOR_EINTR)
				continue;
		}
		
	}

	//to close reactor server .
	reactor_close_2sockets();
	Is_Reactor_run_OK = false;
	
	return REACTOR_UNKOWN;
}

V_void_t
xm_MacServer_CReactor::reactor_close_2sockets()
{
	for (S_int32_t i = 0; i < 3; i++) 
	{
		if (G_data->m_sockfd[i] == -1)
			continue;
		
		shutdown(G_data->m_sockfd[i], 0x01);
		close(G_data->m_sockfd[i]);
		G_data->m_sockfd[i] = -1;
	}
	
	return;
}


V_void_t 
xm_MacServer_CReactor::thread_threadproc()
{
	if (Is_NeedDebug)
		Debug("To start reactor services.");
	//to init sockets
	if (reactor_init_2sockets() == REACTOR_ERR)
	{
		Is_Reactor_Init_OK = false;
		Is_Reactor_run_OK = false;
		return;
	}
	Is_Reactor_Init_OK = true;
	Is_Reactor_run_OK = true;

	//to start select
	reactor_start_2select();
	return;	
}

S_int32_t
xm_MacServer_CReactor::reactor_init_2sockets()
{
	
	for (S_int32_t i = 0; i < XM_MACSERVER_THR_BUFLEN; i++)
	{
		//create sockets
		G_data->m_sockfd[i] = socket(AF_INET, SOCK_DGRAM, 0);
		if (G_data->m_sockfd[i] == 0)
			return REACTOR_ERR;

		//to set sockets reuse
		S_int32_t sock_on = 1;
		if (setsockopt(G_data->m_sockfd[i], SOL_SOCKET, 
			SO_REUSEADDR, &sock_on, sizeof(sock_on)) == -1)
		{
			Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"we call setsockopt() err.");
			return REACTOR_ERR;
		}
	}

	//to make address for macserver
	G_data->m_addr[0].sin_family = AF_INET;
	G_data->m_addr[0].sin_port = htons(XM_MACSEV_SEV_PORT);
	G_data->m_addr[0].sin_addr.s_addr = INADDR_ANY;

	//to make address for statu server
	G_data->m_addr[1].sin_family = AF_INET;
	G_data->m_addr[1].sin_port = htons(XM_MACSEV_STA_PORT);
	G_data->m_addr[1].sin_addr.s_addr = inet_addr(XM_MACSEV_STATU_IP);

	//to bind mac server
	if (bind(G_data->m_sockfd[0], (sockaddr*)&(G_data->m_addr[0]), sizeof(G_data->m_addr[0])) 
		== -1)
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"we call bind() err.");
		return REACTOR_ERR;
	}

	//to set non-blocking
	if (reactor_set_nonblocking(false, 0) == REACTOR_ERR)
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"set nonblocking err.");
		return REACTOR_ERR;
	}

	return REACTOR_SUC;
}

S_int32_t
xm_MacServer_CReactor::reactor_set_nonblocking(B_bool_t IsOneSockfd, int Index)
{

	U_int32_t flags_old, flags_new;

	if (IsOneSockfd) {

		flags_old = 0;
		flags_new = 0;

		//get current socket flags; return if error
		flags_old = fcntl(G_data->m_sockfd[Index], F_GETFL, 0);
		if (flags_old < 0)
			return REACTOR_ERR;

		//set socket to blocking; return if error.
		flags_new = flags_old | O_NONBLOCK;
		if (fcntl(G_data->m_sockfd[Index], F_SETFL, flags_new) < 0)
		{
			//set flags back to where they were
			fcntl(G_data->m_sockfd[Index], F_SETFL, flags_old);
			return REACTOR_ERR;
		}

		return REACTOR_SUC;
	}

	for (U_int32_t i = 0; i < 3; i++) 
	{
		flags_old = 0;
		flags_new = 0;

		//get current socket flags; return if error
		flags_old = fcntl(G_data->m_sockfd[i], F_GETFL, 0);
		if (flags_old < 0)
			return REACTOR_ERR;

		//set socket to blocking; return if error.
		flags_new = flags_old | O_NONBLOCK;
		if (fcntl(G_data->m_sockfd[i], F_SETFL, flags_new) < 0)
		{
			//set flags back to where they were
			fcntl(G_data->m_sockfd[i], F_SETFL, flags_old);
			return REACTOR_ERR;
		}
	}

	return REACTOR_SUC;
}
























