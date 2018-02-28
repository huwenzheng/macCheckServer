//////////////////////////////////
//
//	@author		:	huwenzheng
//	@date		:	2016/01/20
//	@purpose	:	msg header file
//
//////////////////////////////////



#ifndef __MSGDATA_H
#define __MSGDATA_H

#include "commUint.h"
#include "attachment.h"
#include "thread.h"

#define ASK_EXTINFO_REQ	     0x03f22012
#define ASK_EXTINFO_RSP	     0x03f32012

#define MACSEV_MOD_FILE		 "Modify_log.txt"
#define MACSEV_ERR_FILE	     "Exception.txt"

#define XM_MACSEV_SEV_PORT	5556
#define XM_MACSEV_STA_PORT	8777
#define XM_MACSEV_STATU_IP  "112.124.0.188"

//define some return code
#define XM_MACSERVER_ERR		-1
#define XM_MACSERVER_SUCCESS	0
#define XM_MACSERVER_UNKOWN		2

//define some buffer len
#define XM_MACSERVER_THR_BUFLEN     3
#define XM_MACSERVER_MIN_BUFLEN		32
#define XM_MACSERVER_MED_BUFLEN     256
#define XM_MACSERVER_BIG_BUFLEN		(2*1024)


//the message format
typedef struct  _MacInfo{	
		char pFuncFlag[6];	//头标记，这个请求默认为"mac00"
		char pMyMac[18];	//"AA:BB:CC:DD:EE:FF"		
		char pSerialNo[24]; //序列号，有效位数16位
		char pBurnDate[12];	//"2015-08-19"
		char pRandom[6];	//"00000" ~ "99999"
} MacInfo;

//reply message format
typedef struct  _NewMacInfo{
		char needChange;	// '1':修改   '0':不修改
		char pNewMac[18];	//"AA:BB:CC:DD:EE:FF"
} NewmacInfo;

//the node record infomation for the msg
typedef struct _P_MacNode{
	S_int32_t fRemoteAddr;
	S_int16_t fRemotePort;
	S_int16_t fRegFlag;
	S_int8_t  fMyOldMac[XM_MACSERVER_MIN_BUFLEN];
	S_int8_t  fSerialNo[XM_MACSERVER_MIN_BUFLEN];
	S_int8_t  fEncryDate[XM_MACSERVER_MIN_BUFLEN];
	S_int8_t  fMyNewMac[XM_MACSERVER_MIN_BUFLEN];
	S_int8_t  fRemoteEncrydate[XM_MACSERVER_MIN_BUFLEN];
} P_MacNode;

//the system time 
struct SystemTime
{
	int  year;	   
	int  month;		// January = 1, February = 2, and so on.   
	int  day;	  
	int  wday;		// Sunday = 0, Monday = 1, and so on   
	int  hour;	  
	int  minute;  
	int  second;   
	int  isdst;	  
};

//the exinfo format
struct ExInfo{
	unsigned int hostip;
	unsigned short tcpport;
	unsigned short udpport;
	unsigned short httpport;
	unsigned short mobileport;
	unsigned int	msgID;
	unsigned int authorizeStat; 	//授权状态:1授权，0非法
	unsigned char mac[32];			//mac地址
	unsigned char SerialNo[64]; 	//设备序列号
	unsigned char version[64]; 		//系统版本号
	int encryptionType; 			//加密型号
	SystemTime encryptionDate;	//加密日期
	SystemTime buildDate;			//程序日期
};

//message type
typedef enum status_helper_msg_id
{
	QUERY_MULT_DEV_NAT_REQ		= 0xB0002015,//C->S
	QUERY_MULT_DEV_NAT_RSP		= 0xB0012015,//S->C
	QUERY_MULT_DEV_AUTH_REQ		= 0xB0022015,//C->S
	QUERY_MULT_DEV_AUTH_RSP		= 0xB0032015,//S->C
}STATUS_HELPER_MSG_ID;

//the return code of the status
typedef enum status_helper_ret_code
{
	AUTH_HELPER_ONLINE = 1,			
	AUTH_HELPER_ERROR = -1,			
	AUTH_HELPER_TIMEOUT = -2,		
	AUTH_HELPER_OFFLINE = -3,		
	AUTH_HELPER_MSGID_ERROR = -4,	
}STATUS_HELPER_RET_CODE;

typedef struct
{
	int Ret;
	char Uuid[100];	
	char ServerAddr[20];	//设备所在服务器地址
	int Port; 		//设备所在服务器端口
	int NatType;	//设备Nat类型
	int DevInfo;	//设备信息(是否雄迈精品)

} QueryMultDevNatRsp;


//the struct to record the device info
typedef struct _RecordDevInfo{

	char Mac_ip[17];
	int  nFlag;
	int  RandNum;
	
} RecordDevInfo;


//the new mac ip we maked
typedef struct _RecordMakeMacIp{
	char Make_MacIp[17];
} RecordMakeMacIp;


//define a class to manager the data
class xm_MacSev_CDataManager :public xm_MacServer_CThread
{
public:

	enum
	{
		DATAMANA_ERR	= 	-1,
		DATAMANA_SUC	=	0,
		DATAMANA_UKOWN	= 	100,
	};
	static xm_MacSev_CDataManager *instance();

	xm_MacSev_CDataManager();
	~xm_MacSev_CDataManager()
	{ this->datam_stopserver(); };

	S_int32_t
	datam_startserver();

	S_int32_t
	datam_stopserver();
	
	V_void_t 
	datam_mac_2autoadd();

	V_void_t 
	datam_destroy_4data();
	
protected:

	virtual V_void_t 
	thread_threadproc();

	V_void_t
	datam_loop_4data();

	S_int32_t 
	datam_initialize_4data();

	B_bool_t
	datam_mac_usable_check(S_int8_t *mac_t1, S_int8_t *mac_t2);

	S_int32_t
	datam_sleep(S_int32_t nSec);

	
public:

	//the socket for msg、status、info
	S_int32_t m_sockfd[XM_MACSERVER_THR_BUFLEN];
	struct sockaddr_in m_addr[XM_MACSERVER_THR_BUFLEN];

	//the mac address's begin and end
	S_int8_t mac_end[XM_MACSERVER_MIN_BUFLEN];
	S_int8_t mac_cur[XM_MACSERVER_MIN_BUFLEN];
	B_bool_t Is_Mac_Finish;
	B_bool_t Is_Datam_Loop;

	//the queue for msg
	std::queue<P_MacNode*> MacInfo_queue_t1;
	CMutex Mutex_MacInfo_queue_t1;
	B_bool_t t1_can_get_it;
	
	//the list for had got on cloud
	std::map<std::string, P_MacNode*> Goton_cloud_4status;
	std::map<std::string, P_MacNode*> Sendto_8777_4status;
	std::map<std::string, P_MacNode*> Sendto_tosev_4information;
	std::queue<P_MacNode*> Need_Change_Mac;
	CMutex Mutex_Goton_cloud_t3;
	CMutex Mutex_Sendto_8777_t4;
	CMutex Mutex_Sendto_tosev_t5;
	CMutex Mutex_NeedChange_Mac_t6;

	static B_bool_t Is_Initdata_Failed;
	
};

#define G_data (xm_MacSev_CDataManager::instance())

#endif // __MSGDATA_H
