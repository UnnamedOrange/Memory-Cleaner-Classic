#pragma once
#include "..\TKernel\TKernel.h"
#include <Iphlpapi.h>
#include "resource.h"
#pragma comment(lib, "Iphlpapi.lib")

class NetSpeed
{
	DWORD dwSleepTime;

private:
	int64_t qwIn_Total;
	int64_t qwIn_Pre;
	int64_t qwIn_Cnt;
	int64_t qwIn_Speed;
	int64_t qwOut_Total;
	int64_t qwOut_Pre;
	int64_t qwOut_Cnt;
	int64_t qwOut_Speed;
	uint32_t dwLastTime;
	uint32_t dwTimeout;

	HANDLE hThread;
	HANDLE hEvent;

	std::set<std::string> descr;
	VOID InitAdapter()
	{
		descr.clear();
		PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO[1]; // PIP_ADAPTER_INFO结构体指针存储本机网卡信息
		unsigned long stSize = sizeof(IP_ADAPTER_INFO); // 得到结构体大小,用于GetAdaptersInfo参数
		int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize); // 调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量

		if (ERROR_BUFFER_OVERFLOW == nRel)
		{
			// 如果函数返回的是ERROR_BUFFER_OVERFLOW
			// 则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
			// 这也是说明为什么stSize既是一个输入量也是一个输出量
			delete[] pIpAdapterInfo;	// 释放原来的内存空间
			pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];	// 重新申请内存空间用来存储所有网卡信息
			nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);		// 再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
		}

		PIP_ADAPTER_INFO pIpAdapterInfoHead = pIpAdapterInfo;	// 保存pIpAdapterInfo链表中第一个元素的地址
		if (ERROR_SUCCESS == nRel)
		{
			while (pIpAdapterInfo)
			{
				descr.insert(pIpAdapterInfo->Description);
				pIpAdapterInfo = pIpAdapterInfo->Next;
			}
		}
		// 释放内存空间
		if (pIpAdapterInfoHead)
		{
			delete[] pIpAdapterInfoHead;
		}
	}

public:
	NetSpeed()
	{
		dwSleepTime = 1000;

		qwIn_Total = NULL;
		qwIn_Pre = NULL;
		qwIn_Cnt = NULL;
		qwIn_Speed = NULL;
		qwOut_Total = NULL;
		qwOut_Pre = NULL;
		qwOut_Cnt = NULL;
		qwOut_Speed = NULL;
		dwLastTime = NULL;
		dwTimeout = NULL;

		InitAdapter();

		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		tk::CreateThreadEz(ThreadRoutine, (LPVOID)this, NULL, NULL, &hThread);
	}
	~NetSpeed()
	{
		SetEvent(hEvent);
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		CloseHandle(hEvent);
	}
	int64_t GetDownloadSpeed()
	{
		if (!dwTimeout) return 0;
		return qwIn_Speed * 1000 / dwTimeout;
	} // 单位:byte
	int64_t GetDownloadTotal()
	{
		return qwIn_Total;
	} // 本次流量总量，单位byte
	int64_t GetUploadSpeed()
	{
		if (!dwTimeout) return 0;
		return qwOut_Speed * 1000 / dwTimeout;
	}
	int64_t GetUploadTotal()
	{
		return qwOut_Total;
	}

	DWORD SetSleepTime(DWORD dwTime)
	{
		if (dwTime >= 500 && dwTime <= 2000) return dwSleepTime = dwTime;
		return NULL;
	}

	DWORD OnThread()
	{
		const size_t iBaseSize = 65536;
		PMIB_IFTABLE pTable = PMIB_IFTABLE(new BYTE[iBaseSize]);
		if (!pTable)
		{
			return 0;
		}
		DWORD dwSize = 0;
		ULONG uRetCode = GetIfTable(pTable, &dwSize, TRUE);
		if (uRetCode == ERROR_NOT_SUPPORTED)
		{
			delete[] pTable;
			return uRetCode;
		}

		if (uRetCode == ERROR_INSUFFICIENT_BUFFER) //如果缓冲区不够大
		{
			delete[] pTable;
			pTable = PMIB_IFTABLE(new BYTE[dwSize + iBaseSize]);
		}

		dwLastTime = GetTickCount();
		DWORD dwWaitRet = WAIT_TIMEOUT;
		while (dwWaitRet == WAIT_TIMEOUT)
		{
			InitAdapter();
			GetIfTable(pTable, &dwSize, TRUE);

			qwIn_Cnt = 0;
			qwOut_Cnt = 0;

			// 将所有端口的流量进行统计
			for (UINT i = 0; i < pTable->dwNumEntries; i++)
			{
				const MIB_IFROW& Row = pTable->table[i];
				if (descr.count((CHAR*)Row.bDescr))
				{
					qwIn_Cnt += Row.dwInOctets;
					qwOut_Cnt += Row.dwOutOctets;
				}
			}

			if (!qwIn_Pre && !qwOut_Pre || qwIn_Cnt < qwIn_Pre || qwOut_Cnt < qwOut_Pre)
			{
				qwIn_Pre = qwIn_Cnt;
				qwOut_Pre = qwOut_Cnt;
			}

			qwIn_Speed = qwIn_Cnt - qwIn_Pre; //单位时间下载量
			qwOut_Speed = qwOut_Cnt - qwOut_Pre; //单位时间上传量
			qwIn_Total += qwIn_Speed;
			qwOut_Total += qwOut_Speed;
			dwTimeout = GetTickCount() - dwLastTime; //单位时间

			qwIn_Pre = qwIn_Cnt;
			qwOut_Pre = qwOut_Cnt;
			dwLastTime = GetTickCount();

			dwWaitRet = WaitForSingleObject(hEvent, dwSleepTime); //休眠指定时间
		}

		delete[] pTable;
		return 0;
	}
	static DWORD WINAPI ThreadRoutine(LPVOID lpClass)
	{
		return ((NetSpeed*)lpClass)->OnThread();
	}
};