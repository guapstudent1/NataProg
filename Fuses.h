#pragma once

#include "resource.h"
#include "common.h"


class CAdvancedDlg {
	HWND hWndParent;
	HWND hwndDlg;
	INT_PTR CALLBACK DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	
	void OnInitDialog();
public:
	CAdvancedDlg(HWND hParent) : hWndParent(hParent) {}
	CAdvancedDlg() :hWndParent(0) {}
	void SetHwnd(HWND hWnd) { hwndDlg = hWnd; }

	INT_PTR CALLBACK		DoModal();
	static INT_PTR CALLBACK DialogProcDelegate(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		// Retrieve a pointer to the instance of Widget
		// that called DialogBoxParam.
		CAdvancedDlg* const widget = GetDialogCaller<CAdvancedDlg>(hwnd, uMsg, wParam, lParam);
		
		
		// Delegate the message handling.
		return widget->DialogProc(uMsg, wParam, lParam);
	}
};
