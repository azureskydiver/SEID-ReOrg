#include "Functions.h"
#include <fstream>

using namespace std;

static OPENFILENAME ofn;
char szFileName[500]= "\0";


HDC GetPrinterDC (HWND Hwnd)
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

BOOL OpenFileDialog(HWND hwnd, LPTSTR pFileName ,LPTSTR pTitleName)

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

BOOL SaveFileDialog(HWND hwnd, LPTSTR pFileName ,LPTSTR pTitleName)

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

void InitialiseDialog(HWND hwnd)
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


void SaveBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
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


PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
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

bool BitmapToClipboard(HBITMAP hBM, HWND hWnd)
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



HBITMAP CopyScreenToBitmap(HWND Hwnd,int x1, int y1, int nWidth, int nHeight)
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


void PaintLoadBitmap(HWND hwnd,SCROLLINFO si, BITMAP bitmap, int pcxsize, int pcysize, int xMaxScroll,int xCurrentScroll, int xMinScroll,int yMaxScroll, int yCurrentScroll ,int yMinScroll)
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

void DrawBoundingBox(HDC hdc, int StartxPos, int EndxPos, int StartyPos, int EndyPos)
{
    HPEN greenPen=CreatePen(PS_SOLID, 1, RGB(0,255,0));
    SelectObject(hdc, greenPen);

    SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));


    MoveToEx(hdc, StartxPos, StartyPos, NULL);
    Rectangle(hdc, StartxPos, StartyPos , EndxPos, EndyPos);


}

void Cut(HDC hdc, HDC hdcMem, HBITMAP hBitmap, double Width, double Height, double oWidth, double oHeight, double StartxPos, double EndxPos, double StartyPos, double EndyPos, int xCurrentScroll, int yCurrentScroll)
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

void Paste(HWND hwnd, HBITMAP hBitmap,double Width,double Height ,double oWidth, double oHeight ,HDC hdc, HDC hdcMem, HDC MemoryDC, double StartxPos, double EndxPos, int xCurrentScroll, double StartyPos, double EndyPos, int yCurrentScroll )
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

void ZeroScrollbars(HWND hwnd, SCROLLINFO si, BITMAP bitmap, int cxsize, int cysize,int xCurrentScroll, int yCurrentScroll, int xMaxScroll, int yMaxScroll, int xMinScroll, int yMinScroll)
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
