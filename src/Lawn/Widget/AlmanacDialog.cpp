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

#include "../Board.h"
#include "../Plant.h"
#include "../Zombie.h"
#include "GameButton.h"
#include "../SeedPacket.h"
#include "../../LawnApp.h"
#include "AlmanacDialog.h"
#include "../../Resources.h"
#include "../System/Music.h"
#include "../../GameConstants.h"
#include "../System/PlayerInfo.h"
#include "../System/PoolEffect.h"
#include "../System/ReanimationLawn.h"
#include "../../Sexy.TodLib/TodStringFile.h"
#include "widget/WidgetManager.h"
#include "graphics/SysFont.h"
#include "AlmanacText.h"
#include <algorithm>
#include <array>

namespace
{
// 图鉴变种使用独立条目 ID，预览时映射回普通僵尸及其变种标记。
constexpr ZombieType ALMANAC_CART_ZOMBIE = static_cast<ZombieType>(NUM_CACHED_ZOMBIE_TYPES);
constexpr int ALMANAC_ERROR_ZOMBIE_INDEX = ZOMBIE_BOSS + 1;
constexpr int ALMANAC_CART_ZOMBIE_INDEX = ALMANAC_ERROR_ZOMBIE_INDEX + 1;

_Font* AlmanacFont(int size, _Font* fallback)
{
	static std::unordered_map<int, SysFont*> fonts;
	if (auto found = fonts.find(size); found != fonts.end()) return found->second;
	SysFont* primary = Sexy::GetCJKFont(size);
	if (!primary || !primary->IsValid()) return fallback;
	// Keep Almanac fallback independent of the game's shared font cache.
	SysFont* font = static_cast<SysFont*>(primary->Duplicate());
	font->SetFallbackFont(Sexy::GetSystemCJKFont(size));
	fonts[size] = font;
	return font;
}
}

bool gZombieDefeated[NUM_ZOMBIE_TYPES] = { false };

AlmanacDialog::AlmanacDialog(LawnApp* theApp) : LawnDialog(theApp, DIALOG_ALMANAC, true, "图鉴", "", "", BUTTONS_NONE)
{
	mApp = (LawnApp*)gSexyAppBase;
	mOpenPage = ALMANAC_PAGE_INDEX;
	mSelectedSeed = SEED_PEASHOOTER;
	mSelectedZombie = ZOMBIE_NORMAL;
	mZombie = nullptr;
	mPlant = nullptr;
	mDrawStandardBack = false;
	mLoadedResourceNames.push_back("DelayLoad_Almanac");
	for (size_t i = 0; i < LENGTH(mZombiePerfTest); i++) mZombiePerfTest[i] = nullptr;
	LawnDialog::Resize(0, 0, BOARD_WIDTH, BOARD_HEIGHT);

	for (std::string& resource : mLoadedResourceNames)
		TodLoadResources(resource.c_str());

	mCloseButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_CLOSE);
	mCloseButton->SetLabel("关闭");
	mCloseButton->mButtonImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTON;
	mCloseButton->mOverImage = Sexy::IMAGE_ALMANAC_CLOSEBUTTONHIGHLIGHT;
	mCloseButton->mDownImage = nullptr;
	mCloseButton->SetFont(AlmanacFont(14, Sexy::FONT_BRIANNETOD12));
	Color aColor = Color(42, 42, 90);
	mCloseButton->mColors[ButtonWidget::COLOR_LABEL] = aColor;
	mCloseButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = aColor;
	mCloseButton->Resize(676, 567, 89, 26);
	mCloseButton->mParentWidget = this;
	mCloseButton->mTextOffsetX = -8;
	mCloseButton->mTextOffsetY = 1;

	mIndexButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_INDEX);
	mIndexButton->SetLabel("图鉴目录");
	mIndexButton->mButtonImage = Sexy::IMAGE_ALMANAC_INDEXBUTTON;
	mIndexButton->mOverImage = Sexy::IMAGE_ALMANAC_INDEXBUTTONHIGHLIGHT;
	mIndexButton->mDownImage = nullptr;
	mIndexButton->SetFont(AlmanacFont(14, Sexy::FONT_BRIANNETOD12));
	mIndexButton->mColors[ButtonWidget::COLOR_LABEL] = aColor;
	mIndexButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = aColor;
	mIndexButton->Resize(32, 567, 164, 26);
	mIndexButton->mParentWidget = this;
	mIndexButton->mTextOffsetX = 8;
	mIndexButton->mTextOffsetY = 1;

	mPlantButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_PLANT);
	mPlantButton->SetLabel("查看植物");
	mPlantButton->mButtonImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON;
	mPlantButton->mOverImage = nullptr;
	mPlantButton->mDownImage = nullptr;
	mPlantButton->mDisabledImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_DISABLED;
	mPlantButton->mOverOverlayImage = Sexy::IMAGE_SEEDCHOOSER_BUTTON_GLOW;
	mPlantButton->SetFont(AlmanacFont(18, Sexy::FONT_DWARVENTODCRAFT18YELLOW));
	mPlantButton->mColors[ButtonWidget::COLOR_LABEL] = Color::White;
	mPlantButton->mColors[ButtonWidget::COLOR_LABEL_HILITE] = Color::White;
	mPlantButton->Resize(130, 345, 156, 42);
	mPlantButton->mTextOffsetY = -1;
	mPlantButton->mParentWidget = this;

	mZombieButton = new GameButton(AlmanacDialog::ALMANAC_BUTTON_ZOMBIE);
	mZombieButton->SetLabel("查看僵尸");
	mZombieButton->Resize(487, 345, 210, 48);
	mZombieButton->mDrawStoneButton = true;
	mZombieButton->SetFont(AlmanacFont(18, Sexy::FONT_DWARVENTODCRAFT18GREENINSET));
	mZombieButton->mParentWidget = this;

	SetPage(ALMANAC_PAGE_INDEX);
	if (!mApp->mBoard || !mApp->mBoard->mPaused)
		mApp->mMusic->MakeSureMusicIsPlaying(MUSIC_TUNE_CHOOSE_YOUR_SEEDS);
}

