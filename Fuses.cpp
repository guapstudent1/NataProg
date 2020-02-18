#include "framework.h"
#include "Fuses.h"

INT_PTR CALLBACK CAdvancedDlg::DoModal()
{
    return DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ADVANCED), 
        hWndParent,
        DialogProcDelegate, LPARAM(this));
}
void CAdvancedDlg::OnInitDialog()
{
    HICON hIconSmall = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SMALL));    

    SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);    
    // 
    HMENU hSysMenu = GetSystemMenu(hwndDlg, FALSE);
    EnableMenuItem(hSysMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
    LONG idc_groupbox[] = {
        IDC_STATIC3, IDC_STATIC4, IDC_STATIC5,
        IDC_STATIC6, IDC_STATIC7, IDC_STATIC8
    }; // 
    // 
    for (int i = 0; i < sizeof(idc_groupbox) / sizeof(idc_groupbox[0]); i++)
    {
        SetWindowTheme(GetDlgItem(hwndDlg, idc_groupbox[i]), L"", L"");
    }
    // 
    MakeWindowLook(hwndDlg, INIT_WINDOW);
    SetDlgItemText(hwndDlg, IDC_SIGNATURE, TEXT("00000000"));
    SetDlgItemText(hwndDlg, IDC_CALIBRATION, TEXT("00000000"));

}
INT_PTR CALLBACK CAdvancedDlg::DialogProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    
    switch (message) {
    case WM_INITDIALOG: 

        OnInitDialog();
        break;
    case WM_COMMAND:
    {
        //OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
        int wmId = LOWORD(wParam);
        //
        switch (wmId)
        {
        case IDC_CLOSE_ADVANCED:
            return EndDialog(hwndDlg, IDOK);
        case IDC_SWAP_CONSOLE:
            MakeWindowLook(hwndDlg, SWAP_WINDOW);
            break;
        default:
            return DefWindowProc(hwndDlg, message, wParam, lParam);
        }
    }
    break;
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
        default:break;
        }
        return (INT_PTR)FALSE;
    }
    case WM_CLOSE:
        return FALSE;
        //return EndDialog(hDlg, IDOK);
    default:
        return FALSE;
    }
    return TRUE;
}
  /*
case WM_DRAWITEM: {
    WORD wID = (WORD)wParam;

    if (wID == IDC_OPENFILE) {
        DRAWITEMSTRUCT& dis = *(DRAWITEMSTRUCT*)lParam;
        // Focus change?
        if (dis.itemAction & ODA_FOCUS) {
            // Toggle focus rectangle
            DrawFocusRect(dis.hDC, &dis.rcItem);
        }
        else if (dis.itemAction & ODA_DRAWENTIRE) {
            // Not a focus change -> render rectangle if requested
           
            BITMAP          bitmap;
            HDC             hdcMem;
            HGDIOBJ         oldBitmap;

            HBITMAP hImg = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OPENFILE));
            hdcMem = CreateCompatibleDC(dis.hDC);
            oldBitmap = SelectObject(hdcMem, hImg);

            GetObject(hImg, sizeof(bitmap), &bitmap);


            HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));

            HGDIOBJ hOldBrush = SelectObject(dis.hDC, hBrush);

            Rectangle(dis.hDC, dis.rcItem.left,dis.rcItem.top, dis.rcItem.right, dis.rcItem.bottom);
            TransparentBlt(dis.hDC, 3, 3, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0,
                bitmap.bmWidth, bitmap.bmHeight, RGB(255, 255, 255));
            SelectObject(dis.hDC, hOldBrush);
            SelectObject(hdcMem, oldBitmap);
            DeleteDC(hdcMem);
            DeleteObject(hBrush);

            if (dis.itemState & ODS_FOCUS) {
                DrawFocusRect(dis.hDC, &dis.rcItem);
            }
        }
        return (INT_PTR)TRUE;
    }

}
*/