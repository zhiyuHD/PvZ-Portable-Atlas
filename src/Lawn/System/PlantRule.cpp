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

#include "PlantRule.h"
#include "../../SexyAppFramework/Common.h"
#include "../../Sexy.TodLib/TodDebug.h"

using namespace Sexy;

constinit const PlantRuleDef gPlantRuleDefs[NUM_PLANT_RULES] = {
    // ==================== 通用池（12） ====================
    { .mId = PLANT_RULE_IRON_BODY,      .mName = "铁骨",   .mDesc = "最大生命 +40%",                     .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 40 },
    { .mId = PLANT_RULE_REGEN,          .mName = "再生",   .mDesc = "每 5 秒回复 2% 最大生命",            .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 2 },
    { .mId = PLANT_RULE_REFLECT,        .mName = "反甲",   .mDesc = "被啃时反弹 20% 伤害",                .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 20 },
    { .mId = PLANT_RULE_LEGACY,         .mName = "遗产",   .mDesc = "死亡时掉 50 阳光",                  .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 50 },
    { .mId = PLANT_RULE_EMBER,          .mName = "余烬",   .mDesc = "死亡时小范围爆炸（50 伤害）",         .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 50 },
    { .mId = PLANT_RULE_DODGE,          .mName = "闪避",   .mDesc = "10% 概率免疫一次啃咬",              .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 10 },
    { .mId = PLANT_RULE_FRAIL,          .mName = "脆皮",   .mDesc = "最大生命 -30%",                     .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 30 },
    { .mId = PLANT_RULE_BRITTLE,        .mName = "易碎",   .mDesc = "受到伤害 +15%",                     .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 15 },
    { .mId = PLANT_RULE_SHORT_LIFE,     .mName = "短命",   .mDesc = "每 10 秒损失 2% 最大生命",            .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 2 },
    { .mId = PLANT_RULE_TAUNT,          .mName = "招啃",   .mDesc = "被啃时啃食速度 +20%",               .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 20 },
    { .mId = PLANT_RULE_BAD_LUCK,       .mName = "霉运",   .mDesc = "种下时损失 25 阳光",                .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 25 },
    { .mId = PLANT_RULE_STIFF,          .mName = "僵直",   .mDesc = "种下后 3 秒不能行动",                .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_UNIVERSAL, .mParam = 300 },

    // ==================== 射手池（10） ====================
    { .mId = PLANT_RULE_DOUBLE_SHOT,    .mName = "连珠",   .mDesc = "10% 概率双发",                      .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 10 },
    { .mId = PLANT_RULE_HEAVY_SHOT,     .mName = "重弹",   .mDesc = "伤害 +40%",                         .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 40 },
    { .mId = PLANT_RULE_FROST_ENCHANT,  .mName = "冰附魔", .mDesc = "命中 10% 概率冰冻 1 秒",            .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 10 },
    { .mId = PLANT_RULE_PIERCE,         .mName = "穿透",   .mDesc = "弹丸穿透 1 个目标",                 .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 1 },
    { .mId = PLANT_RULE_SPLASH,         .mName = "溅射",   .mDesc = "命中产生小范围溅射",                .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 1 },
    { .mId = PLANT_RULE_MISFIRE,        .mName = "哑火",   .mDesc = "10% 概率不发射",                    .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 10 },
    { .mId = PLANT_RULE_SHAKY_AIM,      .mName = "手抖",   .mDesc = "10% 概率打偏到相邻行",              .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 10 },
    { .mId = PLANT_RULE_SOFT_SHOT,      .mName = "软弹",   .mDesc = "伤害 -25%",                         .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 25 },
    { .mId = PLANT_RULE_JAM,            .mName = "卡壳",   .mDesc = "每 10 发停火 1 秒",                 .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 100 },
    { .mId = PLANT_RULE_SELF_HIT,       .mName = "反伤",   .mDesc = "5% 概率打中自己，扣 20 生命",        .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_SHOOTER, .mParam = 20 },

    // ==================== 生产池（8） ====================
    { .mId = PLANT_RULE_DOUBLE_YIELD,   .mName = "双倍",   .mDesc = "20% 概率双倍产出",                  .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 20 },
    { .mId = PLANT_RULE_HIGH_YIELD,     .mName = "高产",   .mDesc = "产出 +25%",                         .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 25 },
    { .mId = PLANT_RULE_FAST_YIELD,     .mName = "快产",   .mDesc = "生产间隔 -20%",                     .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 20 },
    { .mId = PLANT_RULE_GOLD_FINGER,    .mName = "金手指", .mDesc = "10% 概率额外产金币",                .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 10 },
    { .mId = PLANT_RULE_EMPTY_HAND,     .mName = "空手",   .mDesc = "20% 概率产出为空",                  .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 20 },
    { .mId = PLANT_RULE_LOW_YIELD,      .mName = "减产",   .mDesc = "25% 概率只产小阳光",                .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 25 },
    { .mId = PLANT_RULE_SLOW_YIELD,     .mName = "慢产",   .mDesc = "生产间隔 +25%",                     .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 25 },
    { .mId = PLANT_RULE_GREEDY,         .mName = "贪吃",   .mDesc = "每 5 次产出有 1 次自己吃掉，扣 10 生命", .mPolarity = PLANT_RULE_BAD, .mPool = PLANT_RULE_POOL_PRODUCER, .mParam = 5 },

    // ==================== 防御池（8） ====================
    { .mId = PLANT_RULE_THICK_ARMOR,    .mName = "厚甲",   .mDesc = "最大生命 +50%",                     .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 50 },
    { .mId = PLANT_RULE_THORNS,         .mName = "荆棘",   .mDesc = "被啃时反弹 25% 伤害",                .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 25 },
    { .mId = PLANT_RULE_ICE_ARMOR,      .mName = "冰甲",   .mDesc = "被啃时 10% 概率冻住僵尸 1 秒",       .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 10 },
    { .mId = PLANT_RULE_SPIKES,         .mName = "尖刺",   .mDesc = "被啃时对啃食者造成 5 伤害",          .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 5 },
    { .mId = PLANT_RULE_THIN_SHELL,     .mName = "薄壳",   .mDesc = "最大生命 -30%",                     .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 30 },
    { .mId = PLANT_RULE_CRACK,          .mName = "裂缝",   .mDesc = "每被啃 10 次，最大生命 -5%",         .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 10 },
    { .mId = PLANT_RULE_LOOSE,          .mName = "松动",   .mDesc = "被啃时 15% 概率额外损失 10% 最大生命", .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 15 },
    { .mId = PLANT_RULE_SHATTER,        .mName = "碎裂",   .mDesc = "死亡时对周围植物造成 50 伤害",       .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_DEFENDER, .mParam = 50 },

    // ==================== 瞬时池（8） ====================
    { .mId = PLANT_RULE_BIG_BLAST,      .mName = "大爆炸", .mDesc = "触发时额外对周围僵尸造成 100 伤害",  .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 100 },
    { .mId = PLANT_RULE_FAST_FUSE,      .mName = "快引信", .mDesc = "触发延迟 -30%",                     .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 30 },
    { .mId = PLANT_RULE_AFTERMATH,      .mName = "余波",   .mDesc = "触发时追加 50 范围伤害",             .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 50 },
    { .mId = PLANT_RULE_REFUND,         .mName = "回本",   .mDesc = "种下时返还 50 阳光",                .mPolarity = PLANT_RULE_GOOD, .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 50 },
    { .mId = PLANT_RULE_DUD,            .mName = "哑弹",   .mDesc = "15% 概率无效果",                    .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 15 },
    { .mId = PLANT_RULE_SLOW_FUSE,      .mName = "慢引信", .mDesc = "触发延迟 +50%",                     .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 50 },
    { .mId = PLANT_RULE_FRIENDLY_FIRE,  .mName = "误伤",   .mDesc = "触发时对周围植物造成 50 伤害",       .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 50 },
    { .mId = PLANT_RULE_EXPENSIVE,      .mName = "昂贵",   .mDesc = "种下时额外消耗 50 阳光",             .mPolarity = PLANT_RULE_BAD,  .mPool = PLANT_RULE_POOL_INSTANT, .mParam = 50 },
};