AlmanacDialog::~AlmanacDialog()
{
	if (mCloseButton)	delete mCloseButton;
	if (mIndexButton)	delete mIndexButton;
	if (mPlantButton)	delete mPlantButton;
	if (mZombieButton)	delete mZombieButton;

	ClearPlantsAndZombies();
}

void AlmanacDialog::ClearPlantsAndZombies()
{
	if (mPlant)
	{
		mPlant->Die();
		delete mPlant;
		mPlant = nullptr;
	}
	if (mZombie)
	{
		mZombie->DieNoLoot();
		delete mZombie;
		mZombie = nullptr;
	}
	for (Zombie* &aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			aZombie->DieNoLoot();
			delete aZombie;
		}
		aZombie = nullptr;
	}
}

void AlmanacDialog::RemovedFromManager(WidgetManager* theWidgetManager)
{
	LawnDialog::RemovedFromManager(theWidgetManager);
	ClearPlantsAndZombies();
}

// GOTY @Patoke: 0x402C50
void AlmanacDialog::SetupPlant()
{
	mDescriptionScroll = mDescriptionMaxScroll = 0;
	mDraggingDescription = false;
	ClearPlantsAndZombies();

	float aPosX = ALMANAC_PLANT_POSITION_X;
	float aPosY = ALMANAC_PLANT_POSITION_Y;
	if (mSelectedSeed == SEED_TALLNUT)				aPosY += 18;
	else if (mSelectedSeed == SEED_COBCANNON)		aPosX -= 40;
	else if (mSelectedSeed == SEED_FLOWERPOT)		aPosY -= 20;
	else if (mSelectedSeed == SEED_INSTANT_COFFEE)	aPosY += 20;
	else if (mSelectedSeed == SEED_GRAVEBUSTER)		aPosY += 55;

	mPlant = new Plant();
	mPlant->mBoard = nullptr;
	mPlant->mIsOnBoard = false;
	mPlant->PlantInitialize(0, 0, mSelectedSeed, SEED_NONE);
	mPlant->mX = aPosX;
	mPlant->mY = aPosY;
}

// GOTY @Patoke: 0x402D90
void AlmanacDialog::SetupZombie()
{
	mDescriptionScroll = mDescriptionMaxScroll = 0;
	mDraggingDescription = false;
	ClearPlantsAndZombies();

	mZombie = new Zombie();
	mZombie->mBoard = nullptr;
	ZombieType previewType = mSelectedZombie == static_cast<ZombieType>(NUM_ZOMBIE_TYPES) || mSelectedZombie == ALMANAC_CART_ZOMBIE ? ZombieType::ZOMBIE_NORMAL : mSelectedZombie;
	int variantFlags = mSelectedZombie == ALMANAC_CART_ZOMBIE ? ZOMBIE_SPAWN_VARIANT_CART : 0;
	mZombie->ZombieInitialize(0, previewType, false, nullptr, Zombie::ZOMBIE_WAVE_UI, variantFlags);
	if (mSelectedZombie == static_cast<ZombieType>(NUM_ZOMBIE_TYPES)) mZombie->MakeErrorZombie();
	mZombie->mPosX = ALMANAC_ZOMBIE_POSITION_X;
	mZombie->mPosY = ALMANAC_ZOMBIE_POSITION_Y;
}

void AlmanacDialog::SetPage(AlmanacPage thePage)
{
	mDescriptionScroll = mDescriptionMaxScroll = 0;
	mDraggingDescription = false;
	mDraggingPlantList = mDraggingPlantScrollBar = false;
	mOpenPage = thePage;
	ClearPlantsAndZombies();

	if (mOpenPage == AlmanacPage::ALMANAC_PAGE_INDEX)
	{
		mPlant = new Plant();
		mPlant->mBoard = nullptr;
		mPlant->mIsOnBoard = false;
		mPlant->PlantInitialize(0, 0, SeedType::SEED_SUNFLOWER, SeedType::SEED_NONE);
		mPlant->mX = ALMANAC_INDEXPLANT_POSITION_X;
		mPlant->mY = ALMANAC_INDEXPLANT_POSITION_Y;

		mZombie = new Zombie();
		mZombie->mBoard = nullptr;
		mZombie->ZombieInitialize(0, ZombieType::ZOMBIE_NORMAL, false, nullptr, Zombie::ZOMBIE_WAVE_UI, 0);
		mZombie->mPosX = ALMANAC_INDEXZOMBIE_POSITION_X;
		mZombie->mPosY = ALMANAC_INDEXZOMBIE_POSITION_Y;

		mIndexButton->mBtnNoDraw = true;
		mPlantButton->mBtnNoDraw = false;
		mZombieButton->mBtnNoDraw = false;
	}
	else
	{
		if (mOpenPage == AlmanacPage::ALMANAC_PAGE_PLANTS)
			SetupPlant();
		else if (mOpenPage == AlmanacPage::ALMANAC_PAGE_ZOMBIES)
			SetupZombie();
		else return;

		mIndexButton->mBtnNoDraw = false;
		mPlantButton->mBtnNoDraw = true;
		mZombieButton->mBtnNoDraw = true;
	}
}

