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

#include "Board.h"
#include "Plant.h"
#include "Zombie.h"
#include "Cutscene.h"
#include "Challenge.h"
#include "Projectile.h"
#include "../LawnApp.h"
#include "../Resources.h"
#include "../GameConstants.h"
#include "../Sexy.TodLib/TodFoley.h"
#include "../Sexy.TodLib/TodDebug.h"
#include "../Sexy.TodLib/Reanimator.h"
#include "../Sexy.TodLib/Attachment.h"
#include "Widget/AchievementsScreen.h"

constinit const ProjectileDefinition gProjectileDefinition[] = {
	{ .mProjectileType = ProjectileType::PROJECTILE_PEA, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_SNOWPEA, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_CABBAGE, .mImageRow = 0, .mDamage = 40 },
	{ .mProjectileType = ProjectileType::PROJECTILE_MELON, .mImageRow = 0, .mDamage = 80 },
	{ .mProjectileType = ProjectileType::PROJECTILE_PUFF, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_WINTERMELON, .mImageRow = 0, .mDamage = 80 },
	{ .mProjectileType = ProjectileType::PROJECTILE_FIREBALL, .mImageRow = 0, .mDamage = 40 },
	{ .mProjectileType = ProjectileType::PROJECTILE_STAR, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_SPIKE, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_BASKETBALL, .mImageRow = 0, .mDamage = 75 },
	{ .mProjectileType = ProjectileType::PROJECTILE_KERNEL, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_COBBIG, .mImageRow = 0, .mDamage = 300 },
	{ .mProjectileType = ProjectileType::PROJECTILE_BUTTER, .mImageRow = 0, .mDamage = 40 },
	{ .mProjectileType = ProjectileType::PROJECTILE_ZOMBIE_PEA, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_SNOWPEA_PIERCING, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_SNOWPEA_FIRE, .mImageRow = 0, .mDamage = 100 },
	{ .mProjectileType = ProjectileType::PROJECTILE_SNOWPEA_FIRE2, .mImageRow = 0, .mDamage = 100 },
	{ .mProjectileType = ProjectileType::PROJECTILE_IRONPEA, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_HYPNOSHROOM_PEA, .mImageRow = 0, .mDamage = 20 },
	{ .mProjectileType = ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE, .mImageRow = 0, .mDamage = 40 },
	{ .mProjectileType = ProjectileType::PROJECTILE_FIRE_SUPER, .mImageRow = 0, .mDamage = 80 }
};

static int ClampCricketProjectileDamage(int64_t theDamage, LawnApp* theApp)
{
	theDamage = std::max<int64_t>(0, theDamage);
	if (theApp->IsCricketMode())
		theDamage = std::min<int64_t>(theDamage, 1000);
	return static_cast<int>(std::min<int64_t>(theDamage, INT_MAX));
}

Projectile::Projectile()
{
}

Projectile::~Projectile()
{
	AttachmentDie(mAttachmentID);
}

void Projectile::ProjectileInitialize(int theX, int theY, int theRenderOrder, int theRow, ProjectileType theProjectileType)
{
	int aGridX = mBoard->PixelToGridXKeepOnBoard(theX, theY);
	mProjectileType = theProjectileType;
	mIronMagnetPlantID = PlantID::PLANTID_NULL;
	mIronOrbitAngle = 0.0f;
	mIronSpeed = 20.0f;
	mIronOrbitTicks = 0;
	mIronHeated = false;
	mIronMagnetized = false;
	mIronAttachedZombieID = ZombieID::ZOMBIEID_NULL;
	mIronAttachCounter = 0;
	mPosX = theX;
	mPosY = theY;
	mPosZ = 0.0f;
	mVelX = 0.0f;
	mVelY = 0.0f;
	mVelZ = 0.0f;
	mAccZ = 0.0f;
	mShadowY = mBoard->GridToPixelY(aGridX, theRow) + 67.0f;
	mHitTorchwoodGridX = -1;
	mMotionType = ProjectileMotion::MOTION_STRAIGHT;
	mFrame = 0;
	mNumFrames = 1;
	mRow = theRow;
	mCobTargetX = 0.0f;
	mDamageRangeFlags = 0;
	mDead = false;
	mAttachmentID = AttachmentID::ATTACHMENTID_NULL;
	mCobTargetRow = 0;
	mTargetZombieID = ZombieID::ZOMBIEID_NULL;
	mPiercedZombieID = ZombieID::ZOMBIEID_NULL;
	mPiercedZombieIDCount = 0;
	mOwnerPlantID = PlantID::PLANTID_NULL;
	mCharmProbability = 0;
	mIsBouncingPea = false;
	mIsBigMelon = false;
	mIsPiercingSpike = false;
    mIsCatapultZombieBall = false;
    mIsUltWinterMelon = false;
    mIsCattailShot = false;
    mDamageMultiplierPercent = 100;
    mErrorDamageMultiplier = 1;
    mDamageSourceRow = -1;
    mStarBounceCount = 0;
    mStarDamagePercent = 100;
    mIsPiercingStar = false;
    mRuleSplash = false;
    mRuleFreezeChance = 0;
    mHasStarfruitSplit = false;
    mStarfruitChecked = false;
    mStarfruitBounceCount = 0;
	mPvPHostProjectileID = -1;
	mPvPSyncSeen = false;
	mBloverSpeedBoost = false;
	mOnHighGround = mBoard->mGridSquareType[aGridX][theRow] == GridSquareType::GRIDSQUARE_HIGH_GROUND;
	if (mBoard->StageHasRoof())
	{
		mShadowY -= 12.0f;
	}
	mRenderOrder = theRenderOrder;
	mRotation = 0.0f;
	mRotationSpeed = 0.0f;
	mWidth = 40;
	mHeight = 40;
	mProjectileAge = 0;
	mClickBackoffCounter = 0;
	mAnimTicksPerFrame = 0;

	switch (mProjectileType)
	{
	case ProjectileType::PROJECTILE_CABBAGE:
	case ProjectileType::PROJECTILE_BUTTER:
		mRotation = -7 * PI / 25;  // DEG_TO_RAD(-50.4f);
		mRotationSpeed = RandRangeFloat(-0.08f, -0.02f);
		break;
	case ProjectileType::PROJECTILE_MELON:
	case ProjectileType::PROJECTILE_WINTERMELON:
		mRotation = -2 * PI / 5;  // DEG_TO_RAD(-72.0f);
		mRotationSpeed = RandRangeFloat(-0.08f, -0.02f);
		break;
	case ProjectileType::PROJECTILE_KERNEL:
		mRotation = 0.0f;
		mRotationSpeed = RandRangeFloat(-0.2f, -0.08f);
		break;
	case ProjectileType::PROJECTILE_SNOWPEA:
	case ProjectileType::PROJECTILE_SNOWPEA_PIERCING:
	{
		TodParticleSystem* aParticle = mApp->AddTodParticle(mPosX + 8.0f, mPosY + 13.0f, 400000, ParticleEffect::PARTICLE_SNOWPEA_TRAIL);
		AttachParticle(mAttachmentID, aParticle, 8.0f, 13.0f);
		break;
	}
	case ProjectileType::PROJECTILE_FIREBALL:
	case ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE:
	case ProjectileType::PROJECTILE_FIRE_SUPER:
		TOD_ASSERT(false);
		break;
	case ProjectileType::PROJECTILE_COBBIG:
		mWidth = IMAGE_REANIM_COBCANNON_COB->GetWidth();
		mHeight = IMAGE_REANIM_COBCANNON_COB->GetHeight();
		mRotation = PI / 2;
		break;
	case ProjectileType::PROJECTILE_PUFF:
	{
		TodParticleSystem* aParticle = mApp->AddTodParticle(mPosX + 13.0f, mPosY + 13.0f, 400000, ParticleEffect::PARTICLE_PUFFSHROOM_TRAIL);
		AttachParticle(mAttachmentID, aParticle, 13.0f, 13.0f);
		break;
	}
	case ProjectileType::PROJECTILE_BASKETBALL:
		mRotation = RandRangeFloat(0.0f, 2 * PI);
		mRotationSpeed = RandRangeFloat(0.05f, 0.1f);
		break;
	case ProjectileType::PROJECTILE_STAR:
		mShadowY += 15.0f;
		mRotationSpeed = RandRangeFloat(0.05f, 0.1f);
		if (Rand(2) == 0)
		{
			mRotationSpeed = -mRotationSpeed;
		}
		break;
	default:
		break;
	}

	mAnimCounter = 0;
	mX = static_cast<int>(mPosX);
	mY = static_cast<int>(mPosY);
}

Plant* Projectile::FindCollisionTargetPlant()
{
	Rect aProjectileRect = GetProjectileRect();

	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (aPlant->mRow != mRow)
			continue;

        if (aPlant->mSeedType == SeedType::SEED_SHIELDWALLNUT && mErrorDamageMultiplier <= 1)
            continue;

		if (mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA)
		{
			if (aPlant->mSeedType == SeedType::SEED_PUFFSHROOM ||
				aPlant->mSeedType == SeedType::SEED_SUNSHROOM ||
				aPlant->mSeedType == SeedType::SEED_POTATOMINE ||
				aPlant->mSeedType == SeedType::SEED_SPIKEWEED ||
				aPlant->mSeedType == SeedType::SEED_SPIKEROCK ||
				aPlant->mSeedType == SeedType::SEED_LILYPAD)  // 僵尸豌豆不能击中低矮植物
				continue;
		}

		Rect aPlantRect = aPlant->GetPlantRect();
		if (GetRectOverlap(aProjectileRect, aPlantRect) > 8)
		{
            const PlantPriority aPriority = mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA ?
                PlantPriority::TOPPLANT_EATING_ORDER : PlantPriority::TOPPLANT_CATAPULT_ORDER;
            Plant* aTopPlant = mBoard->GetTopPlantAt(aPlant->mPlantCol, aPlant->mRow, aPriority);
            if (aTopPlant && (aTopPlant->mSeedType != SeedType::SEED_SHIELDWALLNUT || mErrorDamageMultiplier > 1))
                return aTopPlant;
		}
	}

	return nullptr;
}

