#include <WindowsX.h>
#include "WinProc.h"
#include "resource.h"
#include <cmath>
#include <algorithm>
#include <fstream>

using namespace std;

/*   variables  */
char MainWindow::szClassName[] = "Snoopy's Image Editing Demo";

/*
 * class MainWindow
 *
 * Implementation goes below.
 */

MainWindow::MainWindow()
    : zoom(0)
    , fZoom(false)
    , cxsize(0)
    , cxpage(0)
    , cysize(0)
    , cypage(0)
{
}

MainWindow::~MainWindow()
{
}

BOOL MainWindow::OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
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

void MainWindow::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    StartxPos = x;
    StartyPos = y;
}

void MainWindow::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    EndxPos = x;
    EndyPos = y;

    mmov = true;
    InvalidateRect(hwnd,&rect,true);
}

BOOL MainWindow::OnQueryNewPalette(HWND hwnd)
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

void MainWindow::OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
{
    if (!hpal || hwndPaletteChange == hwnd)
        return;
    hdc = GetDC(hwnd);
    SelectPalette (hdc, hpal, FALSE);
    RealizePalette (hdc);
    UpdateColors(hdc);
    ReleaseDC(hwnd,hdc);
}

void MainWindow::OnDrawClipboard(HWND hwnd)
{
    if (hwndViewer)
    {
        FORWARD_WM_DRAWCLIPBOARD(hwnd, SendMessage);
    }

    EnableMenuItem(menu, IDM_PASTE, MF_ENABLED);
    InvalidateRect(hwnd, NULL, true);
}

void MainWindow::OnChangeCBChain(HWND hwnd, HWND hwndRemove, HWND hwndNext)
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

void MainWindow::OnCommand_Open_BM(HWND hwnd)
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

void MainWindow::OnCommand_Print_BM(HWND hwnd)
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

void MainWindow::OnCommand_Save_BM(HWND hwnd)
{
    BOOL result = SaveFileDialog(hwnd,szFileName,(TCHAR*)"Save a Bitmap.");
    if(result != false)
    {
        PBITMAPINFO pbi = CreateBitmapInfoStruct(hwnd, hBitmap);
        hdc= GetDC(hwnd);
        SaveBMPFile(hwnd, szFileName, pbi, hBitmap, hdc);
    }
}

void MainWindow::OnCommand_Exit(HWND hwnd)
{
    PostQuitMessage(0);
}

void MainWindow::OnCommand_Copy(HWND hwnd)
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

void MainWindow::OnCommand_Cut(HWND hwnd)
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

void MainWindow::OnCommand_Paste(HWND hwnd)
{
    paste = true;

    InvalidateRect(hwnd,NULL,true);
}

void MainWindow::OnCommand_ZoomOut(HWND hwnd)
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

void MainWindow::OnCommand_ZoomIn(HWND hwnd)
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

void MainWindow::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDM_OPEN_BM:
        OnCommand_Open_BM(hwnd);
        break;

    case IDM_PRINT_BM:
        OnCommand_Print_BM(hwnd);
        break;

    case IDM_SAVE_BM:
        OnCommand_Save_BM(hwnd);
        break;

    case IDM_EXIT:
        OnCommand_Exit(hwnd);
        break;

    case IDM_COPY:
        OnCommand_Copy(hwnd);
        break;

    case IDM_CUT:
        OnCommand_Cut(hwnd);
        break;

    case IDM_PASTE:
        OnCommand_Paste(hwnd);
        break;

    case IDM_ZOOMOUT:
        OnCommand_ZoomOut(hwnd);
        break;

    case IDM_ZOOMIN:
        OnCommand_ZoomIn(hwnd);
        break;
    }
}

void MainWindow::OnPaint(HWND hwnd)
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

void MainWindow::OnSize(HWND hwnd, UINT state, int cx, int cy)
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

void MainWindow::OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
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

void MainWindow::OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
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

void MainWindow::OnDestroy(HWND hwnd)
{
    ChangeClipboardChain(hwnd, hwndViewer);
    PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
}

HDC MainWindow::GetPrinterDC (HWND Hwnd)
{

// Initialize a PRINTDLG structure's size and set the PD_RETURNDC flag set the Owner flag to hwnd.
// The PD_RETURNDC flag tells the dialog to return a printer device context.
    HDC hdc;
    PRINTDLG pd = {0};
    pd.lStructSize = sizeof( pd );
    pd.hwndOwner = Hwnd;
    pd.Flags = PD_RETURNDC;

// Retrieves the printer DC
    PrintDlg(&pd);
    hdc =pd.hDC;
    return hdc ;


}

