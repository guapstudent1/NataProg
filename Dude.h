#pragma once

#ifdef _UNICODE
typedef std::wstring _tstring;
typedef std::wifstream _tifstream;
#else
typedef std::string _tstring;
typedef std::ifstream _tifstream;
#endif

typedef _tstring PROG;

struct DEVICE {
	TCHAR name[20];
	TCHAR alias[10];
	TCHAR fuses[8];
};
struct PORT {
	TCHAR name[10];
	TCHAR alias[15];
};
struct SPEED {
	TCHAR name[20];
	TCHAR alias[20];
};
struct FUSE {
	TCHAR name[20];
	TCHAR value[8];
};

struct DUDE {
	_tstring cmd;
	_tstring output;
};
typedef std::vector<PROG>			PROG_VEC;
typedef std::vector<struct DEVICE>	DEVICE_VEC;
typedef std::vector<struct PORT>	PORT_VEC;
typedef std::vector<struct SPEED>	SPEED_VEC;


typedef std::tuple<_tstring, struct FUSE> DEVICE_FUSE;
typedef std::tuple<PROG, struct PORT> PROG_PORT;
typedef std::vector<DEVICE_FUSE> DF_VEC;
typedef std::vector<PROG_PORT> PP_VEC;
typedef std::tuple<PROG, struct DEVICE, struct PORT, struct SPEED> PARAMETR;


INT FlashRead(LPTCH, LPTCH lpOutput, DWORD dwOutputMaxLength);
INT EepromRead(LPTCH, LPTCH lpOutput, DWORD dwOutputMaxLength);
INT FlashWrite(LPTCH, LPTCH lpOutput, DWORD dwOutputMaxLength);
INT EepromWrite(LPTCH, LPTCH lpOutput, DWORD dwOutputMaxLength);
INT FlashVerify(LPTCH, LPTCH lpOutput, DWORD dwOutputMaxLength);
INT EepromVerify(LPTCH, LPTCH lpOutput, DWORD dwOutputMaxLength);

VOID WriteToPipe(LPCTCH);
LPTCH ReadFromPipe(LPTCH lpOutStr, DWORD iMaxLength, LPDWORD pRead);
	
PROG_VEC	LoadProgrammers(int &progpos, LPCTCH progDefault);
DEVICE_VEC	LoadDevices(int& devpos, LPCTCH devDefault);
PP_VEC		LoadPorts(int& portpos, LPCTCH portDefault);
SPEED_VEC	LoadSpeed(int& speedpos, LPCTCH speedDefault);
DF_VEC		LoadFuseData();

_tstring ProgAlias(const PROG&);
_tstring MakeDudeParam(const PARAMETR&);

DWORD WINAPI AvrDudeThreadProc(CONST LPVOID lpParam);

