// NataProg.cpp : Определяет точку входа для приложения.
//

#include "common.h"
#include "NataProg.h"
#include "Fuses.h"
#include "Dude.h"
#include <stdio.h>
#define MAX_LOADSTRING 100
#define BORDER_PADDING 7
// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
TCHAR szCurDir[MAX_PATH];
_tstring currentHexFile;
BOOL bInfoFocus = FALSE;
DF_VEC vDevFuse; // База данных фьюзов и устройств
PP_VEC vProgPort; // База данных программаторов и портов

// Отправить объявления функций, включенных в этот модуль кода:

BOOL                InitInstance(HINSTANCE, int);

INT_PTR CALLBACK    MainDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK OwnerDrawButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NATAPROG, szWindowClass, MAX_LOADSTRING);
    //MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NATAPROG));

    MSG msg;

    
    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            if (!IsDialogMessage(msg.hwnd, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return (int) msg.wParam;
}



//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной
  /* INITCOMMONCONTROLSEX icx;
   icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
   icx.dwICC = ICC_STANDARD_CLASSES;
   BOOL b = InitCommonControlsEx(&icx);*/
   
   HWND hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), nullptr, MainDlgProc);
   
   if (!hDlg)
   {
       return FALSE;
   }
       
   return TRUE;
}

//
//  ФУНКЦИЯ: MainDlgProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
// MakeWindowLook

void MakeWindowLook(HWND hWnd, int nCmdLook)
{

    static BOOL bWide = TRUE;
    static LONG initialDlgWidth = 0;
    RECT rect;
    LONG curWidth;

    GetWindowRect(hWnd, &rect);
    curWidth = rect.right - rect.left;

    switch (nCmdLook)
    {
    case INIT_WINDOW:
        initialDlgWidth = curWidth;
        bWide = FALSE;
        break;
    
    case SWAP_WINDOW:       
        bWide = !bWide;
        break;

    case SMALL_WINDOW:
        bWide = FALSE;       
        break;

    case WIDE_WINDOW:

    default:
        bWide = TRUE;
    }

    if (bWide)
        curWidth = initialDlgWidth;
    else
        curWidth = initialDlgWidth / 2 + BORDER_PADDING * 2;

    SetDlgItemText(hWnd, IDC_SWAP_CONSOLE, bWide ? TEXT("<") : TEXT(">"));

    SetWindowPos(hWnd, nullptr, rect.left, rect.top,
        curWidth, rect.bottom - rect.top,
        SWP_NOMOVE);
}
HBRUSH MakeControlGray(HDC hDC)
{
    SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
    return GetSysColorBrush(COLOR_BTNFACE);
}
HBRUSH MakeControlBlue(HDC hDC)
{
    
    SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
    SetTextColor(hDC, RGB(0, 0, 200));
    SetBkMode(hDC, TRANSPARENT);    
    return GetSysColorBrush(COLOR_BTNFACE);
}
HBRUSH MakeControlWhiteBackground(HDC hDC, BOOL state)
{
    SetBkColor(hDC, RGB(255, 255, 255));
    SetBkMode(hDC, TRANSPARENT);
    return state? (HBRUSH)GetStockObject(DC_BRUSH): GetSysColorBrush(COLOR_BTNFACE);
}
DEVICE_VEC  vDeviceData;
SPEED_VEC   vSpeedData;
PROG_VEC    vProgData;
PORT_VEC    vPortData;
BOOL        bParamChange = TRUE;

