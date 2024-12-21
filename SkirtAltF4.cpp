#include <windows.h>
#include <psapi.h>
#include <shellapi.h>
#include <string>
#include <tchar.h>
#include "resource.h"

#define TRAY_ICON_ID 1
#define WM_TRAY (WM_USER + 1)

// Global variables
HINSTANCE hInst;
NOTIFYICONDATA nid;
bool running = true;

DWORD GetForegroundProcessID() {
    HWND hwnd = GetForegroundWindow();  // Get handle of the foreground window
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);  // Get the process ID
    return pid;
}

void KillForegroundProcess() {
    DWORD pid = GetForegroundProcessID();
    if (pid == 0) return;

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess) {
        TerminateProcess(hProcess, 1);  // Forcefully terminate the process
        CloseHandle(hProcess);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TRAY:
        if (lParam == WM_RBUTTONUP) {
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, 1, _T("Ewxit me >:3"));
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
            if (cmd == 1) {
                running = false;
                PostQuitMessage(0);
            }
        }
        break;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid); // Remove tray icon on exit
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// P.S don't ever make the shortcut ":3"
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (p->vkCode == VK_F3 && (GetAsyncKeyState(VK_MENU) & 0x8000)) {  // Alt+F3 pressed :3
            KillForegroundProcess();  // Kill the active process
            return 1;  // Block the default Alt+F4 behavior
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("UwseMe3"); // Updated to UwseMe3
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(_T("UwseMe3"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
    _tcscpy_s(nid.szTip, _T("Uwse me :3"));  // Tooltip text updated
    Shell_NotifyIcon(NIM_ADD, &nid);

    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);

    MSG msg;
    while (running && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    Shell_NotifyIcon(NIM_DELETE, &nid);
    return 0;
}
