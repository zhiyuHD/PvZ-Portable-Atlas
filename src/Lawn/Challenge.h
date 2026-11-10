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

#ifndef __CHALLENGE_H__
#define __CHALLENGE_H__

#include <cstdint>
#include "../ConstEnums.h"
#include "../GameConstants.h"
#include "../Sexy.TodLib/FilterEffect.h"
#include "graphics/Graphics.h"

#define BEGHOULED_MAX_GRIDSIZEX 8
#define BEGHOULED_MAX_GRIDSIZEY 5
#define ART_CHALLEGE_SIZE_X MAX_GRID_SIZE_X
#define MAX_PICK_GRID_SIZE 50

constexpr const int BEGHOULED_WINNING_SCORE = 75;
constexpr const int SLOT_MACHINE_WINNING_SCORE = 2000;
constexpr const int ZOMBIQUARIUM_WINNING_SCORE = 1000;
constexpr const int I_ZOMBIE_WINNING_SCORE = 5;
constexpr const int MAX_PORTALS = 4;
constexpr const int MAX_SQUIRRELS = 7;
constexpr const int MAX_SCARY_POTS = 54;
constexpr const int STORM_FLASH_TIME = 150;

// Endless Zombie Horde (custom survival mode): Endless: Day with triple the spawn volume.
constexpr const int ENDLESS_HORDE_SPAWN_MULTIPLIER = 3; // 出怪量倍数

// Super Gargantuar (超大白): chance that a white-eye gargantuar becomes the 3x/5x/half-speed variant.
constexpr const int SUPER_GARGANTUAR_CHANCE = 15; // 出怪时成为超大白的基础概率（百分比）

// Cricket arena (斗蛐蛐) constants
constexpr const int CRICKET_ROWS = 5;                 // 植物行数
constexpr const int CRICKET_PLANTS_PER_ROW = 5;       // 每行植物数（占第 0..4 列）
constexpr const int CRICKET_MATCH_INTERVAL = 100;     // 初始出怪间隔（逻辑帧，100=1s）
constexpr const int CRICKET_RANDOM_COL = 4;           // 矿工/气球限制到达的最左列（第 5 列，0 起始）
constexpr const int CRICKET_MAX_ZOMBIES = 400;        // 场上最大僵尸数（重动画上限已提升到 4096）
constexpr const int CRICKET_CHAMPIONS = 10;

using namespace Sexy;

class LawnApp;
class Board;
class Plant;
class Zombie;
class GridItem;
class SeedPacket;
class HitResult;
struct TodWeightedGridArray;

enum BeghouledUpgrade : int32_t
{
    BEGHOULED_UPGRADE_REPEATER,
    BEGHOULED_UPGRADE_FUMESHROOM,
    BEGHOULED_UPGRADE_TALLNUT,
    NUM_BEGHOULED_UPGRADES
};

struct BeghouledBoardState
{
	SeedType		        mSeedType[9][6];
};

class Challenge
{
public:
	LawnApp*				mApp;
	Board*					mBoard;
	int32_t					mBeghouledMouseCapture;
    int32_t                 mBeghouledMouseDownX;
    int32_t                 mBeghouledMouseDownY;
    int32_t                mBeghouledEated[9][6];
    int32_t                mBeghouledPurcasedUpgrade[NUM_BEGHOULED_UPGRADES];
    int32_t                 mBeghouledMatchesThisMove;
    ChallengeState          mChallengeState;
    int32_t                 mChallengeStateCounter;
    int32_t                 mConveyorBeltCounter;
    int32_t                 mChallengeScore;
    int32_t                mShowBowlingLine;
    SeedType                mLastConveyorSeedType;
    int32_t                 mSurvivalStage;
    int32_t                 mSlotMachineRollCount;
    ReanimationID           mReanimChallenge;
    ReanimationID           mReanimClouds[6];
    int32_t                 mCloudsCounter[6];
    int32_t                 mChallengeGridX;
    int32_t                 mChallengeGridY;
    int32_t                 mScaryPotterPots;
    int32_t                 mRainCounter;
    int32_t                 mTreeOfWisdomTalkIndex;

public:
    Challenge();

