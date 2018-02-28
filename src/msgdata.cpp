//////////////////////////////////////////////
//
//	@author		:	huwenzheng
//	@date		: 	2016/01/22
//	@purpose	:	the realization of the msgdata class
//
//////////////////////////////////////////////

#include "msgdata.h"

#define MACSEV_MAC_FILE	 "youxiao_mac.txt"
#define MACSEV_MAC_LEN	 17

extern B_bool_t Is_NeedDebug;

B_bool_t xm_MacSev_CDataManager::Is_Initdata_Failed;

xm_MacSev_CDataManager *
xm_MacSev_CDataManager::instance()
{
	static xm_MacSev_CDataManager *_instance = NULL;
	if (NULL == _instance)
	{
		_instance = new xm_MacSev_CDataManager();
	}
	return _instance;
}

xm_MacSev_CDataManager::xm_MacSev_CDataManager()
{
	//to initializa some datas
	memset(mac_cur, 0x00, sizeof(mac_cur));
	memset(mac_end, 0x00, sizeof(mac_end));
	Is_Mac_Finish = false;
	
	//the list for had got on cloud
	Goton_cloud_4status.clear();
	Sendto_8777_4status.clear();
	Sendto_tosev_4information.clear();
	t1_can_get_it = false;

	Is_Datam_Loop = true;
	Is_Initdata_Failed = false;
}

S_int32_t
xm_MacSev_CDataManager::datam_startserver()
{
	return thread_createthread();
}

S_int32_t
xm_MacSev_CDataManager::datam_stopserver()
{
	Is_Datam_Loop = false;

	usleep(1000000);
	if (Is_NeedDebug)
		Debug("datam is ready to exit out.");
	
	return thread_destroythread();
}


V_void_t 
xm_MacSev_CDataManager::thread_threadproc()
{
	//to init datas.
	if (datam_initialize_4data() == DATAMANA_ERR)
	{
		if (Is_NeedDebug)
			Debug("Initdata failed ! Is_Initdata_Failed = true.");
		
		Is_Initdata_Failed = true;
		return;
	}

	//to loop and clear data
	Is_Datam_Loop = true;
	if (Is_NeedDebug)
		Debug("To start control data services.");
	datam_loop_4data();
	return;
}

S_int32_t
xm_MacSev_CDataManager::datam_sleep(S_int32_t nSec)
{
	if (nSec <= 0)
		return usleep(0);

	S_int32_t nSecond_t = nSec;
	while (Is_Datam_Loop && nSecond_t > 0)
	{
		usleep(1000000);
		--nSecond_t;
	}

	return nSecond_t;
}


V_void_t
xm_MacSev_CDataManager::datam_loop_4data()
{
	while (Is_Datam_Loop)
	{
		datam_sleep(50);

		//to clear get on cloud flap
		G_data->Mutex_Goton_cloud_t3.Get_lock();
		G_data->Goton_cloud_4status.clear();
		G_data->Mutex_Goton_cloud_t3.Leave_lock();

		//5 points to clean up once a day
		struct tm *local;
		time_t tv;
		tv = time(NULL);
		local = localtime(&tv);

		if (local->tm_hour == 5 && local->tm_min == 0)
		{
			//to clear then 8777 map
			std::map<std::string, P_MacNode*>::iterator iter1;
			G_data->Mutex_Sendto_8777_t4.Get_lock();
			for(iter1 = (G_data->Sendto_8777_4status).begin(); iter1 != (G_data->Sendto_8777_4status).end();iter1++)
			{
				delete iter1->second;
				iter1->second = NULL;

			}
			G_data->Sendto_8777_4status.erase(G_data->Sendto_8777_4status.begin(), 
				G_data->Sendto_8777_4status.end());
			G_data->Mutex_Sendto_8777_t4.Leave_lock();

			//to clear the 8765 or 8000 map
			std::map<std::string, P_MacNode*>::iterator iter2;
			G_data->Mutex_Sendto_tosev_t5.Get_lock();
			for(iter2 = G_data->Sendto_tosev_4information.begin(); iter2 != G_data->Sendto_tosev_4information.end();iter2++)
			{
				delete iter2->second;
				iter2->second = NULL;
			}
			G_data->Sendto_tosev_4information.erase(G_data->Sendto_tosev_4information.begin(), 
				G_data->Sendto_tosev_4information.end());
			G_data->Mutex_Sendto_tosev_t5.Leave_lock();
		}

	}
	
}


S_int32_t
xm_MacSev_CDataManager::datam_initialize_4data()
{
	
	//to get the available mac
	FILE *p_macfile = fopen(MACSEV_MAC_FILE, "r");
	if (p_macfile == NULL) 
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"open youxiao_mac.txt error, you must make sure you have it.");
		return DATAMANA_ERR;
	}
	if (fread(mac_cur, sizeof(char), MACSEV_MAC_LEN, p_macfile) < 17
		|| fread(mac_end, sizeof(char), MACSEV_MAC_LEN, p_macfile) < 17)
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"read youxiao mac error, you can your file's format.");
		return DATAMANA_ERR;
	}
	fclose(p_macfile);
	p_macfile = NULL;

	//to debug
	if (Is_NeedDebug)
		Debug("mac_cur: %s, mac_end: %s", mac_cur, mac_end);

	//to check mac, we must make sure it is can be use
	if (!datam_mac_usable_check(mac_cur, mac_end))
	{
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE,
			(S_int8_t*)"youxiao mac can't be use, you can check it again.");
		return DATAMANA_ERR;
	}

	return DATAMANA_SUC;
}

