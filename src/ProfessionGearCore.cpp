
#include "ProfessionGearCore.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace PGP {

RuleConfig::RuleConfig()
    : enabled(true), autoClassify(true), verboseLogging(false), globalChance(1.0f),
      npcRoleMultiplier(1.35f), playerCraftMultiplier(1.15f),
      poorNpcMultiplier(0.20f), worldLootMultiplier(0.50f), maxAffixes(3) {}

void NormalizeConfig(RuleConfig& c) {
  if(c.globalChance<0) c.globalChance=0;
  if(c.npcRoleMultiplier<0) c.npcRoleMultiplier=0;
  if(c.playerCraftMultiplier<0) c.playerCraftMultiplier=0;
  if(c.poorNpcMultiplier<0) c.poorNpcMultiplier=0;
  if(c.worldLootMultiplier<0) c.worldLootMultiplier=0;
  if(c.maxAffixes<1) c.maxAffixes=1;
  if(c.maxAffixes>3) c.maxAffixes=3;
}

std::string Lower(const std::string& value) {
  std::string out = value;
  for (size_t i = 0; i < out.size(); ++i)
    out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
  return out;
}

std::string Trim(const std::string& value) {
  size_t a = value.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = value.find_last_not_of(" \t\r\n");
  return value.substr(a, b - a + 1);
}

static bool Has(const std::string& h, const char* n) {
  return h.find(n) != std::string::npos;
}

static bool IsWordChar(char c) {
  const unsigned char u=static_cast<unsigned char>(c);
  return std::isalnum(u)!=0 || c=='_';
}

static bool HasWord(const std::string& h, const char* word) {
  const std::string w(word);
  size_t pos=0;
  while((pos=h.find(w,pos))!=std::string::npos){
    const bool left=(pos==0)||!IsWordChar(h[pos-1]);
    const size_t end=pos+w.size();
    const bool right=(end>=h.size())||!IsWordChar(h[end]);
    if(left&&right) return true;
    ++pos;
  }
  return false;
}

std::string StatName(ProfessionStat stat) {
  switch (stat) {
    case STAT_LABOURING: return "Labouring";
    case STAT_SCIENCE: return "Science";
    case STAT_ENGINEERING: return "Engineering";
    case STAT_ROBOTICS: return "Robotics";
    case STAT_WEAPON_SMITH: return "Weapon Smithing";
    case STAT_ARMOUR_SMITH: return "Armour Smithing";
    case STAT_CROSSBOW_SMITH: return "Crossbow Smithing";
    case STAT_MEDIC: return "Medic";
    case STAT_TURRETS: return "Turrets";
    case STAT_FARMING: return "Farming";
    case STAT_COOKING: return "Cooking";
    case STAT_ATHLETICS: return "Athletics";
    case STAT_SWIMMING: return "Swimming";
    case STAT_PERCEPTION: return "Perception";
    case STAT_STEALTH: return "Stealth";
    case STAT_ASSASSINATION: return "Assassination";
    case STAT_LOCKPICKING: return "Lockpicking";
    case STAT_THIEVERY: return "Thievery";
    default: return "None";
  }
}

ProfessionStat ParseStat(const std::string& value) {
  const std::string v = Lower(Trim(value));
  if (v=="labouring" || v=="laboring") return STAT_LABOURING;
  if (v=="science") return STAT_SCIENCE;
  if (v=="engineering") return STAT_ENGINEERING;
  if (v=="robotics") return STAT_ROBOTICS;
  if (v=="weapon smithing" || v=="weapon_smithing") return STAT_WEAPON_SMITH;
  if (v=="armour smithing" || v=="armor smithing" || v=="armour_smithing") return STAT_ARMOUR_SMITH;
  if (v=="crossbow smithing" || v=="crossbow_smithing") return STAT_CROSSBOW_SMITH;
  if (v=="medic" || v=="medicine") return STAT_MEDIC;
  if (v=="turrets") return STAT_TURRETS;
  if (v=="farming") return STAT_FARMING;
  if (v=="cooking") return STAT_COOKING;
  if (v=="athletics") return STAT_ATHLETICS;
  if (v=="swimming") return STAT_SWIMMING;
  if (v=="perception") return STAT_PERCEPTION;
  if (v=="stealth") return STAT_STEALTH;
  if (v=="assassination") return STAT_ASSASSINATION;
  if (v=="lockpicking") return STAT_LOCKPICKING;
  if (v=="thievery") return STAT_THIEVERY;
  return STAT_NONE;
}

