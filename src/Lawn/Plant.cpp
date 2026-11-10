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

#include "Coin.h"
#include "Plant.h"
#include "Board.h"
#include "Zombie.h"
#include "Cutscene.h"
#include "GridItem.h"
#include "ZenGarden.h"
#include "Challenge.h"
#include "Projectile.h"
#include "SeedPacket.h"
#include "../LawnApp.h"
#include "CursorObject.h"
#include "../GameConstants.h"
#include "System/PlayerInfo.h"
#include "System/ReanimationLawn.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/Attachment.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../SexyAppFramework/widget/WidgetManager.h"
#include "../Sexy.TodLib/TodParticle.h"
#include "../Sexy.TodLib/EffectSystem.h"
#include "../Sexy.TodLib/TodStringFile.h"
#include "Widget/AchievementsScreen.h"
#include <cmath>
#include "graphics/Font.h"
#include "graphics/SysFont.h"

constinit const PlantDefinition gPlantDefs[SeedType::NUM_SEED_TYPES] = {
    { .mSeedType = SeedType::SEED_PEASHOOTER,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_PEASHOOTER,    .mPacketIndex = 0,  .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "PEASHOOTER" },
    { .mSeedType = SeedType::SEED_SUNFLOWER,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SUNFLOWER,     .mPacketIndex = 1,  .mSeedCost = 50,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 2500, .mPlantName = "SUNFLOWER" },
    { .mSeedType = SeedType::SEED_CHERRYBOMB,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_CHERRYBOMB,    .mPacketIndex = 3,  .mSeedCost = 150, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "CHERRY_BOMB" },
    { .mSeedType = SeedType::SEED_WALLNUT,           .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_WALLNUT,       .mPacketIndex = 2,  .mSeedCost = 50,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "WALL_NUT" },
    { .mSeedType = SeedType::SEED_POTATOMINE,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_POTATOMINE,    .mPacketIndex = 37, .mSeedCost = 25,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "POTATO_MINE" },
    { .mSeedType = SeedType::SEED_SNOWPEA,           .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SNOWPEA,       .mPacketIndex = 4,  .mSeedCost = 175, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "SNOW_PEA" },
    { .mSeedType = SeedType::SEED_CHOMPER,           .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_CHOMPER,       .mPacketIndex = 31, .mSeedCost = 150, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "CHOMPER" },
    { .mSeedType = SeedType::SEED_REPEATER,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_REPEATER,      .mPacketIndex = 5,  .mSeedCost = 200, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "REPEATER" },
    { .mSeedType = SeedType::SEED_PUFFSHROOM,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_PUFFSHROOM,    .mPacketIndex = 6,  .mSeedCost = 0,   .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "PUFF_SHROOM" },
    { .mSeedType = SeedType::SEED_SUNSHROOM,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SUNSHROOM,     .mPacketIndex = 7,  .mSeedCost = 25,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 2500, .mPlantName = "SUN_SHROOM" },
    { .mSeedType = SeedType::SEED_FUMESHROOM,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_FUMESHROOM,    .mPacketIndex = 9,  .mSeedCost = 75,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "FUME_SHROOM" },
    { .mSeedType = SeedType::SEED_GRAVEBUSTER,       .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_GRAVE_BUSTER,  .mPacketIndex = 40, .mSeedCost = 75,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "GRAVE_BUSTER" },
    { .mSeedType = SeedType::SEED_HYPNOSHROOM,       .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_HYPNOSHROOM,   .mPacketIndex = 10, .mSeedCost = 75,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "HYPNO_SHROOM" },
    { .mSeedType = SeedType::SEED_SCAREDYSHROOM,     .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SCRAREYSHROOM, .mPacketIndex = 33, .mSeedCost = 25,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "SCAREDY_SHROOM" },
    { .mSeedType = SeedType::SEED_ICESHROOM,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_ICESHROOM,     .mPacketIndex = 36, .mSeedCost = 75,  .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "ICE_SHROOM" },
    { .mSeedType = SeedType::SEED_DOOMSHROOM,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_DOOMSHROOM,    .mPacketIndex = 20, .mSeedCost = 125, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "DOOM_SHROOM" },
    { .mSeedType = SeedType::SEED_LILYPAD,           .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_LILYPAD,       .mPacketIndex = 19, .mSeedCost = 25,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "LILY_PAD" },
    { .mSeedType = SeedType::SEED_SQUASH,            .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SQUASH,        .mPacketIndex = 21, .mSeedCost = 50,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "SQUASH" },
    { .mSeedType = SeedType::SEED_THREEPEATER,       .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_THREEPEATER,   .mPacketIndex = 12, .mSeedCost = 325, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "THREEPEATER" },
    { .mSeedType = SeedType::SEED_TANGLEKELP,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_TANGLEKELP,    .mPacketIndex = 17, .mSeedCost = 25,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "TANGLE_KELP" },
    { .mSeedType = SeedType::SEED_JALAPENO,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_JALAPENO,      .mPacketIndex = 11, .mSeedCost = 125, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "JALAPENO" },
    { .mSeedType = SeedType::SEED_SPIKEWEED,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SPIKEWEED,     .mPacketIndex = 22, .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "SPIKEWEED" },
    { .mSeedType = SeedType::SEED_TORCHWOOD,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_TORCHWOOD,     .mPacketIndex = 29, .mSeedCost = 175, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "TORCHWOOD" },
    { .mSeedType = SeedType::SEED_TALLNUT,           .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_TALLNUT,       .mPacketIndex = 28, .mSeedCost = 125, .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "TALL_NUT" },
    { .mSeedType = SeedType::SEED_SEASHROOM,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SEASHROOM,     .mPacketIndex = 39, .mSeedCost = 0,   .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "SEA_SHROOM" },
    { .mSeedType = SeedType::SEED_PLANTERN,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_PLANTERN,      .mPacketIndex = 38, .mSeedCost = 25,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 2500, .mPlantName = "PLANTERN" },
    { .mSeedType = SeedType::SEED_CACTUS,            .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_CACTUS,        .mPacketIndex = 15, .mSeedCost = 125, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "CACTUS" },
    { .mSeedType = SeedType::SEED_BLOVER,            .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_BLOVER,        .mPacketIndex = 18, .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "BLOVER" },
    { .mSeedType = SeedType::SEED_SPLITPEA,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SPLITPEA,      .mPacketIndex = 32, .mSeedCost = 125, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "SPLIT_PEA" },
    { .mSeedType = SeedType::SEED_STARFRUIT,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_STARFRUIT,     .mPacketIndex = 30, .mSeedCost = 125, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "STARFRUIT" },
    { .mSeedType = SeedType::SEED_PUMPKINSHELL,      .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_PUMPKIN,       .mPacketIndex = 25, .mSeedCost = 125, .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "PUMPKIN" },
    { .mSeedType = SeedType::SEED_MAGNETSHROOM,      .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_MAGNETSHROOM,  .mPacketIndex = 35, .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "MAGNET_SHROOM" },
    { .mSeedType = SeedType::SEED_CABBAGEPULT,       .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_CABBAGEPULT,   .mPacketIndex = 13, .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 300,  .mPlantName = "CABBAGE_PULT" },
    { .mSeedType = SeedType::SEED_FLOWERPOT,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_FLOWER_POT,    .mPacketIndex = 33, .mSeedCost = 25,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "FLOWER_POT" },
    { .mSeedType = SeedType::SEED_KERNELPULT,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_KERNELPULT,    .mPacketIndex = 13, .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 300,  .mPlantName = "KERNEL_PULT" },
    { .mSeedType = SeedType::SEED_INSTANT_COFFEE,    .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_COFFEEBEAN,    .mPacketIndex = 33, .mSeedCost = 75,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "COFFEE_BEAN" },
    { .mSeedType = SeedType::SEED_GARLIC,            .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_GARLIC,        .mPacketIndex = 8,  .mSeedCost = 50,  .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "GARLIC" },
    { .mSeedType = SeedType::SEED_UMBRELLA,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_UMBRELLALEAF,  .mPacketIndex = 23, .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "UMBRELLA_LEAF" },
    { .mSeedType = SeedType::SEED_MARIGOLD,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_MARIGOLD,      .mPacketIndex = 24, .mSeedCost = 50,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 2500, .mPlantName = "MARIGOLD" },
    { .mSeedType = SeedType::SEED_MELONPULT,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_MELONPULT,     .mPacketIndex = 14, .mSeedCost = 300, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 300,  .mPlantName = "MELON_PULT" },
    { .mSeedType = SeedType::SEED_GATLINGPEA,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_GATLINGPEA,    .mPacketIndex = 5,  .mSeedCost = 250, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "GATLING_PEA" },
    { .mSeedType = SeedType::SEED_TWINSUNFLOWER,     .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_TWIN_SUNFLOWER,.mPacketIndex = 1,  .mSeedCost = 150, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 2500, .mPlantName = "TWIN_SUNFLOWER" },
    { .mSeedType = SeedType::SEED_GLOOMSHROOM,       .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_GLOOMSHROOM,   .mPacketIndex = 27, .mSeedCost = 150, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 200,  .mPlantName = "GLOOM_SHROOM" },
    { .mSeedType = SeedType::SEED_CATTAIL,           .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_CATTAIL,       .mPacketIndex = 27, .mSeedCost = 225, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "CATTAIL" },
    { .mSeedType = SeedType::SEED_WINTERMELON,       .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_WINTER_MELON,  .mPacketIndex = 27, .mSeedCost = 200, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 300,  .mPlantName = "WINTER_MELON" },
    { .mSeedType = SeedType::SEED_GOLD_MAGNET,       .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_GOLD_MAGNET,   .mPacketIndex = 27, .mSeedCost = 50,  .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "GOLD_MAGNET" },
    { .mSeedType = SeedType::SEED_SPIKEROCK,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_SPIKEROCK,     .mPacketIndex = 27, .mSeedCost = 125, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "SPIKEROCK" },
    { .mSeedType = SeedType::SEED_COBCANNON,         .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_COBCANNON,     .mPacketIndex = 16, .mSeedCost = 500, .mRefreshTime = 5000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 600,  .mPlantName = "COB_CANNON" },
    { .mSeedType = SeedType::SEED_IMITATER,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_IMITATER,      .mPacketIndex = 33, .mSeedCost = 0,   .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "IMITATER" },
    { .mSeedType = SeedType::SEED_IRONPEASHOOTER,    .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_PEASHOOTER,    .mPacketIndex = 49, .mSeedCost = 150, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "IRON_PEA_SHOOTER" },
    { .mSeedType = SeedType::SEED_DOOMPEA,           .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_PEASHOOTER,    .mPacketIndex = 50, .mSeedCost = 300, .mRefreshTime = 1500,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "DOOM_PEA" },
    { .mSeedType = SeedType::SEED_SHIELDWALLNUT,   .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_WALLNUT,       .mPacketIndex = 51, .mSeedCost = 50,  .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "SHIELD_WALLNUT" },
    { .mSeedType = SeedType::SEED_EXPLODE_O_NUT,     .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_WALLNUT,       .mPacketIndex = 2,  .mSeedCost = 0,   .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "EXPLODE_O_NUT" },
    { .mSeedType = SeedType::SEED_GIANT_WALLNUT,     .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_WALLNUT,       .mPacketIndex = 2,  .mSeedCost = 0,   .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "GIANT_WALLNUT" },
    { .mSeedType = SeedType::SEED_SPROUT,            .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_ZENGARDEN_SPROUT, .mPacketIndex = 33, .mSeedCost = 0,   .mRefreshTime = 3000,   .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "SPROUT" },
    { .mSeedType = SeedType::SEED_LEFTPEATER,        .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_REPEATER,      .mPacketIndex = 5,  .mSeedCost = 200, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_SHOOTER, .mLaunchRate = 150,  .mPlantName = "REPEATER" },
    { .mSeedType = SeedType::SEED_GIFT_BOX,          .mPlantImage = nullptr, .mReanimationType = ReanimationType::REANIM_NONE,         .mPacketIndex = 0,  .mSeedCost = 100, .mRefreshTime = 750,    .mSubClass = PlantSubClass::SUBCLASS_NORMAL,  .mLaunchRate = 0,    .mPlantName = "GIFT_BOX" },
};

Plant::Plant()
{
}

// GOTY @Patoke: 0x461483
void Plant::PlantInitialize(int theGridX, int theGridY, SeedType theSeedType, SeedType theImitaterType)
{
    mPlantCol = theGridX;
    mRow = theGridY;
    if (mBoard)
    {
        mX = mBoard->GridToPixelX(theGridX, theGridY);
        mY = mBoard->GridToPixelY(theGridX, theGridY);
    }
    mAnimCounter = 0;
    mAnimPing = true;
    mIsPvPZombieSunflower = false;
	mPvPHostPlantID = -1;
	mPvPSyncSeen = false;
    mFrame = 0;
    mShootingCounter = 0;
    mHypnoSpawnCounter = 1500;
    mDoomPeaPlantTime = (mBoard != nullptr) ? mBoard->mBoardUpdateCounter : 0;
    mDoomPeaCounter = 1500;
    mDoomPeaAngle = 0;
    mDoomPeaBeamTimer = 0;
    mDoomPeaRestore = 0;
    mDoomPeaPrimaryCounter = 120;
    mDoomPeaPrimaryFlash = 0;
    mGatlingUlt = false;
    mGatlingUltRemaining = 0;
    mGatlingUltYOffset = 0;
    mWinterMelonUlt = false;
    mWinterMelonUltRemaining = 0;
    mCharmProbability = 10;
    mOverdrawnSuns = 0;
    mOverdrawnTimer = 0;
    mShakeOffsetX = 0.0f;
    mShakeOffsetY = 0.0f;
    mFrameLength = RandRangeInt(12, 18);
    mTargetX = -1;
    mTargetY = -1;
    mStartRow = mRow;
    mNumFrames = 5;
    mState = PlantState::STATE_NOTREADY;
    mDead = false;
    mSquished = false;
    mSeedType = theSeedType;
    mImitaterType = theImitaterType;
    mPlantHealth = (theSeedType == SeedType::SEED_SHIELDWALLNUT) ? 16000 : 300;
    mDoSpecialCountdown = 0;
    mDisappearCountdown = 200;
    mStateCountdown = 0;
    if (theSeedType == SeedType::SEED_GIFT_BOX)
        mStateCountdown = 120; // closed for 1 second, then open for 0.2 seconds
    mSilenceCountdown = 0;
    mErrorCountdown = 0;
    mChomperBonusChance = 0;
    mChomperEatenCount = 0;
    mChomperBiteStreak = 0;
    mScaredyShotCount = 0;
    mPuffColonyMembers = 1;
    mPuffColonyOffspring = false;
    mCricketShieldHealthSnapshot = 0;
    mCricketShieldSnapshotValid = false;
    mStarTrackCount = 0;
    mSpikeweedCooldown = 1800;
    mChainCount = 0;
    mChainDelay = 0;
    mChainOriginX = 0.0f;
    mChainOriginY = 0.0f;
    mChainDamageFlags = 0;
    mChainRow = 0;
    mChainThreepeaterCycle = 0;
    mChainProjectileType = ProjectileType::PROJECTILE_PEA;
    mParticleID = ParticleSystemID::PARTICLESYSTEMID_NULL;
    mBodyReanimID = ReanimationID::REANIMATIONID_NULL;
    mHeadReanimID = ReanimationID::REANIMATIONID_NULL;
    mHeadReanimID2 = ReanimationID::REANIMATIONID_NULL;
    mHeadReanimID3 = ReanimationID::REANIMATIONID_NULL;
    mBlinkReanimID = ReanimationID::REANIMATIONID_NULL;
    mLightReanimID = ReanimationID::REANIMATIONID_NULL;
    mSleepingReanimID = ReanimationID::REANIMATIONID_NULL;
    mBlinkCountdown = 0;
    mRecentlyEatenCountdown = 0;
    mEatenFlashCountdown = 0;
    mBeghouledFlashCountdown = 0;
    mWidth = 80;
    mHeight = 80;
    memset(mMagnetItems, 0, sizeof(mMagnetItems));
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    mIsAsleep = false;
    mWakeUpCounter = 0;
    mOnBungeeState = PlantOnBungeeState::NOT_ON_BUNGEE;
    mPottedPlantIndex = -1;
    mLaunchRate = aPlantDef.mLaunchRate;
    mSubclass = aPlantDef.mSubClass;

    // 「我真是搞不明白......」：每株植物独立抽取一条规则，不按种类固定。
    mRuleId = PLANT_RULE_NONE;
    mRuleRevealed = false;
    mRuleRevealTimer = 200;
    mRuleTimer = 0;
    mRuleCounter = 0;
    mRuleAux = 0;
    mRuleJam = 0;
    mRuleStun = 0;
    if (mBoard != nullptr && mApp->IsWhatRulesMode() && theSeedType != SeedType::SEED_GIFT_BOX)
    {
        SeedType aRuleSeed = theSeedType;
        if (theSeedType == SeedType::SEED_IMITATER && theImitaterType != SeedType::SEED_NONE)
            aRuleSeed = theImitaterType;
        mRuleId = RollPlantRule(aRuleSeed);
    }
    mRenderOrder = CalcRenderOrder();
    // The beam is an effect over the battlefield and must be composited after
    // zombies. Keep the plant itself in the projectile layer for this plant.
    if (mSeedType == SeedType::SEED_DOOMPEA)
        mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PROJECTILE, mRow, 900);

    Reanimation* aBodyReanim = nullptr;
    if (aPlantDef.mReanimationType != ReanimationType::REANIM_NONE)
    {
        float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);
        aBodyReanim = mApp->AddReanimation(0.0f, aOffsetY, mRenderOrder + 1, aPlantDef.mReanimationType);
        aBodyReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
        aBodyReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);

        if (aBodyReanim->TrackExists("anim_idle"))
            aBodyReanim->SetFramesForLayer("anim_idle");

        if (mSeedType == SeedType::SEED_SHIELDWALLNUT)
        {
            // Brown wall-nut hues shifted by 194 degrees become blue.
            aBodyReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT_194;
            aBodyReanim->mColorOverride = Color(255, 255, 255, 128);
        }

        if (mApp->IsWallnutBowlingLevel() && aBodyReanim->TrackExists("_ground"))
        {
            aBodyReanim->SetFramesForLayer("_ground");
            if (mSeedType == SeedType::SEED_WALLNUT || mSeedType == SeedType::SEED_EXPLODE_O_NUT)
                aBodyReanim->mAnimRate = RandRangeFloat(12.0f, 18.0f);
            else if (mSeedType == SeedType::SEED_GIANT_WALLNUT)
                aBodyReanim->mAnimRate = RandRangeFloat(6.0f, 10.0f);
        }

        aBodyReanim->mIsAttachment = true;
        mBodyReanimID = mApp->ReanimationGetID(aBodyReanim);
        mBlinkCountdown = 400 + Sexy::Rand(400);
    }

    if (IsNocturnal(mSeedType) && mBoard && !mBoard->StageIsNight())
        SetSleeping(true);

    if (mLaunchRate > 0)
    {
        if (MakesSun())
            mLaunchCounter = RandRangeInt(300, mLaunchRate / 2);
        else
            mLaunchCounter = RandRangeInt(0, mLaunchRate);
    }
    else
        mLaunchCounter = 0;

    switch (theSeedType)
    {
    case SeedType::SEED_BLOVER:
    {
        mDoSpecialCountdown = 50;

        if (IsInPlay())
        {
            aBodyReanim->SetFramesForLayer("anim_blow");
            aBodyReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
            aBodyReanim->mAnimRate = 20.0f;
        }
        else
        {
            aBodyReanim->SetFramesForLayer("anim_idle");
            aBodyReanim->mAnimRate = 10.0f;
        }

        break;
    }
    case SeedType::SEED_PEASHOOTER:
    case SeedType::SEED_IRONPEASHOOTER:
    case SeedType::SEED_DOOMPEA:
    case SeedType::SEED_SNOWPEA:
    case SeedType::SEED_REPEATER:
    case SeedType::SEED_LEFTPEATER:
    case SeedType::SEED_GATLINGPEA:
        if (aBodyReanim)
        {
            aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
            Reanimation* aHeadReanim = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
            aHeadReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
            aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim->SetFramesForLayer("anim_head_idle");
            mHeadReanimID = mApp->ReanimationGetID(aHeadReanim);

            if (aBodyReanim->TrackExists("anim_stem"))
                aHeadReanim->AttachToAnotherReanimation(aBodyReanim, "anim_stem");
            else if (aBodyReanim->TrackExists("anim_idle"))
                aHeadReanim->AttachToAnotherReanimation(aBodyReanim, "anim_idle");
        }
        break;
    case SeedType::SEED_SPLITPEA:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
        Reanimation* aHeadReanim1 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim1->mLoopType = ReanimLoopType::REANIM_LOOP;
        aHeadReanim1->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim1->SetFramesForLayer("anim_head_idle");
        aHeadReanim1->AttachToAnotherReanimation(aBodyReanim, "anim_idle");
        mHeadReanimID = mApp->ReanimationGetID(aHeadReanim1);

        Reanimation* aHeadReanim2 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim2->mLoopType = ReanimLoopType::REANIM_LOOP;
        aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim2->SetFramesForLayer("anim_splitpea_idle");
        aHeadReanim2->AttachToAnotherReanimation(aBodyReanim, "anim_idle");
        mHeadReanimID2 = mApp->ReanimationGetID(aHeadReanim2);

        break;
    }
    case SeedType::SEED_THREEPEATER:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
        Reanimation* aHeadReanim1 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim1->mLoopType = ReanimLoopType::REANIM_LOOP;
        aHeadReanim1->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim1->SetFramesForLayer("anim_head_idle1");
        aHeadReanim1->AttachToAnotherReanimation(aBodyReanim, "anim_head1");
        mHeadReanimID = mApp->ReanimationGetID(aHeadReanim1);

        Reanimation* aHeadReanim2 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim2->mLoopType = ReanimLoopType::REANIM_LOOP;
        aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim2->SetFramesForLayer("anim_head_idle2");
        aHeadReanim2->AttachToAnotherReanimation(aBodyReanim, "anim_head2");
        mHeadReanimID2 = mApp->ReanimationGetID(aHeadReanim2);

        Reanimation* aHeadReanim3 = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, aPlantDef.mReanimationType);
        aHeadReanim3->mLoopType = ReanimLoopType::REANIM_LOOP;
        aHeadReanim3->mAnimRate = aBodyReanim->mAnimRate;
        aHeadReanim3->SetFramesForLayer("anim_head_idle3");
        aHeadReanim3->AttachToAnotherReanimation(aBodyReanim, "anim_head3");
        mHeadReanimID3 = mApp->ReanimationGetID(aHeadReanim3);

        break;
    }
    case SeedType::SEED_WALLNUT:
        mPlantHealth = 4000;
        mBlinkCountdown = 1000 + Sexy::Rand(1000);
        break;
    case SeedType::SEED_EXPLODE_O_NUT:
        mPlantHealth = 4000;
        mBlinkCountdown = 1000 + Sexy::Rand(1000);
        aBodyReanim->mColorOverride = Color(255, 64, 64);
        break;
    case SeedType::SEED_GIANT_WALLNUT:
        mPlantHealth = 4000;
        mBlinkCountdown = 1000 + Sexy::Rand(1000);
        break;
    case SeedType::SEED_TALLNUT:
        mPlantHealth = 8000;
        mHeight = 80;
        mBlinkCountdown = 1000 + Sexy::Rand(1000);
        break;
    case SeedType::SEED_GARLIC:
        TOD_ASSERT(aBodyReanim);
        mPlantHealth = 400;
        aBodyReanim->SetTruncateDisappearingFrames();
        break;
    case SeedType::SEED_GOLD_MAGNET:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->SetTruncateDisappearingFrames();
        break;
    case SeedType::SEED_IMITATER:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->mAnimRate = RandRangeFloat(25.0f, 30.0f);
        mStateCountdown = 200;
        break;
    case SeedType::SEED_CHERRYBOMB:
    case SeedType::SEED_JALAPENO:
    {
        TOD_ASSERT(aBodyReanim);

        if (IsInPlay())
        {
            mDoSpecialCountdown = 100;

            aBodyReanim->SetFramesForLayer("anim_explode");
            aBodyReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;

            mApp->PlayFoley(FoleyType::FOLEY_REVERSE_EXPLOSION);
        }

        break;
    }
    case SeedType::SEED_POTATOMINE:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mAnimRate = 12.0f;

        if (IsInPlay())
        {
            aBodyReanim->AssignRenderGroupToTrack("anim_glow", RENDER_GROUP_HIDDEN);
            mStateCountdown = 1500;
        }
        else
        {
            aBodyReanim->SetFramesForLayer("anim_armed");
            mState = PlantState::STATE_POTATO_ARMED;
        }

        break;
    }
    case SeedType::SEED_GRAVEBUSTER:
    {
        TOD_ASSERT(aBodyReanim);

        if (IsInPlay())
        {
            aBodyReanim->SetFramesForLayer("anim_land");
            aBodyReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;

            mState = PlantState::STATE_GRAVEBUSTER_LANDING;
            mApp->PlayFoley(FoleyType::FOLEY_GRAVEBUSTERCHOMP);
        }

        break;
    }
    case SeedType::SEED_SUNSHROOM:
    {
        TOD_ASSERT(aBodyReanim);

        aBodyReanim->mFrameBasePose = 6;

        if (IsInPlay())
        {
            mX += Sexy::Rand(10) - 5;
            mY += Sexy::Rand(10) - 5;
        }
        else if (mIsAsleep)
            aBodyReanim->SetFramesForLayer("anim_bigsleep");
        else
            aBodyReanim->SetFramesForLayer("anim_bigidle");

        mState = PlantState::STATE_SUNSHROOM_SMALL;
        mStateCountdown = 12000;

        break;
    }
    case SeedType::SEED_PUFFSHROOM:
    case SeedType::SEED_SEASHROOM:
        if (IsInPlay())
        {
            mX += Sexy::Rand(10) - 5;
            mY += Sexy::Rand(6) - 3;
        }
        break;
    case SeedType::SEED_PUMPKINSHELL:
    {
        mPlantHealth = 4000;
        mWidth = 120;

        TOD_ASSERT(aBodyReanim);
        aBodyReanim->AssignRenderGroupToTrack("Pumpkin_back", 1);
        break;
    }
    case SeedType::SEED_CHOMPER:
        mState = PlantState::STATE_READY;
        break;
    case SeedType::SEED_PLANTERN:
    {
        mStateCountdown = 50;

        if (!IsOnBoard() || mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
        {
            AddAttachedParticle(mX + 40, mY + 40, static_cast<int>(RenderLayer::RENDER_LAYER_FOG) + 1, ParticleEffect::PARTICLE_LANTERN_SHINE);
        }
        if (IsInPlay())
        {
            mApp->PlaySample(Sexy::SOUND_PLANTERN);
        }
        
        break;
    }
    case SeedType::SEED_TORCHWOOD:
        break;
    case SeedType::SEED_MARIGOLD:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->mAnimRate = RandRangeFloat(15.0f, 20.0f);
        break;
    case SeedType::SEED_CACTUS:
        mState = PlantState::STATE_CACTUS_LOW;
        break;
    case SeedType::SEED_INSTANT_COFFEE:
        mDoSpecialCountdown = 100;
        break;
    case SeedType::SEED_SCAREDYSHROOM:
        mState = PlantState::STATE_READY;
        break;
    case SeedType::SEED_COBCANNON:
        if (IsInPlay())
        {
            mState = PlantState::STATE_COBCANNON_ARMING;
            mStateCountdown = 500;

            TOD_ASSERT(aBodyReanim);
            aBodyReanim->SetFramesForLayer("anim_unarmed_idle");
        }
        break;
    case SeedType::SEED_KERNELPULT:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->AssignRenderGroupToPrefix("Cornpult_butter", RENDER_GROUP_HIDDEN);
        break;
    case SeedType::SEED_MAGNETSHROOM:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->SetTruncateDisappearingFrames();
        break;
    case SeedType::SEED_SPIKEROCK:
        mPlantHealth = 450;
        TOD_ASSERT(aBodyReanim);
        break;
    case SeedType::SEED_SPROUT:
        break;
    case SeedType::SEED_FLOWERPOT:
        if (IsInPlay())
        {
            mState = PlantState::STATE_FLOWERPOT_INVULNERABLE;
            mStateCountdown = 100;
        }
        break;
    case SeedType::SEED_LILYPAD:
        if (IsInPlay())
        {
            mState = PlantState::STATE_LILYPAD_INVULNERABLE;
            mStateCountdown = 100;
        }
        break;
    case SeedType::SEED_TANGLEKELP:
        TOD_ASSERT(aBodyReanim);
        aBodyReanim->SetTruncateDisappearingFrames();
    default:
        break;
    }

    if (mSeedType == SeedType::SEED_SHIELDWALLNUT)
    {
        auto ApplyShieldVisual = [this](ReanimationID theID)
        {
            Reanimation* aReanim = mApp->ReanimationTryToGet(theID);
            if (aReanim)
            {
                aReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT_194;
                aReanim->mColorOverride = Color(255, 255, 255, 128);
            }
        };
        ApplyShieldVisual(mBodyReanimID);
        ApplyShieldVisual(mHeadReanimID);
        ApplyShieldVisual(mHeadReanimID2);
        ApplyShieldVisual(mHeadReanimID3);
    }

    // Iron pea is washed out; Doom pea is rendered as a solid black plant.
    if (mSeedType == SeedType::SEED_IRONPEASHOOTER)
    {
        if (aBodyReanim)
                aBodyReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_WASHED_OUT;
        for (ReanimationID aReanimID : { mHeadReanimID, mHeadReanimID2, mHeadReanimID3 })
        {
            Reanimation* aReanim = mApp->ReanimationTryToGet(aReanimID);
            if (aReanim)
                aReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_WASHED_OUT;
        }
    }
    
    if ((mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BIG_TIME) &&
        (theSeedType == SeedType::SEED_WALLNUT || theSeedType == SeedType::SEED_SUNFLOWER || theSeedType == SeedType::SEED_MARIGOLD))
    {
        mPlantHealth *= 2;
    }
    if (mApp->IsWhatRulesMode())
        ApplyPlantRule();
    mPlantMaxHealth = mPlantHealth;
	mCricketShieldHealthSnapshot = mPlantHealth;

    if (mSeedType != SeedType::SEED_FLOWERPOT && IsOnBoard())
    {
        TOD_ASSERT(mBoard);
        Plant* aFlowerPot = mBoard->GetFlowerPotAt(mPlantCol, mRow);
        if (aFlowerPot)
            mApp->ReanimationGet(aFlowerPot->mBodyReanimID)->mAnimRate = 0.0f;
    }
}

int Plant::CalcRenderOrder()
{
    PLANT_ORDER anOrder = PLANT_ORDER::PLANT_ORDER_NORMAL;
    RenderLayer aLayer = RenderLayer::RENDER_LAYER_PLANT;

    SeedType aSeedType = mSeedType;
    if (mSeedType == SeedType::SEED_IMITATER && mImitaterType != SeedType::SEED_NONE)
        aSeedType = mImitaterType;

    if (mApp->IsWallnutBowlingLevel())
    {
        aLayer = RenderLayer::RENDER_LAYER_PROJECTILE;
    }
    else if (aSeedType == SeedType::SEED_PUMPKINSHELL)
    {
        anOrder = PLANT_ORDER::PLANT_ORDER_PUMPKIN;
    }
    else if (IsFlying(aSeedType))
    {
        anOrder = PLANT_ORDER::PLANT_ORDER_FLYER;
    }
    else if (aSeedType == SeedType::SEED_FLOWERPOT || (aSeedType == SeedType::SEED_LILYPAD && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN))
    {
        anOrder = PLANT_ORDER::PLANT_ORDER_LILYPAD;
    }

    return Board::MakeRenderOrder(aLayer, mRow, anOrder * 5 - mX + 800);
}

void Plant::SetSleeping(bool theIsAsleep)
{
    if (mIsAsleep == theIsAsleep || NotOnGround())
        return;

    mIsAsleep = theIsAsleep;
    if (theIsAsleep)
    {
        float aPosX = mX + 50.0f;
        float aPosY = mY + 40.0f;
        if (mSeedType == SeedType::SEED_FUMESHROOM)
            aPosX += 12.0f;
        else if (mSeedType == SeedType::SEED_SCAREDYSHROOM)
            aPosY -= 20.0f;
        else if (mSeedType == SeedType::SEED_GLOOMSHROOM)
            aPosY -= 12.0f;

        Reanimation* aSleepReanim = mApp->AddReanimation(aPosX, aPosY, mRenderOrder + 2, ReanimationType::REANIM_SLEEPING);
        aSleepReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
        aSleepReanim->mAnimRate = RandRangeFloat(6.0f, 8.0f);
        aSleepReanim->mAnimTime = RandRangeFloat(0.0f, 0.9f);
        mSleepingReanimID = mApp->ReanimationGetID(aSleepReanim);
    }
    else
    {
        mApp->RemoveReanimation(mSleepingReanimID);
        mSleepingReanimID = ReanimationID::REANIMATIONID_NULL;
    }

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == nullptr)
        return;

    if (theIsAsleep)
    {
        if (!IsInPlay() && mSeedType == SeedType::SEED_SUNSHROOM)
        {
            aBodyReanim->SetFramesForLayer("anim_bigsleep");
        }
        else if (aBodyReanim->TrackExists("anim_sleep"))
        {
            float aAnimTime = aBodyReanim->mAnimTime;
            aBodyReanim->StartBlend(20);
            aBodyReanim->SetFramesForLayer("anim_sleep");
            aBodyReanim->mAnimTime = aAnimTime;
        }
        else
        {
            aBodyReanim->mAnimRate = 1.0f;
        }

        EndBlink();
    }
    else
    {
        if (!IsInPlay() && mSeedType == SeedType::SEED_SUNSHROOM)
        {
            aBodyReanim->SetFramesForLayer("anim_bigidle");
        }
        else if (aBodyReanim->TrackExists("anim_idle"))
        {
            float aAnimTime = aBodyReanim->mAnimTime;
            aBodyReanim->StartBlend(20);
            aBodyReanim->SetFramesForLayer("anim_idle");
            aBodyReanim->mAnimTime = aAnimTime;
        }

        if (aBodyReanim->mAnimRate < 2.0f && IsInPlay())
            aBodyReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);
    }
}

