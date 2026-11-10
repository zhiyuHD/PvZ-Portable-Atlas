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

#ifndef __SYSFONT_H__
#define __SYSFONT_H__

#include "Font.h"
#include <string>
#include <unordered_map>
#include <vector>

struct FT_LibraryRec_;
struct FT_FaceRec_;
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

namespace Sexy
{

class Image;
class Graphics;

// 基于 FreeType 的系统字体，支持任意 Unicode（含中文）
class SysFont : public _Font
{
public:
	SysFont(const std::string& theFontFilePath, int thePointSize);
	SysFont(const void* theData, size_t theDataLen, int thePointSize);  // 从内存字节加载（如 pak 内的字体）
	virtual ~SysFont();

	bool IsValid() const { return mFTFace != nullptr; }
	bool HasGlyph(char32_t theChar) const;
	// Non-owning fallback; cached system fonts outlive Almanac widgets.
	void SetFallbackFont(SysFont* theFont) { mFallbackFont = theFont == this ? nullptr : theFont; }

	virtual int StringWidth(std::string_view theString) override;
	virtual int CharWidth(char32_t theChar) override;
	virtual int CharWidthKern(char32_t theChar, char32_t thePrevChar) override;
	virtual void DrawString(Graphics* g, int theX, int theY, std::string_view theString, const Color& theColor, const Rect& theClipRect) override;
	virtual _Font* Duplicate() override;

private:
	struct Glyph
	{
		Image* mImage;      // 字形贴图（白色，alpha=覆盖率）
		int mAdvance;
		int mBearingX;
		int mBearingY;
	};

	const Glyph* GetGlyph(char32_t theChar);
	bool RenderGlyph(char32_t theChar, Glyph& theGlyph);

	SysFont* mFallbackFont = nullptr;
	std::string mFontFilePath;
	std::vector<uint8_t> mFontData;  // 内存字体数据（需在 face 生命周期内有效）
	FT_Library mFTLibrary;
	FT_Face mFTFace;
	int mPointSize;
	std::unordered_map<char32_t, Glyph> mGlyphs;
};

// 便捷：缓存并返回一个 CJK 字体（优先 pak 内置宋体，其次系统字体）
SysFont* GetCJKFont(int thePointSize);
SysFont* GetSystemCJKFont(int thePointSize);

}

#endif //__SYSFONT_H__
