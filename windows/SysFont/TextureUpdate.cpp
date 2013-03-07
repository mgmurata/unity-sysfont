#include "StdAfx.h"
#include "TextureUpdate.h"
#include <gl/gl.h>
#include <gl/glu.h>

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")

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