BOOL MainWindow::OpenFileDialog(HWND hwnd, LPTSTR pFileName ,LPTSTR pTitleName)

{
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hInstance = GetModuleHandle(NULL);
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;

    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = pFileName;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrTitle = pTitleName;
    ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
    ofn.lpstrFilter = TEXT("Bitmap Files (*.bmp)\0*.bmp\0\0");



    return GetOpenFileName(&ofn);
}

BOOL MainWindow::SaveFileDialog(HWND hwnd, LPTSTR pFileName ,LPTSTR pTitleName)

{
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hInstance = GetModuleHandle(NULL);
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;

    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = pFileName;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrTitle = pTitleName;
    ofn.Flags = OFN_EXPLORER|OFN_OVERWRITEPROMPT;
    ofn.lpstrFilter = TEXT("Bitmap Files (*.bmp)\0*.bmp\0\0");



    return GetSaveFileName(&ofn);
}

void MainWindow::InitialiseDialog(HWND hwnd)
{
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = NULL;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = 500;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = NULL;
    ofn.Flags = 0;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lCustData = 0L;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;
}


void MainWindow::SaveBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{
    std::ofstream hf;                  // file handle
    BITMAPFILEHEADER hdr;       // bitmap file-header
    PBITMAPINFOHEADER pbih;     // bitmap info-header
    LPBYTE lpBits;              // memory pointer
    DWORD cb;                   // incremental count of bytes
    BYTE *hp;                   // byte pointer


    pbih = (PBITMAPINFOHEADER) pbi;
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits)
    {
        MessageBox(hwnd,"GlobalAlloc","Error", MB_OK );
    }
// Retrieve the color table (RGBQUAD array) and the bits
// (array of palette indices) from the DIB.
    if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi,DIB_RGB_COLORS))
    {
        MessageBox(hwnd,"GetDIBits","Error",MB_OK );
    }
// Create the .BMP file.
    hf.open(pszFile,std::ios::binary);
    if (hf.fail() == true)
    {
        MessageBox( hwnd,"CreateFile","Error", MB_OK);
    }

    hdr.bfType = 0x4d42;  // File type designator "BM" 0x42 = "B" 0x4d = "M"
// Compute the size of the entire file.
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;
// Compute the offset to the array of color indices.
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD);
// Copy the BITMAPFILEHEADER into the .BMP file.
    hf.write((char*) &hdr, sizeof(BITMAPFILEHEADER));
    if (hf.fail() == true )
    {
        MessageBox(hwnd,"WriteFileHeader","Error",MB_OK );
    }
// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
    hf.write((char*) pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD));
    if (hf.fail() == true )
    {
        MessageBox(hwnd,"WriteInfoHeader","Error",MB_OK );
    }
// Copy the array of color indices into the .BMP file.
    cb = pbih->biSizeImage;
    hp = lpBits;
    hf.write((char*) hp, (int) cb);
    if (hf.fail() == true )
    {
        MessageBox(hwnd,"WriteData","Error",MB_OK );
    }
// Close the .BMP file.
    hf.close();
    if (hf.fail() == true)
    {
        MessageBox(hwnd,"CloseHandle","Error",MB_OK );
    }

// Free memory.
    GlobalFree((HGLOBAL)lpBits);
}
//End of BMP Save


PBITMAPINFO MainWindow::CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD cClrBits;
// Retrieve the bitmap color format, width, and height.
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
    {
        MessageBox(hwnd,"GetObject","Error",MB_OK );
    }
// Convert the color format to a count of bits.
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

// Allocate memory for the BITMAPINFO structure. (This structure
// contains a BITMAPINFOHEADER structure and an array of RGBQUAD
// data structures.)

    if (cClrBits != 24)
    {
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR,sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits));
    }
// There is no RGBQUAD array for the 24-bit-per-pixel format.
    else
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

// Initialize the fields in the BITMAPINFO structure.
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
    {
        pbmi->bmiHeader.biClrUsed = (1<<cClrBits);
    }
// If the bitmap is not compressed, set the BI_RGB flag.
    pbmi->bmiHeader.biCompression = BI_RGB;

