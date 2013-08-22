#include "Main.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;

	// Get the location of the "shell" key
	DWORD dwRegType = REG_SZ, dwRegSize = MAX_PATH;
	LoadString (GetModuleHandle(NULL), IDS_32BITINI, g_sz32bitIni, MAX_PATH);
	LoadString (GetModuleHandle(NULL), IDS_64BITINI, g_sz64bitIni, MAX_PATH);
	//SHGetValue(HKEY_LOCAL_MACHINE, g_sz32bitIni, TEXT("shell"), &dwRegType, &g_sz32bitshellLocation, &dwRegSize);
	HRESULT hTest = SHGetValue(HKEY_LOCAL_MACHINE, g_sz32bitIni, TEXT("shell"), &dwRegType, g_sz32bitshellLocation, &dwRegSize);

	SHGetValue(HKEY_LOCAL_MACHINE, g_sz64bitIni, TEXT("shell"), &dwRegType, g_sz64bitshellLocation, &dwRegSize);

	// Get the location of the shell
	_tcscpy_s(g_szShellLocation, MAX_PATH, TEXT(""));
	LPTSTR pszKey = g_sz64bitshellLocation + 4;
	TCHAR szIniMappingDif[1024], szIniMappingDifTitle[256];
	LoadString (GetModuleHandle(NULL), IDS_INIMAPPINGDIFF, szIniMappingDif, 1024);
	LoadString (GetModuleHandle(NULL), IDS_INIMAPPINGDIFFTITLE, szIniMappingDifTitle, 1024);
	if (_tcscmp(g_sz32bitshellLocation, g_sz64bitshellLocation) != 0)
		MessageBox(NULL, szIniMappingDif, szIniMappingDifTitle, MB_OK);
	else if (g_sz64bitshellLocation[0] == 'S')
		SHGetValue(HKEY_LOCAL_MACHINE, pszKey, TEXT("shell"), &dwRegType, g_szShellLocation, &dwRegSize);
	else
		SHGetValue(HKEY_CURRENT_USER, pszKey, TEXT("shell"), &dwRegType, g_szShellLocation, &dwRegSize);

	// Show the main window
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc);
	return 0;
}

void SetShell(const TCHAR* pszShell, bool bPerUser)
{
	TCHAR szShellLoc[MAX_PATH], szIniMapping[MAX_PATH];
	LoadString (GetModuleHandle(NULL), IDS_SHELLKEY, szShellLoc, MAX_PATH);
	if (bPerUser)
	{
		_tcscpy_s(szIniMapping, MAX_PATH, TEXT("USR:"));
		_tcscat_s(szIniMapping, MAX_PATH, szShellLoc);
		SHSetValue(HKEY_LOCAL_MACHINE, g_sz32bitIni, TEXT("shell"), REG_SZ, szIniMapping, MAX_PATH);
		SHSetValue(HKEY_LOCAL_MACHINE, g_sz64bitIni, TEXT("shell"), REG_SZ, szIniMapping, MAX_PATH);
		SHSetValue(HKEY_CURRENT_USER, szShellLoc, TEXT("shell"), REG_SZ, pszShell, MAX_PATH);
	}
	else
	{
		_tcscpy_s(szIniMapping, MAX_PATH, TEXT("SYS:"));
		_tcscat_s(szIniMapping, MAX_PATH, szShellLoc);
		SHSetValue(HKEY_LOCAL_MACHINE, g_sz32bitIni, TEXT("shell"), REG_SZ, szIniMapping, MAX_PATH);
		SHSetValue(HKEY_LOCAL_MACHINE, g_sz64bitIni, TEXT("shell"), REG_SZ, szIniMapping, MAX_PATH);
		SHSetValue(HKEY_LOCAL_MACHINE, szShellLoc, TEXT("shell"), REG_SZ, pszShell, MAX_PATH);
	}
}

bool file_exists(const TCHAR* szFilename)
{
	FILE* file;
	_tfopen_s(&file, szFilename, TEXT("r"));
	if (file)
	{
		fclose(file);
		return true;
	}
	return false;
}