    void                    StartLevel();
    void                    BeghouledPopulateBoard();
    void                    LoadBeghouledBoardState(BeghouledBoardState* theState);
    SeedType                BeghouledPickSeed(int theGridX, int theGridY, BeghouledBoardState* theBoardState, int theAllowMatches);
    int                    BeghouledBoardHasMatch(BeghouledBoardState* theBoardState);
    /*inline*/ SeedType     BeghouledGetPlantAt(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    int                     BeghouledVerticalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    int                     BeghouledHorizontalMatchLength(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    /*inline*/ void         BeghouledDragStart(int x, int y);
    void                    BeghouledDragUpdate(int x, int y);
    inline void             BeghouledDragCancel() { mBeghouledMouseCapture = false; }
    int                    MouseMove(int x, int y);
    int                    MouseDown(int x, int y, int theClickCount, HitResult* theHitResult);
    int                    MouseUp(int x, int y);
    void                    ClearCursor();
    void                    BeghouledRemoveHorizontalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    void                    BeghouledRemoveVerticalMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    void                    BeghouledRemoveMatches(BeghouledBoardState* theBoardState);
    void                    Update();
    void                    UpdateBeghouled();
    int                    UpdateBeghouledPlant(Plant* thePlant);
    void                    BeghouledFallIntoSquare(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    void                    BeghouledMakePlantsFall(BeghouledBoardState* theBoardState);
    void                    ZombieAtePlant(/*Zombie* theZombie,*/ Plant* thePlant);
    void                    DrawBackdrop(Graphics* g);
    void                    DrawArtChallenge(Graphics* g);
    void                    CheckForCompleteArtChallenge(int theGridX, int theGridY);
    /*inline*/ SeedType     GetArtChallengeSeed(int theGridX, int theGridY);
    void                    PlantAdded(Plant* thePlant);
    PlantingReason          CanPlantAt(int theGridX, int theGridY, SeedType theSeedType);
    void                    DrawBeghouled(Graphics* g);
    int                    BeghouledIsValidMove(int theFromX, int theFromY, int theToX, int theToY, BeghouledBoardState* theBoardState);
    int                    BeghouledCheckForPossibleMoves(BeghouledBoardState* theBoardState);
    void                    BeghouledCheckStuckState();
    void                    InitZombieWavesSurvival();
    /*inline*/ void         InitZombieWavesFromList(ZombieType* theZombieList, int theListLength);
    void                    InitZombieWaves();
    void                    InitErrorTrialWaves();
    void                    InitGiftBoxNightmareWaves();
    void                    UpdateGiftBoxes();
    /*inline*/ Rect         SlotMachineGetHandleRect();
    void                    UpdateSlotMachine();
    void                    DrawSlotMachine(Graphics* g);
    int                    UpdateToolTip(int theX, int theY);
    void                    WhackAZombieSpawning();
    int                    UpdateZombieSpawning();
    void                    BeghouledClearCrater(int theCount);
    void                    MouseDownWhackAZombie(int theX, int theY);
    void                    DrawStormNight(Graphics* g);
    void                    UpdateStormyNight();
    void                    InitLevel();
    void                    SpawnZombieWave();
    void                    GraveDangerSpawnRandomGrave();
    void                    GraveDangerSpawnGraveAt(int theGridX, int theGridY);
    void                    SpawnLevelAward(int theGridX, int theGridY);
    void                    BeghouledScore(int theGridX, int theGridY, int theNumPlants, int theIsHorizontal);
    void                    DrawStormFlash(Graphics* g, int theTime, int theMaxAmount);
    void                    UpdateRainingSeeds();
    /*inline*/ void         PlayBossEnter();
    void                    UpdateConveyorBelt();
    void                    PortalStart();
    void                    UpdatePortalCombat();
    GridItem*               GetOtherPortal(GridItem* thePortal);
    void                    UpdatePortal(GridItem* thePortal);
    float                   PortalCombatRowSpawnWeight(int theGridY);
    int                    CanTargetZombieWithPortals(Plant* thePlant, Zombie* theZombie);
    GridItem*               GetPortalToRight(int theGridX, int theGridY);
    GridItem*               GetPortalAt(int theGridX, int theGridY);
    void                    MoveAPortal();
    int                     GetPortalDistanceToMower(int theGridY);
    GridItem*               GetPortalToLeft(int theGridX, int theGridY);
    int                     mCricketRowDefeated[CRICKET_ROWS];
    int                     mCricketRound;
    int                     mCricketAdvancingRow;
    int                     mCricketSpawnCounter;
    int                     mCricketFireCounter;
    int                     mCricketElapsedFrames;
    int                     mCricketRowDamageMultiplierPercent[CRICKET_ROWS];
    int                     mCricketRowPlantKills[CRICKET_ROWS];
    int                     mCricketRowShield[CRICKET_ROWS];
    bool                    mCricketPendingNewMatch;
    bool                    mCricketCustomStarted;
    int                     mCricketTournamentStage;
    int                     mCricketTournamentHeats;
    int                     mCricketTournamentWinnersMask;
    int                     mCricketTournamentFinalWinnerRow;
    int                     mCricketTournamentMatchInHeat;
    int                     mCricketTournamentChampionCount;
    int                     mCricketTournamentEliteMatch;
    bool                    mCricketTournamentUseLineup;
    SeedType                mCricketQualifierLineups[CRICKET_ROWS][CRICKET_PLANTS_PER_ROW];
    SeedType                mCricketQualifierInnerLineups[CRICKET_ROWS][CRICKET_PLANTS_PER_ROW];
    SeedType                mCricketChampionLineup[CRICKET_CHAMPIONS][CRICKET_PLANTS_PER_ROW];
    SeedType                mCricketChampionInnerLineup[CRICKET_CHAMPIONS][CRICKET_PLANTS_PER_ROW];
    SeedType                mCricketEliteWinnerLineup[2][CRICKET_PLANTS_PER_ROW];
    SeedType                mCricketEliteWinnerInnerLineup[2][CRICKET_PLANTS_PER_ROW];
    SeedType                mCricketLineup[CRICKET_ROWS][CRICKET_PLANTS_PER_ROW];
    SeedType                mCricketInnerLineup[CRICKET_ROWS][CRICKET_PLANTS_PER_ROW];  // 南瓜头内的植物（回合重铺时保持）
    void                    CricketInit();
    void                    TestCrushInit();
    void                    CricketUpdate();
    void                    CricketPlaceArenaPlants();
    void                    CricketPlaceRowPlants(int theRow);
    void                    CricketPlaceLineup(int theRow);
    void                    CricketPlaceOnePlant(int theCol, int theRow, SeedType theSeed, SeedType theInnerSeed = SeedType::SEED_NONE);
    void                    CricketSpawnZombie();
    void                    CricketCustomStart();
    void                    CricketCustomChooseCards();
    void                    CricketFireDefeatedRows();
    SeedType                CricketPickRangedSeed();
    SeedType                CricketPickUsefulSeed();
    SeedType                CricketPickUsefulSeedWithTorchBoost();
    SeedType                CricketPickRightmostSeed();
    ZombieType              CricketPickTransformZombie();
    ZombieType              CricketPickBerserkTransformZombie();
    int                     CricketNextSpawnInterval();
    void                    CricketRowDefeated(int theRow);
    void                    CricketRegisterCrossRowCattailKill(int theSourceRow);
    void                    CricketRegisterRowPlantKill(int theRow);
    int                     CricketRowPlantKills(int theRow) const;
    void                    CricketAddRowShield(int theRow, int theAmount);
    bool                    CricketAbsorbRowShield(int theRow, int theDamage);
    int                     CricketRowShield(int theRow) const;
    int                     CricketDamageMultiplierPercent(int theRow) const;
    void                    CricketStartNewMatch();
    void                    CricketChampionshipAdvance();
    void                    BeghouledPacketClicked(SeedPacket* theSeedPacket);
    void                    BeghouledShuffle();
    /*inline*/ int         BeghouledCanClearCrater();
    void                    BeghouledUpdateCraters();
    Zombie*                 ZombiquariumSpawnSnorkle();
    void                    ZombiquariumPacketClicked(SeedPacket* theSeedPacket);
    void                    ZombiquariumMouseDown(int x, int y);
    void                    ZombiquariumDropBrain(int x, int y);
    void                    ZombiquariumUpdate();
    /*inline*/ void         ShovelAddWallnuts();
    void                    ScaryPotterPlacePot(ScaryPotType theScaryPotType, ZombieType theZombieType, SeedType theSeedType, int theCount, TodWeightedGridArray* theGridArray, int theGridArrayCount);
    void                    ScaryPotterStart();
    void                    ScaryPotterUpdate();
    void                    ScaryPotterOpenPot(GridItem* theScaryPot);
    void                    ScaryPotterJackExplode(int thePosX, int thePosY);
    int                    ScaryPotterIsCompleted();
    void                    ScaryPotterChangePotType(GridItemState thePotType, int theCount);
    void                    ScaryPotterPopulate();
    /*inline*/ void         ScaryPotterDontPlaceInCol(int theCol, TodWeightedGridArray* theGridArray, int theGridArrayCount);
    void                    ScaryPotterFillColumnWithPlant(int theCol, SeedType theSeedType, TodWeightedGridArray* theGridArray, int theGridArrayCount);
    void                    PuzzleNextStageClear();
    void                    ScaryPotterMalletPot(GridItem* theScaryPot);
    static ZombieType       IZombieSeedTypeToZombieType(SeedType theSeedType);
    static /*inline*/ int  IsZombieSeedType(SeedType theSeedType);
    void                    IZombieMouseDownWithZombie(int theX, int theY, int theClickCount);
    void                    IZombieStart();
    void                    IZombiePlacePlants(SeedType theSeedType, int theCount, int theGridY = -1);
    void                    IZombieUpdate();
    void                    IZombieDrawPlant(Graphics* g, Plant* thePlant);
    void                    IZombieSetPlantFilterEffect(Plant* thePlant, FilterEffect theFilterEffect);
    /*inline*/ int          ScaryPotterCountSunInPot(GridItem* theScaryPot);
    int                     ScaryPotterCountPots();
    void                    IZombieInitLevel();
    void                    DrawRain(Graphics* g);
    void                    DrawWeather(Graphics* g);
    void                    SquirrelUpdate();
    /*inline*/ int          SquirrelCountUncaught();
    void                    SquirrelStart();
    void                    SquirrelFound(GridItem* theSquirrel);
    void                    SquirrelPeek(GridItem* theSquirrel);
    void                    SquirrelChew(GridItem* theSquirrel);
    void                    SquirrelUpdateOne(GridItem* theSquirrel);
    void                    IZombieSetupPlant(Plant* thePlant);
    void                    UpdateRain();
    int                    IZombieEatBrain(Zombie* theZombie);
    GridItem*               IZombieGetBrainTarget(Zombie* theZombie);
    /*inline*/ void         IZombiePlacePlantInSquare(SeedType theSeedType, int theGridX, int theGridY = -1);
    void                    AdvanceCrazyDaveDialog();
    void                    BeghouledFlashPlant(int theFlashX, int theFlashY, int theFromX, int theFromY, int theToX, int theToY);
    void                    BeghouledFlashAMatch();
    int                    BeghouledFlashFromBoardState(BeghouledBoardState* theBoardState, int theFromX, int theFromY, int theToX, int theToY);
    void                    IZombiePlantDropRemainingSun(Plant* thePlant);
    void                    IZombieSquishBrain(GridItem* theBrain);
    void                    IZombieScoreBrain(GridItem* theBrain);
    void                    LastStandUpdate();
    void                    WhackAZombiePlaceGraves(int theGraveCount);
    int                    BeghouledTwistSquareFromMouse(int theX, int theY, int& theGridX, int& theGridY);
    int                    BeghouledTwistValidMove(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    void                    BeghouledTwistMouseDown(int x, int y);
    int                    BeghouledTwistMoveCausesMatch(int theGridX, int theGridY, BeghouledBoardState* theBoardState);
    int                    BeghouledTwistFlashMatch(BeghouledBoardState* theBoardState, int theGridX, int theGridY);
    /*inline*/ void         BeghouledCancelMatchFlashing();
    void                    BeghouledStartFalling(ChallengeState theState);
    void                    BeghouledFillHoles(BeghouledBoardState* theBoardState, int theAllowMatches);
    /*inline*/ void         BeghouledMakeStartBoard();
    void                    BeghouledCreatePlants(BeghouledBoardState* theOldBoardState, BeghouledBoardState* theNewBoardState);
    void                    PuzzlePhaseComplete(int theGridX, int theGridY);
    /*inline*/ int         PuzzleIsAwardStage();
    void                    IZombiePlaceZombie(ZombieType theZombieType, int theGridX, int theGridY, int theForcedVariantFlags = -1);
    void                    WhackAZombieUpdate();
    void                    LastStandCompletedStage();
    void                    TreeOfWisdomUpdate();
    void                    TreeOfWisdomFertilize();
    void                    TreeOfWisdomInit();
    /*inline*/ int         TreeOfWisdomMouseOn(int theX, int theY);
    /*inline*/ int          TreeOfWisdomGetSize();
    void                    TreeOfWisdomDraw(Graphics* g);
    /*inline*/ void         TreeOfWisdomNextGarden();
    /*inline*/ void         TreeOfWisdomToolUpdate(GridItem* theZenTool);
    void                    TreeOfWisdomOpenStore();
    void                    TreeOfWisdomLeave();
    void                    TreeOfWisdomGrow();
    /*inline*/ void         TreeOfWisdomTool(int theMouseX, int theMouseY);
    int                    TreeOfWisdomHitTest(int theX, int theY, HitResult* theHitResult);
    void                    TreeOfWisdomBabble();
    void                    TreeOfWisdomGiveWisdom();
    void                    TreeOfWisdomSayRepeat();
    int                    TreeOfWisdomCanFeed();

    GridItem*               GetPortalLeftRight(int theGridX, int theGridY, int theToLeft = true);
};

extern SeedType gArtChallengeWallnut[6][9];
extern SeedType gArtChallengeSunFlower[6][9];
extern SeedType gArtChallengeStarFruit[6][9];

class ZombieAllowedLevels
{
public:
    ZombieType                      mZombieType;
    int32_t                         mAllowedOnLevel[50];
};
extern int gZombieWaves[NUM_LEVELS];
extern ZombieAllowedLevels gZombieAllowedLevels[NUM_ZOMBIE_TYPES];

#endif
