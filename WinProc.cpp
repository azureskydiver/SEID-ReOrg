#include "WinProc.h"
#include "Functions.h"
#include "resource.h"
#include <cmath>
#include <algorithm>

HPALETTE hpal;
HMENU menu;
HBITMAP hBitmap;
double bxWidth, bxHeight;
double bxWidth2, bxHeight2;
double zoom =0;
bool fZoom = false;
int cxsize = 0, cxpage = 0;
int cysize = 0, cypage = 0;
HDC hdc,hdcMem;
RECT rect,WinRect;
SCROLLINFO si;
BITMAP bitmap;
double StartxPos;
double StartyPos;
double EndxPos;
double EndyPos;

// "Virtual" globals

// These variables are required by BitBlt.

static BOOL fBlt;
static BOOL fLoad;           // TRUE if BitBlt occurred
static BOOL fScroll;         // TRUE if scrolling occurred
static BOOL fSize;           // TRUE if fBlt & WM_SIZE

// These variables are required for horizontal scrolling.
static int xMinScroll;       // minimum horizontal scroll value
static int xCurrentScroll;   // current horizontal scroll value
static int xMaxScroll;       // maximum horizontal scroll value

// These variables are required for vertical scrolling.
static int yMinScroll;       // minimum vertical scroll value
static int yCurrentScroll;   // current vertical scroll value
static int yMaxScroll;       // maximum vertical scroll value
static HWND hwndViewer;

// These variables are required by BitBlt.

static BOOL mmov;
static BOOL cut;
static BOOL paste;

using namespace std;

BOOL MainWindow_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    InitialiseDialog(hwnd);
    hwndViewer = SetClipboardViewer(hwnd);

    menu = GetMenu(hwnd);

    // Initialize the flags.
    fBlt = FALSE;
    fScroll = FALSE;
    fSize = FALSE;

    // Initialize the horizontal scrolling variables.
    xMinScroll = 0;
    xCurrentScroll = 0;
    xMaxScroll = 0;

    // Initialize the vertical scrolling variables.
    yMinScroll = 0;
    yCurrentScroll = 0;
    yMaxScroll = 0;
    return TRUE;
}

void MainWindow_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    StartxPos = x;
    StartyPos = y;
}

void MainWindow_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    EndxPos = x;
    EndyPos = y;

    mmov = true;
    InvalidateRect(hwnd,&rect,true);
}

BOOL MainWindow_OnQueryNewPalette(HWND hwnd)
{
    if (!hpal)
        return FALSE;
    hdc = GetDC(hwnd);
    SelectPalette (hdc, hpal, FALSE);
    RealizePalette (hdc);
    InvalidateRect(hwnd,NULL,TRUE);
    ReleaseDC(hwnd,hdc);
    return TRUE;
}

void MainWindow_OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
{
    if (!hpal || hwndPaletteChange == hwnd)
        return;
    hdc = GetDC(hwnd);
    SelectPalette (hdc, hpal, FALSE);
    RealizePalette (hdc);
    UpdateColors(hdc);
    ReleaseDC(hwnd,hdc);
}

void MainWindow_OnDrawClipboard(HWND hwnd)
{
    if (hwndViewer)
    {
        FORWARD_WM_DRAWCLIPBOARD(hwnd, SendMessage);
    }

    EnableMenuItem(menu, IDM_PASTE, MF_ENABLED);
    InvalidateRect(hwnd, NULL, true);
}

void MainWindow_OnChangeCBChain(HWND hwnd, HWND hwndRemove, HWND hwndNext)
{
    if(hwndRemove == hwndViewer)
    {
        hwndViewer = hwndNext;
    }
    else if(hwndViewer)
    {
        FORWARD_WM_CHANGECBCHAIN(hwnd, hwndRemove, hwndNext, SendMessage);
    }
}