B_bool_t
xm_MacSev_CDataManager::datam_mac_usable_check(S_int8_t *mac_t1, S_int8_t *mac_t2)
{
	B_bool_t nFlag = true;

	//to check format
	for(int i = 0; i < 17; i++){
		
		if(i == 2 || i == 5 || i == 8 || i == 11 || i == 14){
			if(mac_t1[i] != ':' || mac_t2[i] != ':'){
				nFlag = false;
				break;	
			}	
		}else{
		
			if( (mac_t1[i] >= '0' && mac_t1[i] <= '9') 
				|| (mac_t1[i] >= 'A' && mac_t1[i] <= 'F') )	
				nFlag = true;
			else{
				nFlag = false;
				break;
			}
	
			if( (mac_t2[i] >= '0' && mac_t2[i] <= '9') 
				|| (mac_t2[i] >= 'A' && mac_t2[i] <= 'F') )	
				nFlag = true;
			else{
				nFlag = false;
				break;
			}
			
		}	
	}

	//to check usable
	if (strncmp(mac_t1, mac_t2, 17) >= 0)
		nFlag = false;
	
	return nFlag;
}


V_void_t 
xm_MacSev_CDataManager::datam_destroy_4data()
{

	std::map<std::string, P_MacNode*>::iterator iter;
	G_data->Mutex_Sendto_8777_t4.Get_lock();
	for(iter = G_data->Sendto_8777_4status.begin(); iter != G_data->Sendto_8777_4status.end();iter++)
	{
		delete iter->second;
		iter->second = NULL;

		//iter = G_data->Sendto_8777_4status.erase(iter);
	}
	G_data->Sendto_8777_4status.erase(G_data->Sendto_8777_4status.begin(), 
		G_data->Sendto_8777_4status.end());
	G_data->Mutex_Sendto_8777_t4.Leave_lock();

	G_data->Mutex_Sendto_tosev_t5.Get_lock();
	for(iter = G_data->Sendto_tosev_4information.begin(); iter != G_data->Sendto_tosev_4information.end();iter++)
	{
		delete iter->second;
		iter->second = NULL;
		//iter = (G_data->Sendto_tosev_4information).erase(iter);
	}
	G_data->Sendto_tosev_4information.erase(G_data->Sendto_tosev_4information.begin(), 
		G_data->Sendto_tosev_4information.end());
	G_data->Mutex_Sendto_tosev_t5.Leave_lock();
	
	return;
}

V_void_t 
xm_MacSev_CDataManager::datam_mac_2autoadd()
{
	if( ( mac_cur[16] = mac_cur[16] == '9' ? 
		'A' : (mac_cur[16] += 1) ) == 'G'){
		mac_cur[16] = '0';
		if( (mac_cur[15] = mac_cur[15] == '9' ? 
			'A' : (mac_cur[15] += 1) ) == 'G'){
			mac_cur[15] = '0';
			if( (mac_cur[13] = mac_cur[13] == '9' ? 
				'A' : (mac_cur[13] += 1) ) == 'G'){
				mac_cur[13] = '0';
				if( ( mac_cur[12] = mac_cur[12] == '9' ? 
					'A' : (mac_cur[12] += 1) ) == 'G'){
					mac_cur[12] = '0';
					if( ( mac_cur[10] = mac_cur[10] == '9' ? 
						'A' : (mac_cur[10] += 1) ) == 'G'){
						mac_cur[10] = '0';
						if( ( mac_cur[9] = mac_cur[9] == '9' ?
							 'A' : (mac_cur[9] += 1) ) == 'G'){
							mac_cur[9] = '0';	
							if( ( mac_cur[7] = mac_cur[7] == '9' ?
								 'A' : (mac_cur[7] += 1) ) == 'G'){
								mac_cur[7] = '0';	
								if( ( mac_cur[6] = mac_cur[6] == '9' ?
									 'A' : (mac_cur[6] += 1) ) == 'G'){
									mac_cur[6] = '0';	
									if( ( mac_cur[4] = mac_cur[4] == '9' ?
										 'A' : (mac_cur[4] += 1) ) == 'G'){
										mac_cur[4] = '0';	
										if( ( mac_cur[3] = mac_cur[3] == '9' ?
											 'A' : (mac_cur[3] += 1) ) == 'G'){
											mac_cur[3] = '0';	
											if( ( mac_cur[1] = mac_cur[3] == '9' ?
											 		'A' : (mac_cur[1] += 1) ) == 'G'){
												mac_cur[1] = '0';
												if( ( mac_cur[0] = mac_cur[3] == '9' ?
											 			'A' : (mac_cur[0] += 1) ) == 'G'){
													printf("I'am Wellson.\n");	
													
	}	}	}	}	}	}	}	}	}	}	}	}

	return;
}










