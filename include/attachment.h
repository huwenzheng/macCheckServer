////////////////////////////////////
//
//	@auther		:	huwenzheng
//	@data		:	2016/01/15
//	@purpose	:	some classes we will use in this project 
//
////////////////////////////////////

#ifndef	__ATTACHMENT_H__
#define __ATTACHMENT_H__

#include "commUint.h"
using namespace std;

//this class had realization the simple semaphores
class CSemaphore
{
public:
	inline CSemaphore(U_int32_t SeInitialCount = 0)
	{
		sem_init(&fSemphore, 0, SeInitialCount);
	};

	inline ~CSemaphore()
	{
		sem_destroy(&fSemphore);
	};

	inline S_int32_t Sem_Wait()
	{
		return sem_wait(&fSemphore);
	};

	inline S_int32_t Sem_Post()
	{
		return sem_post(&fSemphore);
	};
	
protected:
private:
	sem_t	fSemphore;
};

//the realization of the simple mutex
class CMutex
{
public:
	inline CMutex()
	{
		pthread_mutex_init(&fMutex, NULL);
	};

	inline ~CMutex()
	{
		pthread_mutex_destroy(&fMutex);
	};

	inline S_int32_t Get_lock()
	{
		return pthread_mutex_lock(&fMutex);
	};

	inline S_int32_t Leave_lock()
	{
		return pthread_mutex_unlock(&fMutex);
	};

protected:
private:
	pthread_mutex_t fMutex;
};

//the realization of the simple time
class Ctime
{
public:

	static Ctime* instance() 
	{
		static Ctime *_instance = NULL;
		if (_instance == NULL) 
		{
			_instance = new Ctime();
		}

		return _instance;
	};

	Ctime()
	{
		tm = 0;
	};

	inline S_int32_t Get_Second()
	{
		tm = time(NULL);
		return tm;
	};

	inline S_int8_t *Get_Ym()
	{
		memset(date, 0x00, sizeof(date));

		struct tm *local;
		tm = time(NULL);
		local = localtime(&tm);

		sprintf(date, "%04d_%02d", 
			local->tm_year+1900, 
			local->tm_mon+1);
		return date;
	};

	inline S_int8_t *Get_Ymm_Hms()
	{
		memset(date, 0x00, sizeof(date));

		struct tm *local;
		tm = time(NULL);
		local = localtime(&tm);

		sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d",
			local->tm_year+1900,
			local->tm_mon+1,
			local->tm_mday, 
			local->tm_hour,
			local->tm_min,
			local->tm_sec);
		return date;
	};

	inline S_int8_t *Get_gmt_time()
	{
		memset(date, 0x00, sizeof(date));

		time_t now = time(NULL);
		strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));

		return date;
	};
protected:
private:
	
	time_t tm;
	S_int8_t date[64];
};

//the realization of the printf class
class Cprintf
{
public:
	static Cprintf* instance() 
	{
		static Cprintf *_instance = NULL;
		if (_instance == NULL) 
		{
			_instance = new Cprintf();
		}

		return _instance;
	};

	inline V_void_t exception_printf(S_int8_t* p_FileName, S_int8_t* p_Exception)
	{
		if (p_FileName == NULL || p_Exception == NULL)
			return;

		Ctime TT_time;
		std::string str_printf(TT_time.Get_Ymm_Hms());
		str_printf += '\n';
		str_printf += p_Exception;
		str_printf += '\n';

		FILE *p_file = fopen(p_FileName, "a+");
		if (p_file)
			fwrite(str_printf.c_str(), sizeof(char), str_printf.length(), p_file);
		fclose(p_file);
		p_file = NULL;

		printf("%s\n", p_Exception);
	};
protected:
private:
};

#endif //__ATTACHMENT_H__




