bool Projectile::PeaAboutToHitTorchwood()
{
	if (mMotionType != ProjectileMotion::MOTION_STRAIGHT)
		return false;

	if (mProjectileType != ProjectileType::PROJECTILE_PEA && mProjectileType != ProjectileType::PROJECTILE_IRONPEA && mProjectileType != ProjectileType::PROJECTILE_HYPNOSHROOM_PEA && mProjectileType != ProjectileType::PROJECTILE_SNOWPEA && mProjectileType != ProjectileType::PROJECTILE_SNOWPEA_PIERCING)
		return false;

	Plant* aPlant = nullptr;
	while (mBoard->IteratePlants(aPlant))
	{
		if (aPlant->mSeedType == SeedType::SEED_TORCHWOOD && aPlant->mRow == mRow && !aPlant->NotOnGround() && mHitTorchwoodGridX != aPlant->mPlantCol)
		{
			Rect aPlantAttackRect = aPlant->GetPlantAttackRect(PlantWeapon::WEAPON_PRIMARY);
			Rect aProjectileRect = GetProjectileRect();
			aProjectileRect.mX += 40;

			if (GetRectOverlap(aPlantAttackRect, aProjectileRect) > 10)
			{
				return true;
			}
		}
	}

	return false;
}

Zombie* Projectile::FindCollisionTarget()
{
	if (PeaAboutToHitTorchwood())  // “卡火炬”的原理，这段代码在两版内测版中均不存在
		return nullptr;

	Rect aProjectileRect = GetProjectileRect();
	Zombie* aBestZombie = nullptr;
	int aMinX = 0;

	Zombie* aZombie = nullptr;
	while (mBoard->IterateZombies(aZombie))
	{
		// 僵尸豌豆由普通机枪僵尸发射时，目标明确是魅惑僵尸；
		// 不依赖通用伤害筛选，避免被其他伤害范围标记误过滤。
		const bool aZombiePeaTarget =
			mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA && aZombie->mMindControlled;
		if ((aZombie->mZombieType == ZombieType::ZOMBIE_BOSS || aZombie->mRow == mRow) &&
			(aZombiePeaTarget || aZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags))))
		{
			if (aZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL && mPosZ >= 45.0f)
			{
				continue;
			}

			if (mProjectileType == ProjectileType::PROJECTILE_STAR && mProjectileAge < 25 && mVelX >= 0.0f && aZombie->mZombieType == ZombieType::ZOMBIE_DIGGER)
			{
				continue;
			}

			Rect aZombieRect = aZombie->GetZombieRect();
			if (GetRectOverlap(aProjectileRect, aZombieRect) >= 0)
			{
				if (aBestZombie == nullptr || aZombie->mX < aMinX)
				{
					aBestZombie = aZombie;
					aMinX = aZombie->mX;
				}
			}
		}
	}

	return aBestZombie;
}

void Projectile::CheckForCollision()
{
	if (mPosX > WIDE_BOARD_WIDTH || mPosX + mWidth < 0.0f)
	{
		if (mIsBouncingPea || (mHasStarfruitSplit && mStarfruitBounceCount < 3))
		{
			// 碰到墙 100% 反弹：转向反方向
			if (mPosX + mWidth < 0.0f)
				mPosX = 0.0f;
			else
				mPosX = WIDE_BOARD_WIDTH - mWidth;
			mVelX = -mVelX;
			if (mHasStarfruitSplit)
				mStarfruitBounceCount++;
			mHitTorchwoodGridX = -1;  // 反向后可再次过火
		}
		else
		{
			Die();
		}
		return;
	}
	if (mHasStarfruitSplit && (mPosY > BOARD_HEIGHT || mPosY + mHeight < 0.0f))
	{
		if (mStarfruitBounceCount >= 3)
		{
			Die();
			return;
		}
		mPosY = mPosY < 0.0f ? 0.0f : BOARD_HEIGHT - mHeight;
		mVelY = -mVelY;
		mShadowY += mVelY;
		mStarfruitBounceCount++;
		return;
	}

	if (SplitAtStarfruit())
		return;

	if (mMotionType == ProjectileMotion::MOTION_HOMING)
	{
		Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
		if (aZombie && aZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)))
		{
			Rect aProjectileRect = GetProjectileRect();
			Rect aZombieRect = aZombie->GetZombieRect();
			if (GetRectOverlap(aProjectileRect, aZombieRect) >= 0 && mPosY > aZombieRect.mY&& mPosY < aZombieRect.mY + aZombieRect.mHeight)
			{
				DoImpact(aZombie);
			}
		}
		return;
	}

	if (mProjectileType == ProjectileType::PROJECTILE_STAR && (mPosY > 600.0f || mPosY < 0.0f))
	{
		Die();
		return;
	}

	if ((mProjectileType == ProjectileType::PROJECTILE_PEA || mProjectileType == ProjectileType::PROJECTILE_IRONPEA || mProjectileType == ProjectileType::PROJECTILE_STAR) && mShadowY - mPosY > 90.0f)
	{
		return;
	}

	if (mMotionType == ProjectileMotion::MOTION_FLOAT_OVER)
	{
		return;
	}

	if (mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA)
	{
		// 普通机枪僵尸的子弹先检查魅惑僵尸，避免穿透己方僵尸后才打到植物。
		Zombie* aZombie = FindCollisionTarget();
		if (aZombie)
		{
			aZombie->TakeDamage(ClampCricketProjectileDamage(GetDamage(), mApp), GetDamageFlags(aZombie));
			PlayImpactSound(aZombie);
			DoImpact(aZombie);
			return;
		}

		Plant* aPlant = FindCollisionTargetPlant();
		if (aPlant)
		{
            if (aPlant->mSeedType == SeedType::SEED_SHIELDWALLNUT)
            {
                if (mErrorDamageMultiplier > 1)
                    aPlant->ShieldWallnutTakeDamage(GetDamage() / mErrorDamageMultiplier * 4);
            }
            else if (!aPlant->AbsorbDamageByShield(GetDamage()) &&
                (!mApp->IsCricketMode() || !mBoard->mChallenge->CricketAbsorbRowShield(aPlant->mRow, GetDamage())))
                aPlant->mPlantHealth -= GetDamage();
			aPlant->mEatenFlashCountdown = std::max(aPlant->mEatenFlashCountdown, 25);

			mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
			mApp->AddTodParticle(mPosX - 3.0f, mPosY + 17.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PEA_SPLAT);
			Die();
		}
		return;
	}

	Zombie* aZombie = FindCollisionTarget();
	if (aZombie)
	{
		if (aZombie->mOnHighGround && CantHitHighGround())
		{
			return;
		}

		if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING || mIsPiercingSpike || mIsPiercingStar)
		{
			// 用列表去重，避免密集尸群时反复命中同一只而漏掉后面的
			ZombieID aZombieID = mBoard->ZombieGetID(aZombie);
			bool aAlreadyPierced = false;
			for (int i = 0; i < mPiercedZombieIDCount; i++)
			{
				if (mPiercedZombieIDList[i] == aZombieID)
				{
					aAlreadyPierced = true;
					break;
				}
			}
			if (aAlreadyPierced)
				return;

			if (mPiercedZombieIDCount < 16)
			{
				mPiercedZombieIDList[mPiercedZombieIDCount++] = aZombieID;
			}

			unsigned int aDamageFlags = GetDamageFlags(aZombie);
			aZombie->TakeDamage(ClampCricketProjectileDamage(static_cast<int64_t>(GetDamage()) * mDamageMultiplierPercent * mStarDamagePercent / 10000, mApp), aDamageFlags);
			PlayImpactSound(aZombie);

			if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING && !aZombie->mIsMadDogZombie && !aZombie->mIsErrorZombie)
			{
				// 穿透弹每只僵尸只会命中一次，直接叠上寒意
				aZombie->ApplyChill(false);
				aZombie->mChilledCounter = 600;
			}

			float aSplatPosX = mPosX - 3.0f;
			float aSplatPosY = mPosY + 12.0f;
			float aPosX = aSplatPosX + 52.0f - aZombie->mX;
			float aPosY = aSplatPosY - aZombie->mY;
			if (aZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL || aZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL)
			{
				aPosY += 60.0f;
			}
			aPosY = ClampFloat(aPosY, 20.0f, 100.0f);
			aZombie->AddAttachedParticle(aPosX, aPosY, mIsPiercingSpike ? ParticleEffect::PARTICLE_PEA_SPLAT : ParticleEffect::PARTICLE_SNOWPEA_SPLAT);
			return;
		}

		DoImpact(aZombie);
		if (mProjectileType == ProjectileType::PROJECTILE_IRONPEA && !mIronMagnetized)
		{
			mIronMagnetized = true;
			mIronAttachedZombieID = mBoard->ZombieGetID(aZombie);
			mIronAttachCounter = 300;
			return;
		}
	}
}

