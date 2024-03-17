#define WM_ICON WM_APP
#define ID_TRAYICON WM_USER
#include "WaitableTimerDlg.h"

CWaitableTimerDlg* CWaitableTimerDlg::ptr = NULL;





CWaitableTimerDlg::CWaitableTimerDlg(void)
{
	ptr = this;
	pNID = new NOTIFYICONDATA;
}

CWaitableTimerDlg::~CWaitableTimerDlg(void)
{
	delete pNID;
}

void CWaitableTimerDlg::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}

BOOL CWaitableTimerDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hSpin1 = GetDlgItem(hwnd, IDC_SPIN1);
	hEdit4 = GetDlgItem(hwnd, IDC_EDIT4);
	hSpin2 = GetDlgItem(hwnd, IDC_SPIN2);
	hEdit2 = GetDlgItem(hwnd, IDC_EDIT2);
	hEdit3 = GetDlgItem(hwnd, IDC_EDIT3);
	hButton = GetDlgItem(hwnd, IDC_BUTTON1);
	hList1 = GetDlgItem(hwnd, IDC_LIST1);
	hList2 = GetDlgItem(hwnd, IDC_LIST2);
	hDialog = hwnd;

	
	SendMessage(hSpin1, UDM_SETRANGE32, 0, 23);
	SendMessage(hSpin2, UDM_SETRANGE32, 0, 59);


	return TRUE;
}

DWORD WINAPI Thread(LPVOID lp)
{
	CWaitableTimerDlg* p = (CWaitableTimerDlg*)lp;
	TCHAR buffer[256];
	GetWindowText(p->hEdit3, buffer, 256);
	SendMessage(p->hList1, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(buffer));

	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	TCHAR buf[10];
	int hours, minutes;
	GetWindowText(p->hEdit2, buf, 10);
	hours = _tstoi(buf);
	GetWindowText(p->hEdit3, buf, 10);
	minutes = _tstoi(buf);

	SYSTEMTIME st;
	GetLocalTime(&st);

	if (st.wHour > hours || st.wHour == hours && st.wMinute > minutes ||
		st.wHour == hours && st.wMinute == minutes)
	{
		CloseHandle(hTimer);
		EnableWindow(p->hButton, TRUE);
		EnableWindow(p->hEdit2, TRUE);
		EnableWindow(p->hEdit4, TRUE);
		return 0;
	}
	st.wHour = hours;
	st.wMinute = minutes;
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	LocalFileTimeToFileTime(&ft, &ft);

	SetWaitableTimer(hTimer, (LARGE_INTEGER*)&ft, 0, NULL, NULL, FALSE);
	if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0)
	{
		int result = MessageBox(p->hDialog, buffer, _T("Время выполнения задачи?"), MB_OKCANCEL | MB_ICONQUESTION);
		if (result == IDOK) {
			SendMessage(p->hList1, LB_DELETESTRING, 0, 0);
		}
		else {
			SendMessage(p->hList1, LB_DELETESTRING, 0, 0);
			SendMessage(p->hList2, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(buffer));
		}

		SetForegroundWindow(p->hDialog);
		for (int i = 0; i < 3; i++)
		{
			Beep(1000, 500);
			Sleep(1000);
		}
	}
	CancelWaitableTimer(hTimer);
	CloseHandle(hTimer);
	EnableWindow(p->hButton, TRUE);
	EnableWindow(p->hEdit2, TRUE);
	EnableWindow(p->hEdit4, TRUE);

	return 0;
}


void CWaitableTimerDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BUTTON1) {


		TCHAR taskName[256];
		TCHAR timeBuffer[10];
		GetWindowText(hEdit3, taskName, 256);
		GetWindowText(hEdit2, timeBuffer, 10);
		int hours = _ttoi(timeBuffer);
		GetWindowText(hEdit4, timeBuffer, 10);
		int minutes = _ttoi(timeBuffer);

	
		TCHAR taskInfo[256];
		_stprintf_s(taskInfo, _T("%s - %02d:%02d"), taskName, hours, minutes);
		SendMessage(hList1, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(taskInfo));

		/*HANDLE h;
		h = CreateThread(NULL, 0, Thread, this, 0, NULL);
		CloseHandle(h);
		EnableWindow(hButton, FALSE);
		EnableWindow(hEdit2, FALSE);
		EnableWindow(hEdit3, FALSE);*/
	}
	if (id == IDC_BUTTON2) {
		int selIndex = SendMessage(hList1, LB_GETCURSEL, 0, 0);
		if (selIndex != LB_ERR) {
			SendMessage(hList1, LB_DELETESTRING, selIndex, 0);
		}
	}
	if (id == IDC_BUTTON3)
	{
	}
}

// обработчик пользовательского сообщения
void CWaitableTimerDlg::OnTrayIcon(WPARAM wp, LPARAM lp)
{
	// WPARAM - идентификатор иконки
	// LPARAM - сообщение от мыши или клавиатурное сообщение
	if (lp == WM_LBUTTONDBLCLK)
	{
		Shell_NotifyIcon(NIM_DELETE, pNID); // Удаляем иконку из трэя
		ShowWindow(hDialog, SW_NORMAL); // Восстанавливаем окно
		SetForegroundWindow(hDialog); // устанавливаем окно на передний план
	}
}


INT_PTR CALLBACK CWaitableTimerDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
		//HANDLE_MSG(hwnd, WM_SIZE, ptr->Cls_OnSize);
	}
	
	if (message == WM_ICON)
	{
		ptr->OnTrayIcon(wParam, lParam);
		return TRUE;
	}
	return FALSE;
}