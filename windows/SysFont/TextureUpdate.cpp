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
// �������ɕ`�悵�����e���r�b�g�}�b�v�ɏo�͂���
// ����   : HDC     - �f�o�C�X�R���e�L�X�g�̃n���h��
//          LPCTSTR - �o�͂���t�@�C����
// �߂�l : BOOL
//          �r�b�g�}�b�v�o�͂����������[TRUE]��Ԃ�
//          �r�b�g�}�b�v�o�͂����s�����[FALSE]��Ԃ�
// --------------------------------------------------
BOOL SaveToBitmapFile(HDC hDC, LPCTSTR lpszFileName)
{
    int errorCode = 0;  // �G���[�`�F�b�N�ϐ�

    HBITMAP hBmp = (HBITMAP)GetCurrentObject(hDC, OBJ_BITMAP);  // ���ݑI������Ă���r�b�g�}�b�v���擾����
    if(hBmp == NULL)
        return FALSE;  // GetCurrentObject() ���G���[

    BITMAPINFO  sBmpInfo;  // BITMAPINFO �\���́AWindows �̃f�o�C�X�Ɉˑ����Ȃ��r�b�g�}�b�v (DIB) �̐��@�ƐF�̏����`����
    sBmpInfo.bmiHeader.biSize = sizeof(sBmpInfo.bmiHeader);
    sBmpInfo.bmiHeader.biBitCount = 0;
    errorCode = GetDIBits(hDC, hBmp, 0, 0, NULL, &sBmpInfo, DIB_RGB_COLORS);  // �w�肳�ꂽ�r�b�g�}�b�v�̃r�b�g���擾���A�w�肳�ꂽ�`���Ńo�b�t�@�ɃR�s�[����
    if(errorCode == 0)
        return FALSE;  // GetDIBits() ���G���[

    ULONG iBmpInfoSize;
    switch(sBmpInfo.bmiHeader.biBitCount)  // 1�s�N�Z���̃J���[�r�b�g���𔻕ʂ���
    {
        case 24:  // 1677���F(true color)�r�b�g�}�b�v
            iBmpInfoSize = sizeof(BITMAPINFOHEADER);
            break;
        case 16:  // 65536�F(high color)�r�b�g�}�b�v
        case 32:  // 1677���F+��(true color)�r�b�g�}�b�v
            iBmpInfoSize = sizeof(BITMAPINFOHEADER) + sizeof(DWORD) * 3;
            break;
        default:
            iBmpInfoSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << sBmpInfo.bmiHeader.biBitCount);
            break;
    }

    PBITMAPINFO psBmpInfo = &sBmpInfo;
    if(iBmpInfoSize != sizeof(BITMAPINFOHEADER))
    {
        psBmpInfo = (PBITMAPINFO)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, iBmpInfoSize);  // �q�[�v����w�肳�ꂽ�o�C�g���̃��������m�ۂ���
        if(psBmpInfo == NULL)
            return FALSE;  // GlobalAlloc() ���G���[
        PBYTE pbtBmpInfoDest = (PBYTE)psBmpInfo;
        PBYTE pbtBmpInfoSrc = (PBYTE)&sBmpInfo;
        ULONG iSizeTmp = sizeof(BITMAPINFOHEADER);

        // �r�b�g�}�b�v�f�[�^�𕡐�����
        while(iSizeTmp--)
        {
            *pbtBmpInfoDest = *pbtBmpInfoSrc;
            pbtBmpInfoDest++;
            pbtBmpInfoSrc++;
        }
    }

    // �r�b�g�}�b�v�t�@�C���̍쐬
    HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        return FALSE;  // CreateFile() ���G���[
    }

    BITMAPFILEHEADER sBmpFileHder;
    sBmpFileHder.bfType = MAKEWORD('B', 'M');  // 'BM' 16-bit �������쐬����(DIB�`���̃t�@�C��)
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
        return FALSE;  // WriteFile() ���G���[
    }

    PBYTE pBits = (PBYTE)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sBmpInfo.bmiHeader.biSizeImage);
    if(pBits == NULL)
    {
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        return FALSE;  // GlobalAlloc() ���G���[
    }

    // �r�b�g�}�b�v���쐬����
    HBITMAP hTmpBmp = CreateCompatibleBitmap(hDC, psBmpInfo->bmiHeader.biWidth, psBmpInfo->bmiHeader.biHeight);
    if(hTmpBmp == NULL)
    {
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        if(pBits != NULL)
            GlobalFree(pBits);
        return FALSE;  // CreateCompatibleBitmap() ���G���[
    }
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hDC, hTmpBmp);
    if(hOldBmp == NULL)
    {
        CloseHandle(hFile);
        if(psBmpInfo != NULL)
            GlobalFree(psBmpInfo);
        if(pBits != NULL)
            GlobalFree(pBits);
        return FALSE;  // SelectObject() ���G���[
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
        return FALSE;  // GetDIBits() ���G���[
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
        return FALSE;  // WriteFile() ���G���[
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
        return FALSE;  // WriteFile() ���G���[
    }

    SelectObject(hDC, hOldBmp);
    DeleteObject(hTmpBmp);
    CloseHandle(hFile);
    GlobalFree(psBmpInfo);
    GlobalFree(pBits);

    return TRUE;
}
#endif	// OUTPUT_BMP