void AlmanacDialog::ShowPlant(SeedType theSeedType)
{
	mSelectedSeed = theSeedType;
	SetPage(ALMANAC_PAGE_PLANTS);
}

void AlmanacDialog::ShowZombie(ZombieType theZombieType)
{
	mSelectedZombie = theZombieType;
	SetPage(ALMANAC_PAGE_ZOMBIES);
}

void AlmanacDialog::Update()
{
	mCloseButton->Update();
	mIndexButton->Update();
	mPlantButton->Update();
	mZombieButton->Update();
	if (mPlant) mPlant->Update();
	if (mZombie) mZombie->Update();
	if (mOpenPage == AlmanacPage::ALMANAC_PAGE_ZOMBIES && mSelectedZombie == static_cast<ZombieType>(NUM_ZOMBIE_TYPES) && ++mErrorPreviewTicks >= 50)
	{
		mErrorPreviewTicks = 0;
		mErrorPreviewType = (mErrorPreviewType + 1) % ALMANAC_ERROR_ZOMBIE_INDEX;
		ClearPlantsAndZombies();
		mZombie = new Zombie(); mZombie->mBoard = nullptr;
		mZombie->ZombieInitialize(0, static_cast<ZombieType>(mErrorPreviewType), false, nullptr, Zombie::ZOMBIE_WAVE_UI, 0);
		mZombie->MakeErrorZombie();
		mZombie->mPosX = ALMANAC_ZOMBIE_POSITION_X; mZombie->mPosY = ALMANAC_ZOMBIE_POSITION_Y;
	}
	for (Zombie* aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			aZombie->Update();
		}
	}

	int aMouseX = mApp->mWidgetManager->mLastMouseX;
	int aMouseY = mApp->mWidgetManager->mLastMouseY;
	if (SeedHitTest(aMouseX, aMouseY) != SeedType::SEED_NONE || ZombieHitTest(aMouseX, aMouseY) != ZombieType::ZOMBIE_INVALID || 
		mCloseButton->IsMouseOver() || mIndexButton->IsMouseOver() || mPlantButton->IsMouseOver() || mZombieButton->IsMouseOver())
	{
		mApp->SetCursor(CURSOR_HAND);
	}
	else
	{
		mApp->SetCursor(CURSOR_POINTER);
	}

	mApp->mPoolEffect->PoolEffectUpdate();
	MarkDirty();
}

ZombieType AlmanacDialog::GetZombieType(int theIndex)
{
	if (theIndex < 0 || theIndex >= NUM_ALMANAC_ZOMBIES) return ZOMBIE_INVALID;
	if (theIndex == ALMANAC_ERROR_ZOMBIE_INDEX) return static_cast<ZombieType>(NUM_ZOMBIE_TYPES);
	if (theIndex == ALMANAC_CART_ZOMBIE_INDEX) return ALMANAC_CART_ZOMBIE;
	return static_cast<ZombieType>(theIndex);
}

void AlmanacDialog::DrawIndex(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_INDEXBACK, 0, 0);
	TodDrawString(g, "庭院图鉴", BOARD_WIDTH / 2, 60, AlmanacFont(24, Sexy::FONT_HOUSEOFTERROR28), Color(220, 220, 220), DrawStringJustification::DS_ALIGN_CENTER);
	
	if (mPlant)
	{
		Graphics aPlantGraphics = Graphics(*g);
		mPlant->BeginDraw(&aPlantGraphics);
		mPlant->Draw(&aPlantGraphics);
	}
	if (mZombie)
	{
		Graphics aZombieGraphics = Graphics(*g);
		mZombie->BeginDraw(&aZombieGraphics);
		mZombie->Draw(&aZombieGraphics);
	}
}