std::string TagName(ItemTag tag) {
  switch (tag) {
    case TAG_TOOL_FARMING: return "TOOL_FARMING";
    case TAG_TOOL_MINING: return "TOOL_MINING";
    case TAG_TOOL_RESEARCH: return "TOOL_RESEARCH";
    case TAG_TOOL_ENGINEERING: return "TOOL_ENGINEERING";
    case TAG_TOOL_ROBOTICS: return "TOOL_ROBOTICS";
    case TAG_TOOL_MEDIC: return "TOOL_MEDIC";
    case TAG_TOOL_WEAPON_SMITH: return "TOOL_WEAPON_SMITH";
    case TAG_TOOL_ARMOUR_SMITH: return "TOOL_ARMOUR_SMITH";
    case TAG_TOOL_CROSSBOW_SMITH: return "TOOL_CROSSBOW_SMITH";
    case TAG_TOOL_COOKING: return "TOOL_COOKING";
    case TAG_HEAD_FARMING: return "HEAD_FARMING";
    case TAG_HEAD_MINING: return "HEAD_MINING";
    case TAG_HEAD_RESEARCH: return "HEAD_RESEARCH";
    case TAG_BODY_RESEARCH: return "BODY_RESEARCH";
    case TAG_BODY_ENGINEERING: return "BODY_ENGINEERING";
    case TAG_BODY_MEDIC: return "BODY_MEDIC";
    case TAG_BODY_SMITH: return "BODY_SMITH";
    case TAG_GLOVES_WORK: return "GLOVES_WORK";
    case TAG_BOOTS_WORK: return "BOOTS_WORK";
    case TAG_BOOTS_TRAVEL: return "BOOTS_TRAVEL";
    case TAG_PACK_ORE: return "PACK_ORE";
    case TAG_PACK_CROP: return "PACK_CROP";
    case TAG_PACK_CONSTRUCTION: return "PACK_CONSTRUCTION";
    case TAG_PACK_MEDICAL: return "PACK_MEDICAL";
    case TAG_PACK_TRADE: return "PACK_TRADE";
    case TAG_PACK_TECH: return "PACK_TECH";
    case TAG_TURRET_GEAR: return "TURRET_GEAR";
    case TAG_SCOUT_GEAR: return "SCOUT_GEAR";
    case TAG_STEALTH_GEAR: return "STEALTH_GEAR";
    case TAG_ASSASSIN_GEAR: return "ASSASSIN_GEAR";
    case TAG_THIEF_GEAR: return "THIEF_GEAR";
    case TAG_SWIM_GEAR: return "SWIM_GEAR";
    case TAG_PACK_HAULING: return "PACK_HAULING";
    case TAG_PACK_GENERIC: return "PACK_GENERIC";
    case TAG_WORKWEAR_GENERIC: return "WORKWEAR_GENERIC";
    case TAG_GOGGLES_GENERIC: return "GOGGLES_GENERIC";
    default: return "NONE";
  }
}

ItemTag ParseTag(const std::string& value) {
  const std::string v = Lower(Trim(value));
  for (int i = TAG_TOOL_FARMING; i <= TAG_GOGGLES_GENERIC; ++i) {
    ItemTag t = static_cast<ItemTag>(i);
    if (Lower(TagName(t)) == v) return t;
  }
  return TAG_NONE;
}

int QualityTier(float q) {
  if (q < 0.15f) return 0;
  if (q < 0.30f) return 1;
  if (q < 0.48f) return 2;
  if (q < 0.66f) return 3;
  if (q < 0.82f) return 4;
  if (q < 0.94f) return 5;
  return 6;
}

int WeaponGradeRank(int level) {
  // Vanilla model levels: Rusted Junk 5 through Edge 3 80, Meitou 100.
  static const int levels[] = {5,10,15,20,25,30,35,40,50,55,60,70,75,80,100};
  if(level<=levels[0]) return 0;
  for(int i=1;i<15;++i) if(level<=levels[i]) return i;
  return 14;
}

int ProgressionTier(const ItemDescriptor& item) {
  if(!item.weapon || item.weaponLevel<0) return QualityTier(item.quality);
  const int rank=WeaponGradeRank(item.weaponLevel);
  if(rank<=1) return 0;      // Rusted / Rusting
  if(rank<=3) return 1;      // Mid-grade / Old Refitted
  if(rank<=5) return 2;      // Refitted / Catun 1
  if(rank<=7) return 3;      // Catun 2-3
  if(rank<=10) return 4;     // Mk I-III
  if(rank<=12) return 5;     // Edge 1-2
  return 6;                  // Edge 3; Meitou is blocked as legendary
}