void MainWindow_OnCommand_Open_BM(HWND hwnd)
{
    hdc = NULL;

    OpenFileDialog(hwnd, szFileName, (TCHAR*)"Open a Bitmap File.");

    if(szFileName!= NULL)
    {
        ZeroMemory(&hBitmap, sizeof(HBITMAP));
        hBitmap = (HBITMAP)LoadImage(NULL,szFileName,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE|LR_VGACOLOR);
        if(hBitmap)
        {
            EnableMenuItem(menu, IDM_SAVE_BM, MF_ENABLED);
            EnableMenuItem(menu, IDM_PRINT_BM, MF_ENABLED);
            EnableMenuItem(menu, IDM_ZOOMIN, MF_ENABLED);
            EnableMenuItem(menu, IDM_ZOOMOUT, MF_ENABLED);
            cxpage = GetDeviceCaps (hdc, HORZRES);
            cypage = GetDeviceCaps (hdc, VERTRES);
            GetObject(hBitmap,sizeof(BITMAP),&bitmap);
            bxWidth = bitmap.bmWidth;
            bxHeight = bitmap.bmHeight;
            bxWidth2 = bxWidth;
            bxHeight2 = bxHeight;
            rect.left = 0;
            rect.top =0;
            rect.right = (long)&cxpage;
            rect.bottom = (long)&cypage;
			zoom =0;
			fZoom =false;
            fBlt = TRUE;
            fLoad = TRUE;
            hdc = CreateCompatibleDC(NULL);
            if(IsClipboardFormatAvailable(CF_BITMAP))
            {
                EnableMenuItem(menu, IDM_PASTE, MF_ENABLED);
            }
            InvalidateRect(hwnd,&rect,true);


        }
    }
}

void MainWindow_OnCommand_Print_BM(HWND hwnd)
{
    DOCINFO di= { sizeof (DOCINFO), TEXT ("Printing Picture...")};
    HDC prn;

    prn = GetPrinterDC(hwnd);
    cxpage = GetDeviceCaps (prn, HORZRES);
    cypage = GetDeviceCaps (prn, VERTRES);
    hdcMem = CreateCompatibleDC(prn);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

    StartDoc (prn, &di);
    StartPage (prn) ;
    SetMapMode (prn, MM_ISOTROPIC);
    SetWindowExtEx(prn, cxpage,cypage, NULL);
    SetViewportExtEx(prn, cxpage, cypage,NULL);

    SetViewportOrgEx(prn, 0, 0, NULL);
	StretchBlt(prn, 0, 0, cxpage, cypage, hdcMem, 0, 0, (int)bxWidth, (int)bxHeight, SRCCOPY);
    EndPage (prn);
    EndDoc(prn);
    DeleteDC(prn);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
}

void MainWindow_OnCommand_Save_BM(HWND hwnd)
{
    BOOL result = SaveFileDialog(hwnd,szFileName,(TCHAR*)"Save a Bitmap.");
    if(result != false)
    {
        PBITMAPINFO pbi = CreateBitmapInfoStruct(hwnd, hBitmap);
        hdc= GetDC(hwnd);
        SaveBMPFile(hwnd, szFileName, pbi, hBitmap, hdc);
    }
}

void MainWindow_OnCommand_Exit(HWND hwnd)
{
    PostQuitMessage(0);
}

void MainWindow_OnCommand_Copy(HWND hwnd)
{
    int cutwidth = (int)abs(StartxPos - EndxPos);
    int cutheight = (int)abs(StartyPos - EndyPos);

    int oldxpos = (int)StartxPos;
    int oldendpos = (int)EndxPos;
    int oldypos = (int)StartyPos;
    int oldendypos = (int)EndyPos;

    if (StartxPos > EndxPos)
    {
        StartxPos = oldendpos;
        EndxPos = oldxpos;
        StartyPos = oldendypos;
        EndyPos = oldypos;
    }

    HBITMAP hbmp;
    ZeroMemory(&hbmp, sizeof(HBITMAP));

    hbmp = CopyScreenToBitmap(hwnd,(int)StartxPos,(int)StartyPos,cutwidth,cutheight);
    if(hbmp == NULL)
    {
        MessageBox(hwnd,"Copy not created properly","Error",MB_OK);
    }

    BitmapToClipboard(hbmp, hwnd);
    DeleteBitmap(hbmp);
}

