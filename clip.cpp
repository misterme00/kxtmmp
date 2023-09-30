#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>

HWND hWndNextViewer = NULL;

LRESULT CALLBACK ClipboardViewerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DRAWCLIPBOARD:
    {
        // Clipboard content has changed
        HANDLE hClipboardData = GetClipboardData(CF_TEXT);
        if (hClipboardData != NULL)
        {
            char* pszClipboardText = static_cast<char*>(GlobalLock(hClipboardData));
            if (pszClipboardText != NULL)
            {
                // Get the window title
                char szWindowTitle[256];
                HWND hWnd = GetForegroundWindow();
                GetWindowText(hWnd, szWindowTitle, sizeof(szWindowTitle));

                // Get the process ID
                DWORD dwProcessId;
                GetWindowThreadProcessId(hWnd, &dwProcessId);

                // Get the timestamp
                time_t now = time(0);
                struct tm tstruct;
                char buf[80];
                localtime_s(&tstruct, &now);
                strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

                // Log the clipboard data along with window information and timestamp
                std::ofstream logFile("clipboard_log.txt", std::ios::app);
                logFile << "Window Title: " << szWindowTitle << std::endl;
                logFile << "Process ID: " << dwProcessId << std::endl;
                logFile << "Timestamp: " << buf << std::endl;
                logFile << "Clipboard Text: " << pszClipboardText << std::endl << std::endl;
                logFile.close();

                GlobalUnlock(hClipboardData);
            }
        }
        break;
    }
    case WM_CHANGECBCHAIN:
        // The next viewer's window handle is being removed
        if ((HWND)wParam == hWndNextViewer)
            hWndNextViewer = (HWND)lParam;
        else if (hWndNextViewer != NULL)
            SendMessage(hWndNextViewer, uMsg, wParam, lParam);
        break;
    }

    return CallNextHookEx(NULL, uMsg, wParam, lParam);
}

int main()
{
    // Register clipboard viewer
    hWndNextViewer = SetClipboardViewer(NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unregister clipboard viewer
    ChangeClipboardChain(NULL, hWndNextViewer);
    return 0;
}
