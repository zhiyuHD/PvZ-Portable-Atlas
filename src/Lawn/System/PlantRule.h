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

#ifndef __PLANT_RULE_H__
#define __PLANT_RULE_H__

#include <cstdint>
#include "../../ConstEnums.h"

// 「我真是搞不明白......」：每株种下的植物独立抽取一条规则。
enum PlantRuleId : int32_t
{
    PLANT_RULE_NONE = -1,

    // 通用池
    PLANT_RULE_IRON_BODY,
    PLANT_RULE_REGEN,
    PLANT_RULE_REFLECT,
    PLANT_RULE_LEGACY,
    PLANT_RULE_EMBER,
    PLANT_RULE_DODGE,
    PLANT_RULE_FRAIL,
    PLANT_RULE_BRITTLE,
    PLANT_RULE_SHORT_LIFE,
    PLANT_RULE_TAUNT,
    PLANT_RULE_BAD_LUCK,
    PLANT_RULE_STIFF,

    // 射手池
    PLANT_RULE_DOUBLE_SHOT,
    PLANT_RULE_HEAVY_SHOT,
    PLANT_RULE_FROST_ENCHANT,
    PLANT_RULE_PIERCE,
    PLANT_RULE_SPLASH,
    PLANT_RULE_MISFIRE,
    PLANT_RULE_SHAKY_AIM,
    PLANT_RULE_SOFT_SHOT,
    PLANT_RULE_JAM,
    PLANT_RULE_SELF_HIT,

    // 生产池
    PLANT_RULE_DOUBLE_YIELD,
    PLANT_RULE_HIGH_YIELD,
    PLANT_RULE_FAST_YIELD,
    PLANT_RULE_GOLD_FINGER,
    PLANT_RULE_EMPTY_HAND,
    PLANT_RULE_LOW_YIELD,
    PLANT_RULE_SLOW_YIELD,
    PLANT_RULE_GREEDY,

    // 防御池
    PLANT_RULE_THICK_ARMOR,
    PLANT_RULE_THORNS,
    PLANT_RULE_ICE_ARMOR,
    PLANT_RULE_SPIKES,
    PLANT_RULE_THIN_SHELL,
    PLANT_RULE_CRACK,
    PLANT_RULE_LOOSE,
    PLANT_RULE_SHATTER,

    // 瞬时池
    PLANT_RULE_BIG_BLAST,
    PLANT_RULE_FAST_FUSE,
    PLANT_RULE_AFTERMATH,
    PLANT_RULE_REFUND,
    PLANT_RULE_DUD,
    PLANT_RULE_SLOW_FUSE,
    PLANT_RULE_FRIENDLY_FIRE,
    PLANT_RULE_EXPENSIVE,

    NUM_PLANT_RULES
};

enum PlantRulePolarity : uint8_t
{
    PLANT_RULE_GOOD = 0,
    PLANT_RULE_BAD = 1
};

enum PlantRulePool : uint8_t
{
    PLANT_RULE_POOL_UNIVERSAL = 0,   // 所有非瞬时植物共用
    PLANT_RULE_POOL_SHOOTER,
    PLANT_RULE_POOL_PRODUCER,
    PLANT_RULE_POOL_DEFENDER,
    PLANT_RULE_POOL_INSTANT,
    PLANT_RULE_POOL_GENERIC    // 功能型植物：只抽通用池
};

struct PlantRuleDef
{
    PlantRuleId            mId;
    const char*            mName;       // 卡片上显示的短名（2 字）
    const char*            mDesc;       // 完整说明
    PlantRulePolarity      mPolarity;
    PlantRulePool          mPool;
    int32_t                mParam;      // 百分比/帧数等通用参数
};

extern const PlantRuleDef gPlantRuleDefs[NUM_PLANT_RULES];

const PlantRuleDef& GetPlantRuleDef(PlantRuleId theRule);
PlantRulePool       GetPlantRulePool(SeedType theSeedType);
PlantRuleId         RollPlantRule(SeedType theSeedType);
bool                PlantRuleIsGood(PlantRuleId theRule);

#endif
