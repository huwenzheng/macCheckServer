/////////////////////////////////////////
//
//	@author		: huwenzheng
//	@date		: 2016/01/22
//	@purpose	: the realization file of parsemsg
//
/////////////////////////////////////////

#include "parsemsg.h"
#include "libeznat.h"
#include "reactor.h"

extern B_bool_t Is_NeedDebug;
B_bool_t xm_MacSev_CParseMsg::Is_ParseMsg_run_OK; 
B_bool_t xm_MacSev_CParseMsg::Is_ParseMsg_Loop;

xm_MacSev_CParseMsg *
xm_MacSev_CParseMsg::instance()
{
	static xm_MacSev_CParseMsg *_instance = NULL;
	if (NULL == _instance)
	{
		_instance = new xm_MacSev_CParseMsg();
	}
	return _instance;
}

S_int32_t
xm_MacSev_CParseMsg::parse_startserver()
{	
	//to check the socket he need, if it is ok he will start
	if (G_data->m_sockfd[1] == -1)
		return PARSE_ERR;
	Is_ParseMsg_run_OK = true;
	return thread_createthread();
}

S_int32_t
xm_MacSev_CParseMsg::parse_stopserver()
{
	Is_ParseMsg_Loop = false;

	usleep(5000);
	if (Is_NeedDebug)
		Debug("===>parse server is ready to exit out.");
	
	return thread_destroythread();
}

V_void_t 
xm_MacSev_CParseMsg::thread_threadproc()
{
	if (Is_NeedDebug)
		Debug("To start process message services.");
	parse_start_procmsg();
	Is_ParseMsg_run_OK = false;
	return;
}

V_void_t
xm_MacSev_CParseMsg::parse_start_procmsg()
{
	
	Is_ParseMsg_Loop = true;
	while (Is_ParseMsg_Loop) 
	{
		P_MacNode *p_node = NULL;
	
		G_data->Mutex_MacInfo_queue_t1.Get_lock();
		if (!G_data->MacInfo_queue_t1.empty())
		{
			p_node = G_data->MacInfo_queue_t1.front();
			G_data->MacInfo_queue_t1.pop();
		}
		G_data->Mutex_MacInfo_queue_t1.Leave_lock();

		if (p_node != NULL)
			parse_proc_node(p_node);
		else
			usleep(5000);
	}

	return;
}

V_void_t
xm_MacSev_CParseMsg::parse_proc_node(P_MacNode *node)
{

	if (Is_NeedDebug & 0)
		Debug("处理的设备的序列号为:%s,烧加密时间为:%s", node->fSerialNo, node->fEncryDate);

	std::map<std::string, P_MacNode*>::iterator iter;
	std::string p_node(node->fSerialNo);

	G_data->Mutex_Goton_cloud_t3.Get_lock();
	iter = G_data->Goton_cloud_4status.find(p_node);
	if (iter != G_data->Goton_cloud_4status.end())
	{
		G_data->Mutex_Goton_cloud_t3.Leave_lock();
		//had got on the cloud, we must descard it.
		delete node;
		node = NULL;
	}
	else
	{
		//get on the cloud, add into the map table, but we only need uuid.
		G_data->Goton_cloud_4status[p_node] = node;
		G_data->Mutex_Goton_cloud_t3.Leave_lock();

		//to send to status server
		if (!parse_send_2statusev(node)) {
			delete node;
			node = NULL;
			return;
		}

		//to add into the map, we will use it.
		std::pair< std::map<std::string, P_MacNode*>::iterator, B_bool_t> ret;
		G_data->Mutex_Sendto_8777_t4.Get_lock();
		ret = G_data->Sendto_8777_4status.insert(std::pair<std::string, P_MacNode*>(p_node, node));
		G_data->Mutex_Sendto_8777_t4.Leave_lock();

		//if add into the map failed, we will delete it and free memory
		if (!(ret.second)) {
			delete node;
			node = NULL;
		}
		
	}

	return;
}

B_bool_t
xm_MacSev_CParseMsg::parse_send_2statusev(P_MacNode *Pnode)
{
	S_int8_t SendBuf[XM_MACSERVER_MIN_BUFLEN];
	memset(SendBuf, 0x00, sizeof(SendBuf));
	Device_Info dev_info;

	//make send package
	*((S_int32_t*)(SendBuf)) = QUERY_MULT_DEV_NAT_REQ;
	strncpy(dev_info.uuid, Pnode->fSerialNo, 17);

	sprintf((SendBuf+4),"%s",dev_info.uuid);
	S_int32_t sendsize = 21;

	if (G_data->m_sockfd[1] == -1) {
		//to tell the Rector the socket err
		xm_MacServer_CReactor::instance()->Is_Reactor_run_OK = false;
		return false;
	}
	
	//send to 8777 for status.
	if (sendto(G_data->m_sockfd[1], SendBuf, sendsize, 0, 
		(struct sockaddr*)&(G_data->m_addr[1]), sizeof(struct sockaddr_in)) == -1)
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
				(S_int8_t*)"sendto msg to statu servre error, port is 8777.");

		//to close the socket if err
		shutdown(G_data->m_sockfd[1], 0x01);
		close(G_data->m_sockfd[1]);
		G_data->m_sockfd[1] = -1;
		
		//to tell the Rector the socket err
		xm_MacServer_CReactor::instance()->Is_Reactor_run_OK = false;

		Is_ParseMsg_Loop = false;
		return false;
	}

	//to debug
	if (Is_NeedDebug)
		Debug("===>Send to 8777  序列号:%s,烧加密时间:%s", Pnode->fSerialNo, Pnode->fEncryDate);

	return true;
}






