void TierRange(int tier, float& minP, float& maxP) {
  static const float mins[] = {2,3,5,7,10,13,16};
  static const float maxs[] = {4,6,9,12,16,20,25};
  if (tier < 0) tier = 0;
  if (tier > 6) tier = 6;
  minP = mins[tier]; maxP = maxs[tier];
}

float TierAffixChance(int tier) {
  static const float c[] = {.20f,.30f,.45f,.60f,.75f,.88f,.96f};
  if (tier < 0) tier = 0;
  if (tier > 6) tier = 6;
  return c[tier];
}

int TierAffixCap(int tier) {
  if (tier < 0) tier = 0;
  if (tier > 6) tier = 6;
  if (tier <= 2) return 1;
  if (tier <= 4) return 2;
  return 3;
}

static void AddTag(std::vector<ItemTag>& out, ItemTag t) {
  if (t != TAG_NONE && std::find(out.begin(),out.end(),t)==out.end()) out.push_back(t);
}

std::vector<ItemTag> Classify(const ItemDescriptor& item,
                              const std::map<std::string,std::vector<ItemTag> >& overrides,
                              const std::set<std::string>& exclusions) {
  std::vector<ItemTag> out;
  const std::string id = Lower(item.baseId);
  if (exclusions.count(id)) return out;
  std::map<std::string,std::vector<ItemTag> >::const_iterator it = overrides.find(id);
  if (it != overrides.end()) return it->second;
  if (item.legendary) return out;
  // Weapons are classified from their name only. Weapon descriptions are flavour text about who
  // carries them ("popular with poor farmers", "to slay thieves"), which made an ordinary Staff a
  // Farming tool live. Armour/clothing/packs keep using the description.
  const std::string n = Lower(item.name+" "+(item.weapon?std::string():item.description)+" "+
                              item.category+" "+item.slot);

  const bool strongFarming =
      HasWord(n,"hoe") || HasWord(n,"sickle") || HasWord(n,"pitchfork") ||
      HasWord(n,"scythe") || HasWord(n,"scythes") ||
      HasWord(n,"farmer") || HasWord(n,"farmers") || HasWord(n,"farmhand") ||
      Has(n,"farm tool") || Has(n,"agricultural") || Has(n,"harvesting");
  const bool strongMining =
      Has(n,"pickaxe") || Has(n,"pick axe") || HasWord(n,"miner") ||
      HasWord(n,"miners") || HasWord(n,"mining");
  const bool strongResearch =
      Has(n,"research") || Has(n,"scientist") || Has(n,"science ") || Has(n,"laboratory") || Has(n,"lab tool");
  const bool strongEngineering =
      Has(n,"engineer") || Has(n,"engineering") || Has(n,"builder") || Has(n,"construction tool") ||
      Has(n,"mechanic ") || Has(n,"mechanic's") || Has(n,"mechanics ");
  const bool strongRobotics =
      Has(n,"robotic") || Has(n,"robotics") || Has(n,"roboticist");
  const bool strongMedic =
      Has(n,"medic") || Has(n,"medical") || Has(n,"doctor") || Has(n,"surgeon") || Has(n,"first aid");
  const bool strongCooking =
      Has(n,"chef") || Has(n,"cook's") || Has(n,"cooks ") || Has(n,"cooking") || Has(n,"kitchen");
  const bool strongSmith =
      Has(n,"weapon smith") || Has(n,"armour smith") || Has(n,"armor smith") ||
      Has(n,"crossbow smith") || Has(n,"smithing") || Has(n,"forge tool");
  const bool strongUtilitySemantic =
      HasWord(n,"assassin") || HasWord(n,"assassins") || HasWord(n,"thief") ||
      HasWord(n,"thieves") || Has(n,"stealth") || Has(n,"infiltrat") ||
      HasWord(n,"ninja") || HasWord(n,"ninjas") || HasWord(n,"scout") ||
      HasWord(n,"scouts") || HasWord(n,"ranger") || HasWord(n,"rangers") ||
      HasWord(n,"traveler") || HasWord(n,"travelers") || HasWord(n,"traveller") ||
      HasWord(n,"travellers") || HasWord(n,"wanderer") || HasWord(n,"wanderers");

  // Weapon-class items are allowed when their normal Kenshi name/description strongly implies
  // a profession or utility role. This deliberately supports roleplay items such as Farmer's
  // Sword, Pitchfork, Chef's Knife, Engineer's Hammer, Assassin's Blade, Thief's Dagger, etc.
  // Ordinary combat weapons with no such semantics stay out.
  if (item.weapon && !(strongFarming || strongMining || strongResearch || strongEngineering ||
                       strongRobotics || strongMedic || strongCooking || strongSmith ||
                       strongUtilitySemantic)) return out;

  if (strongFarming) AddTag(out,TAG_TOOL_FARMING);
  if (strongMining) AddTag(out,TAG_TOOL_MINING);
  if (Has(n,"lab coat") || Has(n,"research coat")) AddTag(out,TAG_BODY_RESEARCH);
  if (strongResearch) AddTag(out,TAG_TOOL_RESEARCH);
  if (strongEngineering || Has(n,"tool belt")) AddTag(out,TAG_TOOL_ENGINEERING);
  if (strongRobotics) AddTag(out,TAG_TOOL_ROBOTICS);
  if (strongMedic) AddTag(out,TAG_TOOL_MEDIC);
  if (Has(n,"weapon smith")) AddTag(out,TAG_TOOL_WEAPON_SMITH);
  if (Has(n,"armour smith") || Has(n,"armor smith")) AddTag(out,TAG_TOOL_ARMOUR_SMITH);
  if (Has(n,"crossbow smith")) AddTag(out,TAG_TOOL_CROSSBOW_SMITH);
  if (strongCooking) AddTag(out,TAG_TOOL_COOKING);
  if (Has(n,"straw hat")) { AddTag(out,TAG_HEAD_FARMING); AddTag(out,TAG_SCOUT_GEAR); }

  // Ambiguous real/modded gear: generic fallback is only used when stronger semantics are absent.
  // This prevents Assassin's Rags, Straw Hats, Doctor masks, etc. from also becoming arbitrary workwear.
  const bool strongProfession = strongFarming || strongMining || strongResearch || strongEngineering ||
                                strongRobotics || strongMedic || strongCooking || strongSmith;
  const bool strongUtility = strongUtilitySemantic || HasWord(n,"burglar") || HasWord(n,"burglars") ||
                             Has(n,"swim") || Has(n,"diving") || Has(n,"diver") ||
                             Has(n,"load bearing") || Has(n,"load-bearing") ||
                             Has(n,"cargo frame");
  const bool plainHat = (Has(n," hat") || Has(n,"hat ")) && !Has(n,"helmet") && !Has(n,"armoured") && !Has(n,"armored") && !Has(n,"plate");
  const bool simpleWorkCloth = HasWord(n,"rag") || HasWord(n,"rags") || Has(n,"workwear") || HasWord(n,"worker") || Has(n,"work shirt") || Has(n,"cloth shirt") || HasWord(n,"apron") || Has(n,"overalls");
  if ((plainHat || simpleWorkCloth) && !item.weapon && !strongProfession && !strongUtility) AddTag(out,TAG_WORKWEAR_GENERIC);
  if ((Has(n,"goggle") || Has(n,"glasses") || HasWord(n,"visor") || HasWord(n,"visors")) &&
      !Has(n,"helmet") && !strongProfession && !strongUtility &&
      (item.armour || item.robotLimb || Has(n,"head")))
    AddTag(out,TAG_GOGGLES_GENERIC);
  if (Has(n,"running shoe") || Has(n,"running boot") || Has(n,"runner shoe") || Has(n,"sneaker") ||
      Has(n,"wooden sandal") || Has(n,"drifter's boot")) AddTag(out,TAG_BOOTS_TRAVEL);
  if (Has(n,"miner") && (Has(n,"hat") || Has(n,"helmet") || Has(n,"goggle"))) AddTag(out,TAG_HEAD_MINING);
  if ((Has(n,"research") || Has(n,"science")) &&
      (Has(n,"goggle") || Has(n,"glass") || HasWord(n,"visor") || HasWord(n,"visors")))
    AddTag(out,TAG_HEAD_RESEARCH);
  if (Has(n,"glove") && (Has(n,"work") || Has(n,"industrial"))) AddTag(out,TAG_GLOVES_WORK);
  if (Has(n,"boot") && (Has(n,"work") || Has(n,"industrial"))) AddTag(out,TAG_BOOTS_WORK);
  if (Has(n,"boot") && (Has(n,"travel") || Has(n,"scout") || Has(n,"runner"))) AddTag(out,TAG_BOOTS_TRAVEL);
  const bool packLike=item.container || Has(n,"backpack") || Has(n," satchel") ||
                      Has(n," pack") || Has(n," bag") || Has(n,"basket");
  if (Has(n,"ore pack") || Has(n,"mining pack") ||
      (packLike && (HasWord(n,"miner") || HasWord(n,"miners") || HasWord(n,"mining"))))
    AddTag(out,TAG_PACK_ORE);
  if (Has(n,"crop pack") || Has(n,"farm pack") ||
      (packLike && (HasWord(n,"farmer") || HasWord(n,"farmers") || HasWord(n,"farmhand"))))
    AddTag(out,TAG_PACK_CROP);
  if (Has(n,"construction pack") || Has(n,"builder pack") ||
      (packLike && (strongEngineering || Has(n,"construction"))))
    AddTag(out,TAG_PACK_CONSTRUCTION);
  if (Has(n,"medical pack") || Has(n,"medic pack") ||
      (packLike && strongMedic))
    AddTag(out,TAG_PACK_MEDICAL);
  if (Has(n,"trade pack") || Has(n,"caravan pack") ||
      ((Has(n,"trader's") || Has(n,"trader ") || HasWord(n,"traders")) && Has(n,"backpack")))
    AddTag(out,TAG_PACK_TRADE);
  if (Has(n,"research pack") || Has(n,"tech pack") ||
      (packLike && (strongResearch || strongRobotics || Has(n,"tech "))))
    AddTag(out,TAG_PACK_TECH);
  if (Has(n,"hauling pack") || Has(n,"hauler pack") || Has(n,"cargo pack") ||
      Has(n,"porter pack") || Has(n,"load bearing") || Has(n,"load-bearing") ||
      Has(n,"cargo frame")) AddTag(out,TAG_PACK_HAULING);
  if (Has(n,"turret") && (Has(n,"goggle") || HasWord(n,"visor") || HasWord(n,"visors") || Has(n,"gear"))) AddTag(out,TAG_TURRET_GEAR);
  if (HasWord(n,"scout") || HasWord(n,"scouts") || HasWord(n,"ranger") || HasWord(n,"rangers") ||
      HasWord(n,"traveler") || HasWord(n,"travelers") || HasWord(n,"traveller") ||
      HasWord(n,"travellers") || HasWord(n,"wanderer") || HasWord(n,"wanderers"))
    AddTag(out,TAG_SCOUT_GEAR);
  if (Has(n,"stealth") || Has(n,"infiltrat") || Has(n,"ninja")) AddTag(out,TAG_STEALTH_GEAR);
  if (HasWord(n,"assassin") || HasWord(n,"assassins")) AddTag(out,TAG_ASSASSIN_GEAR);
  if (HasWord(n,"thief") || HasWord(n,"thieves") || HasWord(n,"burglar") ||
      HasWord(n,"burglars") || Has(n,"lockpick")) AddTag(out,TAG_THIEF_GEAR);
  if (Has(n,"swim") || Has(n,"diving") || Has(n,"diver") || Has(n,"flipper") ||
      Has(n,"swim fin") || Has(n,"wetsuit")) AddTag(out,TAG_SWIM_GEAR);

  const bool hasSpecialPack =
      std::find(out.begin(),out.end(),TAG_PACK_ORE)!=out.end() ||
      std::find(out.begin(),out.end(),TAG_PACK_CROP)!=out.end() ||
      std::find(out.begin(),out.end(),TAG_PACK_CONSTRUCTION)!=out.end() ||
      std::find(out.begin(),out.end(),TAG_PACK_MEDICAL)!=out.end() ||
      std::find(out.begin(),out.end(),TAG_PACK_TRADE)!=out.end() ||
      std::find(out.begin(),out.end(),TAG_PACK_TECH)!=out.end() ||
      std::find(out.begin(),out.end(),TAG_PACK_HAULING)!=out.end();
  if(item.container && !hasSpecialPack &&
     (Has(n,"backpack") || Has(n," bag") || Has(n,"bag ") || Has(n,"basket")))
    AddTag(out,TAG_PACK_GENERIC);
  return out;
}