const PlantRuleDef& GetPlantRuleDef(PlantRuleId theRule)
{
    TOD_ASSERT(theRule >= 0 && theRule < NUM_PLANT_RULES);
    return gPlantRuleDefs[theRule];
}

bool PlantRuleIsGood(PlantRuleId theRule)
{
    return theRule >= 0 && theRule < NUM_PLANT_RULES &&
        gPlantRuleDefs[theRule].mPolarity == PLANT_RULE_GOOD;
}

PlantRulePool GetPlantRulePool(SeedType theSeedType)
{
    switch (theSeedType)
    {
    case SeedType::SEED_PEASHOOTER:
    case SeedType::SEED_SNOWPEA:
    case SeedType::SEED_REPEATER:
    case SeedType::SEED_PUFFSHROOM:
    case SeedType::SEED_FUMESHROOM:
    case SeedType::SEED_SCAREDYSHROOM:
    case SeedType::SEED_THREEPEATER:
    case SeedType::SEED_CACTUS:
    case SeedType::SEED_SPLITPEA:
    case SeedType::SEED_STARFRUIT:
    case SeedType::SEED_CABBAGEPULT:
    case SeedType::SEED_KERNELPULT:
    case SeedType::SEED_MELONPULT:
    case SeedType::SEED_GATLINGPEA:
    case SeedType::SEED_GLOOMSHROOM:
    case SeedType::SEED_CATTAIL:
    case SeedType::SEED_WINTERMELON:
    case SeedType::SEED_SEASHROOM:
    case SeedType::SEED_IRONPEASHOOTER:
    case SeedType::SEED_LEFTPEATER:
        return PLANT_RULE_POOL_SHOOTER;

    case SeedType::SEED_SUNFLOWER:
    case SeedType::SEED_SUNSHROOM:
    case SeedType::SEED_TWINSUNFLOWER:
    case SeedType::SEED_MARIGOLD:
        return PLANT_RULE_POOL_PRODUCER;

    case SeedType::SEED_WALLNUT:
    case SeedType::SEED_TALLNUT:
    case SeedType::SEED_PUMPKINSHELL:
    case SeedType::SEED_GARLIC:
    case SeedType::SEED_SPIKEWEED:
    case SeedType::SEED_SPIKEROCK:
    case SeedType::SEED_SHIELDWALLNUT:
        return PLANT_RULE_POOL_DEFENDER;

    case SeedType::SEED_CHERRYBOMB:
    case SeedType::SEED_POTATOMINE:
    case SeedType::SEED_SQUASH:
    case SeedType::SEED_JALAPENO:
    case SeedType::SEED_ICESHROOM:
    case SeedType::SEED_DOOMSHROOM:
    case SeedType::SEED_BLOVER:
    case SeedType::SEED_GRAVEBUSTER:
    case SeedType::SEED_INSTANT_COFFEE:
    case SeedType::SEED_TANGLEKELP:
        return PLANT_RULE_POOL_INSTANT;

    default:
        return PLANT_RULE_POOL_GENERIC;
    }
}

