#include <Windows.h>
#include <PathCch.h>
#include <strsafe.h>
#include <sal.h>

#pragma comment(lib, "pathcch.lib")
#define WMain wmain
#define SYSINTERNALS_FOLDER L"C:\\SysInternals"
#define SUBKEY_PATH L"SOFTWARE\\SysInternals"
#define VALUE_NAME L"EulaAccepted"
#define ACCEPTED_EULA 0x00000001

_Check_return_
_Success_(return == ERROR_SUCCESS)


LSTATUS WINAPI InsertRegistryEntry(_In_reads_or_z_(MAX_PATH) WCHAR *wszFileName)
{
	LSTATUS lStatus;
	HKEY hKey = NULL;
	DWORD dwData = ACCEPTED_EULA;

	lStatus = RegCreateKeyExW(HKEY_CURRENT_USER, SUBKEY_PATH, 0, NULL, 0, KEY_WRITE | KEY_READ, NULL, &hKey, NULL);
	if (lStatus != ERROR_SUCCESS)
	{
		goto cleanup;
	}

	lStatus = RegCreateKeyW(hKey, wszFileName, &hKey);
	if (lStatus != ERROR_SUCCESS)
	{
		goto cleanup;
	}

	lStatus = RegSetValueExW(hKey, VALUE_NAME, 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
	if (lStatus != ERROR_SUCCESS)
	{
		goto cleanup;
	}

	lStatus = ERROR_SUCCESS;

cleanup:
	if (hKey != NULL)
	{
		RegCloseKey(hKey);
		hKey = NULL;
	}

	return lStatus;
}

INT WINAPIV WMain (_In_ INT nArgc, _In_reads_(nArgc) WCHAR *pArgv[])
{
	WIN32_FIND_DATAW wfd;
	WCHAR wszFullPath[MAX_PATH];
	HANDLE hFind;
	DWORD dwError = ERROR_SUCCESS;

	PathCchCombine(wszFullPath, MAX_PATH, SYSINTERNALS_FOLDER, L"*.exe");
	hFind = FindFirstFileW(wszFullPath, &wfd);
	if (INVALID_HANDLE_VALUE == hFind || NULL == hFind)
	{
		dwError = GetLastError();
		goto cleanup;
	}

	do
	{
		WCHAR *pExt = NULL, *pDot = NULL;
		WCHAR wszFileArgument[MAX_PATH];
		LSTATUS ls;

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		
		StringCchCopyW(wszFileArgument, MAX_PATH, wfd.cFileName);
		
		if ((ls = InsertRegistryEntry(wszFileArgument)) != ERROR_SUCCESS)
		{
			fwprintf_s(stderr, L"Failed to create registry entry for %s: %I32u\n", wszFileArgument, ls);
		}
		else
		{
			wprintf_s(L"Successfully wrote registry entry for %s.\n", wszFileArgument);
		}
		
	}
	while (FindNextFileW(hFind, &wfd));
	FindClose(hFind);

cleanup:
	return dwError;
}