void AlmanacDialog::DrawPlants(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_PLANTBACK, 0, 0);
	TodDrawString(g, "植物图鉴", BOARD_WIDTH / 2, 48, AlmanacFont(20, Sexy::FONT_HOUSEOFTERROR20), Color(213, 159, 43), DrawStringJustification::DS_ALIGN_CENTER);

	UpdatePlantListScrollRange();
	Graphics cardGraphics(*g);
	cardGraphics.ClipRect(PlantListRect());
	SeedType aSeedMouseOn = SeedHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	for (SeedType aSeedType = SeedType::SEED_PEASHOOTER; aSeedType < NUM_ALMANAC_SEEDS; aSeedType = (SeedType)(aSeedType + 1))
	{
		int aPosX, aPosY;
		GetSeedPosition(aSeedType, aPosX, aPosY);
		if (mApp->HasSeedType(aSeedType))
		{
			if (aSeedType == SeedType::SEED_IMITATER)
			{
				if (aSeedType == aSeedMouseOn)
					g->DrawImage(Sexy::IMAGE_ALMANAC_IMITATER, aPosX, aPosY);
				g->DrawImage(Sexy::IMAGE_ALMANAC_IMITATER, aPosX, aPosY);
			}
			else
			{
				DrawSeedPacket(&cardGraphics, aPosX, aPosY, aSeedType, SeedType::SEED_NONE, 0, 255, true, false);
				if (aSeedType == aSeedMouseOn)
					cardGraphics.DrawImage(Sexy::IMAGE_SEEDPACKETFLASH, aPosX, aPosY);
			}
		}
	}

	if (mPlantListMaxScroll > 0)
	{
		g->SetColor(Color(95, 57, 28, 150));
		g->FillRect(444, 86, 8, 466);
		g->SetColor(Color(213, 159, 43));
		g->FillRect(PlantScrollThumb());
	}

	if (mSelectedSeed == SeedType::SEED_LILYPAD || mSelectedSeed == SeedType::SEED_TANGLEKELP || 
		mSelectedSeed == SeedType::SEED_CATTAIL || mSelectedSeed == SeedType::SEED_SEASHROOM)
	{
		bool aNight = mSelectedSeed == SeedType::SEED_SEASHROOM;
		g->DrawImage(aNight ? Sexy::IMAGE_ALMANAC_GROUNDNIGHTPOOL : Sexy::IMAGE_ALMANAC_GROUNDPOOL, 521, 107);

		if (mApp->Is3DAccelerated())
		{
			g->SetClipRect(475, 0, 397, 500);
			g->mTransY -= 145;
			mApp->mPoolEffect->PoolEffectDraw(g, aNight);
			g->mTransY += 145;
			g->ClearClipRect();
		}
	}
	else
	{
		g->DrawImage(
			Plant::IsNocturnal(mSelectedSeed) || mSelectedSeed == SeedType::SEED_GRAVEBUSTER || mSelectedSeed == SeedType::SEED_PLANTERN ? Sexy::IMAGE_ALMANAC_GROUNDNIGHT :
			mSelectedSeed == SeedType::SEED_FLOWERPOT ? Sexy::IMAGE_ALMANAC_GROUNDROOF : Sexy::IMAGE_ALMANAC_GROUNDDAY,
			521, 107
		);
	}
	
	if (mPlant)
	{
		Graphics aPlantGraphics = Graphics(*g);
		mPlant->BeginDraw(&aPlantGraphics);
		mPlant->Draw(&aPlantGraphics);
	}

	g->DrawImage(Sexy::IMAGE_ALMANAC_PLANTCARD, 459, 86);
	const PlantDefinition& aPlantDef = GetPlantDefinition(mSelectedSeed);
	const auto& text = kAlmanacPlants[mSelectedSeed];
	TodDrawString(g, text.name, 617, 288, AlmanacFont(18, Sexy::FONT_DWARVENTODCRAFT18YELLOW), Color::White, DS_ALIGN_CENTER);
	DrawDescription(g, text.description);

	if (mSelectedSeed != SeedType::SEED_IMITATER)
	{
		TodDrawStringWrapped(g, StrFormat("{KEYWORD}阳光：{STAT}%d", aPlantDef.mSeedCost),
			Rect(485, 520, 125, 40), AlmanacFont(14, Sexy::FONT_BRIANNETOD12), Color::White, DS_ALIGN_LEFT);
		const char* recharge = aPlantDef.mRefreshTime <= 1500 ? "短" : aPlantDef.mRefreshTime <= 3000 ? "长" : "很长";
		TodDrawStringWrapped(g, StrFormat("{KEYWORD}冷却：{STAT}%s", recharge),
			Rect(610, 520, 130, 40), AlmanacFont(14, Sexy::FONT_BRIANNETOD12), Color(40, 50, 90), DS_ALIGN_RIGHT);
	}
}