bool Projectile::SplitAtStarfruit()
{
	const bool aSplittableLobbedProjectile =
		mProjectileType == ProjectileType::PROJECTILE_CABBAGE ||
		mProjectileType == ProjectileType::PROJECTILE_KERNEL ||
		mProjectileType == ProjectileType::PROJECTILE_BUTTER ||
		mProjectileType == ProjectileType::PROJECTILE_MELON ||
		mProjectileType == ProjectileType::PROJECTILE_WINTERMELON;
	if (mHasStarfruitSplit || (mMotionType == ProjectileMotion::MOTION_LOBBED && !aSplittableLobbedProjectile))
		return false;

	Plant* aStarfruit = nullptr;
	while (mBoard->IteratePlants(aStarfruit))
	{
		if (aStarfruit->mSeedType != SeedType::SEED_STARFRUIT || aStarfruit->mRow != mRow || aStarfruit->mDead)
			continue;
		// 杨桃自己射出的五角星不消耗本株的分裂判定，才能在飞到另一株杨桃时分裂。
		if (mOwnerPlantID == static_cast<PlantID>(mBoard->mPlants.DataArrayGetID(aStarfruit)))
			continue;
		if (GetRectOverlap(GetProjectileRect(), aStarfruit->GetPlantRect()) <= 0)
			continue;
		if (mStarfruitChecked)
			return false;
		mStarfruitChecked = true;
		if (Rand(100) >= 25)
			return false;

		const float aSpeed = 3.33f;
		const float aDiagonalX = cos(DEG_TO_RAD(30.0f)) * aSpeed;
		const float aDiagonalY = sin(DEG_TO_RAD(30.0f)) * aSpeed;
		const float aVelX[5] = { -aSpeed, 0.0f, 0.0f, aDiagonalX, aDiagonalX };
		const float aVelY[5] = { 0.0f, aSpeed, -aSpeed, aDiagonalY, -aDiagonalY };
		ProjectileType aCopyType = mProjectileType;
		if (aCopyType == ProjectileType::PROJECTILE_FIREBALL || aCopyType == ProjectileType::PROJECTILE_FIRE_SUPER)
			aCopyType = ProjectileType::PROJECTILE_PEA;
		else if (aCopyType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || aCopyType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2)
			aCopyType = ProjectileType::PROJECTILE_SNOWPEA;
		else if (aCopyType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE)
			aCopyType = ProjectileType::PROJECTILE_HYPNOSHROOM_PEA;
		for (int i = 0; i < 5; i++)
		{
			Projectile* aCopy = mBoard->AddProjectile(static_cast<int>(mPosX), static_cast<int>(mPosY),
				mRenderOrder, mRow, aCopyType);
			if (!aCopy)
				continue;
			aCopy->mDamageRangeFlags = mDamageRangeFlags;
			aCopy->mDamageMultiplierPercent = mDamageMultiplierPercent * 2;
            aCopy->mErrorDamageMultiplier = mErrorDamageMultiplier;
			aCopy->mOwnerPlantID = mOwnerPlantID;
			aCopy->mCharmProbability = mCharmProbability;
			aCopy->mIsPiercingStar = mIsPiercingStar;
			aCopy->mIsBigMelon = mIsBigMelon;
			aCopy->mIsUltWinterMelon = mIsUltWinterMelon;
			aCopy->mDamageSourceRow = mDamageSourceRow;
			aCopy->mMotionType = ProjectileMotion::MOTION_STAR;
			aCopy->mVelX = aVelX[i];
			aCopy->mVelY = aVelY[i];
			aCopy->mHasStarfruitSplit = true;
			aCopy->mBloverSpeedBoost = mBloverSpeedBoost;
			if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL)
				aCopy->ConvertToFireball(aStarfruit->mPlantCol);
			else if (mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER)
				aCopy->ConvertToSuperFire(aStarfruit->mPlantCol);
			else if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE)
				aCopy->ConvertToIceFire(aStarfruit->mPlantCol);
			else if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2)
				aCopy->ConvertToIceFire2(aStarfruit->mPlantCol);
			else if (mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE)
				aCopy->ConvertToFireHypno(aStarfruit->mPlantCol);
		}
		Die();
		return true;
	}
	return false;
}

bool Projectile::CantHitHighGround()
{
	if (mMotionType == ProjectileMotion::MOTION_BACKWARDS || mMotionType == ProjectileMotion::MOTION_HOMING)
		return false;

	return (
		mProjectileType == ProjectileType::PROJECTILE_PEA ||
		mProjectileType == ProjectileType::PROJECTILE_IRONPEA ||
		mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING ||
		mProjectileType == ProjectileType::PROJECTILE_STAR ||
		mProjectileType == ProjectileType::PROJECTILE_PUFF ||
		mProjectileType == ProjectileType::PROJECTILE_FIREBALL ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 ||
		mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE ||
		mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER
		) && !mOnHighGround;
}

void Projectile::CheckForHighGround()
{
	float aShadowDelta = mShadowY - mPosY;

	if (mProjectileType == ProjectileType::PROJECTILE_PEA ||
		mProjectileType == ProjectileType::PROJECTILE_IRONPEA ||
		mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING ||
		mProjectileType == ProjectileType::PROJECTILE_FIREBALL ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 ||
		mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE ||
		mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER ||
		mProjectileType == ProjectileType::PROJECTILE_SPIKE ||
		mProjectileType == ProjectileType::PROJECTILE_COBBIG)
	{
		if (aShadowDelta < 28.0f)
		{
			DoImpact(nullptr);
			return;
		}
	}

	if (mProjectileType == ProjectileType::PROJECTILE_PUFF && aShadowDelta < 0.0f)
	{
		DoImpact(nullptr);
		return;
	}

	if (mProjectileType == ProjectileType::PROJECTILE_STAR && aShadowDelta < 23.0f)
	{
		DoImpact(nullptr);
		return;
	}

	if (CantHitHighGround())
	{
		int aGridX = mBoard->PixelToGridXKeepOnBoard(mPosX + 30, mPosY);
		if (mBoard->mGridSquareType[aGridX][mRow] == GridSquareType::GRIDSQUARE_HIGH_GROUND)
		{
			DoImpact(nullptr);
		}
	}
}

bool Projectile::IsSplashDamage(Zombie* theZombie)
{
	if ((mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER) && theZombie && theZombie->IsFireResistant())
		return false;

	return
		mRuleSplash ||
		mProjectileType == ProjectileType::PROJECTILE_MELON ||
		mProjectileType == ProjectileType::PROJECTILE_WINTERMELON ||
		mProjectileType == ProjectileType::PROJECTILE_FIREBALL ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 ||
		mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE ||
		mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER;
}

unsigned int Projectile::GetDamageFlags(Zombie* theZombie)
{
	unsigned int aDamageFlags = 0U;

	if (IsSplashDamage(theZombie))
	{
		SetBit(aDamageFlags, static_cast<int>(DamageFlags::DAMAGE_HITS_SHIELD_AND_BODY), true);
	}
	else if (mMotionType == ProjectileMotion::MOTION_LOBBED || mMotionType == ProjectileMotion::MOTION_BACKWARDS)
	{
		SetBit(aDamageFlags, static_cast<int>(DamageFlags::DAMAGE_BYPASSES_SHIELD), true);
	}
	else if (mMotionType == ProjectileMotion::MOTION_STAR && mVelX < 0.0f)
	{
		SetBit(aDamageFlags, static_cast<int>(DamageFlags::DAMAGE_BYPASSES_SHIELD), true);
	}

	if (mProjectileType == ProjectileType::PROJECTILE_WINTERMELON || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2)
	{
		SetBit(aDamageFlags, static_cast<int>(DamageFlags::DAMAGE_FREEZE), true);
	}

	return aDamageFlags;
}

bool Projectile::IsZombieHitBySplash(Zombie* theZombie)
{
	Rect aProjectileRect = GetProjectileRect();
	if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER)
	{
		aProjectileRect.mWidth = 100;
	}

	int aRowDeviation = theZombie->mRow - mRow;
	Rect aZombieRect = theZombie->GetZombieRect();
	if (theZombie->IsFireResistant() && (mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER))
	{
		return false;
	}

	if (theZombie->mZombieType == ZombieType::ZOMBIE_BOSS)
	{
		aRowDeviation = 0;
	}
	if (mIsBigMelon)
	{
		// 大西瓜：全场溅射（命中所有僵尸）；斗蛐蛐模式下仅本行全行溅射
		if (mApp->IsCricketMode())
		{
			return theZombie->mRow == mRow && theZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags));
		}
		return theZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags));
	}
	if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER)
	{
		if (aRowDeviation != 0)
		{
			return false;
		}
	}
	else if (aRowDeviation > 1 || aRowDeviation < -1)
	{
		return false;
	}

	return theZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)) && GetRectOverlap(aProjectileRect, aZombieRect) >= 0;
}

void Projectile::DoSplashDamage(Zombie* theZombie)
{
	int aZombiesGetSplashed = 0;
	Zombie* aZombie = nullptr;
	while (mBoard->IterateZombies(aZombie))
	{
		if (aZombie != theZombie && IsZombieHitBySplash(aZombie))
		{
			aZombiesGetSplashed++;
		}
	}

	int64_t aOriginalDamage = static_cast<int64_t>(GetDamage()) * mDamageMultiplierPercent / 100;
	if (mIsBigMelon)
		aOriginalDamage *= 60;  // 大西瓜 60 倍伤害
	if (aOriginalDamage < 0)
		aOriginalDamage = 0;
	int64_t aSplashDamage = aOriginalDamage / 3;
	int64_t aMaxSplashDamageAmount = aOriginalDamage * 7;
	if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER)
	{
		aMaxSplashDamageAmount = aOriginalDamage;
	}
	int64_t aSplashDamageAmount = aSplashDamage * aZombiesGetSplashed;
	if (aSplashDamageAmount > aMaxSplashDamageAmount)
	{
		//aSplashDamage *= aMaxSplashDamageAmount / aSplashDamage;
		aSplashDamage = std::max<int64_t>(aOriginalDamage * aMaxSplashDamageAmount / (aSplashDamageAmount * 3), 1);
	}

	aZombie = nullptr;
	while (mBoard->IterateZombies(aZombie))
	{
		if (IsZombieHitBySplash(aZombie))
		{
			unsigned int aDamageFlags = GetDamageFlags(aZombie);
			if (aZombie == theZombie)
			{
				aZombie->TakeDamage(ClampCricketProjectileDamage(aOriginalDamage, mApp), aDamageFlags);
			}
			else
			{
				aZombie->TakeDamage(ClampCricketProjectileDamage(aSplashDamage, mApp), aDamageFlags);
			}

			// 冰西瓜也叠寒意
			if (mProjectileType == ProjectileType::PROJECTILE_WINTERMELON && !aZombie->mIsMadDogZombie && !aZombie->mIsErrorZombie)
			{
				if (mIsUltWinterMelon)
				{
					// 大招弹：主目标直接冰冻 5s（500 帧）；溅射目标 +5 层寒意
					if (aZombie == theZombie)
					{
						aZombie->ApplyChill(false);
						aZombie->mChilledCounter = 500;
					}
					else
					{
						aZombie->mChillStacks += 5;
					}
				}
				else
				{
					aZombie->ApplyChill(false);
					aZombie->mChilledCounter = 600;
				}
			}

			if (mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE && mOwnerPlantID != PlantID::PLANTID_NULL && !aZombie->mMindControlled && !aZombie->mIsErrorZombie && Rand(100) < mCharmProbability)
			{
				aZombie->StartMindControlled();
				Plant* aPlant = mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mOwnerPlantID));
				if (aPlant)
					aPlant->mCharmProbability = 10;
			}
		}
	}
}