int Plant::GetDamageRangeFlags(PlantWeapon thePlantWeapon)
{
    switch (mSeedType)
    {
    case SeedType::SEED_CACTUS:
        return thePlantWeapon == PlantWeapon::WEAPON_SECONDARY ? 1 : 2;
    case SeedType::SEED_CHERRYBOMB:
    case SeedType::SEED_JALAPENO:
    case SeedType::SEED_COBCANNON:
    case SeedType::SEED_DOOMSHROOM:
        return 127;
    case SeedType::SEED_MELONPULT:
    case SeedType::SEED_CABBAGEPULT:
    case SeedType::SEED_KERNELPULT:
    case SeedType::SEED_WINTERMELON:
        return 13;
    case SeedType::SEED_POTATOMINE:
        return 77;
    case SeedType::SEED_SQUASH:
        return 13;
    case SeedType::SEED_PUFFSHROOM:
    case SeedType::SEED_SEASHROOM:
    case SeedType::SEED_FUMESHROOM:
    case SeedType::SEED_GLOOMSHROOM:
    case SeedType::SEED_CHOMPER:
        return 9;
    case SeedType::SEED_CATTAIL:
        return 11;
    case SeedType::SEED_TANGLEKELP:
        return 5;
    case SeedType::SEED_GIANT_WALLNUT:
        return 17;
    default:
        return 3;  // DAMAGES_GROUND | DAMAGES_FLYING
    }
}

bool Plant::IsOnHighGround()
{
    return mBoard && mBoard->mGridSquareType[mPlantCol][mRow] == GridSquareType::GRIDSQUARE_HIGH_GROUND;
}

void Plant::SpikeRockTakeDamage(int theDamage)
{
    if (AbsorbDamageByShield(theDamage))
        return;
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    SpikeweedAttack();

    mPlantHealth -= theDamage;
    if (mPlantHealth <= 300)
    {
        aBodyReanim->AssignRenderGroupToTrack("bigspike3", RENDER_GROUP_HIDDEN);
    }
    if (mPlantHealth <= 150)
    {
        aBodyReanim->AssignRenderGroupToTrack("bigspike2", RENDER_GROUP_HIDDEN);
    }
    if (mPlantHealth <= 0)
    {
        mApp->PlayFoley(FoleyType::FOLEY_SQUISH);
        Die();
    }
}

bool Plant::IsSpiky()
{
    return mSeedType == SeedType::SEED_SPIKEWEED || mSeedType == SeedType::SEED_SPIKEROCK;
}

void Plant::DoRowAreaDamage(int theDamage, unsigned int theDamageFlags)
{
	if (mApp->IsCricketMode())
		theDamage = theDamage * mBoard->mChallenge->CricketDamageMultiplierPercent(mRow) / 100;
    if (mApp->IsWhatRulesMode())
    {
        if (mRuleId == PLANT_RULE_HEAVY_SHOT)
            theDamage = theDamage * 140 / 100;
        else if (mRuleId == PLANT_RULE_SOFT_SHOT)
            theDamage = theDamage * 75 / 100;
    }
    int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        int aDiffY = (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) ? 0 : (aZombie->mRow - mRow);
        if (aZombie->mMindControlled)  // 己方魅惑僵尸不受伤
            continue;
        if (mSeedType == SeedType::SEED_GLOOMSHROOM)
        {
            if (aDiffY < -1 || aDiffY > 1)
                continue;
        }
        else if (aDiffY)
            continue;

        if (aZombie->mOnHighGround == IsOnHighGround() && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            if (GetRectOverlap(aAttackRect, aZombieRect) > 0)
            {
                int aDamage = theDamage;
                if ((aZombie->mZombieType == ZombieType::ZOMBIE_ZAMBONI || aZombie->mZombieType == ZombieType::ZOMBIE_CATAPULT) &&
                    (TestBit(theDamageFlags, DamageFlags::DAMAGE_SPIKE)))
                {
                    aDamage = 1800;

                    if (mSeedType == SeedType::SEED_SPIKEROCK)
                    {
                        SpikeRockTakeDamage();
                    }
                    else
                    {
                        Die();
                    }
                }

                aZombie->TakeDamage(GetAttackDamage(aDamage), theDamageFlags);
                if (mSeedType == SeedType::SEED_FUMESHROOM)
                    aZombie->mPoisonStacks += HasBloverBehind() ? 3 : 1;
                mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
            }
        }
    }
}

TodParticleSystem* Plant::AddAttachedParticle(int thePosX, int thePosY, int theRenderPosition, ParticleEffect theEffect)
{
    TodParticleSystem* aParticle = mApp->ParticleTryToGet(mParticleID);
    if (aParticle)
        aParticle->ParticleSystemDie();

    TodParticleSystem* aNewParticle = mApp->AddTodParticle(thePosX, thePosY, theRenderPosition, theEffect);
    if (aNewParticle)
        mParticleID = mApp->ParticleGetID(aNewParticle);

    return aNewParticle;
}

bool Plant::FindTargetAndFire(int theRow, PlantWeapon thePlantWeapon)
{
	if (mApp->IsPvPMode() && !mApp->IsPvPHost())
		return false;

    // 魅惑菇不再发射魅惑子弹（改为召唤魅惑僵尸）
    if (mSeedType == SeedType::SEED_HYPNOSHROOM)
        return false;

    Zombie* aZombie = FindTargetZombie(theRow, thePlantWeapon);
    // 双向射手后向子弹：即使左边没有僵尸也会发射
    bool aBackPea = (mSeedType == SeedType::SEED_SPLITPEA && thePlantWeapon == PlantWeapon::WEAPON_SECONDARY);
    if (aZombie == nullptr && !aBackPea)
        return false;

    if (aBackPea)
    {
        // 限制不朽反弹豌豆数量，防止无限堆积导致渲染列表溢出崩溃
        int aBounceCount = 0;
        Projectile* aProjectile = nullptr;
        while (mBoard->IterateProjectiles(aProjectile))
        {
            if (aProjectile->mIsBouncingPea && aProjectile->mRow == theRow)
                aBounceCount++;
        }
        if (aBounceCount >= MAX_BOUNCING_PEAS_PER_ROW)
            return false;
    }

    EndBlink();
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    Reanimation* aHeadReanim = mApp->ReanimationTryToGet(mHeadReanimID);

    if (mSeedType == SeedType::SEED_SPLITPEA && thePlantWeapon == PlantWeapon::WEAPON_SECONDARY)
    {
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
        aHeadReanim2->StartBlend(20);
        aHeadReanim2->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
        aHeadReanim2->mAnimRate = 35.0f;
        aHeadReanim2->SetFramesForLayer("anim_splitpea_shooting");
        mShootingCounter = 26;
    }
    else if (aHeadReanim && aHeadReanim->TrackExists("anim_shooting"))
    {
        aHeadReanim->StartBlend(20);
        aHeadReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
        aHeadReanim->mAnimRate = 35.0f;
        aHeadReanim->SetFramesForLayer("anim_shooting");

        mShootingCounter = 33;
        if (mSeedType == SeedType::SEED_REPEATER || mSeedType == SeedType::SEED_SPLITPEA || mSeedType == SeedType::SEED_LEFTPEATER)
        {
            aHeadReanim->mAnimRate = 45.0f;
            mShootingCounter = 26;
        }
        else if (mSeedType == SeedType::SEED_GATLINGPEA)
        {
            aHeadReanim->mAnimRate = 38.0f;
            // 击杀越多开大概率越高，基础 10%，每 8000 击杀 +10%，最高 20%
            int aKills = std::min(mApp->mTotalZombieKills, 8000);
            int aChance = 10 + 10 * aKills / 8000;
            mGatlingUlt = (Rand(100) < aChance);
            if (mGatlingUlt)
            {
                aHeadReanim->mLoopType = ReanimLoopType::REANIM_LOOP;  // 大招期间枪管持续旋转
                mShootingCounter = 1;          // 立即发射第一颗
                mGatlingUltRemaining = 200;
                mGatlingUltYOffset = 0;
            }
            else
            {
                aHeadReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
                mShootingCounter = 100;
            }
        }
    }
    else if (mState == PlantState::STATE_CACTUS_HIGH)
    {
        PlayBodyReanim("anim_shootinghigh", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 35.0f);
        mShootingCounter = 23;
    }
    else if (mSeedType == SeedType::SEED_GLOOMSHROOM)
    {
        PlayBodyReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 14.0f);
        mShootingCounter = 200;
    }
    else if (mSeedType == SeedType::SEED_CATTAIL)
    {
        PlayBodyReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 30.0f);
        mShootingCounter = 50;
    }
    else if (mSeedType == SeedType::SEED_HYPNOSHROOM)
    {
        PlayBodyReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 20, 35.0f);
        mShootingCounter = 50;
    }
    else if (aBodyReanim && aBodyReanim->TrackExists("anim_shooting"))
    {
        PlayBodyReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 35.0f);

        switch (mSeedType)
        {
        case SeedType::SEED_FUMESHROOM:     mShootingCounter = 50;  break;
        case SeedType::SEED_PUFFSHROOM:     mShootingCounter = 29;  break;
        case SeedType::SEED_SCAREDYSHROOM:  mShootingCounter = std::max(3, 25 - mScaredyShotCount * 2);  break;
        case SeedType::SEED_CABBAGEPULT:    mShootingCounter = 32;  break;
        case SeedType::SEED_MELONPULT:
        case SeedType::SEED_WINTERMELON:    mShootingCounter = 36;  break;
        case SeedType::SEED_KERNELPULT:
        {
            if (Sexy::Rand(4) == 0)
            {
                aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_butter", RENDER_GROUP_NORMAL);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_kernal", RENDER_GROUP_HIDDEN);
                mState = PlantState::STATE_KERNELPULT_BUTTER;
            }

            mShootingCounter = 30;
            break;
        }
        case SeedType::SEED_CACTUS:         mShootingCounter = 35;  break;
        default:                            mShootingCounter = 29;  break;
        }
    }
    else
        Fire(aZombie, theRow, thePlantWeapon);

    return true;
}

void Plant::LaunchThreepeater()
{
    int rowAbove = mRow - 1;
    int rowBelow = mRow + 1;

    if ((FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY)) ||
        (mBoard->RowCanHaveZombies(rowAbove) && FindTargetZombie(rowAbove, PlantWeapon::WEAPON_PRIMARY)) ||
        (mBoard->RowCanHaveZombies(rowBelow) && FindTargetZombie(rowBelow, PlantWeapon::WEAPON_PRIMARY)))
    {
        Reanimation* aHeadReanim1 = mApp->ReanimationGet(mHeadReanimID);
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
        Reanimation* aHeadReanim3 = mApp->ReanimationGet(mHeadReanimID3);

        if (mBoard->RowCanHaveZombies(rowBelow))
        {
            aHeadReanim1->StartBlend(10);
            aHeadReanim1->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
            aHeadReanim1->mAnimRate = 20.0f;
            aHeadReanim1->SetFramesForLayer("anim_shooting1");
        }

        aHeadReanim2->StartBlend(10);
        aHeadReanim2->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
        aHeadReanim2->mAnimRate = 20.0f;
        aHeadReanim2->SetFramesForLayer("anim_shooting2");

        if (mBoard->RowCanHaveZombies(rowAbove))
        {
            aHeadReanim3->StartBlend(10);
            aHeadReanim3->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
            aHeadReanim3->mAnimRate = 20.0f;
            aHeadReanim3->SetFramesForLayer("anim_shooting3");
        }

        mShootingCounter = 35;
    }
}

bool Plant::FindStarFruitTarget()
{
    if (mRecentlyEatenCountdown > 0)
        return true;

    int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
    int aCenterStarX = mX + 40;
    int aCenterStarY = mY + 40;

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        Rect aZombieRect = aZombie->GetZombieRect();
        if (aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS && mPlantCol >= 5)
                return true;

            if (aZombie->mRow == mRow)
            {
                if (aZombieRect.mX + aZombieRect.mWidth < aCenterStarX)
                    return true;
            }
            else
            {
                if (aZombie->mZombieType == ZombieType::ZOMBIE_DIGGER)
                    aZombieRect.mX += 10;

                float aProjectileTime = Distance2D(aCenterStarX, aCenterStarY, aZombieRect.mX + aZombieRect.mWidth / 2, aZombieRect.mY + aZombieRect.mHeight / 2) / 3.33f;
                int aZombieHitX = aZombie->ZombieTargetLeadX(aProjectileTime) - aZombieRect.mWidth / 2;
                if ((aZombieHitX + aZombieRect.mWidth > aCenterStarX) && (aZombieHitX < aCenterStarX))
                    return true;

                int aCenterZombieX = aZombieHitX + aZombieRect.mWidth / 2;
                int aCenterZombieY = aZombieRect.mY + aZombieRect.mHeight / 2;
                float angle = RAD_TO_DEG(atan2(aCenterZombieY - aCenterStarY, aCenterZombieX - aCenterStarX));
                if (abs(aZombie->mRow - mRow) < 2)
                {
                    if ((angle > 20.0f && angle < 40.0f) || (angle < -25.0f && angle > -45.0f))
                        return true;
                }
                else
                {
                    if ((angle > 25.0f && angle < 35.0f) || (angle < -28.0f && angle > -38.0f))
                        return true;
                }
            }
        }
    }

    return false;
}

void Plant::LaunchStarFruit()
{
    if (FindStarFruitTarget())
    {
        PlayBodyReanim("anim_shoot", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 28.0f);
        mShootingCounter = 40;
    }
}

void Plant::StarFruitFire()
{
	if (mApp->IsPvPMode() && !mApp->IsPvPHost())
		return;

    mApp->PlayFoley(FoleyType::FOLEY_THROW);

    const bool aPiercingBurst = mStarTrackCount >= 5;
    if (aPiercingBurst)
        mStarTrackCount = 0;

    float aShootAngleX = cos(DEG_TO_RAD(30.0f)) * 3.33f;
    float aShootAngleY = sin(DEG_TO_RAD(30.0f)) * 3.33f;
    for (int i = 0; i < 5; i++)
    {
        Projectile* aProjectile = mBoard->AddProjectile(mX + 25, mY + 25, mRenderOrder - 1, mRow, ProjectileType::PROJECTILE_STAR);
        aProjectile->mErrorDamageMultiplier = GetAttackDamage(1);
        aProjectile->mDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
        aProjectile->mMotionType = ProjectileMotion::MOTION_STAR;
        aProjectile->mOwnerPlantID = static_cast<PlantID>(mBoard->mPlants.DataArrayGetID(this));
        aProjectile->mIsPiercingStar = aPiercingBurst;
        if (mApp->IsCricketMode())
            aProjectile->mDamageMultiplierPercent = mBoard->mChallenge->CricketDamageMultiplierPercent(mRow);

        switch (i)
        {
        case 0:     aProjectile->mVelX = -3.33f;         aProjectile->mVelY = 0.0f;             break;
        case 1:     aProjectile->mVelX = 0.0f;          aProjectile->mVelY = 3.33f;             break;
        case 2:     aProjectile->mVelX = 0.0f;          aProjectile->mVelY = -3.33f;            break;
        case 3:     aProjectile->mVelX = aShootAngleX;  aProjectile->mVelY = aShootAngleY;      break;
        case 4:     aProjectile->mVelX = aShootAngleX;  aProjectile->mVelY = -aShootAngleY;     break;
        default:    TOD_ASSERT(false);                                                               break;
        }
    }
}

void Plant::UpdateShooter()
{
    mLaunchCounter--;
    if (mLaunchCounter <= 0)
    {
        if (mSeedType == SeedType::SEED_SPLITPEA)
        {
            // 双向射手：本行豌豆越少射得越快
            int aPeaCount = 0;
            Projectile* aProjectile = nullptr;
            while (mBoard->IterateProjectiles(aProjectile))
            {
                if (aProjectile->mRow == mRow &&
                    (aProjectile->mProjectileType == ProjectileType::PROJECTILE_PEA ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_FIREBALL ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE ||
                     aProjectile->mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER))
                {
                    aPeaCount++;
                }
            }
            const int aMinRate = 40;
            const int aBaseRate = mLaunchRate;
            mLaunchCounter = std::min(aMinRate + aPeaCount * 18, aBaseRate) - Sexy::Rand(15);
        }
        else
        {
            mLaunchCounter = mLaunchRate - Sexy::Rand(15);
        }

        if (mSeedType == SeedType::SEED_THREEPEATER)
        {
            LaunchThreepeater();
        }
        else if (mSeedType == SeedType::SEED_STARFRUIT)
        {
            LaunchStarFruit();
        }
        else if (mSeedType == SeedType::SEED_SPLITPEA)
        {
            FindTargetAndFire(mRow, PlantWeapon::WEAPON_PRIMARY);
            FindTargetAndFire(mRow, PlantWeapon::WEAPON_SECONDARY);
        }
        else if (mSeedType == SeedType::SEED_CACTUS)
        {
            if (mState == PlantState::STATE_CACTUS_HIGH)
            {
                FindTargetAndFire(mRow, PlantWeapon::WEAPON_PRIMARY);
            }
            else if (mState == PlantState::STATE_CACTUS_LOW)
            {
                FindTargetAndFire(mRow, PlantWeapon::WEAPON_SECONDARY);
            }
        }
        else
        {
            FindTargetAndFire(mRow, PlantWeapon::WEAPON_PRIMARY);
        }
    }

    if (mLaunchCounter == 50 && mSeedType == SeedType::SEED_CATTAIL)
    {
        FindTargetAndFire(mRow, PlantWeapon::WEAPON_PRIMARY);
    }
    if (mLaunchCounter == 25)
    {
        if (mSeedType == SeedType::SEED_REPEATER || mSeedType == SeedType::SEED_LEFTPEATER)
        {
            FindTargetAndFire(mRow, PlantWeapon::WEAPON_PRIMARY);
        }
        else if (mSeedType == SeedType::SEED_SPLITPEA)
        {
            FindTargetAndFire(mRow, PlantWeapon::WEAPON_SECONDARY);
        }
    }
}

bool Plant::MakesSun()
{
    return mSeedType == SeedType::SEED_SUNFLOWER || mSeedType == SeedType::SEED_TWINSUNFLOWER || mSeedType == SeedType::SEED_SUNSHROOM;
}

void Plant::UpdateProductionPlant()
{
    if (mState == PlantState::STATE_OVERDRAWN)
        return;
    if (!IsInPlay() || mApp->IsIZombieLevel() || mApp->mGameMode == GameMode::GAMEMODE_UPSELL || mApp->mGameMode == GameMode::GAMEMODE_INTRO)
        return;

    if (mBoard->HasLevelAwardDropped())
        return;

    if (mSeedType == SeedType::SEED_MARIGOLD && mBoard->mCurrentWave == mBoard->mNumWaves)
    {
        if (mState != PlantState::STATE_MARIGOLD_ENDING)
        {
            mState = PlantState::STATE_MARIGOLD_ENDING;
            mStateCountdown = 6000;
        }
        else if (mStateCountdown <= 0)
            return;
    }

    if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_LAST_STAND && mBoard->mChallenge->mChallengeState != ChallengeState::STATECHALLENGE_LAST_STAND_ONSLAUGHT)
        return;

    mLaunchCounter--;
    if (mLaunchCounter <= 100)
    {
        int aFlashCountdown = TodAnimateCurve(100, 0, mLaunchCounter, 0, 100, TodCurves::CURVE_LINEAR);
        mEatenFlashCountdown = std::max(mEatenFlashCountdown, aFlashCountdown);
    }
    if (mLaunchCounter <= 0)
    {
        mLaunchCounter = RandRangeInt(mLaunchRate - 150, mLaunchRate);
        mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);

        bool aSkip = false;
        bool aSmall = false;
        bool aDouble = false;
        bool aHigh = false;
        bool aGold = false;
        if (mApp->IsWhatRulesMode())
        {
            switch (mRuleId)
            {
            case PLANT_RULE_EMPTY_HAND:
                if (Rand(100) < 20) aSkip = true;
                break;
            case PLANT_RULE_GREEDY:
                if (++mRuleAux % 5 == 0)
                {
                    aSkip = true;
                    mPlantHealth -= 10;
                }
                break;
            case PLANT_RULE_DOUBLE_YIELD:
                if (Rand(100) < 20) aDouble = true;
                break;
            case PLANT_RULE_HIGH_YIELD:
                if (Rand(100) < 50) aHigh = true;
                break;
            case PLANT_RULE_GOLD_FINGER:
                if (Rand(100) < 10) aGold = true;
                break;
            case PLANT_RULE_LOW_YIELD:
                if (Rand(100) < 25) aSmall = true;
                break;
            default:
                break;
            }
        }

        auto SpawnSun = [&](CoinType theType, CoinMotion theMotion)
        {
            if (aSkip)
                return;
            const CoinType aType = aSmall ? CoinType::COIN_SMALLSUN : theType;
            mBoard->AddCoin(mX, mY, aType, theMotion);
            if (aDouble)
                mBoard->AddCoin(mX, mY, aType, theMotion);
            if (aHigh)
                mBoard->AddCoin(mX, mY, CoinType::COIN_SMALLSUN, theMotion);
            if (aGold)
                mBoard->AddCoin(mX, mY, CoinType::COIN_GOLD, CoinMotion::COIN_MOTION_COIN);
        };

        if (mSeedType == SeedType::SEED_SUNSHROOM)
        {
            if (mState == PlantState::STATE_SUNSHROOM_SMALL)
            {
                SpawnSun(CoinType::COIN_SMALLSUN, CoinMotion::COIN_MOTION_FROM_PLANT);
				if (mApp->IsCricketMode())
					mBoard->mChallenge->CricketAddRowShield(mRow, 15);
            }
            else
            {
                SpawnSun(CoinType::COIN_SUN, CoinMotion::COIN_MOTION_FROM_PLANT);
				if (mApp->IsCricketMode())
					mBoard->mChallenge->CricketAddRowShield(mRow, 25);
            }
        }
        else if (mSeedType == SeedType::SEED_SUNFLOWER && mIsPvPZombieSunflower)
        {
            mBoard->AddPvPZombieSun(25);
        }
        else if (mSeedType == SeedType::SEED_SUNFLOWER)
        {
            SpawnSun(CoinType::COIN_SUN, CoinMotion::COIN_MOTION_FROM_PLANT);
        }
        else if (mSeedType == SeedType::SEED_TWINSUNFLOWER)
        {
            SpawnSun(CoinType::COIN_SUN, CoinMotion::COIN_MOTION_FROM_PLANT);
            SpawnSun(CoinType::COIN_SUN, CoinMotion::COIN_MOTION_FROM_PLANT);
        }
        else if (mSeedType == SeedType::SEED_MARIGOLD)
        {
            SpawnSun((Sexy::Rand(100) < 10) ? CoinType::COIN_GOLD : CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
        }

        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BIG_TIME)
        {
            if (mSeedType == SeedType::SEED_SUNFLOWER)
            {
                mBoard->AddCoin(mX, mY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_FROM_PLANT);
            }
            else if (mSeedType == SeedType::SEED_MARIGOLD)
            {
                mBoard->AddCoin(mX, mY, CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
            }
        }
    }
}

// Shared by physical shovel hover and the built-in agent preview.
void Plant::OnShovelHover()
{
    if (!IsOnBoard()) return;
    if (IsNocturnal(mSeedType) && mIsAsleep && mWakeUpCounter == 0)
        mWakeUpCounter = 100;
    if ((mSeedType == SeedType::SEED_SUNFLOWER || mSeedType == SeedType::SEED_TWINSUNFLOWER) &&
        mOverdrawnSuns == 0 && mState != PlantState::STATE_OVERDRAWN)
    {
        mOverdrawnSuns = 50;
        mOverdrawnTimer = 0;
    }
}

void Plant::UpdateOverdrawn()
{
    if (IsNocturnal(mSeedType) && mIsAsleep && mWakeUpCounter == 0 && IsOnBoard() &&
        mBoard->mCursorObject->mCursorType == CursorType::CURSOR_TYPE_SHOVEL)
    {
        int aMouseX = mApp->mWidgetManager->mLastMouseX;
        int aMouseY = mApp->mWidgetManager->mLastMouseY;
        int aGridX = mBoard->PixelToGridX(aMouseX, aMouseY);
        int aGridY = mBoard->PixelToGridY(aMouseX, aMouseY);
        if (aGridX == mPlantCol && aGridY == mRow)
        {
            mWakeUpCounter = 100;
        }
    }

    bool isSunProducer = mSeedType == SeedType::SEED_SUNFLOWER || mSeedType == SeedType::SEED_TWINSUNFLOWER;
    if (!isSunProducer || !IsOnBoard())
        return;

    if (mOverdrawnSuns > 0)
    {
        mOverdrawnTimer++;
        if (mOverdrawnTimer % 3 == 0)
        {
            mBoard->AddCoin(mX, mY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_FROM_PLANT);
            mOverdrawnSuns--;
        }

        if (mOverdrawnSuns == 0)
        {
            mState = PlantState::STATE_OVERDRAWN;
            mStateCountdown = 3600;
            Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
            if (aBodyReanim)
            {
                aBodyReanim->mColorOverride = Color(0, 0, 0, 100);
                aBodyReanim->mEnableExtraAdditiveDraw = true;
                aBodyReanim->mExtraAdditiveColor = Color(0, 0, 0, 80);
            }
        }
        return;
    }

    if (mState == PlantState::STATE_OVERDRAWN)
    {
        if (mStateCountdown > 0)
        {
            mStateCountdown--;
            return;
        }
        mState = PlantState::STATE_NOTREADY;
        Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
        if (aBodyReanim)
        {
            aBodyReanim->mColorOverride = Color(255, 255, 255, 255);
            aBodyReanim->mEnableExtraAdditiveDraw = false;
        }
        return;
    }

    if (mBoard->mCursorObject->mCursorType == CursorType::CURSOR_TYPE_SHOVEL)
    {
        int aMouseX = mApp->mWidgetManager->mLastMouseX;
        int aMouseY = mApp->mWidgetManager->mLastMouseY;
        int aGridX = mBoard->PixelToGridX(aMouseX, aMouseY);
        int aGridY = mBoard->PixelToGridY(aMouseX, aMouseY);
        if (aGridX == mPlantCol && aGridY == mRow)
        {
            OnShovelHover();
        }
    }
}

void Plant::UpdateSunShroom()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == PlantState::STATE_SUNSHROOM_SMALL)
    {
        if (mStateCountdown == 0)
        {
            PlayBodyReanim("anim_grow", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, 12.0f);
            mState = PlantState::STATE_SUNSHROOM_GROWING;
            mApp->PlayFoley(FoleyType::FOLEY_PLANTGROW);
        }

        UpdateProductionPlant();
    }
    else if (mState == PlantState::STATE_SUNSHROOM_GROWING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_bigidle", ReanimLoopType::REANIM_LOOP, 10, RandRangeFloat(12.0f, 15.0f));
            mState = PlantState::STATE_SUNSHROOM_BIG;
        }
    }
    else
    {
        UpdateProductionPlant();
    }
}

void Plant::UpdateGraveBuster()
{
    if (mState == PlantState::STATE_GRAVEBUSTER_LANDING)
    {
        if (mApp->ReanimationGet(mBodyReanimID)->mLoopCount > 0)
        {
            PlayBodyReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 10, 12.0f);
            mStateCountdown = 400;
            mState = PlantState::STATE_GRAVEBUSTER_EATING;
            AddAttachedParticle(mX + 40, mY + 40, mRenderOrder + 4, ParticleEffect::PARTICLE_GRAVE_BUSTER);
        }
    }
    else if (mState == PlantState::STATE_GRAVEBUSTER_EATING && mStateCountdown == 0)
    {
        GridItem* aGraveStone = mBoard->GetGraveStoneAt(mPlantCol, mRow);
        if (aGraveStone)
        {
            aGraveStone->GridItemDie();
            mBoard->mGravesCleared++;
        }

        mApp->AddTodParticle(mX + 40, mY + 40, mRenderOrder + 4, ParticleEffect::PARTICLE_GRAVE_BUSTER_DIE);
        Die();
        mBoard->DropLootPiece(mX + 40, mY, 12);
    }
}

// GOTY @Patoke: 0x463760
void Plant::PlayBodyReanim(const char* theTrackName, ReanimLoopType theLoopType, int theBlendTime, float theAnimRate)
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    if (theBlendTime > 0)
        aBodyReanim->StartBlend(theBlendTime);
    if (theAnimRate > 0.0f)
        aBodyReanim->mAnimRate = theAnimRate;

    aBodyReanim->mLoopType = theLoopType;
    aBodyReanim->mLoopCount = 0;
    aBodyReanim->SetFramesForLayer(theTrackName);
}

void Plant::UpdateHypnoShroom()
{
    if (mApp->mGameScene != GameScenes::SCENE_PLAYING)
        return;

    mHypnoSpawnCounter--;
    if (mHypnoSpawnCounter > 0)
        return;

    mHypnoSpawnCounter = 1500;  // 每 15 秒召唤一次

    Zombie* aZombie = mBoard->AddZombieInRow(ZombieType::ZOMBIE_RANDOM, mRow, -1);
    if (aZombie == nullptr)
        return;

    // 在自己所处格子生成一个被魅惑的随机僵尸
    aZombie->mPosX = mBoard->GridToPixelX(mPlantCol, mRow);
    aZombie->mPosY = mBoard->GridToPixelY(mPlantCol, mRow);
    aZombie->StartMindControlled();
}

void Plant::UpdatePotato()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == PlantState::STATE_NOTREADY)
    {
        if (mStateCountdown == 0)
        {
            mApp->AddTodParticle(mX + mWidth / 2, mY + mHeight / 2, mRenderOrder, ParticleEffect::PARTICLE_POTATO_MINE_RISE);
            PlayBodyReanim("anim_rise", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 18.0f);
            mState = PlantState::STATE_POTATO_RISING;
            mApp->PlayFoley(FoleyType::FOLEY_DIRT_RISE);
        }
    }
    else if (mState == PlantState::STATE_POTATO_RISING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            float aRate = RandRangeFloat(12.0f, 15.0f);
            PlayBodyReanim("anim_armed", ReanimLoopType::REANIM_LOOP, 0, aRate);

            Reanimation* aLightReanim = mApp->AddReanimation(0.0f, 0.0f, mRenderOrder + 2, GetPlantDefinition(mSeedType).mReanimationType);
            aLightReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
            aLightReanim->mAnimRate = aRate - 2.0f;
            aLightReanim->SetFramesForLayer("anim_glow");
            aLightReanim->mFrameCount = 10;
            aLightReanim->ShowOnlyTrack("anim_glow");
            aLightReanim->SetTruncateDisappearingFrames("anim_glow", false);
            mLightReanimID = mApp->ReanimationGetID(aLightReanim);
            aLightReanim->AttachToAnotherReanimation(aBodyReanim, "anim_light");

            mState = PlantState::STATE_POTATO_ARMED;
            mBlinkCountdown = 400 + Sexy::Rand(4000);
        }
    }
    else if (mState == PlantState::STATE_POTATO_ARMED)
    {
        if (FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY))
        {
            DoSpecial();
        }
        else
        {
            Reanimation* aLightReanim = mApp->ReanimationTryToGet(mLightReanimID);
            if (aLightReanim)
            {
                aLightReanim->mFrameCount = TodAnimateCurve(200, 50, DistanceToClosestZombie(), 10, 3, TodCurves::CURVE_LINEAR);
            }
        }
    }
}

void Plant::UpdateTanglekelp()
{
    if (mState != PlantState::STATE_TANGLEKELP_GRABBING)
    {
        Zombie* aZombie = FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY);
        if (aZombie)
        {
            mApp->PlayFoley(FoleyType::FOLEY_FLOOP);
            mState = PlantState::STATE_TANGLEKELP_GRABBING;
            mStateCountdown = 100;
            aZombie->PoolSplash(false);

            float aVinesPosX = -13.0f;
            float aVinesPosY = 15.0f;
            if (aZombie->mZombieType == ZombieType::ZOMBIE_SNORKEL)
            {
                aVinesPosX = -43.0f;
                aVinesPosY = 55.0f;
            }
            if (aZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_RIDING)
            {
                aVinesPosX = -20.0f;
                aVinesPosY = 37.0f;
            }
            Reanimation* aGrabReanim = aZombie->AddAttachedReanim(aVinesPosX, aVinesPosY, ReanimationType::REANIM_TANGLEKELP);
            if (aGrabReanim)
            {
                aGrabReanim->SetFramesForLayer("anim_grab");
                aGrabReanim->mAnimRate = 24.0f;
                aGrabReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
            }

            mTargetZombieID = mBoard->ZombieGetID(aZombie);
        }
    }
    else
    {
        if (mStateCountdown == 50)
        {
            Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
            if (aZombie)
            {
                aZombie->DragUnder();
                aZombie->PoolSplash(false);
            }
        }

        if (mStateCountdown == 20)
        {
            int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, mRow, 0);
            Reanimation* aSplashReanim = mApp->AddReanimation(mX - 23, mY + 7, aRenderPosition, ReanimationType::REANIM_SPLASH);
            aSplashReanim->OverrideScale(1.3f, 1.3f);

            mApp->AddTodParticle(mX + 31, mY + 64, aRenderPosition, ParticleEffect::PARTICLE_PLANTING_POOL);
            mApp->PlayFoley(FoleyType::FOLEY_ZOMBIE_ENTERING_WATER);
        }

        if (mStateCountdown == 0)
        {
            Die();

            Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
            if (aZombie)
            {
                aZombie->DieWithLoot();
            }
        }
    }
}

void Plant::SpikeweedAttack()
{
    TOD_ASSERT(IsSpiky());

    if (mState != PlantState::STATE_SPIKEWEED_ATTACKING)
    {
        PlayBodyReanim("anim_attack", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 18.0f);
        mApp->PlaySample(SOUND_THROW);
        
        mState = PlantState::STATE_SPIKEWEED_ATTACKING;
        mStateCountdown = 100;
    }
}