// GOTY @Patoke: 0x403DE0
void AlmanacDialog::DrawZombies(Graphics* g)
{
	g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEBACK, 0, 0);
	TodDrawString(g, "僵尸图鉴", BOARD_WIDTH / 2, 54, AlmanacFont(24, Sexy::FONT_DWARVENTODCRAFT24), Color(0, 196, 0), DS_ALIGN_CENTER);

	ZombieType aZombieMouseOn = ZombieHitTest(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY);
	for (int i = 0; i < NUM_ALMANAC_ZOMBIES; i++)
	{
		ZombieType aZombieType = GetZombieType(i);
		int aPosX, aPosY;
		GetZombiePosition(aZombieType, aPosX, aPosY);
		if (aZombieType != ZombieType::ZOMBIE_INVALID)
		{
			if (!ZombieIsShown(aZombieType))
				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEBLANK, aPosX, aPosY);
			else
			{
				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW, aPosX, aPosY);
				if (aZombieType == aZombieMouseOn)
				{
					g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
					g->SetColor(Color(255, 255, 255, 48));
					g->SetColorizeImages(true);
					g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW, aPosX, aPosY);
					g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
					g->SetColorizeImages(false);
				}

				ZombieType aZombieTypeToDraw = aZombieType == static_cast<ZombieType>(NUM_ZOMBIE_TYPES) || aZombieType == ALMANAC_CART_ZOMBIE ? ZombieType::ZOMBIE_NORMAL : aZombieType;
				Graphics aZombieGraphics = Graphics(*g);
				aZombieGraphics.SetClipRect(aPosX + 2, aPosY + 2, 72, 72);
				aZombieGraphics.Translate(aPosX + 1, aPosY - 6);
				aZombieGraphics.mScaleX = 0.5f;
				aZombieGraphics.mScaleY = 0.5f;
				switch (aZombieType)
				{
				case ZombieType::ZOMBIE_POLEVAULTER:
					aZombieGraphics.TranslateF(2, -3);
					aZombieTypeToDraw = ZombieType::ZOMBIE_CACHED_POLEVAULTER_WITH_POLE;		break;
				case ZombieType::ZOMBIE_FLAG:			aZombieGraphics.TranslateF(2, 10);	break;
				case ZombieType::ZOMBIE_TRAFFIC_CONE:	aZombieGraphics.TranslateF(0, 12);	break;
				case ZombieType::ZOMBIE_PAIL:			aZombieGraphics.TranslateF(0, 9);		break;
				case ZombieType::ZOMBIE_FOOTBALL:		aZombieGraphics.TranslateF(-15, -1);	break;
				case ZombieType::ZOMBIE_ZAMBONI:		aZombieGraphics.TranslateF(0, 3);		break;
				case ZombieType::ZOMBIE_DOLPHIN_RIDER:	aZombieGraphics.TranslateF(-2, -10);	break;
				case ZombieType::ZOMBIE_POGO:			aZombieGraphics.TranslateF(0, -3);	break;
				case ZombieType::ZOMBIE_GARGANTUAR:		aZombieGraphics.TranslateF(15, 17);	break;
				case ZombieType::ZOMBIE_IMP:			aZombieGraphics.TranslateF(-8, -7);	break;
				case ZombieType::ZOMBIE_BUNGEE:			aZombieGraphics.TranslateF(-4, 3);	break;
				case ZombieType::ZOMBIE_DANCER:			aZombieGraphics.TranslateF(0, 15);	break;
				case ZombieType::ZOMBIE_BACKUP_DANCER:	aZombieGraphics.TranslateF(-4, 20);	break;
				case ZombieType::ZOMBIE_SNORKEL:		aZombieGraphics.TranslateF(-10, 0);	break;
				case ZombieType::ZOMBIE_YETI:			aZombieGraphics.TranslateF(0, 4);		break;
				case ZombieType::ZOMBIE_CATAPULT:		aZombieGraphics.TranslateF(-24, -1);	break;
				case ZombieType::ZOMBIE_BOBSLED:		aZombieGraphics.TranslateF(0, -8);	break;
				case ZombieType::ZOMBIE_LADDER:			aZombieGraphics.TranslateF(0, -3);	break;
				default: break;
				}
				if (ZombieHasSilhouette(aZombieType))
				{
					aZombieGraphics.SetColor(Color(0, 0, 0, 40));
					aZombieGraphics.SetColorizeImages(true);
				}
				mApp->mReanimatorCache->DrawCachedZombie(&aZombieGraphics, 0, 0, aZombieTypeToDraw, aZombieType == ALMANAC_CART_ZOMBIE);
				aZombieGraphics.SetColorizeImages(false);

				g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW2, aPosX, aPosY);
				if (aZombieType == aZombieMouseOn)
				{
					g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
					g->SetColor(Color(255, 255, 255, 48));
					g->SetColorizeImages(true);
					g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIEWINDOW2, aPosX, aPosY);
					g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
					g->SetColorizeImages(false);
				}
			}
		}
	}

	g->DrawImage(mZombie->mZombieType == ZombieType::ZOMBIE_ZAMBONI || mZombie->mZombieType == ZombieType::ZOMBIE_BOBSLED ?
		Sexy::IMAGE_ALMANAC_GROUNDICE : Sexy::IMAGE_ALMANAC_GROUNDDAY, 518, 110);
	if (mZombie &&
		mZombie->mZombieType >= ZombieType::ZOMBIE_INVALID && mZombie->mZombieType < NUM_ZOMBIE_TYPES &&
		!ZombieHasSilhouette(mZombie->mZombieType))
	{
		Graphics aZombieGraphics = Graphics(*g);
		mZombie->BeginDraw(&aZombieGraphics);
		aZombieGraphics.SetClipRect(-42, -51, 197, 187);
		switch (mZombie->mZombieType)
		{
		case ZombieType::ZOMBIE_ZAMBONI:		aZombieGraphics.TranslateF(-30, 5);		break;
		case ZombieType::ZOMBIE_GARGANTUAR:		aZombieGraphics.TranslateF(0, 40);		break;
		case ZombieType::ZOMBIE_FOOTBALL:		aZombieGraphics.TranslateF(-10, 0);		break;
		case ZombieType::ZOMBIE_BALLOON:		aZombieGraphics.TranslateF(0, -20);		break;
		case ZombieType::ZOMBIE_BUNGEE:			aZombieGraphics.TranslateF(15, 0);		break;
		case ZombieType::ZOMBIE_CATAPULT:		aZombieGraphics.TranslateF(-10, 0);		break;
		case ZombieType::ZOMBIE_BOSS:			aZombieGraphics.TranslateF(-540, -175);	break;
		default: break;
		}
		if (mZombie->mZombieType != ZombieType::ZOMBIE_BUNGEE && mZombie->mZombieType != ZombieType::ZOMBIE_BOSS &&
			mZombie->mZombieType != ZombieType::ZOMBIE_ZAMBONI && mZombie->mZombieType != ZombieType::ZOMBIE_CATAPULT)
			mZombie->DrawShadow(&aZombieGraphics);
		mZombie->Draw(&aZombieGraphics);
	}
	g->DrawImage(Sexy::IMAGE_ALMANAC_ZOMBIECARD, 455, 78);

	int textIndex = mSelectedZombie == ALMANAC_CART_ZOMBIE ? ALMANAC_CART_ZOMBIE_INDEX :
		(mSelectedZombie == static_cast<ZombieType>(NUM_ZOMBIE_TYPES) ? ALMANAC_ERROR_ZOMBIE_INDEX :
		(mSelectedZombie >= ZOMBIE_NORMAL && mSelectedZombie <= ZOMBIE_BOSS ? static_cast<int>(mSelectedZombie) : 0));
	const auto& text = kAlmanacZombies[textIndex];
	TodDrawString(g, ZombieHasSilhouette(mSelectedZombie) ? "？？？" : text.name, 613, 362,
		AlmanacFont(18, Sexy::FONT_DWARVENTODCRAFT18GREENINSET), Color(190, 255, 235), DS_ALIGN_CENTER);
	if (ZombieHasDescription(mSelectedZombie))
		DrawDescription(g, text.description);
	else
		TodDrawStringWrapped(g, "尚未遇见这种僵尸", DescriptionRect(), AlmanacFont(14, Sexy::FONT_BRIANNETOD12),
			Color(40, 50, 90), DS_ALIGN_CENTER_VERTICAL_MIDDLE);
}

