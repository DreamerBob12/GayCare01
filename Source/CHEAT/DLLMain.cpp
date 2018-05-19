#define _CRT_SECURE_NO_WARNINGS

//#include "DLLMain.h"
#include "Utilities.h"
#include "INJ/ReflectiveLoader.h"
#include "AntiAntiAim.h"
#include "Offsets.h"
#include "Interfaces.h"
#include "Hooks.h"
#include "RenderManager.h"
#include "Hacks.h"
#include "Menu.h"
#include "AntiAntiAim.h"
#include "hitmarker.h"
#include "AntiLeak.h"

#include "Dumping.h"




bool toggleSideSwitch;
int ResolverStage[65];
int missedLogHits[65];
float enemyLBYDelta[65];

float lineLBY;
float lineRealAngle;
float lineFakeAngle;

template <int XORSTART, int BUFLEN, int XREFKILLER>

class Xor
{
private:
	Xor();
public:
	char s[BUFLEN];

	Xor(const char* xs);
	~Xor()
	{
		for (int i = 0; i < BUFLEN; i++) s[i] = 0;
	}
};

template <int XORSTART, int BUFLEN, int XREFKILLER>

Xor<XORSTART, BUFLEN, XREFKILLER>::Xor(const char* xs)
{
	int xvalue = XORSTART;
	int i = 0;
	for (; i < (BUFLEN - 1); i++)
	{
		s[i] = xs[i - XREFKILLER] ^ xvalue;
		xvalue += 1;
		xvalue %= 256;
	}
	s[BUFLEN - 1] = (2 * 2 - 3) - 1;
}

extern HINSTANCE hAppInstance;

UCHAR szFileSys[255], szVolNameBuff[255];
DWORD dwMFL, dwSysFlags;
DWORD dwSerial;
LPCTSTR szHD = "C:\\";

bool ErasePEHeader(HMODULE hModule)
{
	hModule = GetModuleHandle(NULL);
	if ((DWORD)hModule == 0) return 0;
	DWORD IMAGE_NT_HEADER = *(int*)((DWORD)hModule + 60);
	for (int i = 0; i<0x108; i++)
		*(BYTE*)(IMAGE_NT_HEADER + i) = 0;
	for (int i = 0; i<120; i++)
		*(BYTE*)((DWORD)hModule + i) = 0;
	return 1;
}

HINSTANCE HThisModule;
bool DoUnload;


void create_folder(const char * path) {
	if (!CreateDirectory(path, NULL)) {
		return;
	}
}

void initialize_configs_file() {

	create_folder("yeti");
	create_folder("yeti\\cfg");

}



int InitialThread()
{

	Sleep(150);

	PrintMetaHeader();

	Offsets::Initialise();
	
	Interfaces::Initialise(); 
	Utilities::Log("[GayCare] Systems Awake");
	NetVar.RetrieveClasses();
	NetvarManager::Instance()->CreateDatabase();
	Render::Initialise();
	hitmarker::singleton()->initialize();
	Hacks::SetupHacks();
	Menu::SetupMenu();
	Hooks::Initialise();
	SkinChanger();

	Dump::DumpClassIds();


	

	while (DoUnload == false)
	{
		Sleep(1000);
	}

	Hooks::UndoHooks();
	Sleep(2000); 
	FreeLibraryAndExitThread(HThisModule, 0);

	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		GetVolumeInformation(szHD, (LPTSTR)szVolNameBuff, 255, &dwSerial, &dwMFL, &dwSysFlags, (LPTSTR)szFileSys, 255);
		Sleep(100);
		{
			{
				DisableThreadLibraryCalls(hModule);

				CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)InitialThread, NULL, NULL, NULL);

				return TRUE;
			}
		}
		return FALSE;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
	}
	return TRUE;
}