void Plant::UpdateSpikeweed()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    if (mSeedType == SeedType::SEED_SPIKEWEED || mSeedType == SeedType::SEED_SPIKEROCK)
    {
        mSpikeweedCooldown--;
        if (mSpikeweedCooldown <= 0)
        {
    mSpikeweedCooldown = 1800;
            int aPlantGridX = mBoard->PixelToGridXKeepOnBoard(static_cast<int>(mX), static_cast<int>(mY));
            Zombie* aZombie = nullptr;
            while (mBoard->IterateZombies(aZombie))
            {
                if (aZombie->mRow == mRow && !aZombie->mDead && !aZombie->mMindControlled && aZombie->mHasHead && !aZombie->IsBobsledTeamWithSled() && !aZombie->IsFlying() &&
                    aZombie->mZombiePhase != ZombiePhase::PHASE_POLEVAULTER_PRE_VAULT && aZombie->mZombiePhase != ZombiePhase::PHASE_POLEVAULTER_IN_VAULT)
                {
                    int aZombieGridX = mBoard->PixelToGridXKeepOnBoard(static_cast<int>(aZombie->mPosX), static_cast<int>(aZombie->mPosY));
                    if (std::abs(aZombieGridX - aPlantGridX) <= 3)
                    {
                        if (aZombie->mIsErrorZombie)
                        {
                            // 接触异常会感染地刺，但异常僵尸不会被吸附或定身。
                            if (mSeedType != SeedType::SEED_DOOMPEA && (mErrorCountdown == 0 || mErrorCountdown > 100))
                                mErrorCountdown = 100;
                            continue;
                        }
                        if (aZombie->mZombieType == ZombieType::ZOMBIE_DIGGER && aZombie->mHasObject)
                        {
                            aZombie->DiggerLoseAxe();
                        }
                        aZombie->mPullTargetX = mX;  // 吸到地刺身上，而不是前面
                        aZombie->mPullCounter = 30;
                        aZombie->mHeldCounter = 300;
                        aZombie->mButteredCounter = 300;
                        if (!aZombie->mIsMadDogZombie)
                            aZombie->mChilledCounter = 300;
                    }
                }
            }
            mApp->PlayFoley(FoleyType::FOLEY_PLANTGROW);
        }
    }

    if (mState == PlantState::STATE_SPIKEWEED_ATTACKING)
    {
        if (mStateCountdown == 0)
        {
            mState = PlantState::STATE_NOTREADY;
        }
        else if (mSeedType == SeedType::SEED_SPIKEROCK)
        {
            if (mStateCountdown == 69 || mStateCountdown == 33)
            {
                DoRowAreaDamage(20, 33U);
            }
        }
        else if (mStateCountdown == 75)
        {
            DoRowAreaDamage(20, 33U);
        }

        if (aBodyReanim->mLoopCount > 0)
        {
            PlayIdleAnim(RandRangeFloat(12.0f, 15.0f));
        }
    }
    else if (FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY))
    {
        SpikeweedAttack();
    }
}

void Plant::UpdateScaredyShroom()
{
    if (mShootingCounter > 0)
        return;

    bool aHasZombieNearby = false;

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        Rect aZombieRect = aZombie->GetZombieRect();
        int aDiffY = (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) ? 0 : (aZombie->mRow - mRow);
        if (!aZombie->mMindControlled && !aZombie->IsDeadOrDying() && aDiffY <= 1 && aDiffY >= -1 && GetCircleRectOverlap(mX, mY + 20.0f, 120, aZombieRect))
        {
            aHasZombieNearby = true;
            break;
        }
    }

    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == PlantState::STATE_READY)
    {
        if (aHasZombieNearby && mShootingCounter >= 10)
        {
            mState = PlantState::STATE_SCAREDYSHROOM_LOWERING;
            PlayBodyReanim("anim_scared", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, 10.0f);
        }
    }
    else if (mState == PlantState::STATE_SCAREDYSHROOM_LOWERING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = PlantState::STATE_SCAREDYSHROOM_SCARED;
            PlayBodyReanim("anim_scaredidle", ReanimLoopType::REANIM_LOOP, 10, 0.0f);
        }
    }
    else if (mState == PlantState::STATE_SCAREDYSHROOM_SCARED)
    {
        if (!aHasZombieNearby || mShootingCounter < 10)
        {
            mState = PlantState::STATE_SCAREDYSHROOM_RAISING;

            float aAnimRate = RandRangeFloat(7.0f, 12.0f);
            PlayBodyReanim("anim_grow", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, aAnimRate);
        }
    }
    else if (mState == PlantState::STATE_SCAREDYSHROOM_RAISING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = PlantState::STATE_READY;

            float aAnimRate = RandRangeFloat(10.0f, 15.0f);
            PlayIdleAnim(aAnimRate);
        }
    }

    if (mState != PlantState::STATE_READY)
    {
        mLaunchCounter = mLaunchRate;
    }
}

void Plant::UpdateTorchwood()
{
    Rect aAttackRect = GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);

    Projectile* aProjectile = nullptr;
    while (mBoard->IterateProjectiles(aProjectile))
    {
        if ((aProjectile->mRow == mRow) && 
			(aProjectile->mProjectileType == ProjectileType::PROJECTILE_PEA || aProjectile->mProjectileType == ProjectileType::PROJECTILE_IRONPEA || aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING || aProjectile->mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA || aProjectile->mProjectileType == ProjectileType::PROJECTILE_FIREBALL || aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || aProjectile->mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE))
        {
            Rect aProjectileRect = aProjectile->GetProjectileRect();
            if (GetRectOverlap(aAttackRect, aProjectileRect) >= 10)
            {
				if (aProjectile->mProjectileType == ProjectileType::PROJECTILE_IRONPEA)
				{
					aProjectile->mIronHeated = true;
					aProjectile->mHitTorchwoodGridX = mPlantCol;
				}
				else if (aProjectile->mProjectileType == ProjectileType::PROJECTILE_PEA || aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING)
                {
                    aProjectile->ConvertToIceFire(mPlantCol);
                }
                else if (aProjectile->mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE)
                {
                    aProjectile->ConvertToIceFire2(mPlantCol);
                }
                else if (aProjectile->mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA)
                {
                    aProjectile->ConvertToFireHypno(mPlantCol);
                }
                else
                {
                    aProjectile->ConvertToSuperFire(mPlantCol);
                }
            }
        }
    }
}

void Plant::DoSquashDamage()
{
    int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        if ((aZombie->mRow == mRow || aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            if (GetRectOverlap(aAttackRect, aZombieRect) > (aZombie->mZombieType == ZombieType::ZOMBIE_FOOTBALL ? -20 : 0))
            {
                aZombie->TakeDamage(GetAttackDamage(1800), 18U);
            }
        }
    }
}

Zombie* Plant::FindSquashTarget()
{
    int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);

    int aClosestRange = 0;
    Zombie* aClosestZombie = nullptr;

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        if ((aZombie->mRow == mRow || aZombie->mZombieType == ZombieType::ZOMBIE_BOSS) &&
            aZombie->mHasHead && !aZombie->IsTangleKelpTarget() && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();

            if ((
                    aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_PRE_VAULT && aZombieRect.mX < mX + 20
                ) || (
                    aZombie->mZombiePhase != ZombiePhase::PHASE_POLEVAULTER_PRE_VAULT && 
                    aZombie->mZombiePhase != ZombiePhase::PHASE_POLEVAULTER_IN_VAULT &&
                    aZombie->mZombiePhase != ZombiePhase::PHASE_SNORKEL_INTO_POOL && 
                    aZombie->mZombiePhase != ZombiePhase::PHASE_DOLPHIN_INTO_POOL &&
                    aZombie->mZombiePhase != ZombiePhase::PHASE_DOLPHIN_RIDING && 
                    aZombie->mZombiePhase != ZombiePhase::PHASE_DOLPHIN_IN_JUMP &&
                    !aZombie->IsBobsledTeamWithSled()
                ))
            {
                int aRange = -GetRectOverlap(aAttackRect, aZombieRect);
                if (aRange <= (aZombie->mIsEating ? 110 : 70))
                {
                    int aPlantX = aAttackRect.mX;
                    if (aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_POST_VAULT || aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_PRE_VAULT ||
                        aZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL || aZombie->mZombieType == ZombieType::ZOMBIE_IMP ||
                        aZombie->mZombieType == ZombieType::ZOMBIE_FOOTBALL || mApp->IsScaryPotterLevel())
                    {
                        aPlantX = aAttackRect.mX - 60;
                    }

                    if (aZombie->IsWalkingBackwards() || aZombieRect.mX + aZombieRect.mWidth >= aPlantX)
                    {
                        if (mBoard->ZombieGetID(aZombie) == mTargetZombieID)
                            return aZombie;  // 是锁定的目标僵尸，则直接返回该僵尸

                        if (aClosestZombie == nullptr || aRange < aClosestRange)
                        {
                            aClosestZombie = aZombie;
                            aClosestRange = aRange;
                        }
                    }
                }
            }
        }
    }

    return aClosestZombie;
}

void Plant::UpdateSquash()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    (void)aBodyReanim;
    TOD_ASSERT(aBodyReanim);

    if (mState == PlantState::STATE_NOTREADY)
    {
        Zombie* aZombie = FindSquashTarget();
        if (aZombie)
        {
            mTargetZombieID = mBoard->ZombieGetID(aZombie);
            mTargetX = aZombie->ZombieTargetLeadX(0.0f) - mWidth / 2;
            mState = PlantState::STATE_SQUASH_LOOK;
            mStateCountdown = 80;
            PlayBodyReanim(mTargetX < mX ? "anim_lookleft" : "anim_lookright", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 10, 24.0f);
            mApp->PlayFoley(FoleyType::FOLEY_SQUASH_HMM);
        }
    }
    else if (mState == PlantState::STATE_SQUASH_LOOK)
    {
        if (mStateCountdown <= 0)
        {
            Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
            if (!aZombie || aZombie->mDead || !aZombie->mHasHead)
            {
                aZombie = FindSquashTarget();
                if (!aZombie)
                {
                    mState = PlantState::STATE_NOTREADY;
                    return;
                }
                mTargetZombieID = mBoard->ZombieGetID(aZombie);
            }
            mTargetX = aZombie->ZombieTargetLeadX(30.0f) - mWidth / 2;
            PlayBodyReanim("anim_jumpup", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
            mState = PlantState::STATE_SQUASH_PRE_LAUNCH;
            mStateCountdown = 45;
        }
    }
    else if (mState == PlantState::STATE_SQUASH_PRE_LAUNCH)
    {
        Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
        if (!aZombie || aZombie->mDead || !aZombie->mHasHead)
        {
            aZombie = FindSquashTarget();
            if (!aZombie)
            {
                mState = PlantState::STATE_NOTREADY;
                return;
            }
            mTargetZombieID = mBoard->ZombieGetID(aZombie);
        }
        mTargetX = aZombie->ZombieTargetLeadX(30.0f) - mWidth / 2;

        if (mStateCountdown <= 0)
        {
            mState = PlantState::STATE_SQUASH_RISING;
            mStateCountdown = 50;
            mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, mRow, 0);
        }
    }
    else
    {
        Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
        if (!aZombie || aZombie->mDead || !aZombie->mHasHead)
        {
            aZombie = FindSquashTarget();
            if (!aZombie)
            {
                int aOrigX = mBoard->GridToPixelX(mPlantCol, mStartRow);
                int aOrigY = mBoard->GridToPixelY(mPlantCol, mStartRow);
                mX += (aOrigX - mX) * 0.1f;
                mY += (aOrigY - mY) * 0.1f;
                if (std::abs(mX - aOrigX) < 5.0f)
                {
                    mX = static_cast<float>(aOrigX);
                    mY = static_cast<float>(aOrigY);
                    mState = PlantState::STATE_NOTREADY;
                    mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PLANT, mRow, 0);
                    PlayIdleAnim(RandRangeFloat(10.0f, 15.0f));
                }
                return;
            }
            mTargetZombieID = mBoard->ZombieGetID(aZombie);
        }
        mTargetX = aZombie->ZombieTargetLeadX(30.0f) - mWidth / 2;
        int aTargetCol = mBoard->PixelToGridXKeepOnBoard(mTargetX, mY);
        int aDestY = mBoard->GridToPixelY(aTargetCol, mRow) + 8;

        if (mState == PlantState::STATE_SQUASH_RISING)
        {
            mX = TodAnimateCurve(50, 20, mStateCountdown, mBoard->GridToPixelX(mPlantCol, mStartRow), mTargetX, TodCurves::CURVE_EASE_IN_OUT);
            mY = TodAnimateCurve(50, 20, mStateCountdown, mBoard->GridToPixelY(mPlantCol, mStartRow), aDestY - 120, TodCurves::CURVE_EASE_IN_OUT);

            if (mStateCountdown == 0)
            {
                PlayBodyReanim("anim_jumpdown", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 60.0f);
                mState = PlantState::STATE_SQUASH_FALLING;
                mStateCountdown = 10;
            }
        }
        else if (mState == PlantState::STATE_SQUASH_FALLING)
        {
            mY = TodAnimateCurve(10, 0, mStateCountdown, aDestY - 120, aDestY, TodCurves::CURVE_EASE_IN_OUT);

            if (mStateCountdown == 5)
            {
                DoSquashDamage();
            }

            if (mStateCountdown == 0)
            {
                if (mBoard->IsPoolSquare(aTargetCol, mRow))
                {
                    mApp->AddReanimation(mX - 11, mY + 20, mRenderOrder + 1, ReanimationType::REANIM_SPLASH);
                    mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
                    mApp->PlaySample(SOUND_ZOMBIESPLASH);
                    Die();
                }
                else
                {
                    mState = PlantState::STATE_SQUASH_DONE_FALLING;
                    mStateCountdown = 100;
                    mBoard->ShakeBoard(1, 4);
                    mApp->PlayFoley(FoleyType::FOLEY_THUMP);
                    float aOffsetY = mBoard->StageHasRoof() ? 69.0f : 80.0f;
                    mApp->AddTodParticle(mX + 40, mY + aOffsetY, mRenderOrder + 4, ParticleEffect::PARTICLE_DUST_SQUASH);
                }
            }
        }
        else if (mState == PlantState::STATE_SQUASH_DONE_FALLING)
        {
            if (mStateCountdown == 0)
            {
                Die();
            }
        }
    }
}

void Plant::UpdateDoomShroom()
{
    if (mIsAsleep || mState == PlantState::STATE_DOINGSPECIAL)
        return;

    mState = PlantState::STATE_DOINGSPECIAL;
    mDoSpecialCountdown = 100;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    TOD_ASSERT(aBodyReanim);

    aBodyReanim->SetFramesForLayer("anim_explode");
    aBodyReanim->mAnimRate = 23.0f;
    aBodyReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;
    aBodyReanim->SetShakeOverride("DoomShroom_head1", 1.0f);
    aBodyReanim->SetShakeOverride("DoomShroom_head2", 2.0f);
    aBodyReanim->SetShakeOverride("DoomShroom_head3", 2.0f);
    mApp->PlayFoley(FoleyType::FOLEY_REVERSE_EXPLOSION);
}

void Plant::UpdateIceShroom()
{
    if (!mIsAsleep && mState != PlantState::STATE_DOINGSPECIAL)
    {
        mState = PlantState::STATE_DOINGSPECIAL;
        mDoSpecialCountdown = 100;
    }
}

void Plant::UpdateBlover()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (aBodyReanim->mLoopCount > 0 && aBodyReanim->mLoopType != ReanimLoopType::REANIM_LOOP)
    {
        aBodyReanim->SetFramesForLayer("anim_loop");
        aBodyReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
    }
	// A planted Blover keeps the fog pushed back for as long as it survives.
	mBoard->mFogBlownCountDown = std::max(mBoard->mFogBlownCountDown, 4000);
}

bool Plant::HasBloverBehind() const
{
	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (!aPlant->mDead && aPlant->mSeedType == SeedType::SEED_BLOVER &&
			aPlant->mRow == mRow && aPlant->mPlantCol > mPlantCol)
			return true;
	}
	return false;
}

bool Plant::HasBloverInRow() const
{
	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (!aPlant->mDead && aPlant->mSeedType == SeedType::SEED_BLOVER && aPlant->mRow == mRow)
			return true;
	}
	return false;
}

void Plant::UpdatePuffColony()
{
	if (!mApp->IsCricketMode() ||
		(mSeedType != SeedType::SEED_PUFFSHROOM && mSeedType != SeedType::SEED_SEASHROOM))
		return;
	if (mPuffColonyOffspring)
	{
		// 分株的逻辑行列始终与本体一致；每帧重定位，避免动画/载入旧存档
		// 把实体推入下一行。槽位只使用当前格子的安全区域。
		const int aCellX = mBoard->GridToPixelX(mPlantCol, mRow);
		const int aCellY = mBoard->GridToPixelY(mPlantCol, mRow);
		int aMemberIndex = 1;
		Plant* aMember = nullptr;
		while (mBoard->IteratePlants(aMember))
		{
			if (aMember == this)
				break;
			if (!aMember->mDead && aMember->mPuffColonyOffspring &&
				aMember->mSeedType == mSeedType && aMember->mPlantCol == mPlantCol && aMember->mRow == mRow)
				aMemberIndex++;
		}
		const int aSlotX = ((aMemberIndex - 1) % 3) * 22;
		const int aSlotY = ((aMemberIndex - 1) / 3) * 24 - 14;
		mWidth = 36;
		mHeight = 36;
		mX = static_cast<float>(std::clamp(aCellX + aSlotX, aCellX, aCellX + 80 - mWidth));
		mY = static_cast<float>(aCellY + std::clamp(aSlotY, -16, 8));
		return;
	}

	// 旧存档没有分株标记。用离格子原点最近的实体作为主株，重建同格菌落关系。
	const int aGridX = mBoard->GridToPixelX(mPlantCol, mRow);
	const int aGridY = mBoard->GridToPixelY(mPlantCol, mRow);
	Plant* aRoot = this;
	int aRootDistance = abs(mX - aGridX) + abs(mY - aGridY);
	Plant* aCandidate = nullptr;
	while (mBoard->IteratePlants(aCandidate))
	{
		if (aCandidate->mDead || aCandidate->mSeedType != mSeedType ||
			aCandidate->mPlantCol != mPlantCol || aCandidate->mRow != mRow)
			continue;
		const int aDistance = abs(aCandidate->mX - aGridX) + abs(aCandidate->mY - aGridY);
		if (aDistance < aRootDistance)
		{
			aRoot = aCandidate;
			aRootDistance = aDistance;
		}
	}
	if (aRoot != this)
	{
		mPuffColonyOffspring = true;
		return;
	}

	int aExistingMembers = 1;  // 主株本身
	while (mBoard->IteratePlants(aCandidate))
	{
		if (aCandidate->mDead || aCandidate->mSeedType != mSeedType ||
			aCandidate->mPlantCol != mPlantCol || aCandidate->mRow != mRow)
			continue;
		if (aCandidate == this)
			continue;
		if (aExistingMembers >= 6)
		{
			aCandidate->Die();
			continue;
		}
		aCandidate->mPuffColonyOffspring = true;
		aExistingMembers++;
	}
	mPuffColonyOffspring = false;
	mPuffColonyMembers = std::max(mPuffColonyMembers, aExistingMembers);

	const int aMembersFromKills = 1 + mBoard->mChallenge->CricketRowPlantKills(mRow) / 100;
	const int aTargetMembers = std::min(6, aMembersFromKills);
	// 主株居中；分株使用 2x3 网格，保证每个实体都有稳定且不同的位置。
	while (mPuffColonyMembers < aTargetMembers)
	{
		const int aMemberIndex = mPuffColonyMembers;
		Plant* aOffspring = mBoard->NewPlant(mPlantCol, mRow, mSeedType, SeedType::SEED_NONE);
		if (!aOffspring)
			break;
		aOffspring->mPuffColonyOffspring = true;
		aOffspring->mWidth = 36;
		aOffspring->mHeight = 36;
		// 分株始终限制在自己的草坪格内。使用紧凑的 3x2 槽位，纵向只在
		// 当前行基准附近错位，避免实体落入下一行。
		const int aCellX = mBoard->GridToPixelX(mPlantCol, mRow);
		const int aCellY = mBoard->GridToPixelY(mPlantCol, mRow);
		const int aSlotX = ((aMemberIndex - 1) % 3) * 22;
		const int aSlotY = ((aMemberIndex - 1) / 3) * 24 - 14;
		aOffspring->mX = static_cast<float>(std::clamp(
			aCellX + aSlotX, aCellX, aCellX + 80 - aOffspring->mWidth));
		aOffspring->mY = static_cast<float>(aCellY + std::clamp(aSlotY, -16, 8));
		aOffspring->mRenderOrder = aOffspring->CalcRenderOrder();
		if (IsNocturnal(aOffspring->mSeedType))
			aOffspring->SetSleeping(false);
		Reanimation* aOffspringReanim = mApp->ReanimationTryToGet(aOffspring->mBodyReanimID);
		if (aOffspringReanim)
			aOffspringReanim->OverrideScale(0.58f, 0.58f);
		mPuffColonyMembers++;
	}
}

void Plant::UpdateFlowerPot()
{
    if (mState == PlantState::STATE_FLOWERPOT_INVULNERABLE && mStateCountdown == 0)
        mState = PlantState::STATE_NOTREADY;
}

void Plant::UpdateLilypad()
{
    if (mState == PlantState::STATE_LILYPAD_INVULNERABLE && mStateCountdown == 0)
        mState = PlantState::STATE_NOTREADY;
}

void Plant::UpdateCoffeeBean()
{
    if (mState == PlantState::STATE_DOINGSPECIAL)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->mLoopCount > 0)
        {
            Die();
        }
    }
}

void Plant::UpdateUmbrella()
{
    if (mState == PlantState::STATE_UMBRELLA_TRIGGERED)
    {
        if (mStateCountdown == 0)
        {
            mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PROJECTILE, mRow + 1, 0);
            mState = PlantState::STATE_UMBRELLA_REFLECTING;
        }
    }
    else if (mState == PlantState::STATE_UMBRELLA_REFLECTING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayIdleAnim(0.0f);
            mState = PlantState::STATE_NOTREADY;
            mRenderOrder = CalcRenderOrder();
        }
    }
}

// GOTY @Patoke: 0x4649F0
void Plant::UpdateCobCannon()
{
    if (mState == PlantState::STATE_COBCANNON_ARMING)
    {
        if (mStateCountdown == 0)
        {
            mState = PlantState::STATE_COBCANNON_LOADING;
            PlayBodyReanim("anim_charge", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
        }
    }
    else if (mState == PlantState::STATE_COBCANNON_LOADING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->ShouldTriggerTimedEvent(0.5f))
        {
            mApp->PlayFoley(FoleyType::FOLEY_SHOOP);
        }
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = PlantState::STATE_COBCANNON_READY;
            PlayIdleAnim(12.0f);
        }
    }
    else if (mState == PlantState::STATE_COBCANNON_READY)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        ReanimatorTrackInstance* aCobTrack = aBodyReanim->GetTrackInstanceByName("CobCannon_cob");
        aCobTrack->mTrackColor = GetFlashingColor(mBoard->mMainCounter, 75);
    }
    else if (mState == PlantState::STATE_COBCANNON_FIRING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->ShouldTriggerTimedEvent(0.48f))
        {
            mApp->PlayFoley(FoleyType::FOLEY_COB_LAUNCH);
        }
    }
}

void Plant::UpdateCactus()
{
    if (mShootingCounter > 0)
        return;

    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    if (mState == PlantState::STATE_CACTUS_RISING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = PlantState::STATE_CACTUS_HIGH;
            PlayBodyReanim("anim_idlehigh", ReanimLoopType::REANIM_LOOP, 20, 0.0f);
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim->mAnimRate = 0;
            }

            mLaunchCounter = 1;
        }
    }
    else if (mState == PlantState::STATE_CACTUS_HIGH)
    {
        if (FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY) == nullptr)
        {
            mState = PlantState::STATE_CACTUS_LOWERING;
            PlayBodyReanim("anim_lower", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, aBodyReanim->mDefinition->mFPS);
        }
    }
    else if (mState == PlantState::STATE_CACTUS_LOWERING)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = PlantState::STATE_CACTUS_LOW;
            PlayIdleAnim(0.0f);
        }
    }
    else if (FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY))
    {
        mState = PlantState::STATE_CACTUS_RISING;
        PlayBodyReanim("anim_rise", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, aBodyReanim->mDefinition->mFPS);
        mApp->PlayFoley(FoleyType::FOLEY_PLANTGROW);
    }
}

void Plant::UpdateChomper()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (mState == PlantState::STATE_READY)
    {
        if (FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY))
        {
            PlayBodyReanim("anim_bite", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 24.0f);
            mState = PlantState::STATE_CHOMPER_BITING;
            mStateCountdown = 70;
        }
    }
    else if (mState == PlantState::STATE_CHOMPER_BITING)
    {
        if (mStateCountdown == 0)
        {
            mApp->PlayFoley(FoleyType::FOLEY_BIGCHOMP);

            int aEaten = 0;
            int aTotalHeal = 0;
            bool aBitBerserk = false;
            for (int aBite = 0; aBite < 5; aBite++)
            {
                Zombie* aZombie = FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY);
                if (!aZombie)
                    break;

                bool isGarg = aZombie->mZombieType == ZombieType::ZOMBIE_GARGANTUAR || 
                              aZombie->mZombieType == ZombieType::ZOMBIE_REDEYE_GARGANTUAR || 
                              aZombie->mZombieType == ZombieType::ZOMBIE_BOSS;

                // 困难模式下狂暴僵尸不能被一口吃掉：按巨人逻辑，但每口只造成 20 点伤害
                bool isBerserkHard = aZombie->mBerserk && mApp && mApp->IsHardMode();

                if (isGarg || isBerserkHard)
                {
                    mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
                    if (isBerserkHard)
                    {
                        aBitBerserk = true;
                        // 概率秒杀：第一口 1%，之后每口 +2%（3%、5%、7%...），成功或咬空后重置
                        if (Rand(100) < mChomperBiteStreak * 2 + 1)
                        {
                            aZombie->mSuppressCricketTransform = true;
                            if (aZombie->mIsErrorZombie && mSeedType != SeedType::SEED_DOOMPEA && (mErrorCountdown == 0 || mErrorCountdown > 100))
                                mErrorCountdown = 100; // One second; never extend an existing shorter countdown.
                            aZombie->DieWithLoot();
                            mChomperBiteStreak = 0;
                            mChomperEatenCount++;
                            mPlantHealth += 500;
                            aTotalHeal += 500;
                            aEaten++;
                        }
                        else
                        {
                            mChomperBiteStreak++;
                            int aDamage = 20;
                            if (mApp->IsCricketMode())
                                aDamage = aDamage * mBoard->mChallenge->CricketDamageMultiplierPercent(mRow) / 100;
                            aZombie->TakeDamage(GetAttackDamage(aDamage), 0U);
                        }
                    }
                    else
                    {
                        int aDamage = 40;
                        if (mApp->IsCricketMode())
                            aDamage = aDamage * mBoard->mChallenge->CricketDamageMultiplierPercent(mRow) / 100;
                        aZombie->TakeDamage(GetAttackDamage(aDamage), 0U);
                    }
                    mChomperBonusChance += 2;
                }
                else
                {
                    // 斗蛐蛐：大嘴花直接吃掉随机僵尸，不触发变身
                    mChomperBiteStreak = 0;
                    aZombie->mSuppressCricketTransform = true;
                    if (aZombie->mIsErrorZombie && mSeedType != SeedType::SEED_DOOMPEA && (mErrorCountdown == 0 || mErrorCountdown > 100))
                        mErrorCountdown = 100;
                    aZombie->DieWithLoot();
                    mChomperBonusChance = 0;
                    mChomperEatenCount++;
                    mPlantHealth += 500;
                    aTotalHeal += 500;
                    aEaten++;
                }
            }

            // 整口没咬到狂暴僵尸则连击重置
            if (aEaten == 0 && !aBitBerserk)
                mChomperBiteStreak = 0;

            if (aTotalHeal > 800)
            {
                int aExcess = aTotalHeal - 800;
                Plant* aPlants[9];
                int aPlantCount = 0;
                Plant* aIter = nullptr;
                while (mBoard->IteratePlants(aIter))
                {
                    if (aIter->mRow == mRow && !aIter->mDead && aPlantCount < 9)
                        aPlants[aPlantCount++] = aIter;
                }

                if (aPlantCount > 0)
                {
                    for (int i = 0; i < aExcess; i++)
                    {
                        int aBest = 0;
                        for (int j = 1; j < aPlantCount; j++)
                        {
                            if ((int64_t)aPlants[j]->mPlantHealth * aPlants[aBest]->mPlantMaxHealth <
                                (int64_t)aPlants[aBest]->mPlantHealth * aPlants[j]->mPlantMaxHealth)
                            {
                                aBest = j;
                            }
                        }
                        aPlants[aBest]->mPlantHealth++;
                    }
                }
            }

            mState = aEaten > 0 ? PlantState::STATE_CHOMPER_BITING_GOT_ONE : PlantState::STATE_CHOMPER_BITING_MISSED;
        }
    }
    else if (mState == PlantState::STATE_CHOMPER_BITING_GOT_ONE)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_chew", ReanimLoopType::REANIM_LOOP, 0, 15.0f);
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim->mAnimRate = 0;
            }

            mState = PlantState::STATE_CHOMPER_DIGESTING;
            mStateCountdown = std::max(120, 4000 / (mChomperEatenCount + 1));
        }
    }
    else if (mState == PlantState::STATE_CHOMPER_DIGESTING)
    {
        if (mStateCountdown == 0)
        {
            PlayBodyReanim("anim_swallow", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
            mState = PlantState::STATE_CHOMPER_SWALLOWING;
        }
    }
    else if ((mState == PlantState::STATE_CHOMPER_SWALLOWING || mState == PlantState::STATE_CHOMPER_BITING_MISSED) && aBodyReanim->mLoopCount > 0)
    {
        PlayIdleAnim(aBodyReanim->mDefinition->mFPS);
        mState = PlantState::STATE_READY;
    }
}

MagnetItem* Plant::GetFreeMagnetItem()
{
    if (mSeedType == SeedType::SEED_GOLD_MAGNET)
    {
        for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
        {
            if (mMagnetItems[i].mItemType == MagnetItemType::MAGNET_ITEM_NONE)
            {
                return &mMagnetItems[i];
            }
        }

        return nullptr;
    }

    return &mMagnetItems[0];
}

void Plant::MagnetShroomAttactItem(Zombie* theZombie)
{
    mState = PlantState::STATE_MAGNETSHROOM_SUCKING;
    mStateCountdown = 1500;
    PlayBodyReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
    mApp->PlayFoley(FoleyType::FOLEY_MAGNETSHROOM);

    int aArmorHealth = 0;  // 吸到的防具剩余血量（平分给本行植物）

    MagnetItem* aMagnetItem = GetFreeMagnetItem();
    if (theZombie->mHelmType == HelmType::HELMTYPE_PAIL)
    {
        int aDamageIndex = theZombie->GetHelmDamageIndex();

        aArmorHealth = theZombie->mHelmHealth;
        theZombie->mHelmHealth = 0;
        theZombie->mHelmType = HelmType::HELMTYPE_NONE;
        theZombie->GetTrackPosition("anim_bucket", aMagnetItem->mPosX, aMagnetItem->mPosY);
        theZombie->ReanimShowPrefix("anim_bucket", RENDER_GROUP_HIDDEN);
        theZombie->ReanimShowPrefix("anim_hair", RENDER_GROUP_NORMAL);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_BUCKET1->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_BUCKET1->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 25.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MagnetItemType::MAGNET_ITEM_PAIL_1) + aDamageIndex);
    }
    else if (theZombie->mHelmType == HelmType::HELMTYPE_FOOTBALL)
    {
        int aDamageIndex = theZombie->GetHelmDamageIndex();

        aArmorHealth = theZombie->mHelmHealth;
        theZombie->mHelmHealth = 0;
        theZombie->mHelmType = HelmType::HELMTYPE_NONE;
        theZombie->GetTrackPosition("zombie_football_helmet", aMagnetItem->mPosX, aMagnetItem->mPosY);
        theZombie->ReanimShowPrefix("zombie_football_helmet", RENDER_GROUP_HIDDEN);
        theZombie->ReanimShowPrefix("anim_hair", RENDER_GROUP_NORMAL);

        aMagnetItem->mPosX += 37.0f;
        aMagnetItem->mPosY -= 60.0f;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MagnetItemType::MAGNET_ITEM_FOOTBALL_HELMET_1) + aDamageIndex);
    }
    else if (theZombie->mShieldType == ShieldType::SHIELDTYPE_DOOR)
    {
        int aDamageIndex = theZombie->GetShieldDamageIndex();

        aArmorHealth = theZombie->mShieldHealth;
        theZombie->DetachShield();
        theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
        if (!theZombie->mIsEating)
        {
            TOD_ASSERT(theZombie->mZombieHeight == ZombieHeight::HEIGHT_ZOMBIE_NORMAL);
            theZombie->StartWalkAnim(0);
        }
        theZombie->GetTrackPosition("anim_screendoor", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_SCREENDOOR1->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_SCREENDOOR1->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 30.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MagnetItemType::MAGNET_ITEM_DOOR_1) + aDamageIndex);
    }
    else if (theZombie->mShieldType == ShieldType::SHIELDTYPE_LADDER)
    {
        int aDamageIndex = theZombie->GetShieldDamageIndex();

        aArmorHealth = theZombie->mShieldHealth;
        theZombie->DetachShield();

        aMagnetItem->mPosX = theZombie->mPosX + 31.0f;
        aMagnetItem->mPosY = theZombie->mPosY + 20.0f;
        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_LADDER_5->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_LADDER_5->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 30.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
        aMagnetItem->mItemType = static_cast<MagnetItemType>(static_cast<int>(MagnetItemType::MAGNET_ITEM_LADDER_1) + aDamageIndex);
    }
    else if (theZombie->mZombieType == ZombieType::ZOMBIE_POGO)
    {
        theZombie->PogoBreak(16U);
        // ZombieDrawPosition aDrawPos;
        // theZombie->GetDrawPos(aDrawPos);
        theZombie->GetTrackPosition("Zombie_pogo_stick", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX += 40.0f - IMAGE_REANIM_ZOMBIE_LADDER_5->GetWidth() / 2;
        aMagnetItem->mPosY += 84.0f - IMAGE_REANIM_ZOMBIE_LADDER_5->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 30.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
        aMagnetItem->mItemType = theZombie->mHasArm ? MagnetItemType::MAGNET_ITEM_POGO_1 : MagnetItemType::MAGNET_ITEM_POGO_3;
    }
    else if (theZombie->mZombiePhase == ZombiePhase::PHASE_JACK_IN_THE_BOX_RUNNING)
    {
        theZombie->StopZombieSound();
        theZombie->PickRandomSpeed();
        theZombie->mZombiePhase = ZombiePhase::PHASE_ZOMBIE_NORMAL;
        theZombie->ReanimShowPrefix("Zombie_jackbox_box", RENDER_GROUP_HIDDEN);
        theZombie->ReanimShowPrefix("Zombie_jackbox_handle", RENDER_GROUP_HIDDEN);
        theZombie->GetTrackPosition("Zombie_jackbox_box", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_JACKBOX_BOX->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_JACKBOX_BOX->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 20.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 15.0f;
        aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_JACK_IN_THE_BOX;
    }
    else if (theZombie->mZombieType == ZombieType::ZOMBIE_DIGGER)
    {
        theZombie->DiggerLoseAxe();
        theZombie->GetTrackPosition("Zombie_digger_pickaxe", aMagnetItem->mPosX, aMagnetItem->mPosY);

        aMagnetItem->mPosX -= IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE->GetWidth() / 2;
        aMagnetItem->mPosY -= IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE->GetHeight() / 2;
        aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 45.0f;
        aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f) + 15.0f;
        aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_PICK_AXE;
    }

    // 非防具装备（跳跳杆/镐/小丑盒等）没有剩余血量，按 1000 血算
    if (aArmorHealth <= 0)
    {
        aArmorHealth = 1000;
    }

    // 吸到装备后：把防具剩余血量平分给本行所有植物（可突破血量上限）
    if (aArmorHealth > 0)
    {
        int aPlantCount = 0;
        Plant* aPlant = nullptr;
        while (mBoard->IteratePlants(aPlant))
        {
            if (aPlant->mRow == mRow && aPlant->IsInPlay() && !aPlant->NotOnGround())
                aPlantCount++;
        }
        if (aPlantCount > 0)
        {
            int aHealPerPlant = aArmorHealth / aPlantCount;
            if (aHealPerPlant > 0)
            {
                aPlant = nullptr;
                while (mBoard->IteratePlants(aPlant))
                {
                    if (aPlant->mRow == mRow && aPlant->IsInPlay() && !aPlant->NotOnGround())
                    {
                        aPlant->mPlantHealth += aHealPerPlant;
                    }
                }
            }
        }
    }
}

