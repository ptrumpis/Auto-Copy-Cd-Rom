#include "stdafx.h"
#include "AutoCopyCdRom.h"
#include "CINIFile.h"

#define MAX_LOADSTRING	100
#define CONFIG_FILE		"AutoCopyCdRom.ini"


typedef struct DRVIVECOPYSTRUCT {
	CHAR letter;
	UINT state;
	UINT type;
	TCHAR label[256];
	TCHAR status[256];
	TCHAR from[512];
	TCHAR to[512];
	HANDLE thread;
	DWORD threadid;
}DRVIVECOPYSTRUCT; //drc

typedef struct CONFIGSTRUCT {
	TCHAR label[256];
	TCHAR path[512];
}CONFIGSTRUCT; //cfg

typedef struct DEFCONFIGSTRUCT {
    TCHAR path[512] = "";
    int subfolder = 0;
}DEFCONFIGSTRUCT; //cfg


HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
HWND hWnd;
DRVIVECOPYSTRUCT drc[26];
CONFIGSTRUCT cfg[128];
DEFCONFIGSTRUCT defcfg;
BOOL isDefaultPresent = FALSE;
UINT nDrives = 0;
UINT nLabels = 0;
UINT g_uQueryCancelAutoPlay = 0;
CINIFile Config;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void				onPaint(HDC, LPPAINTSTRUCT);
void				onDeviceChange(WPARAM, LPARAM);
void				LoadAppConfiguration();
void				DetectCdromDrives();
BOOL				EjectCdromDrive(TCHAR);
char				FirstDriveFromMask(ULONG);
void				CopyFolderStructure(LPCTSTR, LPCTSTR);
DWORD WINAPI		CopyThreadFunc(LPVOID);


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	DetectCdromDrives();
	LoadAppConfiguration();

	MSG msg;
	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDS_AUTOCOPYCDROM, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUTOCOPYCDROM));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUTOCOPYCDROM));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_AUTOCOPYCDROM);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hWnd) {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		onPaint(hdc, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_DEVICECHANGE:
		onDeviceChange(wParam, lParam);
		break;

	default:
		if (!g_uQueryCancelAutoPlay)
		{
			g_uQueryCancelAutoPlay = RegisterWindowMessage(_T("QueryCancelAutoPlay"));
		}
		if (message && message == g_uQueryCancelAutoPlay)
		{
			// cancel auto-play
			return TRUE;
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void LoadAppConfiguration()
{
    DWORD size = 256;
    TCHAR cwd[1024], configfile[1024], section[256];

    GetCurrentDirectory(1024, cwd);
    wsprintf(configfile, _T("%s\\%s"), cwd, CONFIG_FILE);
    Config.SetFileName(configfile);

    if(Config.GetFirstSectionName(section, &size))
    {
        do
        {
            if(_strstr(section, "default") != NULL)
            {
                isDefaultPresent = TRUE;
                // default configuration has a path and a sub folder creation flag
                Config.ReadString(_T("default"), _T("Path"), defcfg.path, 512);
                Config.ReadValue(_T("default"), _T("SubFolder"), &defcfg.subfolder);
                continue;
            }

            // configuration profiles require a cd label and a file path
            Config.ReadString(section, _T("Label"), cfg[nLabels].label, 256);
            Config.ReadString(section, _T("Path"), cfg[nLabels].path, 512);
            size = 256;

            nLabels++;
        }
        while(Config.GetNextSectionName(section, &size));
    }
    else
    {
        TCHAR error[2048];
        wsprintf(error, _T("Error: Unable to process config file: '%s'"), configfile);
        MessageBox(NULL, error, _T("Error"), 0);
        exit(-1);
    }
}

void onPaint(HDC hdc, LPPAINTSTRUCT ps)
{
	TCHAR out[1024];
	int x_drive = 20,
		y_drive = 20,
		x_status = 300,
		y_status = 20,
		line = 25;

	// Head
	wsprintf(out, _T("CD-Rom Drives"));
	TextOut(hdc, x_drive, y_drive, out, lstrlen(out));
	y_drive += line + line;
	wsprintf(out, _T("Status"));
	TextOut(hdc, x_status, y_status, out, lstrlen(out));
	y_status += line + line;

	// Body
	for(int i=0; i<nDrives; i++)
	{
		wsprintf(out, _T("%c:\\%s"), toupper(drc[i].letter), drc[i].label);
		TextOut(hdc, x_drive, y_drive, out, lstrlen(out));
		y_drive += line;

		lstrcpy(out, drc[i].status);
		TextOut(hdc, x_status, y_status, out, lstrlen(out));
		y_status += line;
	}
}

void onDeviceChange(WPARAM wEvent, LPARAM lData)
{
	PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lData;
	CHAR letter;
	DWORD dwSerial, dwMFL, dwSysFlags;
	TCHAR szRootPath[16], szFileSys[255], szVolNameBuff[255], buffer[512];

	switch(wEvent)
	{
        // cd inserted
        case DBT_DEVICEARRIVAL:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags & DBTF_MEDIA)
                {
                    letter = FirstDriveFromMask(lpdbv->dbcv_unitmask);
                    wsprintf(szRootPath, _T("%c:\\"), letter);
                    GetVolumeInformation(szRootPath, szVolNameBuff, 255, &dwSerial, &dwMFL, &dwSysFlags, szFileSys, 255);

                    BOOL bFound = FALSE;
                    for(int i=0; i<nDrives && bFound == FALSE; i++)
                    {
                        if(_tolower(letter) == _tolower(drc[i].letter))
                        {
                            drc[i].state = 1;
                            lstrcpy(drc[i].label, szVolNameBuff);
                            lstrcpy(drc[i].status, _T("CD-ROM inserted..."));

                            // re-draw window
                            InvalidateRect(hWnd, NULL, true);

                            for(int j=0; j<nLabels && bFound == FALSE; j++)
                            {
                                // compare cd labels against config profiles
                                if(_strstr(cfg[j].label, drc[i].label) != NULL)
                                {
                                    bFound = TRUE;

                                    wsprintf(drc[i].from, _T("%c:\\*"), drc[i].letter);
                                    wsprintf(drc[i].to, _T("%s\\"), cfg[j].path);

                                    // copy to specific path
                                    drc[i].thread = CreateThread(NULL, 0, CopyThreadFunc, (LPVOID)&drc[i], 0, &drc[i].threadid);
                                    break;
                                }
                            }

                            // fallback to default config
                            if(bFound == FALSE && isDefaultPresent == TRUE)
                            {
                                wsprintf(drc[i].from, _T("%c:\\*"), drc[i].letter);

                                // optional sub folder creation
                                if (defcfg.subfolder == 1) {
                                    wsprintf(drc[i].to, _T("%s\\%s\\"), defcfg.path, drc[i].label);
                                } else {
                                    wsprintf(drc[i].to, _T("%s\\"), defcfg.path);
                                }

                                drc[i].thread = CreateThread(NULL, 0, CopyThreadFunc, (LPVOID)&drc[i], 0, &drc[i].threadid);
                            }
                            break;
                        }
                    }

                }
            }
            break;

        // cd removed
        case DBT_DEVICEREMOVECOMPLETE:
            if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                if(lpdbv->dbcv_flags & DBTF_MEDIA)
                {
                    letter = FirstDriveFromMask(lpdbv->dbcv_unitmask);
                    for(int i=0; i<nDrives; i++)
                    {
                        if(_tolower(letter) == _tolower(drc[i].letter))
                        {
                            drc[i].state = 2;
                            lstrcpy(drc[i].label, _T(""));
                            lstrcpy(drc[i].status, _T("CD-ROM ejected..."));

                            // re-draw window
                            InvalidateRect(hWnd, NULL, true);

                            if(drc[i].thread != NULL)
                            {
                                CloseHandle(drc[i].thread);
                                drc[i].thread = NULL;
                                drc[i].threadid = 0;
                            }
                            break;
                        }
                    }
                }
            }
            break;

        case DBT_DEVICEQUERYREMOVEFAILED:
            // todo
        case DBT_DEVICEQUERYREMOVE:
            // todo
        case DBT_DEVICEREMOVEPENDING:
            // todo
        default:
            break;
	}
}

