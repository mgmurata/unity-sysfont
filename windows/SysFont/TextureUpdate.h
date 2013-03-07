#pragma once

#include <string>

class TextureUpdate
{
public:
	TextureUpdate(const char* text, const char* fontName, int fontSize,
		bool isBold, bool isItalic, int alignment, int maxWidthPixels,
		int maxHeightPixels, int textureID);

	~TextureUpdate(void);

	int getTextureWidth() {
		return textureWidth;
	}

	int getTextureHeight() {
		return textureHeight;
	}

	int getTextWidth() {
		return textWidth;
	}

	int getTextHeight() {
		return textHeight;
	}

	bool isReady() {
		return _isReady;
	}

	void setReady() {
		_isReady = true;
	}

	void render();

private:

	HFONT getFont();

	void prepare();

	std::wstring text;
	std::wstring fontName;
	int fontSize;
	bool isBold;
	bool isItalic;
	int alignment;
	int maxWidthPixels;
	int maxHeightPixels;
	int textureID;

	bool _isReady;

	int textWidth;
	int textHeight;
	int textureWidth;
	int textureHeight;

	HFONT font;
	HDC dc;
	HBITMAP bitmap;
};