void MainWindow_OnCommand_Cut(HWND hwnd)
{
    int cutwidth = (int)abs(StartxPos - EndxPos);
    int cutheight = (int)abs(StartyPos - EndyPos);

    int oldxpos = (int)StartxPos;
    int oldendpos = (int)EndxPos;
    int oldypos = (int)StartyPos;
    int oldendypos = (int)EndyPos;

    if (StartxPos > EndxPos)
    {
        StartxPos = oldendpos;
        EndxPos = oldxpos;
        StartyPos = oldendypos;
        EndyPos = oldypos;
    }

    HBITMAP hbmp;
    ZeroMemory(&hbmp, sizeof(HBITMAP));

    hbmp = CopyScreenToBitmap(hwnd,(int)StartxPos,(int)StartyPos,cutwidth,cutheight);
    if(hbmp == NULL)
    {
        MessageBox(hwnd,"Copy not created properly","Error",MB_OK);
    }

    OpenClipboard(hwnd);
    EmptyClipboard();
    CloseClipboard();

    BitmapToClipboard(hbmp, hwnd);
    DeleteBitmap(hbmp);

    cut = true;
    InvalidateRect(hwnd,&rect,true);
}

void MainWindow_OnCommand_Paste(HWND hwnd)
{
    paste = true;

    InvalidateRect(hwnd,NULL,true);
}