PARAMETR & GetParameters(HWND hDlg)
{
    static PARAMETR localp;
    // Устройство
    if (bParamChange) 
    {
        INT deviceIndex = SendDlgItemMessage(hDlg, IDC_DEVICE, CB_GETCURSEL, 0, 0L);
        INT progIndex = SendDlgItemMessage(hDlg, IDC_PROGRAMMER, CB_GETCURSEL, 0, 0L);
        INT portIndex = SendDlgItemMessage(hDlg, IDC_PORT, CB_GETCURSEL, 0, 0L);
        INT speedIndex = SendDlgItemMessage(hDlg, IDC_BR_SPEED, CB_GETCURSEL, 0, 0L);
        bParamChange = FALSE;
        localp = std::make_tuple(vProgData[progIndex], vDeviceData[deviceIndex], 
            vPortData[portIndex], vSpeedData[speedIndex]);
       
    }
    return localp;   
}
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    
    UNREFERENCED_PARAMETER(lParam);
    
    HICON hIcon, hIconSmall;
    
    //static HBRUSH hbrBkgnd = NULL;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        //int i;

        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NATAPROG));
        hIconSmall = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PROCESOR));
        //hIconOpen = LoadIcon(hInst, MAKEINTRESOURCE(IDI_OPENFOLDER));
        HBITMAP hImgOpenFolder = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OPENFILE));

        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        

        SendDlgItemMessage(hDlg, IDC_OPENFILE, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hImgOpenFolder);
       
        SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Программа в процессе разработки."));
        
        GetCurrentDirectory(MAX_PATH, szCurDir);

        PathCchRemoveFileSpec(szCurDir, MAX_PATH); // Здесь директория
        _tcscat_s(szCurDir, MAX_PATH, _T("\\Debug\\data"));

        SetCurrentDirectory(szCurDir);
        // Заполнить список устройств (микроконтроллеров)
        /*const TCHAR* szDevice[] = {
            TEXT("ATtiny13"), TEXT("Atmega8")
        };*/
        int devpos = -1;
        const DEVICE_VEC vDev = LoadDevices(devpos, _T("attiny13"));
        for (auto device: vDev)
        {
            SendDlgItemMessage(hDlg, IDC_DEVICE, CB_ADDSTRING, 0, (LPARAM)device.name);
        }
        vDeviceData = vDev;
        // Выбрать устройство по-умолчанию
        if (devpos >= 0)
            SendDlgItemMessage(hDlg, IDC_DEVICE, CB_SETCURSEL, (WPARAM)devpos, 0L);

        SendDlgItemMessage(hDlg, IDC_DEVICE, CB_SETMINVISIBLE, (WPARAM)10, 0L);
        int progpos = -1;
        // Загружаем файл Programmer.txt (по умолчанию ищем позицию avrisp
        const PROG_VEC vszProg = LoadProgrammers(progpos, _T("arduino"));      
        
        for (auto prog : vszProg)
        {            
            SendDlgItemMessage(hDlg, IDC_PROGRAMMER, CB_ADDSTRING, 0, (LPARAM)prog.c_str());           
        }
        vProgData = vszProg;
        // Ограничить выпадающий список до 10 элементов, остальные через прокрутку
        SendDlgItemMessage(hDlg, IDC_PROGRAMMER, CB_SETMINVISIBLE, (WPARAM)10, 0L);

        // Выбрать программатор по-умолчанию
        if (progpos >= 0)
            SendDlgItemMessage(hDlg, IDC_PROGRAMMER, CB_SETCURSEL, (WPARAM)progpos, 0L);

        int portpos = -1;
        vProgPort = LoadPorts(portpos, _T("COM4"));
        
        // Извлечь порты 
        PORT_VEC vPort;
        for (auto progport : vProgPort)
        {
            if (std::get<0>(progport)==_T("Default"))
                vPort.push_back(std::get<1>(progport));
        }
        for (auto port : vPort)
        {
            SendDlgItemMessage(hDlg, IDC_PORT, CB_ADDSTRING, 0, (LPARAM)port.name);
        }
        vPortData = vPort;
        // Ограничить выпадающий список до 10 элементов, остальные через прокрутку
        SendDlgItemMessage(hDlg, IDC_PORT, CB_SETMINVISIBLE, (WPARAM)10, 0L);
        // Выбрать порт по-умолчанию
        SendDlgItemMessage(hDlg, IDC_PORT, CB_SETCURSEL, (WPARAM)portpos, 0L);

        /*const TCHAR* szSpeed[] = {
            TEXT("BR 1200"),  TEXT("BR 2400"),  TEXT("BR 4800"),  TEXT("BR 9600"),
            TEXT("BR 14400"), TEXT("BR 19200"), TEXT("BR 38400"), TEXT("BR 57600"),
            TEXT("BR 115200"),TEXT("BC 1200"),  TEXT("BC 2400"),  TEXT("BC 4800"),
            TEXT("BC 9600"),  TEXT("BC 14400"), TEXT("BC 19200"), TEXT("BC 38400"),
            TEXT("BC 57600"), TEXT("BC 115200"),TEXT("BC 230400"),TEXT("BC 460800"),
            TEXT("BC 921600"), TEXT("BC 3000000")
            
        };*/
        int speedpos = -1;
        const SPEED_VEC vSpeed = LoadSpeed(speedpos, _T("BR 19200"));
        for (auto speed : vSpeed)
        {
            SendDlgItemMessage(hDlg, IDC_BR_SPEED, CB_ADDSTRING, 0, (LPARAM)speed.name);
        }
        // Ограничить выпадающий список до 10 элементов, остальные через прокрутку
        vSpeedData = vSpeed;
        SendDlgItemMessage(hDlg, IDC_BR_SPEED, CB_SETMINVISIBLE, (WPARAM)10, 0L);
        
        // Выбрать строку по-умолчанию
        if (speedpos >= 0)
        SendDlgItemMessage(hDlg, IDC_BR_SPEED, CB_SETCURSEL, (WPARAM)speedpos, 0L);

        vDevFuse = LoadFuseData();
        
        SendDlgItemMessage(hDlg, IDC_FUSES, CB_SETMINVISIBLE, (WPARAM)10, 0L);

        // Выбрать строку по-умолчанию
        if (false)
            SendDlgItemMessage(hDlg, IDC_FUSES, CB_SETCURSEL, (WPARAM)0, 0L);

        LONG idc_groupbox[] = { IDC_STATIC3, IDC_STATIC4, IDC_STATIC5, 
            IDC_STATIC6, IDC_STATIC7, IDC_STATIC8
        }; // Потом сюда еще добавить

        // Выключить стили для Group Boxes
        for (int i = 0; i < sizeof(idc_groupbox) / sizeof(idc_groupbox[0]); i++)
        {
            SetWindowTheme(GetDlgItem(hDlg, idc_groupbox[i]), L"", L"");
        }

        // Сделать некоторые элементы недоступными
        EnableWindow(GetDlgItem(hDlg, IDC_FUSES), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_FUSES_PROGRAM), FALSE);
        

        MakeWindowLook(hDlg, INIT_WINDOW);
        //MakeWindowLook(hDlg, SMALL_WINDOW);
        HWND button = GetDlgItem(hDlg, IDC_OPENFILE);
        
        
        //SetWindowSubclass(button, OwnerDrawButtonProc, 0, 0);
        return (INT_PTR)TRUE;
    }
    case WM_CTLCOLORSTATIC:
    {
        switch (GetWindowLong((HWND)lParam, GWL_ID))
        {
        case IDC_STATIC1: case IDC_STATIC2:
            return (INT_PTR)MakeControlGray((HDC)wParam);
        case IDC_STATIC3:  case IDC_STATIC4:
        case IDC_STATIC5:  case IDC_STATIC6:
        case IDC_STATIC7:  case IDC_STATIC8:
            return (INT_PTR)MakeControlBlue((HDC)wParam);
        case IDC_AVRDUDE1:
            return (INT_PTR)MakeControlWhiteBackground((HDC)wParam, bInfoFocus);
        default:break;
        }
        return (INT_PTR)FALSE;
    }
    
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDC_OPENFILE:
            {
                OPENFILENAME ofn;
                TCHAR szFile[MAX_PATH] = { 0 };       // buffer for file name
                TCHAR szCustom[MAX_PATH] = { 0 };
                TCHAR szTitle[100] = { 0 };
                TCHAR szDir[MAX_PATH] = { 0 };
                TCHAR** lppPart = { NULL };

                ZeroMemory(&ofn, sizeof(ofn));

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hDlg;
                ofn.hInstance = hInst;
                ofn.lpstrFilter = TEXT("Все файлы\0*\0Hex файлы(*.hex;*.eep)\0*.hex;*.eep\0");
                ofn.nFilterIndex = 2;
                ofn.lpstrCustomFilter = szCustom;                
                ofn.nMaxCustFilter = MAX_PATH;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
                ofn.lpstrDefExt = TEXT("hex");
                ofn.lpstrFileTitle = szTitle;
                ofn.nMaxFileTitle = 100;
                
                
               //
                if (GetOpenFileName(&ofn))
                {
                    currentHexFile = ofn.lpstrFile;
                    PathCchRemoveFileSpec(ofn.lpstrFile, MAX_PATH); // Здесь директория
                    SendDlgItemMessage(hDlg, IDC_HEX_FILENAME, WM_SETTEXT, 0, (LPARAM)ofn.lpstrFileTitle);

                    _tstring fn = ofn.lpstrFileTitle;
                    _tstring ext = fn.substr(fn.find_last_of(_T(".")) + 1);

                    if ( ext == _T("hex"))
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_FLASH_PROGRAM), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_FLASH_VERIFY), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_EEPROM_PROGRAM), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_EEPROM_VERIFY), FALSE);
                    }
                    else if (ext == _T("eep"))
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_EEPROM_PROGRAM), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_EEPROM_VERIFY), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_FLASH_PROGRAM), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_FLASH_VERIFY), FALSE);
                    }
                }
                //return TRUE;
                break;
            }
            case IDC_FLASH_READ:
            {
                
                OPENFILENAME ofn;
                TCHAR szFile[MAX_PATH] = { 0 };       // buffer for file name
                TCHAR szCustom[MAX_PATH] = { 0 };
                TCHAR szTitle[100] = { 0 };
                TCHAR szDir[MAX_PATH] = { 0 };
                TCHAR** lppPart = { NULL };

                ZeroMemory(&ofn, sizeof(ofn));

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hDlg;
                ofn.hInstance = hInst;
                ofn.lpstrFilter = TEXT("Все файлы\0*\0Hex файлы(*.hex)\0*.hex\0");
                ofn.nFilterIndex = 2;
                ofn.lpstrCustomFilter = szCustom;
                ofn.nMaxCustFilter = MAX_PATH;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
                ofn.lpstrDefExt = TEXT("hex");
                ofn.lpstrFileTitle = szTitle;
                ofn.nMaxFileTitle = 100;

                // Для начала откроем диалог
                if (GetSaveFileName(&ofn))
                {
                    //PathCchRemoveFileSpec(ofn.lpstrFile, MAX_PATH); // Здесь директория
                    SendDlgItemMessage(hDlg, IDC_HEX_FILENAME, WM_SETTEXT, 0, (LPARAM)ofn.lpstrFileTitle);

                    TCHAR tmp[2048] = { 0 };
                    SetCurrentDirectory(szCurDir);

                    TCHAR szCmdline[MAX_PATH] = { 0 };
                    // Следующая фаза - сформировать эту строчку
                    TCHAR szFormat[] = _T("avrdude.exe -C avrdude.conf %s -U flash:r:\"%s\":i -q");

                    PARAMETR myParam = GetParameters(hDlg);             
                    
                    _tstring param = MakeDudeParam(myParam);
                    
                    _stprintf_s(szCmdline, MAX_PATH, szFormat, param.c_str(), ofn.lpstrFile);

                    SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szCmdline);

                    if (!FlashRead(szCmdline, tmp, 2048))
                    {
                        TCHAR szText[2048] = { 0 };
                        _stprintf_s(szText, 2048, _T("%s\r\n\r\n\r\n%s"), szCmdline, tmp);
                        SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szText);
                    }
                    else {
                        SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Не могу прочитать флеш"));
                    }
                    
                }
                break;
            }
            case IDC_EEPROM_READ:
            {
                OPENFILENAME ofn;
                TCHAR szFile[MAX_PATH] = { 0 };       // buffer for file name
                TCHAR szCustom[MAX_PATH] = { 0 };
                TCHAR szTitle[100] = { 0 };
                TCHAR szDir[MAX_PATH] = { 0 };
                TCHAR** lppPart = { NULL };

                ZeroMemory(&ofn, sizeof(ofn));

                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hDlg;
                ofn.hInstance = hInst;
                ofn.lpstrFilter = TEXT("Все файлы\0*\0EEPROM файлы(*.eep)\0*.eep\0");
                ofn.nFilterIndex = 2;
                ofn.lpstrCustomFilter = szCustom;
                ofn.nMaxCustFilter = MAX_PATH;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
                ofn.lpstrDefExt = TEXT("eep");
                ofn.lpstrFileTitle = szTitle;
                ofn.nMaxFileTitle = 100;

                // Для начала откроем диалог
                if (GetSaveFileName(&ofn))
                {
                    //PathCchRemoveFileSpec(ofn.lpstrFile, MAX_PATH); // Здесь директория
                    SendDlgItemMessage(hDlg, IDC_HEX_FILENAME, WM_SETTEXT, 0, (LPARAM)ofn.lpstrFileTitle);

                    TCHAR tmp[2048] = { 0 };
                    SetCurrentDirectory(szCurDir);

                    TCHAR szCmdline[MAX_PATH] = { 0 };
                    // Следующая фаза - сформировать эту строчку
                    TCHAR szFormat[] = _T("avrdude.exe -C avrdude.conf %s -U eeprom:r:\"%s\":i -q");

                    PARAMETR myParam = GetParameters(hDlg);

                    _tstring param = MakeDudeParam(myParam);

                    _stprintf_s(szCmdline, MAX_PATH, szFormat, param.c_str(), ofn.lpstrFile);

                    SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szCmdline);
                    //EepromRead(ofn.lpstrFile);
                    if (!EepromRead(szCmdline, tmp, 2048))
                    {
                        TCHAR szText[2048] = { 0 };
                        _stprintf_s(szText, 2048, _T("%s\r\n\r\n\r\n%s"), szCmdline, tmp);
                        SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szText);
                    }
                    else {
                        SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Не могу прочитать EEPROM"));
                    }
                }
                break;
            }
            case IDC_FLASH_PROGRAM:
            {
                //TCHAR buf[MAX_PATH] = { 0 };
                //SendDlgItemMessage(hDlg, IDC_HEX_FILENAME, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)buf);

                if (currentHexFile.size()==0)
                {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Укажите файл"));
                    break;
                }
                TCHAR tmp[4096] = { 0 };
                

                TCHAR szCmdline[MAX_PATH] = { 0 };
                // Следующая фаза - сформировать эту строчку
                TCHAR szFormat[] = _T("avrdude.exe -C avrdude.conf %s -U flash:w:\"%s\":a -q");

                PARAMETR myParam = GetParameters(hDlg);
                _tstring param = MakeDudeParam(myParam);
                _stprintf_s(szCmdline, MAX_PATH, szFormat, param.c_str(), currentHexFile.c_str());
                
                SetCurrentDirectory(szCurDir);
                SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szCmdline);
                if (!FlashWrite(szCmdline, tmp, 4096))
                {
                    TCHAR szText[4096] = { 0 };
                    _stprintf_s(szText, 4096, _T("%s\r\n\r\n\r\n%s"), szCmdline, tmp);
                    SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szText);
                }
                else {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Ошибка записи флеш"));
                }
                break;
            }
            case IDC_EEPROM_PROGRAM:
            {
                if (currentHexFile.size() == 0)
                {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Укажите файл"));
                    break;
                }
                TCHAR tmp[2048] = { 0 };


                TCHAR szCmdline[MAX_PATH] = { 0 };
                // Следующая фаза - сформировать эту строчку
                TCHAR szFormat[] = _T("avrdude.exe -C avrdude.conf %s -U eeprom:w:\"%s\":a -q");

                PARAMETR myParam = GetParameters(hDlg);
                _tstring param = MakeDudeParam(myParam);
                _stprintf_s(szCmdline, MAX_PATH, szFormat, param.c_str(), currentHexFile.c_str());

                SetCurrentDirectory(szCurDir);
                SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szCmdline);
                if (!EepromWrite(szCmdline, tmp, 2048))
                {
                    TCHAR szText[2048] = { 0 };
                    _stprintf_s(szText, 2048, _T("%s\r\n\r\n\r\n%s"), szCmdline, tmp);
                    SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szText);
                }
                else {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Ошибка записи EEPROM"));
                }
                break;
            }
            case IDC_FLASH_VERIFY:
            {
                if (currentHexFile.size() == 0)
                {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Укажите файл"));
                    break;
                }
                TCHAR tmp[2048] = { 0 };

                TCHAR szCmdline[MAX_PATH] = { 0 };
                // Следующая фаза - сформировать эту строчку
                TCHAR szFormat[] = _T("avrdude.exe -C avrdude.conf %s -U flash:v:\"%s\":a -q");

                PARAMETR myParam = GetParameters(hDlg);
                _tstring param = MakeDudeParam(myParam);
                _stprintf_s(szCmdline, MAX_PATH, szFormat, param.c_str(), currentHexFile.c_str());

                SetCurrentDirectory(szCurDir);
                SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szCmdline);
                // Запустить поток, который принимает параметр
                DUDE dude = { szCmdline, _T("") };
                // Flash Verify
                HANDLE hDude = CreateThread(NULL, 0, &AvrDudeThreadProc, &dude, 0, NULL);

                WaitForSingleObject(hDude, INFINITE);

                DWORD retvalue;

                GetExitCodeThread(hDude, &retvalue);

                while (retvalue == STILL_ACTIVE) { Sleep(100); GetExitCodeThread(hDude, &retvalue); }
                                  
                if (!retvalue)
                {
                    _tstring text = dude.cmd + _T("\r\n\r\n\r\n") + dude.output;
                    SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)text.c_str());
                }
                else
                {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Ошибка записи EEPROM"));
                }
                break;
            }
            case IDC_EEPROM_VERIFY:
            {
                if (currentHexFile.size() == 0)
                {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Укажите файл"));
                    break;
                }
                TCHAR tmp[2048] = { 0 };


                TCHAR szCmdline[MAX_PATH] = { 0 };
                // Следующая фаза - сформировать эту строчку
                TCHAR szFormat[] = _T("avrdude.exe -C avrdude.conf %s -U eeprom:v:\"%s\":a -q");

                PARAMETR myParam = GetParameters(hDlg);
                _tstring param = MakeDudeParam(myParam);
                _stprintf_s(szCmdline, MAX_PATH, szFormat, param.c_str(), currentHexFile.c_str());

                SetCurrentDirectory(szCurDir);
                SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szCmdline);
                if (!EepromVerify(szCmdline, tmp, 2048))
                {
                    TCHAR szText[2048] = { 0 };
                    _stprintf_s(szText, 2048, _T("%s\r\n\r\n\r\n%s"), szCmdline, tmp);
                    SendDlgItemMessage(hDlg, IDC_AVRDUDE1, WM_SETTEXT, 0, (LPARAM)szText);
                }
                else {
                    SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT, 0, (LPARAM)TEXT("Ошибка записи EEPROM"));
                }
                break;
            }
            case IDC_SWAP_CONSOLE:            
                MakeWindowLook(hDlg, SWAP_WINDOW);                
                break;
            case IDC_FUSES_ADVANCED:
            {
                CAdvancedDlg dialog(hDlg);

                dialog.DoModal();                
            }
                break;
            case IDC_DEVICE:
            {
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    DWORD curSelection = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                    
                    // Проверить фьюзы
                    DEVICE &dev = vDeviceData[curSelection];

                    SendDlgItemMessage(hDlg, IDC_FUSES, CB_RESETCONTENT, 0, 0);
                    // Ищем устройство в базе фьюзов
                    for (auto tuple : vDevFuse)
                    {
                        _tstring &str = std::get<0>(tuple);
                        FUSE &fuse = std::get<1>(tuple);
                        if (str == dev.name) // Нашли
                        {
                            // Здесь проверим и выставим фьюзы
                            SendDlgItemMessage(hDlg, IDC_FUSES, CB_ADDSTRING, 0, (LPARAM)fuse.name);                            
                        }
                    }
                    DWORD cnt = SendDlgItemMessage(hDlg, IDC_FUSES, CB_GETCOUNT, 0, 0);
                    SendDlgItemMessage(hDlg, IDC_FUSES, CB_SETCURSEL, (WPARAM)0, 0L);
                    // Сделать некоторые элементы недоступными
                    EnableWindow(GetDlgItem(hDlg, IDC_FUSES), cnt > 0); 
                    EnableWindow(GetDlgItem(hDlg, IDC_FUSES_PROGRAM), cnt > 0);
                    bParamChange = TRUE;
                }
                break;
            }
            case IDC_PROGRAMMER:
            {
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    DWORD curSelection = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
               
                    PROG& curprog = vProgData[curSelection];

                    static bool lastid = true;
                    bool found = false;
                    for (auto tuple : vProgPort)
                    {
                        PROG& str = std::get<0>(tuple);
                        PORT& port = std::get<1>(tuple);
                        if (str == curprog) // Нашли
                        {
                            if (!found)
                                SendDlgItemMessage(hDlg, IDC_PORT, CB_RESETCONTENT, 0, 0);
                            found = true;
                            // Здесь проверим и выставим порты
                            SendDlgItemMessage(hDlg, IDC_PORT, CB_ADDSTRING, 0, (LPARAM)port.name);
                        }
                    }
                    
                    if (!found && lastid)
                    {
                        // делаем по дефолту
                        SendDlgItemMessage(hDlg, IDC_PORT, CB_RESETCONTENT, 0, 0);
                        for (auto port : vPortData)
                        {
                            SendDlgItemMessage(hDlg, IDC_PORT, CB_ADDSTRING, 0, (LPARAM)port.name);
                        }
                    }
                    lastid = found;
                    //DWORD cnt = SendDlgItemMessage(hDlg, IDC_PORT, CB_GETCOUNT, 0, 0);
                    SendDlgItemMessage(hDlg, IDC_PORT, CB_SETCURSEL, (WPARAM)0, 0L);
                    bParamChange = TRUE;
                }
                break;
            }
            case IDC_PORT:
            case IDC_BR_SPEED:
            {
                if (HIWORD(wParam) == CBN_SELCHANGE)
                {
                    bParamChange = TRUE;
                }
                break;
            }
            case IDC_AVRDUDE1:
            {
                if (HIWORD(wParam) == EN_SETFOCUS)
                {
                    bInfoFocus = TRUE;
                }
                else if (HIWORD(wParam) == EN_KILLFOCUS)
                {
                    bInfoFocus = FALSE;
                }
                break;
            }
            case IDCANCEL:
            case IDOK:
                DestroyWindow(hDlg);
                break;
            default:
                return DefWindowProc(hDlg, message, wParam, lParam);
            }
        }
        break;
    
    case WM_CLOSE:
        {
            // EndDialog( hwnd, 0 ); -- DONT DO THAT! 
            // EndDialog is valid ONLY for Modal Dialogs, created with DialogBox(Param)
            DestroyWindow(hDlg);
            break;
        }
    case WM_DESTROY:
        {
            //DeleteObject(hbrBkgnd);
            PostQuitMessage(0);
            break;
        }
    
    }

    return (INT_PTR)FALSE;
}
LRESULT CALLBACK OwnerDrawButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    static int state = 0;
    switch (uMsg)
    {
    case WM_MOUSEMOVE: {
        TRACKMOUSEEVENT eventTrack;
        eventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
        eventTrack.dwFlags = TME_HOVER | TME_LEAVE;
        eventTrack.hwndTrack = hWnd;
        eventTrack.dwHoverTime = 50;
        TrackMouseEvent(&eventTrack);
    } break;
    case WM_MOUSEHOVER: {
        RECT rect = { 0,0, 23,20 };

        state = 1;
        
        return InvalidateRect(hWnd, &rect, FALSE);
    }
    case WM_MOUSELEAVE:
    {
        RECT rect = { 0,0, 23,20 };

        state = 0;
        
        return InvalidateRect(hWnd, &rect, FALSE);
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BITMAP          bitmap;
        HDC             hdcMem;
        HGDIOBJ         oldBitmap;
        HDC hDC = BeginPaint(hWnd, &ps);

        HBITMAP hImg = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OPENFILE));
        hdcMem = CreateCompatibleDC(hDC);
        oldBitmap = SelectObject(hdcMem, hImg);

        GetObject(hImg, sizeof(bitmap), &bitmap);


        HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        HPEN hPen = CreatePen(PS_SOLID, 0, (state==1?RGB(100, 100, 200):RGB(160,160,160)));
        HGDIOBJ hOldPen = SelectObject(hDC, hPen);
        HGDIOBJ hOldBrush = SelectObject(hDC, hBrush);
        RECT rect = { 0,0, 23,20 };
        //DrawFrameControl(hDC, &rect, DFC_BUTTON, 0);
        Rectangle(hDC, 0, 0, 23, 20);
        TransparentBlt(hDC, 3, 3, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0,
            bitmap.bmWidth, bitmap.bmHeight, RGB(255, 255, 255));
        SelectObject(hDC, hOldBrush);
        SelectObject(hdcMem, oldBitmap);
        SelectObject(hDC, hOldPen);
        DeleteDC(hdcMem);
        DeleteObject(hBrush);
        DeleteObject(hPen);
        EndPaint(hWnd, &ps);
    }
        
        return TRUE;
        // Other cases...
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}