/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

#include "SysFont.h"
#include "Graphics.h"
#include "MemoryImage.h"
#include "Common.h"
#include "../SexyAppBase.h"
#include "../misc/Buffer.h"

#ifdef PVZ_HAS_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cstdio>
#include <filesystem>
#include <vector>
#endif

namespace Sexy
{

#ifdef PVZ_HAS_FREETYPE

SysFont::SysFont(const std::string& theFontFilePath, int thePointSize)
	: mFontFilePath(theFontFilePath), mFTLibrary(nullptr), mFTFace(nullptr), mPointSize(thePointSize)
{
	if (FT_Init_FreeType(&mFTLibrary) != 0)
		return;

	if (FT_New_Face(mFTLibrary, theFontFilePath.c_str(), 0, &mFTFace) != 0)
	{
		FT_Done_FreeType(mFTLibrary);
		mFTLibrary = nullptr;
		return;
	}

	FT_Set_Pixel_Sizes(mFTFace, 0, mPointSize);
	mAscent = mFTFace->size->metrics.ascender >> 6;
	mAscentPadding = 0;
	mHeight = mFTFace->size->metrics.height >> 6;
	mLineSpacingOffset = 0;
}

SysFont::SysFont(const void* theData, size_t theDataLen, int thePointSize)
	: mFTLibrary(nullptr), mFTFace(nullptr), mPointSize(thePointSize)
{
	if (theData == nullptr || theDataLen == 0)
		return;

	mFontData.assign(static_cast<const uint8_t*>(theData), static_cast<const uint8_t*>(theData) + theDataLen);

	if (FT_Init_FreeType(&mFTLibrary) != 0)
		return;

	if (FT_New_Memory_Face(mFTLibrary, mFontData.data(), static_cast<FT_Long>(mFontData.size()), 0, &mFTFace) != 0)
	{
		FT_Done_FreeType(mFTLibrary);
		mFTLibrary = nullptr;
		return;
	}

	FT_Set_Pixel_Sizes(mFTFace, 0, mPointSize);
	mAscent = mFTFace->size->metrics.ascender >> 6;
	mAscentPadding = 0;
	mHeight = mFTFace->size->metrics.height >> 6;
	mLineSpacingOffset = 0;
}

SysFont::~SysFont()
{
	for (auto& aPair : mGlyphs)
	{
		delete aPair.second.mImage;
	}
	if (mFTFace)
		FT_Done_Face(mFTFace);
	if (mFTLibrary)
		FT_Done_FreeType(mFTLibrary);
}

bool SysFont::HasGlyph(char32_t theChar) const
{
	return mFTFace && FT_Get_Char_Index(mFTFace, theChar) != 0;
}

bool SysFont::RenderGlyph(char32_t theChar, Glyph& theGlyph)
{
	theGlyph.mImage = nullptr;
	theGlyph.mAdvance = 0;
	theGlyph.mBearingX = 0;
	theGlyph.mBearingY = 0;

	if (!mFTFace)
		return false;

	FT_UInt aGlyphIndex = FT_Get_Char_Index(mFTFace, theChar);
	if (aGlyphIndex == 0)
		return false;  // 字体里没有这个字符

	if (FT_Load_Glyph(mFTFace, aGlyphIndex, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL) != 0)
		return false;

	FT_Bitmap& aBitmap = mFTFace->glyph->bitmap;
	theGlyph.mAdvance = mFTFace->glyph->advance.x >> 6;
	theGlyph.mBearingX = mFTFace->glyph->bitmap_left;
	theGlyph.mBearingY = -(mFTFace->glyph->bitmap_top);  // 基线对齐

	if (aBitmap.width == 0 || aBitmap.rows == 0)
		return true;  // 空白字符（如空格），无贴图

	int aW = static_cast<int>(aBitmap.width);
	int aH = static_cast<int>(aBitmap.rows);

	MemoryImage* aImage = new MemoryImage();
	uint32_t* aBits = new uint32_t[aW * aH + 1];
	for (int y = 0; y < aH; y++)
	{
		for (int x = 0; x < aW; x++)
		{
			uint8_t aCoverage = aBitmap.buffer[y * aBitmap.pitch + x];
			// ARGB：白色字形，alpha=覆盖率（绘制时用 SetColorizeImages 染色）
			aBits[y * aW + x] = (static_cast<uint32_t>(aCoverage) << 24) | 0x00FFFFFF;
		}
	}
	aBits[aW * aH] = Sexy::MEMORYCHECK_ID;
	aImage->mBits = aBits;
	aImage->mWidth = aW;
	aImage->mHeight = aH;
	aImage->mHasTrans = true;
	aImage->mHasAlpha = true;
	aImage->BitsChanged();

	theGlyph.mImage = aImage;
	return true;
}

const SysFont::Glyph* SysFont::GetGlyph(char32_t theChar)
{
	if (!HasGlyph(theChar) && mFallbackFont && mFallbackFont->HasGlyph(theChar))
		return mFallbackFont->GetGlyph(theChar);
	auto anItr = mGlyphs.find(theChar);
	if (anItr != mGlyphs.end())
		return &anItr->second;

	Glyph aGlyph;
	if (!RenderGlyph(theChar, aGlyph))
	{
		// 缺字或空格：缓存空字形，避免反复尝试；前进量用最大推进
		aGlyph.mImage = nullptr;
		aGlyph.mAdvance = mFTFace ? (mFTFace->size->metrics.max_advance >> 6) : 0;
		aGlyph.mBearingX = 0;
		aGlyph.mBearingY = 0;
	}

	return &(mGlyphs.emplace(theChar, aGlyph).first->second);
}

int SysFont::CharWidth(char32_t theChar)
{
	const Glyph* aGlyph = GetGlyph(theChar);
	return aGlyph ? aGlyph->mAdvance : 0;
}

int SysFont::CharWidthKern(char32_t theChar, char32_t thePrevChar)
{
	(void)thePrevChar;
	return CharWidth(theChar);
}

int SysFont::StringWidth(std::string_view theString)
{
	int aWidth = 0;
	size_t aOffset = 0;
	char32_t aChar = 0;
	while (UTF8DecodeNext(theString, aOffset, aChar))
	{
		const Glyph* aGlyph = GetGlyph(aChar);
		aWidth += aGlyph ? aGlyph->mAdvance : 0;
	}
	return aWidth;
}

void SysFont::DrawString(Graphics* g, int theX, int theY, std::string_view theString, const Color& theColor, const Rect& theClipRect)
{
	int aPenX = theX;
	size_t aOffset = 0;
	char32_t aChar = 0;

	while (UTF8DecodeNext(theString, aOffset, aChar))
	{
		const Glyph* aGlyph = GetGlyph(aChar);
		if (aGlyph && aGlyph->mImage)
		{
			g->SetColorizeImages(true);
			g->SetColor(theColor);
			g->DrawImage(aGlyph->mImage, aPenX + aGlyph->mBearingX, theY + aGlyph->mBearingY);
		}
		aPenX += aGlyph ? aGlyph->mAdvance : 0;
	}
	g->SetColorizeImages(false);
}

_Font* SysFont::Duplicate()
{
	SysFont* font = !mFontData.empty() ? new SysFont(mFontData.data(), mFontData.size(), mPointSize) : new SysFont(mFontFilePath, mPointSize);
	font->SetFallbackFont(mFallbackFont);
	return font;
}

SysFont* GetCJKFont(int thePointSize)
{
	static std::unordered_map<int, SysFont*> sCache;
	auto anItr = sCache.find(thePointSize);
	if (anItr != sCache.end())
		return anItr->second;

	SysFont* aResult = nullptr;

	// 优先：从 pak 内置宋体加载（自包含，不依赖系统字体）
	{
		Buffer aBuffer;
		if (gSexyAppBase && gSexyAppBase->ReadBufferFromFile("reanim/NotoSerifCJK_SC.otf", &aBuffer, true) &&
			aBuffer.GetDataLen() > 0)
		{
			aResult = new SysFont(aBuffer.GetDataPtr(), aBuffer.GetDataLen(), thePointSize);
		}
	}

	if (!(aResult && aResult->IsValid()))
	{
		delete aResult;
		return GetSystemCJKFont(thePointSize);
	}

	if (aResult && aResult->IsValid())
	{
		sCache[thePointSize] = aResult;
		return aResult;
	}
	delete aResult;
	return nullptr;
}

SysFont* GetSystemCJKFont(int thePointSize)
{
	static std::unordered_map<int, SysFont*> cache;
	if (auto found = cache.find(thePointSize); found != cache.end()) return found->second;
	SysFont* aResult = nullptr;
	std::string aFontPath;
#if !defined(_WIN32) && !defined(__ANDROID__) && !defined(__APPLE__)
	FILE* aPipe = popen("fc-match -f \"%{file}\" \"sans:lang=zh\"", "r");
	if (aPipe)
	{
		char aBuf[1024] = {};
		if (fgets(aBuf, sizeof(aBuf), aPipe))
		{
			aFontPath = aBuf;
			while (!aFontPath.empty() && (aFontPath.back() == '\n' || aFontPath.back() == '\r'))
				aFontPath.pop_back();
		}
		pclose(aPipe);
	}
#endif

	static const std::vector<std::string> aCandidates = {
		"/system/fonts/NotoSansCJK-Regular.ttc",
		"/system/fonts/NotoSerifCJK-Regular.ttc",
		"/system/fonts/NotoSansSC-Regular.otf",
		"/system/fonts/DroidSansFallback.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"/System/Library/Fonts/PingFang.ttc",
		"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/noto-cjk/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/opentype/noto-cjk/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
		"/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
		"/usr/share/fonts/truetype/arphic/uming.ttc",
		"/usr/share/fonts/opentype/unifont/unifont.otf",
		"/usr/share/fonts/unifont/unifont.otb"
	};

	if (std::filesystem::exists(aFontPath))
	{
		aResult = new SysFont(aFontPath, thePointSize);
	}
	else
	{
		for (const std::string& aPath : aCandidates)
		{
			if (std::filesystem::exists(aPath))
			{
				aResult = new SysFont(aPath, thePointSize);
				break;
			}
		}
	}
	if (aResult && !aResult->IsValid()) { delete aResult; aResult = nullptr; }
	cache[thePointSize] = aResult;
	return aResult;
}

#else

// 无 FreeType 时（Android/iOS/Switch/3DS/Wasm 等）的 stub：一律无效，回退到位图字体
SysFont::SysFont(const std::string& theFontFilePath, int thePointSize)
	: mFontFilePath(theFontFilePath), mFTLibrary(nullptr), mFTFace(nullptr), mPointSize(thePointSize)
{
}

SysFont::SysFont(const void* theData, size_t theDataLen, int thePointSize)
	: mFTLibrary(nullptr), mFTFace(nullptr), mPointSize(thePointSize)
{
	(void)theData;
	(void)theDataLen;
}

SysFont::~SysFont()
{
	for (auto& aPair : mGlyphs)
	{
		delete aPair.second.mImage;
	}
}

bool SysFont::HasGlyph(char32_t) const { return false; }
SysFont* GetSystemCJKFont(int) { return nullptr; }

bool SysFont::RenderGlyph(char32_t theChar, Glyph& theGlyph)
{
	(void)theChar;
	theGlyph.mImage = nullptr;
	theGlyph.mAdvance = 0;
	theGlyph.mBearingX = 0;
	theGlyph.mBearingY = 0;
	return false;
}

const SysFont::Glyph* SysFont::GetGlyph(char32_t theChar)
{
	(void)theChar;
	return nullptr;
}

int SysFont::CharWidth(char32_t theChar)
{
	(void)theChar;
	return 0;
}

int SysFont::CharWidthKern(char32_t theChar, char32_t thePrevChar)
{
	(void)theChar;
	(void)thePrevChar;
	return 0;
}

int SysFont::StringWidth(std::string_view theString)
{
	(void)theString;
	return 0;
}

void SysFont::DrawString(Graphics* g, int theX, int theY, std::string_view theString, const Color& theColor, const Rect& theClipRect)
{
	(void)g;
	(void)theX;
	(void)theY;
	(void)theString;
	(void)theColor;
	(void)theClipRect;
}

_Font* SysFont::Duplicate()
{
	return new SysFont(mFontFilePath, mPointSize);
}

SysFont* GetCJKFont(int thePointSize)
{
	(void)thePointSize;
	return nullptr;
}

#endif

}
