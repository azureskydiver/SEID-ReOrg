#ifndef WINPROC_H_INCLUDED
#define WINPROC_H_INCLUDED
#include <Windows.h>

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
    virtual LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    virtual void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
    virtual void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
    virtual BOOL OnQueryNewPalette(HWND hwnd);
    virtual void OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange);
    virtual void OnDrawClipboard(HWND hwnd);
    virtual void OnChangeCBChain(HWND hwnd, HWND hwndRemove, HWND hwndNext);
    virtual void OnCommand_Open_BM(HWND hwnd);
    virtual void OnCommand_Print_BM(HWND hwnd);
    virtual void OnCommand_Save_BM(HWND hwnd);
    virtual void OnCommand_Exit(HWND hwnd);
    virtual void OnCommand_Copy(HWND hwnd);
    virtual void OnCommand_Cut(HWND hwnd);
    virtual void OnCommand_Paste(HWND hwnd);
    virtual void OnCommand_ZoomOut(HWND hwnd);
    virtual void OnCommand_ZoomIn(HWND hwnd);
    virtual void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    virtual void OnPaint(HWND hwnd);
    virtual void OnSize(HWND hwnd, UINT state, int cx, int cy);
    virtual void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
    virtual void OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
    virtual void OnDestroy(HWND hwnd);
};

#endif // WINPROC_H_INCLUDED