#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace PGP {

enum ProfessionStat {
  STAT_NONE = 0,
  STAT_LABOURING,
  STAT_SCIENCE,
  STAT_ENGINEERING,
  STAT_ROBOTICS,
  STAT_WEAPON_SMITH,
  STAT_ARMOUR_SMITH,
  STAT_CROSSBOW_SMITH,
  STAT_MEDIC,
  STAT_TURRETS,
  STAT_FARMING,
  STAT_COOKING,
  STAT_ATHLETICS,
  STAT_SWIMMING,
  STAT_PERCEPTION,
  STAT_STEALTH,
  STAT_ASSASSINATION,
  STAT_LOCKPICKING,
  STAT_THIEVERY
};

enum ItemTag {
  TAG_NONE = 0,
  TAG_TOOL_FARMING,
  TAG_TOOL_MINING,
  TAG_TOOL_RESEARCH,
  TAG_TOOL_ENGINEERING,
  TAG_TOOL_ROBOTICS,
  TAG_TOOL_MEDIC,
  TAG_TOOL_WEAPON_SMITH,
  TAG_TOOL_ARMOUR_SMITH,
  TAG_TOOL_CROSSBOW_SMITH,
  TAG_TOOL_COOKING,
  TAG_HEAD_FARMING,
  TAG_HEAD_MINING,
  TAG_HEAD_RESEARCH,
  TAG_BODY_RESEARCH,
  TAG_BODY_ENGINEERING,
  TAG_BODY_MEDIC,
  TAG_BODY_SMITH,
  TAG_GLOVES_WORK,
  TAG_BOOTS_WORK,
  TAG_BOOTS_TRAVEL,
  TAG_PACK_ORE,
  TAG_PACK_CROP,
  TAG_PACK_CONSTRUCTION,
  TAG_PACK_MEDICAL,
  TAG_PACK_TRADE,
  TAG_PACK_TECH,
  TAG_TURRET_GEAR,
  TAG_SCOUT_GEAR,
  TAG_STEALTH_GEAR,
  TAG_ASSASSIN_GEAR,
  TAG_THIEF_GEAR,
  TAG_SWIM_GEAR,
  TAG_PACK_HAULING,
  TAG_PACK_GENERIC,
  TAG_WORKWEAR_GENERIC,
  TAG_GOGGLES_GENERIC
};

struct Affix {
  ProfessionStat stat;
  float percent;
  Affix() : stat(STAT_NONE), percent(0.0f) {}
  Affix(ProfessionStat s, float p) : stat(s), percent(p) {}
};

struct ItemDescriptor {
  std::string baseId;
  std::string name;
  std::string slot;
  std::string category;
  std::string description;
  float quality;
  int weaponLevel;
  bool equipped;
  bool stackable;
  bool equippable;
  bool container;
  bool weapon;
  bool armour;
  bool robotLimb;
  bool legendary;
  ItemDescriptor() : quality(0.0f), weaponLevel(-1), equipped(false), stackable(false), equippable(true), container(false), weapon(false), armour(false), robotLimb(false), legendary(false) {}
};

struct RoleProfile {
  ProfessionStat primary;
  float wealth01;
  bool slave;
  bool unique;
  bool traderSource;
  bool worldLootSource;
  RoleProfile() : primary(STAT_NONE), wealth01(0.5f), slave(false), unique(false), traderSource(false), worldLootSource(false) {}
};

struct AffixRecord {
  std::string instanceKey;
  std::string baseId;
  int tier;
  std::vector<Affix> affixes;
  AffixRecord() : tier(0) {}
};

struct RuleConfig {
  bool enabled;
  bool autoClassify;
  bool verboseLogging;
  float globalChance;
  float npcRoleMultiplier;
  float playerCraftMultiplier;
  float poorNpcMultiplier;
  float worldLootMultiplier;
  int maxAffixes;
  RuleConfig();
};

void NormalizeConfig(RuleConfig& config);

std::string Lower(const std::string& value);
std::string Trim(const std::string& value);
std::string StatName(ProfessionStat stat);
ProfessionStat ParseStat(const std::string& value);
std::string TagName(ItemTag tag);
ItemTag ParseTag(const std::string& value);

int QualityTier(float quality);
int WeaponGradeRank(int level0to100);
int ProgressionTier(const ItemDescriptor& item);
void TierRange(int tier, float& minPercent, float& maxPercent);
float TierAffixChance(int tier);
int TierAffixCap(int tier);

std::vector<ItemTag> Classify(const ItemDescriptor& item,
                              const std::map<std::string, std::vector<ItemTag> >& overrides,
                              const std::set<std::string>& exclusions);

std::vector<ProfessionStat> AllowedStats(const std::vector<ItemTag>& tags);
bool IsProfessionStat(ProfessionStat stat);

unsigned int Hash32(const std::string& text);
float UnitRoll(unsigned int& state);

AffixRecord RollAffixes(const ItemDescriptor& item,
                        const RoleProfile& role,
                        const RuleConfig& config,
                        const std::vector<ItemTag>& tags,
                        const std::string& instanceKey,
                        unsigned int seed,
                        bool playerCrafted);

float AggregatePercent(const std::vector<AffixRecord>& records,
                       ProfessionStat stat);

float EffectiveStatValue(float baseValue, float totalPercent, bool unmodified,
                         float hardCap);

// 0..1 wealth/competence from the NPC's best profession skill; < 0.15 (best skill < 12) = poor.
float WealthFromBestSkill(float bestSkill);

float SpecialistPackItemWeightMultiplier(const std::vector<ItemTag>& packTags,
                                         const std::string& itemName,
                                         const std::string& itemBaseId,
                                         bool isTradeItem);

std::string SerializeRecord(const AffixRecord& record);
bool ParseRecord(const std::string& line, AffixRecord& out);

}  // namespace PGP