void DetectCdromDrives()
{
	CHAR curletter = 'a';
	TCHAR drive[16] = _T(""), szFileSys[255] = _T(""), szVolNameBuff[255] = _T("");
	DWORD dwSerial, dwMFL, dwSysFlags;

	// check all drives from a to z to find cd-rom drives
	while(curletter <= 'z')
	{
		wsprintf(drive, _T("%c:\\"), curletter);

		UINT uType = GetDriveType(drive);
		if(uType == DRIVE_CDROM)
		{
			GetVolumeInformation(drive, szVolNameBuff, 255, &dwSerial, &dwMFL, &dwSysFlags, szFileSys, 255);

			drc[nDrives].letter = curletter;
			drc[nDrives].type = uType;
			drc[nDrives].state = 0;

			lstrcpy(drc[nDrives].label, szVolNameBuff);
			lstrcpy(drc[nDrives].status, _T("Waiting for event..."));
			lstrcpy(drc[nDrives].from, _T(""));
			lstrcpy(drc[nDrives].to, _T(""));

			drc[nDrives].thread = NULL;
			drc[nDrives].threadid = 0;

			nDrives++;
		}
		curletter++;
	}
}

BOOL EjectCdromDrive(TCHAR letter)
{
	BOOL bResult = FALSE;
	DWORD dwError = 0, dwBytesReturned = 0;
    HANDLE hDevice = NULL;
	TCHAR drivename[32];

	wsprintf(drivename, _T("\\\\.\\%c:"), letter);

	hDevice = CreateFile(drivename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(hDevice == INVALID_HANDLE_VALUE)
    {
        SetLastError(NO_ERROR);
        hDevice = CreateFile(drivename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }

    dwError=GetLastError();

    if((hDevice != INVALID_HANDLE_VALUE) && (dwError == NO_ERROR))
    {
        bResult = DeviceIoControl(hDevice, IOCTL_STORAGE_EJECT_MEDIA, 0, 0, 0, 0, &dwBytesReturned, 0);
    }
    else
    {
        bResult = FALSE;
    }

    if(hDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hDevice);
    }

	return bResult;
}

char FirstDriveFromMask (ULONG unitmask)
{
	CHAR i;
	for(i = 0; i < 26; ++i)
	{
		if(unitmask & 0x1)
			break;

		unitmask = unitmask >> 1;
	}

	return (i + 'a');
}

void CopyFolderStructure(LPCTSTR lpFrom, LPCTSTR lpTo)
{
    SHFILEOPSTRUCT sfo;

    ZeroMemory(&sfo,sizeof(sfo));

    sfo.hwnd = NULL;
    sfo.wFunc = FO_COPY;
    sfo.pFrom = lpFrom;
    sfo.pTo = lpTo;
    sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

    SHFileOperation(&sfo);
}

DWORD WINAPI CopyThreadFunc(LPVOID lpParam)
{
	DRVIVECOPYSTRUCT * p = (DRVIVECOPYSTRUCT*) lpParam;

    wsprintf(p->status, _T("Copying files to '%s'"), p->to);

    // re-draw window
    InvalidateRect(hWnd, NULL, true);

	CopyFolderStructure(p->from, p->to);

	lstrcpy(p->status, _T("Copy Complete"));

    // re-draw window
    InvalidateRect(hWnd, NULL, true);

	// eject cd after copying
	EjectCdromDrive(p->letter);

	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}
