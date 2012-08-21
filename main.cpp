#include <Windows.h>
#include "WinProc.h"
#include "resource.h"

/*Start of Program Entry point*/

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    MainWindow mainwindow;

    /* Register the window class, and if it fails quit the program */
    if (!MainWindow::RegisterClass(hThisInstance))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = mainwindow.Create(hThisInstance);

    /* Make the window visible on the screen */
    ShowWindow (hwnd, SW_MAXIMIZE);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}
