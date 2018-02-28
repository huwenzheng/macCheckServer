#ifndef __LIB_EZNAT_H__
#define __LIB_EZNAT_H__

typedef struct _vv_callback
{
	void (*OnConnect)(int conn_id);					//���ӳɹ�
	void (*OnDisconnect)(int conn_id,int ret);			//���ӶϿ�
	void (*OnRecv)(int conn_id,char* data,int len);		//�յ�����
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
��ʼ��

����ֵ:

-1: ����
-2:��������ַΪ��
-3:��������ַ��Ч
-4:�˿ڲ���8765
-5:MAC��ַΪ�ջ��߻�ȡ����MAC��ַ
-6:���ļ�ʧ��
-7:��Ȩ��ʱ
-8:У���Ȩ�ļ�ʧ��
-9:��ȡ�������б�ʱ
-10:IE�����ֻ��ͻ���,uuid��Ϊ��
-11:��Ȩ���������߹�����Ч�ڡ�
-12:�����ļ�Ȩ��������û�в����ID��
1: �ɹ�
*/
int vv_nat_init(	
			char* svrip,					//������IP
			unsigned short svrport,			//�������˿�
			char* uuid,					//����UUID		, NULL ��ʾ�ɷ���������
			unsigned short local_port,		//���������˿�,���Ϊ0 ��ʾ�������
			PVVCallBack callbak,			//�ص�����
			int type                                    //0�豸�˿ͻ�, 1 IE�ͻ���,2�ֻ��ͻ���
);

/*
vv_nat_set_reconnect
�����Զ�����������Ĭ��Ϊ0	
*/
void vv_nat_set_reconnect(int times);	

/*
vv_nat_set_param
���ñ������Ӳ���

����ֵ:
0:			ʧ��
1:			�ɹ�
*/
int vv_nat_set_param(
			void*		param,				//����
			int			len					//����, ������1024�ֽ�
);

/*
vv_nat_get_param
��ѯĳ���豸�ĸ��Ӳ���

����:
uuid			�Է���UUID
buff			���ղ����Ļ����ַ
buffsize		�����С
ipaddr		����Է�ip �Ļ����ַ

����ֵ:
>0:		���յ��ĸ��Ӳ�����С		
0:		�Է����ߣ����޸��Ӳ���	
-1:		�Է�������
-2:		��ȡʧ��
-3:		ϵͳ����
-4:		�յ�<===������Ϣ���ȳ���buffsize
*/
int vv_nat_get_param_ex(
		int					timeout_sec,			//��ʱʱ�䣬��
		char*				uuid,				//uuid
		void*				buff,				//���ղ����Ļ����ַ
		int					buffsize,				//�����С
		char*				ipaddr				//����Է�IP�Ļ����ַ
);

/*
int vv_nat_connect		
��������		

����:
dest_uuid		���Ӷ���uuid
block:		0 ������ 1 ����


����ֵ:
>0:	����ID��: conn_id
0:	����ʧ��
-1:	��δ���ӷ���������������ѶϿ�
-2:	����������
*/
int vv_nat_connect(char* dest_uuid,int block);

/*
int vv_nat_send
����UDP ����

����ֵ:
>0:		�����ֽ���
0:		δ����
-1:		��Ч���Ӻ�
-2:		���ݳ��ȳ���1400 �ֽ�
*/
int vv_nat_send(int conn_id,char* data,int len);

/*
����ÿһ��slice�Ĵ�С��Ĭ��Ϊ1400
*/
int vv_nat_set_safeslice(int slicesize);

/*
int vv_nat_send
����UDP ����, ��ȫģʽ���ͣ�����ֽ���32K
����ֵ:
>0:		�����ֽ���
0:		δ����
-1:		��Ч���Ӻ�
-2:		���ݳ��ȳ���32K �ֽ�
*/
int vv_nat_safe_send(int conn_id,char* data,int len);
int vv_nat_adv_safe_send(int conn_id,char* data,int len,int head,int tail);

/*
vv_nat_get_status
��ѯ��ǰ״̬

����ֵ:
1:		״̬����
0:		δ���ӷ�����
-1:		SDKδ��ʼ��
*/
int vv_nat_get_status();

/*
�ر�����
*/
void vv_nat_close(int conn_id);

/*
�ͷ���Դ
*/
int vv_nat_exit();



void vv_nat_dump(int conn_id);

/*�������л�ȡ�Ʒ�������ʱ��*/
unsigned int  vv_nat_get_srvtime();
/*
vv_nat_get_devlist
��ѯĳ���������ϵ��豸�б���Ϣ
����ֵ: ��ǰ����˵���ص��豸�ڵ���
*/
int vv_nat_get_devlist(
		char* 				svrip,				//������IP
		unsigned short 		svrport,				//�������˿�
		int					timeout_sec,			//��ʱʱ�䣬��
		int 					sartindex,			//��ʼ���
		int					want_num,			//��Ҫ�Ľڵ���������32��
		char*				buff,				//���ղ����Ļ����ַ
		int					buffsize				//�����С
);

/*
vv_nat_get_peerinfo
��ѯĳ���������ϵ����нڵ���Ϣ
����ֵ: 
>=0:		״̬����
-1: 		ʧ��
*/
int vv_nat_get_peerinfo(
		char* 				svrip,				//������IP
		unsigned short 		svrport,				//�������˿�
		int					timeout_sec,			//��ʱʱ�䣬��
		int*					totalnum,			//�ܽڵ���
		int*					devicenum,			//�豸�ڵ���
		int*					clientnum			//�ͻ��˽ڵ���
);


//��ѯ���ӵ����ͣ���͸�ɹ��Ļ����ߵĴ���
//
//=1:		����
//=0:		��͸
//-1: 		ʧ��
//status[1]�������ơ�0 :�����ơ�1���Ʋ��ܹۿ���������
//status[2]����ʱ�䡣0:�����ơ�n:����n���ӡ�
//status[3]�������ʣ�Ŀǰ��Ϊ�ĵ���0:�����ơ�1:����ΪCIF 6֡ 100K ����������
//status[4]����λ���������䡣
int vv_nat_get_connect_type(int conn_id,int status[32]);

//geyijun @ 20131113
//==========���ӽӿ�=======================
/*
��ѯ�������б�
*/
//serverIplist �Ľṹ20  ���ֽ�һ��IP��ַ
int vv_nat_get_serverlist(
						  char* 				svrip,			//������IP
						  unsigned short 		svrport,			//�������˿�
						  int					timeout_sec,		//��ʱʱ�䣬��
						  char*				serverIplist,		//���ղ����Ļ����ַ
						  int*					getNum			//�����С
						  );

 /*
 ��ѯuuid �Ƿ��ڷ�������
*/		
//=0:		����
//-1: 		�����߻������쳣
int vv_nat_check_uuid_exist(				
							int					timeout_sec,		//��ʱʱ�䣬��
							char*				uuid
							);

 /*
 ��ѯ���uuid �Ƿ��ڷ�������,״̬��ͨ���ṹ����صġ�
*/
//=0:		
//-1: 		�ͻ����쳣
typedef int (*OnFoundDevCB)(char * uuid,int state,int userData);
int vv_nat_check_multi_uuids_exist(int timeout_sec,Device_Info *dev_info,OnFoundDevCB findDevCB, int userData);
int vv_nat_check_multi_uuids_exist_V2(int timeout_sec,Device_Info *dev_info,OnFoundDevCB findDevCB, int userData);
/*
��ѯһ̨NAT SVR��IP  ��ַ
*/
//=0:		�ɹ�
//-1: 		ʧ��
//natIp 20  ���ֽ�
int vv_dns_get_natsvr(
					  char* 				dnssvrip,			//DNS_������IP
					  unsigned short 		dnssvrport,		//DNS_�������˿�
					  int					timeout_sec,		//��ʱʱ�䣬��
					  char*				natIp			//�õ���IP  ��ַ
					  );
//zhoucb @ 20140310 
//���ӻ���ʹ�ܿ���
int vv_enable_connect_cache(int  enable);
//��ѯ�豸�������绷����nat���͡�
//=0 �Գ��ͣ�=1��ȫԲ׶�ͣ�=2��ַ�����ͣ�=3�˿������ͣ�
//-1 nat����쳣��-2 :�������б�����������̨���޷�֪������nat���͡�
int vv_get_nat_type();

//��ѯԶ���豸�������绷����nat���͡�
//=0 �Գ��ͣ�=1��ȫԲ׶�ͣ�=2��ַ�����ͣ�=3�˿������ͣ�
//-1nat����쳣���߲�ѯ��ʱ ��-2 :���豸δ֪����
int vv_get_remote_nat_type(char *uuid);

//���ӳ�ʼ�����������豸�����еĴ���ŵķ���
int  vv_nat_get_last_error(char *errinfo,int size,int language);

//Ӧ����ˮҪ�����ӻ�ȡ��İ汾�Žӿ�
void vv_nat_get_version(char *version,int size);

//����ǿ����ת���Ľӿڣ�skipP2P=1  ǿ��ת����skipP2P=0 ���������������̡�
void vv_nat_force_skip_p2p(int skipP2P);

//�˽ӿ���Ҫ��vv_nat_connet()����ſ��Ե���
int	vv_nat_is_family_series();
#ifdef __cplusplus 
}
#endif

#endif//__LIB_EZNAT_H__

