#include "framework.h"
#include "Dude.h"
#include <fstream>
// 
HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

VOID WriteToPipe(LPCTCH lpStr)
{
	//DWORD dwWritten;
	CHAR chBuf[4096] = { 0 };

	//std::string s = ws2s(str);

	//WriteFile(g_hChildStd_IN_Wr, s.c_str(), s.size(), &dwWritten, NULL);

	CloseHandle(g_hChildStd_IN_Wr);
}
LPTCH ReadFromPipe(LPTCH lpOutStr, DWORD iMaxLength, LPDWORD pRead = NULL)
{
	DWORD dwRead;
	CHAR* chBuf = new CHAR[iMaxLength]{ 0 };

	ReadFile(g_hChildStd_OUT_Rd, chBuf, iMaxLength, &dwRead, NULL);
		
	DWORD size_needed = MultiByteToWideChar(CP_ACP, 0, chBuf, dwRead, NULL, 0);
	
	if (iMaxLength < size_needed) 
	{
		delete[] chBuf;
		return NULL; // 
	}		

	MultiByteToWideChar(CP_ACP, 0, chBuf, dwRead, lpOutStr, size_needed);
	if (pRead != NULL)
	{
		*pRead = dwRead;
	}
	delete[] chBuf;

	return lpOutStr;
}
DWORD WINAPI AvrDudeThreadProc(CONST LPVOID lpParam)
{
	// 
	DUDE* dude = reinterpret_cast<DUDE*>(lpParam);
	SECURITY_ATTRIBUTES saAttr;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		ExitThread(-1);

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		ExitThread(-1);

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		ExitThread(-1);

	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		ExitThread(-1);

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	bSuccess = CreateProcess(NULL,
		(LPTCH)dude->cmd.c_str(),
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,  // Рабочая папка 
		&siStartInfo,
		&piProcInfo);

	if (!bSuccess)
		ExitThread(-1);
	else
	{
		WaitForSingleObject(piProcInfo.hProcess, INFINITE);

		TCHAR chBuf[4096] = { 0 };
		DWORD dwRead = 0;
		ReadFromPipe(chBuf, sizeof(chBuf) / sizeof(TCHAR), &dwRead);
		dude->output = chBuf;		
	}
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);

	ExitThread(0);
}
INT AvrDudeExecute(DUDE *dude)
{
	// 
	//CreateThread(NULL, 0, &AvrDudeThreadProc, &dude, 0, NULL);
	SECURITY_ATTRIBUTES saAttr;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		return -1;

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return -1;

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		return -1;

	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		return -1;

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	bSuccess = CreateProcess(NULL,
		(LPTCH)dude->cmd.c_str(),
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,  // Рабочая папка
		&siStartInfo,
		&piProcInfo);

	if (!bSuccess)
		return -1;
	else
	{
		WaitForSingleObject(piProcInfo.hProcess, INFINITE);

		TCHAR chBuf[4096] = { 0 };
		DWORD dwRead = 0;
		ReadFromPipe(chBuf, sizeof(chBuf) / sizeof(TCHAR), &dwRead);
		dude->output = chBuf;
	}
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);	
	return 0;
}
INT FlashRead(LPTCH lpCmdline, LPTCH lpOutput, DWORD dwOutputMaxLength)
{
	DUDE dude = { lpCmdline, _T("") };
	INT result = AvrDudeExecute(&dude);
	
	_tcscpy_s(lpOutput, dwOutputMaxLength, dude.output.c_str());
	return result;
}
INT EepromRead(LPTCH lpCmdline, LPTCH lpOutput, DWORD dwOutputMaxLength)
{
	DUDE dude = { lpCmdline, _T("") };
	INT result = AvrDudeExecute(&dude);

	_tcscpy_s(lpOutput, dwOutputMaxLength, dude.output.c_str());
	return result;
}

