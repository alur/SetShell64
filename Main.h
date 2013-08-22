#include <windows.h>
#include <shlwapi.h>
#include "resource.h"
#include <tchar.h>
#include <Windowsx.h>
#include <commdlg.h>
#include <iostream>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void SetShell(const char* pszShell, bool bPerUser);
bool file_exists(const char* szFilename);
void AskReboot(bool bActuallyAsk);
void Reboot();

HINSTANCE g_hInst;
TCHAR g_sz32bitshellLocation[MAX_PATH], g_sz64bitshellLocation[MAX_PATH];
TCHAR g_szShellLocation[MAX_PATH];
TCHAR g_sz32bitIni[MAX_PATH], g_sz64bitIni[MAX_PATH];