// GOTY @Patoke: 0x471B41
void Projectile::UpdateLobMotion()
{
	if (mProjectileType == ProjectileType::PROJECTILE_COBBIG && mPosZ < -700.0f)
	{
		mVelZ = 8.0f;
		mRow = mCobTargetRow;
		mPosX = mCobTargetX;
		int aCobTargetCol = mBoard->PixelToGridXKeepOnBoard(mCobTargetX, 0);
		mPosY = mBoard->GridToPixelY(aCobTargetCol, mCobTargetRow);
		mShadowY = mPosY + 67.0f;
		mRotation = -PI / 2;
	}

	mVelZ += mAccZ;
	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY)
	{
		mVelZ += mAccZ;
	}
	mPosX += mVelX;
	mPosY += mVelY;
	mPosZ += mVelZ;

	bool isRising = mVelZ < 0.0f;
	if (isRising && (mProjectileType == ProjectileType::PROJECTILE_BASKETBALL || mProjectileType == ProjectileType::PROJECTILE_COBBIG))
	{
		return;
	}
	if (mProjectileAge > 20)
	{
		if (isRising)
		{
			return;
		}

		float aMinCollisionZ = 0.0f;
		if (mProjectileType == ProjectileType::PROJECTILE_BUTTER)
		{
			aMinCollisionZ = -32.0f;
		}
		else if (mProjectileType == ProjectileType::PROJECTILE_BASKETBALL)
		{
			aMinCollisionZ = 60.0f;
		}
		else if (mProjectileType == ProjectileType::PROJECTILE_MELON || mProjectileType == ProjectileType::PROJECTILE_WINTERMELON)
		{
			aMinCollisionZ = -35.0f;
		}
		else if (mProjectileType == ProjectileType::PROJECTILE_CABBAGE || mProjectileType == ProjectileType::PROJECTILE_KERNEL)
		{
			aMinCollisionZ = -30.0f;
		}
		else if (mProjectileType == ProjectileType::PROJECTILE_COBBIG)
		{
			aMinCollisionZ = -60.0f;
		}
		if (mBoard->mPlantRow[mRow] == PlantRowType::PLANTROW_POOL)
		{
			aMinCollisionZ += 40.0f;
		}

		if (mPosZ <= aMinCollisionZ)
		{
			return;
		}
	}

	// 常规抛射物在下落时也能掠过杨桃，转化成五向分裂投掷物。
	const bool aSplittableLobbedProjectile =
		mProjectileType == ProjectileType::PROJECTILE_CABBAGE ||
		mProjectileType == ProjectileType::PROJECTILE_KERNEL ||
		mProjectileType == ProjectileType::PROJECTILE_BUTTER ||
		mProjectileType == ProjectileType::PROJECTILE_MELON ||
		mProjectileType == ProjectileType::PROJECTILE_WINTERMELON;
	if (!isRising && aSplittableLobbedProjectile)
	{
		mX = static_cast<int>(mPosX);
		mY = static_cast<int>(mPosY + mPosZ);
		if (SplitAtStarfruit())
			return;
	}

	Plant* aPlant = nullptr;
	Zombie* aZombie = nullptr;
	if (mProjectileType == ProjectileType::PROJECTILE_BASKETBALL || mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA)
	{
		if (mIsCatapultZombieBall)  // 魅惑篮球车的球：找僵尸
			aZombie = FindCollisionTarget();
		else
			aPlant = FindCollisionTargetPlant();
	}
	else
	{
		aZombie = FindCollisionTarget();
	}

	float aGroundZ = 80.0f;
	if (mProjectileType == ProjectileType::PROJECTILE_COBBIG)
	{
		aGroundZ = -40.0f;
	}
	bool hitGround = mPosZ > aGroundZ;
	if (aZombie == nullptr && aPlant == nullptr && !hitGround)
	{
		return;
	}

	if (aZombie && mIsCatapultZombieBall)  // 魅惑篮球：砸僵尸
	{
		aZombie->TakeDamage(GetDamage(), GetDamageFlags(aZombie));
		mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
		mApp->AddTodParticle(mPosX + 10.0f, mPosY + 15.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_PEA_SPLAT);
		Die();
	}
	else if (aPlant)
	{
		Plant* aUmbrellaPlant = mBoard->FindUmbrellaPlant(aPlant->mPlantCol, aPlant->mRow);
		if (aUmbrellaPlant)
		{
			if (aUmbrellaPlant->mState == PlantState::STATE_UMBRELLA_REFLECTING)
			{
				mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
				int aRenderPosition = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_TOP, 0, 1);
				mApp->AddTodParticle(mPosX + 20.0f, mPosY + 20.0f, aRenderPosition, ParticleEffect::PARTICLE_UMBRELLA_REFLECT);
				Die();
			}
			else if (aUmbrellaPlant->mState != PlantState::STATE_UMBRELLA_TRIGGERED)
			{
				mApp->PlayFoley(FoleyType::FOLEY_UMBRELLA);
				aUmbrellaPlant->DoSpecial();
			}
		}
		else
		{
            if (aPlant->mSeedType == SeedType::SEED_SHIELDWALLNUT)
            {
                if (mErrorDamageMultiplier > 1)
                    aPlant->ShieldWallnutTakeDamage(GetDamage() / mErrorDamageMultiplier * 4);
            }
            else if (!aPlant->AbsorbDamageByShield(GetDamage()) &&
                (!mApp->IsCricketMode() || !mBoard->mChallenge->CricketAbsorbRowShield(aPlant->mRow, GetDamage())))
                aPlant->mPlantHealth -= GetDamage();
			aPlant->mEatenFlashCountdown = std::max(aPlant->mEatenFlashCountdown, 25);
			mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
			Die();
		}
	}
	else if (mProjectileType == ProjectileType::PROJECTILE_COBBIG)
	{
		// @Patoke: implemented
		int aBeforeGargantuarCount = mBoard->GetLiveGargantuarCount();
		mBoard->KillAllZombiesInRadius(mRow, mPosX + 80, mPosY + 40, 115, 1, true, mDamageRangeFlags, 1800 * mErrorDamageMultiplier);
		int aAfterGargantuarCount = mBoard->GetLiveGargantuarCount();
		mBoard->mGargantuarsKillsByCornCob += aBeforeGargantuarCount - aAfterGargantuarCount;
		if (mBoard->mGargantuarsKillsByCornCob >= 2)
			ReportAchievement::GiveAchievement(mApp, PopcornParty, true);

		DoImpact(nullptr);
	}
	else
	{
		DoImpact(aZombie);
	}
}

