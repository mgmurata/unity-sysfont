#include "StdAfx.h"
#include "TextureUpdate.h"
#include <gl/gl.h>
#include <gl/glu.h>

//#define	OUTPUT_BMP

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")
#ifdef OUTPUT_BMP
BOOL SaveToBitmapFile(HDC hDC, LPCTSTR lpszFileName);
#endif	// OUTPUT_BMP

static const int BUFFER_SIZE = 1024;
static wchar_t buffer[BUFFER_SIZE];

TextureUpdate::TextureUpdate(const char* text, const char* fontName, int fontSize,
	bool isBold, bool isItalic, int alignment, int maxWidthPixels,
	int maxHeightPixels, int textureID): 
	fontSize(fontSize), isBold(isBold), isItalic(isItalic), alignment(DT_LEFT), _isReady(false),
	maxWidthPixels(maxWidthPixels), maxHeightPixels(maxHeightPixels), textureID(textureID),
	dc(NULL), font(NULL), bitmap(NULL)
{
	MultiByteToWideChar(CP_UTF8, NULL, text, -1, buffer, BUFFER_SIZE);
	this->text = buffer;
	MultiByteToWideChar(CP_UTF8, NULL, fontName, -1, buffer, BUFFER_SIZE);
	this->fontName = buffer;

	if(alignment == 1)
	{
		this->alignment = DT_CENTER;
	}
	else if(alignment == 2)
	{
		this->alignment = DT_RIGHT;
	}
	prepare();
}

TextureUpdate::~TextureUpdate(void)
{
	DeleteObject(font);
	DeleteObject(bitmap);
	DeleteDC(dc);
}

static int drawTextFlag = DT_WORDBREAK;
static const int INFO_SIZE = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;
static char bitmapInfo[INFO_SIZE];
static bool isBitmapInfoOk = false;
#ifdef OUTPUT_BMP
//static bool bSave_ = false;
#endif	// OUTPUT_BMP

void TextureUpdate::render() 
{
	BITMAPINFO* bmi = (BITMAPINFO*)bitmapInfo;
	if(!isBitmapInfoOk)
	{
		ZeroMemory(bmi, INFO_SIZE);
		bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi->bmiHeader.biPlanes = 1;
		bmi->bmiHeader.biBitCount = 8;
		bmi->bmiHeader.biCompression = BI_RGB;
		for(int i=0; i<256; i++)
		{
			bmi->bmiColors[i].rgbRed = i;
			bmi->bmiColors[i].rgbGreen = i;
			bmi->bmiColors[i].rgbBlue = i;
			bmi->bmiColors[i].rgbReserved = 0;
		}
		isBitmapInfoOk = true;
	}
	bmi->bmiHeader.biWidth = textureWidth;
	bmi->bmiHeader.biHeight = textureHeight;
	void *data = NULL;
	bitmap = CreateDIBSection(GetDC(NULL), bmi, DIB_RGB_COLORS, &data, NULL, 0);
	SelectObject(dc, bitmap);

	SetTextColor(dc, RGB(255, 255, 255)); 
	SetBkColor(dc, RGB(0, 0, 0));
	SetBkMode(dc, OPAQUE);

	RECT rect;
	rect.left = 0;
	rect.top = textureHeight - textHeight;
	rect.right = textWidth;
	rect.bottom = textureHeight;

	BOOL result = DrawText(dc, text.c_str(), -1, &rect, drawTextFlag|alignment);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA8, textureWidth, textureHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

#ifdef OUTPUT_BMP
if(!bSave_){
	SaveToBitmapFile(dc, L"aaaa.bmp");
	bSave_ = true;
}
#endif	// OUTPUT_BMP
}

HFONT TextureUpdate::getFont() 
{
	const wchar_t* fontFamily = NULL;
	if(fontName.size())
	{
		fontFamily = fontName.c_str();
	}
	int weight = isBold ? FW_BOLD: FW_NORMAL;
	HFONT font = CreateFont(
		-fontSize,
		0,
		0,
		0,
		weight,
		isItalic ? TRUE: FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE,
		fontFamily
	);
	return font;
}

static int getNextPowerOfTwo(int n) 
{
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	++n;
	return (n <= 0) ? 1 : n;
}

void TextureUpdate::prepare() 
{
	dc = CreateCompatibleDC(GetDC(NULL));
	SetMapMode(dc, MM_TEXT);
	SetTextAlign(dc, TA_TOP|TA_LEFT|TA_NOUPDATECP);
	font = getFont();
	SelectObject(dc, font);

	RECT rect;
	rect.top = 0;
	rect.bottom = maxHeightPixels;
	rect.left = 0;
	rect.right = maxWidthPixels;
	DrawText(dc, text.c_str(), -1, &rect, DT_CALCRECT|drawTextFlag|alignment);

	textWidth = rect.right - rect.left;
	if (textWidth > maxWidthPixels) 
	{
		textWidth = maxWidthPixels;
	}
	else if (textWidth <= 0) 
	{
		textWidth = 1;
	}

	textHeight = rect.bottom - rect.top;
	if (textHeight > maxHeightPixels) 
	{
		textHeight = maxHeightPixels;
	}
	else if (textHeight <= 0) 
	{
		textHeight = 1;
	}

	textureWidth = getNextPowerOfTwo(textWidth);
	textureHeight = getNextPowerOfTwo(textHeight);

}

