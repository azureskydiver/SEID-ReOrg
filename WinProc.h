#ifndef WINPROC_H_INCLUDED
#define WINPROC_H_INCLUDED
#include <Windows.h>

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif // WINPROC_H_INCLUDED