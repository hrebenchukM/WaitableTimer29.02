#define WM_ICON WM_APP
#define ID_TRAYICON WM_USER
#include "WaitableTimerDlg.h"


INT_PTR CALLBACK SecondDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


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
	hDialog = hwnd;
	hSpin1 = GetDlgItem(hwnd, IDC_SPIN1);
	hEdit4 = GetDlgItem(hwnd, IDC_EDIT4);
	hSpin2 = GetDlgItem(hwnd, IDC_SPIN2);
	hEdit2 = GetDlgItem(hwnd, IDC_EDIT2);
	hEdit3 = GetDlgItem(hwnd, IDC_EDIT3);
	hButton = GetDlgItem(hwnd, IDC_BUTTON1);
	hList1 = GetDlgItem(hwnd, IDC_LIST1);
	hList2 = GetDlgItem(hwnd, IDC_LIST2);

	//pNID = new NOTIFYICONDATA;
	
	SendMessage(hSpin1, UDM_SETRANGE32, 0, 23);
	SendMessage(hSpin2, UDM_SETRANGE32, 0, 59);

	// Получим дескриптор экземпляра приложения
	HINSTANCE hInst = GetModuleHandle(NULL);

	hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)); // загружаем иконку
	SetClassLong(hDialog, GCL_HICON, LONG(hIcon)); // устанавливаем иконку в главном окне приложения
	memset(pNID, 0, sizeof(NOTIFYICONDATA)); //Обнуление структуры
	pNID->cbSize = sizeof(NOTIFYICONDATA); //размер структуры
	pNID->hIcon = hIcon; //загружаем пользовательскую иконку
	pNID->hWnd = hwnd; //дескриптор окна, которое будет получать уведомляющие сообщения,
	// ассоциированные с иконкой в трэе.	
	lstrcpy(pNID->szTip, TEXT("Будильник")); // Подсказка

	pNID->uCallbackMessage = WM_ICON; // Пользовательское сообщение
	// Система использует этот идентификатор для посылки уведомляющих
	// сообщений окну, дескриптор которого хранится в поле hWnd. Эти сообщения
	// посылаются, когда происходит "мышиное" сообщение в прямоугольнике, где
	// расположена иконка, или иконка выбирается или активизируется с помощью
	// клавиатуры. Параметр сообщения wParam содержит при этом идентификатор
	// иконки в трэе, где произошло событие, а параметр сообщения lParam - 
	// "мышиное" или клавиатурное сообщение, ассоциированное с событием.
	// Пример события: щелчок мышки по иконке в трэе.

	pNID->uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE | NIF_INFO;
	// NIF_ICON - поле hIcon содержит корректное значение (позволяет создать иконку в трэе).
	// NIF_MESSAGE - поле uCallbackMessage содержит корректное значение
	// (позволяет получать сообщения от иконки в трэе).
	// NIF_TIP - поле szTip содержит корректное значение (позволяет создать всплывающую подсказку для иконки в трэе).
	// NIF_INFO - поле szInfo содержит корректное значение (позволяет создать Balloon подсказку для иконки в трэе).
	lstrcpy(pNID->szInfo, TEXT("Приложение демонстрирует работу таймера синхронизации"));
	lstrcpy(pNID->szInfoTitle, TEXT("Будильник!"));
	pNID->uID = ID_TRAYICON; // предопределённый идентификатор иконки
	return TRUE;
}



DWORD WINAPI Thread(LPVOID lp) {
	CWaitableTimerDlg* p = (CWaitableTimerDlg*)lp;
	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);// создаем таймер синхронизации
	TCHAR buffer[256];
	GetWindowText(p->hEdit3, buffer, 256);



	TCHAR timeBuffer[10];
	int hours, minutes;
	int seconds = 0;

	GetWindowText(p->hEdit2, timeBuffer, 10);
	hours = _tstoi(timeBuffer);
	GetWindowText(p->hEdit4, timeBuffer, 10);
    minutes = _tstoi(timeBuffer);

	TCHAR taskInfo[256];
	_stprintf_s(taskInfo, _T("%s - %02d:%02d"), buffer, hours, minutes);

	SendMessage(p->hList1, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(taskInfo));

	

	SYSTEMTIME st;
	GetLocalTime(&st); // получим текущее локальное время
	if (st.wHour > hours || st.wHour == hours && st.wMinute > minutes ||
		st.wHour == hours && st.wMinute == minutes && st.wSecond > seconds)
	{
     	CloseHandle(hTimer);
		EnableWindow(p->hButton, TRUE);
		EnableWindow(p->hEdit2, TRUE);
		EnableWindow(p->hEdit4, TRUE);
		return 0;
	}


	st.wHour = hours;
	st.wMinute = minutes;
	st.wSecond = seconds;
	FILETIME ft;



	SystemTimeToFileTime(&st, &ft); // преобразуем структуру SYSTEMTIME в FILETIME
	LocalFileTimeToFileTime(&ft, &ft); // преобразуем местное время в UTC-время 
	SetWaitableTimer(hTimer, (LARGE_INTEGER*)&ft, 0, NULL, NULL, FALSE); // устанавливаем таймер



    // ожидаем переход таймера в сигнальное состояние
	if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0) {
		
		DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG2), p->hDialog, SecondDialogProc); // Открываем второе окно
	}




	


	CancelWaitableTimer(hTimer); // отменяем таймер
	CloseHandle(hTimer); // закрываем дескриптор таймера
	EnableWindow(p->hButton, TRUE);
	EnableWindow(p->hEdit2, TRUE);
	EnableWindow(p->hEdit3, TRUE);
	EnableWindow(p->hEdit4, TRUE);

	return 0;
}



void CWaitableTimerDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_BUTTON1) {


	

		HANDLE h;
		h = CreateThread(NULL, 0, Thread, this, 0, NULL);
		CloseHandle(h);
		EnableWindow(hButton, FALSE);
		EnableWindow(hEdit2, FALSE);
		EnableWindow(hEdit3, FALSE);
		EnableWindow(hEdit4, FALSE);
		Shell_NotifyIcon(NIM_ADD, pNID); // Добавляем иконку в трэй
	}
	if (id == IDC_BUTTON2) {
		int selIndex = SendMessage(hList1, LB_GETCURSEL, 0, 0);
		if (selIndex != LB_ERR) {
			SendMessage(hList1, LB_DELETESTRING, selIndex, 0);
		}
	}
	if (id == IDC_BUTTON3)
	{
		int selIndex = SendMessage(hList1, LB_GETCURSEL, 0, 0);
		if (selIndex != LB_ERR) {
			TCHAR taskInfo[256];
			GetWindowText(hEdit2, taskInfo, 256); 
			lstrcat(taskInfo, _T(":"));
			TCHAR timeBuffer[10];
			GetWindowText(hEdit4, timeBuffer, 10); 
			lstrcat(taskInfo, timeBuffer);

			SendMessage(hList1, LB_DELETESTRING, selIndex, 0);
			SendMessage(hList1, LB_INSERTSTRING, selIndex, reinterpret_cast<LPARAM>(taskInfo)); 

			// Перезапускаем таймер для обновленной задачи
			HANDLE hThread = CreateThread(NULL, 0, Thread, this, 0, NULL);
			CloseHandle(hThread);

			SetWindowText(hEdit2, _T("")); 
			SetWindowText(hEdit4, _T("")); 
		}
	
	}
}




void CWaitableTimerDlg::Cls_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state == SIZE_MINIMIZED)
	{
		ShowWindow(hwnd, SW_HIDE); // Прячем окно
		Shell_NotifyIcon(NIM_ADD, pNID); // Добавляем иконку в трэй
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




BOOL CALLBACK CWaitableTimerDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
		HANDLE_MSG(hwnd, WM_SIZE, ptr->Cls_OnSize);
	}
	// пользовательское сообщение
	if (message == WM_ICON)
	{
		ptr->OnTrayIcon(wParam, lParam);
		return TRUE;
	}
	return FALSE;
}


INT_PTR CALLBACK SecondDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hDialog2;
	HWND hButtonOK;
	HWND hButtonNOTOK;

	 //HWND hList1;
	 //HWND hList2;

	
	switch (message)
	{
	case WM_INITDIALOG:
		
		hDialog2 = hwnd;
		hButtonOK = GetDlgItem(hwnd, IDGOOD);
		hButtonNOTOK = GetDlgItem(hwnd, IDNOTOK);
		/*hList1 = GetDlgItem(hwnd, IDC_LIST1);
		hList2 = GetDlgItem(hwnd, IDC_LIST2);*/

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDGOOD:
		{

			/*int selIndex = SendMessage(hList1, LB_GETCURSEL, 0, 0);
			if (selIndex != LB_ERR)
			{
				TCHAR taskInfo[256];
				SendMessage(hList1, LB_GETTEXT, selIndex, reinterpret_cast<LPARAM>(taskInfo));

				SendMessage(hList2, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(taskInfo));
				SendMessage(hList1, LB_DELETESTRING, selIndex, 0);
			}*/
			
				
			}
			break;
		case IDNOTOK:
			break;
		case IDCANCEL:
			EndDialog(hwnd, 0);
			return TRUE;
		}
		break;
		}

		return FALSE;
}
