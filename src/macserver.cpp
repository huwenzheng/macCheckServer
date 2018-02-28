/////////////////////////////////
//
//	@author		: huwenzheng
//	@date		: 2016/01/21
//	@purpose	: mac server main
//
/////////////////////////////////

/*
  * 该服务器基于事件驱动模式
  * 采用全异步的设计理念
  * 整个程序中类与类之间的耦合度极低
  * 体现了分段设计分段实施异步的思想
  * 为高效的处理消息奠定了基础
  */

/* 
  * 该服务器在启动的时候加401，会启动调试模式
  * 将会有大量的调试信息输出
  * 该程序运行时会产生两个日志文件，一个记录程序
  * 运行过程中出现的异常，一个记录mac地址修改的详细情况
  */ 

#include "manager.h"

B_bool_t Is_NeedDebug = false;

V_void_t 
macserver_sigcatcher(int sig, int, struct sigcontext*);

V_void_t
macserver_sigcatcher(int sig,int,struct sigcontext *)
{
	if(sig == SIGINT)
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE, 
			(S_int8_t*)"we got a SIGINT.");
    if(sig == SIGTERM)
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE, 
			(S_int8_t*)"we got a SIGTERM.");
    if(sig == SIGQUIT)
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE, 
			(S_int8_t*)"we got a SIGQUIT.");
    if(sig == SIGALRM)
		Cprintf::instance()->exception_printf((S_int8_t*)MACSEV_ERR_FILE, 
			(S_int8_t*)"we got a SIGALRM.");

	xm_MacSev_CManager::instance()->manager_stopserver();
	return;
}


S_int32_t
main(S_int32_t argc, S_int8_t **argv)
{
	//to check if need to open debug module or not
	//Is_NeedDebug = false;
	if (argc == 2 || strncmp(argv[1], "401", strlen("401") == 0))
		Is_NeedDebug = true;

	//to process signal
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = (void(*)(int))&macserver_sigcatcher;

	//to set signal, we need to do something, when we may be exit.
	(V_void_t)::sigaction(SIGINT, &act, NULL);
    (V_void_t)::sigaction(SIGTERM, &act, NULL);
    (V_void_t)::sigaction(SIGQUIT, &act, NULL);
    (V_void_t)::sigaction(SIGALRM, &act, NULL);

	//to use manager class start server
	xm_MacSev_CManager::instance()->manager_startserver();

	return 0;
}







