static void AddStat(std::vector<ProfessionStat>& out, ProfessionStat s) {
  if (s!=STAT_NONE && std::find(out.begin(),out.end(),s)==out.end()) out.push_back(s);
}

std::vector<ProfessionStat> AllowedStats(const std::vector<ItemTag>& tags) {
  std::vector<ProfessionStat> out;
  for (size_t i=0;i<tags.size();++i) {
    switch(tags[i]) {
      case TAG_TOOL_FARMING: case TAG_HEAD_FARMING: case TAG_PACK_CROP: AddStat(out,STAT_FARMING); break;
      case TAG_TOOL_MINING: case TAG_HEAD_MINING: case TAG_PACK_ORE: AddStat(out,STAT_LABOURING); break;
      case TAG_TOOL_RESEARCH: case TAG_HEAD_RESEARCH: case TAG_BODY_RESEARCH: case TAG_PACK_TECH:
        AddStat(out,STAT_SCIENCE); AddStat(out,STAT_ROBOTICS); break;
      case TAG_TOOL_ENGINEERING: case TAG_BODY_ENGINEERING: case TAG_PACK_CONSTRUCTION:
        AddStat(out,STAT_ENGINEERING); AddStat(out,STAT_LABOURING); break;
      case TAG_TOOL_ROBOTICS: AddStat(out,STAT_ROBOTICS); AddStat(out,STAT_ENGINEERING); break;
      case TAG_TOOL_MEDIC: case TAG_BODY_MEDIC: case TAG_PACK_MEDICAL: AddStat(out,STAT_MEDIC); break;
      case TAG_TOOL_WEAPON_SMITH: AddStat(out,STAT_WEAPON_SMITH); break;
      case TAG_TOOL_ARMOUR_SMITH: case TAG_BODY_SMITH: AddStat(out,STAT_ARMOUR_SMITH); AddStat(out,STAT_WEAPON_SMITH); break;
      case TAG_TOOL_CROSSBOW_SMITH: AddStat(out,STAT_CROSSBOW_SMITH); break;
      case TAG_TOOL_COOKING: AddStat(out,STAT_COOKING); break;
      case TAG_GLOVES_WORK: AddStat(out,STAT_LABOURING); AddStat(out,STAT_ENGINEERING); break;
      case TAG_BOOTS_WORK: AddStat(out,STAT_LABOURING); AddStat(out,STAT_ATHLETICS); break;
      case TAG_BOOTS_TRAVEL: case TAG_SCOUT_GEAR: AddStat(out,STAT_ATHLETICS); AddStat(out,STAT_PERCEPTION); break;
      case TAG_TURRET_GEAR: AddStat(out,STAT_TURRETS); AddStat(out,STAT_PERCEPTION); break;
      case TAG_STEALTH_GEAR: AddStat(out,STAT_STEALTH); AddStat(out,STAT_LOCKPICKING); break;
      case TAG_ASSASSIN_GEAR: AddStat(out,STAT_STEALTH); AddStat(out,STAT_ASSASSINATION); break;
      case TAG_THIEF_GEAR: AddStat(out,STAT_STEALTH); AddStat(out,STAT_LOCKPICKING); AddStat(out,STAT_THIEVERY); break;
      case TAG_SWIM_GEAR: AddStat(out,STAT_SWIMMING); break;
      case TAG_PACK_TRADE: case TAG_PACK_HAULING: case TAG_PACK_GENERIC: AddStat(out,STAT_ATHLETICS); break;
      case TAG_WORKWEAR_GENERIC:
        AddStat(out,STAT_FARMING); AddStat(out,STAT_LABOURING); AddStat(out,STAT_ENGINEERING);
        AddStat(out,STAT_COOKING); AddStat(out,STAT_MEDIC); AddStat(out,STAT_SCIENCE);
        AddStat(out,STAT_ROBOTICS); AddStat(out,STAT_WEAPON_SMITH); AddStat(out,STAT_ARMOUR_SMITH);
        AddStat(out,STAT_CROSSBOW_SMITH); break;
      case TAG_GOGGLES_GENERIC:
        AddStat(out,STAT_PERCEPTION); AddStat(out,STAT_SCIENCE); AddStat(out,STAT_ENGINEERING); AddStat(out,STAT_ROBOTICS); AddStat(out,STAT_TURRETS); break;
      default: break;
    }
  }
  return out;
}