void Projectile::UpdateNormalMotion()
{
	if (mProjectileType == ProjectileType::PROJECTILE_IRONPEA && mIronMagnetized)
	{
		Zombie* aZombie = mBoard->ZombieTryToGet(mIronAttachedZombieID);
		if (!aZombie || aZombie->IsDeadOrDying() || mIronAttachCounter <= 0)
		{
			Die();
			return;
		}
		mIronAttachCounter--;
		mPosX = aZombie->mX + 35.0f;
		mPosY = aZombie->mY + 35.0f;
		mShadowY = mPosY + 30.0f;
		mVelX = mVelY = 0.0f;
		// Nearby projectiles have a chance to bend into this zombie's row.
		Projectile* aNearby = nullptr;
		while (mBoard->IterateProjectiles(aNearby))
		{
			if (aNearby == this || aNearby->mDead || aNearby->mRow != aZombie->mRow)
				continue;
			int aDX = aNearby->mX - aZombie->mX;
			int aDY = aNearby->mY - aZombie->mY;
			if (std::abs(aDX) <= 160 && std::abs(aDY) <= 160 && Rand(100) < 25)
			{
				aNearby->mRow = aZombie->mRow;
				aNearby->mVelY = 0.0f;
			}
		}
		return;
	}
	if (mProjectileType == ProjectileType::PROJECTILE_IRONPEA && mIronMagnetPlantID != PlantID::PLANTID_NULL)
	{
		Plant* aMagnet = mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mIronMagnetPlantID));
		if (aMagnet && aMagnet->mPlantHealth > 0)
		{
			// Adjacent magnet shrooms cooperate: every additional shroom expands
			// the orbit and raises its speed cap without a hard stack limit.
			int aMagnetStack = 0;
			float aGroupCenterX = 0.0f;
			float aGroupCenterY = 0.0f;
			Plant* aNearbyMagnet = nullptr;
			while (mBoard->IteratePlants(aNearbyMagnet))
			{
				if (aNearbyMagnet->mSeedType != SeedType::SEED_MAGNETSHROOM ||
					aNearbyMagnet->mPlantHealth <= 0 || aNearbyMagnet->mRow != aMagnet->mRow)
					continue;
				// All active magnet shrooms in the row contribute to the shared orbit.
				aMagnetStack++;
				aGroupCenterX += aNearbyMagnet->mX + 40.0f;
				aGroupCenterY += aNearbyMagnet->mY + 40.0f;
			}
			aMagnetStack = std::max(1, aMagnetStack);
			aGroupCenterX /= static_cast<float>(aMagnetStack);
			aGroupCenterY /= static_cast<float>(aMagnetStack);
			const float aSpeedCap = 33.3f * static_cast<float>(aMagnetStack);
			mIronSpeed = std::min(aSpeedCap, mIronSpeed + 0.08f);
			mIronOrbitTicks++;
			if (!mIronMagnetized && mIronOrbitTicks >= 600)
				mIronMagnetized = true;
			mIronOrbitAngle += 0.035f * (mIronSpeed / 3.33f);
			float aRadiusX = 52.0f * static_cast<float>(aMagnetStack);
			float aRadiusY = 34.0f * static_cast<float>(aMagnetStack);
			mPosX = aGroupCenterX + cosf(mIronOrbitAngle) * aRadiusX;
			mPosY = aGroupCenterY + sinf(mIronOrbitAngle) * aRadiusY;
			mShadowY = aGroupCenterY + 27.0f;
			mVelX = -sinf(mIronOrbitAngle) * mIronSpeed;
			mVelY = cosf(mIronOrbitAngle) * mIronSpeed;
			mRotation = -atan2f(mVelY, mVelX);
			CheckForCollision();
			return;
		}
		mIronMagnetPlantID = PlantID::PLANTID_NULL;
	}
	const float aSpeedMultiplier = mBloverSpeedBoost ? 1.5f : 1.0f;
	if (mMotionType == ProjectileMotion::MOTION_BACKWARDS)
	{
		if (mIsBouncingPea)
			mPosX += ((mVelX != 0.0f) ? mVelX : -3.33f) * aSpeedMultiplier;  // 反弹豌豆按 mVelX 方向移动
		else
			mPosX -= 3.33f * aSpeedMultiplier;
	}
	else if (mMotionType == ProjectileMotion::MOTION_HOMING)
	{
		Zombie* aZombie = mBoard->ZombieTryToGet(mTargetZombieID);
		if (aZombie && aZombie->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)))
		{
			Rect aZombieRect = aZombie->GetZombieRect();
			SexyVector2 aTargetCenter(aZombie->ZombieTargetLeadX(0.0f), aZombieRect.mY + aZombieRect.mHeight / 2);
			SexyVector2 aProjectileCenter(mPosX + mWidth / 2, mPosY + mHeight / 2);
			SexyVector2 aToTarget = (aTargetCenter - aProjectileCenter).Normalize();
			SexyVector2 aMotion(mVelX, mVelY);

			aMotion += aToTarget * (0.001f * mProjectileAge);
			aMotion = aMotion.Normalize();
			aMotion *= 2.0f * aSpeedMultiplier;

			mVelX = aMotion.x;
			mVelY = aMotion.y;
			mRotation = -atan2(mVelY, mVelX);
		}

		mPosY += mVelY;
		mPosX += mVelX;
		mShadowY += mVelY;
		mRow = mBoard->PixelToGridYKeepOnBoard(mPosX, mPosY);
	}
	else if (mMotionType == ProjectileMotion::MOTION_STAR)
	{
		mPosY += mVelY;
		mPosX += mVelX;
		mShadowY += mVelY;

		if (mVelY != 0.0f)
		{
			mRow = mBoard->PixelToGridYKeepOnBoard(mPosX, mPosY);
		}
	}
	else if (mMotionType == ProjectileMotion::MOTION_BEE)
	{
		if (mProjectileAge < 60)
		{
			mPosY -= 0.5f;
		}
		mPosX += 3.33f;
	}
	else if (mMotionType == ProjectileMotion::MOTION_FLOAT_OVER)
	{
		if (mVelZ < 0.0f)
		{
			mVelZ += 0.002f;
			mVelZ = std::min(mVelZ, 0.0f);
			mPosY += mVelZ;
			mRotation = 0.3f - 0.7f * mVelZ * PI * 0.25f;
		}
		mPosX += 0.4f;
	}
	else if (mMotionType == ProjectileMotion::MOTION_BEE_BACKWARDS)
	{
		if (mProjectileAge < 60)
		{
			mPosY -= 0.5f;
		}
		mPosX -= 3.33f;
	}
	else if (mMotionType == ProjectileMotion::MOTION_THREEPEATER)
	{
		mPosX += 3.33f * aSpeedMultiplier;
		mPosY += mVelY;
		mVelY *= 0.97f;
		mShadowY += mVelY;
	}
	else
	{
		mPosX += 3.33f * aSpeedMultiplier;
	}

	if (mApp->mGameMode == GameMode::GAMEMODE_CHALLENGE_HIGH_GRAVITY)
	{
		if (mMotionType == ProjectileMotion::MOTION_FLOAT_OVER)
		{
			mVelZ += 0.004f;
		}
		else
		{
			mVelZ += 0.2f;
		}

		mPosY += mVelZ;
	}

	CheckForCollision();
	CheckForHighGround();
}

void Projectile::UpdateMotion()
{
	if (mAnimTicksPerFrame > 0)
	{
		mAnimCounter = (mAnimCounter + 1) % (mNumFrames * mAnimTicksPerFrame);
		mFrame = mAnimCounter / mAnimTicksPerFrame;
	}

	int aOldRow = mRow;
	float aOldY = mBoard->GetPosYBasedOnRow(mPosX, mRow);
	if (mMotionType == ProjectileMotion::MOTION_LOBBED)
	{
		UpdateLobMotion();
	}
	else
	{
		UpdateNormalMotion();
	}

	float aSlopeHeightChange = mBoard->GetPosYBasedOnRow(mPosX, aOldRow) - aOldY;
	if (mProjectileType == ProjectileType::PROJECTILE_COBBIG)
	{
		aSlopeHeightChange = 0.0f;  // Fix The Roof Offset Bug of Corn Cob
	}
	if (mMotionType == ProjectileMotion::MOTION_FLOAT_OVER)
	{
		mPosY += aSlopeHeightChange;
	}
	if (mMotionType == ProjectileMotion::MOTION_LOBBED)
	{
		mPosY += aSlopeHeightChange;
		mPosZ -= aSlopeHeightChange;
	}
	mShadowY += aSlopeHeightChange;
	mX = static_cast<int>(mPosX);
	mY = static_cast<int>(mPosY + mPosZ);
}

void Projectile::PlayImpactSound(Zombie* theZombie)
{
	bool aPlayHelmSound = true;
	bool aPlaySplatSound = true;
	if (mProjectileType == ProjectileType::PROJECTILE_KERNEL)
	{
		mApp->PlayFoley(FoleyType::FOLEY_KERNEL_SPLAT);
		aPlayHelmSound = false;
		aPlaySplatSound = false;
	}
	else if (mProjectileType == ProjectileType::PROJECTILE_BUTTER)
	{
		mApp->PlayFoley(FoleyType::FOLEY_BUTTER);
		aPlaySplatSound = false;
	}
	else if ((mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER) && IsSplashDamage(theZombie))
	{
		mApp->PlayFoley(FoleyType::FOLEY_IGNITE);
		aPlayHelmSound = false;
		aPlaySplatSound = false;
	}
	else if (mProjectileType == ProjectileType::PROJECTILE_MELON || mProjectileType == ProjectileType::PROJECTILE_WINTERMELON)
	{
		mApp->PlayFoley(FoleyType::FOLEY_MELONIMPACT);
		aPlaySplatSound = false;
	}

	if (aPlayHelmSound && theZombie)
	{
		if (theZombie->mHelmType == HELMTYPE_PAIL)
		{
			mApp->PlayFoley(FoleyType::FOLEY_SHIELD_HIT);
			aPlaySplatSound = false;
		}
		else if (theZombie->mHelmType == HELMTYPE_TRAFFIC_CONE || theZombie->mHelmType == HELMTYPE_DIGGER || theZombie->mHelmType == HELMTYPE_FOOTBALL)
		{
			mApp->PlayFoley(FoleyType::FOLEY_PLASTIC_HIT);
		}
	}

	if (aPlaySplatSound)
	{
		mApp->PlayFoley(FoleyType::FOLEY_SPLAT);
	}
}

