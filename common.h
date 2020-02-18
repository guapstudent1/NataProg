#pragma once
#include "framework.h"

#define INIT_WINDOW  0
#define SMALL_WINDOW 1
#define WIDE_WINDOW  2
#define SWAP_WINDOW  3

void				MakeWindowLook(HWND, int);
HBRUSH              MakeControlGray(HDC hDC);
HBRUSH              MakeControlBlue(HDC hDC);

template< typename CallerT >
inline CallerT* GetDialogCaller(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (WM_INITDIALOG != uMsg) {
		// Retrieves information about the specified window.
		// 1. A handle to the window and, indirectly, the class to which the window belongs.
		// 2. Retrieves the user data associated with the window.
		return reinterpret_cast<CallerT*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
	}

	CallerT* const caller = reinterpret_cast<CallerT*>(lParam);
	
	// Здесь можно заранее установить хендл окна класса, вызывается при инициализации
	caller->SetHwnd(hwndDlg);

	// Changes an attribute of the specified window.
	// 1. A handle to the window and, indirectly, the class to which the window belongs.
	// 2. Sets the user data associated with the window.
	// 3. The replacement value.
	SetWindowLongPtr(hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(caller));

	return caller;
}