// GOTY @Patoke: 0x4656B0
bool Plant::DrawMagnetItemsOnTop()
{
    if (mSeedType == SeedType::SEED_GOLD_MAGNET)
    {
        for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
        {
            if (mMagnetItems[i].mItemType != MagnetItemType::MAGNET_ITEM_NONE)
            {
                return true;
            }
        }

        return false;
    }

    if (mSeedType == SeedType::SEED_MAGNETSHROOM)
    {
        for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
        {
            MagnetItem* aMagnetItem = &mMagnetItems[i];
            if (aMagnetItem->mItemType != MagnetItemType::MAGNET_ITEM_NONE)
            {
                SexyVector2 aVectorToPlant(mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX, mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
                if (aVectorToPlant.Magnitude() > 20.0f)
                {
                    return true;
                }
            }
        }

        return false;
    }

    return false;
}

void Plant::UpdateMagnetShroom()
{
	// Magnet shrooms also attract iron peas fired on their row.
	const PlantID aMagnetID = static_cast<PlantID>(mBoard->mPlants.DataArrayGetID(this));
	bool aCanAbsorb = true;
	Plant* aRowMagnet = nullptr;
	while (mBoard->IteratePlants(aRowMagnet))
	{
		if (aRowMagnet != this && aRowMagnet->mSeedType == SeedType::SEED_MAGNETSHROOM &&
			aRowMagnet->mPlantHealth > 0 && aRowMagnet->mRow == mRow &&
			aRowMagnet->mPlantCol < mPlantCol)
		{
			aCanAbsorb = false;
			break;
		}
	}
	if (aCanAbsorb)
	{
		Projectile* aProjectile = nullptr;
		while (mBoard->IterateProjectiles(aProjectile))
		{
			if (aProjectile->mProjectileType != ProjectileType::PROJECTILE_IRONPEA ||
				aProjectile->mIronMagnetPlantID != PlantID::PLANTID_NULL || aProjectile->mIronOrbitTicks != 0 || aProjectile->mRow != mRow)
				continue;
			float aDX = aProjectile->mPosX - (mX + 40.0f);
			float aDY = aProjectile->mPosY - (mY + 40.0f);
			if (aDX * aDX + aDY * aDY <= 160.0f * 160.0f)
			{
				aProjectile->mIronMagnetPlantID = aMagnetID;
				aProjectile->mIronOrbitAngle = atan2f(aDY / 34.0f, aDX / 52.0f);
				aProjectile->mIronSpeed = std::max(20.0f, std::min(33.3f, aProjectile->mIronSpeed));
			}
		}
	}

    for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
    {
        MagnetItem* aMagnetItem = &mMagnetItems[i];
        if (aMagnetItem->mItemType != MagnetItemType::MAGNET_ITEM_NONE)
        {
            SexyVector2 aVectorToPlant(mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX, mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
            if (aVectorToPlant.Magnitude() > 20.0f)
            {
                aMagnetItem->mPosX += aVectorToPlant.x * 0.05f;
                aMagnetItem->mPosY += aVectorToPlant.y * 0.05f;
            }
        }
    }

	// Only the primary (leftmost) magnet in a row performs absorption.
	// Secondary magnets still contribute to iron-pea orbit stacking.
	if (!aCanAbsorb)
	{
		if (mState != PlantState::STATE_READY)
			mState = PlantState::STATE_READY;
		return;
	}

    if (mState == PlantState::STATE_MAGNETSHROOM_CHARGING)
    {
        if (mStateCountdown == 0)
        {
            mState = PlantState::STATE_READY;

            float aAnimRate = RandRangeFloat(10.0f, 15.0f);
            PlayBodyReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 30, aAnimRate);
            if (mApp->IsIZombieLevel())
            {
                Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
                aBodyReanim->mAnimRate = 0.0f;
            }

            mMagnetItems[0].mItemType = MagnetItemType::MAGNET_ITEM_NONE;
        }
    }
    else if (mState == PlantState::STATE_MAGNETSHROOM_SUCKING)
    {
        Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_nonactive_idle2", ReanimLoopType::REANIM_LOOP, 20, 2.0f);
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
                aBodyReanim->mAnimRate = 0.0f;
            }

            mState = PlantState::STATE_MAGNETSHROOM_CHARGING;
        }
    }
    else
    {
        float aClosestDistance = 0.0f;
        Zombie* aClosestZombie = nullptr;

        Zombie* aZombie = nullptr;
        while (mBoard->IterateZombies(aZombie))
        {
            int aDiffY = aZombie->mRow - mRow;
            Rect aZombieRect = aZombie->GetZombieRect();

            if (aZombie->mMindControlled)
                continue;

            if (!aZombie->mHasHead)
                continue;

            if (aZombie->mZombieHeight != ZombieHeight::HEIGHT_ZOMBIE_NORMAL || aZombie->mZombiePhase == ZombiePhase::PHASE_RISING_FROM_GRAVE)
                continue;

            if (aZombie->IsDeadOrDying())
                continue;

            if (aZombieRect.mX > BOARD_WIDTH || aDiffY > 2 || aDiffY < -2)
                continue;

            if (aZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_TUNNELING ||
                aZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_STUNNED ||
                aZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_WALKING ||
                aZombie->mZombieType == ZombieType::ZOMBIE_POGO)
            {
                if (!aZombie->mHasObject)
                    continue;
            }
            else if (!(aZombie->mHelmType == HelmType::HELMTYPE_PAIL ||
                aZombie->mHelmType == HelmType::HELMTYPE_FOOTBALL ||
                aZombie->mShieldType == ShieldType::SHIELDTYPE_DOOR ||
                aZombie->mShieldType == ShieldType::SHIELDTYPE_LADDER ||
                aZombie->mZombiePhase == ZombiePhase::PHASE_JACK_IN_THE_BOX_RUNNING))
                continue;

            int aRadius = aZombie->mIsEating ? 320 : 270;
            if (GetCircleRectOverlap(mX, mY + 20, aRadius, aZombieRect))
            {
                float aDistance = Distance2D(mX, mY, aZombieRect.mX, aZombieRect.mY);
                aDistance += abs(aDiffY) * 80.0f;

                if (aClosestZombie == nullptr || aDistance < aClosestDistance)
                {
                    aClosestZombie = aZombie;
                    aClosestDistance = aDistance;
                }
            }
        }

        if (aClosestZombie)
        {
            MagnetShroomAttactItem(aClosestZombie);
            return;
        }

        ////////////////////

        float aClosestLadderDist = 0.0f;
        GridItem* aClosestLadder = nullptr;
        
        GridItem* aGridItem = nullptr;
        while (mBoard->IterateGridItems(aGridItem))
        {
            if (aGridItem->mGridItemType == GridItemType::GRIDITEM_LADDER)
            {
                int aDiffX = abs(aGridItem->mGridX - mPlantCol);
                int aDiffY = abs(aGridItem->mGridY - mRow);
                int aSquareDistance = std::max(aDiffX, aDiffY);
                if (aSquareDistance <= 2)
                {
                    float aDistance = aSquareDistance + aDiffY * 0.05f;
                    if (aClosestLadder == nullptr || aDistance < aClosestLadderDist)
                    {
                        aClosestLadder = aGridItem;
                        aClosestLadderDist = aDistance;
                    }
                }
            }
        }

        if (aClosestLadder)
        {
            mState = PlantState::STATE_MAGNETSHROOM_SUCKING;
            mStateCountdown = 1500;
            PlayBodyReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
            mApp->PlayFoley(FoleyType::FOLEY_MAGNETSHROOM);

            aClosestLadder->GridItemDie();

            MagnetItem* aMagnetItem = GetFreeMagnetItem();
            aMagnetItem->mPosX = mBoard->GridToPixelX(aClosestLadder->mGridX, aClosestLadder->mGridY) + 40;
            aMagnetItem->mPosY = mBoard->GridToPixelY(aClosestLadder->mGridX, aClosestLadder->mGridY);
            aMagnetItem->mDestOffsetX = RandRangeFloat(-10.0f, 10.0f) + 10.0f;
            aMagnetItem->mDestOffsetY = RandRangeFloat(-10.0f, 10.0f);
            aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_LADDER_PLACED;
        }
    }
}

Coin* Plant::FindGoldMagnetTarget()
{
    Coin* aClosestCoin = nullptr;
    float aClosestDistance = 0.0f;

    Coin* aCoin = nullptr;
    while (mBoard->IterateCoins(aCoin))
    {
        if (aCoin->IsMoney() && aCoin->mCoinMotion != CoinMotion::COIN_MOTION_FROM_PRESENT && !aCoin->mIsBeingCollected && aCoin->mCoinAge >= 50)
        {
            float aDistance = Distance2D(mX + mWidth / 2, mY + mHeight / 2, aCoin->mPosX + aCoin->mWidth / 2, aCoin->mPosY + aCoin->mHeight / 2);
            if (aClosestCoin == nullptr || aDistance < aClosestDistance)
            {
                aClosestCoin = aCoin;
                aClosestDistance = aDistance;
            }
        }
    }

    return aClosestCoin;
}

void Plant::GoldMagnetFindTargets()
{
    if (GetFreeMagnetItem() == nullptr)
    {
        TOD_ASSERT(false);
        return;
    }

    for (;;)
    {
        MagnetItem* aMagnetItem = GetFreeMagnetItem();
        if (aMagnetItem == nullptr)
            break;

        Coin* aCoin = FindGoldMagnetTarget();
        if (aCoin == nullptr)
            break;

        aMagnetItem->mPosX = aCoin->mPosX + 15.0f;
        aMagnetItem->mPosY = aCoin->mPosY + 15.0f;
        aMagnetItem->mDestOffsetX = RandRangeFloat(20.0f, 40.0f);
        aMagnetItem->mDestOffsetY = RandRangeFloat(-20.0f, 0.0f) + 20.0f;

        switch (aCoin->mType)
        {
        case CoinType::COIN_SILVER:     aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_SILVER_COIN;   break;
        case CoinType::COIN_GOLD:       aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_GOLD_COIN;     break;
        case CoinType::COIN_DIAMOND:    aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_DIAMOND;       break;
        default:                        TOD_ASSERT(false);                                                       return;
        }
        
        aCoin->Die();
    }
}

bool Plant::IsAGoldMagnetAboutToSuck()
{
    Plant* aPlant = nullptr;
    while (mBoard->IteratePlants(aPlant))
    {
        if (!aPlant->NotOnGround() && aPlant->mSeedType == SeedType::SEED_GOLD_MAGNET && aPlant->mState == PlantState::STATE_MAGNETSHROOM_SUCKING)
        {
            Reanimation* aBodyReanim = mApp->ReanimationGet(aPlant->mBodyReanimID);
            if (aBodyReanim->mAnimTime < 0.5f)
            {
                return true;
            }
        }
    }

    return false;
}

void Plant::UpdateGoldMagnetShroom()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);

    bool aIsSuckingCoin = false;
    for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
    {
        MagnetItem* aMagnetItem = &mMagnetItems[i];
        if (aMagnetItem->mItemType != MagnetItemType::MAGNET_ITEM_NONE)
        {
            SexyVector2 aVectorToPlant(mX + aMagnetItem->mDestOffsetX - aMagnetItem->mPosX, mY + aMagnetItem->mDestOffsetY - aMagnetItem->mPosY);
            float aDistance = aVectorToPlant.Magnitude();
            if (aDistance < 20.0f)
            {
                CoinType aCoinType;
                switch (aMagnetItem->mItemType)
                {
                case MagnetItemType::MAGNET_ITEM_SILVER_COIN:   aCoinType = CoinType::COIN_SILVER;      break;
                case MagnetItemType::MAGNET_ITEM_GOLD_COIN:     aCoinType = CoinType::COIN_GOLD;        break;
                case MagnetItemType::MAGNET_ITEM_DIAMOND:       aCoinType = CoinType::COIN_DIAMOND;     break;
                default:                                        TOD_ASSERT(false);                           return;
                }

                int aValue = Coin::GetCoinValue(aCoinType);
                mApp->mPlayerInfo->AddCoins(aValue);
                mBoard->mCoinsCollected += aValue;
                mApp->PlayFoley(FoleyType::FOLEY_COIN);

                aMagnetItem->mItemType = MagnetItemType::MAGNET_ITEM_NONE;
            }
            else
            {
                float aSpeed = TodAnimateCurveFloatTime(30.0f, 0.0f, aDistance, 0.02f, 0.05f, TodCurves::CURVE_LINEAR);
                aMagnetItem->mPosX += aVectorToPlant.x * aSpeed;
                aMagnetItem->mPosY += aVectorToPlant.y * aSpeed;

                aIsSuckingCoin = true;
            }
        }
    }

    if (mState == PlantState::STATE_MAGNETSHROOM_CHARGING)
    {
        if (mStateCountdown == 0)
        {
            mState = PlantState::STATE_READY;
        }
    }
    else if (mState == PlantState::STATE_MAGNETSHROOM_SUCKING)
    {
        if (aBodyReanim->ShouldTriggerTimedEvent(0.4f))
        {
            mApp->PlayFoley(FoleyType::FOLEY_MAGNETSHROOM);
            GoldMagnetFindTargets();
        }

        if (aBodyReanim->mLoopCount > 0 && !aIsSuckingCoin)
        {
            PlayIdleAnim(14.0f);
            mState = PlantState::STATE_MAGNETSHROOM_CHARGING;
            mStateCountdown = RandRangeInt(200, 300);
        }
    }
    else if (!IsAGoldMagnetAboutToSuck() && Sexy::Rand(50) == 0 && FindGoldMagnetTarget())
    {
        mBoard->ShowCoinBank();
        mState = PlantState::STATE_MAGNETSHROOM_SUCKING;
        PlayBodyReanim("anim_attract", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);
    }
}

void Plant::RemoveEffects()
{
    mApp->RemoveParticle(mParticleID);
    mApp->RemoveReanimation(mBodyReanimID);
    mApp->RemoveReanimation(mHeadReanimID);
    mApp->RemoveReanimation(mHeadReanimID2);
    mApp->RemoveReanimation(mHeadReanimID3);
    mApp->RemoveReanimation(mLightReanimID);
    mApp->RemoveReanimation(mBlinkReanimID);
    mApp->RemoveReanimation(mSleepingReanimID);
}

void Plant::ShieldWallnutTakeDamage(int theDamage)
{
    if (mDead || mSeedType != SeedType::SEED_SHIELDWALLNUT || theDamage <= 0)
        return;
    mPlantHealth -= theDamage;
    mEatenFlashCountdown = std::max(mEatenFlashCountdown, 25);
    if (mPlantHealth <= 0)
        Die();
}

bool Plant::AbsorbDamageByShield(int theDamage)
{
    if (!mBoard || mSeedType == SeedType::SEED_SHIELDWALLNUT)
        return false;
    Plant* aPlant = nullptr;
    while (mBoard->IteratePlants(aPlant))
    {
        if (aPlant != this && !aPlant->mDead && aPlant->mSeedType == SeedType::SEED_SHIELDWALLNUT && aPlant->mRow == mRow)
        {
            aPlant->mPlantHealth -= theDamage;
            aPlant->mEatenFlashCountdown = std::max(aPlant->mEatenFlashCountdown, 25);
            if (aPlant->mPlantHealth <= 0)
                aPlant->Die();
            return true;
        }
    }
    return false;
}

void Plant::Squish(int theDamage)
{
    if (mSeedType == SeedType::SEED_SHIELDWALLNUT)
        return;
    if (AbsorbDamageByShield(theDamage))
        return;
    if (NotOnGround())
        return;
	if (mApp->IsCricketMode() && mBoard->mChallenge->CricketAbsorbRowShield(mRow, 500))
		return;

    if (!mIsAsleep)
    {
        if (mSeedType == SeedType::SEED_CHERRYBOMB || mSeedType == SeedType::SEED_JALAPENO ||
            mSeedType == SeedType::SEED_DOOMSHROOM || mSeedType == SeedType::SEED_ICESHROOM)
        {
            DoSpecial();
            return;
        }
        else if (mSeedType == SeedType::SEED_POTATOMINE && mState != PlantState::STATE_NOTREADY)
        {
            DoSpecial();
            return;
        }
    }

    if (mSeedType == SeedType::SEED_SQUASH && mState != PlantState::STATE_NOTREADY)
        return;

    if (mSeedType == SeedType::SEED_CHOMPER)
    {
        mPlantHealth -= 250;
        mEatenFlashCountdown = std::max(mEatenFlashCountdown, 50);
        if (mPlantHealth <= 0)
        {
            mBoard->mPlantsEaten++;
            mApp->PlayFoley(FoleyType::FOLEY_SQUISH);
            Die();
        }
        return;
    }

    if (mSeedType == SeedType::SEED_TALLNUT)
    {
        mPlantHealth -= 500;
        mEatenFlashCountdown = std::max(mEatenFlashCountdown, 50);
        if (mPlantHealth <= 0)
        {
            mBoard->mPlantsEaten++;
            mApp->PlayFoley(FoleyType::FOLEY_SQUISH);
            Die();
        }
        return;
    }

    mBoard->mPlantsEaten++;
    mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_GRAVE_STONE, mRow, 8);
    mSquished = true;
    mDisappearCountdown = 500;
    mApp->PlayFoley(FoleyType::FOLEY_SQUISH);
    RemoveEffects();

    GridItem* aLadder = mBoard->GetLadderAt(mPlantCol, mRow);
    if (aLadder)
    {
        aLadder->GridItemDie();
    }

    if (mApp->IsIZombieLevel())
    {
        mBoard->mChallenge->IZombiePlantDropRemainingSun(this);
    }
}

// GOTY @Patoke: 0x4666E0
void Plant::UpdateBowling()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim && aBodyReanim->TrackExists("_ground"))
    {
        float aSpeed = aBodyReanim->GetTrackVelocity("_ground");
        if (mSeedType == SeedType::SEED_GIANT_WALLNUT)
        {
            aSpeed *= 2;
        }

        mX -= aSpeed;
        if (mX > 800)
            Die();
    }

    if (mState == PlantState::STATE_BOWLING_UP)
    {
        mY -= 2;
    }
    else if (mState == PlantState::STATE_BOWLING_DOWN)
    {
        mY += 2;
    }
    int aDistToGrid = mBoard->GridToPixelY(0, mRow) - mY;
    if (aDistToGrid < -2 || aDistToGrid > 2)
        return;

    PlantState aNewState = mState;
    if (mState == PlantState::STATE_BOWLING_UP && mRow <= 0)
    {
        aNewState = PlantState::STATE_BOWLING_DOWN;
    }
    else if (mState == PlantState::STATE_BOWLING_DOWN && mRow >= 4)
    {
        aNewState = PlantState::STATE_BOWLING_UP;
    }

    Zombie* aZombie = FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY);
    if (aZombie)
    {
        int aPosX = mX + mWidth / 2;
        int aPosY = mY + mHeight / 2;

        if (mSeedType == SeedType::SEED_EXPLODE_O_NUT)
        {
            mApp->PlayFoley(FoleyType::FOLEY_CHERRYBOMB);
            mApp->PlaySample(SOUND_BOWLINGIMPACT2);

            int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY) | 32U;
            mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 90, 1, true, aDamageRangeFlags, GetAttackDamage(1800));
            mApp->AddTodParticle(aPosX, aPosY, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_POWIE);
            mBoard->ShakeBoard(3, -4);

            Die();

            return;
        }

        mApp->PlayFoley(FoleyType::FOLEY_BOWLINGIMPACT);
        mBoard->ShakeBoard(1, -2);

        if (mSeedType == SeedType::SEED_GIANT_WALLNUT)
        {
            aZombie->TakeDamage(GetAttackDamage(1800), 0U);
        }
        else if (aZombie->mShieldType == ShieldType::SHIELDTYPE_DOOR && mState != PlantState::STATE_NOTREADY)
        {
            aZombie->TakeDamage(GetAttackDamage(1800), 0U);
        }
        else if (aZombie->mShieldType != ShieldType::SHIELDTYPE_NONE)
        {
            aZombie->TakeShieldDamage(400, 0U);
        }
        else if (aZombie->mHelmType != HelmType::HELMTYPE_NONE)
        {
            if (aZombie->mHelmType == HelmType::HELMTYPE_PAIL)
            {
                mApp->PlayFoley(FoleyType::FOLEY_SHIELD_HIT);
            }
            else if (aZombie->mHelmType == HelmType::HELMTYPE_TRAFFIC_CONE)
            {
                mApp->PlayFoley(FoleyType::FOLEY_PLASTIC_HIT);
            }
            
            aZombie->TakeHelmDamage(GetAttackDamage(900), 0U);
        }
        else
        {
            aZombie->TakeDamage(GetAttackDamage(1800), 0U);
        }

        if ((!mApp->IsFirstTimeAdventureMode() || mApp->mPlayerInfo->GetLevel() > 10) && mSeedType == SeedType::SEED_WALLNUT)
        {
            mLaunchCounter++;
            if (mLaunchCounter == 2)
            {
                mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX, aPosY, CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
            }
            else if (mLaunchCounter == 3)
            {
                mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX - 5.0f, aPosY, CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
                mBoard->AddCoin(aPosX + 5.0f, aPosY, CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
            }
            else if (mLaunchCounter == 4)
            {
                mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX - 10.0f, aPosY, CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
                mBoard->AddCoin(aPosX, aPosY, CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
                mBoard->AddCoin(aPosX + 10.0f, aPosY, CoinType::COIN_SILVER, CoinMotion::COIN_MOTION_COIN);
            }
            else if (mLaunchCounter >= 5)
            {
                mApp->PlayFoley(FoleyType::FOLEY_SPAWN_SUN);
                mBoard->AddCoin(aPosX, aPosY, CoinType::COIN_GOLD, CoinMotion::COIN_MOTION_COIN);
                ReportAchievement::GiveAchievement(mApp, RollSomeHeads, true); // @Patoke: add achievement
            }
        }

        if (mSeedType != SeedType::SEED_GIANT_WALLNUT)
        {
            if (mRow == 4 || mState == PlantState::STATE_BOWLING_DOWN)
            {
                aNewState = PlantState::STATE_BOWLING_UP;
            }
            else if (mRow == 0 || mState == PlantState::STATE_BOWLING_UP)
            {
                aNewState = PlantState::STATE_BOWLING_DOWN;
            }
            else
            {
                aNewState = Sexy::Rand(2) ? PlantState::STATE_BOWLING_UP : PlantState::STATE_BOWLING_DOWN;
            }
        }
    }

    if (aNewState == PlantState::STATE_BOWLING_UP)
    {
        mRow--;
        mState = PlantState::STATE_BOWLING_UP;
        mRenderOrder = CalcRenderOrder();
    }
    else if (aNewState == PlantState::STATE_BOWLING_DOWN)
    {
        mState = PlantState::STATE_BOWLING_DOWN;
        mRenderOrder = CalcRenderOrder();
        mRow++;
    }
}

void Plant::UpdateAbilities()
{
    if (!IsInPlay())
        return;

    if (mState == PlantState::STATE_DOINGSPECIAL || mSquished)
    {
        mDisappearCountdown--;
        if (mDisappearCountdown < 0)
        {
            Die();
            return;
        }
    }

    if (mWakeUpCounter > 0)
    {
        mWakeUpCounter--;
        if (mWakeUpCounter == 60)
        {
            mApp->PlayFoley(FoleyType::FOLEY_WAKEUP);
        }
        if (mWakeUpCounter == 0)
        {
            SetSleeping(false);
        }
    }

    // 狂暴飞贼落下后会沉默目标植物。沉默时禁止一切植物行为，包括攻击、
    // 生产、辅助光环和计时技能；已有的投射物不会由此植物继续产生。
    if (mSilenceCountdown > 0)
    {
        mSilenceCountdown--;
        return;
    }

    if (mIsAsleep || mSquished || mOnBungeeState != PlantOnBungeeState::NOT_ON_BUNGEE)
        return;

    // 「僵直」：种下后一段时间内不执行任何行为更新。
    if (mRuleStun > 0)
    {
        --mRuleStun;
        return;
    }

    if (mSeedType == SeedType::SEED_DOOMPEA)
    {
        constexpr int DOOM_PEA_BEAM_DURATION = 240;
        // 普攻：每 2 秒发射一次贯穿本行的直线激光，每个敌方僵尸 200 点伤害。
        if (mDoomPeaPrimaryFlash > 0)
            --mDoomPeaPrimaryFlash;
        if (--mDoomPeaPrimaryCounter <= 0)
        {
            mDoomPeaPrimaryCounter = 120;
            // Keep the beam visible long enough to show a short muzzle flash,
            // travel out, and then fade like the game's other beam effects.
            mDoomPeaPrimaryFlash = 24;
            int aPeaHeadOffsetX = 0;
            int aPeaHeadOffsetY = 0;
            GetPeaHeadOffset(aPeaHeadOffsetX, aPeaHeadOffsetY);
            const float aOriginX = static_cast<float>(mX + aPeaHeadOffsetX + 24);
            Zombie* aZombie = nullptr;
            while (mBoard->IterateZombies(aZombie))
            {
                if (aZombie->IsDeadOrDying() || aZombie->mMindControlled || aZombie->mRow != mRow)
                    continue;
                if (aZombie->GetZombieRect().mX + aZombie->GetZombieRect().mWidth < aOriginX)
                    continue;
                aZombie->TakeDamage(GetAttackDamage(200),
                    (1U << static_cast<int>(DamageFlags::DAMAGE_BYPASSES_SHIELD)) |
                    (1U << static_cast<int>(DamageFlags::DAMAGE_DOOMPEA_PRIMARY)));
            }
        }
        if (mDoomPeaAngle > 0)
        {
            int aPeaHeadOffsetX = 0;
            int aPeaHeadOffsetY = 0;
            GetPeaHeadOffset(aPeaHeadOffsetX, aPeaHeadOffsetY);
            const float aOriginX = static_cast<float>(mX + aPeaHeadOffsetX + 24);
            const float aOriginY = static_cast<float>(mY + aPeaHeadOffsetY - 33);
            Zombie* aZombie = nullptr;
            while (mBoard->IterateZombies(aZombie))
			{
                if (aZombie->IsDeadOrDying() || aZombie->mMindControlled)
                    continue;
                Rect aZombieRect = aZombie->GetZombieRect();
                const float aZombieCenterX = aZombieRect.mX + aZombieRect.mWidth * 0.5f;
                const float aZombieCenterY = aZombieRect.mY + aZombieRect.mHeight * 0.5f;
                float dx = aZombieCenterX - aOriginX;
                float dy = aZombieCenterY - aOriginY;
                if (dx <= 0.0f)
                    continue;
                float angle = std::atan2(std::fabs(dy), dx) * 180.0f / 3.14159265f;
                if (angle <= static_cast<float>(mDoomPeaAngle))
                    aZombie->TakeDamage(GetAttackDamage(std::max(1, 100 / std::max(1, mDoomPeaAngle))),
                        (1U << static_cast<int>(DamageFlags::DAMAGE_BYPASSES_SHIELD)) |
                        (1U << static_cast<int>(DamageFlags::DAMAGE_DOOMPEA_BEAM)));
            }
            ++mDoomPeaBeamTimer;
            mDoomPeaRestore = DOOM_PEA_BEAM_DURATION - mDoomPeaBeamTimer;
            if (mDoomPeaBeamTimer >= DOOM_PEA_BEAM_DURATION)
            {
                aZombie = nullptr;
                while (mBoard->IterateZombies(aZombie))
					if (aZombie->mRow == mRow && !aZombie->mMindControlled && !aZombie->IsDeadOrDying() && aZombie->mPosX >= aOriginX)
                        aZombie->TakeDamage(GetAttackDamage(10000),
                            (1U << static_cast<int>(DamageFlags::DAMAGE_BYPASSES_SHIELD)) |
                            (1U << static_cast<int>(DamageFlags::DAMAGE_DOOMPEA_BEAM)) |
                            (1U << static_cast<int>(DamageFlags::DAMAGE_DOOMPEA_FINISH)));
                mDoomPeaAngle = 0;
                mDoomPeaRestore = 0;
            }
            else
            {
                mDoomPeaAngle = 45 - (mDoomPeaBeamTimer * 45 / DOOM_PEA_BEAM_DURATION);
                if (mDoomPeaAngle < 1)
                    mDoomPeaAngle = 1;
            }
        }
        else if (mDoomPeaRestore > 0)
        {
            --mDoomPeaRestore;
        }
        else if (--mDoomPeaCounter <= 0)
        {
            mDoomPeaCounter = 1500;
            mDoomPeaAngle = 45;
            mDoomPeaBeamTimer = 0;
            mDoomPeaRestore = DOOM_PEA_BEAM_DURATION;
        }
        return;
    }
    
    UpdateShooting();

    if (mStateCountdown > 0)
        mStateCountdown--;

    if (mApp->IsWallnutBowlingLevel())
    {
        UpdateBowling();
        return;
    }

    if (mSeedType == SeedType::SEED_SQUASH)                                                     UpdateSquash();
    else if (mSeedType == SeedType::SEED_DOOMSHROOM)                                            UpdateDoomShroom();
    else if (mSeedType == SeedType::SEED_ICESHROOM)                                             UpdateIceShroom();
    else if (mSeedType == SeedType::SEED_CHOMPER)                                               UpdateChomper();
    else if (mSeedType == SeedType::SEED_BLOVER)                                                UpdateBlover();
    else if (mSeedType == SeedType::SEED_FLOWERPOT)                                             UpdateFlowerPot();
    else if (mSeedType == SeedType::SEED_LILYPAD)                                               UpdateLilypad();
    else if (mSeedType == SeedType::SEED_IMITATER)                                              UpdateImitater();
    else if (mSeedType == SeedType::SEED_INSTANT_COFFEE)                                        UpdateCoffeeBean();
    else if (mSeedType == SeedType::SEED_UMBRELLA)                                              UpdateUmbrella();
    else if (mSeedType == SeedType::SEED_COBCANNON)                                             UpdateCobCannon();
    else if (mSeedType == SeedType::SEED_CACTUS)                                                UpdateCactus();
    else if (mSeedType == SeedType::SEED_MAGNETSHROOM)                                          UpdateMagnetShroom();
    else if (mSeedType == SeedType::SEED_GOLD_MAGNET)                                           UpdateGoldMagnetShroom();
    else if (mSeedType == SeedType::SEED_SUNSHROOM)                                             UpdateSunShroom();
    else if (MakesSun() || mSeedType == SeedType::SEED_MARIGOLD)                                UpdateProductionPlant();
    else if (mSeedType == SeedType::SEED_GRAVEBUSTER)                                           UpdateGraveBuster();
    else if (mSeedType == SeedType::SEED_TORCHWOOD)                                             UpdateTorchwood();
    else if (mSeedType == SeedType::SEED_POTATOMINE)                                            UpdatePotato();
    else if (mSeedType == SeedType::SEED_HYPNOSHROOM)                                           UpdateHypnoShroom();
    else if (mSeedType == SeedType::SEED_SPIKEWEED || mSeedType == SeedType::SEED_SPIKEROCK)    UpdateSpikeweed();
    else if (mSeedType == SeedType::SEED_TANGLEKELP)                                            UpdateTanglekelp();
    else if (mSeedType == SeedType::SEED_SCAREDYSHROOM)                                         UpdateScaredyShroom();
    else if (mSeedType == SeedType::SEED_PUFFSHROOM || mSeedType == SeedType::SEED_SEASHROOM)     UpdatePuffColony();

    if (mSubclass == PlantSubClass::SUBCLASS_SHOOTER)
    {
        UpdateShooter();
    }
    UpdateChainFiring();
    if (mDoSpecialCountdown > 0)
    {
        mDoSpecialCountdown--;
        if (mDoSpecialCountdown == 0)
        {
            DoSpecial();
        }
    }
}