void Projectile::DoImpact(Zombie* theZombie)
{
	int aDamageMultiplierPercent = mDamageMultiplierPercent;
	if (mIsCattailShot && theZombie && mDamageSourceRow >= 0 && theZombie->mRow != mDamageSourceRow)
		aDamageMultiplierPercent = 100;
	PlayImpactSound(theZombie);
	if (mIsCattailShot && theZombie && mApp->IsCricketMode() &&
		mDamageSourceRow >= 0 && mDamageSourceRow < CRICKET_ROWS && theZombie->mRow != mDamageSourceRow)
	{
		theZombie->mCattailCrossRowAttackers |= 1U << mDamageSourceRow;
	}

	if (IsSplashDamage(theZombie))
	{
		if ((mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER) && theZombie)
		{
			theZombie->RemoveColdEffects();
		}

		DoSplashDamage(theZombie);
	}
    else if (theZombie)
	{
		unsigned int aDamageFlags = GetDamageFlags(theZombie);
		theZombie->TakeDamage(ClampCricketProjectileDamage(static_cast<int64_t>(GetDamage()) * aDamageMultiplierPercent * mStarDamagePercent / 10000, mApp), aDamageFlags);
		if (mProjectileType == ProjectileType::PROJECTILE_IRONPEA && mIronHeated)
			theZombie->ApplyBurn();
	}

    if (mRuleFreezeChance > 0 && theZombie && !theZombie->IsDeadOrDying() &&
        !theZombie->mIsMadDogZombie && !theZombie->mIsErrorZombie && Rand(100) < mRuleFreezeChance)
    {
        theZombie->ApplyChill(false);
        theZombie->mChilledCounter = std::max(theZombie->mChilledCounter, 100);
    }

    if (mIsCattailShot && theZombie && !theZombie->IsDeadOrDying())
    {
        constexpr int CATTAIL_MARK_HITS = 3;
        constexpr int CATTAIL_MARK_TIME = 600;
        if (theZombie->mCattailMarkCounter == 0)
            theZombie->mCattailMarkStacks = 0;
        theZombie->mCattailMarkStacks = std::min(CATTAIL_MARK_HITS, theZombie->mCattailMarkStacks + 1);
        theZombie->mCattailMarkCounter = CATTAIL_MARK_TIME;
        if (theZombie->mCattailMarkStacks == CATTAIL_MARK_HITS)
        {
            // Marked prey takes a half-strength bonus hit from every cattail shot.
			theZombie->TakeDamage(ClampCricketProjectileDamage(static_cast<int64_t>(GetDamage()) * aDamageMultiplierPercent * mStarDamagePercent / 20000, mApp), 0U);
		}
	}
	if (mProjectileType == ProjectileType::PROJECTILE_STAR && !mIsPiercingStar && theZombie &&
		mStarBounceCount < 2 && Rand(100) < 30)
	{
		Zombie* aNextTarget = nullptr;
		float aNearestDistance = 0.0f;
		Zombie* aCandidate = nullptr;
		while (mBoard->IterateZombies(aCandidate))
		{
			if (aCandidate == theZombie || aCandidate->IsDeadOrDying() ||
				!aCandidate->EffectedByDamage(static_cast<unsigned int>(mDamageRangeFlags)))
				continue;
			float aDistance = Distance2D(mPosX, mPosY, aCandidate->mPosX, aCandidate->mPosY);
			if (!aNextTarget || aDistance < aNearestDistance)
			{
				aNextTarget = aCandidate;
				aNearestDistance = aDistance;
			}
		}
		if (aNextTarget)
		{
			Plant* aOwner = mOwnerPlantID == PlantID::PLANTID_NULL ? nullptr :
				mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mOwnerPlantID));
			if (aOwner && aNextTarget->mRow != theZombie->mRow)
				aOwner->mStarTrackCount = std::min(5, aOwner->mStarTrackCount + 1);
			mTargetZombieID = mBoard->ZombieGetID(aNextTarget);
			mMotionType = ProjectileMotion::MOTION_HOMING;
			mStarBounceCount++;
			mStarDamagePercent = mStarBounceCount == 1 ? 70 : 45;
			mPosX = theZombie->mPosX + theZombie->mWidth + 4.0f;
			mPosY = theZombie->mPosY;
			mRow = theZombie->mRow;
			return;
		}
	}

	float aLastPosX = mPosX - mVelX;
	float aLastPosY = mPosY + mPosZ - mVelY - mVelZ;
	ParticleEffect aEffect = ParticleEffect::PARTICLE_NONE;
	float aSplatPosX = mPosX + 12.0f;
	float aSplatPosY = mPosY + 12.0f;
	switch (mProjectileType)
	{
	case ProjectileType::PROJECTILE_MELON:
		mApp->AddTodParticle(aLastPosX + 30.0f, aLastPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_MELONSPLASH);
		break;
	case ProjectileType::PROJECTILE_WINTERMELON:
		mApp->AddTodParticle(aLastPosX + 30.0f, aLastPosY + 30.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_WINTERMELON);
		break;
	case ProjectileType::PROJECTILE_COBBIG:
	{
		int aRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_GROUND, mCobTargetRow, 2);
		mApp->AddTodParticle(mPosX + 80.0f, mPosY + 40.0f, aRenderOrder, ParticleEffect::PARTICLE_BLASTMARK);
		mApp->AddTodParticle(mPosX + 80.0f, mPosY + 40.0f, mRenderOrder + 1, ParticleEffect::PARTICLE_POPCORNSPLASH);
		mApp->PlaySample(SOUND_DOOMSHROOM);
		mBoard->ShakeBoard(3, -4);
		break;
	}
	case ProjectileType::PROJECTILE_HYPNOSHROOM_PEA:
	case ProjectileType::PROJECTILE_PEA:
	case ProjectileType::PROJECTILE_IRONPEA:
		aSplatPosX -= 15.0f;
		aEffect = ParticleEffect::PARTICLE_PEA_SPLAT;
		break;
	case ProjectileType::PROJECTILE_SNOWPEA:
	case ProjectileType::PROJECTILE_SNOWPEA_PIERCING:
		aSplatPosX -= 15.0f;
		aEffect = ParticleEffect::PARTICLE_SNOWPEA_SPLAT;
		break;
	case ProjectileType::PROJECTILE_FIREBALL:
	case ProjectileType::PROJECTILE_SNOWPEA_FIRE:
	case ProjectileType::PROJECTILE_SNOWPEA_FIRE2:
	case ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE:
	case ProjectileType::PROJECTILE_FIRE_SUPER:
	{
		if (IsSplashDamage(theZombie))
		{
			Reanimation* aFireReanim = mApp->AddReanimation(mPosX + 38.0f, mPosY - 20.0f, mRenderOrder + 1, ReanimationType::REANIM_JALAPENO_FIRE);
			aFireReanim->mAnimTime = 0.25f;
			aFireReanim->mAnimRate = 24.0f;
			aFireReanim->OverrideScale(0.7f, 0.4f);
		}
		break;
	}
	case ProjectileType::PROJECTILE_STAR:
		aEffect = ParticleEffect::PARTICLE_STAR_SPLAT;
		break;
	case ProjectileType::PROJECTILE_PUFF:
		aSplatPosX -= 20.0f;
		aEffect = ParticleEffect::PARTICLE_PUFF_SPLAT;
		break;
	case ProjectileType::PROJECTILE_CABBAGE:
		aSplatPosX = aLastPosX - 38.0f;
		aSplatPosY = aLastPosY + 23.0f;
		aEffect = ParticleEffect::PARTICLE_CABBAGE_SPLAT;
		break;
	case ProjectileType::PROJECTILE_BUTTER:
		aSplatPosX = aLastPosX - 20.0f;
		aSplatPosY = aLastPosY + 63.0f;
		aEffect = ParticleEffect::PARTICLE_BUTTER_SPLAT;

		if (theZombie)
		{
			theZombie->ApplyButter();
		}
		break;
	default:
		break;
	}

	if (aEffect != ParticleEffect::PARTICLE_NONE)
	{
		if (theZombie)
		{
			float aPosX = aSplatPosX + 52.0f - theZombie->mX;
			float aPosY = aSplatPosY - theZombie->mY;
			if (theZombie->mZombiePhase == ZombiePhase::PHASE_SNORKEL_WALKING_IN_POOL || theZombie->mZombiePhase == ZombiePhase::PHASE_DOLPHIN_WALKING_IN_POOL)
			{
				aPosY += 60.0f;
			}
			if (mMotionType == ProjectileMotion::MOTION_BACKWARDS)
			{
				aPosX -= 80.0f;
			}
			else if (mPosX > theZombie->mX + 40 && mMotionType != ProjectileMotion::MOTION_LOBBED)
			{
				aPosX -= 60.0f;
			}

			aPosY = ClampFloat(aPosY, 20.0f, 100.0f);
			theZombie->AddAttachedParticle(aPosX, aPosY, aEffect);
		}
		else
		{
			mApp->AddTodParticle(aSplatPosX, aSplatPosY, mRenderOrder + 1, aEffect);
		}
	}

	if (mProjectileType == ProjectileType::PROJECTILE_PUFF && theZombie)
	{
		Plant* aOwner = mOwnerPlantID == PlantID::PLANTID_NULL ? nullptr :
			mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mOwnerPlantID));
		const int aPoisonStacks = aOwner && aOwner->HasBloverBehind() ? 3 : 1;
		// 抗毒变体：毒最多只叠 1 层
		if (!theZombie->mIsPoisonResistant)
			theZombie->mPoisonStacks += aPoisonStacks;
		theZombie->mPoisonCounter = 0;
	}

	if ((mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING) && theZombie && !theZombie->mIsMadDogZombie && !theZombie->mIsErrorZombie)
	{
		int aChillThreshold = theZombie->mBerserk ? 30 : 15;
		theZombie->mChillStacks += 10;
		if (theZombie->mChillStacks >= aChillThreshold)
		{
			theZombie->mChillStacks = 0;
			theZombie->ApplyChill(false);
			theZombie->mChilledCounter = 600;
		}
	}

	if (mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA && theZombie && mOwnerPlantID != PlantID::PLANTID_NULL && !theZombie->mMindControlled && !theZombie->mIsErrorZombie)
	{
		if (Rand(100) < mCharmProbability)
		{
			theZombie->StartMindControlled();
			Plant* aPlant = mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mOwnerPlantID));
			if (aPlant)
				aPlant->mCharmProbability = 10;
		}
		else
		{
			Plant* aPlant = mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mOwnerPlantID));
			if (aPlant)
				aPlant->mCharmProbability = std::min(aPlant->mCharmProbability + 5, 100);
		}
	}

	if (mIsBouncingPea)
	{
		// 碰到僵尸：75% 反弹（正常造成伤害），25% 正常消失
		if (theZombie && Rand(100) < 75)
		{
			mVelX = -mVelX;
			mHitTorchwoodGridX = -1;  // 反向后可再次过火
			// 把豌豆弹到僵尸边缘，避免下一帧仍重叠而反复命中同一只
			Rect aZombieRect = theZombie->GetZombieRect();
			if (mVelX > 0.0f)
				mPosX = aZombieRect.mX + aZombieRect.mWidth + 4.0f;
			else
				mPosX = aZombieRect.mX - mWidth - 4.0f;
			return;
		}
		Die();
	}
	else
	{
		Die();
	}
}