// Compute the number of bytes in the array of color
// indices and store the result in biSizeImage.
// For Windows NT, the width must be DWORD aligned unless
// the bitmap is RLE compressed. This example shows this.
// For Windows 95/98/Me, the width must be WORD aligned unless the
// bitmap is RLE compressed.
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8 * pbmi->bmiHeader.biHeight;
// Set biClrImportant to 0, indicating that all of the
// device colors are important.
    pbmi->bmiHeader.biClrImportant = 0;

    return pbmi; //return BITMAPINFO
}

bool MainWindow::BitmapToClipboard(HBITMAP hBM, HWND hWnd)
{
    if (!OpenClipboard(hWnd))
        return false;
    EmptyClipboard();

    BITMAP bm;
    GetObject(hBM, sizeof(bm), &bm);

    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = bm.bmBitsPixel;
    bi.biCompression = BI_RGB;
    if (bi.biBitCount <= 1)	// make sure bits per pixel is valid
        bi.biBitCount = 1;
    else if (bi.biBitCount <= 4)
        bi.biBitCount = 4;
    else if (bi.biBitCount <= 8)
        bi.biBitCount = 8;
    else // if greater than 8-bit, force to 24-bit
        bi.biBitCount = 24;

    // Get size of color table.
    SIZE_T dwColTable = (bi.biBitCount <= 8) ? (1 << bi.biBitCount) * sizeof(RGBQUAD) : 0;

    // Create a device context with palette
    HDC hDC = GetDC(NULL);
    HPALETTE hPal = static_cast<HPALETTE>(GetStockObject(DEFAULT_PALETTE));
    HPALETTE hOldPal = SelectPalette(hDC, hPal, FALSE);
    RealizePalette(hDC);

    // Use GetDIBits to calculate the image size.
    GetDIBits(hDC, hBM, 0, static_cast<UINT>(bi.biHeight), NULL,
              reinterpret_cast<LPBITMAPINFO>(&bi), DIB_RGB_COLORS);
    // If the driver did not fill in the biSizeImage field, then compute it.
    // Each scan line of the image is aligned on a DWORD (32bit) boundary.
    if (0 == bi.biSizeImage)
        bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;

    // Allocate memory
    HGLOBAL hDIB = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dwColTable + bi.biSizeImage);
    if (hDIB)
    {
        union tagHdr_u
        {
            LPVOID             p;
            LPBYTE             pByte;
            LPBITMAPINFOHEADER pHdr;
            LPBITMAPINFO       pInfo;
        } Hdr;

        Hdr.p = GlobalLock(hDIB);
        // Copy the header
        CopyMemory(Hdr.p, &bi, sizeof(BITMAPINFOHEADER));
        // Convert/copy the image bits and create the color table
        int nConv = GetDIBits(hDC, hBM, 0, static_cast<UINT>(bi.biHeight),
                              Hdr.pByte + sizeof(BITMAPINFOHEADER) + dwColTable,
                              Hdr.pInfo, DIB_RGB_COLORS);
        GlobalUnlock(hDIB);
        if (!nConv)
        {
            GlobalFree(hDIB);
            hDIB = NULL;
        }
    }
    if (hDIB)
        SetClipboardData(CF_DIB, hDIB);
    CloseClipboard();
    SelectPalette(hDC, hOldPal, FALSE);
    ReleaseDC(NULL, hDC);
    return NULL != hDIB;
}



