#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
#include <Windows.h>
#include <WindowsX.h>

extern char szFileName[500];

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

#endif // FUNCTIONS_H_INCLUDED