void Projectile::Update()
{
	mProjectileAge++;
	if (mApp->IsPvPMode() && !mApp->IsPvPHost())
	{
		// Projectile motion and collision resolution are host-authoritative.
		// A replica remains at its latest snapshot position until the next
		// host update or snapshot-end removes it.
		AttachmentUpdateAndMove(mAttachmentID, mPosX, mPosY + mPosZ);
		return;
	}
	if (mApp->mGameScene != GameScenes::SCENE_PLAYING && !mBoard->mCutScene->ShouldRunUpsellBoard())
		return;

	int aTime = 20;
	if (mProjectileType == ProjectileType::PROJECTILE_PEA || 
		mProjectileType == ProjectileType::PROJECTILE_IRONPEA || 
		mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA || 
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || 
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING || 
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || 
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || 
		mProjectileType == ProjectileType::PROJECTILE_CABBAGE || 
		mProjectileType == ProjectileType::PROJECTILE_MELON || 
		mProjectileType == ProjectileType::PROJECTILE_WINTERMELON || 
		mProjectileType == ProjectileType::PROJECTILE_KERNEL || 
		mProjectileType == ProjectileType::PROJECTILE_BUTTER || 
		mProjectileType == ProjectileType::PROJECTILE_COBBIG || 
		mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA || 
		mProjectileType == ProjectileType::PROJECTILE_SPIKE)
	{
		aTime = 0;
	}
	if (mProjectileAge > aTime)
	{
		mRenderOrder = Board::MakeRenderOrder(RenderLayer::RENDER_LAYER_PROJECTILE, mRow, 0);
	}

	if (mClickBackoffCounter > 0)
	{
		mClickBackoffCounter--;
	}
	mRotation += mRotationSpeed;

	UpdateMotion();
	AttachmentUpdateAndMove(mAttachmentID, mPosX, mPosY + mPosZ);
}

void Projectile::Draw(Graphics* g)
{
	const ProjectileDefinition& aProjectileDef = GetProjectileDef();

	Image* aImage = nullptr;
	float aScale = 1.0f;
	switch (mProjectileType)
	{
	case ProjectileType::PROJECTILE_COBBIG:
		aImage = IMAGE_REANIM_COBCANNON_COB;
		aScale = 0.9f;
		break;
	case ProjectileType::PROJECTILE_HYPNOSHROOM_PEA:
		aImage = FilterEffectGetImage(IMAGE_PROJECTILEPEA, FilterEffect::FILTER_EFFECT_HUE_SHIFT_240);
		break;
	case ProjectileType::PROJECTILE_PEA:
	case ProjectileType::PROJECTILE_ZOMBIE_PEA:
		aImage = IMAGE_PROJECTILEPEA;
		break;
	case ProjectileType::PROJECTILE_IRONPEA:
		aImage = FilterEffectGetImage(IMAGE_PROJECTILEPEA, mIronMagnetized ? FilterEffect::FILTER_EFFECT_HUE_SHIFT_240 : FilterEffect::FILTER_EFFECT_WASHED_OUT);
		break;
	case ProjectileType::PROJECTILE_SNOWPEA:
	case ProjectileType::PROJECTILE_SNOWPEA_PIERCING:
		aImage = IMAGE_PROJECTILESNOWPEA;
		break;
	case ProjectileType::PROJECTILE_FIREBALL:
	case ProjectileType::PROJECTILE_SNOWPEA_FIRE:
	case ProjectileType::PROJECTILE_SNOWPEA_FIRE2:
	case ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE:
	case ProjectileType::PROJECTILE_FIRE_SUPER:
		aImage = nullptr;
		break;
	case ProjectileType::PROJECTILE_SPIKE:
		aImage = IMAGE_PROJECTILECACTUS;
		break;
	case ProjectileType::PROJECTILE_STAR:
		aImage = IMAGE_PROJECTILE_STAR;
		break;
	case ProjectileType::PROJECTILE_PUFF:
		aImage = IMAGE_PUFFSHROOM_PUFF1;
		aScale = TodAnimateCurveFloat(0, 30, mProjectileAge, 0.3f, 1.0f, TodCurves::CURVE_LINEAR);
		break;
	case ProjectileType::PROJECTILE_BASKETBALL:
		aImage = IMAGE_REANIM_ZOMBIE_CATAPULT_BASKETBALL;
		aScale = 1.1f;
		break;
	case ProjectileType::PROJECTILE_CABBAGE:
		aImage = IMAGE_REANIM_CABBAGEPULT_CABBAGE;
		aScale = 1.0f;
		break;
	case ProjectileType::PROJECTILE_KERNEL:
		aImage = IMAGE_REANIM_CORNPULT_KERNAL;
		aScale = 0.95f;
		break;
	case ProjectileType::PROJECTILE_BUTTER:
		aImage = IMAGE_REANIM_CORNPULT_BUTTER;
		aScale = 0.8f;
		break;
	case ProjectileType::PROJECTILE_MELON:
		aImage = IMAGE_REANIM_MELONPULT_MELON;
		aScale = mIsBigMelon ? 2.0f : 1.0f;  // 大西瓜体型翻倍
		break;
	case ProjectileType::PROJECTILE_WINTERMELON:
		aImage = IMAGE_REANIM_WINTERMELON_PROJECTILE;
		aScale = 1.0f;
		break;
	default:
		TOD_ASSERT(false);
		break;
	}

	bool aMirror = false;
	if (mMotionType == ProjectileMotion::MOTION_BEE_BACKWARDS)
	{
		aMirror = true;
	}

	if (aImage)
	{
		TOD_ASSERT(aProjectileDef.mImageRow < aImage->mNumRows);
		TOD_ASSERT(mFrame < aImage->mNumCols);

		int aCelWidth = aImage->GetCelWidth();
		int aCelHeight = aImage->GetCelHeight();
		Rect aSrcRect(aCelWidth * mFrame, aCelHeight * aProjectileDef.mImageRow, aCelWidth, aCelHeight);
		if (FloatApproxEqual(mRotation, 0.0f) && FloatApproxEqual(aScale, 1.0f))
		{
			Rect aDestRect(0, 0, aCelWidth, aCelHeight);
			g->DrawImageMirror(aImage, aDestRect, aSrcRect, aMirror);

			if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING && !mDead)
			{
				g->SetColorizeImages(true);
				g->SetColor(Color(0, 0, 0, 110));
				g->DrawImageMirror(aImage, aDestRect, aSrcRect, aMirror);
				g->SetColorizeImages(false);
				g->SetColor(Color::White);
			}
		}
		else
		{
			float aOffsetX = mPosX + aCelWidth * 0.5f;
			float aOffsetY = mPosZ + mPosY + aCelHeight * 0.5f;
			SexyTransform2D aTransform;
			TodScaleRotateTransformMatrix(aTransform, aOffsetX + mBoard->mX, aOffsetY + mBoard->mY, mRotation, aScale, aScale);
			TodBltMatrix(g, aImage, aTransform, g->mClipRect, Color::White, g->mDrawMode, aSrcRect);

			if (mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING && !mDead)
			{
				TodBltMatrix(g, aImage, aTransform, g->mClipRect, Color(0, 0, 0, 110), g->mDrawMode, aSrcRect);
			}
		}
	}

	if (mAttachmentID != AttachmentID::ATTACHMENTID_NULL)
	{
		Graphics theParticleGraphics(*g);
		MakeParentGraphicsFrame(&theParticleGraphics);
		AttachmentDraw(mAttachmentID, &theParticleGraphics, false);
	}
}

void Projectile::DrawShadow(Graphics* g)
{
	int aCelCol = 0;
	float aScale = 1.0f;
	float aStretch = 1.0f;
	float aOffsetX = mPosX - mX;
	float aOffsetY = mPosY - mY;

	int aGridX = mBoard->PixelToGridXKeepOnBoard(mX, mY);
	bool isHighGround = false;
	if (mBoard->mGridSquareType[aGridX][mRow] == GridSquareType::GRIDSQUARE_HIGH_GROUND)
	{
		isHighGround = true;
	}
	if (mOnHighGround && !isHighGround)
	{
		aOffsetY += HIGH_GROUND_HEIGHT;
	}
	else if (!mOnHighGround && isHighGround)
	{
		aOffsetY -= HIGH_GROUND_HEIGHT;
	}

	if (mBoard->StageIsNight())
	{
		aCelCol = 1;
	}

	switch (mProjectileType)
	{
	case ProjectileType::PROJECTILE_HYPNOSHROOM_PEA:
	case ProjectileType::PROJECTILE_PEA:
	case ProjectileType::PROJECTILE_IRONPEA:
	case ProjectileType::PROJECTILE_ZOMBIE_PEA:
		aOffsetX += 3.0f;
		break;

	case ProjectileType::PROJECTILE_SNOWPEA:
	case ProjectileType::PROJECTILE_SNOWPEA_PIERCING:
		aOffsetX += -1.0f;
		aScale = 1.3f;
		break;

	case ProjectileType::PROJECTILE_STAR:
		aOffsetX += 7.0f;
		break;

	case ProjectileType::PROJECTILE_CABBAGE:
	case ProjectileType::PROJECTILE_KERNEL:
	case ProjectileType::PROJECTILE_BUTTER:
	case ProjectileType::PROJECTILE_MELON:
	case ProjectileType::PROJECTILE_WINTERMELON:
		aOffsetX += 3.0f;
		aOffsetY += 10.0f;
		aScale = 1.6f;
		break;

	case ProjectileType::PROJECTILE_PUFF:
		return;
		
	case ProjectileType::PROJECTILE_COBBIG:
		aScale = 1.0f;
		aStretch = 3.0f;
		aOffsetX += 57.0f;
		break;

	case ProjectileType::PROJECTILE_FIREBALL:
	case ProjectileType::PROJECTILE_SNOWPEA_FIRE:
	case ProjectileType::PROJECTILE_SNOWPEA_FIRE2:
	case ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE:
	case ProjectileType::PROJECTILE_FIRE_SUPER:
		aScale = 1.4f;
		break;
	default:
		break;
	}

	if (mMotionType == ProjectileMotion::MOTION_LOBBED)
	{
		float aHeight = ClampFloat(-mPosZ, 0.0f, 200.0f);
		aScale *= 200.0f / (aHeight + 200.0f);
	}

	TodDrawImageCelScaledF(g, IMAGE_PEA_SHADOWS, aOffsetX, (mShadowY - mPosY + aOffsetY), aCelCol, 0, aScale * aStretch, aScale);
}

