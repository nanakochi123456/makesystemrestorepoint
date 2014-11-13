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

#define	DefaultRestorePtrType		APPLICATION_INSTALL;
#define	DefaultRestoreDescription	_T("makesystemrestorepoint")

#define MAX_LOADSTRING 40
#define	MAX_STRINGS	256

#define	SP_OK		0
#define	SP_NG1		-1
#define	SP_NG2		-2
#define	SP_HELP		-64
#define	SP_OPTERR	-128

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

#endif /* ndef UNICODE */

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

int	iSilent = 0;

int msgbox(HINSTANCE hInst, int iMsgid, int iButtonid) {
	TCHAR	szMsg[MAX_STRINGS];
	TCHAR	szTitle[MAX_STRINGS];

	LoadString(hInst, iMsgid, szMsg, sizeof(szMsg) / sizeof(szMsg[0]));
	LoadString(hInst, IDS_APP_TITLE, szTitle, sizeof(szTitle) / sizeof(szTitle[0]));
	MessageBox(NULL, szMsg, szTitle, iButtonid);
	return SP_NG1;
}

int infobox(HINSTANCE hInst, int iMsgid) {
	if (!iSilent) {
		msgbox(hInst, iMsgid, MB_OK | MB_ICONINFORMATION);
	}
	return SP_NG1;
}

int errmsg(HINSTANCE hInst, int iMsgid) {
	if (!iSilent) {
		msgbox(hInst, iMsgid, MB_OK | MB_ICONERROR);
	}
	return SP_NG1;
}

int erropt(HINSTANCE hInst, int iMsgid) {
	msgbox(hInst, iMsgid, MB_OK | MB_ICONWARNING);
	return SP_OPTERR;
}

RESTOREPOINTINFO RstPt;
STATEMGRSTATUS MgrStat;

INT64 tMakeSystemBegin(int iMode, int iSilent, LPTSTR szRestoreMsg) {

	RstPt.dwEventType = BEGIN_SYSTEM_CHANGE;
	RstPt.dwRestorePtType = iMode;
	_tcscpy_s((wchar_t *)RstPt.szDescription, _tcslen(szRestoreMsg) + 1, szRestoreMsg);

	if (!SRSetRestorePoint(&RstPt, &MgrStat)) {
		return -1;
	}
	return MgrStat.llSequenceNumber;
}

int tMakeSystemEnd(int iMode, int iSilent, INT64 iSeq) {
	RstPt.dwEventType = END_SYSTEM_CHANGE;
	RstPt.llSequenceNumber = iSeq;
	RstPt.dwRestorePtType = iMode;

	if (!SRSetRestorePoint(&RstPt, &MgrStat)) {
		return 1;
	}
	return 0;
}

int optcmp(HINSTANCE hInst, LPTSTR argv, int iMsgid) {
	TCHAR	szTmp[MAX_STRINGS];
	TCHAR	szTmp2[MAX_STRINGS];
	TCHAR	*szTok;
	TCHAR	*next_token = NULL;
	int		iFlg = 0;

	LoadString(hInst, iMsgid, szTmp, sizeof(szTmp) / sizeof(szTmp[0]));
	_tcsupr_s(szTmp, MAX_STRINGS);
	_tcscpy_s(szTmp2, MAX_STRINGS, argv);
	_tcsupr_s(szTmp2, MAX_STRINGS);

	szTok = _tcstok_s(szTmp, _T(","), &next_token);
	do {
		if (_tcscmp(szTok, szTmp2) == 0) {
			iFlg = 1;
		}
	} while ((szTok = _tcstok_s(NULL, _T(","), &next_token)) != NULL);
	return iFlg;
}


int tMakeSystemRestore(HINSTANCE hInst, int argc, LPTSTR *argv) {
	LPTSTR	szRestoreMsg = DefaultRestoreDescription;
	int	i;
	int iFlg = 0;
	int iMode = 0;
	int iRet = SP_OK;
	INT64 iSeq = 0;

	for (i = 1; i < argc; i++) {
		if (optcmp(hInst, argv[i], IDS_SP_OPT_HELP)) {
			infobox(hInst, IDS_SP_HELP);
			return SP_HELP;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_SILENT)) {
			iSilent = 1;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_BEGIN)) {
			if (iFlg) {
				return erropt(hInst, IDS_SP_ERR_OPT);
			}
			iFlg = 1;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_END)) {
			if (iFlg) {
				return erropt(hInst, IDS_SP_ERR_OPT);
			}
			iFlg = 2;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_INSTALL)) {
			if (iMode) {
				return erropt(hInst, IDS_SP_ERR_OPT);
			}
			iMode = APPLICATION_INSTALL;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_UNINSTALL)) {
			if (iMode) {
				return erropt(hInst, IDS_SP_ERR_OPT);
			}
			iMode = APPLICATION_UNINSTALL;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_DEVICE)) {
			if (iMode) {
				return erropt(hInst, IDS_SP_ERR_OPT);
			}
			iMode = DEVICE_DRIVER_INSTALL;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_MODIFY)) {
			if (iMode) {
				return erropt(hInst, IDS_SP_ERR_OPT);
			}
			iMode = MODIFY_SETTINGS;
		}
		else if (optcmp(hInst, argv[i], IDS_SP_OPT_CANCEL)) {
			if (iMode) {
				return erropt(hInst, IDS_SP_ERR_OPT);
			}
			iMode = CANCELLED_OPERATION;
		}
		else {
			szRestoreMsg = argv[i];
			if (_tcslen(szRestoreMsg) > MAX_LOADSTRING) {
				return erropt(hInst, IDS_SP_ERR_LONG);
			}
		}
	}

	if (!iMode) {
		iMode = DefaultRestorePtrType;
	}
	if (!iFlg || iFlg == 1) {
		if ((iSeq = tMakeSystemBegin(iMode, iSilent, szRestoreMsg)) == -1) {
			return errmsg(hInst, IDS_SP_ERR_NG1);
		}
	}
	if (!iFlg || iFlg == 2) {
		if (iFlg == 2) {
			iSeq = atoi(wtoa(szRestoreMsg));
			if (iSeq < 1) {
				return erropt(hInst, IDS_SP_ERR_NUM);
			}
		}
		if ((iSeq = tMakeSystemEnd(iMode, iSilent, iSeq)) == -1) {
			return errmsg(hInst, IDS_SP_ERR_NG2);
		}
		infobox(hInst, IDS_SP_OK);
	}
	return (iFlg == 1 ? iSeq : SP_OK);
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	int argc = 0, iRet = 0;
	LPTSTR *argv;
	argv = parse_args(GetCommandLine(), &argc);

	iRet = (int)tMakeSystemRestore(hInstance, argc, argv);

	free_args(argc, argv);
	return iRet;
}