HBITMAP MainWindow::CopyScreenToBitmap(HWND Hwnd,int x1, int y1, int nWidth, int nHeight)
{
    HDC hScrDC,hMemDC; // screen DC and memory DC
    HBITMAP hxBitmap, hOldBitmap; // handles to deice-dependent bitmaps





// create a DC for the screen and create
// a memory DC compatible to screen DC

     hScrDC = GetDC(Hwnd);



    if((hMemDC = CreateCompatibleDC(hScrDC))==NULL)
    {

        MessageBox(Hwnd,"Can not create compatible DC","Error",MB_OK);
        return NULL;
    }


// create a bitmap compatible with the screen DC
    if((hxBitmap = CreateCompatibleBitmap(hScrDC, nWidth-1, nHeight-1))==NULL)
    {
        MessageBox(Hwnd,"Can not create compatible Bitmap","Error",MB_OK);

        return NULL;
    }

// select new bitmap into memory DC
    hOldBitmap = (HBITMAP)SelectObject(hMemDC, hxBitmap);

    if(hOldBitmap == NULL)
    {
        MessageBox(Hwnd,"Can not select old Bitmap","Error",MB_OK);
        return NULL;
    }

// bitblt screen DC to memory DC
    if(!BitBlt(hMemDC, 0, 0, nWidth-2, nHeight-2, hScrDC, x1+1, y1+1, SRCCOPY))
    {
        MessageBox(Hwnd,"Can not read screen memory","Error",MB_OK);
        return NULL;
    }

// select old bitmap back into memory DC and get handle to bitmap of the screen

    hxBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
    if(hxBitmap == NULL)
    {
        MessageBox(Hwnd,"Can not select object","Error",MB_OK);
        return NULL;
    }
// clean up
    DeleteDC(hScrDC);
    DeleteDC(hMemDC);

// return handle to the bitmap

    return hxBitmap;
}