PlantRuleId RollPlantRule(SeedType theSeedType)
{
    const PlantRulePool aPlantPool = GetPlantRulePool(theSeedType);
    const PlantRulePolarity aPolarity = (Rand(2) == 0) ? PLANT_RULE_GOOD : PLANT_RULE_BAD;

    PlantRuleId aCandidates[NUM_PLANT_RULES];
    int aCandidateCount = 0;
    for (int i = 0; i < NUM_PLANT_RULES; i++)
    {
        const PlantRuleDef& aDef = gPlantRuleDefs[i];
        if (aDef.mPolarity != aPolarity)
            continue;

        bool aApplicable = false;
        if (aPlantPool == PLANT_RULE_POOL_INSTANT)
            aApplicable = (aDef.mPool == PLANT_RULE_POOL_INSTANT);
        else if (aPlantPool == PLANT_RULE_POOL_GENERIC)
            aApplicable = (aDef.mPool == PLANT_RULE_POOL_UNIVERSAL);
        else
            aApplicable = (aDef.mPool == PLANT_RULE_POOL_UNIVERSAL || aDef.mPool == aPlantPool);

        if (aApplicable)
            aCandidates[aCandidateCount++] = aDef.mId;
    }

    if (aCandidateCount <= 0)
        return PLANT_RULE_NONE;

    return aCandidates[Rand(aCandidateCount)];
}