Rect AlmanacDialog::PlantListRect() const
{
	return Rect(20, 86, 422, 466);
}

void AlmanacDialog::UpdatePlantListScrollRange()
{
	int bottom = 552;
	for (int seed = SEED_PEASHOOTER; seed < NUM_ALMANAC_SEEDS; ++seed)
	{
		if (seed == SEED_IMITATER || !mApp->HasSeedType(static_cast<SeedType>(seed))) continue;
		int x, y;
		GetSeedPosition(static_cast<SeedType>(seed), x, y);
		bottom = std::max(bottom, y + mPlantListScroll + SEED_PACKET_HEIGHT);
	}
	mPlantListMaxScroll = std::max(0, bottom - 552);
	mPlantListScroll = std::clamp(mPlantListScroll, 0, mPlantListMaxScroll);
}

Rect AlmanacDialog::PlantScrollThumb() const
{
	const int height = std::max(30, 466 * 466 / (466 + mPlantListMaxScroll));
	const int y = 86 + (mPlantListMaxScroll ? mPlantListScroll * (466 - height) / mPlantListMaxScroll : 0);
	return Rect(444, y, 8, height);
}

void AlmanacDialog::DragPlantScrollBar(int y)
{
	const int travel = 466 - PlantScrollThumb().mHeight;
	mPlantListScroll = travel > 0 ? std::clamp((y - 86 - mPlantScrollGrabOffset) * mPlantListMaxScroll / travel, 0, mPlantListMaxScroll) : 0;
	MarkDirty();
}

Rect AlmanacDialog::DescriptionRect() const
{
	return mOpenPage == ALMANAC_PAGE_PLANTS ? Rect(485, 309, 258, 200) : Rect(484, 377, 258, 170);
}

void AlmanacDialog::DrawDescription(Graphics* g, const std::string& text)
{
	// Almanac visibility changes must not mutate formatting used elsewhere.
	std::array<TodStringListFormat, 12> formats;
	std::copy(std::begin(gLawnStringFormats), std::end(gLawnStringFormats), formats.begin());
	for (auto& format : formats)
	{
		if (TestBit(format.mFormatFlags, TOD_FORMAT_HIDE_UNTIL_MAGNETSHROOM))
		{
			const bool visible = mApp->HasSeedType(SEED_MAGNETSHROOM);
			format.mNewColor.mAlpha = visible ? 255 : 0;
			format.mLineSpacingOffset = visible ? 0 : -17;
		}
	}
	struct RestoreFormats
	{
		TodStringListFormat* previous = gTodStringFormats;
		int count = gTodStringFormatCount;
		~RestoreFormats() { gTodStringFormats = previous; gTodStringFormatCount = count; }
	} restore;
	gTodStringFormats = formats.data();
	gTodStringFormatCount = static_cast<int>(formats.size());

	const Rect viewport = DescriptionRect();
	Rect layout = viewport;
	layout.mWidth -= 8;
	_Font* font = AlmanacFont(14, Sexy::FONT_BRIANNETOD12);
	const int height = TodDrawStringWrappedHelper(g, text, layout, font, Color(40, 50, 90), DS_ALIGN_LEFT, false);
	mDescriptionMaxScroll = std::max(0, height - viewport.mHeight);
	mDescriptionScroll = std::clamp(mDescriptionScroll, 0, mDescriptionMaxScroll);
	Graphics clipped(*g);
	clipped.ClipRect(viewport);
	layout.mY -= mDescriptionScroll;
	TodDrawStringWrappedHelper(&clipped, text, layout, font, Color(40, 50, 90), DS_ALIGN_LEFT, true);
	if (mDescriptionMaxScroll > 0)
	{
		clipped.SetColor(Color(143, 67, 27, 80));
		clipped.FillRect(viewport.mX + viewport.mWidth - 4, viewport.mY, 3, viewport.mHeight);
		const int thumb = std::max(18, viewport.mHeight * viewport.mHeight / height);
		const int y = viewport.mY + mDescriptionScroll * (viewport.mHeight - thumb) / mDescriptionMaxScroll;
		clipped.SetColor(Color(143, 67, 27, 190));
		clipped.FillRect(viewport.mX + viewport.mWidth - 4, y, 3, thumb);
	}
}

void AlmanacDialog::MouseWheel(int theDelta)
{
	if (mOpenPage == ALMANAC_PAGE_INDEX) return;
	if (mOpenPage == ALMANAC_PAGE_PLANTS &&
		Rect(20, 86, 432, 466).Contains(mApp->mWidgetManager->mLastMouseX, mApp->mWidgetManager->mLastMouseY))
	{
		mPlantListScroll = std::clamp(mPlantListScroll - theDelta * 39, 0, mPlantListMaxScroll);
		MarkDirty();
		return;
	}
	mDescriptionScroll = std::clamp(mDescriptionScroll - theDelta * 42, 0, mDescriptionMaxScroll);
	MarkDirty();
}

void AlmanacDialog::MouseDrag(int x, int y)
{
	(void)x;
	if (mDraggingPlantScrollBar)
	{
		DragPlantScrollBar(y);
		return;
	}
	if (mDraggingPlantList)
	{
		mPlantListScroll = std::clamp(mPlantListScroll + mPlantListDragY - y, 0, mPlantListMaxScroll);
		mPlantListDragY = y;
		MarkDirty();
		return;
	}
	if (!mDraggingDescription) return;
	mDescriptionScroll = std::clamp(mDescriptionScroll + mDescriptionDragY - y, 0, mDescriptionMaxScroll);
	mDescriptionDragY = y;
	MarkDirty();
}