bool Plant::IsPartOfUpgradableTo(SeedType theUpgradedType)
{
    if (theUpgradedType == SeedType::SEED_COBCANNON && mSeedType == SeedType::SEED_KERNELPULT)
    {
        return mBoard->IsValidCobCannonSpot(mPlantCol, mRow) || mBoard->IsValidCobCannonSpot(mPlantCol - 1, mRow);
    }

    return IsUpgradableTo(theUpgradedType);
}

bool Plant::IsUpgradableTo(SeedType theUpgradedType)
{
    if (theUpgradedType == SeedType::SEED_GATLINGPEA && mSeedType == SeedType::SEED_REPEATER)
    {
        return true;
    }
    if (theUpgradedType == SeedType::SEED_WINTERMELON && mSeedType == SeedType::SEED_MELONPULT)
    {
        return true;
    }
    if (theUpgradedType == SeedType::SEED_TWINSUNFLOWER && mSeedType == SeedType::SEED_SUNFLOWER)
    {
        return true;
    }
    if (theUpgradedType == SeedType::SEED_SPIKEROCK && mSeedType == SeedType::SEED_SPIKEWEED)
    {
        return true;
    }
    if (theUpgradedType == SeedType::SEED_COBCANNON && mSeedType == SeedType::SEED_KERNELPULT)
    {
        return mBoard->IsValidCobCannonSpot(mPlantCol, mRow);
    }
    if (theUpgradedType == SeedType::SEED_GOLD_MAGNET && mSeedType == SeedType::SEED_MAGNETSHROOM)
    {
        return true;
    }
    if (theUpgradedType == SeedType::SEED_GLOOMSHROOM && mSeedType == SeedType::SEED_FUMESHROOM)
    {
        return true;
    }
    if (theUpgradedType == SeedType::SEED_CATTAIL && mSeedType == SeedType::SEED_LILYPAD)
    {
        Plant* aPlant = mBoard->GetTopPlantAt(mPlantCol, mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
        return aPlant == nullptr || aPlant->mSeedType != SeedType::SEED_CATTAIL;
    }
    return false;
}

// GOTY @Patoke: 0x466FD0
void Plant::UpdateReanimColor()
{
    if (!IsOnBoard())
        return;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == nullptr)
        return;

    SeedType aSeedType = mBoard->GetSeedTypeInCursor();
    Color aColorOverride;

    bool isOnGlove = false;
    if (mBoard->mCursorObject->mCursorType == CursorType::CURSOR_TYPE_PLANT_FROM_GLOVE)
    {
        Plant* aPlant = mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mBoard->mCursorObject->mGlovePlantID));
        if (aPlant && aPlant->mPlantCol == mPlantCol && aPlant->mRow == mRow)
        {
            isOnGlove = true;
        }
    }

    if (isOnGlove)
    {
        aColorOverride = Color(128, 128, 128);
    }
    else if (IsPartOfUpgradableTo(aSeedType) && mBoard->CanPlantAt(mPlantCol, mRow, aSeedType) == PLANTING_OK)
    {
        aColorOverride = GetFlashingColor(mBoard->mMainCounter, 90);
    }
    else if (aSeedType == SeedType::SEED_COBCANNON && mSeedType == SeedType::SEED_KERNELPULT && mBoard->CanPlantAt(mPlantCol - 1, mRow, aSeedType) == PLANTING_OK)
    {
        aColorOverride = GetFlashingColor(mBoard->mMainCounter, 90);
    }
    else if (mSeedType == SeedType::SEED_EXPLODE_O_NUT)
    {
        aColorOverride = Color(255, 64, 64);
    }
    else
    {
        aColorOverride = Color(255, 255, 255);
    }

    if (mSeedType == SeedType::SEED_SHIELDWALLNUT)
    {
        // UpdateReanimColor runs every frame, including for restored plants.
        // Keep the shield appearance here so normal highlight updates cannot
        // reset its alpha, and repair filters restored from older saves.
        aBodyReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT_194;
        aColorOverride.mAlpha = 128;
        Reanimation* aBlinkReanim = mApp->ReanimationTryToGet(mBlinkReanimID);
        if (aBlinkReanim)
            aBlinkReanim->mFilterEffect = aBodyReanim->mFilterEffect;
    }
    aBodyReanim->mColorOverride = aColorOverride;

    if (mHighlighted)
    {
        aBodyReanim->mExtraAdditiveColor = Color(255, 255, 255, 196);
        aBodyReanim->mEnableExtraAdditiveDraw = true;
        if (mImitaterType == SeedType::SEED_IMITATER)
        {
            aBodyReanim->mExtraAdditiveColor = Color(255, 255, 255, 92);
        }
    }
    else if (mBeghouledFlashCountdown > 0)
    {
        int anAlpha = TodAnimateCurve(50, 0, mBeghouledFlashCountdown % 50, 1, 128, TodCurves::CURVE_BOUNCE);
        aBodyReanim->mExtraAdditiveColor = Color(255, 255, 255, anAlpha);
        aBodyReanim->mEnableExtraAdditiveDraw = true;
    }
    else if (mEatenFlashCountdown > 0)
    {
        int aGrayness = ClampInt(mEatenFlashCountdown * 3, 0, mImitaterType == SeedType::SEED_IMITATER ? 128 : 255);
        aBodyReanim->mExtraAdditiveColor = Color(aGrayness, aGrayness, aGrayness);
        aBodyReanim->mEnableExtraAdditiveDraw = true;
    }
    else
    {
        aBodyReanim->mEnableExtraAdditiveDraw = false;
    }

    if (mBeghouledFlashCountdown > 0)
    {
        int anAlpha = TodAnimateCurve(50, 0, mBeghouledFlashCountdown % 50, 1, 128, TodCurves::CURVE_BOUNCE);
        aBodyReanim->mExtraOverlayColor = Color(255, 255, 255, anAlpha);
        aBodyReanim->mEnableExtraOverlayDraw = true;
    }
    else
    {
        aBodyReanim->mEnableExtraOverlayDraw = false;
    }

    aBodyReanim->PropogateColorToAttachments();
}

// GOTY @Patoke: 0x469649
bool Plant::IsOnBoard()
{
    if (!mIsOnBoard)
        return false;

    TOD_ASSERT(mBoard);
    return true;
}

bool Plant::IsInPlay()
{
    return IsOnBoard() && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mGameMode != GameMode::GAMEMODE_TREE_OF_WISDOM;
}

void Plant::UpdateReanim()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == nullptr)
        return;

    UpdateReanimColor();

    float aOffsetX = mShakeOffsetX;
    float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);
    float aScaleX = 1.0f, aScaleY = 1.0f;
    if ((mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BIG_TIME) &&
        (mSeedType == SeedType::SEED_WALLNUT || mSeedType == SeedType::SEED_SUNFLOWER || mSeedType == SeedType::SEED_MARIGOLD))
    {
        aScaleX = 1.5f;
        aScaleY = 1.5f;
        aOffsetX -= 20.0f;
        aOffsetY -= 40.0f;
    }
    if (mSeedType == SeedType::SEED_GIANT_WALLNUT)
    {
        aScaleX = 2.0f;
        aScaleY = 2.0f;
        aOffsetX -= 76.0f;
        aOffsetY -= 64.0f;
    }
    if (mSeedType == SeedType::SEED_INSTANT_COFFEE)
    {
        aScaleX = 0.8f;
        aScaleY = 0.8f;
        aOffsetX += 12.0f;
        aOffsetY += 10.0f;
    }
    if (mSeedType == SeedType::SEED_POTATOMINE)
    {
        aScaleX = 0.8f;
        aScaleY = 0.8f;
        aOffsetX += 12.0f;
        aOffsetY += 12.0f;
    }
    if (mState == PlantState::STATE_GRAVEBUSTER_EATING)
    {
        aOffsetY += TodAnimateCurveFloat(400, 0, mStateCountdown, 0.0f, 30.0f, TodCurves::CURVE_LINEAR);
    }
    if (mWakeUpCounter > 0)
    {
        float aScaleFactor = TodAnimateCurveFloat(70, 0, mWakeUpCounter, 1.0f, 0.8f, TodCurves::CURVE_EASE_SIN_WAVE);
        aScaleY *= aScaleFactor;
        aOffsetY += 80.0f - 80.0f * aScaleFactor;
    }

    aBodyReanim->Update();

    if (mSeedType == SeedType::SEED_LEFTPEATER)
    {
        aOffsetX += 80.0f * aScaleX;
        aScaleX *= -1.0f;
    }

    if (mPottedPlantIndex != -1)
    {
        PottedPlant* aPottedPlant = &mApp->mPlayerInfo->mPottedPlant[mPottedPlantIndex];

        if (aPottedPlant->mFacing == PottedPlant::FacingDirection::FACING_LEFT)
        {
            aOffsetX += 80.0f * aScaleX;
            aScaleX *= -1.0f;
        }

        float aOffsetXStart, aOffsetXEnd;
        float aOffsetYStart, aOffsetYEnd;
        float aScaleStart, aScaleEnd;
        if (aPottedPlant->mPlantAge == PottedPlantAge::PLANTAGE_SMALL)
        {
            aOffsetXStart = 20.0f;
            aOffsetXEnd = 20.0f;
            aOffsetYStart = 40.0f;
            aOffsetYEnd = 40.0f;
            aScaleStart = 0.5f;
            aScaleEnd = 0.5f;
        }
        else if (aPottedPlant->mPlantAge == PottedPlantAge::PLANTAGE_MEDIUM)
        {
            aOffsetXStart = 20.0f;
            aOffsetXEnd = 10.0f;
            aOffsetYStart = 40.0f;
            aOffsetYEnd = 20.0f;
            aScaleStart = 0.5f;
            aScaleEnd = 0.75f;
        }
        else
        {
            aOffsetXStart = 10.0f;
            aOffsetXEnd = 0.0f;
            aOffsetYStart = 20.0f;
            aOffsetYEnd = 0.0f;
            aScaleStart = 0.75f;
            aScaleEnd = 1.0f;
        }

        float aAnimatedOffsetX = TodAnimateCurveFloat(100, 0, mStateCountdown, aOffsetXStart, aOffsetXEnd, TodCurves::CURVE_LINEAR);
        float aAnimatedOffsetY = TodAnimateCurveFloat(100, 0, mStateCountdown, aOffsetYStart, aOffsetYEnd, TodCurves::CURVE_LINEAR);
        float aAnimatedScale = TodAnimateCurveFloat(100, 0, mStateCountdown, aScaleStart, aScaleEnd, TodCurves::CURVE_LINEAR);

        aOffsetX += aAnimatedOffsetX * aScaleX;
        aOffsetY += aAnimatedOffsetY * aScaleY;
        aScaleX *= aAnimatedScale;
        aScaleY *= aAnimatedScale;
        aOffsetX += mApp->mZenGarden->ZenPlantOffsetX(aPottedPlant);
        aOffsetY += mApp->mZenGarden->PlantPottedDrawHeightOffset(mSeedType, aScaleY);
    }

    aBodyReanim->SetPosition(aOffsetX, aOffsetY);
    aBodyReanim->OverrideScale(aScaleX, aScaleY);
}

void Plant::Update()
{
    bool doUpdate = false;
    if (IsOnBoard() && mApp->mGameScene == GameScenes::SCENE_LEVEL_INTRO && mApp->IsWallnutBowlingLevel())
        doUpdate = true;
    else if (IsOnBoard() && mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
        doUpdate = true;
    else if (IsOnBoard() && mBoard->mCutScene->ShouldRunUpsellBoard())
        doUpdate = true;
    else if (!IsOnBoard() || mApp->mGameScene == GameScenes::SCENE_PLAYING)
        doUpdate = true;

    if (doUpdate)
    {
        if (mApp->mGameScene == GameScenes::SCENE_PLAYING &&
            mErrorCountdown > 0 && --mErrorCountdown == 0)
        {
            Die();
            return;
        }
		const bool aShielded = mApp->IsCricketMode() && mBoard->mChallenge->CricketRowShield(mRow) > 0;
		if (aShielded)
		{
			if (mCricketShieldSnapshotValid)
				mPlantHealth = mCricketShieldHealthSnapshot;
			else
			{
				mCricketShieldHealthSnapshot = mPlantHealth;
				mCricketShieldSnapshotValid = true;
			}
		}
		else
		{
			mCricketShieldSnapshotValid = false;
		}

        UpdateAbilities();
        UpdatePlantRule();
        Animate();
		if (aShielded)
			mPlantHealth = mCricketShieldHealthSnapshot;

        if (mPlantHealth < 0)
            Die();

        UpdateOverdrawn();
        UpdateReanim();
    }
}

bool Plant::NotOnGround()
{
    if (mSeedType == SeedType::SEED_SQUASH)
    {
        if (mState == PlantState::STATE_SQUASH_RISING || mState == PlantState::STATE_SQUASH_FALLING || mState == PlantState::STATE_SQUASH_DONE_FALLING)
            return true;
    }

    return mSquished || mOnBungeeState == PlantOnBungeeState::RISING_WITH_BUNGEE || mDead;
}

Reanimation* Plant::AttachBlinkAnim(Reanimation* theReanimBody)
{
    const PlantDefinition& aPlantDef = GetPlantDefinition(mSeedType);
    LawnApp* aApp = (LawnApp*)gSexyAppBase;
    Reanimation* aAnimToAttach = theReanimBody;
    const char* aTrackToPlay = "anim_blink";
    const char* aTrackToAttach = nullptr;

    if (mSeedType == SeedType::SEED_WALLNUT || mSeedType == SeedType::SEED_TALLNUT || 
        mSeedType == SeedType::SEED_EXPLODE_O_NUT || mSeedType == SeedType::SEED_GIANT_WALLNUT || mSeedType == SeedType::SEED_SHIELDWALLNUT)
    {
        int aHit = Rand(10);
        if (aHit < 1 && theReanimBody->TrackExists("anim_blink_twitch"))
        {
            aTrackToPlay = "anim_blink_twitch";
        }
        else
        {
            aTrackToPlay = aHit < 7 ? "anim_blink_twice" : "anim_blink_thrice";
        }
    }
    else if (mSeedType == SeedType::SEED_THREEPEATER)
    {
        int aHit = Rand(3);
        if (aHit == 0)
        {
            aTrackToPlay = "anim_blink1";
            aTrackToAttach = "anim_face1";
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_head1");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
        else if (aHit == 1)
        {
            aTrackToPlay = "anim_blink2";
            aTrackToAttach = "anim_face2";
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_head2");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
        else
        {
            aTrackToPlay = "anim_blink3";
            aTrackToAttach = "anim_face3";
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_head3");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
    }
    else if (mSeedType == SeedType::SEED_SPLITPEA)
    {
        if (Rand(2) == 0)
        {
            aTrackToPlay = "anim_blink";
            aTrackToAttach = "anim_face";
            aAnimToAttach = mApp->ReanimationTryToGet(mHeadReanimID);
        }
        else
        {
            aTrackToPlay = "anim_blink2";
            aTrackToAttach = "anim_face2";
            aAnimToAttach = mApp->ReanimationTryToGet(mHeadReanimID2);
        }
    }
    else if (mSeedType == SeedType::SEED_TWINSUNFLOWER)
    {
        if (Rand(2) == 0)
        {
            aTrackToPlay = "anim_blink";
            aTrackToAttach = "anim_face";
        }
        else
        {
            aTrackToPlay = "anim_blink2";
            aTrackToAttach = "anim_face2";
        }
    }
    else if (mSeedType == SeedType::SEED_PEASHOOTER || mSeedType == SeedType::SEED_IRONPEASHOOTER || mSeedType == SeedType::SEED_SNOWPEA || mSeedType == SeedType::SEED_REPEATER || mSeedType == SeedType::SEED_LEFTPEATER || mSeedType == SeedType::SEED_GATLINGPEA)
    {
        if (theReanimBody->TrackExists("anim_stem"))
        {
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_stem");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
        else if (theReanimBody->TrackExists("anim_idle"))
        {
            ReanimatorTrackInstance* aTrackInstance = theReanimBody->GetTrackInstanceByName("anim_idle");
            aAnimToAttach = FindReanimAttachment(aTrackInstance->mAttachmentID);
        }
    }

    if (aAnimToAttach == nullptr)
    {
        TodTrace("Missing head anim");
        return nullptr;
    }

    if (!theReanimBody->TrackExists(aTrackToPlay))
        return nullptr;

    Reanimation* aBlinkReanim = aApp->mEffectSystem->mReanimationHolder->AllocReanimation(0.0f, 0.0f, 0, aPlantDef.mReanimationType);
    aBlinkReanim->SetFramesForLayer(aTrackToPlay);
    aBlinkReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_FULL_LAST_FRAME_AND_HOLD;
    aBlinkReanim->mAnimRate = 15.0f;
    aBlinkReanim->mColorOverride = theReanimBody->mColorOverride;

    if (aTrackToAttach && aAnimToAttach->TrackExists(aTrackToAttach))
    {
        aBlinkReanim->AttachToAnotherReanimation(aAnimToAttach, aTrackToAttach);
    }
    else if (aAnimToAttach->TrackExists("anim_face"))
    {
        aBlinkReanim->AttachToAnotherReanimation(aAnimToAttach, "anim_face");
    }
    else if (aAnimToAttach->TrackExists("anim_idle"))
    {
        aBlinkReanim->AttachToAnotherReanimation(aAnimToAttach, "anim_idle");
    }
    else
    {
        TodTrace("Missing anim_idle for blink");
    }

    aBlinkReanim->mFilterEffect = theReanimBody->mFilterEffect;
    return aBlinkReanim;
}

void Plant::DoBlink()
{
    mBlinkCountdown = 400 + Rand(400);

    if (NotOnGround() || mShootingCounter != 0)
        return;

    if (mSeedType == SeedType::SEED_POTATOMINE && mState != PlantState::STATE_POTATO_ARMED)
        return;

    if (mState == PlantState::STATE_CACTUS_RISING || mState == PlantState::STATE_CACTUS_HIGH || mState == PlantState::STATE_CACTUS_LOWERING ||
        mState == PlantState::STATE_MAGNETSHROOM_SUCKING || mState == PlantState::STATE_MAGNETSHROOM_CHARGING)
        return;

    EndBlink();
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == nullptr)
        return;

    if ((mSeedType == SeedType::SEED_TALLNUT && aBodyReanim->GetImageOverride("anim_idle") == IMAGE_REANIM_TALLNUT_CRACKED2) || 
        (mSeedType == SeedType::SEED_GARLIC && aBodyReanim->GetImageOverride("anim_face") == IMAGE_REANIM_GARLIC_BODY3))
        return;

    if (mSeedType == SeedType::SEED_WALLNUT || mSeedType == SeedType::SEED_TALLNUT || 
        mSeedType == SeedType::SEED_EXPLODE_O_NUT || mSeedType == SeedType::SEED_GIANT_WALLNUT)
    {
        mBlinkCountdown = 1000 + Rand(1000);
    }

    Reanimation* aBlinkReanim = AttachBlinkAnim(aBodyReanim);
    if (aBlinkReanim)
    {
        mBlinkReanimID = mApp->ReanimationGetID(aBlinkReanim);
    }
    aBodyReanim->AssignRenderGroupToPrefix("anim_eye", RENDER_GROUP_HIDDEN);
}

void Plant::EndBlink()
{
    if (mBlinkReanimID != ReanimationID::REANIMATIONID_NULL)
    {
        mApp->RemoveReanimation(mBlinkReanimID);
        mBlinkReanimID = ReanimationID::REANIMATIONID_NULL;

        Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
        if (aBodyReanim)
        {
            aBodyReanim->AssignRenderGroupToPrefix("anim_eye", RENDER_GROUP_NORMAL);
        }
    }
}

void Plant::UpdateBlink()
{
    if (mBlinkReanimID != ReanimationID::REANIMATIONID_NULL)
    {
        Reanimation* aBlinkReanim = mApp->ReanimationTryToGet(mBlinkReanimID);
        if (aBlinkReanim == nullptr || aBlinkReanim->mLoopCount > 0)
        {
            EndBlink();
        }
    }

    if (mIsAsleep)
        return;

    if (mBlinkCountdown > 0)
    {
        mBlinkCountdown--;
        if (mBlinkCountdown == 0)
        {
            DoBlink();
        }
    }
}

void Plant::AnimateNuts()
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == nullptr)
        return;

    Image* aCracked1;
    Image* aCracked2;
    const char* aTrackToOverride;
    if (mSeedType == SeedType::SEED_WALLNUT)
    {
        aCracked1 = IMAGE_REANIM_WALLNUT_CRACKED1;
        aCracked2 = IMAGE_REANIM_WALLNUT_CRACKED2;
        aTrackToOverride = "anim_face";
    }
    else if (mSeedType == SeedType::SEED_TALLNUT)
    {
        aCracked1 = IMAGE_REANIM_TALLNUT_CRACKED1;
        aCracked2 = IMAGE_REANIM_TALLNUT_CRACKED2;
        aTrackToOverride = "anim_idle";
    }
    else return;

    int aPosX = mX + 40;
    int aPosY = mY + 10;
    if (mSeedType == SeedType::SEED_TALLNUT)
    {
        aPosY -= 32;
    }

    Image* aImageOverride = aBodyReanim->GetImageOverride(aTrackToOverride);
    if (mPlantHealth < mPlantMaxHealth / 3)
    {
        if (aImageOverride != aCracked2)
        {
            aBodyReanim->SetImageOverride(aTrackToOverride, aCracked2);
            mApp->AddTodParticle(aPosX, aPosY, mRenderOrder + 4, ParticleEffect::PARTICLE_WALLNUT_EAT_LARGE);
        }
    }
    else if (mPlantHealth < mPlantMaxHealth * 2 / 3)
    {
        if (aImageOverride != aCracked1)
        {
            aBodyReanim->SetImageOverride(aTrackToOverride, aCracked1);
            mApp->AddTodParticle(aPosX, aPosY, mRenderOrder + 4, ParticleEffect::PARTICLE_WALLNUT_EAT_LARGE);
        }
    }
    else
    {
        aBodyReanim->SetImageOverride(aTrackToOverride, nullptr);
    }

    if (IsInPlay() && !mApp->IsIZombieLevel())
    {
        if (mRecentlyEatenCountdown > 0)
        {
            aBodyReanim->mAnimRate = 0.1f;
            return;
        }

        if (aBodyReanim->mAnimRate < 1.0f && mOnBungeeState != PlantOnBungeeState::RISING_WITH_BUNGEE)
        {
            aBodyReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);
        }
    }
}

void Plant::AnimateGarlic()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    Image* aImageOverride = aBodyReanim->GetImageOverride("anim_face");

    if (mPlantHealth < mPlantMaxHealth / 3)
    {
        if (aImageOverride != IMAGE_REANIM_GARLIC_BODY3)
        {
            aBodyReanim->SetImageOverride("anim_face", IMAGE_REANIM_GARLIC_BODY3);
            aBodyReanim->AssignRenderGroupToPrefix("Garlic_stem", RENDER_GROUP_HIDDEN);
        }
    }
    else if (mPlantHealth < mPlantMaxHealth * 2 / 3)
    {
        if (aImageOverride != IMAGE_REANIM_GARLIC_BODY2)
        {
            aBodyReanim->SetImageOverride("anim_face", IMAGE_REANIM_GARLIC_BODY2);
        }
    }
    else
    {
        aBodyReanim->SetImageOverride("anim_face", nullptr);
    }
}

void Plant::AnimatePumpkin()
{
    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    Image* aImageOverride = aBodyReanim->GetImageOverride("Pumpkin_front");

    if (mPlantHealth < mPlantMaxHealth / 3)
    {
        if (aImageOverride != IMAGE_REANIM_PUMPKIN_DAMAGE3)
            aBodyReanim->SetImageOverride("Pumpkin_front", IMAGE_REANIM_PUMPKIN_DAMAGE3);
    }
    else if (mPlantHealth < mPlantMaxHealth * 2 / 3)
    {
        if (aImageOverride != IMAGE_REANIM_PUMPKIN_DAMAGE1)
            aBodyReanim->SetImageOverride("Pumpkin_front", IMAGE_REANIM_PUMPKIN_DAMAGE1);
    }
    else
    {
        aBodyReanim->SetImageOverride("Pumpkin_front", nullptr);
    }
}

void Plant::UpdateShooting()
{
    if (NotOnGround() || mShootingCounter == 0)
        return;

    mShootingCounter--;

    if (mSeedType == SeedType::SEED_FUMESHROOM && mShootingCounter == 15)
    {
        int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, mRow, 0);
        AddAttachedParticle(mX + 85, mY + 31, aRenderPosition, ParticleEffect::PARTICLE_FUMECLOUD);
    }

    if (mSeedType == SeedType::SEED_GLOOMSHROOM)
    {
        if (mShootingCounter == 136 || mShootingCounter == 108 || mShootingCounter == 80 || mShootingCounter == 52)
        {
            int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, mRow, 0);
            AddAttachedParticle(mX + 40, mY + 40, aRenderPosition, ParticleEffect::PARTICLE_GLOOMCLOUD);
        }
        if (mShootingCounter == 126 || mShootingCounter == 98 || mShootingCounter == 70 || mShootingCounter == 42)
        {
            Fire(nullptr, mRow, PlantWeapon::WEAPON_PRIMARY);
        }
    }
    else if (mSeedType == SeedType::SEED_GATLINGPEA)
    {
        if (mGatlingUlt)
        {
            // 大招：逐发发射，间隔 1-3 帧，共 200 颗，纵向散布但同一行
            if (mShootingCounter <= 0)
            {
                mGatlingUltYOffset = RandRangeInt(-25, 25);
                Fire(nullptr, mRow, PlantWeapon::WEAPON_PRIMARY);
                mGatlingUltRemaining--;
                if (mGatlingUltRemaining <= 0)
                {
                    mGatlingUlt = false;
                    mGatlingUltYOffset = 0;
                    mShootingCounter = 0;
                    Reanimation* aHeadReanim = mApp->ReanimationTryToGet(mHeadReanimID);
                    if (aHeadReanim)
                        aHeadReanim->mLoopType = ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD;  // 大招结束恢复
                }
                else
                {
                    mShootingCounter = RandRangeInt(1, 3);  // 1-3 帧间隔
                }
            }
        }
        else if (mShootingCounter == 18 || mShootingCounter == 28 || mShootingCounter == 38 || mShootingCounter == 48 || mShootingCounter == 58 || mShootingCounter == 68)
        {
            Fire(nullptr, mRow, PlantWeapon::WEAPON_PRIMARY);
        }
    }
    else if (mSeedType == SeedType::SEED_CATTAIL)
    {
        if (mShootingCounter == 19)
        {
            Zombie* aZombie = FindTargetZombie(mRow, PlantWeapon::WEAPON_PRIMARY);
            if (aZombie)
            {
                Fire(aZombie, mRow, PlantWeapon::WEAPON_PRIMARY);
            }
        }
    }
    else if (mShootingCounter == 1)
    {
        if (mSeedType == SeedType::SEED_THREEPEATER)
        {
            int rowAbove = mRow - 1;
            int rowBelow = mRow + 1;
            Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
            Reanimation* aHeadReanim3 = mApp->ReanimationGet(mHeadReanimID3);
            Reanimation* aHeadReanim1 = mApp->ReanimationGet(mHeadReanimID);

            if (aHeadReanim1->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(nullptr, rowBelow, PlantWeapon::WEAPON_PRIMARY);
            }
            if (aHeadReanim2->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(nullptr, mRow, PlantWeapon::WEAPON_PRIMARY);
            }
            if (aHeadReanim3->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(nullptr, rowAbove, PlantWeapon::WEAPON_PRIMARY);
            }
        }
        else if (mSeedType == SeedType::SEED_SPLITPEA)
        {
            Reanimation* aHeadBackReanim = mApp->ReanimationTryToGet(mHeadReanimID2);
            Reanimation* aHeadFrontReanim = mApp->ReanimationTryToGet(mHeadReanimID);
            if (aHeadFrontReanim->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD && mLaunchCounter > 25)
            {
                Fire(nullptr, mRow, PlantWeapon::WEAPON_PRIMARY);
            }
            if (aHeadBackReanim->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
            {
                Fire(nullptr, mRow, PlantWeapon::WEAPON_SECONDARY);
            }
        }
        else if (mState == PlantState::STATE_CACTUS_LOW)
        {
            Fire(nullptr, mRow, PlantWeapon::WEAPON_SECONDARY);
        }
        else if (mSeedType == SeedType::SEED_CABBAGEPULT || mSeedType == SeedType::SEED_KERNELPULT || mSeedType == SeedType::SEED_MELONPULT || mSeedType == SeedType::SEED_WINTERMELON)
        {
            PlantWeapon aPlantWeapon = PlantWeapon::WEAPON_PRIMARY;
            if (mState == PlantState::STATE_KERNELPULT_BUTTER)
            {
                Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_butter", RENDER_GROUP_HIDDEN);
                aBodyReanim->AssignRenderGroupToPrefix("Cornpult_kernal", RENDER_GROUP_NORMAL);
                mState = PlantState::STATE_NOTREADY;
                aPlantWeapon = PlantWeapon::WEAPON_SECONDARY;
            }

            if (mSeedType == SeedType::SEED_WINTERMELON && mWinterMelonUlt)
            {
                // 冰西瓜大招：连发 10 发冰西瓜（间隔 8 帧），每发都是大招弹
                Zombie* aUltTarget = FindTargetZombie(mRow, aPlantWeapon);
                Fire(aUltTarget, mRow, aPlantWeapon);
                mWinterMelonUltRemaining--;
                if (mWinterMelonUltRemaining <= 0)
                {
                    mWinterMelonUlt = false;
                    mShootingCounter = 0;
                }
                else
                {
                    mShootingCounter = 8;  // 8 帧间隔连发
                }
                return;
            }

            Zombie* aZombie = FindTargetZombie(mRow, aPlantWeapon);
            Fire(aZombie, mRow, aPlantWeapon);

            // 冰西瓜 10% 触发大招：连发 10 发
            if (mSeedType == SeedType::SEED_WINTERMELON && !mWinterMelonUlt && Rand(100) < 10)
            {
                mWinterMelonUlt = true;
                mWinterMelonUltRemaining = 10;
                mShootingCounter = 8;  // 立即进入连发循环（8 帧后发出第一发大招弹）
            }
        }
        else
        {
            Fire(nullptr, mRow, PlantWeapon::WEAPON_PRIMARY);
        }

        return;
    }

    if (mShootingCounter != 0)
        return;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    Reanimation* aHeadReanim = mApp->ReanimationTryToGet(mHeadReanimID);
    if (mSeedType == SeedType::SEED_THREEPEATER)
    {
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);
        Reanimation* aHeadReanim3 = mApp->ReanimationGet(mHeadReanimID3);

        if (aHeadReanim2->mLoopCount > 0)
        {
            if (aHeadReanim->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
            {
                aHeadReanim->StartBlend(20);
                aHeadReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
                aHeadReanim->SetFramesForLayer("anim_head_idle1");
                aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
                aHeadReanim->mAnimTime = aBodyReanim->mAnimTime;
            }

            aHeadReanim2->StartBlend(20);
            aHeadReanim2->mLoopType = ReanimLoopType::REANIM_LOOP;
            aHeadReanim2->SetFramesForLayer("anim_head_idle2");
            aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim2->mAnimTime = aBodyReanim->mAnimTime;

            if (aHeadReanim3->mLoopType == ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD)
            {
                aHeadReanim3->StartBlend(20);
                aHeadReanim3->mLoopType = ReanimLoopType::REANIM_LOOP;
                aHeadReanim3->SetFramesForLayer("anim_head_idle3");
                aHeadReanim3->mAnimRate = aBodyReanim->mAnimRate;
                aHeadReanim3->mAnimTime = aBodyReanim->mAnimTime;
            }
            
            return;
        }
    }
    else if (mSeedType == SeedType::SEED_SPLITPEA)
    {
        Reanimation* aHeadReanim2 = mApp->ReanimationGet(mHeadReanimID2);

        if (aHeadReanim->mLoopCount > 0)
        {
            aHeadReanim->StartBlend(20);
            aHeadReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
            aHeadReanim->SetFramesForLayer("anim_head_idle");
            aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim->mAnimTime = aBodyReanim->mAnimTime;
        }

        if (aHeadReanim2->mLoopCount > 0)
        {
            aHeadReanim2->StartBlend(20);
            aHeadReanim2->mLoopType = ReanimLoopType::REANIM_LOOP;
            aHeadReanim2->SetFramesForLayer("anim_splitpea_idle");
            aHeadReanim2->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim2->mAnimTime = aBodyReanim->mAnimTime;
        }

        return;
    }
    else if (mState == PlantState::STATE_CACTUS_HIGH)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            PlayBodyReanim("anim_idlehigh", ReanimLoopType::REANIM_LOOP, 20, 0.0f);

            aBodyReanim->mAnimRate = aBodyReanim->mDefinition->mFPS;
            if (mApp->IsIZombieLevel())
            {
                aBodyReanim->mAnimRate = 0.0f;
            }

            return;
        }
    }
    else if (aHeadReanim)
    {
        if (aHeadReanim->mLoopCount > 0)
        {
            aHeadReanim->StartBlend(20);
            aHeadReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
            aHeadReanim->SetFramesForLayer("anim_head_idle");
            aHeadReanim->mAnimRate = aBodyReanim->mAnimRate;
            aHeadReanim->mAnimTime = aBodyReanim->mAnimTime;
            return;
        }
    }
    else if (mSeedType == SeedType::SEED_COBCANNON)
    {
        if (aBodyReanim->mLoopCount > 0)
        {
            mState = PlantState::STATE_COBCANNON_ARMING;
            mStateCountdown = 3000;
            PlayBodyReanim("anim_unarmed_idle", ReanimLoopType::REANIM_LOOP, 20, aBodyReanim->mDefinition->mFPS);
            return;
        }
    }
    else if (aBodyReanim && aBodyReanim->mLoopCount > 0)
    {
        PlayIdleAnim(aBodyReanim->mDefinition->mFPS);
        return;
    }

    mShootingCounter = 1;
}