#ifdef OUTPUT_BMP
// --------------------------------------------------
// メモリに描画した内容をビットマップに出力する
// 引数   : HDC     - デバイスコンテキストのハンドル
//          LPCTSTR - 出力するファイル名
// 戻り値 : BOOL
//          ビットマップ出力が成功すれば[TRUE]を返す
//          ビットマップ出力が失敗すれば[FALSE]を返す
// --------------------------------------------------
BOOL SaveToBitmapFile(HDC hDC, LPCTSTR lpszFileName)
{
    int errorCode = 0;  // エラーチェック変数

    HBITMAP hBmp = (HBITMAP)GetCurrentObject(hDC, OBJ_BITMAP);  // 現在選択されているビットマップを取得する
    if(hBmp == NULL)
        return FALSE;  // GetCurrentObject() がエラー

    BITMAPINFO  sBmpInfo;  // BITMAPINFO 構造体、Windows のデバイスに依存しないビットマップ (DIB) の寸法と色の情報を定義する
    sBmpInfo.bmiHeader.biSize = sizeof(sBmpInfo.bmiHeader);
    sBmpInfo.bmiHeader.biBitCount = 0;
    errorCode = GetDIBits(hDC, hBmp, 0, 0, NULL, &sBmpInfo, DIB_RGB_COLORS);  // 指定されたビットマップのビットを取得し、指定された形式でバッファにコピーする
    if(errorCode == 0)
        return FALSE;  // GetDIBits() がエラー

    ULONG iBmpInfoSize;
    switch(sBmpInfo.bmiHeader.biBitCount)  // 1ピクセルのカラービット数を判別する
    {
        case 24:  // 1677万色(true color)ビットマップ
            iBmpInfoSize = sizeof(BITMAPINFOHEADER);
            break;
        case 16:  // 65536色(high color)ビットマップ
        case 32:  // 1677万色+α(true color)ビットマップ
            iBmpInfoSize = sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * 3;
            break;
        default:
            iBmpInfoSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << sBmpInfo.bmiHeader.biBitCount);
            break;
    }

    PBITMAPINFO psBmpInfo = &sBmpInfo;
    if(iBmpInfoSize != sizeof(BITMAPINFOHEADER))
    {
        psBmpInfo = (PBITMAPINFO)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, iBmpInfoSize);  // ヒープから指定されたバイト数のメモリを確保する
        if(psBmpInfo == NULL)
            return FALSE;  // GlobalAlloc() がエラー
        PBYTE pbtBmpInfoDest = (PBYTE)psBmpInfo;
        PBYTE pbtBmpInfoSrc = (PBYTE)&sBmpInfo;
        ULONG iSizeTmp = sizeof(BITMAPINFOHEADER);

        // ビットマップデータを複製する
        while(iSizeTmp--)
        {
            *pbtBmpInfoDest = *pbtBmpInfoSrc;
            pbtBmpInfoDest++;
            pbtBmpInfoSrc++;
        }
    }

    // ビットマップファイルの作成
    HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        return FALSE;  // CreateFile() がエラー
    }

    BITMAPFILEHEADER sBmpFileHder;
    sBmpFileHder.bfType = MAKEWORD('B', 'M');  // 'BM' 16-bit 整数を作成する(DIB形式のファイル)
    sBmpFileHder.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + iBmpInfoSize + psBmpInfo->bmiHeader.biSizeImage;
    sBmpFileHder.bfReserved1 = 0;
    sBmpFileHder.bfReserved2 = 0;
    sBmpFileHder.bfOffBits = sizeof(BITMAPFILEHEADER) + iBmpInfoSize;

    DWORD dwRet;
    errorCode = WriteFile(hFile, (LPCVOID)&sBmpFileHder, sizeof(BITMAPFILEHEADER), &dwRet, NULL);
    if(errorCode == 0)
    {
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        return FALSE;  // WriteFile() がエラー
    }

    PBYTE pBits = (PBYTE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sBmpInfo.bmiHeader.biSizeImage);
    if(pBits == NULL)
    {
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        return FALSE;  // GlobalAlloc() がエラー
    }

    // ビットマップを作成する
    HBITMAP hTmpBmp = CreateCompatibleBitmap(hDC, psBmpInfo->bmiHeader.biWidth, psBmpInfo->bmiHeader.biHeight);
    if(hTmpBmp == NULL)
    {
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        if(pBits != NULL)
            GlobalFree(pBits);
        return FALSE;  // CreateCompatibleBitmap() がエラー
    }
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hDC, hTmpBmp);
    if(hOldBmp == NULL)
    {
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        if(pBits != NULL)
            GlobalFree(pBits);
        return FALSE;  // SelectObject() がエラー
    }
    errorCode = GetDIBits(hDC, hBmp, 0, psBmpInfo->bmiHeader.biHeight, (LPSTR)pBits, psBmpInfo, DIB_RGB_COLORS);
    if(errorCode == 0)
    {
        SelectObject(hDC, hOldBmp);
        DeleteObject(hTmpBmp);
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        if(pBits != NULL)
            GlobalFree(pBits);
        return FALSE;  // GetDIBits() がエラー
    }

    errorCode = WriteFile(hFile, (LPCVOID)psBmpInfo, iBmpInfoSize, &dwRet, NULL);
    if(errorCode == 0)
    {
        SelectObject(hDC, hOldBmp);
        DeleteObject(hTmpBmp);
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        if(pBits != NULL)
            GlobalFree(pBits);
        return FALSE;  // WriteFile() がエラー
    }

    errorCode = WriteFile(hFile, (LPCVOID)pBits, psBmpInfo->bmiHeader.biSizeImage, &dwRet, NULL);
    if(errorCode == 0)
    {
        SelectObject(hDC, hOldBmp);
        DeleteObject(hTmpBmp);
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        if(pBits != NULL)
            GlobalFree(pBits);
        return FALSE;  // WriteFile() がエラー
    }

    SelectObject(hDC, hOldBmp);
    DeleteObject(hTmpBmp);
    CloseHandle(hFile);
    GlobalFree(psBmpInfo);
    GlobalFree(pBits);

    return TRUE;
}
#endif	// OUTPUT_BMP