INT FlashWrite(LPTCH lpCmdline, LPTCH lpOutput, DWORD dwOutputMaxLength)
{
	DUDE dude = { lpCmdline, _T("") };
	INT result = AvrDudeExecute(&dude);

	_tcscpy_s(lpOutput, dwOutputMaxLength, dude.output.c_str());
	return result;
}
INT EepromWrite(LPTCH lpCmdline, LPTCH lpOutput, DWORD dwOutputMaxLength)
{
	DUDE dude = { lpCmdline, _T("") };
	INT result = AvrDudeExecute(&dude);

	_tcscpy_s(lpOutput, dwOutputMaxLength, dude.output.c_str());
	return result;
}
INT FlashVerify(LPTCH lpCmdline, LPTCH lpOutput, DWORD dwOutputMaxLength)
{
	DUDE dude = { lpCmdline, _T("") };
	INT result = AvrDudeExecute(&dude);

	_tcscpy_s(lpOutput, dwOutputMaxLength, dude.output.c_str());
	return result;
}
INT EepromVerify(LPTCH lpCmdline, LPTCH lpOutput, DWORD dwOutputMaxLength)
{
	DUDE dude = { lpCmdline, _T("") };
	INT result = AvrDudeExecute(&dude);

	_tcscpy_s(lpOutput, dwOutputMaxLength, dude.output.c_str());
	return result;
}
PROG_VEC LoadProgrammers(int &progpos, LPCTCH progDefault)
{
	
	PROG_VEC result;
	TCHAR tmp[24];	
	_tifstream progfile("Programmer.txt");

	if (progfile.is_open())
	{
		for (int i=0; !progfile.eof(); i++)
		{
			progfile.getline(tmp, 24);
			result.push_back(tmp);

			if (!_tcsicmp(tmp, progDefault))
			{
				progpos = i;
			}						
		}
		progfile.close();
	}
	
	return result;
}

DEVICE_VEC LoadDevices(int& devpos, LPCTCH devDefault)
{
	DEVICE_VEC result;
	TCHAR tmp[100];

	_tifstream progfile("Device.txt");

	if (progfile.is_open())
	{
		for (int i = 0; !progfile.eof(); i++)
		{
			DEVICE dtmp = { 0 };
			progfile.getline(tmp, sizeof(tmp)/sizeof(TCHAR));
			_stscanf_s(tmp, _T("%s%s%s"), dtmp.name, 20,  dtmp.alias, 10, dtmp.fuses, 8);
			

			if (!_tcsicmp(dtmp.name, devDefault))
			{
				devpos = i;
			}
			if (_tcslen(dtmp.name) > 0 && _tcslen(dtmp.alias) > 0 && _tcslen(dtmp.fuses) == 6)
				result.push_back(dtmp);
		}
		progfile.close();
	}
	return result;
}

PP_VEC LoadPorts(int& portpos, LPCTCH portDefault)
{
	PP_VEC result;
	TCHAR tmp[100];

	_tifstream portfile("Port.txt");

	if (!portfile.is_open()) return result;

	while (!portfile.eof()) 
	{
		portfile.getline(tmp, 100);
		// 
		TCHAR* context = NULL;
		PROG_VEC vProg;
		TCHAR* pch = _tcstok_s(tmp, _T("."), &context);

		while (pch)
		{
			vProg.push_back(pch);
			pch = _tcstok_s(NULL, _T("."), &context);
		}

		size_t len;
		bool has_start = false;
		bool has_stop = false;

		do
		{
			PORT dtmp = { 0 };
			// 
			portfile.getline(tmp, 100);
			len = _tcslen(tmp);

			pch = tmp;
			if (!has_start && tmp[0] == _T('[')) // 
			{
				has_start = true;
				pch = tmp + 1;
				len--;
			}
			if (len > 0 && has_start && !has_stop)
			{
				if (pch[len - 1] == ']')// 
				{
					pch[len - 1] = 0;
					//len--;
					has_stop = true;
				}
			}
			else
			{
				break;
			}
			if (has_start && len > 1)
			{
				//
				context = NULL;
				pch = _tcstok_s(pch, _T("\t"), &context);
				_tcscpy_s(dtmp.name, 10, pch);
				pch = _tcstok_s(NULL, _T("\t"), &context);
				_tcscpy_s(dtmp.alias, 15, pch);

				for (auto dev : vProg)
				{
					PROG_PORT dftmp(dev, dtmp);
					result.push_back(dftmp);
				}
			}
		} while (len > 0);
	}
	portfile.close();
	/*PORT ptmp[] = { { _T("Default"), _T("x") }, { _T("COM1"), _T("com1") }, { _T("COM2"), _T("com2") },
		{ _T("COM3"), _T("com3") },{ _T("COM4"), _T("com4") }, { _T("COM6"), _T("com6") },
		{ _T("LPT1"), _T("lpt1") },{ _T("USB"), _T("usb") }
	};
	for (int i = 0; i < 8; i++)
	{
		result.push_back(ptmp[i]);
	}*/
	portpos = 0;
	return result;
}