void MainWindow::PaintLoadBitmap(HWND hwnd,SCROLLINFO si, BITMAP bitmap, int pcxsize, int pcysize, int xMaxScroll,int xCurrentScroll, int xMinScroll,int yMaxScroll, int yCurrentScroll ,int yMinScroll)
{


    // The horizontal scrolling range is defined by
    // (bitmap_width) - (client_width). The current horizontal
    // scroll value remains within the horizontal scrolling range.
    int t1 =(bitmap.bmWidth-pcxsize);
    xMaxScroll = max(t1,0);
    xCurrentScroll = min(xCurrentScroll, xMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = xMinScroll;
    si.nMax   = bitmap.bmWidth;
    si.nPage  = pcxsize;
    si.nPos   = xCurrentScroll;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

    // The vertical scrolling range is defined by
    // (bitmap_height) - (client_height). The current vertical
    // scroll value remains within the vertical scrolling range.
    int t2 = bitmap.bmHeight - pcysize;
    yMaxScroll = max(t2, 0);
    yCurrentScroll = min(yCurrentScroll, yMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = yMinScroll;
    si.nMax   = bitmap.bmHeight;
    si.nPage  = pcysize;
    si.nPos   = yCurrentScroll;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void MainWindow::DrawBoundingBox(HDC hdc, int StartxPos, int EndxPos, int StartyPos, int EndyPos)
{
    HPEN greenPen=CreatePen(PS_SOLID, 1, RGB(0,255,0));
    SelectObject(hdc, greenPen);

    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));


    MoveToEx(hdc, StartxPos, StartyPos, NULL);
    Rectangle(hdc, StartxPos, StartyPos , EndxPos, EndyPos);


}

void MainWindow::Cut(HDC hdc, HDC hdcMem, HBITMAP hBitmap, double Width, double Height, double oWidth, double oHeight, double StartxPos, double EndxPos, double StartyPos, double EndyPos, int xCurrentScroll, int yCurrentScroll)
{
    RECT crect;
    StartxPos = StartxPos*(oWidth/Width);
    StartyPos = StartyPos*(oHeight/Height);
    EndxPos = EndxPos*(oWidth/Width);
    EndyPos = EndyPos*(oHeight/Height);

    crect.left = (long)StartxPos+xCurrentScroll;
    crect.top =  (long)StartyPos+yCurrentScroll;
    crect.right = (long)EndxPos+xCurrentScroll;
    crect.bottom = (long)EndyPos+yCurrentScroll;

    SelectObject(hdcMem, hBitmap);

    FillRect(hdcMem, &crect, GetStockBrush(LTGRAY_BRUSH));
    SetStretchBltMode(hdc,HALFTONE);
    StretchBlt(hdc, 0, 0, (int)Width,(int)Height, hdcMem,xCurrentScroll,yCurrentScroll,(int)oWidth,(int)oHeight, SRCCOPY);

}

void MainWindow::Paste(HWND hwnd, HBITMAP hBitmap,double Width,double Height ,double oWidth, double oHeight ,HDC hdc, HDC hdcMem, HDC MemoryDC, double StartxPos, double EndxPos, int xCurrentScroll, double StartyPos, double EndyPos, int yCurrentScroll )
{
    HBITMAP clip;
    BITMAP clipmap;



    ZeroMemory(&clip, sizeof(HBITMAP));
    ZeroMemory(&clipmap, sizeof(BITMAP));
    OpenClipboard(hwnd);

    clip = (HBITMAP)GetClipboardData(CF_BITMAP);
    GetObject(clip,sizeof(BITMAP),&clipmap);
    SelectObject(MemoryDC, clip);

    SelectObject(hdcMem,hBitmap);

    BitBlt(hdcMem, (int)StartxPos+xCurrentScroll, (int)StartyPos+yCurrentScroll ,(int)oWidth,
              (int)oHeight, MemoryDC ,0,0,SRCCOPY);
    SetStretchBltMode(hdc,HALFTONE);
    StretchBlt(hdc, 0, 0, (int)Width,(int)Height, hdcMem,xCurrentScroll,yCurrentScroll, (int)oWidth, (int)oHeight, SRCCOPY);


    CloseClipboard();
    DeleteBitmap(clip);
}

void MainWindow::ZeroScrollbars(HWND hwnd, SCROLLINFO si, BITMAP bitmap, int cxsize, int cysize,int xCurrentScroll, int yCurrentScroll, int xMaxScroll, int yMaxScroll, int xMinScroll, int yMinScroll)
{
    int t1 =(bitmap.bmWidth-cxsize);
    xMaxScroll = max(t1,0);
    xCurrentScroll = min(xCurrentScroll, xMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = xMinScroll;
    si.nMax   = bitmap.bmWidth;
    si.nPage  = cxsize;
    si.nPos   = 0;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

    // The scrolling range is defined by
    // (bitmap_height) - (client_height). The current
    // scroll value remains within the scrolling range.
    int t2 = bitmap.bmHeight - cysize;
    yMaxScroll = max(t2, 0);
    yCurrentScroll = min(yCurrentScroll, yMaxScroll);
    si.cbSize = sizeof(si);
    si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin   = yMinScroll;
    si.nMax   = bitmap.bmHeight;
    si.nPage  = cysize;
    si.nPos   = 0;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    xCurrentScroll = 0;
    yCurrentScroll =0;
}


LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    HANDLE_MSG(hwnd, WM_CREATE,             OnCreate);
    HANDLE_MSG(hwnd, WM_LBUTTONDOWN,        OnLButtonDown);
    HANDLE_MSG(hwnd, WM_LBUTTONUP,          OnLButtonUp);
    HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE,    OnQueryNewPalette);
    HANDLE_MSG(hwnd, WM_PALETTECHANGED,     OnPaletteChanged);
    HANDLE_MSG(hwnd, WM_DRAWCLIPBOARD,      OnDrawClipboard);
    HANDLE_MSG(hwnd, WM_CHANGECBCHAIN,      OnChangeCBChain);
    HANDLE_MSG(hwnd, WM_COMMAND,            OnCommand);
    HANDLE_MSG(hwnd, WM_PAINT,              OnPaint);
    HANDLE_MSG(hwnd, WM_SIZE,               OnSize);
    HANDLE_MSG(hwnd, WM_HSCROLL,            OnHScroll);
    HANDLE_MSG(hwnd, WM_VSCROLL,            OnVScroll);
    HANDLE_MSG(hwnd, WM_DESTROY,            OnDestroy);
    }

    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
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

    if (pThis && message == WM_DESTROY)
    {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
    }

    if (pThis)
    {
        return pThis->WndProc(hwnd, message, wParam, lParam);
    }

    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL MainWindow::RegisterClass(HINSTANCE hinstance)
{
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hinstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
    wincl.hIconSm = LoadIcon (GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = MAKEINTRESOURCE(hMenu);                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    return RegisterClassEx (&wincl);
}

HWND MainWindow::Create(HINSTANCE hinstance)
{
    hwnd = CreateWindowEx (
               0,                   /* Extended possibilites for variation */
               szClassName,         /* Classname */
               szClassName,         /* Title Text */
               WS_OVERLAPPEDWINDOW, /* default window */
               CW_USEDEFAULT,       /* Windows decides the position */
               CW_USEDEFAULT,       /* where the window ends up on the screen */
               544,                 /* The programs width */
               375,                 /* and height in pixels */
               HWND_DESKTOP,        /* The window is a child-window to desktop */
               NULL,                /* menu */
               hinstance,           /* Program Instance handler */
               this                 /* MainWindow instance */
           );
    return hwnd;
}
