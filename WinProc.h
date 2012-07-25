#ifndef WINPROC_H_INCLUDED
#define WINPROC_H_INCLUDED

#include "Functions.h"
#include "resource.h"
#include <Winspool.h>
#include <CommDlg.h>
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

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
     PAINTSTRUCT ps;


    // These variables are required by BitBlt.

    

    static BOOL fBlt;
    static BOOL fLoad;           // TRUE if BitBlt occurred
    static BOOL fScroll;         // TRUE if scrolling occurred
    static BOOL fSize;           // TRUE if fBlt & WM_SIZE

    static BOOL mmov;
    static BOOL cut;
    static BOOL paste;
    // These variables are required for horizontal scrolling.
    static int xMinScroll;       // minimum horizontal scroll value
    static int xCurrentScroll;   // current horizontal scroll value
    static int xMaxScroll;       // maximum horizontal scroll value

    // These variables are required for vertical scrolling.
    static int yMinScroll;       // minimum vertical scroll value
    static int yCurrentScroll;   // current vertical scroll value
    static int yMaxScroll;       // maximum vertical scroll value
    static HWND hwndViewer;

    switch (message)                  /* handle the messages */
    {
    case WM_CREATE:
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
        return 0;

    case WM_LBUTTONDOWN:
    {
        StartxPos = LOWORD(lParam);
        StartyPos = HIWORD(lParam);



        return 0;
    }

    case WM_LBUTTONUP:
    {
        EndxPos = LOWORD(lParam);
        EndyPos = HIWORD(lParam);

        mmov = true;
        InvalidateRect(hwnd,&rect,true);
        return 0;
    }



    case WM_QUERYNEWPALETTE:
        if (!hpal)
            return FALSE;
        hdc = GetDC(hwnd);
        SelectPalette (hdc, hpal, FALSE);
        RealizePalette (hdc);
        InvalidateRect(hwnd,NULL,TRUE);
        ReleaseDC(hwnd,hdc);
        return TRUE;

    case WM_PALETTECHANGED:
        if (!hpal || (HWND)wParam == hwnd)
            break;
        hdc = GetDC(hwnd);
        SelectPalette (hdc, hpal, FALSE);
        RealizePalette (hdc);
        UpdateColors(hdc);
        ReleaseDC(hwnd,hdc);
        break;

    case WM_DRAWCLIPBOARD:
    {

        if (hwndViewer)
        {
            SendMessage(hwndViewer,message, wParam, lParam);
        }

        EnableMenuItem(menu, IDM_PASTE, MF_ENABLED);
        InvalidateRect(hwnd, NULL, true);
        return 0;
    }

    case WM_CHANGECBCHAIN:
    {
        if((HWND)wParam == hwndViewer)
        {
            hwndViewer = (HWND)lParam;
        }
        else if(hwndViewer)
        {
            SendMessage(hwndViewer,message,wParam, lParam);
        }

        return 0;
    }



    case WM_COMMAND:
        switch LOWORD(wParam)
        {

        case IDM_OPEN_BM:
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
            return 0;
        }

        case IDM_PRINT_BM:
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

            return 0;
        }

        case IDM_SAVE_BM:
        {
            BOOL result = SaveFileDialog(hwnd,szFileName,(TCHAR*)"Save a Bitmap.");
            if(result != false)
            {
                PBITMAPINFO pbi = CreateBitmapInfoStruct(hwnd, hBitmap);
                hdc= GetDC(hwnd);
                SaveBMPFile(hwnd, szFileName, pbi, hBitmap, hdc);
            }
            return 0;
        }

        case IDM_EXIT:
        {
            PostQuitMessage(0);
            return 0;

        }

        case IDM_COPY:
        {
            int cutwidth = (int)abs(StartxPos - EndxPos);
            int cutheight = (int)abs(StartyPos - EndyPos);

            HBITMAP hbmp;

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

            ZeroMemory(&hbmp, sizeof(HBITMAP));


            hbmp = CopyScreenToBitmap(hwnd,(int)StartxPos,(int)StartyPos,cutwidth,cutheight);
            if(hbmp == NULL)
            {
                MessageBox(hwnd,"Copy not created properly","Error",MB_OK);
            }


            BitmapToClipboard(hbmp, hwnd);
            DeleteBitmap(hbmp);


            return 0;
        }

        case IDM_CUT:
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
            return 0;
        }
        case IDM_PASTE:
        {
            paste = true;

            InvalidateRect(hwnd,NULL,true);
            return 0;
        }

        case IDM_ZOOMOUT:
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
            return 0;
        }

        case IDM_ZOOMIN:
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

            return 0;
        }

        break;
        }

    case WM_PAINT:
    {


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
        return 0;
    }



    case  WM_SIZE:
    {


        cxsize = LOWORD(lParam);
        cysize = HIWORD(lParam);
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
        break;
    }
    case WM_HSCROLL:
    {
        int xDelta;     // xDelta = new_pos - current_pos
        int xNewPos;    // new position
        int yDelta = 0;

        switch (LOWORD(wParam))
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
            xNewPos = HIWORD(wParam);
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
            break;

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

        break;
    }


    case WM_VSCROLL:
    {
        int xDelta = 0;
        int yDelta;     // yDelta = new_pos - current_pos
        int yNewPos;    // new position

        switch (LOWORD(wParam))
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
            yNewPos = HIWORD(wParam);
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
            break;

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

        break;
    }


    case WM_DESTROY:
        ChangeClipboardChain(hwnd, hwndViewer);
        PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        break;
    default:                      /* for messages that we don't deal with */
        return DefWindowProc (hwnd, message, wParam, lParam);

    }
    return 0;

}


#endif // WINPROC_H_INCLUDED