void Plant::Animate()
{
    if ((mSeedType == SeedType::SEED_CHERRYBOMB || mSeedType == SeedType::SEED_JALAPENO) && mApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
    {
        mShakeOffsetX = RandRangeFloat(-1.0f, 1.0f);
        mShakeOffsetY = RandRangeFloat(-1.0f, 1.0f);
    }

    if (mRecentlyEatenCountdown > 0)
    {
        mRecentlyEatenCountdown--;
    }
    if (mEatenFlashCountdown > 0)
    {
        mEatenFlashCountdown--;
    }
    if (mBeghouledFlashCountdown > 0)
    {
        mBeghouledFlashCountdown--;
    }

    if (mSquished)
    {
        mFrame = 0;
        return;
    }

    if (mSeedType == SeedType::SEED_WALLNUT || mSeedType == SeedType::SEED_TALLNUT)
    {
        AnimateNuts();
    }
    else if (mSeedType == SeedType::SEED_GARLIC)
    {
        AnimateGarlic();
    }
    else if (mSeedType == SeedType::SEED_PUMPKINSHELL)
    {
        AnimatePumpkin();
    }

    UpdateBlink();

    if (mAnimPing)
    {
        if (mAnimCounter < mFrameLength * mNumFrames - 1)
        {
            mAnimCounter++;
        }
        else
        {
            mAnimPing = false;
            mAnimCounter -= mFrameLength;
        }
    }
    else if (mAnimCounter > 0)
    {
        mAnimCounter--;
    }
    else
    {
        mAnimPing = true;
        mAnimCounter += mFrameLength;
    }
    mFrame = mAnimCounter / mFrameLength;
}

float PlantFlowerPotHeightOffset(SeedType theSeedType, float theFlowerPotScale)
{
    float aHeightOffset = -5.0f * theFlowerPotScale;
    float aScaleOffsetFix = 0.0f;

    switch (theSeedType)
    {
    case SeedType::SEED_CHOMPER:
    case SeedType::SEED_PLANTERN:
        aHeightOffset -= 5.0f;
        break;
    case SeedType::SEED_SCAREDYSHROOM:
        aHeightOffset += 5.0f;
        aScaleOffsetFix -= 8.0f;
        break;
    case SeedType::SEED_SUNSHROOM:
    case SeedType::SEED_PUFFSHROOM:
        aScaleOffsetFix -= 4.0f;
        break;
    case SeedType::SEED_HYPNOSHROOM:
    case SeedType::SEED_MAGNETSHROOM:
    case SeedType::SEED_PEASHOOTER:
    case SeedType::SEED_IRONPEASHOOTER:
    case SeedType::SEED_REPEATER:
    case SeedType::SEED_LEFTPEATER:
    case SeedType::SEED_SNOWPEA:
    case SeedType::SEED_THREEPEATER:
    case SeedType::SEED_SUNFLOWER:
    case SeedType::SEED_MARIGOLD:
    case SeedType::SEED_CABBAGEPULT:
    case SeedType::SEED_MELONPULT:
    case SeedType::SEED_TANGLEKELP:
    case SeedType::SEED_BLOVER:
    case SeedType::SEED_SPIKEWEED:
        aScaleOffsetFix -= 8.0f;
        break;
    case SeedType::SEED_SEASHROOM:
    case SeedType::SEED_POTATOMINE:
        aScaleOffsetFix -= 4.0f;
        break;
    case SeedType::SEED_LILYPAD:
        aScaleOffsetFix -= 16.0f;
        break;
    case SeedType::SEED_INSTANT_COFFEE:
        aScaleOffsetFix -= 20.0f;
        break;
    default:
        break;
    }

    return aHeightOffset + (theFlowerPotScale * aScaleOffsetFix - aScaleOffsetFix);
}

// GOTY @Patoke: 0x468A90
float PlantDrawHeightOffset(Board* theBoard, Plant* thePlant, SeedType theSeedType, int theCol, int theRow)
{
    float aHeightOffset = 0.0f;

    bool doFloating = false;
    if (Plant::IsFlying(theSeedType))
    {
        doFloating = false;
    }
    else if (theBoard == nullptr)
    {
        if (Plant::IsAquatic(theSeedType))
        {
            doFloating = true;
        }
    }
    else if (theBoard->IsPoolSquare(theCol, theRow))
    {
        doFloating = true;
    }
    else if (thePlant != nullptr && theBoard->mBackground == BackgroundType::BACKGROUND_ZOMBIQUARIUM)
    {
        doFloating = true;
    }

    if (doFloating)
    {
        uint32_t aCounter = theBoard ? theBoard->mMainCounter : gLawnApp->mAppCounter;

        float aPos = theRow * PI + theCol * 0.25f * PI;
        float aTime = static_cast<float>(aCounter % 200) * (2.0f * PI / 200.0f);
        float aFloatingHeight = sin(aPos + aTime) * 2.0f;
        aHeightOffset += aFloatingHeight;
    }

    if (theBoard && (thePlant == nullptr || !thePlant->mSquished))
    {
        Plant* aPot = theBoard->GetFlowerPotAt(theCol, theRow);
        if (aPot && !aPot->mSquished && theSeedType != SeedType::SEED_FLOWERPOT)
        {
            aHeightOffset += PlantFlowerPotHeightOffset(theSeedType, 1.0f);
        }
    }

    if (theSeedType == SeedType::SEED_FLOWERPOT)
    {
        aHeightOffset += 26.0f;
    }
    else if (theSeedType == SeedType::SEED_LILYPAD)
    {
        aHeightOffset += 25.0f;
    }
    else if (theSeedType == SeedType::SEED_STARFRUIT)
    {
        aHeightOffset += 10.0f;
    }
    else if (theSeedType == SeedType::SEED_TANGLEKELP)
    {
        aHeightOffset += 24.0f;
    }
    else if (theSeedType == SeedType::SEED_SEASHROOM)
    {
        aHeightOffset += 28.0f;
    }
    else if (theSeedType == SeedType::SEED_INSTANT_COFFEE)
    {
        aHeightOffset -= 20.0f;
    }
    //else if (Plant::IsFlying(theSeedType))
    //{
    //    aHeightOffset -= 30.0f;
    //}
    else if (theSeedType == SeedType::SEED_CACTUS)
    {
        return aHeightOffset;
    }
    else if (theSeedType == SeedType::SEED_PUMPKINSHELL)
    {
        aHeightOffset += 15.0f;
    }
    else if (theSeedType == SeedType::SEED_PUFFSHROOM)
    {
        aHeightOffset += 5.0f;
    }
    else if (theSeedType == SeedType::SEED_SCAREDYSHROOM)
    {
        aHeightOffset -= 14.0f;
    }
    else if (theSeedType == SeedType::SEED_GRAVEBUSTER)
    {
        aHeightOffset -= 40.0f;
    }
    else if (theSeedType == SeedType::SEED_SPIKEWEED || theSeedType == SeedType::SEED_SPIKEROCK)
    {
        int aBottomRow = 4;
        if (theBoard && theBoard->StageHas6Rows())
        {
            aBottomRow = 5;
        }

        if (theSeedType == SeedType::SEED_SPIKEROCK)
        {
            aHeightOffset += 6.0f;
        }

        if (theBoard && theBoard->GetFlowerPotAt(theCol, theRow) && gLawnApp->mGameMode != GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN)
        {
            aHeightOffset += 5.0f;
        }
        else if (theBoard && theBoard->StageHasRoof())
        {
            aHeightOffset += 15.0f;
        }
        else if (theBoard && theBoard->IsPoolSquare(theCol, theRow))
        {
            aHeightOffset += 0.0f;
        }
        else if (theRow == aBottomRow && theCol >= 7 && theBoard->StageHas6Rows())
        {
            aHeightOffset += 1.0f;
        }
        else if (theRow == aBottomRow && theCol < 7)
        {
            aHeightOffset += 12.0f;
        }
        else
        {
            aHeightOffset += 15.0f;
        }
    }

    return aHeightOffset;
}

void Plant::GetPeaHeadOffset(int& theOffsetX, int& theOffsetY)
{
    // Plants can be drawn during the short interval between their creation /
    // removal and the corresponding reanimation being registered / deleted.
    // The pea firing and Doom Pea beam paths both use this helper, so never
    // dereference a missing body animation while rendering.
    theOffsetX = 0;
    theOffsetY = 0;
    if (mApp == nullptr)
        return;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim == nullptr)
        return;

    int aTrackIndex = 0;
    if (aBodyReanim->TrackExists("anim_stem"))
    {
        aTrackIndex = aBodyReanim->FindTrackIndex("anim_stem");
    }
    else if(aBodyReanim->TrackExists("anim_idle"))
    {
        aTrackIndex = aBodyReanim->FindTrackIndex("anim_idle");
    }
    else
    {
        // Some temporary or custom plant animations have no stem/idle track.
        // GetCurrentTransform requires a valid track index, so retain the
        // safe zero offset in that case.
        return;
    }

    ReanimatorTransform aTransform;
    aBodyReanim->GetCurrentTransform(aTrackIndex, &aTransform);
    theOffsetX = aTransform.mTransX;
    theOffsetY = aTransform.mTransY;
}

// GOTY @Patoke: 0x468EB0
void Plant::DrawMagnetItems(Graphics* g)
{
    float aOffsetX = 0.0f;
    float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);

    for (int i = 0; i < MAX_MAGNET_ITEMS; i++)
    {
        MagnetItem* aMagnetItem = &mMagnetItems[i];
        if (aMagnetItem->mItemType != MagnetItemType::MAGNET_ITEM_NONE)
        {
            int aCelRow = 0, aCelCol = 0;
            Image* aImage = nullptr;
            float aScale = 0.8f;

            if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_PAIL_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_BUCKET1;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_PAIL_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_BUCKET2;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_PAIL_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_BUCKET3;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_FOOTBALL_HELMET_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_FOOTBALL_HELMET_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET2;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_FOOTBALL_HELMET_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_FOOTBALL_HELMET3;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_DOOR_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_SCREENDOOR1;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_DOOR_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_SCREENDOOR2;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_DOOR_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_SCREENDOOR3;
            }
            else if (aMagnetItem->mItemType >= MagnetItemType::MAGNET_ITEM_POGO_1 && aMagnetItem->mItemType <= MagnetItemType::MAGNET_ITEM_POGO_3)
            {
                aCelCol = static_cast<int>(aMagnetItem->mItemType) - static_cast<int>(MagnetItemType::MAGNET_ITEM_POGO_1);
                aImage = IMAGE_ZOMBIEPOGO;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_LADDER_1)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_1;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_LADDER_2)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE1;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_LADDER_3)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_1_DAMAGE2;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_LADDER_PLACED)
            {
                aImage = IMAGE_REANIM_ZOMBIE_LADDER_5;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_JACK_IN_THE_BOX)
            {
                aImage = IMAGE_REANIM_ZOMBIE_JACKBOX_BOX;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_PICK_AXE)
            {
                aImage = IMAGE_REANIM_ZOMBIE_DIGGER_PICKAXE;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_SILVER_COIN)
            {
                aScale = 1.0f;
                aImage = IMAGE_REANIM_COIN_SILVER_DOLLAR;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_GOLD_COIN)
            {
                aScale = 1.0f;
                aImage = IMAGE_REANIM_COIN_GOLD_DOLLAR;
            }
            else if (aMagnetItem->mItemType == MagnetItemType::MAGNET_ITEM_DIAMOND)
            {
                aScale = 1.0f;
                aImage = IMAGE_REANIM_DIAMOND;
            }
            else
            {
                TOD_ASSERT(false);
            }

            if (aScale == 1.0f)
            {
                g->DrawImageCel(aImage, aMagnetItem->mPosX - mX + aOffsetX, aMagnetItem->mPosY - mY + aOffsetY, aCelCol, aCelRow);
            }
            else
            {
                TodDrawImageCelScaledF(g, aImage, aMagnetItem->mPosX - mX + aOffsetX, aMagnetItem->mPosY - mY + aOffsetY, aCelCol, aCelRow, aScale, aScale);
            }
        }
    }
}

Image* Plant::GetImage(SeedType theSeedType)
{
    Image** aImages = GetPlantDefinition(theSeedType).mPlantImage;
    return aImages ? aImages[0] : nullptr;
}

void Plant::DrawShadow(Sexy::Graphics* g, float theOffsetX, float theOffsetY)
{
    if (mSeedType == SeedType::SEED_LILYPAD || mSeedType == SeedType::SEED_STARFRUIT || mSeedType == SeedType::SEED_TANGLEKELP || 
        mSeedType == SeedType::SEED_SEASHROOM || mSeedType == SeedType::SEED_COBCANNON || mSeedType == SeedType::SEED_SPIKEWEED || 
        mSeedType == SeedType::SEED_SPIKEROCK || mSeedType == SeedType::SEED_GRAVEBUSTER || mSeedType == SeedType::SEED_CATTAIL || 
        mOnBungeeState == PlantOnBungeeState::RISING_WITH_BUNGEE)
        return;

    if (IsOnBoard() && mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_ZEN_GARDEN && mApp->mZenGarden->mGardenType == GardenType::GARDEN_MAIN)
        return;

    int aShadowType = 0;
    float aShadowOffsetX = -3.0f;
    float aShadowOffsetY = 51.0f;
    float aScale = 1.0f;
    if (mBoard && mBoard->StageIsNight())
    {
        aShadowType = 1;
    }

    if (mSeedType == SeedType::SEED_SQUASH)
    {
        if (mBoard)
        {
            aShadowOffsetY += mBoard->GridToPixelY(mPlantCol, mRow) - mY;
        }
        aShadowOffsetY += 5.0f;
    }
    else if (mSeedType == SeedType::SEED_PUFFSHROOM)
    {
        aScale = 0.5f;
        aShadowOffsetY = 42.0f;
    }
    else if (mSeedType == SeedType::SEED_SUNSHROOM)
    {
        aShadowOffsetY = 42.0f;
        if (mState == PlantState::STATE_SUNSHROOM_SMALL)
        {
            aScale = 0.5f;
        }
        else if (mState == PlantState::STATE_SUNSHROOM_GROWING)
        {
            Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
            aScale = 0.5f + 0.5f * aBodyReanim->mAnimTime;
        }
    }
    else if (mSeedType == SeedType::SEED_UMBRELLA)
    {
        aScale = 0.5f;
        aShadowOffsetX = -7.0f;
        aShadowOffsetY = 52.0f;
    }
    else if (mSeedType == SeedType::SEED_FUMESHROOM || mSeedType == SeedType::SEED_GLOOMSHROOM)
    {
        aScale = 1.3f;
        aShadowOffsetY = 47.0f;
    }
    else if (mSeedType == SeedType::SEED_CABBAGEPULT || mSeedType == SeedType::SEED_MELONPULT || mSeedType == SeedType::SEED_WINTERMELON)
    {
        aShadowOffsetY = 47.0f;
    }
    else if (mSeedType == SeedType::SEED_KERNELPULT)
    {
        aShadowOffsetX = 0.0f;
        aShadowOffsetY = 47.0f;
    }
    else if (mSeedType == SeedType::SEED_SCAREDYSHROOM)
    {
        aShadowOffsetX = -9.0f;
        aShadowOffsetY = 55.0f;
    }
    else if (mSeedType == SeedType::SEED_CHOMPER)
    {
        aShadowOffsetX = -21.0f;
        aShadowOffsetY = 57.0f;
    }
    else if (mSeedType == SeedType::SEED_FLOWERPOT)
    {
        aShadowOffsetX = -4.0f;
        aShadowOffsetY = 46.0f;
    }
    else if (mSeedType == SeedType::SEED_TALLNUT)
    {
        aShadowOffsetY = 54.0f;
        aScale = 1.3f;
    }
    else if (mSeedType == SeedType::SEED_PUMPKINSHELL)
    {
        aShadowOffsetY = 46.0f;
        aScale = 1.4f;
    }
    else if (mSeedType == SeedType::SEED_CACTUS)
    {
        aShadowOffsetX = -8.0f;
        aShadowOffsetY = 50.0f;
    }
    else if (mSeedType == SeedType::SEED_PLANTERN)
    {
        aShadowOffsetY = 57.0f;
    }
    else if (mSeedType == SeedType::SEED_INSTANT_COFFEE)
    {
        aShadowOffsetY = 71.0f;
    }
    else if (mSeedType == SeedType::SEED_GIANT_WALLNUT)
    {
        aShadowOffsetX = -33.0f;
        aShadowOffsetY = 56.0f;
        aScale = 1.7f;
    }

    if (Plant::IsFlying(mSeedType))
    {
        aShadowOffsetY += 10.0f;
        if (mBoard && (mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_ONLY_NORMAL_POSITION) || mBoard->GetTopPlantAt(mPlantCol, mRow, TOPPLANT_ONLY_PUMPKIN)))
            return;
    }

    if (aShadowType == 0)
    {
        TodDrawImageCelCenterScaledF(g, IMAGE_PLANTSHADOW, theOffsetX + aShadowOffsetX, theOffsetY + aShadowOffsetY, 0, aScale, aScale);
    }
    else
    {
        TodDrawImageCelCenterScaledF(g, IMAGE_PLANTSHADOW2, theOffsetX + aShadowOffsetX, theOffsetY + aShadowOffsetY, 0, aScale, aScale);
    }
}

// GOTY @Patoke: 0x469500
void Plant::Draw(Graphics* g)
{
    if (mSeedType == SeedType::SEED_GIFT_BOX)
    {
        Image* image = mStateCountdown > 20 ? IMAGE_PRESENT : IMAGE_PRESENTOPEN;
        const float scale = 0.75f;
        TodDrawImageScaledF(g, image, 40.0f - image->GetWidth() * scale / 2.0f,
            48.0f - image->GetHeight() * scale / 2.0f, scale, scale);
        return;
    }
    if (mSeedType == SeedType::SEED_DOOMPEA && mDoomPeaAngle > 0)
    {
        int activeBeams = 1;
        Plant* anotherPlant = nullptr;
        while (mBoard->IteratePlants(anotherPlant))
        {
            if (anotherPlant != this && anotherPlant->mSeedType == SeedType::SEED_DOOMPEA && anotherPlant->mDoomPeaAngle > 0)
                ++activeBeams;
        }
        // Choose the per-beam alpha from the compositing equation so N
        // overlapping beams have approximately the same opacity as one beam.
        // A simple 160 / N division still gets darker as beams accumulate.
        const auto sharedAlpha = [activeBeams](int targetAlpha) {
            const float target = static_cast<float>(targetAlpha) / 255.0f;
            const float perBeam = 1.0f - std::pow(1.0f - target, 1.0f / static_cast<float>(activeBeams));
            return ClampInt(static_cast<int>(std::lround(perBeam * 255.0f)), 1, 255);
        };
        const int beamAlpha = sharedAlpha(160);
        const int edgeAlpha = sharedAlpha(45);
        g->SetColor(Color(0, 0, 0, beamAlpha));
        // BeginDraw() already translates the graphics context by (mX, mY).
        // Use the same animated pea-head transform as Fire(), so the beam
        // starts exactly at the plant's mouth instead of at a board-relative
        // position that changes with the row and camera.
        int peaHeadOffsetX = 0;
        int peaHeadOffsetY = 0;
        GetPeaHeadOffset(peaHeadOffsetX, peaHeadOffsetY);
        const int originX = peaHeadOffsetX + 24;
        const int originY = peaHeadOffsetY - 33;
        const float radians = static_cast<float>(mDoomPeaAngle) * 3.14159265f / 180.0f;
        const int endX = std::max(originX + 1, BOARD_WIDTH - mX);
        // Rasterize the fan as filled vertical slices. This gives a solid
        // cone without visible spokes or a stack of thin rays.
        const int sliceWidth = 2;
        // A soft one-pixel edge reduces stair-stepping on the cone boundary.
        g->SetColor(Color(0, 0, 0, edgeAlpha));
        for (int x = originX; x < endX; x += sliceWidth)
        {
            const int sliceEndX = std::min(endX, x + sliceWidth);
            const int sliceHalfHeight = static_cast<int>((sliceEndX - originX) * std::tan(radians));
            g->FillRect(x, originY - sliceHalfHeight - 1, sliceEndX - x, sliceHalfHeight * 2 + 3);
        }
        g->SetColor(Color(0, 0, 0, beamAlpha));
        for (int x = originX; x < endX; x += sliceWidth)
        {
            const int sliceEndX = std::min(endX, x + sliceWidth);
            const int sliceHalfHeight = static_cast<int>((sliceEndX - originX) * std::tan(radians));
            g->FillRect(x, originY - sliceHalfHeight, sliceEndX - x, sliceHalfHeight * 2 + 1);
        }
    }
    if (mSeedType == SeedType::SEED_DOOMPEA && mDoomPeaPrimaryFlash > 0)
    {
        int peaHeadOffsetX = 0;
        int peaHeadOffsetY = 0;
        GetPeaHeadOffset(peaHeadOffsetX, peaHeadOffsetY);
        const int originX = peaHeadOffsetX + 24;
        const int originY = peaHeadOffsetY - 33;
        const int fullEndX = std::max(originX + 1, BOARD_WIDTH - mX);
        const int elapsed = 24 - mDoomPeaPrimaryFlash;
        // The beam takes ten frames to travel out, holds briefly, then fades
        // over eight frames. This gives it the soft, readable timing of the
        // game's normal beam attacks instead of an instant full-width bar.
        const float grow = std::min(1.0f, elapsed / 10.0f);
        const float fade = elapsed > 16 ? std::max(0.0f, (24.0f - elapsed) / 8.0f) : 1.0f;
        const int endX = originX + std::max(1, static_cast<int>((fullEndX - originX) * grow));
        const int glowAlpha = ClampInt(static_cast<int>(70.0f * fade), 0, 255);
        const int coreAlpha = ClampInt(static_cast<int>(220.0f * fade), 0, 255);

        // Draw short segments with a gentle falloff toward the moving tip;
        // this removes the hard vertical edge of a single filled rectangle.
        const int segmentWidth = 8;
        for (int x = originX; x < endX; x += segmentWidth)
        {
            const int segmentEnd = std::min(endX, x + segmentWidth);
            const float tipFade = static_cast<float>(endX - x) / std::max(1, endX - originX);
            const int segmentGlow = ClampInt(static_cast<int>(glowAlpha * (0.45f + 0.55f * tipFade)), 0, 255);
            const int segmentCore = ClampInt(static_cast<int>(coreAlpha * (0.55f + 0.45f * tipFade)), 0, 255);
            g->SetColor(Color(20, 20, 20, segmentGlow));
            g->FillRect(x, originY - 4, segmentEnd - x, 9);
            g->SetColor(Color(115, 115, 115, segmentCore));
            g->FillRect(x, originY - 1, segmentEnd - x, 3);
        }
        const int muzzleAlpha = ClampInt(static_cast<int>(200.0f * fade), 0, 255);
        g->SetColor(Color(220, 220, 220, muzzleAlpha));
        g->FillRect(originX, originY, std::min(6, endX - originX), 1);
        g->SetColor(Color(180, 180, 180, muzzleAlpha / 2));
        g->FillRect(originX - 2, originY - 2, 5, 5);
    }
    float aOffsetX = 0.0f;
    float aOffsetY = PlantDrawHeightOffset(mBoard, this, mSeedType, mPlantCol, mRow);
    if (Plant::IsFlying(mSeedType) && mSquished)
    {
        aOffsetY += 30.0f;
    }

    int aImageIndex = mFrame;
    Image* aPlantImage = Plant::GetImage(mSeedType);

    if (mSquished)
    {
        if (mSeedType == SeedType::SEED_FLOWERPOT)
        {
            aOffsetY -= 15.0f;
        }
        if (mSeedType == SeedType::SEED_INSTANT_COFFEE)
        {
            aOffsetY -= 20.0f;
        }

        g->SetScale(1.0f, 0.25f, 0.0f, 0.0f);
        DrawSeedType(g, mSeedType, mImitaterType, DrawVariation::VARIATION_NORMAL, aOffsetX, 60.0f + aOffsetY);
        g->SetScale(1.0f, 1.0f, 0.0f, 0.0f);
    }
    else
    {
        bool aDrawPumpkinBack = false;
        Plant* aPumpkin = nullptr;

        if (IsOnBoard())
        {
            aPumpkin = mBoard->GetPumpkinAt(mPlantCol, mRow);
            if (aPumpkin)
            {
                Plant* aPlantInPumpkin = mBoard->GetTopPlantAt(mPlantCol, mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
                if (aPlantInPumpkin)
                {
                    if (aPlantInPumpkin->mRenderOrder > aPumpkin->mRenderOrder || aPlantInPumpkin->mOnBungeeState == GETTING_GRABBED_BY_BUNGEE)
                    {
                        aPlantInPumpkin = nullptr;
                    }
                }

                if (aPlantInPumpkin == this)
                {
                    aDrawPumpkinBack = true;
                }
                if (aPlantInPumpkin == nullptr && mSeedType == SeedType::SEED_PUMPKINSHELL)
                {
                    aDrawPumpkinBack = true;
                }
            }
            else if (mSeedType == SeedType::SEED_PUMPKINSHELL)
            {
                aDrawPumpkinBack = true;
                aPumpkin = this;
            }
        }
        else if (mSeedType == SeedType::SEED_PUMPKINSHELL)
        {
            aDrawPumpkinBack = true;
            aPumpkin = this;
        }

        DrawShadow(g, aOffsetX, aOffsetY);

        if (Plant::IsFlying(mSeedType))
        {
            uint32_t aCounter = IsOnBoard() ? mBoard->mMainCounter : mApp->mAppCounter;

            float aTime = static_cast<float>(fmod((mRow * 97.0 + mPlantCol * 61.0 + static_cast<double>(aCounter)) * 0.03, 2.0 * PI));
            float aWave = sin(aTime) * 2.0f;
            aOffsetY += aWave;
        }

        if (aDrawPumpkinBack)
        {
            Reanimation* aPumpkinReanim = mApp->ReanimationGet(aPumpkin->mBodyReanimID);
            Graphics aPumpkinGraphics(*g);
            aPumpkinGraphics.mTransX += aPumpkin->mX - mX;
            aPumpkinGraphics.mTransY += aPumpkin->mY - mY;
            aPumpkinReanim->DrawRenderGroup(&aPumpkinGraphics, 1);
        }

        aOffsetX += mShakeOffsetX;
        aOffsetY += mShakeOffsetY;
        if (IsInPlay() && mApp->IsIZombieLevel())
        {
            mBoard->mChallenge->IZombieDrawPlant(g, this);
        }
        else if (mBodyReanimID != ReanimationID::REANIMATIONID_NULL)
        {
            Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
            if (aBodyReanim)
            {
                if (!mApp->Is3DAccelerated() && mSeedType == SeedType::SEED_FLOWERPOT && IsOnBoard() && 
                    aBodyReanim->mAnimRate == 0.0f && aBodyReanim->IsAnimPlaying("anim_idle"))
                {
                    mApp->mReanimatorCache->DrawCachedPlant(g, aOffsetX, aOffsetY, mSeedType, DrawVariation::VARIATION_NORMAL);
                }
                else
                {
                    aBodyReanim->Draw(g);
                }
            }
        }
        else
        {
            SeedType aSeedType = SeedType::SEED_NONE;
            if (mBoard)
            {
                aSeedType = mBoard->GetSeedTypeInCursor();
            }

            if (IsPartOfUpgradableTo(aSeedType) && mBoard->CanPlantAt(mPlantCol, mRow, aSeedType) == PlantingReason::PLANTING_OK)
            {
                g->SetColorizeImages(true);
                g->SetColor(GetFlashingColor(mBoard->mMainCounter, 90));
            }
            else if (aSeedType == SeedType::SEED_COBCANNON && mBoard->CanPlantAt(mPlantCol - 1, mRow, aSeedType) == PlantingReason::PLANTING_OK)
            {
                g->SetColorizeImages(true);
                g->SetColor(GetFlashingColor(mBoard->mMainCounter, 90));
            }
            else if (mBoard && mBoard->mTutorialState == TutorialState::TUTORIAL_SHOVEL_DIG)
            {
                g->SetColorizeImages(true);
                g->SetColor(GetFlashingColor(mBoard->mMainCounter, 90));
            }

            TodDrawImageCelF(g, aPlantImage, aOffsetX, aOffsetY, aImageIndex, 0);
            g->SetColorizeImages(false);
            if (mHighlighted)
            {
                g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
                g->SetColorizeImages(true);
                g->SetColor(Color(255, 255, 255, 196));
                TodDrawImageCelF(g, aPlantImage, aOffsetX, aOffsetY, aImageIndex, 0);
                g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
                g->SetColorizeImages(false);
            }
            else if (mEatenFlashCountdown > 0)
            {
                g->SetDrawMode(Graphics::DRAWMODE_ADDITIVE);
                g->SetColorizeImages(true);
                g->SetColor(Color(255, 255, 255, ClampInt(mEatenFlashCountdown * 3, 0, 255)));
                TodDrawImageCelF(g, aPlantImage, aOffsetX, aOffsetY, aImageIndex, 0);
                g->SetDrawMode(Graphics::DRAWMODE_NORMAL);
                g->SetColorizeImages(false);
            }

        }

        // Doom Pea's permanent visual identity: a restrained RGB split drawn
        // in horizontal bands over the plant silhouette.  Draw the actual
        // reanimations through clipped strips so transparent pixels remain
        // transparent and no rectangular overlay appears around the plant.
        if ((mSeedType == SeedType::SEED_DOOMPEA || mErrorCountdown > 0) && mBodyReanimID != ReanimationID::REANIMATIONID_NULL)
        {
            Reanimation* aReanimations[] = {
                mApp->ReanimationTryToGet(mBodyReanimID),
                mApp->ReanimationTryToGet(mHeadReanimID),
                mApp->ReanimationTryToGet(mHeadReanimID2),
                mApp->ReanimationTryToGet(mHeadReanimID3)
            };
            const float savedTransX = g->mTransX;
            const float savedTransY = g->mTransY;
            const int aCounter = mBoard ? mBoard->mMainCounter : mApp->mAppCounter;
            const int aPhase = (aCounter / 3) % 5;
            constexpr int aStripHeight = 7;
            constexpr int aStripStep = 11;

            int aStripIndex = 0;
            for (int aY = 4 + aPhase; aY < 78; aY += aStripStep, ++aStripIndex)
            {
                Graphics aStripGraphics(*g);
                aStripGraphics.ClipRect(-18, aY, 120, aStripHeight);

                // Digital glitch movement is stepped, not sinusoidal: hold a
                // displacement for a few frames, then jump to a new one.
                // The per-strip hash keeps the bands from moving in lockstep.
                const uint32_t aGlitchTick = static_cast<uint32_t>(aCounter / 3);
                uint32_t aHash = aGlitchTick * 1664525u +
                    static_cast<uint32_t>(aStripIndex) * 1013904223u + 0x9E3779B9u;
                aHash ^= aHash >> 16;
                aHash *= 2246822519u;
                aHash ^= aHash >> 13;
                const int aJitter = 3 + static_cast<int>(aHash % 6u); // 3-8 px

                // Draw clipped, offset silhouettes for the error state. The
                // clip keeps transparent pixels transparent while the larger
                // displacement and stronger alpha make the RGB separation
                // readable against bright plant sprites.
                if (mErrorCountdown > 0)
                {
                    for (Reanimation* aReanim : aReanimations)
                    {
                        if (aReanim == nullptr)
                            continue;
                        const Color aSavedColor = aReanim->mColorOverride;
                        const Color aSavedAdditive = aReanim->mExtraAdditiveColor;
                        const bool aSavedExtra = aReanim->mEnableExtraAdditiveDraw;
                        aReanim->mExtraAdditiveColor = Color::Black;
                        aReanim->mEnableExtraAdditiveDraw = false;
                        aStripGraphics.mTransX = savedTransX - aJitter;
                        aReanim->mColorOverride = Color(255, 25, 45, 150);
                        aReanim->Draw(&aStripGraphics);
                        aStripGraphics.mTransX = savedTransX + aJitter;
                        aReanim->mColorOverride = Color(30, 90, 255, 150);
                        aReanim->Draw(&aStripGraphics);
                        aReanim->mColorOverride = aSavedColor;
                        aReanim->mExtraAdditiveColor = aSavedAdditive;
                        aReanim->mEnableExtraAdditiveDraw = aSavedExtra;
                    }
                }

                // Independent screen-space glitch fragments. These are short
                // RGB blocks rather than scanlines or warped reanimation
                // copies, so plant sway cannot bend the effect.
                const int blockX = 24 + ((int)aHash % 5) * 9;
                const int blockW = 8 + (int)((aHash >> 8) % 16);
                const int blockY = aY;
                g->SetColor(Color(255, 35, 45, mErrorCountdown > 0 ? 180 : 125));
                g->FillRect(blockX - aJitter, blockY, blockW, 2);
                g->SetColor(Color(35, 100, 255, mErrorCountdown > 0 ? 180 : 125));
                g->FillRect(blockX + aJitter, blockY + 2, blockW, 2);
                g->SetColor(Color(50, 255, 120, mErrorCountdown > 0 ? 110 : 75));
                g->FillRect(blockX, blockY + 4, blockW / 2, 1);
                /*
                for (Reanimation* aReanim : aReanimations)
                {
                    if (aReanim == nullptr) continue;

                    const Color aSavedColor = aReanim->mColorOverride;
                    const Color aSavedAdditive = aReanim->mExtraAdditiveColor;
                    const bool aSavedExtra = aReanim->mEnableExtraAdditiveDraw;
                    aReanim->mExtraAdditiveColor = Color::Black;
                    aReanim->mEnableExtraAdditiveDraw = false;

                    // Red and blue channels are separated horizontally. A
                    // faint green center keeps the effect readable without
                    // turning the whole plant into a bright color wash.
                    aStripGraphics.mTransX = savedTransX - aJitter;
                    aReanim->mColorOverride = Color(255, 35, 35, 82);
                    aReanim->Draw(&aStripGraphics);
                    aStripGraphics.mTransX = savedTransX + aJitter;
                    aReanim->mColorOverride = Color(35, 90, 255, 82);
                    aReanim->Draw(&aStripGraphics);

                    if (((aY + 20) / aStripStep) % 2 == 0)
                    {
                        aStripGraphics.mTransX = savedTransX;
                        aReanim->mColorOverride = Color(50, 255, 120, 38);
                        aReanim->Draw(&aStripGraphics);
                    }

                    aStripGraphics.mTransX = savedTransX;
                    aReanim->mColorOverride = aSavedColor;
                    aReanim->mExtraAdditiveColor = aSavedAdditive;
                    aReanim->mEnableExtraAdditiveDraw = aSavedExtra;
                }
                */
            }
            g->mTransX = savedTransX;
            g->mTransY = savedTransY;
        }

        if (mSeedType == SeedType::SEED_MAGNETSHROOM && !DrawMagnetItemsOnTop())
        {
            DrawMagnetItems(g);
        }

        if (mSeedType == SeedType::SEED_DOOMPEA)
        {
            const int indicatorHeight = 72;
            const int fill = mDoomPeaRestore > 0
                ? (mDoomPeaRestore * indicatorHeight / 240)
                : ((1500 - std::max(0, mDoomPeaCounter)) * indicatorHeight / 1500);
            const int clampedFill = ClampInt(fill, 0, indicatorHeight);
            if (clampedFill > 0)
            {
                // Render the actual plant again with the black filter, clipped
                // to the progress region. This preserves the plant silhouette
                // instead of painting a solid rectangular block over the lawn.
                Graphics aMaskGraphics(*g);
                // The plant-local bottom is around y=72; keep the fill
                // anchored there so charging rises from the feet upward.
                aMaskGraphics.ClipRect(-8, 72 - clampedFill, 96, clampedFill);
                Reanimation* aReanimations[] = {
                    mApp->ReanimationTryToGet(mBodyReanimID),
                    mApp->ReanimationTryToGet(mHeadReanimID),
                    mApp->ReanimationTryToGet(mHeadReanimID2),
                    mApp->ReanimationTryToGet(mHeadReanimID3)
                };
                // Do not use FILTER_EFFECT_BLACK here. That filter is a
                // post-process rectangle for reanimations, so clipping it
                // produces the visible black bars under the plant. A black
                // color override is alpha safe and follows the sprite's
                // actual pixels.
                Color aOldColors[] = {
                    Color::White,
                    Color::White,
                    Color::White,
                    Color::White
                };
                for (int i = 0; i < 4; ++i)
                {
                    if (aReanimations[i] == nullptr)
                        continue;
                    aOldColors[i] = aReanimations[i]->mColorOverride;
                    aReanimations[i]->mColorOverride = Color(0, 0, 0, 210);
                    aReanimations[i]->Draw(&aMaskGraphics);
                    aReanimations[i]->mColorOverride = aOldColors[i];
                }
            }
        }

		// Keep the combat readout to repeatable direct attacks. Multi-shot rate,
		// splash and status effects are intentionally not folded into this value.
		int aBaseDamage = 0;
		switch (mSeedType)
		{
		case SeedType::SEED_PEASHOOTER:
		case SeedType::SEED_IRONPEASHOOTER:
		case SeedType::SEED_SNOWPEA:
		case SeedType::SEED_REPEATER:
		case SeedType::SEED_PUFFSHROOM:
		case SeedType::SEED_FUMESHROOM:
		case SeedType::SEED_SCAREDYSHROOM:
		case SeedType::SEED_SEASHROOM:
		case SeedType::SEED_THREEPEATER:
		case SeedType::SEED_STARFRUIT:
		case SeedType::SEED_SPLITPEA:
		case SeedType::SEED_CACTUS:
		case SeedType::SEED_GATLINGPEA:
		case SeedType::SEED_CATTAIL:
		case SeedType::SEED_LEFTPEATER:
		case SeedType::SEED_HYPNOSHROOM:
			aBaseDamage = 20;
			break;
		case SeedType::SEED_CABBAGEPULT:
			aBaseDamage = 40;
			break;
		case SeedType::SEED_KERNELPULT:
			aBaseDamage = 20;
			break;
		case SeedType::SEED_MELONPULT:
		case SeedType::SEED_WINTERMELON:
			aBaseDamage = 80;
			break;
		case SeedType::SEED_COBCANNON:
			aBaseDamage = 300;
			break;
		default:
			break;
		}
		if (aBaseDamage > 0)
		{
			int aMultiplier = mApp->IsCricketMode() ? mBoard->mChallenge->CricketDamageMultiplierPercent(mRow) : 100;
			std::string aDamageText = StrFormat("%d", std::min(1000, aBaseDamage * aMultiplier / 100));
			int aTextWidth = FONT_BRIANNETOD12->StringWidth(aDamageText);
			int aTextRight = static_cast<int>(aOffsetX) + 78;
			int aTextBase = static_cast<int>(aOffsetY) + 79;
			g->SetColor(Color(242, 242, 232, 240));
			g->FillRect(Rect(aTextRight - aTextWidth - 3, aTextBase - 12, aTextWidth + 6, 15));
			g->SetColor(Color(28, 28, 24, 255));
			g->DrawRect(Rect(aTextRight - aTextWidth - 3, aTextBase - 12, aTextWidth + 6, 15));
			TodDrawString(g, aDamageText, aTextRight, aTextBase,
				FONT_BRIANNETOD12, Color(20, 20, 18), DS_ALIGN_RIGHT);
		}
    }
}

// GOTY @Patoke: 0x469AF0
void Plant::DrawSeedType(Graphics* g, SeedType theSeedType, SeedType theImitaterType, DrawVariation theDrawVariation, float thePosX, float thePosY)
{
    if (theSeedType == SeedType::SEED_GIFT_BOX)
    {
        TodDrawImageScaledF(g, IMAGE_PRESENT,
            thePosX + 40.0f - IMAGE_PRESENT->GetWidth() * 0.7f / 2.0f,
            thePosY + 42.0f - IMAGE_PRESENT->GetHeight() * 0.7f / 2.0f, 0.7f, 0.7f);
        return;
    }
    Graphics aSeedG(*g);
    int aCelRow = 0;
    int aCelCol = 2;
    float aOffsetX = 0.0f;
    float aOffsetY = 0.0f;
    SeedType aSeedType = theSeedType;
    DrawVariation aDrawVariation = theDrawVariation;

    if (theSeedType == SeedType::SEED_IMITATER && theImitaterType != SeedType::SEED_NONE)
    {
        aSeedType = theImitaterType;
        aDrawVariation = DrawVariation::VARIATION_IMITATER;
        if (theImitaterType == SeedType::SEED_HYPNOSHROOM || theImitaterType == SeedType::SEED_SQUASH || theImitaterType == SeedType::SEED_POTATOMINE ||
            theImitaterType == SeedType::SEED_GARLIC || theImitaterType == SeedType::SEED_LILYPAD)
            aDrawVariation = DrawVariation::VARIATION_IMITATER_LESS;
    }
    else if (theDrawVariation == DrawVariation::VARIATION_NORMAL && theSeedType == SeedType::SEED_TANGLEKELP)
    {
        aDrawVariation = DrawVariation::VARIATION_AQUARIUM;
    }
    else if (theSeedType == SeedType::SEED_IRONPEASHOOTER)
    {
        aDrawVariation = DrawVariation::VARIATION_IRON;
    }
    else if (theSeedType == SeedType::SEED_DOOMPEA)
    {
        aDrawVariation = DrawVariation::VARIATION_BLACK;
    }

    if (((LawnApp*)gSexyAppBase)->mGameMode == GameMode::GAMEMODE_CHALLENGE_BIG_TIME &&
        (aSeedType == SeedType::SEED_WALLNUT || aSeedType == SeedType::SEED_SUNFLOWER || aSeedType == SeedType::SEED_MARIGOLD))
    {
        aSeedG.mScaleX *= 1.5f;
        aSeedG.mScaleY *= 1.5f;
        aOffsetX = -20.0f;
        aOffsetY = -40.0f;
    }
    if (aSeedType == SeedType::SEED_LEFTPEATER)
    {
        aOffsetX += aSeedG.mScaleX * 80.0f;
        aSeedG.mScaleX *= -1.0f;
    }

    if (Challenge::IsZombieSeedType(aSeedType))
    {
        ZombieType aZombieType = Challenge::IZombieSeedTypeToZombieType(aSeedType);
        if (aZombieType == ZombieType::ZOMBIE_DANCER)
        {
            aSeedG.mScaleX *= 0.8f;
            aSeedG.mScaleY *= 0.8f;
            aOffsetX = 20.0f;
            aOffsetY = 42.0f;
        }
        gLawnApp->mReanimatorCache->DrawCachedZombie(&aSeedG, thePosX + aOffsetX, thePosY + aOffsetY, aZombieType, aSeedType == SeedType::SEED_ZOMBIE_CART);
    }
    else
    {
        const PlantDefinition& aPlantDef = GetPlantDefinition(aSeedType);

        if (aSeedType == SeedType::SEED_GIANT_WALLNUT)
        {
            aSeedG.mScaleX *= 1.4f;
            aSeedG.mScaleY *= 1.4f;
            TodDrawImageScaledF(&aSeedG, IMAGE_REANIM_WALLNUT_BODY, thePosX - 53.0f, thePosY - 56.0f, aSeedG.mScaleX, aSeedG.mScaleY);
        }
        else if (aPlantDef.mReanimationType != ReanimationType::REANIM_NONE)
        {
            gLawnApp->mReanimatorCache->DrawCachedPlant(&aSeedG, thePosX + aOffsetX, thePosY + aOffsetY, aSeedType, aDrawVariation);
        }
        else
        {
            if (aSeedType == SeedType::SEED_KERNELPULT)
            {
                aCelRow = 2;
            }
            else if (aSeedType == SeedType::SEED_TWINSUNFLOWER)
            {
                aCelRow = 1;
            }

            Image* aPlantImage = Plant::GetImage(aSeedType);
            if (aPlantImage->mNumCols <= 2)
            {
                aCelCol = aPlantImage->mNumCols - 1;
            }

            TodDrawImageCelScaledF(&aSeedG, aPlantImage, thePosX + aOffsetX, thePosY + aOffsetY, aCelCol, aCelRow, aSeedG.mScaleX, aSeedG.mScaleY);
        }
    }
}

void Plant::MouseDown(int x, int y, int theClickCount)
{
    if (theClickCount < 0)
        return;

    if (mState == PlantState::STATE_COBCANNON_READY)
    {
        mBoard->ClearCursor();
        mBoard->mCursorObject->mType = SeedType::SEED_NONE;
        mBoard->mCursorObject->mCursorType = CursorType::CURSOR_TYPE_COBCANNON_TARGET;
        mBoard->mCursorObject->mSeedBankIndex = -1;
        mBoard->mCursorObject->mCoinID = CoinID::COINID_NULL;
        mBoard->mCursorObject->mCobCannonPlantID = (PlantID)mBoard->mPlants.DataArrayGetID(this);
        mBoard->mCobCannonCursorDelayCounter = 30;
        mBoard->mCobCannonMouseX = x;
        mBoard->mCobCannonMouseY = y;
    }
}

void Plant::IceZombies()
{
    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        aZombie->HitIceTrap();
    }

    mBoard->mIceTrapCounter = 300;
    TodParticleSystem* aPoolSparklyParticle = mApp->ParticleTryToGet(mBoard->mPoolSparklyParticleID);
    if (aPoolSparklyParticle)
    {
        aPoolSparklyParticle->mDontUpdate = false;
    }

    Zombie* aBossZombie = mBoard->GetBossZombie();
    if (aBossZombie)
    {
        aBossZombie->BossDestroyFireball();
    }
}

