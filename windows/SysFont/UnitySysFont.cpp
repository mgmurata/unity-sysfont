// UnitySysFont.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "TextureUpdate.h"
#include "UnityPluginInterface.h"
#include <d3d9.h>
#include <map>

class Sync
{
public:
	CRITICAL_SECTION* _cs;
	Sync(CRITICAL_SECTION* cs): _cs(cs) 
	{
		EnterCriticalSection(_cs);
	}

	~Sync()
	{
		LeaveCriticalSection(_cs);
	}
};

class UnitySysFont 
{
private:
	static const int UPDATE_QUEUE_CAPACITY = 32;

	CRITICAL_SECTION cs;
	std::map<int, TextureUpdate*> updateQueue;

public:
	UnitySysFont() {
		//updateQueue = new HashMap<Integer, TextureUpdate>(UPDATE_QUEUE_CAPACITY);
		InitializeCriticalSection(&cs);
	}

	~UnitySysFont()
	{
		DeleteCriticalSection(&cs);
	}

	void queueTexture(const char* text, const char* fontName, int fontSize,
		bool isBold, bool isItalic, int alignment, int maxWidthPixels,
		int maxHeightPixels, int textureID) 
	{
		//if(updateQueue.find(textureID) == updateQueue.end())
		//{
			TextureUpdate* update = new TextureUpdate(text, fontName, fontSize, isBold,
				isItalic, alignment, maxWidthPixels, maxHeightPixels, textureID);
			Sync sync(&cs);
			updateQueue[textureID] = update;
		//}
	}

	void updateQueuedTexture(int textureID) {
		Sync sync(&cs);
		std::map<int, TextureUpdate*>::iterator i = updateQueue.find(textureID);
		if (i != updateQueue.end()) {
			i->second->setReady();
		}
	}

	void dequeueTexture(int textureID) {
		Sync sync(&cs);
		std::map<int, TextureUpdate*>::iterator i = updateQueue.find(textureID);
		if(i != updateQueue.end()) 
		{
			delete i->second;
			updateQueue.erase(i);
		}
	}

	int getTextureWidth(int textureID) {
		Sync sync(&cs);
		std::map<int, TextureUpdate*>::iterator i = updateQueue.find(textureID);
		if (i == updateQueue.end()) {
			return -1;
		}
		return i->second->getTextureWidth();
	}

	int getTextureHeight(int textureID) {
		Sync sync(&cs);
		std::map<int, TextureUpdate*>::iterator i = updateQueue.find(textureID);
		if (i == updateQueue.end()) {
			return -1;
		}
		return i->second->getTextureHeight();
	}

	int getTextWidth(int textureID) {
		Sync sync(&cs);
		std::map<int, TextureUpdate*>::iterator i = updateQueue.find(textureID);
		if (i == updateQueue.end()) {
			return -1;
		}
		return i->second->getTextWidth();
	}

	int getTextHeight(int textureID) {
		Sync sync(&cs);
		std::map<int, TextureUpdate*>::iterator i = updateQueue.find(textureID);
		if (i == updateQueue.end()) {
			return -1;
		}
		return i->second->getTextHeight();
	}

	void processQueue() {
		Sync sync(&cs);
		if (updateQueue.empty() == false) {
			std::map<int, TextureUpdate*>::iterator i = updateQueue.begin();
			while (i != updateQueue.end()) {
				TextureUpdate* update = i->second;
				if (update->isReady()) {
					update->render();
					delete update;
					updateQueue.erase(i++);
				}
				else
				{
					++i;
				}
			}
		}
	}
};

static UnitySysFont instance;

extern "C"
{

void EXPORT_API _SysFontQueueTexture(const char *text, const char *fontName,
    int fontSize, BOOL isBold, BOOL isItalic, int alignment,
    int maxWidthPixels, int maxHeightPixels, int textureID)
{
	instance.queueTexture(text, fontName, fontSize, isBold == TRUE, isItalic == TRUE, alignment, maxWidthPixels, maxHeightPixels, textureID);
}

int EXPORT_API _SysFontGetTextureWidth(int textureID)
{
	return instance.getTextureWidth(textureID);
}

int EXPORT_API _SysFontGetTextureHeight(int textureID)
{
	return instance.getTextureHeight(textureID);
}

int EXPORT_API _SysFontGetTextWidth(int textureID)
{
	return instance.getTextWidth(textureID);
}

int EXPORT_API _SysFontGetTextHeight(int textureID)
{
	return instance.getTextHeight(textureID);
}

void EXPORT_API _SysFontUpdateQueuedTexture(int textureID)
{
	instance.updateQueuedTexture(textureID);
}

void EXPORT_API _SysFontRender()
{
	instance.processQueue();
}

void EXPORT_API _SysFontDequeueTexture(int textureID)
{
	instance.dequeueTexture(textureID);
}

void EXPORT_API UnityRenderEvent(int eventID)
{
	_SysFontRender();
}

void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
{
	if (deviceType == kGfxRendererOpenGL)
	{
	}
}

}