SPEED_VEC LoadSpeed(int& speedpos, LPCTCH speedDefault)
{
	SPEED_VEC result;
	TCHAR tmp[100];
	_tifstream progfile("Speed.txt");

	if (progfile.is_open())
	{
		for (int i = 0; !progfile.eof(); i++)
		{
			SPEED dtmp = { 0 };

			progfile.getline(tmp, sizeof(tmp) / sizeof(TCHAR));
			
			TCHAR* context = NULL;
			TCHAR* pch = _tcstok_s(tmp, _T("\t"), &context);
			_tcscpy_s(dtmp.name, 20, tmp);
			pch = _tcstok_s(NULL, _T("\t"), &context);
			_tcscpy_s(dtmp.alias, 20, pch);


			if (!_tcsicmp(dtmp.name, speedDefault))
			{
				speedpos = i;
			}
			if (_tcslen(dtmp.name) > 0 && _tcslen(dtmp.alias) > 0)
				result.push_back(dtmp);
		}
		progfile.close();
	}
	return result;
}
DF_VEC LoadFuseData()
{
	DF_VEC result;
	TCHAR tmp[100];
	_tifstream fusefile("Fuse.txt");
	// 
	// 
	if (!fusefile.is_open()) return result;
	
	while(!fusefile.eof()) 
	{
		fusefile.getline(tmp, 100);
		// 
		TCHAR* context = NULL;
		PROG_VEC vDev;
		TCHAR* pch = _tcstok_s(tmp, _T("."), &context);
		while (pch)
		{
			vDev.push_back(pch);
			pch = _tcstok_s(NULL, _T("."), &context);
		}

		size_t len;
		bool has_start = false;
		bool has_stop = false;

		do 
		{
			FUSE dtmp = { 0 };
			//
			fusefile.getline(tmp, 100);
			len = _tcslen(tmp);

			pch = tmp;
			if (!has_start && tmp[0] == _T('[')) //
			{
				has_start = true;
				pch = tmp + 1;
				len--;
			}
			if (len > 0 && has_start && !has_stop) 
			{
				if (pch[len - 1] == ']')// 
				{
					pch[len - 1] = 0;
					//len--;
					has_stop = true;
				}
			}
			else
			{
				break;
			}
			if (has_start && len > 1)
			{
				// 
				context = NULL;
				pch = _tcstok_s(pch, _T("\t"), &context);
				_tcscpy_s(dtmp.name, 20, pch);
				pch = _tcstok_s(NULL, _T("\t"), &context);
				_tcscpy_s(dtmp.value, 8, pch);
					
				for (auto dev : vDev)
				{
					DEVICE_FUSE dftmp(dev,dtmp);
					result.push_back(dftmp);
				}
			}
		} while (len > 0);		
	}		
	fusefile.close();
	
	return result;
}
_tstring ProgAlias(const PROG &name)
{
	_tstring res;
	for (auto c : name)
	{
		if (c != _T(' ')) res += tolower(c);
	}
	return res;
}
_tstring MakeDudeParam(const PARAMETR& param)
{
	_tstring prog   = ProgAlias(std::get<0>(param));
	_tstring device = std::get<1>(param).alias;
	_tstring port   = std::get<2>(param).alias;
	_tstring speed  = std::get<3>(param).alias;

	_tstring progKey = _T("-c ") + prog;
	_tstring devKey  = _T("-p ") + device;
	_tstring portKey = _T("-P ");
	_tstring speedPort;

	// 
	if (port != _T("x"))
	{
		speedPort = portKey + port;
		if (speed != _T("-b x"))
		{
			speedPort += _T(" ");
			speedPort += speed;
		}
	}
	else {
		if (speed != _T("-b x"))
		{
			speedPort += speed;
		}
	}		
	return progKey + _T(" ") + speedPort + _T(" ") + devKey;
}