bool IsProfessionStat(ProfessionStat s) { return s>STAT_NONE && s<=STAT_THIEVERY; }

unsigned int Hash32(const std::string& text) {
  unsigned int h=2166136261u;
  for(size_t i=0;i<text.size();++i){h^=(unsigned char)text[i];h*=16777619u;}
  return h?h:0x9e3779b9u;
}

float UnitRoll(unsigned int& s) {
  if(!s)s=0x9e3779b9u;
  s^=s<<13; s^=s>>17; s^=s<<5;
  return (float)(s&0x00FFFFFFu)/16777215.0f;
}

AffixRecord RollAffixes(const ItemDescriptor& item,const RoleProfile& role,
                        const RuleConfig& cfg,const std::vector<ItemTag>& tags,
                        const std::string& key,unsigned int seed,bool crafted) {
  AffixRecord out; out.instanceKey=key; out.baseId=item.baseId; out.tier=ProgressionTier(item);
  if(!cfg.enabled || !item.equippable || item.stackable || item.legendary ||
     (item.weapon && role.unique && item.weaponLevel>=70) || tags.empty()) return out;
  std::vector<ProfessionStat> pool=AllowedStats(tags);
  if(pool.empty()) return out;
  const bool contextualGeneric =
      std::find(tags.begin(),tags.end(),TAG_WORKWEAR_GENERIC)!=tags.end() ||
      std::find(tags.begin(),tags.end(),TAG_GOGGLES_GENERIC)!=tags.end();
  const bool hasMatchingRole =
      role.primary!=STAT_NONE && std::find(pool.begin(),pool.end(),role.primary)!=pool.end();
  if(contextualGeneric && !hasMatchingRole && !role.traderSource && !role.worldLootSource)
    return out;

  // Generic fallback gear is broad only when there is no wearer profession context
  // (e.g. trader stock or world loot). Once a real NPC role matches, keep the item
  // coherent with that role instead of filling top-tier extra affixes from unrelated
  // professions in the generic pool.
  if(contextualGeneric && hasMatchingRole) {
    pool.clear();
    pool.push_back(role.primary);
  }

  float chance=TierAffixChance(out.tier)*cfg.globalChance;
  if(role.worldLootSource) chance*=cfg.worldLootMultiplier;
  if(role.slave || role.wealth01<.15f) chance*=cfg.poorNpcMultiplier;
  if(hasMatchingRole) chance*=cfg.npcRoleMultiplier;
  if(crafted) chance*=cfg.playerCraftMultiplier;
  if(chance>1) chance=1;
  unsigned int state=seed^Hash32(key)^Hash32(item.baseId);
  if(UnitRoll(state)>chance) return out;
  float lo,hi; TierRange(out.tier,lo,hi);
  int cap=TierAffixCap(out.tier);
  if(cap>cfg.maxAffixes) cap=cfg.maxAffixes;
  if(cap>(int)pool.size()) cap=(int)pool.size();
  int count=1;
  if(cap>=2){
    float secondChance=(out.tier==3)?0.35f:(out.tier==4)?0.50f:(out.tier==5)?0.65f:0.80f;
    if(UnitRoll(state)<secondChance) count=2;
  }
  if(cap>=3 && count>=2){
    float thirdChance=(out.tier==5)?0.25f:0.45f;
    if(UnitRoll(state)<thirdChance) count=3;
  }
  std::vector<ProfessionStat> rem=pool;
  for(int i=0;i<count && !rem.empty();++i){
    size_t idx=(size_t)(UnitRoll(state)*rem.size()); if(idx>=rem.size()) idx=rem.size()-1;
    ProfessionStat st=rem[idx];
    std::vector<ProfessionStat>::iterator roleIt=std::find(rem.begin(),rem.end(),role.primary);
    if(role.primary!=STAT_NONE && roleIt!=rem.end() && ((contextualGeneric && i==0) || UnitRoll(state)<.70f)){
      st=role.primary; idx=(size_t)(roleIt-rem.begin());
    }
    float p=lo+(hi-lo)*UnitRoll(state); p=(float)((int)(p*10+.5f))/10.0f;
    out.affixes.push_back(Affix(st,p)); rem.erase(rem.begin()+idx);
  }
  return out;
}

