/*
* システムの復元ポイントの生成
* @nanakochi123456
*
*/

/* ライブラリの入力に Srclient.lib を追加
* stdafx.h に以下を追加
* #include <windows.h>
* #include <srrestoreptapi.h>
*/

#include "stdafx.h"
#include "makesystemrestorepoint.h"

#define	RestorePtrType	APPLICATION_INSTALL;
#define	RestoreDescription	_T("Demonstration Restore Point")

#define MAX_LOADSTRING 100

HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#ifndef UNICODE

static LPWSTR atow(LPCSTR src)
{
	LPWSTR buf;
	int dst_size, rc;

	rc = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
	if (rc == 0) {
		return NULL;
	}

	dst_size = rc + 1;
	buf = (LPWSTR)malloc(sizeof(WCHAR) * dst_size);
	if (buf == NULL) {
		return NULL;
	}

	rc = MultiByteToWideChar(CP_ACP, 0, src, -1, buf, dst_size);
	if (rc == 0) {
		free(buf);
		return NULL;
	}
	buf[rc] = L'\0';

	return buf;
}

static LPSTR wtoa(LPCWSTR src)
{
	LPSTR buf;
	int dst_size, rc;

	rc = WideCharToMultiByte(CP_ACP, 0, src, -1, NULL, 0, NULL, NULL);
	if (rc == 0) {
		return NULL;
	}

	dst_size = rc + 1;
	buf = (LPSTR)malloc(dst_size);
	if (buf == NULL) {
		return NULL;
	}

	rc = WideCharToMultiByte(CP_ACP, 0, src, -1, buf, dst_size, NULL, NULL);
	if (rc == 0) {
		free(buf);
		return NULL;
	}
	buf[rc] = '\0';

	return buf;
}
#endif /* ndef UNICODE */

static LPTSTR *parse_args(LPCTSTR args_t, int *argc)
{
	LPCWSTR args_w;
	LPWSTR *argv_w;

	if (args_t[0] == _T('\0')) {
		*argc = 0;
		return NULL;
	}

#ifdef UNICODE
	args_w = args_t;
#else
	args_w = (LPCWSTR)atow(args_t);
	if (args_w == NULL) {
		return NULL;
	}
#endif

	argv_w = CommandLineToArgvW(args_w, argc);

#ifdef UNICODE
	return argv_w;
#else
	free((void *)args_w);
	if (argv_w == NULL) {
		return NULL;
	}

	{
		LPSTR *argv_c = NULL;
		int i, j;

		argv_c = (LPSTR *)malloc(sizeof(argv_c[0]) * (*argc + 1));
		if (argv_c == NULL) {
			goto DONE;
		}
		for (i = 0; i < *argc; ++i) {
			argv_c[i] = wtoa(argv_w[i]);
			if (argv_c[i] == NULL) {
				for (j = 0; j < i; ++j) {
					free(argv_c[j]);
				}
				free(argv_c);
				argv_c = NULL;
				goto DONE;
			}
		}
		argv_c[i] = NULL;

	DONE:
		(void)LocalFree((HLOCAL)argv_w);
		return argv_c;
	}
#endif
}

static void free_args(int argc, LPTSTR *argv)
{
#ifdef UNICODE
	(void) argc;
	(void)LocalFree((HLOCAL)argv);
#else
	int i;

	for (i = 0; i < argc; ++i) {
		free(argv[i]);
	}
	free(argv);
#endif
}

int tMakeSystemRestore(int argc, LPTSTR *argv) {
	RESTOREPOINTINFO RstPt;
	STATEMGRSTATUS MgrStat;

	LPTSTR	szRestoreMsg;

	if (argc < 2) {
		szRestoreMsg = RestoreDescription;
	} else { 
		if (wcscmp(argv[1], _T("/?")) == 0) {
			MessageBox(NULL, _T("システムの復元作成プログラム by ななこっち★\nUsage このプログラム.exe [システムの復元ポイントに残す任意のメッセージ]"), _T("システムの復元作成プログラム"), MB_ICONINFORMATION);
			return 1;
		}
		szRestoreMsg = argv[1];
	}

	if (wcslen(szRestoreMsg) > 40) {
		MessageBox(NULL, _T("そんなに長いメッセージは無理なの"), _T("システムの復元作成プログラム"), MB_ICONINFORMATION);
		return 1;
	}
	RstPt.dwEventType = BEGIN_SYSTEM_CHANGE;
	RstPt.dwRestorePtType = RestorePtrType;
	wcscpy_s((wchar_t *)RstPt.szDescription, wcslen(szRestoreMsg) + 1, szRestoreMsg);

	if (!SRSetRestorePoint(&RstPt, &MgrStat))
	{
		MessageBox(NULL, _T("検索失敗"), _T("＠＠"), MB_ICONINFORMATION);
		return 1;
	}

	RstPt.dwEventType = END_SYSTEM_CHANGE;
	RstPt.llSequenceNumber = MgrStat.llSequenceNumber;

	if (!SRSetRestorePoint(&RstPt, &MgrStat))
	{
		// システムの復元ポイントの失敗時　何もしない
			MessageBox(NULL, _T("作成失敗"), _T("＠＠"), MB_ICONINFORMATION);
			return 1;
	}
	return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	int iRet, argc = 0;
	LPTSTR *argv;
	argv = parse_args(GetCommandLine(), &argc);

	iRet = tMakeSystemRestore(argc, argv);

	free_args(argc, argv);
}