void AlmanacDialog::Draw(Graphics* g)
{
	g->SetLinearBlend(true);
	switch (mOpenPage)
	{
	case AlmanacPage::ALMANAC_PAGE_INDEX:	DrawIndex(g);	break;
	case AlmanacPage::ALMANAC_PAGE_PLANTS:	DrawPlants(g);	break;
	case AlmanacPage::ALMANAC_PAGE_ZOMBIES:	DrawZombies(g);	break;
	}

	for (Zombie* aZombie : mZombiePerfTest)
	{
		if (aZombie)
		{
			Graphics aTestGraphics = Graphics(*g);
			aZombie->Draw(&aTestGraphics);
		}
	}

	mCloseButton->Draw(g);
	mIndexButton->Draw(g);
	mPlantButton->Draw(g);
	mZombieButton->Draw(g);
}

void AlmanacDialog::GetSeedPosition(SeedType theSeedType, int& x, int& y)
{
	if (theSeedType == SeedType::SEED_IMITATER)
		x = 20, y = 23;
	else if (theSeedType >= SEED_IRONPEASHOOTER && theSeedType <= SEED_SHIELDWALLNUT)
		x = 26 + (theSeedType - SEED_IRONPEASHOOTER) * 52, y = 560 - mPlantListScroll;
	else
	{
		x = theSeedType % 8 * 52 + 26;
		y = theSeedType / 8 * 78 + 92 - mPlantListScroll;
	}
}

SeedType AlmanacDialog::SeedHitTest(int x, int y)
{
	if (mMouseVisible && mOpenPage == AlmanacPage::ALMANAC_PAGE_PLANTS)
	{
		for (SeedType aSeedType = SeedType::SEED_PEASHOOTER; aSeedType < NUM_ALMANAC_SEEDS; aSeedType = (SeedType)(aSeedType + 1))
		{
			if (mApp->HasSeedType(aSeedType))
			{
				int aSeedX, aSeedY;
				GetSeedPosition(aSeedType, aSeedX, aSeedY);
				Rect aSeedRect = aSeedType == SeedType::SEED_IMITATER ? Rect(aSeedX, aSeedY, 34, 46) : Rect(aSeedX, aSeedY, SEED_PACKET_WIDTH, SEED_PACKET_HEIGHT);
				if (aSeedRect.Contains(x, y) &&
					(aSeedType == SEED_IMITATER || PlantListRect().Contains(x, y))) return aSeedType;
			}
		}
	}
	return SeedType::SEED_NONE;
}

bool AlmanacDialog::ZombieHasSilhouette(ZombieType theZombieType)
{
	if (theZombieType == static_cast<ZombieType>(NUM_ZOMBIE_TYPES) || theZombieType == ALMANAC_CART_ZOMBIE) return false;
	// 除雪人僵尸以外的其他僵尸，或者雪人僵尸已经可以刷出（已经到达或完成冒险模式二周目 4-10 关卡），则不会显示为剪影
	if (theZombieType != ZombieType::ZOMBIE_YETI || mApp->CanSpawnYetis())
		return false;

	// 排除上述情况后，若已完成雪人僵尸出现的关卡（冒险模式一周目 4-10 关卡），则雪人僵尸显示为剪影
	return mApp->HasFinishedAdventure() || mApp->mPlayerInfo->GetLevel() > GetZombieDefinition(ZombieType::ZOMBIE_YETI).mStartingLevel;
}

// GOTY @Patoke: 0x404C50
bool AlmanacDialog::ZombieIsShown(ZombieType theZombieType)
{
	if (theZombieType == static_cast<ZombieType>(NUM_ZOMBIE_TYPES) || theZombieType == ALMANAC_CART_ZOMBIE) return true;
	// 试玩模式下，仅展示潜水僵尸及其之前出现的僵尸
	if (mApp->IsTrialStageLocked() && theZombieType > ZombieType::ZOMBIE_SNORKEL)
		return false;

	// 对于雪人僵尸，要求其可以在刷怪中出现（已经到达或完成冒险模式二周目 4-10 关卡），
	// 或已得知其存在但未解锁其形象（已经完成冒险模式一周目 4-10 关卡，但未到达二周目 4-10 关卡）
	if (theZombieType == ZombieType::ZOMBIE_YETI)
		return mApp->CanSpawnYetis() || ZombieHasSilhouette(ZombieType::ZOMBIE_YETI);

	// 对于冒险模式中出现的僵尸
	if (theZombieType <= ZombieType::ZOMBIE_BOSS)
	{
		// 冒险模式一周目完成后，图鉴展示所有僵尸
		if (mApp->HasFinishedAdventure())
			return true;

		int aLevel = mApp->mPlayerInfo->GetLevel();
		int aStart = GetZombieDefinition(theZombieType).mStartingLevel;
		// 要求已经达到僵尸首次出现的关卡
		// 对于不能通过自然刷怪出现的僵尸（小鬼僵尸、雪橇僵尸小队、伴舞僵尸），额外要求已通过其首次出现的关卡或已击败过该僵尸
		return aStart <= aLevel && (aStart != aLevel || !Board::IsZombieTypeSpawnedOnly(theZombieType) || gZombieDefeated[theZombieType]);
	}

	return false;
}