void Reboot()
{
	LUID luid;
	TOKEN_PRIVILEGES privs;
	HANDLE token;

	OpenProcessToken (GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token);
	LookupPrivilegeValue (NULL, TEXT("SeShutdownPrivilege"), &luid);

	privs.PrivilegeCount = 1;
	privs.Privileges[0].Luid = luid;
	privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	AdjustTokenPrivileges (token, FALSE, &privs, 0, NULL, NULL);
	ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_MINOR_RECONFIG);
}

void AskReboot(bool bActuallyAsk)
{
	if (bActuallyAsk)
	{
		TCHAR szText[512], szTitle[128];
		LoadString (GetModuleHandle(NULL), IDS_REBOOTMSG, szText, 512);
		LoadString (GetModuleHandle(NULL), IDS_REBOOTTITLE, szTitle, 128);
		if (MessageBox(NULL, szText, szTitle, MB_YESNO) == IDYES)
			Reboot();
	}
	else
	{
		Reboot();
	}
}

INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;
	TCHAR szShell[MAX_PATH];
	switch (msg)
	{
		case WM_INITDIALOG:
			hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
			if(hIcon)
				SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			CheckDlgButton(hwnd, IDPROMPTREBOOT, BST_CHECKED);
			
			//TODO::Fix the "Use the same shell for all user accounts" checkbox and show it again.
			//if (g_sz64bitshellLocation[0] == 'S')
			//	CheckDlgButton(hwnd, IDPERUSER, BST_CHECKED);

			Edit_SetText(GetDlgItem(hwnd, IDC_EDIT1), g_szShellLocation);
			
			return TRUE;

		case WM_COMMAND:
				switch(LOWORD(wParam))
				{
					case IDCUSTOM:
						Edit_GetText(GetDlgItem(hwnd, IDC_EDIT1), (LPWSTR)szShell, MAX_PATH);
						if (file_exists(szShell))
						{
							SetShell(szShell, IsDlgButtonChecked(hwnd, IDPERUSER) == BST_UNCHECKED);
							AskReboot(IsDlgButtonChecked(hwnd, IDPROMPTREBOOT) == BST_CHECKED);
							EndDialog(hwnd, IDCUSTOM);
						}
						else
						{
							TCHAR szText[512], szTitle[128];
							LoadString (GetModuleHandle(NULL), IDS_INVALIDPATH, szText, 512);
							LoadString (GetModuleHandle(NULL), IDS_ERROR, szTitle, 128);
							MessageBox(NULL, szText, szTitle, MB_OK);
						}
						break;

					case IDEXPLORER:
						SetShell(TEXT("explorer.exe"), IsDlgButtonChecked(hwnd, IDPERUSER) == BST_UNCHECKED);
						AskReboot(IsDlgButtonChecked(hwnd, IDPROMPTREBOOT) == BST_CHECKED);
						EndDialog(hwnd, IDEXPLORER);
						break;

					case IDCANCEL:
						EndDialog(hwnd, IDCANCEL);
						break;

					case IDBROWSE:
						OPENFILENAME ofn;
						memset(&ofn, 0, sizeof(ofn));
						TCHAR szShellPath[MAX_PATH];
						szShellPath[0] = 0;
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hwnd;
						ofn.lpstrTitle = TEXT("Select A Shell");
						ofn.lpstrFilter = TEXT("Shell Executables\0*.exe\0All Files\0*.*\0\0");
						ofn.nFilterIndex = 1;
						ofn.lpstrFile = szShellPath;
						ofn.nMaxFile = MAX_PATH;
						ofn.Flags = OFN_FILEMUSTEXIST;

						if (GetOpenFileName(&ofn) != FALSE)
							Edit_SetText(GetDlgItem(hwnd, IDC_EDIT1), ofn.lpstrFile);

						break;

					default:
						return FALSE;
				}
				break;
		default:
			return FALSE;
	}
	return TRUE;
}