void MainWindow_OnCommand_ZoomOut(HWND hwnd)
{
    zoom = zoom - 25;
    if (zoom < -75)
    {
        zoom = -75;
    }
    bxWidth2 = bxWidth + (bxWidth*(zoom/100));
    bxHeight2 = bxHeight + (bxHeight*(zoom/100));

    fZoom = true;

    GetWindowRect(hwnd, &WinRect);
    cxsize = WinRect.right;
    cysize = WinRect.bottom;
    int t1 =(int)(bxWidth2-cxsize);
    if (t1 < 0)
    {
        ShowScrollBar(hwnd,SB_HORZ,false);

    }
    xMaxScroll = max(t1,0);
    xCurrentScroll = min(xCurrentScroll, xMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = xMinScroll;
    si.nMax   = (int)bxWidth2;
    si.nPage  = cxsize;
    si.nPos   = xCurrentScroll;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

    // The vertical scrolling range is defined by
    // (bitmap_height) - (client_height). The current vertical
    // scroll value remains within the vertical scrolling range.
    int t2 = (int)bxHeight2 - cysize;
    yMaxScroll = max(t2, 0);
    yCurrentScroll = min(yCurrentScroll, yMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = yMinScroll;
    si.nMax   = (int)bxHeight2;
    si.nPage  = cysize;
    si.nPos   = yCurrentScroll;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

    InvalidateRect(hwnd,&WinRect,true);
}

void MainWindow_OnCommand_ZoomIn(HWND hwnd)
{
    zoom = zoom + 25;
    if (zoom >  75)
    {
        zoom = 75;
    }
    bxWidth2 = bxWidth + (bxWidth*(zoom/100));
    bxHeight2 = bxHeight + (bxHeight*(zoom/100));

    fZoom = true;

    GetWindowRect(hwnd, &WinRect);
    cxsize = WinRect.right;
    cysize = WinRect.bottom;
    int t1 = (int)(bxWidth2-cxsize);
    if (t1 > 0)
    {
        ShowScrollBar(hwnd,SB_HORZ,true);

    }
    xMaxScroll = max(t1,0);
    xCurrentScroll = min(xCurrentScroll, xMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = xMinScroll;
    si.nMax   = (int)bxWidth2;
    si.nPage  = cxsize;
    si.nPos   = xCurrentScroll;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

    // The vertical scrolling range is defined by
    // (bitmap_height) - (client_height). The current vertical
    // scroll value remains within the vertical scrolling range.
    int t2 = (int)bxHeight2 - cysize;
    yMaxScroll = max(t2, 0);
    yCurrentScroll = min(yCurrentScroll, yMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = yMinScroll;
    si.nMax   = (int)bxHeight2;
    si.nPage  = cysize;
    si.nPos   = yCurrentScroll;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    InvalidateRect(hwnd,&WinRect,true);
}

void MainWindow_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDM_OPEN_BM:
        MainWindow_OnCommand_Open_BM(hwnd);
        break;

    case IDM_PRINT_BM:
        MainWindow_OnCommand_Print_BM(hwnd);
        break;

    case IDM_SAVE_BM:
        MainWindow_OnCommand_Save_BM(hwnd);
        break;

    case IDM_EXIT:
        MainWindow_OnCommand_Exit(hwnd);
        break;

    case IDM_COPY:
        MainWindow_OnCommand_Copy(hwnd);
        break;

    case IDM_CUT:
        MainWindow_OnCommand_Cut(hwnd);
        break;

    case IDM_PASTE:
        MainWindow_OnCommand_Paste(hwnd);
        break;

    case IDM_ZOOMOUT:
        MainWindow_OnCommand_ZoomOut(hwnd);
        break;

    case IDM_ZOOMIN:
        MainWindow_OnCommand_ZoomIn(hwnd);
        break;
    }
}

void MainWindow_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;

    hdc = BeginPaint(hwnd, &ps);

    hdcMem = CreateCompatibleDC(hdc);
    HDC MemoryDC= CreateCompatibleDC(hdc);
    if (fLoad)
    {
        PaintLoadBitmap(hwnd, si, bitmap, cxsize, cysize, xMaxScroll, xCurrentScroll, xMinScroll, yMaxScroll, yCurrentScroll, yMinScroll);

        fLoad= FALSE;

    }
    if (fBlt)
    {
        SelectObject(hdcMem, hBitmap);
        SetStretchBltMode(hdc,HALFTONE);

        StretchBlt(hdc, 0, 0,(int)bxWidth2,(int)bxHeight2, hdcMem,xCurrentScroll,yCurrentScroll,(int)bxWidth,(int)bxHeight, SRCCOPY);
    }

    // If the window has been resized and the user has
    // captured the screen, use the following call to
    // BitBlt to paint the window's client area.

    if (fSize)
    {
        SelectObject(hdcMem, hBitmap);
        SetStretchBltMode(hdc,HALFTONE);

        StretchBlt(hdc, 0, 0, (int)bxWidth2 ,(int)bxHeight2, hdcMem,xCurrentScroll,yCurrentScroll,(int)bxWidth,(int)bxHeight, SRCCOPY);
        fSize = FALSE;
    }

    // If scrolling has occurred, use the following call to
    // BitBlt to paint the invalid rectangle.
    //
    // The coordinates of this rectangle are specified in the
    // RECT structure to which prect points.
    //
    // Note that it is necessary to increment the seventh
    // argument (prect->left) by xCurrentScroll and the
    // eighth argument (prect->top) by yCurrentScroll in
    // order to map the correct pixels from the source bitmap.
    if (fScroll)
    {
        SetStretchBltMode(hdc,HALFTONE);
        StretchBlt(hdc,0,0,(int)bxWidth2,(int)bxHeight2,hdcMem,xCurrentScroll,yCurrentScroll,(int)bxWidth,(int)bxHeight,SRCCOPY);

        fScroll = false;
    }

    if(fZoom == true)
    {
        SelectObject(hdcMem,hBitmap);
        SetStretchBltMode(hdc,HALFTONE);
        StretchBlt(hdc,0,0,(int)bxWidth2,(int)bxHeight2,hdcMem,xCurrentScroll,yCurrentScroll,(int)bxWidth,(int)bxHeight,SRCCOPY);

        fZoom = false;
    }


    if(mmov)
    {
        DrawBoundingBox(hdc, (int)StartxPos, (int)EndxPos, (int)StartyPos, (int)EndyPos);
        mmov = false;
        EnableMenuItem(menu, IDM_COPY, MF_ENABLED);
        EnableMenuItem(menu, IDM_CUT, MF_ENABLED);
    }

    if(cut)
    {
        Cut(hdc, hdcMem, hBitmap, (int)bxWidth2, (int)bxHeight2, (int)bxWidth, (int)bxHeight, StartxPos, EndxPos, StartyPos, EndyPos, xCurrentScroll, yCurrentScroll);

        cut = false;
    }

    if(paste)
    {
        int oldxpos = (int)StartxPos;
        int oldendpos = (int)EndxPos;
        int oldypos = (int)StartyPos;
        int oldendypos = (int)EndyPos;

        if (StartxPos > EndxPos)
        {
            StartxPos = oldendpos;
            EndxPos = oldxpos;
            StartyPos = oldendypos;
            EndyPos = oldypos;
        }

        StartxPos = ((StartxPos)*(bxWidth/bxWidth2));
        StartyPos = ((StartyPos)*(bxHeight/bxHeight2));
        EndxPos = (EndxPos+xCurrentScroll);
        EndyPos = (EndyPos+yCurrentScroll);

        Paste(hwnd, hBitmap, bxWidth2, bxHeight2 ,bxWidth, bxHeight ,hdc, hdcMem, MemoryDC, StartxPos, EndxPos, xCurrentScroll, StartyPos, EndyPos, yCurrentScroll );

        //Reset Scrollbars to zero
        ZeroScrollbars(hwnd, si, bitmap, cxsize, cysize, xCurrentScroll, yCurrentScroll, xMaxScroll, yMaxScroll, xMinScroll, yMinScroll);

        paste = false;
    }

    EndPaint(hwnd, &ps);
    DeleteDC(hdcMem);
    DeleteDC(MemoryDC);
}

void MainWindow_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    cxsize = cx;
    cysize = cy;
    rect.right = cxsize;
    rect.bottom = cysize;

    fSize = TRUE;

    // The horizontal scrolling range is defined by
    // (bitmap_width) - (client_width). The current horizontal
    // scroll value remains within the horizontal scrolling range.
    int t1 = (int)(bxWidth2-cxsize);
    xMaxScroll = max(t1,0);
    xCurrentScroll = min(xCurrentScroll, xMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = xMinScroll;
    si.nMax   = (int)bxWidth2;
    si.nPage  = (int)cxsize;
    si.nPos   = xCurrentScroll;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

    // The vertical scrolling range is defined by
    // (bitmap_height) - (client_height). The current vertical
    // scroll value remains within the vertical scrolling range.
    int t2 = (int)bxHeight2 - cysize;
    yMaxScroll = max(t2, 0);
    yCurrentScroll = min(yCurrentScroll, yMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = yMinScroll;
    si.nMax   = (int)bxHeight2;
    si.nPage  = cysize;
    si.nPos   = yCurrentScroll;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

    InvalidateRect(hwnd, &WinRect, true);
}

void MainWindow_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
    int xDelta;     // xDelta = new_pos - current_pos
    int xNewPos;    // new position
    int yDelta = 0;

    switch (code)
    {
        // User clicked the scroll bar shaft left of the scroll box.
    case SB_PAGEUP:
        xNewPos = xCurrentScroll - 50;
        break;

        // User clicked the scroll bar shaft right of the scroll box.
    case SB_PAGEDOWN:
        xNewPos = xCurrentScroll + 50;
        break;

        // User clicked the left arrow.
    case SB_LINEUP:
        xNewPos = xCurrentScroll - 5;
        break;

        // User clicked the right arrow.
    case SB_LINEDOWN:
        xNewPos = xCurrentScroll + 5;
        break;

        // User dragged the scroll box.
    case SB_THUMBPOSITION:
        xNewPos = pos;
        break;

    default:
        xNewPos = xCurrentScroll;
    }

    // New position must be between 0 and the screen width.
    xMaxScroll = (int)bxWidth2;
    xNewPos = max(0, xNewPos);
    xNewPos = min(xMaxScroll, xNewPos);

    // If the current position does not change, do not scroll.
    if (xNewPos == xCurrentScroll)
        return;

    // Set the scroll flag to TRUE.
    fScroll = TRUE;

    // Determine the amount scrolled (in pixels).
    xDelta = xNewPos ;

    // Reset the current scroll position.
    xCurrentScroll = xNewPos;

    // Scroll the window. (The system repaints most of the
    // client area when ScrollWindowEx is called; however, it is
    // necessary to call UpdateWindow in order to repaint the
    // rectangle of pixels that were invalidated.)
    ScrollWindowEx(hwnd, -xDelta, -yDelta, (CONST RECT *) NULL,
                    (CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL,
                    SW_INVALIDATE);
    InvalidateRect(hwnd, &WinRect, true);

    // Reset the scroll bar.
    si.cbSize = sizeof(si);
    si.fMask  = SIF_POS;
    si.nPos   = xCurrentScroll;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
}

void MainWindow_OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
    int xDelta = 0;
    int yDelta;     // yDelta = new_pos - current_pos
    int yNewPos;    // new position

    switch (code)
    {
        // User clicked the scroll bar shaft above the scroll box.
    case SB_PAGEUP:
        yNewPos = yCurrentScroll - 50;
        break;

        // User clicked the scroll bar shaft below the scroll box.
    case SB_PAGEDOWN:
        yNewPos = yCurrentScroll + 50;
        break;

        // User clicked the top arrow.
    case SB_LINEUP:
        yNewPos = yCurrentScroll - 5;
        break;

        // User clicked the bottom arrow.
    case SB_LINEDOWN:
        yNewPos = yCurrentScroll + 5;
        break;

        // User dragged the scroll box.
    case SB_THUMBPOSITION:
        yNewPos = pos;
        break;

    default:
        yNewPos = yCurrentScroll;
    }

    // New position must be between 0 and the screen height.
    yMaxScroll = (int)bxHeight2;
    yNewPos = max(0, yNewPos);
    yNewPos = min(yMaxScroll, yNewPos);

    // If the current position does not change, do not scroll.
    if (yNewPos == yCurrentScroll)
        return;

    // Set the scroll flag to TRUE.
    fScroll = TRUE;

    // Determine the amount scrolled (in pixels).
    yDelta = yNewPos - yCurrentScroll;

    // Reset the current scroll position.
    yCurrentScroll = yNewPos;

    // Scroll the window. (The system repaints most of the
    // client area when ScrollWindowEx is called; however, it is
    // necessary to call UpdateWindow in order to repaint the
    // rectangle of pixels that were invalidated.)
    ScrollWindowEx(hwnd, -xDelta, -yDelta, (CONST RECT *) NULL,
                    (CONST RECT *) NULL, (HRGN) NULL, (PRECT) NULL,
                    SW_INVALIDATE);
    InvalidateRect(hwnd,&WinRect, true);

    // Reset the scroll bar.
    si.cbSize = sizeof(si);
    si.fMask  = SIF_POS;
    si.nPos   = yCurrentScroll;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void MainWindow_OnDestroy(HWND hwnd)
{
    ChangeClipboardChain(hwnd, hwndViewer);
    PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
}

/*
 * class MainWindow
 *
 * Implementation goes below.
 */

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK MainWindow::WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MainWindow * pThis = reinterpret_cast<MainWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (!pThis && message == WM_CREATE)
    {
        LPCREATESTRUCT pcreatestruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<MainWindow *>(pcreatestruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
    }

    switch (message)
    {
    HANDLE_MSG(hwnd, WM_CREATE,             MainWindow_OnCreate);
    HANDLE_MSG(hwnd, WM_LBUTTONDOWN,        MainWindow_OnLButtonDown);
    HANDLE_MSG(hwnd, WM_LBUTTONUP,          MainWindow_OnLButtonUp);
    HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE,    MainWindow_OnQueryNewPalette);
    HANDLE_MSG(hwnd, WM_PALETTECHANGED,     MainWindow_OnPaletteChanged);
    HANDLE_MSG(hwnd, WM_DRAWCLIPBOARD,      MainWindow_OnDrawClipboard);
    HANDLE_MSG(hwnd, WM_CHANGECBCHAIN,      MainWindow_OnChangeCBChain);
    HANDLE_MSG(hwnd, WM_COMMAND,            MainWindow_OnCommand);
    HANDLE_MSG(hwnd, WM_PAINT,              MainWindow_OnPaint);
    HANDLE_MSG(hwnd, WM_SIZE,               MainWindow_OnSize);
    HANDLE_MSG(hwnd, WM_HSCROLL,            MainWindow_OnHScroll);
    HANDLE_MSG(hwnd, WM_VSCROLL,            MainWindow_OnVScroll);
    HANDLE_MSG(hwnd, WM_DESTROY,            MainWindow_OnDestroy);
    }

    /* for messages that we don't deal with */
    return DefWindowProc (hwnd, message, wParam, lParam);
}