void Projectile::Die()
{
	mDead = true;

	if (mProjectileType == ProjectileType::PROJECTILE_PUFF || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING)
	{
		AttachmentCrossFade(mAttachmentID, "FadeOut");
		AttachmentDetach(mAttachmentID);
	}
	else
	{
		AttachmentDie(mAttachmentID);
	}
}

Rect Projectile::GetProjectileRect()
{
	if (mProjectileType == ProjectileType::PROJECTILE_PEA || 
		mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_PEA || 
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA ||
		mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_PIERCING ||
		mProjectileType == ProjectileType::PROJECTILE_ZOMBIE_PEA)
	{
		return Rect(mX - 15, mY, mWidth + 15, mHeight);
	}
	else if (mProjectileType == ProjectileType::PROJECTILE_COBBIG)
	{
		return Rect(mX + mWidth / 2 - 115, mY + mHeight / 2 - 115, 230, 230);
	}
	else if (mProjectileType == ProjectileType::PROJECTILE_MELON || mProjectileType == ProjectileType::PROJECTILE_WINTERMELON)
	{
		return Rect(mX + 20, mY, 60, mHeight);
	}
	else if (mProjectileType == ProjectileType::PROJECTILE_FIREBALL || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE || mProjectileType == ProjectileType::PROJECTILE_SNOWPEA_FIRE2 || mProjectileType == ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE || mProjectileType == ProjectileType::PROJECTILE_FIRE_SUPER)
	{
		return Rect(mX, mY, mWidth - 10, mHeight);
	}
	else if (mProjectileType == ProjectileType::PROJECTILE_SPIKE)
	{
		return Rect(mX - 25, mY, mWidth + 25, mHeight);
	}
	else
	{
		return Rect(mX, mY, mWidth, mHeight);
	}
}

void Projectile::ConvertToFireball(int theGridX)
{
	if (mHitTorchwoodGridX == theGridX)
		return;

	mProjectileType = ProjectileType::PROJECTILE_FIREBALL;
	mHitTorchwoodGridX = theGridX;
	mApp->PlayFoley(FoleyType::FOLEY_FIREPEA);

	float aOffsetX = -25.0f;
	float aOffsetY = -25.0f;
	Reanimation* aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_FIRE_PEA);
	if (mMotionType == ProjectileMotion::MOTION_BACKWARDS)
	{
		aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
		aOffsetX += 80.0f;
	}

	aFirePeaReanim->SetPosition(mPosX + aOffsetX, mPosY + aOffsetY);
	aFirePeaReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
	aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
	AttachReanim(mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);
}

void Projectile::ConvertToFireHypno(int theGridX)
{
	if (mHitTorchwoodGridX == theGridX)
		return;

	mProjectileType = ProjectileType::PROJECTILE_HYPNOSHROOM_FIRE;
	mHitTorchwoodGridX = theGridX;
	mApp->PlayFoley(FoleyType::FOLEY_FIREPEA);

	AttachmentDie(mAttachmentID);
	float aOffsetX = -25.0f;
	float aOffsetY = -25.0f;
	Reanimation* aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_FIRE_PEA);
	if (mMotionType == ProjectileMotion::MOTION_BACKWARDS)
	{
		aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
		aOffsetX += 80.0f;
	}

	aFirePeaReanim->SetPosition(mPosX + aOffsetX, mPosY + aOffsetY);
	aFirePeaReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
	aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
	aFirePeaReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT_240;
	AttachReanim(mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);
}

void Projectile::ConvertToSuperFire(int theGridX)
{
	if (mHitTorchwoodGridX == theGridX)
		return;

	mProjectileType = ProjectileType::PROJECTILE_FIRE_SUPER;
	mHitTorchwoodGridX = theGridX;
	mApp->PlayFoley(FoleyType::FOLEY_FIREPEA);

	AttachmentDie(mAttachmentID);
	float aOffsetX = -25.0f;
	float aOffsetY = -25.0f;
	Reanimation* aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_FIRE_PEA);
	if (mMotionType == ProjectileMotion::MOTION_BACKWARDS)
	{
		aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
		aOffsetX += 80.0f;
	}

	aFirePeaReanim->SetPosition(mPosX + aOffsetX, mPosY + aOffsetY);
	aFirePeaReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
	aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
	aFirePeaReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT_281;
	AttachReanim(mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);
}

void Projectile::ConvertToPea(int theGridX)
{
	if (mHitTorchwoodGridX == theGridX)
		return;

	AttachmentDie(mAttachmentID);
	mProjectileType = ProjectileType::PROJECTILE_PEA;
	mHitTorchwoodGridX = theGridX;
	mApp->PlayFoley(FoleyType::FOLEY_THROW);
}

void Projectile::ConvertToIceFire(int theGridX)
{
	if (mHitTorchwoodGridX == theGridX)
		return;

	mProjectileType = ProjectileType::PROJECTILE_SNOWPEA_FIRE;
	mHitTorchwoodGridX = theGridX;
	mApp->PlayFoley(FoleyType::FOLEY_FIREPEA);

	AttachmentDie(mAttachmentID);
	float aOffsetX = -25.0f;
	float aOffsetY = -25.0f;
	Reanimation* aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_FIRE_PEA);
	if (mMotionType == ProjectileMotion::MOTION_BACKWARDS)
	{
		aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
		aOffsetX += 80.0f;
	}

	aFirePeaReanim->SetPosition(mPosX + aOffsetX, mPosY + aOffsetY);
	aFirePeaReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
	aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
	aFirePeaReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT;
	AttachReanim(mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);
}

void Projectile::ConvertToIceFire2(int theGridX)
{
	if (mHitTorchwoodGridX == theGridX)
		return;

	mProjectileType = ProjectileType::PROJECTILE_SNOWPEA_FIRE2;
	mHitTorchwoodGridX = theGridX;
	mApp->PlayFoley(FoleyType::FOLEY_FIREPEA);

	AttachmentDie(mAttachmentID);
	float aOffsetX = -25.0f;
	float aOffsetY = -25.0f;
	Reanimation* aFirePeaReanim = mApp->AddReanimation(0.0f, 0.0f, 0, ReanimationType::REANIM_FIRE_PEA);
	if (mMotionType == ProjectileMotion::MOTION_BACKWARDS)
	{
		aFirePeaReanim->OverrideScale(-1.0f, 1.0f);
		aOffsetX += 80.0f;
	}

	aFirePeaReanim->SetPosition(mPosX + aOffsetX, mPosY + aOffsetY);
	aFirePeaReanim->mLoopType = ReanimLoopType::REANIM_LOOP;
	aFirePeaReanim->mAnimRate = RandRangeFloat(50.0f, 80.0f);
	aFirePeaReanim->mFilterEffect = FilterEffect::FILTER_EFFECT_HUE_SHIFT_194;
	AttachReanim(mAttachmentID, aFirePeaReanim, aOffsetX, aOffsetY);
}

const ProjectileDefinition& Projectile::GetProjectileDef()
{
	const ProjectileDefinition& aProjectileDef = gProjectileDefinition[mProjectileType];
	TOD_ASSERT(aProjectileDef.mProjectileType == mProjectileType);

	return aProjectileDef;
}

int Projectile::GetDamage() const
{
	if (mProjectileType != ProjectileType::PROJECTILE_IRONPEA)
		return gProjectileDefinition[mProjectileType].mDamage * mErrorDamageMultiplier;
	if (mIronMagnetized)
		return 0;
	int aMagnetStack = 1;
	if (mIronMagnetPlantID != PlantID::PLANTID_NULL)
	{
		Plant* aMagnet = mBoard->mPlants.DataArrayTryToGet(static_cast<unsigned int>(mIronMagnetPlantID));
		if (aMagnet)
		{
			Plant* aNearbyMagnet = nullptr;
			while (mBoard->IteratePlants(aNearbyMagnet))
			{
				if (aNearbyMagnet->mSeedType == SeedType::SEED_MAGNETSHROOM &&
					aNearbyMagnet->mPlantHealth > 0 && aNearbyMagnet->mRow == aMagnet->mRow)
					aMagnetStack++;
			}
			// The attached magnet itself is included in the loop above.
			aMagnetStack = std::max(1, aMagnetStack - 1);
		}
	}
	const float aSpeedCap = 33.3f * static_cast<float>(aMagnetStack);
	const float aSpeedRatio = ClampFloat((mIronSpeed - 20.0f) / std::max(1.0f, aSpeedCap - 20.0f), 0.0f, 1.0f);
	const int64_t aMaxDamage64 = std::min<int64_t>(500LL << std::min(aMagnetStack - 1, 30), 2000000000LL);
	const int aMaxDamage = static_cast<int>(aMaxDamage64);
	int aDamage = FloatRoundToInt(20.0f + aSpeedRatio * static_cast<float>(aMaxDamage - 20));
	if (mIronHeated)
	{
		aDamage *= 2;
		if (mIronSpeed > 20.0f)
			aDamage = aDamage * 125 / 100;
	}
	return static_cast<int>(std::min<int64_t>(static_cast<int64_t>(std::max(20, aDamage)) * mErrorDamageMultiplier, 2147483647));
}
