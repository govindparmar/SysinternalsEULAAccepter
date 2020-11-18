#include <Windows.h>
#include <PathCch.h>
#include <strsafe.h>
#include <sal.h>

#pragma comment(lib, "pathcch.lib")
#define WMain wmain

_Success_(return == ERROR_SUCCESS)

LSTATUS WINAPI InsertRegistryEntry(_In_reads_or_z_(MAX_PATH) WCHAR *wszFileName)
{
	LSTATUS lStatus = 0L;
	HKEY hKey = NULL;
	DWORD dwData = 0x00000001;

	lStatus = RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\SysInternals", 0, NULL, 0, KEY_WRITE | KEY_READ, NULL, &hKey, NULL);
	if (lStatus != ERROR_SUCCESS)
	{
		goto cleanup;
	}

	lStatus = RegCreateKeyW(hKey, wszFileName, &hKey);
	if (lStatus != ERROR_SUCCESS)
	{
		goto cleanup;
	}

	lStatus = RegSetValueExW(hKey, L"EulaAccepted", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
	if (lStatus != ERROR_SUCCESS)
	{
		goto cleanup;
	}

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

	// TODO: Replace with YOUR path to extracted Sysinternals folder
	PathCchCombine(wszFullPath, MAX_PATH, L"C:\\SysInternals", L"*");
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

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}
		
		PathCchFindExtension(wfd.cFileName, MAX_PATH, &pExt);
		
		if (_wcsicmp(pExt, L"exe") == 0 || _wcsicmp(pExt, L".exe") == 0)
		{
			StringCchCopyW(wszFileArgument, MAX_PATH, wfd.cFileName);
			if ((pDot = wcschr(wszFileArgument, L'.')) != NULL)
			{
				LSTATUS ls;

				*pDot = L'\0';

				if ((ls = InsertRegistryEntry(wszFileArgument)) != ERROR_SUCCESS)
				{
					fwprintf_s(stderr, L"Failed to create registry entry for %s: %I32u\n", wszFileArgument, ls);
				}
			}
		}
	}
	while (FindNextFileW(hFind, &wfd));
	FindClose(hFind);

cleanup:
	return dwError;
}