float AggregatePercent(const std::vector<AffixRecord>& records, ProfessionStat stat) {
  float total=0;
  for(size_t i=0;i<records.size();++i)
    for(size_t j=0;j<records[i].affixes.size();++j)
      if(records[i].affixes[j].stat==stat) total+=records[i].affixes[j].percent;
  return total;
}

float EffectiveStatValue(float baseValue, float totalPercent, bool unmodified,
                         float hardCap) {
  if (unmodified || totalPercent == 0.0f) return baseValue;
  float result = baseValue * (1.0f + totalPercent / 100.0f);
  if (result < 0.0f) result = 0.0f;
  if (hardCap > 0.0f && result > hardCap) result = hardCap;
  return result;
}

float WealthFromBestSkill(float bestSkill) {
  // Was 0.2 + best*0.8/100 (never below 0.2), so the poor-NPC multiplier (wealth < 0.15) only
  // ever applied to slaves. Now linear up to skill 80: best skill below 12 counts as poor.
  if (bestSkill <= 0.0f) return 0.0f;
  if (bestSkill >= 80.0f) return 1.0f;
  return bestSkill / 80.0f;
}

static bool HasTagCore(const std::vector<ItemTag>& tags, ItemTag tag) {
  return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

float SpecialistPackItemWeightMultiplier(const std::vector<ItemTag>& tags,
                                         const std::string& itemName,
                                         const std::string& itemBaseId,
                                         bool isTradeItem) {
  const std::string n = Lower(itemName + " " + itemBaseId);
  if (HasTagCore(tags, TAG_PACK_ORE)) {
    if (Has(n,"ore") || Has(n,"raw iron") || Has(n,"copper")) return 0.25f;
  }
  if (HasTagCore(tags, TAG_PACK_CROP)) {
    if (Has(n,"wheatstraw") || Has(n,"cactus") || Has(n,"greenfruit") ||
        Has(n,"riceweed") || Has(n,"hemp") || Has(n,"cotton")) return 0.30f;
  }
  if (HasTagCore(tags, TAG_PACK_CONSTRUCTION)) {
    if (Has(n,"building material") || Has(n,"iron plate") ||
        Has(n,"steel bar") || Has(n,"copper alloy")) return 0.35f;
  }
  if (HasTagCore(tags, TAG_PACK_MEDICAL)) {
    if (Has(n,"first aid") || Has(n,"splint") || Has(n,"repair kit") ||
        Has(n,"medical")) return 0.35f;
  }
  if (HasTagCore(tags, TAG_PACK_TECH)) {
    if (Has(n,"ancient science") || Has(n,"engineering research") ||
        Has(n,"ai core") || Has(n,"book") || Has(n,"cpu") ||
        Has(n,"power core")) return 0.35f;
  }
  if (HasTagCore(tags, TAG_PACK_TRADE) && isTradeItem) return 0.55f;
  if (HasTagCore(tags, TAG_PACK_HAULING)) return 0.75f;
  return 1.0f;
}

std::string SerializeRecord(const AffixRecord& r) {
  std::ostringstream s; s<<r.instanceKey<<'\t'<<r.baseId<<'\t'<<r.tier<<'\t';
  for(size_t i=0;i<r.affixes.size();++i){if(i)s<<',';s<<(int)r.affixes[i].stat<<':'<<std::fixed<<std::setprecision(1)<<r.affixes[i].percent;}
  return s.str();
}

bool ParseRecord(const std::string& line, AffixRecord& out) {
  std::vector<std::string> f; size_t st=0;
  for(;;){size_t p=line.find('\t',st);if(p==std::string::npos){f.push_back(line.substr(st));break;}f.push_back(line.substr(st,p-st));st=p+1;}
  if(f.size()!=4 || f[0].empty() || f[1].empty()) return false;
  out=AffixRecord();out.instanceKey=f[0];out.baseId=f[1];out.tier=std::atoi(f[2].c_str());
  if(out.tier<0 || out.tier>6) return false;
  if(f[3].empty()) return true;
  st=0;
  while(st<f[3].size()){
    size_t c=f[3].find(',',st); std::string tok=f[3].substr(st,c==std::string::npos?std::string::npos:c-st);
    size_t col=tok.find(':'); if(col==std::string::npos) return false;
    int sn=std::atoi(tok.substr(0,col).c_str()); float p=(float)std::atof(tok.substr(col+1).c_str());
    if(sn<=STAT_NONE || sn>STAT_THIEVERY || p<-100 || p>500) return false;
    out.affixes.push_back(Affix((ProfessionStat)sn,p));
    if(c==std::string::npos) break; st=c+1;
  }
  return true;
}

} // namespace PGP
