#ifndef WINPROC_H_INCLUDED
#define WINPROC_H_INCLUDED
#include <Windows.h>

class MainWindow
{
public:
    MainWindow();
    ~MainWindow();

    HWND Create(HINSTANCE hinstance);

    static BOOL RegisterClass(HINSTANCE hinstance);

protected:
    static LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

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

    HDC GetPrinterDC (HWND Hwnd);
    BOOL OpenFileDialog(HWND hwnd, LPTSTR pFileName ,LPTSTR pTitleName);
    BOOL SaveFileDialog(HWND hwnd, LPTSTR pFileName ,LPTSTR pTitleName);
    void InitialiseDialog(HWND hwnd);
    void SaveBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
    PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
    bool BitmapToClipboard(HBITMAP hBM, HWND hWnd);
    HBITMAP CopyScreenToBitmap(HWND Hwnd,int x1, int y1, int nWidth, int nHeight);
    void PaintLoadBitmap(HWND hwnd,SCROLLINFO si, BITMAP bitmap, int pcxsize, int pcysize, int xMaxScroll,int xCurrentScroll, int xMinScroll,int yMaxScroll, int yCurrentScroll ,int yMinScroll);
    void DrawBoundingBox(HDC hdc, int StartxPos, int EndxPos, int StartyPos, int EndyPos);
    void Cut(HDC hdc, HDC hdcMem, HBITMAP hBitmap, double Width, double Height, double oWidth, double oHeight, double StartxPos, double EndxPos, double StartyPos, double EndyPos, int xCurrentScroll, int yCurrentScroll);
    void Paste(HWND hwnd, HBITMAP hBitmap,double Width,double Height ,double oWidth, double oHeight ,HDC hdc, HDC hdcMem, HDC MemoryDC, double StartxPos, double EndxPos, int xCurrentScroll, double StartyPos, double EndyPos, int yCurrentScroll );
    void ZeroScrollbars(HWND hwnd, SCROLLINFO si, BITMAP bitmap, int cxsize, int cysize,int xCurrentScroll, int yCurrentScroll, int xMaxScroll, int yMaxScroll, int xMinScroll, int yMinScroll);

    static char szClassName[];

    HWND hwnd;
    HPALETTE hpal;
    HMENU menu;
    HBITMAP hBitmap;
    double bxWidth, bxHeight;
    double bxWidth2, bxHeight2;
    double zoom;
    bool fZoom;
    int cxsize, cxpage;
    int cysize, cypage;
    HDC hdc,hdcMem;
    RECT rect,WinRect;
    SCROLLINFO si;
    BITMAP bitmap;
    double StartxPos;
    double StartyPos;
    double EndxPos;
    double EndyPos;

    // These variables are required by BitBlt.

    BOOL fBlt;
    BOOL fLoad;           // TRUE if BitBlt occurred
    BOOL fScroll;         // TRUE if scrolling occurred
    BOOL fSize;           // TRUE if fBlt & WM_SIZE

    // These variables are required for horizontal scrolling.
    int xMinScroll;       // minimum horizontal scroll value
    int xCurrentScroll;   // current horizontal scroll value
    int xMaxScroll;       // maximum horizontal scroll value

    // These variables are required for vertical scrolling.
    int yMinScroll;       // minimum vertical scroll value
    int yCurrentScroll;   // current vertical scroll value
    int yMaxScroll;       // maximum vertical scroll value
    HWND hwndViewer;

    // These variables are required by BitBlt.

    BOOL mmov;
    BOOL cut;
    BOOL paste;

    OPENFILENAME ofn;
    char szFileName[500];
};

#endif // WINPROC_H_INCLUDED