// GOTY @Patoke: 0x404D50
bool AlmanacDialog::ZombieHasDescription(ZombieType theZombieType)
{
	if (theZombieType == static_cast<ZombieType>(NUM_ZOMBIE_TYPES) || theZombieType == ALMANAC_CART_ZOMBIE) return true;
	int aLevel = mApp->mPlayerInfo->GetLevel();
	int aStart = GetZombieDefinition(theZombieType).mStartingLevel;

	// 对于雪人僵尸
	if (theZombieType == ZombieType::ZOMBIE_YETI)
	{
		// 当雪人僵尸不可在刷怪中出现时（冒险模式二周目 4-10 关卡之前），不显示僵尸描述
		if (!mApp->CanSpawnYetis())
			return false;
		// 从第三周目开始，总是显示雪人僵尸的描述
		if (mApp->mPlayerInfo->mFinishedAdventure >= 2)
			return true;
	}
	// 对于雪人僵尸外的其他僵尸，当冒险模式已完成时，总是显示僵尸的描述
	else if (mApp->HasFinishedAdventure())
		return true;

	// 雪人僵尸在二周目 4-10 关卡至三周目之间，或其他僵尸在冒险模式一周目中的情况，
	// 要求已经达到僵尸首次出现的关卡，且已通过其首次出现的关卡或已击败过该僵尸
	return aStart <= aLevel && (aStart != aLevel || gZombieDefeated[theZombieType]);
}

void AlmanacDialog::GetZombiePosition(ZombieType theZombieType, int& x, int& y)
{
	if (theZombieType == ALMANAC_CART_ZOMBIE) { x = 362; y = 486; return; }
	if (theZombieType == static_cast<ZombieType>(NUM_ZOMBIE_TYPES)) { x = 277; y = 486; return; }
	if (theZombieType == ZombieType::ZOMBIE_BOSS)
		x = 192, y = 486;
	else
	{
		x = theZombieType % 5 * 85 + 22;
		y = theZombieType / 5 * 80 + 86;
	}
}

// GOTY @Patoke: 0x404DD0
ZombieType AlmanacDialog::ZombieHitTest(int x, int y)
{
	if (mMouseVisible && mOpenPage == AlmanacPage::ALMANAC_PAGE_ZOMBIES)
	{
		for (int i = 0; i < NUM_ALMANAC_ZOMBIES; i++)
		{
			ZombieType aZombieType = GetZombieType(i);
			// @Patoke: added IsShown check
			if (aZombieType != ZombieType::ZOMBIE_INVALID && ZombieIsShown(aZombieType))
			{
				int aZombieX, aZombieY;
				GetZombiePosition(aZombieType, aZombieX, aZombieY);
				if (Rect(aZombieX, aZombieY, 76, 76).Contains(x, y))
					return aZombieType;
			}
		}
	}
	return ZombieType::ZOMBIE_INVALID;
}

void AlmanacDialog::MouseUp(int x, int y, int theClickCount)
{
	(void)x;(void)y;(void)theClickCount;
	if (mDraggingDescription || mDraggingPlantList || mDraggingPlantScrollBar)
	{
		mDraggingDescription = mDraggingPlantList = mDraggingPlantScrollBar = false;
		return;
	}
	if (mPlantButton->IsMouseOver())		SetPage(ALMANAC_PAGE_PLANTS);
	else if (mZombieButton->IsMouseOver())	SetPage(ALMANAC_PAGE_ZOMBIES);
	else if (mCloseButton->IsMouseOver())	mApp->KillAlmanacDialog();
	else if (mIndexButton->IsMouseOver())	SetPage(ALMANAC_PAGE_INDEX);
}

// GOTY @Patoke: 0x404F10
void AlmanacDialog::MouseDown(int x, int y, int theClickCount)
{
	(void)theClickCount;
	if (mOpenPage == ALMANAC_PAGE_PLANTS && Rect(440, 86, 16, 466).Contains(x, y) && mPlantListMaxScroll > 0)
	{
		const Rect thumb = PlantScrollThumb();
		mPlantScrollGrabOffset = thumb.Contains(x, y) ? y - thumb.mY : thumb.mHeight / 2;
		mDraggingPlantScrollBar = true;
		DragPlantScrollBar(y);
		return;
	}
	if (mOpenPage != ALMANAC_PAGE_INDEX && DescriptionRect().Contains(x, y))
	{
		mDraggingDescription = true;
		mDescriptionDragY = y;
		return;
	}
	if (mPlantButton->IsMouseOver() || mCloseButton->IsMouseOver() || mIndexButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_TAP);
	if (mZombieButton->IsMouseOver())
		mApp->PlaySample(Sexy::SOUND_GRAVEBUTTON);

	SeedType aSeedType = SeedHitTest(x, y);
	if (aSeedType != SeedType::SEED_NONE && aSeedType != mSelectedSeed)
	{
		mSelectedSeed = aSeedType;
		SetupPlant();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
	ZombieType aZombieType = ZombieHitTest(x, y);
	if (aZombieType != ZombieType::ZOMBIE_INVALID && aZombieType != mSelectedZombie)
	{
		mSelectedZombie = aZombieType;
		SetupZombie();
		mApp->PlaySample(Sexy::SOUND_TAP);
	}
	if (mOpenPage == ALMANAC_PAGE_PLANTS && PlantListRect().Contains(x, y))
	{
		mDraggingPlantList = true;
		mPlantListDragY = y;
	}

}

void AlmanacDialog::KeyDown(KeyCode theKey)
{
	if (theKey == KeyCode::KEYCODE_ESCAPE)
	{
		if (mOpenPage == AlmanacPage::ALMANAC_PAGE_INDEX)
			mApp->KillAlmanacDialog();
		else
			SetPage(AlmanacPage::ALMANAC_PAGE_INDEX);
		return;
	}

	LawnDialog::KeyDown(theKey);
}

void AlmanacInitForPlayer()
{
	for (int i = 0; i < ZombieType::NUM_ZOMBIE_TYPES; i++)
		gZombieDefeated[i] = false;
}

void AlmanacPlayerDefeatedZombie(ZombieType theZombieType)
{
	gZombieDefeated[theZombieType] = true;
}