void Plant::BurnRow(int theRow)
{
    int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        if ((aZombie->mZombieType == ZombieType::ZOMBIE_BOSS || aZombie->mRow == theRow) && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            aZombie->RemoveColdEffects();
            aZombie->ApplyBurn(GetAttackDamage(1800));
        }
    }

    GridItem* aGridItem = nullptr;
    while (mBoard->IterateGridItems(aGridItem))
    {
        if (aGridItem->mGridY == theRow && aGridItem->mGridItemType == GridItemType::GRIDITEM_LADDER)
        {
            aGridItem->GridItemDie();
        }
    }

    Zombie* aBossZombie = mBoard->GetBossZombie();
    if (aBossZombie && aBossZombie->mFireballRow == theRow)
    {
        // 注：原版中将 Zombie::BossDestroyIceballInRow(int) 函数改为了 Zombie::BossDestroyIceball()，冰球是否位于目标行的判断则移动至此处进行
        aBossZombie->BossDestroyIceballInRow();
    }
}

void Plant::BlowAwayFliers()
{
    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        if (!aZombie->IsDeadOrDying())
        {
            // Verified as a pure function, safe to remove
            // Rect aZombieRect = aZombie->GetZombieRect();
            if (aZombie->IsFlying())
            {
                aZombie->mBlowingAway = true;
            }
        }
    }

    mApp->PlaySample(SOUND_BLOVER);
    mBoard->mFogBlownCountDown = 4000;
}

void Plant::KillAllPlantsNearDoom()
{
    Plant* aPlant = nullptr;
    while (mBoard->IteratePlants(aPlant))
    {
        if (aPlant->mRow == mRow && aPlant->mPlantCol == mPlantCol)
        {
            aPlant->Die();
        }
    }
}

// GOTY @Patoke: 0x46A110
void Plant::DoSpecial()
{
    int aPosX = mX + mWidth / 2;
    int aPosY = mY + mHeight / 2;
    int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);

    if (mApp->IsWhatRulesMode())
    {
        if (mRuleId == PLANT_RULE_DUD && Rand(100) < 15)
        {
            Die();
            return;
        }
        if (mRuleId == PLANT_RULE_FRIENDLY_FIRE)
        {
            Plant* aPlant = nullptr;
            while (mBoard->IteratePlants(aPlant))
            {
                if (aPlant == this || aPlant->mDead || aPlant->NotOnGround())
                    continue;
                if (std::abs(aPlant->mPlantCol - mPlantCol) <= 1 && std::abs(aPlant->mRow - mRow) <= 1)
                    aPlant->mPlantHealth -= 50;
            }
        }
        if (mRuleId == PLANT_RULE_BIG_BLAST)
            mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 160, 3, false, aDamageRangeFlags, 100);
        if (mRuleId == PLANT_RULE_AFTERMATH)
            mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 115, 1, false, aDamageRangeFlags, 50);
    }

    switch (mSeedType)
    {
    case SeedType::SEED_BLOVER:
    {
        if (mState != PlantState::STATE_DOINGSPECIAL)
        {
            BlowAwayFliers();
        }
        break;
    }
    case SeedType::SEED_CHERRYBOMB:
    {
        mApp->PlayFoley(FoleyType::FOLEY_CHERRYBOMB);
        mApp->PlayFoley(FoleyType::FOLEY_JUICY);

        if (mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 115, 1, true, aDamageRangeFlags, GetAttackDamage(1800)) >= 10)
            ReportAchievement::GiveAchievement(mApp, Explodonator, true); // @Patoke: add achievement

        mApp->AddTodParticle(aPosX, aPosY, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_POWIE);
        mBoard->ShakeBoard(3, -4);

        Die();
        break;
    }
    case SeedType::SEED_DOOMSHROOM:
    {
        mApp->PlaySample(SOUND_DOOMSHROOM);

        mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 250, 3, true, aDamageRangeFlags, GetAttackDamage(1800));
        KillAllPlantsNearDoom();

        mApp->AddTodParticle(aPosX, aPosY, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_DOOM);
        mBoard->AddACrater(mPlantCol, mRow)->mGridItemCounter = 18000;
        mBoard->ShakeBoard(3, -4);

        Die();
        break;
    }
    case SeedType::SEED_JALAPENO:
    {
        mApp->PlayFoley(FoleyType::FOLEY_JALAPENO_IGNITE);
        mApp->PlayFoley(FoleyType::FOLEY_JUICY);

        mBoard->DoFwoosh(mRow);
        mBoard->ShakeBoard(3, -4);

        BurnRow(mRow);
        mBoard->mIceTimer[mRow] = 20;

        Die();
        break;
    }
    case SeedType::SEED_UMBRELLA:
    {
        if (mState != PlantState::STATE_UMBRELLA_TRIGGERED && mState != PlantState::STATE_UMBRELLA_REFLECTING)
        {
            mState = PlantState::STATE_UMBRELLA_TRIGGERED;
            mStateCountdown = 5;

            PlayBodyReanim("anim_block", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 22.0f);
        }

        break;
    }
    case SeedType::SEED_ICESHROOM:
    {
        mApp->PlayFoley(FoleyType::FOLEY_FROZEN);
        IceZombies();
        mApp->AddTodParticle(aPosX, aPosY, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_ICE_TRAP);

        Die();
        break;
    }
    case SeedType::SEED_POTATOMINE:
    {
        aPosX = mX + mWidth / 2 - 20;
        aPosY = mY + mHeight / 2;

        mApp->PlaySample(SOUND_POTATO_MINE);
        if (mBoard->KillAllZombiesInRadius(mRow, aPosX, aPosY, 60, 0, false, aDamageRangeFlags, GetAttackDamage(1800)) >= 1)
            ReportAchievement::GiveAchievement(mApp, Spudow, true); // @Patoke: add achievement

        int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PARTICLE, mRow, 0);
        mApp->AddTodParticle(aPosX + 20.0f, aPosY, aRenderPosition, ParticleEffect::PARTICLE_POTATO_MINE);
        mBoard->ShakeBoard(3, -4);

        Die();
        break;
    }
    case SeedType::SEED_INSTANT_COFFEE:
    {
        Plant* aPlant = mBoard->GetTopPlantAt(mPlantCol, mRow, PlantPriority::TOPPLANT_ONLY_NORMAL_POSITION);
        if (aPlant && aPlant->mIsAsleep)
        {
            aPlant->mWakeUpCounter = 100;
        }

        mState = PlantState::STATE_DOINGSPECIAL;
        PlayBodyReanim("anim_crumble", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 22.0f);
        mApp->PlayFoley(FoleyType::FOLEY_COFFEE);

        break;
    }
    default:
        break;
    }
}

void Plant::ImitaterMorph()
{
    Die();
    Plant* aPlant = mBoard->AddPlant(mPlantCol, mRow, mImitaterType, SeedType::SEED_IMITATER);

    FilterEffect aFilter = FilterEffect::FILTER_EFFECT_WASHED_OUT;
    if (mImitaterType == SeedType::SEED_HYPNOSHROOM || mImitaterType == SeedType::SEED_SQUASH || mImitaterType == SeedType::SEED_POTATOMINE ||
        mImitaterType == SeedType::SEED_GARLIC || mImitaterType == SeedType::SEED_LILYPAD)
        aFilter = FilterEffect::FILTER_EFFECT_LESS_WASHED_OUT;

    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(aPlant->mBodyReanimID);
    if (aBodyReanim)
    {
        aBodyReanim->mFilterEffect = aFilter;
    }
    Reanimation* aHeadReanim = mApp->ReanimationTryToGet(aPlant->mHeadReanimID);
    if (aHeadReanim)
    {
        aHeadReanim->mFilterEffect = aFilter;
    }Reanimation* aHeadReanim2 = mApp->ReanimationTryToGet(aPlant->mHeadReanimID2);
    if (aHeadReanim2)
    {
        aHeadReanim2->mFilterEffect = aFilter;
    }Reanimation* aHeadReanim3 = mApp->ReanimationTryToGet(aPlant->mHeadReanimID3);
    if (aHeadReanim3)
    {
        aHeadReanim3->mFilterEffect = aFilter;
    }
}

void Plant::UpdateImitater()
{
    if (mState != PlantState::STATE_IMITATER_MORPHING)
    {
        if (mStateCountdown == 0)
        {
            mState = PlantState::STATE_IMITATER_MORPHING;
            PlayBodyReanim("anim_explode", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 0, 26.0f);
        }
    }
    else
    {
        Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
        if (aBodyReanim->ShouldTriggerTimedEvent(0.8f))
        {
            mApp->AddTodParticle(mX + 40, mY + 40, static_cast<int>(RenderLayer::RENDER_LAYER_TOP), ParticleEffect::PARTICLE_IMITATER_MORPH);
        }
        if (aBodyReanim->mLoopCount > 0)
        {
            ImitaterMorph();
        }
    }
}

void Plant::CobCannonFire(int theTargetX, int theTargetY)
{
    TOD_ASSERT(mState == PlantState::STATE_COBCANNON_READY);

    mState = PlantState::STATE_COBCANNON_FIRING;
    mShootingCounter = 206;
    PlayBodyReanim("anim_shooting", ReanimLoopType::REANIM_PLAY_ONCE_AND_HOLD, 20, 12.0f);

    mTargetX = theTargetX - 47.0f;
    mTargetY = theTargetY;

    Reanimation* aBodyReanim = mApp->ReanimationGet(mBodyReanimID);
    ReanimatorTrackInstance* aTrackInstance = aBodyReanim->GetTrackInstanceByName("CobCannon_Cob");
    aTrackInstance->mTrackColor = Color::White;
}

void Plant::Fire(Zombie* theTargetZombie, int theRow, PlantWeapon thePlantWeapon)
{
    if (mApp->IsWhatRulesMode())
    {
        if (mRuleId == PLANT_RULE_MISFIRE && Rand(100) < 10)
            return;
        if (mRuleId == PLANT_RULE_SELF_HIT && Rand(100) < 5)
        {
            mPlantHealth -= 20;
            return;
        }
        if (mRuleId == PLANT_RULE_JAM && mRuleJam > 0)
            return;
    }

    if (mSeedType == SeedType::SEED_FUMESHROOM)
    {
        DoRowAreaDamage(20, 2U);
        mApp->PlayFoley(FoleyType::FOLEY_FUME);
        return;
    }
    if (mSeedType == SeedType::SEED_GLOOMSHROOM)
    {
        DoRowAreaDamage(20, 2U);
        return;
    }
    if (mSeedType == SeedType::SEED_STARFRUIT)
    {
        StarFruitFire();
        return;
    }

    ProjectileType aProjectileType;
    bool aIsFireIce = false;
    switch (mSeedType)
    {
    case SeedType::SEED_PEASHOOTER:
    case SeedType::SEED_REPEATER:
    case SeedType::SEED_THREEPEATER:
    case SeedType::SEED_SPLITPEA:
    case SeedType::SEED_LEFTPEATER:
        aProjectileType = ProjectileType::PROJECTILE_PEA;
        break;
    case SeedType::SEED_IRONPEASHOOTER:
        aProjectileType = ProjectileType::PROJECTILE_IRONPEA;
        break;
    case SeedType::SEED_GATLINGPEA:
        // 机枪 75% 概率射出火豆（用 aIsFireIce 触发火焰附加，否则火豆无贴图）
        aProjectileType = ProjectileType::PROJECTILE_PEA;
        if (Rand(100) < 75)
        {
            aIsFireIce = true;
        }
        break;
    case SeedType::SEED_SNOWPEA:
        if (Rand(100) < 25)
        {
            aProjectileType = ProjectileType::PROJECTILE_SNOWPEA;
            aIsFireIce = true;
        }
        else if (Rand(2) == 0)
            aProjectileType = ProjectileType::PROJECTILE_SNOWPEA_PIERCING;
        else
            aProjectileType = ProjectileType::PROJECTILE_SNOWPEA;
        break;
    case SeedType::SEED_PUFFSHROOM:
    case SeedType::SEED_SCAREDYSHROOM:
    case SeedType::SEED_SEASHROOM:
        aProjectileType = ProjectileType::PROJECTILE_PUFF;
        break;
    case SeedType::SEED_HYPNOSHROOM:
        aProjectileType = ProjectileType::PROJECTILE_HYPNOSHROOM_PEA;
        break;
    case SeedType::SEED_CACTUS:
    case SeedType::SEED_CATTAIL:
        aProjectileType = ProjectileType::PROJECTILE_SPIKE;
        break;
    case SeedType::SEED_CABBAGEPULT:
        aProjectileType = ProjectileType::PROJECTILE_CABBAGE;
        break;
    case SeedType::SEED_KERNELPULT:
        aProjectileType = ProjectileType::PROJECTILE_KERNEL;
        break;
    case SeedType::SEED_MELONPULT:
        aProjectileType = ProjectileType::PROJECTILE_MELON;
        break;
    case SeedType::SEED_WINTERMELON:
        aProjectileType = ProjectileType::PROJECTILE_WINTERMELON;
        break;
    case SeedType::SEED_COBCANNON:
        aProjectileType = ProjectileType::PROJECTILE_COBBIG;
        break;
    default:
        TOD_ASSERT(false);
        break;
    }
    if (mSeedType == SeedType::SEED_KERNELPULT && thePlantWeapon == PlantWeapon::WEAPON_SECONDARY)
    {
        aProjectileType = ProjectileType::PROJECTILE_BUTTER;
    }

    mApp->PlayFoley(FoleyType::FOLEY_THROW);
    if (mSeedType == SeedType::SEED_SNOWPEA || mSeedType == SeedType::SEED_WINTERMELON)
    {
        mApp->PlayFoley(FoleyType::FOLEY_SNOW_PEA_SPARKLES);
    }
    else if (mSeedType == SeedType::SEED_PUFFSHROOM || mSeedType == SeedType::SEED_SCAREDYSHROOM || mSeedType == SeedType::SEED_SEASHROOM)
    {
        mApp->PlayFoley(FoleyType::FOLEY_PUFF);
    }

    int aOriginX, aOriginY;
    if (mSeedType == SeedType::SEED_PUFFSHROOM)
    {
        aOriginX = mX + 40;
        aOriginY = mY + 40;
    }
    else if (mSeedType == SeedType::SEED_SEASHROOM)
    {
        aOriginX = mX + 45;
        aOriginY = mY + 63;
    }
    else if (mSeedType == SeedType::SEED_CABBAGEPULT)
    {
        aOriginX = mX + 5;
        aOriginY = mY - 12;
    }
    else if (mSeedType == SeedType::SEED_MELONPULT || mSeedType == SeedType::SEED_WINTERMELON)
    {
        aOriginX = mX + 25;
        aOriginY = mY - 46;
    }
    else if (mSeedType == SeedType::SEED_CATTAIL)
    {
        aOriginX = mX + 20;
        aOriginY = mY - 3;
    }
    else if (mSeedType == SeedType::SEED_KERNELPULT && thePlantWeapon == PlantWeapon::WEAPON_PRIMARY)
    {
        aOriginX = mX + 19;
        aOriginY = mY - 37;
    }
    else if (mSeedType == SeedType::SEED_KERNELPULT && thePlantWeapon == PlantWeapon::WEAPON_SECONDARY)
    {
        aOriginX = mX + 12;
        aOriginY = mY - 56;
    }
    else if (mSeedType == SeedType::SEED_PEASHOOTER || mSeedType == SeedType::SEED_IRONPEASHOOTER || mSeedType == SeedType::SEED_SNOWPEA || mSeedType == SeedType::SEED_REPEATER)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginX = mX + aOffsetX + 24;
        aOriginY = mY + aOffsetY - 33;
    }
    else if (mSeedType == SeedType::SEED_LEFTPEATER)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginX = mX + aOffsetX - 57;
        aOriginY = mY + aOffsetY - 33;
    }
    else if (mSeedType == SeedType::SEED_GATLINGPEA)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginX = mX + aOffsetX + 34;
        aOriginY = mY + aOffsetY - 33;
        if (mGatlingUlt)
            aOriginY += mGatlingUltYOffset;  // 仅大招豌豆做纵向散布
    }
    else if (mSeedType == SeedType::SEED_SPLITPEA)
    {
        int aOffsetX, aOffsetY;
        GetPeaHeadOffset(aOffsetX, aOffsetY);
        aOriginY = mY + aOffsetY - 33;

        if (thePlantWeapon == PlantWeapon::WEAPON_SECONDARY)
        {
            aOriginX = mX + aOffsetX - 64;
        }
        else
        {
            aOriginX = mX + aOffsetX + 24;
        }
    }
    else if (mSeedType == SeedType::SEED_THREEPEATER)
    {
        aOriginX = mX + 45;
        aOriginY = mY + 10;
    }
    else if (mSeedType == SeedType::SEED_SCAREDYSHROOM)
    {
        aOriginX = mX + 29;
        aOriginY = mY + 21;
    }
    else if (mSeedType == SeedType::SEED_CACTUS)
    {
        if (thePlantWeapon == PlantWeapon::WEAPON_PRIMARY)
        {
            aOriginX = mX + 93;
            aOriginY = mY - 50;
        }
        else
        {
            aOriginX = mX + 70;
            aOriginY = mY + 23;
        }
    }
    else if (mSeedType == SeedType::SEED_COBCANNON)
    {
        aOriginX = mX - 44;
        aOriginY = mY - 184;
    }
    else
    {
        aOriginX = mX + 10;
        aOriginY = mY + 5;
    }
    if (mBoard->GetFlowerPotAt(mPlantCol, mRow))
    {
        aOriginY -= 5;
    }
    
    if (mSeedType == SeedType::SEED_SNOWPEA)
    {
        int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_LAWN_MOWER, mRow, 1);
        mApp->AddTodParticle(aOriginX + 8, aOriginY + 13, aRenderPosition, ParticleEffect::PARTICLE_SNOWPEA_PUFF);
    }
    else if (mSeedType == SeedType::SEED_PUFFSHROOM)
    {
        int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_LAWN_MOWER, mRow, 1);
        mApp->AddTodParticle(aOriginX + 18, aOriginY + 13, aRenderPosition, ParticleEffect::PARTICLE_PUFFSHROOM_MUZZLE);
    }
    else if (mSeedType == SeedType::SEED_SCAREDYSHROOM)
    {
        int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_LAWN_MOWER, mRow, 1);
        mApp->AddTodParticle(aOriginX + 27, aOriginY + 13, aRenderPosition, ParticleEffect::PARTICLE_PUFFSHROOM_MUZZLE);
        mScaredyShotCount++;
    }

    Projectile* aProjectile = mBoard->AddProjectile(aOriginX, aOriginY, mRenderOrder - 1, theRow, aProjectileType);
    aProjectile->mErrorDamageMultiplier = GetAttackDamage(1);
    aProjectile->mDamageRangeFlags = GetDamageRangeFlags(thePlantWeapon);
	aProjectile->mOwnerPlantID = static_cast<PlantID>(mBoard->mPlants.DataArrayGetID(this));
	if (mApp->IsCricketMode())
		aProjectile->mDamageMultiplierPercent = mBoard->mChallenge->CricketDamageMultiplierPercent(mRow);
	aProjectile->mDamageSourceRow = mRow;

    if (mApp->IsWhatRulesMode())
    {
        ApplyPlantRuleToProjectile(aProjectile);

        if (mRuleId == PLANT_RULE_DOUBLE_SHOT && Rand(100) < 10)
        {
            Projectile* aSecond = mBoard->AddProjectile(aOriginX + 6, aOriginY + 4, mRenderOrder - 1, theRow, aProjectileType);
            if (aSecond)
            {
                aSecond->mErrorDamageMultiplier = aProjectile->mErrorDamageMultiplier;
                aSecond->mDamageRangeFlags = aProjectile->mDamageRangeFlags;
                aSecond->mOwnerPlantID = aProjectile->mOwnerPlantID;
                aSecond->mDamageMultiplierPercent = aProjectile->mDamageMultiplierPercent;
                aSecond->mDamageSourceRow = mRow;
                ApplyPlantRuleToProjectile(aSecond);
            }
        }

        if (mRuleId == PLANT_RULE_SHAKY_AIM && Rand(100) < 10 &&
            aProjectile->mMotionType != ProjectileMotion::MOTION_LOBBED)
        {
            aProjectile->mRow = ClampInt(theRow + (Rand(2) == 0 ? -1 : 1), 0, MAX_GRID_SIZE_Y - 1);
        }

        if (mRuleId == PLANT_RULE_JAM)
        {
            if (++mRuleAux % 10 == 0)
                mRuleJam = 100;
        }
    }

    // 西瓜投手：敌人血量越高，大西瓜出现概率越高，最高 80%（斗蛐蛐按本行计算，普通模式按全场计算）
    if (mSeedType == SeedType::SEED_MELONPULT && aProjectileType == ProjectileType::PROJECTILE_MELON)
    {
        int64_t aTotalHP = 0;
        Zombie* aZombie = nullptr;
        while (mBoard->IterateZombies(aZombie))
        {
            if (aZombie->IsDeadOrDying() || !aZombie->EffectedByDamage(GetDamageRangeFlags(thePlantWeapon)))
                continue;
            if (mApp->IsCricketMode() && aZombie->mRow != theRow)
                continue;
            aTotalHP += aZombie->mBodyHealth;
        }
        int aBigMelonChance = 10 + 90 * std::min(static_cast<int64_t>(20000), aTotalHP) / 20000;  // 最低 10%，本行总血量 20000 时 100%
        if (Rand(100) < aBigMelonChance)
        {
            aProjectile->mIsBigMelon = true;
        }
    }

    // 仙人掌的刺可穿透僵尸
    if (mSeedType == SeedType::SEED_CACTUS && aProjectileType == ProjectileType::PROJECTILE_SPIKE)
    {
        aProjectile->mIsPiercingSpike = true;
    }

    // 冰西瓜大招弹：命中直接冰冻 5s，溅射目标 +5 层寒意
    if (mSeedType == SeedType::SEED_WINTERMELON && mWinterMelonUlt)
    {
        aProjectile->mIsUltWinterMelon = true;
    }

    if (mSeedType == SeedType::SEED_CABBAGEPULT || mSeedType == SeedType::SEED_KERNELPULT ||
        mSeedType == SeedType::SEED_MELONPULT || mSeedType == SeedType::SEED_WINTERMELON)
    {
        float aRangeX, aRangeY;
        if (theTargetZombie)
        {
            Rect aZombieRect = theTargetZombie->GetZombieRect();
            aRangeX = theTargetZombie->ZombieTargetLeadX(50.0f) - aOriginX - 30.0f;
            aRangeY = aZombieRect.mY - aOriginY;

            if (theTargetZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_RIDING)
            {
                aRangeX -= 60.0f;
            }
            if (theTargetZombie->mZombieType == ZombieType::ZOMBIE_POGO && theTargetZombie->mHasObject)
            {
                aRangeX -= 60.0f;
            }
            if (theTargetZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL)
            {
                aRangeX -= 40.0f;
            }
            if (theTargetZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
            {
                aRangeY = mBoard->GridToPixelY(8, mRow) - aOriginY;
            }
        }
        else
        {
            aRangeX = 700.0f - aOriginX;
            aRangeY = 0.0f;
        }
        if (aRangeX < 40.0f)
        {
            aRangeX = 40.0f;
        }

        aProjectile->mMotionType = ProjectileMotion::MOTION_LOBBED;
        aProjectile->mVelX = aRangeX / 120.0f;
        aProjectile->mVelY = 0.0f;
        aProjectile->mVelZ = aRangeY / 120.0f - 7.0f;
        aProjectile->mAccZ = 0.115f;
    }
    else if (mSeedType == SeedType::SEED_THREEPEATER)
    {
        if (theRow < mRow)
        {
            aProjectile->mMotionType = ProjectileMotion::MOTION_THREEPEATER;
            aProjectile->mVelY = -3.0f;
            aProjectile->mShadowY += 80.0f;
        }
        else if (theRow > mRow)
        {
            aProjectile->mMotionType = ProjectileMotion::MOTION_THREEPEATER;
            aProjectile->mVelY = 3.0f;
            aProjectile->mShadowY -= 80.0f;
        }
    }
    else if (mSeedType == SeedType::SEED_PUFFSHROOM || mSeedType == SeedType::SEED_SEASHROOM)
    {
        aProjectile->mMotionType = ProjectileMotion::MOTION_PUFF;
    }
    else if (mSeedType == SeedType::SEED_SPLITPEA && thePlantWeapon == PlantWeapon::WEAPON_SECONDARY)
    {
        aProjectile->mMotionType = ProjectileMotion::MOTION_BACKWARDS;
        aProjectile->mIsBouncingPea = true;  // 后向豌豆可反弹
        aProjectile->mVelX = -3.33f;  // 初始向左
    }
    else if (mSeedType == SeedType::SEED_LEFTPEATER)
    {
        aProjectile->mMotionType = ProjectileMotion::MOTION_BACKWARDS;
    }
    else if (mSeedType == SeedType::SEED_IRONPEASHOOTER)
    {
        aProjectile->mVelX = 20.0f;
    }
    else if (mSeedType == SeedType::SEED_CATTAIL)
    {
        aProjectile->mVelX = 2.0f;
        aProjectile->mMotionType = ProjectileMotion::MOTION_HOMING;
        aProjectile->mTargetZombieID = mBoard->ZombieGetID(theTargetZombie);
        aProjectile->mIsCattailShot = true;
    }
    else if (mSeedType == SeedType::SEED_COBCANNON)
    {
        aProjectile->mVelX = 0.001f;
        aProjectile->mDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
        aProjectile->mMotionType = ProjectileMotion::MOTION_LOBBED;
        aProjectile->mVelY = 0.0f;
        aProjectile->mAccZ = 0.0f;
        aProjectile->mVelZ = -8.0f;
        aProjectile->mCobTargetX = mTargetX - 40;
        aProjectile->mCobTargetRow = mBoard->PixelToGridYKeepOnBoard(mTargetX, mTargetY);
    }

	// 三叶草为本行所有子弹加速；孢子与抛物线投射物不受影响。
	if (mApp->IsCricketMode() && HasBloverInRow() &&
		aProjectile->mProjectileType != ProjectileType::PROJECTILE_PUFF &&
		aProjectile->mMotionType != ProjectileMotion::MOTION_LOBBED)
	{
		aProjectile->mBloverSpeedBoost = true;
	}

    if (aProjectileType == ProjectileType::PROJECTILE_PEA)
    {
        if (Rand(100) < 25)
            aProjectile->ConvertToFireball(mPlantCol);

        if (mChainCount == 0 && Rand(100) < 40)
        {
            mChainCount = 1;
            mChainDelay = 8;
            mChainOriginX = aOriginX;
            mChainOriginY = aOriginY;
            mChainDamageFlags = aProjectile->mDamageRangeFlags;
            mChainRow = theRow;
            mChainProjectileType = aProjectileType;
        }
    }

    if (mSeedType == SeedType::SEED_CABBAGEPULT)
    {
        if (mChainCount == 0 && Rand(100) < 50)
        {
            mChainCount = 1;
            mChainDelay = 8;
            mChainOriginX = aOriginX;
            mChainOriginY = aOriginY;
            mChainDamageFlags = aProjectile->mDamageRangeFlags;
            mChainRow = theRow;
            mChainProjectileType = aProjectileType;
        }
    }

    if (mSeedType == SeedType::SEED_HYPNOSHROOM)
    {
        aProjectile->mOwnerPlantID = static_cast<PlantID>(mBoard->mPlants.DataArrayGetID(this));
        aProjectile->mCharmProbability = mCharmProbability;
    }

    if (aIsFireIce)
    {
        AttachmentDie(aProjectile->mAttachmentID);
        mApp->PlayFoley(FoleyType::FOLEY_FIREPEA);

        float aOffsetX = -25.0f;
        float aOffsetY = -25.0f;
        Reanimation* aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_FIRE_PEA);
        if (aProjectile->mMotionType == ProjectileMotion::MOTION_BACKWARDS)
        {
            aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
            aOffsetX += 80.0f;
        }
        aFirePeaReanim->SetPosition(aProjectile->mPosX + aOffsetX, aProjectile->mPosY + aOffsetY);
        aFirePeaReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
        aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);

        aFirePeaReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT;

        AttachReanim(aProjectile->mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);

        aProjectile->mProjectileType = ProjectileType::PROJECTILE_SNOWPEA_FIRE;
        aProjectile->mHitTorchwoodGridX = mPlantCol;
    }

}

