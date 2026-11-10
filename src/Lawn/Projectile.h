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

#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include <cstdint>
#include "../ConstEnums.h"
#include "GameObject.h"

class Plant;
class Zombie;
namespace Sexy
{
    class Graphics;
};
using namespace Sexy;

class ProjectileDefinition
{
public:
    ProjectileType          mProjectileType;
    int32_t                 mImageRow;
    int32_t                 mDamage;
};
extern const ProjectileDefinition gProjectileDefinition[NUM_PROJECTILES];

class Projectile : public GameObject
{
public:
    int32_t                 mFrame;
    int32_t                 mNumFrames;
    int32_t                 mAnimCounter;
    float                   mPosX;
    float                   mPosY;
    float                   mPosZ;
    float                   mVelX;
    float                   mVelY;
    float                   mVelZ;
    float                   mAccZ;
    float                   mShadowY;
    bool                    mDead;
    int32_t                 mAnimTicksPerFrame;
    ProjectileMotion        mMotionType;
    ProjectileType          mProjectileType;
    int32_t                 mProjectileAge;
    int32_t                 mClickBackoffCounter;
    float                   mRotation;
    float                   mRotationSpeed;
    bool                    mOnHighGround;
    int32_t                 mDamageRangeFlags;
    int32_t                 mHitTorchwoodGridX;
    AttachmentID            mAttachmentID;
    float                   mCobTargetX;
    int32_t                 mCobTargetRow;
    ZombieID                mTargetZombieID;
    ZombieID                mPiercedZombieID;
    ZombieID                mPiercedZombieIDList[16];
    int                     mPiercedZombieIDCount;
    PlantID                 mOwnerPlantID;
    int32_t                 mCharmProbability;
    int32_t                 mLastPortalX;
    bool                    mIsBouncingPea;   // 双向射手后向豌豆：碰到墙/僵尸会反弹
    bool                    mIsBigMelon;      // 西瓜投手的大西瓜：5 倍伤害 / 全场溅射（斗蛐蛐仅本行）
    bool                    mIsPiercingSpike; // 仙人掌的刺：可穿透僵尸
    bool                    mIsCatapultZombieBall; // 魅惑篮球车的球：命中僵尸
    bool                    mIsUltWinterMelon; // 冰西瓜大招弹：命中直接冰冻 5s，溅射目标 +5 层寒意
    bool                    mIsCattailShot;
    int32_t                 mDamageMultiplierPercent;
    int32_t                 mErrorDamageMultiplier = 1;
    int32_t                 mDamageSourceRow;
    int32_t                 mStarBounceCount;
    int32_t                 mStarDamagePercent;
    bool                    mIsPiercingStar;
    bool                    mRuleSplash;
    int32_t                 mRuleFreezeChance;
    bool                    mHasStarfruitSplit;
    bool                    mStarfruitChecked;
    int32_t                 mStarfruitBounceCount;
    int32_t                 mPvPHostProjectileID;
    bool                    mPvPSyncSeen;
    bool                    mBloverSpeedBoost;
    PlantID                 mIronMagnetPlantID;
    float                   mIronOrbitAngle;
    float                   mIronSpeed;
    int32_t                 mIronOrbitTicks;
    bool                    mIronHeated;
    bool                    mIronMagnetized;
    ZombieID                mIronAttachedZombieID;
    int32_t                 mIronAttachCounter;

public:
    Projectile();
    ~Projectile();

    void                    ProjectileInitialize(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType);
    void                    Update();
    void                    Draw(Graphics* g);
    void                    DrawShadow(Graphics* g);
    void                    Die();
    void                    DoImpact(Zombie* theZombie);
    void                    UpdateMotion();
    void                    CheckForCollision();
    Zombie*                 FindCollisionTarget();
    void                    UpdateLobMotion();
    void                    CheckForHighGround();
    bool                    CantHitHighGround();
    void                    DoSplashDamage(Zombie* theZombie);
    const ProjectileDefinition&   GetProjectileDef();
    int                     GetDamage() const;
    unsigned int            GetDamageFlags(Zombie* theZombie/* = nullptr*/);
    Rect                    GetProjectileRect();
    void                    UpdateNormalMotion();
    Plant*                  FindCollisionTargetPlant();
    void                    ConvertToFireball(int theGridX);
    void                    ConvertToFireHypno(int theGridX);
    void                    ConvertToSuperFire(int theGridX);
    void                    ConvertToIceFire(int theGridX);
    void                    ConvertToIceFire2(int theGridX);
    void                    ConvertToPea(int theGridX);
    bool                    IsSplashDamage(Zombie* theZombie/* = nullptr*/);
    void                    PlayImpactSound(Zombie* theZombie);
    bool                    IsZombieHitBySplash(Zombie* theZombie);
    bool                    PeaAboutToHitTorchwood();
    bool                    SplitAtStarfruit();

};

#endif
