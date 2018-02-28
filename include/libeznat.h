#ifndef __LIB_EZNAT_H__
#define __LIB_EZNAT_H__

typedef struct _vv_callback
{
	void (*OnConnect)(int conn_id);					//连接成功
	void (*OnDisconnect)(int conn_id,int ret);			//连接断开
	void (*OnRecv)(int conn_id,char* data,int len);		//收到数据
	int 	resv[5];
}TVVCallBack,*PVVCallBack;


typedef struct DEVICE_INFO
{
	char uuid[32];
	int state;
}Device_Info;


#ifdef __cplusplus
extern "C"
{
#endif

/*
vv_nat_init
初始化

返回值:

-1: 其他
-2:服务器地址为空
-3:服务器地址无效
-4:端口不是8765
-5:MAC地址为空或者获取不到MAC地址
-6:打开文件失败
-7:鉴权超时
-8:校验鉴权文件失败
-9:获取服务器列表超时
-10:IE或者手机客户端,uuid不为空
-11:鉴权点数满或者过了有效期。
-12:所发的鉴权服务器上没有部署此ID。
1: 成功
*/
int vv_nat_init(	
			char* svrip,					//服务器IP
			unsigned short svrport,			//服务器端口
			char* uuid,					//本机UUID		, NULL 表示由服务器分配
			unsigned short local_port,		//本机监听端口,如果为0 表示随机分配
			PVVCallBack callbak,			//回调函数
			int type                                    //0设备端客户, 1 IE客户度,2手机客户端
);

/*
vv_nat_set_reconnect
设置自动重连次数，默认为0	
*/
void vv_nat_set_reconnect(int times);	

/*
vv_nat_set_param
设置本机附加参数

返回值:
0:			失败
1:			成功
*/
int vv_nat_set_param(
			void*		param,				//参数
			int			len					//长度, 不超过1024字节
);

/*
vv_nat_get_param
查询某个设备的附加参数

参数:
uuid			对方的UUID
buff			接收参数的缓存地址
buffsize		缓存大小
ipaddr		保存对方ip 的缓存地址

返回值:
>0:		接收到的附加参数大小		
0:		对方在线，但无附加参数	
-1:		对方不在线
-2:		获取失败
-3:		系统错误
-4:		收到<===附加信息长度超过buffsize
*/
int vv_nat_get_param_ex(
		int					timeout_sec,			//超时时间，秒
		char*				uuid,				//uuid
		void*				buff,				//接收参数的缓存地址
		int					buffsize,				//缓存大小
		char*				ipaddr				//保存对方IP的缓存地址
);

/*
int vv_nat_connect		
发起连接		

参数:
dest_uuid		连接对象uuid
block:		0 非阻塞 1 阻塞


返回值:
>0:	连接ID号: conn_id
0:	连接失败
-1:	尚未连接服务器或与服务器已断开
-2:	连接数已满
*/
int vv_nat_connect(char* dest_uuid,int block);

/*
int vv_nat_send
发送UDP 数据

返回值:
>0:		发送字节数
0:		未连接
-1:		无效连接号
-2:		数据长度超出1400 字节
*/
int vv_nat_send(int conn_id,char* data,int len);

/*
设置每一个slice的大小，默认为1400
*/
int vv_nat_set_safeslice(int slicesize);

/*
int vv_nat_send
发送UDP 数据, 安全模式发送，最大字节数32K
返回值:
>0:		发送字节数
0:		未连接
-1:		无效连接号
-2:		数据长度超出32K 字节
*/
int vv_nat_safe_send(int conn_id,char* data,int len);
int vv_nat_adv_safe_send(int conn_id,char* data,int len,int head,int tail);

/*
vv_nat_get_status
查询当前状态

返回值:
1:		状态正常
0:		未连接服务器
-1:		SDK未初始化
*/
int vv_nat_get_status();

/*
关闭连接
*/
void vv_nat_close(int conn_id);

/*
释放资源
*/
int vv_nat_exit();



void vv_nat_dump(int conn_id);

/*从心跳中获取云服务器的时间*/
unsigned int  vv_nat_get_srvtime();
/*
vv_nat_get_devlist
查询某个服务器上的设备列表信息
返回值: 当前命令说返回的设备节点数
*/
int vv_nat_get_devlist(
		char* 				svrip,				//服务器IP
		unsigned short 		svrport,				//服务器端口
		int					timeout_sec,			//超时时间，秒
		int 					sartindex,			//起始编号
		int					want_num,			//想要的节点个数，最大32个
		char*				buff,				//接收参数的缓存地址
		int					buffsize				//缓存大小
);

/*
vv_nat_get_peerinfo
查询某个服务器上的所有节点信息
返回值: 
>=0:		状态正常
-1: 		失败
*/
int vv_nat_get_peerinfo(
		char* 				svrip,				//服务器IP
		unsigned short 		svrport,				//服务器端口
		int					timeout_sec,			//超时时间，秒
		int*					totalnum,			//总节点数
		int*					devicenum,			//设备节点数
		int*					clientnum			//客户端节点数
);


//查询连接的类型，穿透成功的还是走的代理
//
//=1:		代理
//=0:		穿透
//-1: 		失败
//status[1]码流限制。0 :不限制。1限制不能观看主码流。
//status[2]限制时间。0:不限制。n:限制n分钟。
//status[3]限制码率，目前分为四档。0:不限制。1:限制为CIF 6帧 100K ，后续待定
//status[4]保留位，后续扩充。
int vv_nat_get_connect_type(int conn_id,int status[32]);

//geyijun @ 20131113
//==========增加接口=======================
/*
查询服务器列表
*/
//serverIplist 的结构20  个字节一个IP地址
int vv_nat_get_serverlist(
						  char* 				svrip,			//服务器IP
						  unsigned short 		svrport,			//服务器端口
						  int					timeout_sec,		//超时时间，秒
						  char*				serverIplist,		//接收参数的缓存地址
						  int*					getNum			//缓存大小
						  );

 /*
 查询uuid 是否在服务器上
*/		
//=0:		在线
//-1: 		不在线或其他异常
int vv_nat_check_uuid_exist(				
							int					timeout_sec,		//超时时间，秒
							char*				uuid
							);

 /*
 查询许多uuid 是否在服务器上,状态是通过结构体带回的。
*/
//=0:		
//-1: 		客户端异常
typedef int (*OnFoundDevCB)(char * uuid,int state,int userData);
int vv_nat_check_multi_uuids_exist(int timeout_sec,Device_Info *dev_info,OnFoundDevCB findDevCB, int userData);
int vv_nat_check_multi_uuids_exist_V2(int timeout_sec,Device_Info *dev_info,OnFoundDevCB findDevCB, int userData);
/*
查询一台NAT SVR的IP  地址
*/
//=0:		成功
//-1: 		失败
//natIp 20  个字节
int vv_dns_get_natsvr(
					  char* 				dnssvrip,			//DNS_服务器IP
					  unsigned short 		dnssvrport,		//DNS_服务器端口
					  int					timeout_sec,		//超时时间，秒
					  char*				natIp			//得到的IP  地址
					  );
//zhoucb @ 20140310 
//连接缓冲使能开关
int vv_enable_connect_cache(int  enable);
//查询设备所处网络环境的nat类型。
//=0 对称型；=1完全圆锥型；=2地址限制型；=3端口限制型；
//-1 nat检测异常，-2 :服务器列表里面少于两台，无法知道自身nat类型。
int vv_get_nat_type();

//查询远端设备所处网络环境的nat类型。
//=0 对称型；=1完全圆锥型；=2地址限制型；=3端口限制型；
//-1nat检测异常或者查询超时 ，-2 :老设备未知类型
int vv_get_remote_nat_type(char *uuid);

//增加初始化或者连接设备过程中的错误号的返回
int  vv_nat_get_last_error(char *errinfo,int size,int language);

//应黄万水要求增加获取库的版本号接口
void vv_nat_get_version(char *version,int size);

//增加强制走转发的接口；skipP2P=1  强制转发。skipP2P=0 按正常检测机制流程。
void vv_nat_force_skip_p2p(int skipP2P);

//此接口需要等vv_nat_connet()掉完才可以调。
int	vv_nat_is_family_series();
#ifdef __cplusplus 
}
#endif

#endif//__LIB_EZNAT_H__