Zombie* Plant::FindTargetZombie(int theRow, PlantWeapon thePlantWeapon)
{
    int aDamageRangeFlags = GetDamageRangeFlags(thePlantWeapon);
    Rect aAttackRect = GetPlantAttackRect(thePlantWeapon);
    int aHighestWeight = 0;
    Zombie* aBestZombie = nullptr;

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        int aRowDeviation = aZombie->mRow - theRow;
        if (aZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
        {
            aRowDeviation = 0;
        }

        if (!aZombie->mHasHead || aZombie->IsTangleKelpTarget())
        {
            if (mSeedType == SeedType::SEED_POTATOMINE || mSeedType == SeedType::SEED_CHOMPER || mSeedType == SeedType::SEED_TANGLEKELP)
            {
                continue;
            }
        }

        bool needPortalCheck = false;
        if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_PORTAL_COMBAT)
        {
            if (mSeedType == SeedType::SEED_PEASHOOTER || mSeedType == SeedType::SEED_CACTUS || mSeedType == SeedType::SEED_REPEATER)
            {
                needPortalCheck = true;
            }
        }

        if (mSeedType != SeedType::SEED_CATTAIL)
        {
            if (mSeedType == SeedType::SEED_GLOOMSHROOM)
            {
                if (aRowDeviation < -1 || aRowDeviation > 1)
                {
                    continue;
                }
            }
            else if (needPortalCheck)
            {
                if (!mBoard->mChallenge->CanTargetZombieWithPortals(this, aZombie))
                {
                    continue;
                }
            }
            else if (aRowDeviation)
            {
                continue;
            }
        }

        if (aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            int aExtraRange = 0;

            if (mSeedType == SeedType::SEED_CHOMPER)
            {
                if (aZombie->mZombiePhase == ZombiePhase::PHASE_DIGGER_WALKING)
                {
                    aAttackRect.mX += 20;
                    aAttackRect.mWidth -= 20;
                }

                if (aZombie->mIsEating || mState == PlantState::STATE_CHOMPER_BITING)
                {
                    aExtraRange = 60;
                }
            }

            if (mSeedType == SeedType::SEED_POTATOMINE)
            {
                if ((aZombie->mZombieType == ZombieType::ZOMBIE_POGO && aZombie->mHasObject) ||
                    aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_IN_VAULT || aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_PRE_VAULT)
                {
                    continue;
                }

                if (aZombie->mZombieType == ZombieType::ZOMBIE_POLEVAULTER)
                {
                    aAttackRect.mX += 40;
                    aAttackRect.mWidth -= 40;  // 原版经典土豆地雷 Bug 及“四撑杆引雷”的原理
                }

                if (aZombie->mZombieType == ZombieType::ZOMBIE_BUNGEE && aZombie->mTargetCol != mPlantCol)
                {
                    continue;
                }

                if (aZombie->mIsEating)
                {
                    aExtraRange = 30;
                }
            }

            if ((mSeedType == SeedType::SEED_EXPLODE_O_NUT && aZombie->mZombiePhase == ZombiePhase::PHASE_POLEVAULTER_IN_VAULT) ||
                (mSeedType == SeedType::SEED_TANGLEKELP && !aZombie->mInPool))
            {
                continue;
            }

            Rect aZombieRect = aZombie->GetZombieRect();
            if (!needPortalCheck && GetRectOverlap(aAttackRect, aZombieRect) < -aExtraRange)
            {
                continue;
            }

            ////////////////////

            int aWeight = -aZombieRect.mX;
            if (mSeedType == SeedType::SEED_CATTAIL)
            {
                aWeight = -Distance2D(mX + 40.0f, mY + 40.0f, aZombieRect.mX + aZombieRect.mWidth / 2, aZombieRect.mY + aZombieRect.mHeight / 2);
				// Cattails normally support other lanes. A zombie that has reached
				// the cattail's immediate area is an emergency and takes priority.
				if (aZombie->mRow != mRow)
					aWeight += 50000;
				else if (aZombieRect.mX <= mX + 120)
					aWeight += 60000;
                if (aZombie->IsFlying())
                {
                    aWeight += 10000;  // 优先攻击飞行单位
                }
                if (aZombie->mCattailMarkStacks >= 3)
                {
                    aWeight += 20000;  // 全部猫尾草集火已标记的猎物
                }
            }

            if (aBestZombie == nullptr || aWeight > aHighestWeight)
            {
                aHighestWeight = aWeight;
                aBestZombie = aZombie;
            }
        }
    }

    return aBestZombie;
}

int Plant::DistanceToClosestZombie()
{
    int aDamageRangeFlags = GetDamageRangeFlags(PlantWeapon::WEAPON_PRIMARY);
    Rect aAttackRect = GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);
    int aClosestDistance = 1000;

    Zombie* aZombie = nullptr;
    while (mBoard->IterateZombies(aZombie))
    {
        if (aZombie->mRow == mRow && aZombie->EffectedByDamage(aDamageRangeFlags))
        {
            Rect aZombieRect = aZombie->GetZombieRect();
            int aDistance = -GetRectOverlap(aAttackRect, aZombieRect);
            if (aDistance < aClosestDistance)
            {
                aClosestDistance = std::max(aDistance, 0);
            }
        }
    }

    return aClosestDistance;
}

void Plant::Die()
{
	if (mApp->IsCricketMode() && mBoard->mChallenge->CricketRowShield(mRow) > 0)
	{
		if (mCricketShieldSnapshotValid)
			mPlantHealth = mCricketShieldHealthSnapshot;
		return;
	}

    if (!mDead && mBoard != nullptr && mApp->IsWhatRulesMode())
    {
        if (mRuleId == PLANT_RULE_LEGACY)
            mBoard->AddCoin(mX + 40, mY, CoinType::COIN_SUN, CoinMotion::COIN_MOTION_FROM_PLANT);
        if (mRuleId == PLANT_RULE_EMBER)
            mBoard->KillAllZombiesInRadius(mRow, mX + 40, mY + 40, 60, 1, false, 0, 50);
        if (mRuleId == PLANT_RULE_SHATTER)
        {
            Plant* aPlant = nullptr;
            while (mBoard->IteratePlants(aPlant))
            {
                if (aPlant == this || aPlant->mDead || aPlant->NotOnGround())
                    continue;
                if (std::abs(aPlant->mPlantCol - mPlantCol) <= 1 && std::abs(aPlant->mRow - mRow) <= 1)
                    aPlant->mPlantHealth -= 50;
            }
        }
    }

    if (IsOnBoard() && mSeedType == SeedType::SEED_TANGLEKELP)
    {
        Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
        if (aZombie)
        {
            aZombie->DieWithLoot();
        }
    }

    mDead = true;
    RemoveEffects();

    if (!Plant::IsFlying(mSeedType) && IsOnBoard())
    {
        GridItem* aLadder = mBoard->GetLadderAt(mPlantCol, mRow);
        if (aLadder)
        {
            aLadder->GridItemDie();
        }
    }

    if (IsOnBoard())
    {
        Plant* aTopPlant = mBoard->GetTopPlantAt(mPlantCol, mRow, PlantPriority::TOPPLANT_BUNGEE_ORDER);
        Plant* aFlowerPot = mBoard->GetFlowerPotAt(mPlantCol, mRow);
        if (aFlowerPot && aTopPlant == aFlowerPot)
        {
            Reanimation* aPotReanim = mApp->ReanimationGet(aFlowerPot->mBodyReanimID);
            aPotReanim->mAnimRate = RandRangeFloat(10.0f, 15.0f);
        }
    }
}

const PlantDefinition& GetPlantDefinition(SeedType theSeedType)
{
    TOD_ASSERT(gPlantDefs[theSeedType].mSeedType == theSeedType);
    TOD_ASSERT(theSeedType >= 0 && theSeedType < static_cast<int>(SeedType::NUM_SEED_TYPES));
    
    return gPlantDefs[theSeedType];
}

void Plant::ApplyPlantRule()
{
    if (mRuleId < 0)
        return;

    switch (mRuleId)
    {
    case PLANT_RULE_IRON_BODY:     mPlantHealth = mPlantHealth * 140 / 100; break;
    case PLANT_RULE_FRAIL:         mPlantHealth = mPlantHealth * 70 / 100; break;
    case PLANT_RULE_THICK_ARMOR:   mPlantHealth = mPlantHealth * 150 / 100; break;
    case PLANT_RULE_THIN_SHELL:    mPlantHealth = mPlantHealth * 70 / 100; break;
    case PLANT_RULE_REGEN:         mRuleTimer = 500; break;
    case PLANT_RULE_SHORT_LIFE:    mRuleTimer = 1000; break;
    case PLANT_RULE_STIFF:         mRuleStun = 300; break;
    case PLANT_RULE_FAST_YIELD:    mLaunchRate = std::max(1, mLaunchRate * 80 / 100); break;
    case PLANT_RULE_SLOW_YIELD:    mLaunchRate = std::max(1, mLaunchRate * 125 / 100); break;
    case PLANT_RULE_BAD_LUCK:      mBoard->mSunMoney = std::max(0, mBoard->mSunMoney - 25); break;
    case PLANT_RULE_REFUND:        mBoard->mSunMoney += 50; break;
    case PLANT_RULE_EXPENSIVE:     mBoard->mSunMoney = std::max(0, mBoard->mSunMoney - 50); break;
    case PLANT_RULE_FAST_FUSE:
    case PLANT_RULE_SLOW_FUSE:
        if (GetPlantRulePool(mSeedType) == PLANT_RULE_POOL_INSTANT)
        {
            const int aPercent = mRuleId == PLANT_RULE_FAST_FUSE ? 70 : 150;
            if (mDoSpecialCountdown > 0)
                mDoSpecialCountdown = std::max(1, mDoSpecialCountdown * aPercent / 100);
            if (mStateCountdown > 0)
                mStateCountdown = std::max(1, mStateCountdown * aPercent / 100);
        }
        break;
    default:
        break;
    }
}

void Plant::UpdatePlantRule()
{
    if (!mApp->IsWhatRulesMode() || mRuleId < 0 || mDead || mSquished || !IsInPlay())
        return;

    if (!mRuleRevealed)
    {
        if (--mRuleRevealTimer <= 0)
            mRuleRevealed = true;
    }

    if (mRuleJam > 0)
        --mRuleJam;

    if (mRuleId == PLANT_RULE_REGEN || mRuleId == PLANT_RULE_SHORT_LIFE)
    {
        if (--mRuleTimer <= 0)
        {
            const int aAmount = std::max(1, mPlantMaxHealth * 2 / 100);
            if (mRuleId == PLANT_RULE_REGEN)
            {
                mRuleTimer = 500;
                mPlantHealth = std::min(mPlantMaxHealth, mPlantHealth + aAmount);
            }
            else
            {
                mRuleTimer = 1000;
                mPlantHealth -= aAmount;
            }
        }
    }
}

void Plant::ApplyPlantRuleToProjectile(Projectile* theProjectile)
{
    if (theProjectile == nullptr || mRuleId < 0)
        return;

    switch (mRuleId)
    {
    case PLANT_RULE_HEAVY_SHOT:
        theProjectile->mDamageMultiplierPercent = theProjectile->mDamageMultiplierPercent * 140 / 100;
        break;
    case PLANT_RULE_SOFT_SHOT:
        theProjectile->mDamageMultiplierPercent = theProjectile->mDamageMultiplierPercent * 75 / 100;
        break;
    case PLANT_RULE_FROST_ENCHANT:
        theProjectile->mRuleFreezeChance = 10;
        break;
    case PLANT_RULE_PIERCE:
        if (theProjectile->mMotionType != ProjectileMotion::MOTION_LOBBED)
            theProjectile->mIsPiercingSpike = true;
        break;
    case PLANT_RULE_SPLASH:
        theProjectile->mRuleSplash = true;
        break;
    default:
        break;
    }
}

void Plant::DrawPlantRuleBadge(Graphics* g)
{
    if (!mApp->IsWhatRulesMode() || mRuleId < 0 || mDead || mSquished || !mIsOnBoard)
        return;

    const PlantRuleDef& aDef = GetPlantRuleDef(mRuleId);
    const std::string aText = mRuleRevealed ? aDef.mName : "?";

    Sexy::_Font* aFont = Sexy::FONT_HOUSEOFTERROR16;
    if (Sexy::SysFont* aCJK = Sexy::GetCJKFont(12); aCJK && aCJK->IsValid())
        aFont = aCJK;

    const int aCenterX = mX + 40;
    int aTopY = mY - 58;
    if (mSeedType == SeedType::SEED_TALLNUT || mSeedType == SeedType::SEED_COBCANNON)
        aTopY = mY - 100;

    const int aWidth = aFont->StringWidth(aText);
    g->SetColor(Color(0, 0, 0, 150));
    g->FillRect(aCenterX - aWidth / 2 - 3, aTopY - 2, aWidth + 6, 16);

    Color aColor(255, 240, 120);
    if (mRuleRevealed)
        aColor = PlantRuleIsGood(mRuleId) ? Color(120, 255, 120) : Color(255, 120, 120);
    aFont->DrawString(g, aCenterX - aWidth / 2, aTopY, aText, aColor, Rect());
}


int Plant::GetCost(SeedType theSeedType, SeedType theImitaterType)
{
    if (gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED || gLawnApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_BEGHOULED_TWIST)
    {
        if (theSeedType == SeedType::SEED_REPEATER)
        {
            return 1000;
        }
        else if (theSeedType == SeedType::SEED_FUMESHROOM)
        {
            return 500;
        }
        else if (theSeedType == SeedType::SEED_TALLNUT)
        {
            return 250;
        }
        else if (theSeedType == SeedType::SEED_BEGHOULED_BUTTON_SHUFFLE)
        {
            return 100;
        }
        else if (theSeedType == SeedType::SEED_BEGHOULED_BUTTON_CRATER)
        {
            return 200;
        }
    }

    switch (theSeedType)
    {
    case SeedType::SEED_SLOT_MACHINE_SUN:           return 0;
    case SeedType::SEED_SLOT_MACHINE_DIAMOND:       return 0;
    case SeedType::SEED_ZOMBIQUARIUM_SNORKLE:       return 100;
    case SeedType::SEED_ZOMBIQUARIUM_TROPHY:        return 1000;
    case SeedType::SEED_ZOMBIE_NORMAL:              return 50;
    case SeedType::SEED_ZOMBIE_CART:                return 50;
    case SeedType::SEED_ZOMBIE_TRAFFIC_CONE:        return 75;
    case SeedType::SEED_ZOMBIE_POLEVAULTER:         return 75;
    case SeedType::SEED_ZOMBIE_PAIL:                return 125;
    case SeedType::SEED_ZOMBIE_LADDER:              return 150;
    case SeedType::SEED_ZOMBIE_DIGGER:              return 125;
    case SeedType::SEED_ZOMBIE_BUNGEE:              return 125;
    case SeedType::SEED_ZOMBIE_FOOTBALL:            return 175;
    case SeedType::SEED_ZOMBIE_BALLOON:             return 150;
    case SeedType::SEED_ZOMBIE_SCREEN_DOOR:         return 100;
    case SeedType::SEED_ZOMBONI:                    return 175;
    case SeedType::SEED_ZOMBIE_POGO:                return 200;
    case SeedType::SEED_ZOMBIE_DANCER:              return 350;
    case SeedType::SEED_ZOMBIE_GARGANTUAR:          return 300;
    case SeedType::SEED_ZOMBIE_IMP:                 return 50;
    default:
    {
        if (theSeedType == SeedType::SEED_IMITATER && theImitaterType != SeedType::SEED_NONE)
        {
            const PlantDefinition& aPlantDef = GetPlantDefinition(theImitaterType);
            return aPlantDef.mSeedCost;
        }
        else
        {
            const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
            return aPlantDef.mSeedCost;
        }
    }
    }
}

// GOTY @Patoke: 0x46B6C0
std::string Plant::GetNameString(SeedType theSeedType, SeedType theImitaterType)
{
    if (theSeedType == SeedType::SEED_GIFT_BOX)
        return "礼物盒";
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    if (theSeedType == SeedType::SEED_IRONPEASHOOTER)
        return "Iron Pea Shooter";
    if (theSeedType == SeedType::SEED_DOOMPEA)
        return "Doom Pea";
    if (theSeedType == SeedType::SEED_SHIELDWALLNUT)
        return "Shield Wall-nut";
    std::string aName = StrFormat("[%s]", aPlantDef.mPlantName);
    std::string aTranslatedName = TodStringTranslate(aName);

    if (theSeedType == SeedType::SEED_IMITATER && theImitaterType != SeedType::SEED_NONE)
    {
        const PlantDefinition& aImitaterDef = GetPlantDefinition(theImitaterType);
        std::string aImitaterName = StrFormat("[%s]", aImitaterDef.mPlantName);
        std::string aTranslatedImitaterName = TodStringTranslate(aImitaterName);
        return StrFormat("%s %s", aTranslatedName.c_str(), aTranslatedImitaterName.c_str());
    }

    return aTranslatedName;
}

std::string Plant::GetToolTip(SeedType theSeedType)
{
    if (theSeedType == SeedType::SEED_GIFT_BOX)
        return "1秒后打开，随机变为植物。";
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    if (theSeedType == SeedType::SEED_IRONPEASHOOTER)
        return "Shoots gray iron peas.";
    if (theSeedType == SeedType::SEED_DOOMPEA)
        return "Fires a black beam that narrows from 45 degrees and devastates zombies.";
    if (theSeedType == SeedType::SEED_SHIELDWALLNUT)
        return "A blue shield protects plants in its row.";
    std::string aToolTip = StrFormat("[%s_TOOLTIP]", aPlantDef.mPlantName);
    return TodStringTranslate(aToolTip);
}

int Plant::GetRefreshTime(SeedType theSeedType, SeedType theImitaterType)
{
    if (Challenge::IsZombieSeedType(theSeedType))
    {
        return 0;
    }

    if (theSeedType == SeedType::SEED_IMITATER && theImitaterType != SeedType::SEED_NONE)
    {
        const PlantDefinition& aPlantDef = GetPlantDefinition(theImitaterType);
        return aPlantDef.mRefreshTime;
    }
    else
    {
        const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
        return aPlantDef.mRefreshTime;
    }
}

bool Plant::IsNocturnal(SeedType theSeedtype)
{
    return
        theSeedtype == SeedType::SEED_PUFFSHROOM ||
        theSeedtype == SeedType::SEED_SEASHROOM ||
        theSeedtype == SeedType::SEED_SUNSHROOM ||
        theSeedtype == SeedType::SEED_FUMESHROOM ||
        theSeedtype == SeedType::SEED_HYPNOSHROOM ||
        theSeedtype == SeedType::SEED_DOOMSHROOM ||
        theSeedtype == SeedType::SEED_ICESHROOM ||
        theSeedtype == SeedType::SEED_MAGNETSHROOM ||
        theSeedtype == SeedType::SEED_SCAREDYSHROOM ||
        theSeedtype == SeedType::SEED_GLOOMSHROOM;
}

// GOTY @Patoke: inlined 0x40FB2B
bool Plant::IsFungus(SeedType theSeedtype) 
{
    return
        theSeedtype == SeedType::SEED_PUFFSHROOM ||
        theSeedtype == SeedType::SEED_SUNSHROOM ||
        theSeedtype == SeedType::SEED_FUMESHROOM ||
        theSeedtype == SeedType::SEED_HYPNOSHROOM ||
        theSeedtype == SeedType::SEED_SCAREDYSHROOM ||
        theSeedtype == SeedType::SEED_ICESHROOM ||
        theSeedtype == SeedType::SEED_DOOMSHROOM ||
        theSeedtype == SeedType::SEED_SEASHROOM ||
        theSeedtype == SeedType::SEED_MAGNETSHROOM ||
        theSeedtype == SeedType::SEED_GLOOMSHROOM;
}

bool Plant::IsAquatic(SeedType theSeedType)
{
    return
        theSeedType == SeedType::SEED_LILYPAD ||
        theSeedType == SeedType::SEED_TANGLEKELP ||
        theSeedType == SeedType::SEED_SEASHROOM ||
        theSeedType == SeedType::SEED_CATTAIL;
}

// GOTY @Patoke: 0x469543
bool Plant::IsFlying(SeedType theSeedtype)
{
    return theSeedtype == SeedType::SEED_INSTANT_COFFEE;
}

bool Plant::IsUpgrade(SeedType theSeedtype)
{
    return 
        theSeedtype == SeedType::SEED_WINTERMELON || 
        theSeedtype == SeedType::SEED_TWINSUNFLOWER || 
        theSeedtype == SeedType::SEED_SPIKEROCK || 
        theSeedtype == SeedType::SEED_COBCANNON || 
        theSeedtype == SeedType::SEED_GOLD_MAGNET || 
        theSeedtype == SeedType::SEED_GLOOMSHROOM || 
        theSeedtype == SeedType::SEED_CATTAIL;
}

Rect Plant::GetPlantRect()
{
    Rect aRect;
    if (mSeedType == SeedType::SEED_TALLNUT)
    {
        aRect = Rect(mX + 10, mY, mWidth, mHeight);
    }
    else if (mSeedType == SeedType::SEED_PUMPKINSHELL)
    {
        aRect = Rect(mX, mY, mWidth - 20, mHeight);
    }
    else if (mSeedType == SeedType::SEED_COBCANNON)
    {
        aRect = Rect(mX, mY, 140, 80);
    }
    else
    {
        aRect = Rect(mX + 10, mY, mWidth - 20, mHeight);
    }

    return aRect;
}

Rect Plant::GetPlantAttackRect(PlantWeapon thePlantWeapon)
{
    Rect aRect;
    if (mApp->IsWallnutBowlingLevel())
    {
        aRect = Rect(mX, mY, mWidth - 20, mHeight);
    }
    else if (thePlantWeapon == PlantWeapon::WEAPON_SECONDARY && mSeedType == SeedType::SEED_SPLITPEA)
    {
        aRect = Rect(0, mY, mX + 16, mHeight);
    }
    else switch (mSeedType)
    {
    case SeedType::SEED_LEFTPEATER:     aRect = Rect(0,             mY,             mX,                 mHeight);               break;
    case SeedType::SEED_SQUASH:         aRect = Rect(mX + 20,       mY,             mWidth - 35,        mHeight);               break;
    case SeedType::SEED_CHOMPER:        aRect = Rect(mX - 80,       mY,             200 + mChomperEatenCount * 20, mHeight);               break;
    case SeedType::SEED_SPIKEWEED:
    case SeedType::SEED_SPIKEROCK:      aRect = Rect(mX + 20,       mY,             mWidth - 50,        mHeight);               break;
    case SeedType::SEED_POTATOMINE:     aRect = Rect(mX,            mY,             mWidth - 25,        mHeight);               break;
    case SeedType::SEED_TORCHWOOD:      aRect = Rect(mX + 50,       mY,             30,                 mHeight);               break;
    case SeedType::SEED_PUFFSHROOM:
    case SeedType::SEED_SEASHROOM:
        aRect = Rect(mX + 60, mY, mApp->IsCricketMode() ? BOARD_WIDTH : 230, mHeight);
        break;
    case SeedType::SEED_FUMESHROOM:     aRect = Rect(mX + 60,       mY,             BOARD_WIDTH,        mHeight);               break;
    case SeedType::SEED_GLOOMSHROOM:    aRect = Rect(mX - 80,       mY - 80,        240,                240);                   break;
    case SeedType::SEED_TANGLEKELP:     aRect = Rect(mX,            mY,             mWidth,             mHeight);               break;
    case SeedType::SEED_CATTAIL:        aRect = Rect(-BOARD_WIDTH,  -BOARD_HEIGHT,  BOARD_WIDTH * 2,    BOARD_HEIGHT * 2);      break;
    default:                            aRect = Rect(mX + 60,       mY,             BOARD_WIDTH,        mHeight);               break;
    }

    return aRect;
}

void Plant::PreloadPlantResources(SeedType theSeedType)
{
    const PlantDefinition& aPlantDef = GetPlantDefinition(theSeedType);
    if (aPlantDef.mReanimationType != ReanimationType::REANIM_NONE)
    {
        ReanimatorEnsureDefinitionLoaded(aPlantDef.mReanimationType, true);
    }

    if (theSeedType == SeedType::SEED_CHERRYBOMB)
    {
        ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_ZOMBIE_CHARRED, true);
    }
    else if (theSeedType == SeedType::SEED_JALAPENO)
    {
        ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_JALAPENO_FIRE, true);
    }
    else if (theSeedType == SeedType::SEED_TORCHWOOD)
    {
        ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_FIRE_PEA, true);
        ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_JALAPENO_FIRE, true);
    }
    else if (Plant::IsNocturnal(theSeedType))
    {
        ReanimatorEnsureDefinitionLoaded(ReanimationType::REANIM_SLEEPING, true);
    }
}

void Plant::UpdateChainFiring()
{
	if (mApp->IsPvPMode() && !mApp->IsPvPHost())
		return;

    if (mChainCount <= 0)
        return;

    if (mChainDelay > 0)
    {
        mChainDelay--;
        return;
    }

    bool isShooter = mSeedType == SeedType::SEED_PEASHOOTER || mSeedType == SeedType::SEED_REPEATER ||
        mSeedType == SeedType::SEED_THREEPEATER || mSeedType == SeedType::SEED_SPLITPEA ||
        mSeedType == SeedType::SEED_GATLINGPEA || mSeedType == SeedType::SEED_LEFTPEATER || mSeedType == SeedType::SEED_IRONPEASHOOTER ||
        mSeedType == SeedType::SEED_CABBAGEPULT;

    if (!isShooter)
        return;

    int aChainRow = mChainRow;
    if (mSeedType == SeedType::SEED_THREEPEATER)
    {
        int aRowOffsets[] = { 0, -1, 1 };
        aChainRow = mRow + aRowOffsets[mChainThreepeaterCycle % 3];
        aChainRow = ClampInt(aChainRow, 0, MAX_GRID_SIZE_Y - 1);
        mChainThreepeaterCycle++;
    }

    Projectile* aChain = mBoard->AddProjectile(
        static_cast<int>(mChainOriginX), static_cast<int>(mChainOriginY),
        mRenderOrder - 1, aChainRow, mChainProjectileType);
    if (!aChain)
        return;

    aChain->mErrorDamageMultiplier = GetAttackDamage(1);
    aChain->mDamageRangeFlags = mChainDamageFlags;
    if (mApp->IsCricketMode())
        aChain->mDamageMultiplierPercent = mBoard->mChallenge->CricketDamageMultiplierPercent(mRow);

    if (mSeedType == SeedType::SEED_CABBAGEPULT)
    {
        Zombie* aTarget = FindTargetZombie(aChainRow, PlantWeapon::WEAPON_PRIMARY);
        if (aTarget)
        {
            float aRangeX = aTarget->ZombieTargetLeadX(50.0f) - mChainOriginX - 30.0f;
            float aRangeY = aTarget->GetZombieRect().mY - mChainOriginY;
            if (aRangeX < 40.0f) aRangeX = 40.0f;
            aChain->mMotionType = ProjectileMotion::MOTION_LOBBED;
            aChain->mVelX = aRangeX / 120.0f;
            aChain->mVelY = 0.0f;
            aChain->mVelZ = aRangeY / 120.0f - 7.0f;
            aChain->mAccZ = 0.115f;
            int aGridX = mBoard->PixelToGridXKeepOnBoard(static_cast<int>(mChainOriginX), static_cast<int>(mChainOriginY));
            aChain->mShadowY = mBoard->GridToPixelY(aGridX, aChainRow) + 67.0f;
        }
        else
        {
            aChain->mMotionType = ProjectileMotion::MOTION_LOBBED;
            aChain->mVelX = 5.0f;
            aChain->mVelY = 0.0f;
            aChain->mVelZ = -7.0f;
            aChain->mAccZ = 0.115f;
        }
        aChain->mRotation = -7 * PI / 25;
        aChain->mRotationSpeed = RandRangeFloat(-0.08f, -0.02f);
    }
    else
    {
        aChain->mMotionType = ProjectileMotion::MOTION_STRAIGHT;
        aChain->mVelX = 5.0f;
        aChain->mVelY = 0.0f;
        aChain->mRotation = 0.0f;
        aChain->mRotationSpeed = 0.0f;
		if (mApp->IsCricketMode() && HasBloverInRow())
			aChain->mBloverSpeedBoost = true;
    }

    if (mChainProjectileType == ProjectileType::PROJECTILE_PEA && Rand(100) < 25)
        aChain->ConvertToFireball(mPlantCol);

    mChainCount--;
    if (Rand(100) < 40)
    {
        mChainCount++;
        mChainDelay = 8;
    }
}

void Plant::PlayIdleAnim(float theRate)
{
    Reanimation* aBodyReanim = mApp->ReanimationTryToGet(mBodyReanimID);
    if (aBodyReanim)
    {
        PlayBodyReanim("anim_idle", ReanimLoopType::REANIM_LOOP, 20, theRate);
        if (mApp->IsIZombieLevel())
        {
            aBodyReanim->mAnimRate = 0.0f;
        }
    }
}
