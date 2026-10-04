
#include "ProfessionGearCore.h"

#include <windows.h>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <core/Functions.h>
#include <kenshi/Building/Building.h>
#include <kenshi/Building/CraftingBuilding.h>
#include <kenshi/Building/FarmBuilding.h>
#include <kenshi/Building/ProductionBuilding.h>
#include <kenshi/Character.h>
#include <kenshi/CharStats.h>
#include <kenshi/Faction.h>
#include <kenshi/GameWorld.h>
#include <kenshi/GameData.h>
#include <kenshi/Gear.h>
#include <kenshi/Inventory.h>
#include <kenshi/Item.h>
#include <kenshi/PlayerInterface.h>
#include <kenshi/util/StringPair.h>
#include "KenshiAutomationHarness.h" // test commands, if the harness is installed

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace {

CRITICAL_SECTION g_lock;
PGP::RuleConfig g_cfg;
std::map<std::string, PGP::AffixRecord> g_records;
std::map<std::string, std::string> g_runtimeItemIds;
std::map<std::string, std::map<PGP::ProfessionStat, float> > g_bonusCache;
std::set<std::string> g_forcedKeys;           // records set by pg_force_affix (test only, kept active)
std::map<std::string, DWORD> g_shopScanTick;  // trader handle -> last shop-storage scan
LONG g_persistentIdCounter = 0;
const char* kPersistentIdField = "ProfessionGearPersistentId";
bool g_refreshingInventory = false;
std::map<std::string, std::vector<PGP::ItemTag> > g_overrides;
std::set<std::string> g_exclusions;
std::string g_dir;
std::string g_dbPath;
std::string g_logPath;
DWORD g_lastScan = 0;
bool g_dirty = false;
bool g_started = false;

typedef void (*PlayerUpdateFn)(PlayerInterface*);
typedef float (*GetStatFn)(const CharStats*, StatsEnumerated, bool);
typedef void (*CraftFinishedFn)(CraftingBuilding*, Item*);
typedef void (*TooltipFn)(InventoryItemBase*, Ogre::vector<StringPair>::type&);
typedef float (*InventoryWeightFn)(Inventory*);
typedef void (*InventoryAddRemoveFn)(Inventory*, Item*);
typedef void (*InventoryUpdateFn)(Inventory*, Item*, int);
typedef GameData* (*ItemSerialiseInventoryFn)(Item*, GameDataContainer*, GameData*);
typedef void (*ItemLoadInventoryFn)(Item*, GameDataContainer*, GameData*);
typedef Item* (*InventoryBuyItemFn)(Inventory*, Item*, RootObject*);
typedef void (*OperateFn)(Building*, Character*, float);

PlayerUpdateFn g_playerUpdateOrig = 0;
GetStatFn g_getStatOrig = 0;
CraftFinishedFn g_craftOrig = 0;
TooltipFn g_tipBaseOrig = 0;
TooltipFn g_tipArmourOrig = 0;
TooltipFn g_tipContainerOrig = 0;
TooltipFn g_tipCrossbowOrig = 0;
TooltipFn g_tipSwordOrig = 0;
InventoryWeightFn g_inventoryWeightOrig = 0;
InventoryAddRemoveFn g_inventoryAddOrig = 0;
InventoryAddRemoveFn g_inventoryRemoveOrig = 0;
InventoryUpdateFn g_inventoryUpdateOrig = 0;
ItemSerialiseInventoryFn g_itemSerialiseInventoryOrig = 0;
ItemLoadInventoryFn g_itemLoadInventoryOrig = 0;
InventoryBuyItemFn g_buyItemOrig = 0;
OperateFn g_productionOperateOrig = 0;
OperateFn g_farmOperateOrig = 0;

// Job path (row 177). Every worker tick calls Building::operate(worker, amount). Test 177 showed a
// +50% hooked Labouring did not change Stone Mine output, so production may read the raw skill.
// Diagnostics per building (calls, summed amount, output before/after) answer that, and
// JobOperateScaling (ini, or pg_jobscale at run time) multiplies the worker's amount by
// (1 + equipped bonus of the job's stat): Labouring for production machines and mines,
// Farming for farms. Run m16 (Manual Stone Processor, 30 game min windows): +50% hooked Labouring
// alone gave output x1.05 (the job reads the raw skill), with scaling x1.59. Default on.
// Explicit constructor: VS2010 does not zero the POD members of a struct with std::string members
// on map::operator[] (live m15: calls ~2^32, output_start garbage).
struct OperateStats {
  unsigned long long calls; double amount; double scaledAmount;
  float outputStart; float outputLast; double outputProgress; bool hasLast;
  std::string worker; std::string name;
  OperateStats() : calls(0), amount(0), scaledAmount(0), outputStart(0), outputLast(0),
                   outputProgress(0), hasLast(false) {}
};
std::map<Building*, OperateStats> g_operateStats;
bool g_jobOperateScaling = true;   // default on since run m16 (row 177)
// Row 89 (TEST ONLY, off by default, never read from the ini): pg_force_critical on makes
// CraftingBuilding::calculateCriticalChance answer 1.0, so the next real craft is a critical
// success and the roll can be checked against the better finished quality.
bool g_forceCritical = false;
// Balance rows 193-199 (TEST ONLY, off by default): pg_statprobe counts CharStats::getStat calls per profession
// stat for one character (modified vs unmodified reads), so a harness readout (chance, runspeed, detect...) shows
// whether the game formula reads the hooked stat at all.
bool g_statProbe = false;
const Character* g_statProbeWho = 0;
int g_statProbeMod[PGP::STAT_THIEVERY+1] = {0};
int g_statProbeRaw[PGP::STAT_THIEVERY+1] = {0};
typedef float (*CritChanceFn)(CraftingBuilding*, Character*);
CritChanceFn g_critChanceOrig = 0;

std::string IntStr(long long v) { std::ostringstream s; s<<v; return s.str(); }
float HookInventoryWeight(Inventory* inv);

void Log(const std::string& s) {
  std::ofstream f(g_logPath.c_str(), std::ios::app);
  if (f.is_open()) f << s << "\n";
}

std::string DirName(const std::string& p) {
  size_t x=p.find_last_of("\\/");
  return x==std::string::npos?".":p.substr(0,x);
}

std::string RuntimeItemKey(Item* item) {
  if (!item) return "";
  try { return item->getHandle().toString(); } catch (...) { return ""; }
}

std::string BaseId(Item* item);

std::string GeneratePersistentItemId(Item* item) {
  LONG n=InterlockedIncrement(&g_persistentIdCounter);
  std::string runtime=RuntimeItemKey(item);
  std::ostringstream s;
  s<<"pgp1-"<<GetCurrentProcessId()<<"-"<<GetTickCount()<<"-"<<n<<"-"
   <<PGP::Hash32(runtime+"|"+BaseId(item));
  return s.str();
}

void BindPersistentItemId(Item* item,const std::string& id) {
  if(!item || id.empty()) return;
  const std::string runtime=RuntimeItemKey(item);
  if(runtime.empty()) return;
  EnterCriticalSection(&g_lock);
  g_runtimeItemIds[runtime]=id;
  LeaveCriticalSection(&g_lock);
}

void UnbindPersistentItemId(Item* item) {
  if(!item) return;
  const std::string runtime=RuntimeItemKey(item);
  if(runtime.empty()) return;
  EnterCriticalSection(&g_lock);
  g_runtimeItemIds.erase(runtime);
  LeaveCriticalSection(&g_lock);
}

std::string PersistentItemId(Item* item,bool createIfMissing) {
  if(!item) return "";
  const std::string runtime=RuntimeItemKey(item);
  if(runtime.empty()) return "";
  EnterCriticalSection(&g_lock);
  std::map<std::string,std::string>::const_iterator it=g_runtimeItemIds.find(runtime);
  if(it!=g_runtimeItemIds.end()){
    std::string id=it->second;
    LeaveCriticalSection(&g_lock);
    return id;
  }
  LeaveCriticalSection(&g_lock);
  if(!createIfMissing) return "";
  const std::string id=GeneratePersistentItemId(item);
  BindPersistentItemId(item,id);
  return id;
}

std::string BaseId(Item* item) {
  if (!item || !item->data) return "";
  if (!item->data->stringID.empty()) return item->data->stringID;
  std::ostringstream s; s<<"id:"<<item->data->id; return s.str();
}

PGP::ItemDescriptor Describe(Item* item) {
  PGP::ItemDescriptor d;
  if (!item) return d;
  d.baseId=BaseId(item);
  try { d.name=item->getName(); } catch (...) {}
  if (item->data) {
    boost::unordered::unordered_map<std::string,std::string,boost::hash<std::string>,std::equal_to<std::string>,Ogre::STLAllocator<std::pair<std::string const,std::string>,Ogre::GeneralAllocPolicy> >::const_iterator di=item->data->sdata.find("description");
    if(di!=item->data->sdata.end()) d.description=di->second;
  }

  Gear* gear=dynamic_cast<Gear*>(item);
  Weapon* weapon=dynamic_cast<Weapon*>(item);
  Armour* armour=dynamic_cast<Armour*>(item);
  RobotLimbItem* limb=dynamic_cast<RobotLimbItem*>(item);
  ContainerItem* container=dynamic_cast<ContainerItem*>(item);

  d.weapon=(weapon!=0);
  d.armour=(armour!=0);
  d.robotLimb=(limb!=0);
  d.container=(container!=0);
  d.equippable=(gear!=0 || container!=0);
  if(d.weapon) d.category="weapon";
  else if(d.armour) d.category="armour";
  else if(d.robotLimb) d.category="robot_limb";
  else if(d.container) d.category="container";
  else d.category="item";

  d.quality=gear?gear->getLevel01():item->quality;
  d.weaponLevel=(d.weapon&&gear)?gear->level_0_100:-1;
  d.equipped=item->isEquipped;
  d.slot=item->inventorySection;
  d.stackable=item->quantity>1;
  try {
    Inventory* parent=item->getInventory();
    InventorySection* section=parent?parent->getSection(item->inventorySection):0;
    if(section) d.stackable=(item->isStackable(section)>1);
  } catch (...) {}

  // Explicitly unique named/special item instances are author-defined gear and are protected
  // from ProfessionGear augmentation regardless of type or quality.
  d.legendary=item->isUnique;

  if(d.weapon&&gear){
    std::string manufacturer;
    if(item->manufacturerData) manufacturer=PGP::Lower(item->manufacturerData->name+" "+item->manufacturerData->stringID);
    std::string lname=PGP::Lower(d.name+" "+d.description);
    d.legendary=d.legendary || gear->level_0_100>=100 || manufacturer.find("cross")!=std::string::npos ||
                 lname.find("meitou")!=std::string::npos || lname.find("legendary")!=std::string::npos;
  }
  return d;
}

PGP::ProfessionStat MapStat(StatsEnumerated st) {
  switch(st) {
    case STAT_LABOURING:return PGP::STAT_LABOURING;
    case STAT_SCIENCE:return PGP::STAT_SCIENCE;
    case STAT_ENGINEERING:return PGP::STAT_ENGINEERING;
    case STAT_ROBOTICS:return PGP::STAT_ROBOTICS;
    case STAT_SMITHING_WEAPON:return PGP::STAT_WEAPON_SMITH;
    case STAT_SMITHING_ARMOUR:return PGP::STAT_ARMOUR_SMITH;
    case STAT_SMITHING_BOW:return PGP::STAT_CROSSBOW_SMITH;
    case STAT_MEDIC:return PGP::STAT_MEDIC;
    case STAT_TURRETS:return PGP::STAT_TURRETS;
    case STAT_FARMING:return PGP::STAT_FARMING;
    case STAT_COOKING:return PGP::STAT_COOKING;
    case STAT_ATHLETICS:return PGP::STAT_ATHLETICS;
    case STAT_SWIMMING:return PGP::STAT_SWIMMING;
    case STAT_PERCEPTION:return PGP::STAT_PERCEPTION;
    case STAT_STEALTH:return PGP::STAT_STEALTH;
    case STAT_ASSASSINATION:return PGP::STAT_ASSASSINATION;
    case STAT_LOCKPICKING:return PGP::STAT_LOCKPICKING;
    case STAT_THIEVING:return PGP::STAT_THIEVERY;
    default:return PGP::STAT_NONE;
  }
}

StatsEnumerated KenshiStat(PGP::ProfessionStat st) {
  switch(st) {
    case PGP::STAT_LABOURING:return STAT_LABOURING;
    case PGP::STAT_SCIENCE:return STAT_SCIENCE;
    case PGP::STAT_ENGINEERING:return STAT_ENGINEERING;
    case PGP::STAT_ROBOTICS:return STAT_ROBOTICS;
    case PGP::STAT_WEAPON_SMITH:return STAT_SMITHING_WEAPON;
    case PGP::STAT_ARMOUR_SMITH:return STAT_SMITHING_ARMOUR;
    case PGP::STAT_CROSSBOW_SMITH:return STAT_SMITHING_BOW;
    case PGP::STAT_MEDIC:return STAT_MEDIC;
    case PGP::STAT_TURRETS:return STAT_TURRETS;
    case PGP::STAT_FARMING:return STAT_FARMING;
    case PGP::STAT_COOKING:return STAT_COOKING;
    case PGP::STAT_ATHLETICS:return STAT_ATHLETICS;
    case PGP::STAT_SWIMMING:return STAT_SWIMMING;
    case PGP::STAT_PERCEPTION:return STAT_PERCEPTION;
    case PGP::STAT_STEALTH:return STAT_STEALTH;
    case PGP::STAT_ASSASSINATION:return STAT_ASSASSINATION;
    case PGP::STAT_LOCKPICKING:return STAT_LOCKPICKING;
    case PGP::STAT_THIEVERY:return STAT_THIEVING;
    default:return STAT_NONE;
  }
}

static bool HasTag(const std::vector<PGP::ItemTag>& tags, PGP::ItemTag tag) {
  return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

std::vector<PGP::ItemTag> TagsFor(const PGP::ItemDescriptor& item) {
  const std::string id=PGP::Lower(item.baseId);
  if(g_exclusions.count(id)) return std::vector<PGP::ItemTag>();
  std::map<std::string,std::vector<PGP::ItemTag> >::const_iterator it=g_overrides.find(id);
  if(it!=g_overrides.end()) return it->second;
  if(!g_cfg.autoClassify) return std::vector<PGP::ItemTag>();
  return PGP::Classify(item,g_overrides,g_exclusions);
}

// An item can carry a record from an older build or older rules (e.g. a Staff that rolled Farming
// before weapons were classified by name, or an item excluded by a later rule, or AutoClassify
// turned off). Such records stay in the sidecar (nothing is destroyed) but give no bonus and no
// tooltip while the item is not eligible now. Harness-forced records (test only) stay active.
bool ItemCurrentlyEligible(Item* item,const std::string& key) {
  if(!item) return false;
  if(!key.empty()){
    EnterCriticalSection(&g_lock);
    const bool forced=g_forcedKeys.count(key)!=0;
    LeaveCriticalSection(&g_lock);
    if(forced) return true;
  }
  PGP::ItemDescriptor d=Describe(item);
  if(!d.equippable || d.legendary || d.stackable) return false;
  if(g_exclusions.count(PGP::Lower(d.baseId))) return false;
  return !TagsFor(d).empty();
}

PGP::RoleProfile RoleFor(Character* c) {
  PGP::RoleProfile r;
  if (!c) return r;
  try { r.slave=(c->isSlave()!=NOT_SLAVE); } catch (...) {}
  try { r.traderSource=c->isATrader(); } catch (...) {}
  try {
    CharStats* s=c->getStats();
    if (!s) return r;
    struct Pair { PGP::ProfessionStat p; StatsEnumerated k; };
    Pair a[]={
      {PGP::STAT_LABOURING,STAT_LABOURING},{PGP::STAT_SCIENCE,STAT_SCIENCE},
      {PGP::STAT_ENGINEERING,STAT_ENGINEERING},{PGP::STAT_ROBOTICS,STAT_ROBOTICS},
      {PGP::STAT_WEAPON_SMITH,STAT_SMITHING_WEAPON},{PGP::STAT_ARMOUR_SMITH,STAT_SMITHING_ARMOUR},
      {PGP::STAT_CROSSBOW_SMITH,STAT_SMITHING_BOW},{PGP::STAT_MEDIC,STAT_MEDIC},
      {PGP::STAT_TURRETS,STAT_TURRETS},{PGP::STAT_FARMING,STAT_FARMING},
      {PGP::STAT_COOKING,STAT_COOKING}
    };
    float best=-1;
    for(size_t i=0;i<sizeof(a)/sizeof(a[0]);++i){
      float v=g_getStatOrig?g_getStatOrig(s,a[i].k,true):s->getStat(a[i].k,true);
      if(v>best){best=v;r.primary=a[i].p;}
    }
    r.wealth01=PGP::WealthFromBestSkill(best);
    r.unique=c->isUnique();
  } catch (...) {}
  return r;
}

void SaveDb() {
  EnterCriticalSection(&g_lock);
  if (!g_dirty) { LeaveCriticalSection(&g_lock); return; }
  std::string tmp=g_dbPath+".tmp";
  std::ofstream f(tmp.c_str(),std::ios::trunc);
  if (f.is_open()) {
    f<<"# Profession Gear Progression v2 persistent-item-id\n";
    for(std::map<std::string,PGP::AffixRecord>::const_iterator i=g_records.begin();i!=g_records.end();++i)
      f<<PGP::SerializeRecord(i->second)<<"\n";
    f.close();
    if(MoveFileExA(tmp.c_str(),g_dbPath.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
      g_dirty=false;
    else
      Log("failed to atomically replace affix database");
  }
  LeaveCriticalSection(&g_lock);
}

void LoadDb() {
  std::ifstream f(g_dbPath.c_str());
  if(!f.is_open()) return;
  std::string line;
  unsigned long legacyIgnored=0;
  while(std::getline(f,line)){
    if(line.empty()||line[0]=='#') continue;
    PGP::AffixRecord r;
    if(!PGP::ParseRecord(line,r)) continue;
    if(r.instanceKey.find("pgp1-")!=0){ ++legacyIgnored; continue; }
    g_records[r.instanceKey]=r;
  }
  std::ostringstream ss; ss << "loaded affixes=" << (unsigned long)g_records.size()
                            << " legacyIgnored=" << legacyIgnored; Log(ss.str());
}

PGP::AffixRecord* EnsureRecord(Item* item, Character* owner, bool crafted) {
  if(!item || !g_cfg.enabled) return 0;   // Enabled=false: no new records from any path
  PGP::ItemDescriptor d=Describe(item);
  const bool explicitlyExcluded=g_exclusions.count(PGP::Lower(d.baseId))!=0;
  const bool forceIneligible=!d.equippable || d.legendary || d.stackable || explicitlyExcluded;
  if(forceIneligible) return 0;

  std::vector<PGP::ItemTag> tags=TagsFor(d);
  if(tags.empty()) return 0;

  std::string key=PersistentItemId(item,true);
  if(key.empty()) return 0;

  EnterCriticalSection(&g_lock);
  std::map<std::string,PGP::AffixRecord>::iterator existing=g_records.find(key);
  if(existing!=g_records.end() && existing->second.baseId==d.baseId){
    PGP::AffixRecord* p=&existing->second;
    LeaveCriticalSection(&g_lock);
    return p;
  }
  const bool collision=(existing!=g_records.end() && existing->second.baseId!=d.baseId);
  LeaveCriticalSection(&g_lock);

  if(collision){
    key=GeneratePersistentItemId(item);
    BindPersistentItemId(item,key);
    Log(std::string("persistent-id collision/reuse: rebound runtime=")+RuntimeItemKey(item)+" newId="+key+" baseId="+d.baseId);
  }

  PGP::RoleProfile role=RoleFor(owner);
  // Trader stock should be generated from the item's plausible profession pool, not from
  // whatever unrelated skill happens to be highest on the shopkeeper. Equipped trader gear
  // still uses the trader character's own role context.
  if(role.traderSource && !d.equipped) role.primary=PGP::STAT_NONE;

  // A previously unseen non-crafted item first observed on a player character is treated as
  // exploration/world loot, not as gear generated for the player's profession. This avoids
  // chest/ruin loot adapting itself to whoever picked it up.
  if(owner && !crafted){
    try {
      if(owner->isPlayerCharacter()){
        role.primary=PGP::STAT_NONE;
        role.traderSource=false;
        role.worldLootSource=true;
      }
    } catch (...) {}
  }

  // Poverty describes the NPC who wears/carries the gear. Shop stock, world loot and crafted
  // items are not judged by the shopkeeper's, finder's or crafter's own skills.
  if(role.worldLootSource || crafted || (role.traderSource && !d.equipped)){
    role.wealth01=0.5f;
    role.slave=false;
  }

  unsigned int seed=PGP::Hash32(key+"|"+d.baseId);
  PGP::AffixRecord r=PGP::RollAffixes(d,role,g_cfg,tags,key,seed,crafted);

  if(g_cfg.verboseLogging){
    std::ostringstream ss;
    const char* source=crafted?"crafted":(role.traderSource?"trader":(role.worldLootSource?"world_loot":"npc"));
    ss<<"roll source="<<source
      <<" key="<<key
      <<" baseId="<<d.baseId
      <<" name=\""<<d.name<<"\""
      <<" equipped="<<(d.equipped?1:0)
      <<" tier="<<r.tier
      <<" tags=";
    for(size_t i=0;i<tags.size();++i){if(i)ss<<",";ss<<PGP::TagName(tags[i]);}
    ss<<" affixes=";
    if(r.affixes.empty()) ss<<"none";
    for(size_t i=0;i<r.affixes.size();++i){
      if(i)ss<<",";
      ss<<PGP::StatName(r.affixes[i].stat)<<":"<<r.affixes[i].percent;
    }
    Log(ss.str());
  }

  EnterCriticalSection(&g_lock);
  g_records[key]=r;
  g_dirty=true;
  PGP::AffixRecord* p=&g_records[key];
  LeaveCriticalSection(&g_lock);
  return p;
}

void CollectCharacterInventoryItems(Inventory* inv,std::vector<Item*>& out) {
  out.clear();
  if(!inv) return;
  std::set<Item*> seen;
  try {
    lektor<InventorySection*>& sections=inv->getAllSections();
    for(unsigned int si=0;si<sections.size();++si){
      InventorySection* section=sections[si];
      if(!section) continue;
      const Ogre::vector<InventorySection::SectionItem>::type& items=section->getItems();
      for(size_t ii=0;ii<items.size();++ii){
        Item* item=items[ii].item;
        if(item && seen.insert(item).second) out.push_back(item);
      }
    }
  } catch (...) {}

  // Defensive fallback for inventories/mods that do not expose every section normally.
  try {
    const lektor<Item*>& all=inv->getAllItems();
    for(unsigned int i=0;i<all.size();++i)
      if(all[i] && seen.insert(all[i]).second) out.push_back(all[i]);
  } catch (...) {}
}

void RebuildCharacterBonusCache(Character* c) {
  if(!c) return;
  Inventory* inv=0;
  try { inv=c->getInventory(); } catch (...) { return; }
  if(!inv) return;
  std::map<PGP::ProfessionStat,float> totals;
  std::vector<Item*> items;
  std::vector<std::string> equippedIds;
  CollectCharacterInventoryItems(inv,items);
  for(size_t i=0;i<items.size();++i){
    Item* item=items[i];
    if(!item || !item->isEquipped) continue;
    const std::string itemId=PersistentItemId(item,false);
    if(!itemId.empty() && ItemCurrentlyEligible(item,itemId)) equippedIds.push_back(itemId);
  }
  EnterCriticalSection(&g_lock);
  for(size_t i=0;i<equippedIds.size();++i){
    std::map<std::string,PGP::AffixRecord>::const_iterator it=g_records.find(equippedIds[i]);
    if(it==g_records.end()) continue;
    for(size_t j=0;j<it->second.affixes.size();++j)
      totals[it->second.affixes[j].stat]+=it->second.affixes[j].percent;
  }
  std::string characterKey;
  try { characterKey=c->getHandle().toString(); } catch (...) {}
  if(!characterKey.empty()) g_bonusCache[characterKey]=totals;
  LeaveCriticalSection(&g_lock);
}

void ProcessCharacter(Character* c) {
  if(!g_cfg.enabled || !c) return;
  Inventory* inv=0;
  try { inv=c->getInventory(); } catch (...) { return; }
  if(!inv) return;
  std::vector<Item*> items;
  CollectCharacterInventoryItems(inv,items);
  for(size_t i=0;i<items.size();++i) if(items[i]) EnsureRecord(items[i],c,false);
  RebuildCharacterBonusCache(c);
}

// ---- Natural shop stock ----------------------------------------------------
// Kenshi keeps most shop stock in storage of the trader's faction next to her (barrels, weapon
// cabinets, shelves), not in her own inventory. The character scan never saw it, so a bought item
// was first seen on the player and rolled as world loot, and shop tooltips showed nothing.
// Storage of the trader's faction within 60 of a trader is scanned as trader stock (every 10 s per
// trader). Same rule as the harness "shopstock" command.
// 60, not 30: traders walk around their shop; live 2026-10-03 the Trade Ninjas' shop counters and
// barrels were 11-55 from the trader and Apothecary Abia had nothing of her faction within 30.
const float kShopRadius = 60.0f;
const DWORD kShopScanMs = 10000;

void CollectShopStorages(GameWorld* world, Character* trader, std::vector<Building*>& out) {
  out.clear();
  if(!world || !trader) return;
  // One reused result list: lektor has no destructor, a fresh one per call would leak its buffer.
  static lektor<RootObject*> nearby;
  nearby.clear();
  Faction* fac=0;
  try {
    fac=trader->getFaction();
    world->getObjectsWithinSphere(nearby,trader->getPosition(),kShopRadius,BUILDING,256,0);
  } catch (...) { return; }
  for(uint32_t i=0;i<nearby.size();++i){
    Building* b=dynamic_cast<Building*>(nearby.stuff[i]);
    if(!b) continue;
    try { if(b->getFaction()!=fac || !b->getInventory()) continue; } catch (...) { continue; }
    out.push_back(b);
  }
}

// Returns the number of eligible stock items seen; force = ignore the 10 s throttle.
size_t ProcessTraderShop(GameWorld* world, Character* c, bool force) {
  if(!g_cfg.enabled || !world || !c) return 0;
  bool trader=false;
  try { trader=c->isATrader(); } catch (...) {}
  if(!trader) return 0;
  std::string key;
  try { key=c->getHandle().toString(); } catch (...) { return 0; }
  const DWORD now=GetTickCount();
  EnterCriticalSection(&g_lock);
  std::map<std::string,DWORD>::iterator t=g_shopScanTick.find(key);
  if(!force && t!=g_shopScanTick.end() && now-t->second<kShopScanMs){ LeaveCriticalSection(&g_lock); return 0; }
  g_shopScanTick[key]=now;
  const size_t before=g_records.size();
  LeaveCriticalSection(&g_lock);

  std::vector<Building*> shops;
  CollectShopStorages(world,c,shops);
  size_t eligible=0;
  for(size_t s=0;s<shops.size();++s){
    std::vector<Item*> items;
    try { CollectCharacterInventoryItems(shops[s]->getInventory(),items); } catch (...) { continue; }
    for(size_t i=0;i<items.size();++i) if(items[i] && EnsureRecord(items[i],c,false)) ++eligible;
  }
  EnterCriticalSection(&g_lock);
  const size_t after=g_records.size();
  LeaveCriticalSection(&g_lock);
  if(g_cfg.verboseLogging && after>before){
    std::ostringstream ss;
    ss<<"shop stock: trader=\""<<c->getName()<<"\" storages="<<(unsigned long)shops.size()
      <<" eligible="<<(unsigned long)eligible<<" new_records="<<(unsigned long)(after-before);
    Log(ss.str());
  }
  return eligible;
}

float EquippedBonus(Character* c, PGP::ProfessionStat stat) {
  if(!g_cfg.enabled || !c || stat==PGP::STAT_NONE) return 0;
  std::string characterKey;
  try { characterKey=c->getHandle().toString(); } catch (...) { return 0; }
  if(characterKey.empty()) return 0;
  EnterCriticalSection(&g_lock);
  std::map<std::string,std::map<PGP::ProfessionStat,float> >::const_iterator ci=g_bonusCache.find(characterKey);
  if(ci!=g_bonusCache.end()){
    std::map<PGP::ProfessionStat,float>::const_iterator bi=ci->second.find(stat);
    float value=(bi==ci->second.end())?0.0f:bi->second;
    LeaveCriticalSection(&g_lock);
    return value;
  }
  LeaveCriticalSection(&g_lock);
  RebuildCharacterBonusCache(c);
  EnterCriticalSection(&g_lock);
  ci=g_bonusCache.find(characterKey);
  float value=0.0f;
  if(ci!=g_bonusCache.end()){
    std::map<PGP::ProfessionStat,float>::const_iterator bi=ci->second.find(stat);
    if(bi!=ci->second.end()) value=bi->second;
  }
  LeaveCriticalSection(&g_lock);
  return value;
}

void AppendTip(InventoryItemBase* base,Ogre::vector<StringPair>::type& lines) {
  if(!g_cfg.enabled) return;
  Item* item=dynamic_cast<Item*>(base);
  if(!item) return;
  PGP::ItemDescriptor current=Describe(item);
  if(!current.equippable || current.legendary || current.stackable || g_exclusions.count(PGP::Lower(current.baseId))) return;
  for(size_t i=0;i<lines.size();++i) if(lines[i].s1=="Profession Gear") return;
  std::string key=PersistentItemId(item,false);
  if(key.empty()) return;
  if(!ItemCurrentlyEligible(item,key)) return;
  const std::string baseId=current.baseId;
  EnterCriticalSection(&g_lock);
  std::map<std::string,PGP::AffixRecord>::const_iterator it=g_records.find(key);
  if(it!=g_records.end() && it->second.baseId==baseId && !it->second.affixes.empty()){
    lines.push_back(StringPair("Profession Gear",""));
    for(size_t j=0;j<it->second.affixes.size();++j){
      std::ostringstream s; s<<"+"<<it->second.affixes[j].percent<<"%";
      lines.push_back(StringPair(PGP::StatName(it->second.affixes[j].stat),s.str()));
    }
  }
  LeaveCriticalSection(&g_lock);
}

GameData* HookItemSerialiseInInventory(Item* item,GameDataContainer* container,GameData* refs) {
  GameData* state=0;
  try {
    state=g_itemSerialiseInventoryOrig?g_itemSerialiseInventoryOrig(item,container,refs):0;
  } catch (...) {
    Log("exception in Item::serialiseInInventory original");
    return state;
  }
  if(!item || !state || !g_cfg.enabled) return state;
  try {
    PGP::ItemDescriptor d=Describe(item);
    const bool excluded=g_exclusions.count(PGP::Lower(d.baseId))!=0;
    if(!d.equippable || d.legendary || d.stackable || excluded) return state;
    if(TagsFor(d).empty()) return state;
    const std::string id=PersistentItemId(item,true);
    if(!id.empty()){
      state->sdata[kPersistentIdField]=id;
      state->activeValues[kPersistentIdField]=true;
      if(g_cfg.verboseLogging)
        Log(std::string("serialize item id=")+id+" runtime="+RuntimeItemKey(item)+" baseId="+d.baseId);
    }
  } catch (...) {
    Log("exception writing ProfessionGearPersistentId");
  }
  return state;
}

void HookItemLoadFromSerialiseInInventory(Item* item,GameDataContainer* container,GameData* state) {
  std::string id;
  try {
    if(state){
      boost::unordered::unordered_map<std::string,std::string,boost::hash<std::string>,std::equal_to<std::string>,Ogre::STLAllocator<std::pair<std::string const,std::string>,Ogre::GeneralAllocPolicy> >::const_iterator it=
        state->sdata.find(kPersistentIdField);
      if(it!=state->sdata.end()) id=it->second;
    }
  } catch (...) {}

  try {
    if(g_itemLoadInventoryOrig) g_itemLoadInventoryOrig(item,container,state);
  } catch (...) {
    Log("exception in Item::loadFromSerialiseInInventory original");
    return;
  }

  if(!item) return;
  try {
    if(id.empty()){
      UnbindPersistentItemId(item);
      return;
    }
    BindPersistentItemId(item,id);
    if(g_cfg.verboseLogging)
      Log(std::string("restore item id=")+id+" runtime="+RuntimeItemKey(item)+" baseId="+BaseId(item));
  } catch (...) {
    Log("exception restoring ProfessionGearPersistentId");
  }
}

// ---- Kenshi Automation Harness commands (TEST ONLY) ----------------------
KAH_Api g_kah;
bool g_kahConnected = false;
DWORD g_kahLastTry = 0;

GameWorld* KahWorld() {
  HMODULE kl=GetModuleHandleA("KenshiLib.dll");
  GameWorld** pp=kl ? (GameWorld**)GetProcAddress(kl,"?ou@@3PEAVGameWorld@@EA") : 0;
  return pp ? *pp : 0;
}

// Exact (case-insensitive) name; player characters first.
Character* KahFindCharacter(const std::string& name) {
  GameWorld* world=KahWorld();
  if(!world) return 0;
  const std::string wanted=PGP::Lower(name);
  Character* other=0;
  try {
    if(world->player)
      for(uint32_t i=0;i<world->player->playerCharacters.size();++i){
        Character* c=world->player->playerCharacters[i];
        if(c && PGP::Lower(c->getName())==wanted) return c;
      }
    const ogre_unordered_set<Character*>::type& chars=world->getCharacterUpdateList();
    for(ogre_unordered_set<Character*>::type::const_iterator i=chars.begin();i!=chars.end();++i)
      if(*i && PGP::Lower((*i)->getName())==wanted){ other=*i; break; }
  } catch (...) {}
  return other;
}

Item* KahFindItem(Character* c, const std::string& name) {
  Inventory* inv=0;
  try { inv=c->getInventory(); } catch (...) { return 0; }
  if(!inv) return 0;
  std::vector<Item*> items;
  CollectCharacterInventoryItems(inv,items);
  const std::string wanted=PGP::Lower(name);
  Item* partial=0;
  for(size_t i=0;i<items.size();++i){
    Item* item=items[i];
    if(!item) continue;
    std::string n, base;
    try { n=PGP::Lower(item->getName()); base=PGP::Lower(BaseId(item)); } catch (...) { continue; }
    if(n==wanted || base==wanted) return item;
    if(!partial && n.find(wanted)!=std::string::npos) partial=item;
  }
  return partial;
}

std::string KahRecordText(Item* item) {
  const std::string key=PersistentItemId(item,false);
  std::ostringstream ss;
  ss<<item->getName()<<" key="<<(key.empty()?"-":key)<<" equipped="<<(item->isEquipped?1:0);
  EnterCriticalSection(&g_lock);
  std::map<std::string,PGP::AffixRecord>::const_iterator it=key.empty()?g_records.end():g_records.find(key);
  if(it==g_records.end()) ss<<" record=none";
  else {
    ss<<" tier="<<it->second.tier<<" affixes=";
    if(it->second.affixes.empty()) ss<<"none";
    for(size_t i=0;i<it->second.affixes.size();++i)
      ss<<(i?",":"")<<PGP::StatName(it->second.affixes[i].stat)<<":"<<it->second.affixes[i].percent;
  }
  LeaveCriticalSection(&g_lock);
  return ss.str();
}

// Resolves argv[1] (npc) and argv[2] (item); writes an error to the reply.
bool KahNpcItem(int argc,const char* const* argv,KAH_Reply* r,Character*& c,Item*& item,bool needItem) {
  if(argc<2 || (needItem && argc<3)){ r->append(r,"usage: see help"); return false; }
  c=KahFindCharacter(argv[1]);
  if(!c){ r->append(r,(std::string("no character named: ")+argv[1]).c_str()); return false; }
  item=0;
  if(argc>=3 && needItem){
    item=KahFindItem(c,argv[2]);
    if(!item){ r->append(r,(c->getName()+" has no item matching: "+argv[2]).c_str()); return false; }
  }
  return true;
}

int KahInfo(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  Character* c; Item* item;
  if(!KahNpcItem(argc,argv,r,c,item,true)) return KAH_ERROR;
  if(!EnsureRecord(item,c,false)){ r->append(r,(KahRecordText(item)+" (not eligible for affixes)").c_str()); return KAH_OK; }
  r->append(r,KahRecordText(item).c_str());
  return KAH_OK;
}

int KahForce(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  Character* c; Item* item;
  if(argc<5){ r->append(r,"usage: pg_force_affix <npc> <item> <stat> <percent> [<stat> <percent>...] [tier n]"); return KAH_ERROR; }
  if(!KahNpcItem(argc,argv,r,c,item,true)) return KAH_ERROR;
  PGP::AffixRecord rec;
  int tier=-1;
  for(int i=3;i+1<argc;i+=2){
    if(PGP::Lower(argv[i])=="tier"){ tier=atoi(argv[i+1]); continue; }
    PGP::ProfessionStat st=PGP::ParseStat(argv[i]);
    if(st==PGP::STAT_NONE){ r->append(r,(std::string("unknown stat: ")+argv[i]).c_str()); return KAH_ERROR; }
    rec.affixes.push_back(PGP::Affix(st,(float)atof(argv[i+1])));
  }
  const std::string key=PersistentItemId(item,true);
  if(key.empty()){ r->append(r,"item has no persistent id"); return KAH_ERROR; }
  EnterCriticalSection(&g_lock);
  std::map<std::string,PGP::AffixRecord>::iterator it=g_records.find(key);
  rec.instanceKey=key;
  rec.baseId=BaseId(item);
  rec.tier=tier>=0?tier:(it!=g_records.end()?it->second.tier:0);
  g_records[key]=rec;
  g_forcedKeys.insert(key);
  g_dirty=true;
  LeaveCriticalSection(&g_lock);
  RebuildCharacterBonusCache(c);
  Log("harness: forced "+KahRecordText(item)+" on "+c->getName());
  r->append(r,("forced: "+KahRecordText(item)).c_str());
  return KAH_OK;
}

int KahClear(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  Character* c; Item* item;
  if(!KahNpcItem(argc,argv,r,c,item,argc>=3)) return KAH_ERROR;
  std::vector<Item*> targets;
  if(item) targets.push_back(item);
  else {
    Inventory* inv=c->getInventory();
    std::vector<Item*> items;
    if(inv) CollectCharacterInventoryItems(inv,items);
    for(size_t i=0;i<items.size();++i) if(items[i] && items[i]->isEquipped) targets.push_back(items[i]);
  }
  int cleared=0;
  for(size_t i=0;i<targets.size();++i){
    const std::string key=PersistentItemId(targets[i],false);
    if(key.empty()) continue;
    EnterCriticalSection(&g_lock);
    std::map<std::string,PGP::AffixRecord>::iterator it=g_records.find(key);
    if(it!=g_records.end() && !it->second.affixes.empty()){ it->second.affixes.clear(); ++cleared; g_dirty=true; }
    LeaveCriticalSection(&g_lock);
  }
  RebuildCharacterBonusCache(c);
  std::ostringstream ss;
  ss<<c->getName()<<": cleared affixes on "<<cleared<<" of "<<targets.size()<<" item(s)";
  Log("harness: "+ss.str());
  r->append(r,ss.str().c_str());
  return KAH_OK;
}

int KahRoll(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  Character* c; Item* item;
  if(!KahNpcItem(argc,argv,r,c,item,true)) return KAH_ERROR;
  const std::string key=GeneratePersistentItemId(item);
  BindPersistentItemId(item,key);
  if(!EnsureRecord(item,c,false)){ r->append(r,(KahRecordText(item)+" (not eligible for affixes)").c_str()); return KAH_ERROR; }
  RebuildCharacterBonusCache(c);
  Log("harness: rerolled "+KahRecordText(item));
  r->append(r,("rolled: "+KahRecordText(item)).c_str());
  return KAH_OK;
}

int KahBonus(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<3){ r->append(r,"usage: pg_bonus <npc> <stat>"); return KAH_ERROR; }
  Character* c=KahFindCharacter(argv[1]);
  if(!c){ r->append(r,(std::string("no character named: ")+argv[1]).c_str()); return KAH_ERROR; }
  PGP::ProfessionStat st=PGP::ParseStat(argv[2]);
  if(st==PGP::STAT_NONE){ r->append(r,(std::string("unknown stat: ")+argv[2]).c_str()); return KAH_ERROR; }
  RebuildCharacterBonusCache(c);
  const float bonus=EquippedBonus(c,st);
  std::ostringstream ss;
  ss<<c->getName()<<" "<<PGP::StatName(st)<<" equipped_bonus="<<bonus<<"%";
  // The game's own numbers: base (unmodified), vanilla effective (Kenshi without ProfessionGear)
  // and the hooked effective value every job reads; expected = vanilla * (1 + bonus), cap 150.
  const StatsEnumerated k=KenshiStat(st);
  CharStats* stats=0;
  try { stats=c->getStats(); } catch (...) {}
  if(stats && k!=STAT_NONE && g_getStatOrig){
    const float base=g_getStatOrig(stats,k,true);
    const float vanilla=g_getStatOrig(stats,k,false);
    const float effective=stats->getStat(k,false);
    const float expected=PGP::EffectiveStatValue(vanilla,bonus,false,150.0f);
    const float diff=effective-expected;
    ss.setf(std::ios::fixed); ss.precision(2);
    ss<<" base="<<base<<" vanilla_effective="<<vanilla<<" effective="<<effective<<" expected="<<expected
      <<" match="<<((diff<0.05f && diff>-0.05f)?1:0);
  }
  r->append(r,ss.str().c_str());
  return KAH_OK;
}

// Nearest building within `radius` (default 300) of `anchor` (default: the first player
// character) whose name contains `name`.
Building* KahFindBuilding(const std::string& name, float& dist, float radius=300.0f, Character* anchor=0) {
  GameWorld* world=KahWorld();
  if(!world) return 0;
  if(!anchor){
    if(!world->player || world->player->playerCharacters.size()==0) return 0;
    anchor=world->player->playerCharacters[0];
  }
  if(!anchor) return 0;
  static lektor<RootObject*> nearby;   // reused: lektor has no destructor
  nearby.clear();
  const std::string wanted=PGP::Lower(name);
  Building* best=0;
  try {
    const Ogre::Vector3 pos=anchor->getPosition();
    world->getObjectsWithinSphere(nearby,pos,radius,BUILDING,512,0);
    for(uint32_t i=0;i<nearby.size();++i){
      Building* b=dynamic_cast<Building*>(nearby.stuff[i]);
      if(!b || !b->getInventory()) continue;
      if(PGP::Lower(b->getName()).find(wanted)==std::string::npos) continue;
      const float d=b->getPosition().distance(pos);
      if(!best || d<dist){ best=b; dist=d; }
    }
  } catch (...) {}
  return best;
}

// Items of an inventory with a ProfessionGear record or an eligible tag; read only (no new rolls).
std::string KahInventoryRecords(Inventory* inv,const std::string& filter,int& shown,int& total) {
  std::vector<Item*> items;
  CollectCharacterInventoryItems(inv,items);
  const std::string wanted=PGP::Lower(filter);
  std::string out;
  shown=0; total=(int)items.size();
  for(size_t i=0;i<items.size();++i){
    Item* item=items[i];
    if(!item) continue;
    std::string n;
    try { n=PGP::Lower(item->getName()); } catch (...) { continue; }
    if(!wanted.empty() && n.find(wanted)==std::string::npos && PGP::Lower(BaseId(item))!=wanted) continue;
    if(wanted.empty() && PersistentItemId(item,false).empty()) continue;  // list only known items
    out+=" ["+KahRecordText(item)+"]";
    ++shown;
  }
  return out;
}

// pg_shop <trader>: scan the trader's shop storage now and list the stock records.
int KahShop(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<2){ r->append(r,"usage: pg_shop <trader>"); return KAH_ERROR; }
  Character* c=KahFindCharacter(argv[1]);
  if(!c){ r->append(r,(std::string("no character named: ")+argv[1]).c_str()); return KAH_ERROR; }
  GameWorld* world=KahWorld();
  bool trader=false;
  try { trader=c->isATrader(); } catch (...) {}
  const size_t eligible=ProcessTraderShop(world,c,true);
  std::vector<Building*> shops;
  CollectShopStorages(world,c,shops);
  std::ostringstream ss;
  ss<<c->getName()<<" trader="<<(trader?1:0)<<" storages="<<(unsigned long)shops.size()
    <<" eligible_stock="<<(unsigned long)eligible;
  std::string body;
  for(size_t s=0;s<shops.size();++s){
    int shown=0,total=0;
    std::string list=KahInventoryRecords(shops[s]->getInventory(),"",shown,total);
    body+=" || "+shops[s]->getName()+" (items="+IntStr(total)+"):"+list;
  }
  r->append(r,(ss.str()+body).c_str());
  return trader?KAH_OK:KAH_ERROR;
}

// pg_building <building> [item]: records of items in a building (bench output, storage).
int KahBuilding(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<2){ r->append(r,"usage: pg_building <building name> [item]"); return KAH_ERROR; }
  float dist=0;
  Building* b=KahFindBuilding(argv[1],dist);
  if(!b){ r->append(r,(std::string("no building with an inventory matching '")+argv[1]+"' within 300").c_str()); return KAH_ERROR; }
  int shown=0,total=0;
  const std::string list=KahInventoryRecords(b->getInventory(),argc>=3?argv[2]:"",shown,total);
  std::ostringstream ss;
  ss<<b->getName()<<" dist="<<(int)dist<<" items="<<total<<" listed="<<shown<<":"<<list;
  r->append(r,ss.str().c_str());
  return (argc>=3 && shown==0)?KAH_ERROR:KAH_OK;
}

// pg_take <npc> <building> <item>: move the same item instance from a building to the npc.
int KahTake(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<4){ r->append(r,"usage: pg_take <npc> <building> <item>"); return KAH_ERROR; }
  Character* c=KahFindCharacter(argv[1]);
  if(!c){ r->append(r,(std::string("no character named: ")+argv[1]).c_str()); return KAH_ERROR; }
  float dist=0;
  Building* b=KahFindBuilding(argv[2],dist);
  if(!b){ r->append(r,(std::string("no building with an inventory matching '")+argv[2]+"' within 300").c_str()); return KAH_ERROR; }
  Inventory* from=b->getInventory();
  Inventory* to=c->getInventory();
  if(!from || !to){ r->append(r,"missing inventory"); return KAH_ERROR; }
  std::vector<Item*> items;
  CollectCharacterInventoryItems(from,items);
  const std::string wanted=PGP::Lower(argv[3]);
  Item* item=0;
  for(size_t i=0;i<items.size() && !item;++i){
    if(!items[i]) continue;
    std::string n;
    try { n=PGP::Lower(items[i]->getName()); } catch (...) { continue; }
    if(n==wanted || PGP::Lower(BaseId(items[i]))==wanted) item=items[i];
  }
  for(size_t i=0;i<items.size() && !item;++i)
    if(items[i] && PGP::Lower(items[i]->getName()).find(wanted)!=std::string::npos) item=items[i];
  if(!item){ r->append(r,(b->getName()+" has no item matching: "+argv[3]).c_str()); return KAH_ERROR; }
  const std::string before=KahRecordText(item);
  const int qty=item->quantity>0?item->quantity:1;
  Item* moved=from->removeItemDontDestroy_returnsItem(item,qty,false);
  if(!moved){ r->append(r,"remove from the building failed"); return KAH_ERROR; }
  // Main inventory first (never auto-equipped), as a player drag into the bag does.
  InventorySection* main=0;
  try { main=to->getSection("main"); } catch (...) {}
  bool added=main?main->addItem(moved,qty):false;
  if(!added && !to->addItem(moved,qty,false,false)){
    from->addItem(moved,qty,false,false);
    r->append(r,(c->getName()+" has no room; item left in "+b->getName()).c_str());
    return KAH_ERROR;
  }
  RebuildCharacterBonusCache(c);
  Log("harness: took "+KahRecordText(moved)+" from "+b->getName()+" to "+c->getName());
  r->append(r,("took from "+b->getName()+" to "+c->getName()+": "+KahRecordText(moved)+
               " | before: "+before).c_str());
  return KAH_OK;
}

// pg_store <npc> <building> <item>: move the same (unequipped) item instance into a building.
int KahStore(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<4){ r->append(r,"usage: pg_store <npc> <building> <item>"); return KAH_ERROR; }
  Character* c; Item* item;
  const char* args[3]={argv[0],argv[1],argv[3]};
  if(!KahNpcItem(3,args,r,c,item,true)) return KAH_ERROR;
  if(item->isEquipped){ r->append(r,("refusing to store an equipped item: "+KahRecordText(item)).c_str()); return KAH_ERROR; }
  float dist=0;
  Building* b=KahFindBuilding(argv[2],dist);
  if(!b){ r->append(r,(std::string("no building with an inventory matching '")+argv[2]+"' within 300").c_str()); return KAH_ERROR; }
  Inventory* from=c->getInventory();
  Inventory* to=b->getInventory();
  if(!from || !to){ r->append(r,"missing inventory"); return KAH_ERROR; }
  const std::string before=KahRecordText(item);
  const int qty=item->quantity>0?item->quantity:1;
  Item* moved=from->removeItemDontDestroy_returnsItem(item,qty,false);
  if(!moved){ r->append(r,"remove from the character failed"); return KAH_ERROR; }
  if(!to->addItem(moved,qty,false,false)){
    from->addItem(moved,qty,false,false);
    r->append(r,(b->getName()+" has no room; item stays with "+c->getName()).c_str());
    return KAH_ERROR;
  }
  RebuildCharacterBonusCache(c);
  Log("harness: stored "+KahRecordText(moved)+" from "+c->getName()+" in "+b->getName());
  r->append(r,("stored in "+b->getName()+" from "+c->getName()+": "+KahRecordText(moved)+" | before: "+before).c_str());
  return KAH_OK;
}

// pg_check <npc> <item>: the item's record obeys the rules (tier from quality/grade, affix count
// cap, magnitudes inside the tier range, stats inside the item's legal pool).
int KahCheck(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  Character* c; Item* item;
  if(!KahNpcItem(argc,argv,r,c,item,true)) return KAH_ERROR;
  const std::string key=PersistentItemId(item,false);
  PGP::AffixRecord rec;
  bool found=false, forced=false;
  EnterCriticalSection(&g_lock);
  std::map<std::string,PGP::AffixRecord>::const_iterator it=key.empty()?g_records.end():g_records.find(key);
  if(it!=g_records.end()){ rec=it->second; found=true; }
  forced=!key.empty() && g_forcedKeys.count(key)!=0;
  LeaveCriticalSection(&g_lock);
  if(!found){ r->append(r,(KahRecordText(item)+" valid=0 problems=no_record").c_str()); return KAH_ERROR; }
  PGP::ItemDescriptor d=Describe(item);
  std::vector<PGP::ItemTag> tags=TagsFor(d);
  std::vector<PGP::ProfessionStat> pool=PGP::AllowedStats(tags);
  const int expectedTier=PGP::ProgressionTier(d);
  int cap=PGP::TierAffixCap(rec.tier);
  if(cap>g_cfg.maxAffixes) cap=g_cfg.maxAffixes;
  float lo=0,hi=0; PGP::TierRange(rec.tier,lo,hi);
  std::string problems;
  if(rec.baseId!=d.baseId) problems+=" base_mismatch";
  if(rec.tier!=expectedTier) problems+=" tier_mismatch(expected "+IntStr(expectedTier)+")";
  if((int)rec.affixes.size()>cap) problems+=" too_many_affixes(cap "+IntStr(cap)+")";
  std::set<int> seen;
  for(size_t i=0;i<rec.affixes.size();++i){
    const PGP::Affix& a=rec.affixes[i];
    if(std::find(pool.begin(),pool.end(),a.stat)==pool.end()) problems+=" illegal_stat("+PGP::StatName(a.stat)+")";
    if(a.percent<lo-0.051f || a.percent>hi+0.051f) problems+=" out_of_range("+PGP::StatName(a.stat)+")";
    if(!seen.insert((int)a.stat).second) problems+=" duplicate_stat("+PGP::StatName(a.stat)+")";
  }
  if(forced) problems.clear();  // harness-forced records are test values, not rolls
  std::ostringstream ss;
  ss<<KahRecordText(item)<<" expected_tier="<<expectedTier<<" quality="<<d.quality<<" range="<<lo<<"-"<<hi
    <<" cap="<<cap<<" tags=";
  for(size_t i=0;i<tags.size();++i) ss<<(i?",":"")<<PGP::TagName(tags[i]);
  if(tags.empty()) ss<<"none";
  ss<<" pool=";
  for(size_t i=0;i<pool.size();++i) ss<<(i?",":"")<<PGP::StatName(pool[i]);
  ss<<" forced="<<(forced?1:0)<<" valid="<<(problems.empty()?1:0);
  if(!problems.empty()) ss<<" problems="<<problems;
  r->append(r,ss.str().c_str());
  return problems.empty()?KAH_OK:KAH_ERROR;
}

// pg_loot <from npc> <to npc> <item>: move the same item instance between characters, equipped or
// not (looting a body, handing over worn gear). Lands in the target's main inventory.
int KahLoot(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<4){ r->append(r,"usage: pg_loot <from npc> <to npc> <item>"); return KAH_ERROR; }
  Character* from=KahFindCharacter(argv[1]);
  Character* to=KahFindCharacter(argv[2]);
  if(!from || !to){ r->append(r,(std::string("no character named: ")+(from?argv[2]:argv[1])).c_str()); return KAH_ERROR; }
  Item* item=KahFindItem(from,argv[3]);
  if(!item){ r->append(r,(from->getName()+" has no item matching: "+argv[3]).c_str()); return KAH_ERROR; }
  Inventory* fi=from->getInventory();
  Inventory* ti=to->getInventory();
  if(!fi || !ti){ r->append(r,"missing inventory"); return KAH_ERROR; }
  const std::string before=KahRecordText(item);
  const int qty=item->quantity>0?item->quantity:1;
  Item* moved=fi->removeItemDontDestroy_returnsItem(item,qty,false);
  if(!moved){ r->append(r,"remove failed"); return KAH_ERROR; }
  InventorySection* main=0;
  try { main=ti->getSection("main"); } catch (...) {}
  bool added=main?main->addItem(moved,qty):false;
  if(!added) added=ti->addItem(moved,qty,false,false);
  if(!added){
    fi->addItem(moved,qty,false,false);
    r->append(r,(to->getName()+" has no room; item returned to "+from->getName()).c_str());
    return KAH_ERROR;
  }
  RebuildCharacterBonusCache(from);
  RebuildCharacterBonusCache(to);
  Log("harness: looted "+KahRecordText(moved)+" from "+from->getName()+" to "+to->getName());
  r->append(r,("looted from "+from->getName()+" to "+to->getName()+": "+KahRecordText(moved)+" | before: "+before).c_str());
  return KAH_OK;
}

// pg_census [filter]: records of items on loaded characters (whose name or faction contains the filter)
// and in trader shop storage, by owner class (player/npc/trader/shop) and stat. Data for the
// distribution rows (102-104, 220-228, 237, 245).
// pg_lootscan <player npc> [radius <m>] [all]: world-loot rolls for every item lying in the buildings (chests,
// crates, shelves) within radius of a player character, as if he had picked each one up (EnsureRecord with
// him as the finder, the same call the character scan makes). Player-faction buildings are skipped unless
// `all`. Row 250 (ruin/chest loot corpus). TEST ONLY.
int KahLootScan(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<2){ r->append(r,"usage: pg_lootscan <player npc> [radius <m>] [all]"); return KAH_ERROR; }
  GameWorld* world=KahWorld();
  Character* c=KahFindCharacter(argv[1]);
  if(!world || !c){ r->append(r,(std::string("no character named: ")+argv[1]).c_str()); return KAH_ERROR; }
  bool player=false;
  try { player=c->isPlayerCharacter(); } catch (...) {}
  if(!player){ r->append(r,(c->getName()+" is not a player character: world loot needs a player finder").c_str()); return KAH_ERROR; }
  float radius=200.0f; bool all=false;
  for(int i=2;i<argc;++i){
    const std::string a=PGP::Lower(argv[i]);
    if(a=="radius" && i+1<argc) radius=(float)atof(argv[++i]);
    else if(a=="all") all=true;
  }
  if(!(radius>0 && radius<=2000)){ r->append(r,"radius must be 1..2000"); return KAH_ERROR; }
  static lektor<RootObject*> nearby;   // reused: lektor has no destructor
  nearby.clear();
  Faction* mine=0;
  try { mine=c->getFaction(); world->getObjectsWithinSphere(nearby,c->getPosition(),radius,BUILDING,2048,0); }
  catch (...) { r->append(r,"building search failed"); return KAH_ERROR; }
  int buildings=0, withItems=0, skippedOwn=0, items=0, eligible=0, already=0, rolled=0, multi=0;
  std::map<PGP::ProfessionStat,int> stats;
  std::map<int,int> tierAll, tierRolled;
  std::ostringstream per;
  int listed=0;
  for(uint32_t bi=0;bi<nearby.size();++bi){
    Building* b=dynamic_cast<Building*>(nearby.stuff[bi]);
    Inventory* inv=0;
    try { inv=b?b->getInventory():0; } catch (...) { inv=0; }
    if(!inv) continue;
    try { if(!all && mine && b->getFaction()==mine){ ++skippedOwn; continue; } } catch (...) { continue; }
    ++buildings;
    std::vector<Item*> list;
    CollectCharacterInventoryItems(inv,list);
    if(list.empty()) continue;
    ++withItems;
    int bElig=0, bRolled=0;
    for(size_t i=0;i<list.size();++i){
      Item* item=list[i];
      if(!item) continue;
      ++items;
      const std::string oldKey=PersistentItemId(item,false);
      bool had=false;
      if(!oldKey.empty()){
        EnterCriticalSection(&g_lock);
        had=g_records.count(oldKey)!=0;
        LeaveCriticalSection(&g_lock);
      }
      PGP::AffixRecord* rec=EnsureRecord(item,c,false);
      if(!rec) continue;
      ++eligible; ++bElig;
      if(had) ++already;
      ++tierAll[rec->tier];
      if(!rec->affixes.empty()){ ++rolled; ++bRolled; ++tierRolled[rec->tier]; }
      if(rec->affixes.size()>1) ++multi;
      for(size_t a=0;a<rec->affixes.size();++a) ++stats[rec->affixes[a].stat];
    }
    if(listed<15){
      ++listed;
      std::string fac="?";
      try { Faction* f=b->getFaction(); if(f) fac=f->getName(); } catch (...) {}
      per<<" | "<<b->getName()<<" ["<<fac<<"] items="<<(unsigned long)list.size()<<" eligible="<<bElig<<" rolled="<<bRolled;
    }
  }
  std::ostringstream ss;
  ss<<"lootscan finder="<<c->getName()<<" radius="<<(int)radius<<" buildings="<<buildings<<" with_items="<<withItems
    <<" skipped_own="<<skippedOwn<<" items="<<items<<" eligible="<<eligible<<" already_recorded="<<already
    <<" rolled="<<rolled<<" multi="<<multi<<" rate="<<(eligible?(double)rolled/eligible:0.0)<<" stats:";
  for(std::map<PGP::ProfessionStat,int>::const_iterator s=stats.begin();s!=stats.end();++s)
    ss<<" "<<PGP::StatName(s->first)<<"="<<s->second;
  ss<<" tiers(rolled/all):";
  for(std::map<int,int>::const_iterator t=tierAll.begin();t!=tierAll.end();++t)
    ss<<" t"<<t->first<<"="<<tierRolled[t->first]<<"/"<<t->second;
  Log("harness: "+ss.str());
  r->append(r,(ss.str()+per.str()).c_str());
  return KAH_OK;
}

int KahCensus(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  GameWorld* world=KahWorld();
  if(!world){ r->append(r,"no world"); return KAH_ERROR; }
  const std::string filter=argc>=2?PGP::Lower(argv[1]):std::string();
  const char* classes[4]={"player","npc","trader","shop"};
  int recs[4]={0,0,0,0}, rolled[4]={0,0,0,0}, multi[4]={0,0,0,0};
  std::map<PGP::ProfessionStat,int> stats[4];
  std::set<Item*> counted;
  try {
    const ogre_unordered_set<Character*>::type& chars=world->getCharacterUpdateList();
    for(ogre_unordered_set<Character*>::type::const_iterator ci=chars.begin();ci!=chars.end();++ci){
      Character* c=*ci;
      if(!c) continue;
      if(!filter.empty()){
        std::string n, f;
        try { n=PGP::Lower(c->getName()); } catch (...) { continue; }
        try { Faction* fac=c->getFaction(); if(fac) f=PGP::Lower(fac->getName()); } catch (...) {}
        if(n.find(filter)==std::string::npos && f.find(filter)==std::string::npos) continue;
      }
      int cls=1;
      try { if(c->isPlayerCharacter()) cls=0; else if(c->isATrader()) cls=2; } catch (...) {}
      std::vector<Inventory*> invs;
      invs.push_back(c->getInventory());
      std::vector<int> invCls(1,cls);
      if(cls==2){
        std::vector<Building*> shops;
        CollectShopStorages(world,c,shops);
        for(size_t s=0;s<shops.size();++s){ invs.push_back(shops[s]->getInventory()); invCls.push_back(3); }
      }
      for(size_t v=0;v<invs.size();++v){
        std::vector<Item*> items;
        CollectCharacterInventoryItems(invs[v],items);
        for(size_t i=0;i<items.size();++i){
          if(!items[i] || !counted.insert(items[i]).second) continue;
          const std::string key=PersistentItemId(items[i],false);
          if(key.empty()) continue;
          EnterCriticalSection(&g_lock);
          std::map<std::string,PGP::AffixRecord>::const_iterator it=g_records.find(key);
          if(it!=g_records.end()){
            const int k=invCls[v];
            ++recs[k];
            if(!it->second.affixes.empty()) ++rolled[k];
            if(it->second.affixes.size()>1) ++multi[k];
            for(size_t a=0;a<it->second.affixes.size();++a) ++stats[k][it->second.affixes[a].stat];
          }
          LeaveCriticalSection(&g_lock);
        }
      }
    }
  } catch (...) {}
  std::ostringstream ss;
  if(!filter.empty()) ss<<"filter=\""<<filter<<"\" ";
  for(int k=0;k<4;++k){
    ss<<(k?" || ":"")<<classes[k]<<": records="<<recs[k]<<" rolled="<<rolled[k]<<" multi="<<multi[k];
    for(std::map<PGP::ProfessionStat,int>::const_iterator s=stats[k].begin();s!=stats[k].end();++s)
      ss<<" "<<PGP::StatName(s->first)<<"="<<s->second;
  }
  r->append(r,ss.str().c_str());
  return KAH_OK;
}

// pg_pack <npc> <pack>: the backpack's ProfessionGear weight math, vanilla vs hooked.
int KahPack(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  Character* c; Item* item;
  if(!KahNpcItem(argc,argv,r,c,item,true)) return KAH_ERROR;
  ContainerItem* pack=dynamic_cast<ContainerItem*>(item);
  if(!pack){ r->append(r,(item->getName()+" is not a container item").c_str()); return KAH_ERROR; }
  Inventory* inv=pack->getInventory();
  if(!inv){ r->append(r,"backpack inventory unavailable"); return KAH_ERROR; }
  PGP::ItemDescriptor pd=Describe(pack);
  std::vector<PGP::ItemTag> tags=TagsFor(pd);
  float raw=0, adjusted=0;
  int count=0;
  const lektor<Item*>& contents=inv->getAllItems();
  for(unsigned int i=0;i<contents.size();++i){
    if(!contents[i]) continue;
    const float w=contents[i]->getItemWeight();
    raw+=w; ++count;
    adjusted+=w*PGP::SpecialistPackItemWeightMultiplier(tags,contents[i]->getName(),BaseId(contents[i]),contents[i]->isTradeItem);
  }
  inv->recalculateTotalWeight();
  const float vanilla=g_inventoryWeightOrig?g_inventoryWeightOrig(inv):0.0f;
  const float hooked=HookInventoryWeight(inv);
  float charWeight=0;
  try { Inventory* ci=c->getInventory(); if(ci){ ci->recalculateTotalWeight(); charWeight=ci->getTotalWeight(); } } catch (...) {}
  std::ostringstream ss;
  ss<<c->getName()<<" pack="<<pack->getName()<<" base="<<pd.baseId<<" tags=";
  for(size_t i=0;i<tags.size();++i) ss<<(i?",":"")<<PGP::TagName(tags[i]);
  if(tags.empty()) ss<<"none";
  ss.setf(std::ios::fixed); ss.precision(3);
  ss<<" equipped="<<(pack->isEquipped?1:0)<<" items="<<count<<" raw="<<raw
    <<" content_ratio="<<(raw>0?adjusted/raw:1.0f)<<" vanilla_total="<<vanilla<<" hooked_total="<<hooked
    <<" applied_ratio="<<(vanilla>0?hooked/vanilla:1.0f)<<" char_weight="<<charWeight;
  r->append(r,ss.str().c_str());
  return KAH_OK;
}

// pg_operate <building> [reset]: worker ticks on a production building/farm since the last reset.
int KahOperate(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc<2){ r->append(r,"usage: pg_operate <building> [reset] [radius <m>] [near <npc>]"); return KAH_ERROR; }
  bool reset=false; float radius=300.0f; Character* anchor=0;
  for(int i=2;i<argc;++i){
    const std::string a=PGP::Lower(argv[i]);
    if(a=="reset") reset=true;
    else if(a=="radius" && i+1<argc) radius=(float)atof(argv[++i]);
    else if(a=="near" && i+1<argc){
      anchor=KahFindCharacter(argv[++i]);
      if(!anchor){ r->append(r,(std::string("no character named: ")+argv[i]).c_str()); return KAH_ERROR; }
    }
  }
  float dist=0;
  Building* b=KahFindBuilding(argv[1],dist,radius,anchor);
  if(!b){ std::ostringstream e; e<<"no building with an inventory matching '"<<argv[1]<<"' within "<<radius; r->append(r,e.str().c_str()); return KAH_ERROR; }
  float out=0, progress=0;
  try { ProductionBuilding* pb=dynamic_cast<ProductionBuilding*>(b); if(pb) out=pb->getOutput(); } catch (...) {}
  try { UseableStuff* u=dynamic_cast<UseableStuff*>(b); if(u) progress=u->progressBarLevel; } catch (...) {}
  EnterCriticalSection(&g_lock);
  std::map<Building*,OperateStats>::const_iterator oi=g_operateStats.find(b);
  OperateStats o=(oi!=g_operateStats.end())?oi->second:OperateStats();
  if(reset){
    OperateStats fresh;                // baseline = the output right now
    fresh.outputStart=out; fresh.outputLast=out; fresh.hasLast=true; fresh.name=o.name;
    g_operateStats[b]=fresh;
  }
  LeaveCriticalSection(&g_lock);
  std::ostringstream ss;
  ss.setf(std::ios::fixed); ss.precision(4);
  ss<<b->getName()<<" calls="<<o.calls<<" amount="<<o.amount<<" scaled_amount="<<o.scaledAmount
    <<" avg_amount="<<(o.calls?o.amount/o.calls:0.0)<<" output_start="<<o.outputStart<<" output_now="<<out
    <<" output_gain="<<(o.calls?out-o.outputStart:0.0f)<<" output_progress="<<o.outputProgress
    <<" progress_per_call="<<(o.calls?o.outputProgress/(double)o.calls:0.0)<<" progress="<<progress<<" worker="<<(o.worker.empty()?"-":o.worker)
    <<" scaling="<<(g_jobOperateScaling?1:0)<<(reset?" (reset)":"");
  r->append(r,ss.str().c_str());
  return KAH_OK;
}

// pg_jobscale on|off: switch JobOperateScaling at run time (TEST ONLY).
int KahJobScale(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc>=2){ const std::string v=PGP::Lower(argv[1]); g_jobOperateScaling=(v=="on"||v=="1"||v=="true"); }
  Log(std::string("harness: jobOperateScaling=")+(g_jobOperateScaling?"1":"0"));
  r->append(r,(std::string("jobOperateScaling=")+(g_jobOperateScaling?"1":"0")).c_str());
  return KAH_OK;
}

// pg_force_critical on|off: every craft is a critical success while on (TEST ONLY, row 89).
int KahForceCritical(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  if(argc>=2){ const std::string v=PGP::Lower(argv[1]); g_forceCritical=(v=="on"||v=="1"||v=="true"); }
  Log(std::string("harness: forceCritical=")+(g_forceCritical?"1":"0"));
  r->append(r,(std::string("forceCritical=")+(g_forceCritical?"1":"0")+(g_critChanceOrig?"":" (hook missing)")).c_str());
  return g_critChanceOrig?KAH_OK:KAH_ERROR;
}

// pg_statprobe on <npc> | read | off (TEST ONLY): getStat calls per profession stat for that character.
int KahStatProbe(const char*,int argc,const char* const* argv,KAH_Reply* r,void*) {
  const std::string v=argc>=2?PGP::Lower(argv[1]):"read";
  if(v=="on"){
    if(argc<3){ r->append(r,"usage: pg_statprobe on <npc> | read | off"); return KAH_ERROR; }
    Character* c=KahFindCharacter(argv[2]);
    if(!c){ r->append(r,(std::string("no character named: ")+argv[2]).c_str()); return KAH_ERROR; }
    for(int i=0;i<=PGP::STAT_THIEVERY;++i){ g_statProbeMod[i]=0; g_statProbeRaw[i]=0; }
    g_statProbeWho=c; g_statProbe=true;
    r->append(r,(std::string("statprobe on ")+c->getName()).c_str());
    return KAH_OK;
  }
  std::ostringstream ss; ss<<"statprobe "<<(g_statProbe?"on":"off")<<":";
  int n=0;
  for(int i=1;i<=PGP::STAT_THIEVERY;++i){
    if(!g_statProbeMod[i]&&!g_statProbeRaw[i]) continue;
    ss<<" "<<PGP::StatName((PGP::ProfessionStat)i)<<"=mod:"<<g_statProbeMod[i]<<",raw:"<<g_statProbeRaw[i]; ++n;
    g_statProbeMod[i]=0; g_statProbeRaw[i]=0;
  }
  if(!n) ss<<" none";
  if(v=="off"){ g_statProbe=false; g_statProbeWho=0; }
  r->append(r,ss.str().c_str());
  return KAH_OK;
}

void KahTick() {
  if(g_kahConnected) return;
  DWORD now=GetTickCount();
  if(now-g_kahLastTry<1000) return;
  g_kahLastTry=now;
  if(!KAH_Connect(&g_kah)) return;
  g_kahConnected=true;
  int n=g_kah.registerCommand("pg_info","pg_info <npc> <item>",KahInfo,0)
       +g_kah.registerCommand("pg_force_affix","pg_force_affix <npc> <item> <stat> <pct> [<stat> <pct>...] [tier n]",KahForce,0)
       +g_kah.registerCommand("pg_clear","pg_clear <npc> [item]",KahClear,0)
       +g_kah.registerCommand("pg_roll","pg_roll <npc> <item>",KahRoll,0)
       +g_kah.registerCommand("pg_bonus","pg_bonus <npc> <stat>",KahBonus,0)
       +g_kah.registerCommand("pg_shop","pg_shop <trader>",KahShop,0)
       +g_kah.registerCommand("pg_building","pg_building <building> [item]",KahBuilding,0)
       +g_kah.registerCommand("pg_take","pg_take <npc> <building> <item>",KahTake,0)
       +g_kah.registerCommand("pg_pack","pg_pack <npc> <pack>",KahPack,0)
       +g_kah.registerCommand("pg_store","pg_store <npc> <building> <item>",KahStore,0)
       +g_kah.registerCommand("pg_check","pg_check <npc> <item>",KahCheck,0)
       +g_kah.registerCommand("pg_census","pg_census [name filter]",KahCensus,0)
       +g_kah.registerCommand("pg_lootscan","pg_lootscan <player npc> [radius <m>] [all]",KahLootScan,0)
       +g_kah.registerCommand("pg_loot","pg_loot <from npc> <to npc> <item>",KahLoot,0)
       +g_kah.registerCommand("pg_operate","pg_operate <building> [reset] [radius <m>] [near <npc>]",KahOperate,0)
       +g_kah.registerCommand("pg_jobscale","pg_jobscale on|off",KahJobScale,0)
       +g_kah.registerCommand("pg_statprobe","pg_statprobe on <npc> | read | off (TEST ONLY: getStat calls per stat)",KahStatProbe,0)
       +g_kah.registerCommand("pg_force_critical","pg_force_critical on|off (TEST ONLY: every craft is a critical success)",KahForceCritical,0);
  g_kah.log("ProfessionGear: test commands registered");
  std::ostringstream ss; ss<<"harness: connected, "<<n<<" commands (pg_info/pg_force_affix/pg_clear/pg_roll/pg_bonus/pg_shop/pg_building/pg_take/pg_store/pg_pack/pg_check/pg_census/pg_loot/pg_operate/pg_jobscale/pg_force_critical/pg_statprobe)";
  Log(ss.str());
}

void HookPlayerUpdate(PlayerInterface* p) {
  if(g_playerUpdateOrig) g_playerUpdateOrig(p);
  KahTick();
  DWORD now=GetTickCount();
  if(now-g_lastScan<1000) return;
  g_lastScan=now;
  GameWorld* world=0;
  HMODULE kl=GetModuleHandleA("KenshiLib.dll");
  if(kl){
    GameWorld** pp=(GameWorld**)GetProcAddress(kl,"?ou@@3PEAVGameWorld@@EA");
    if(pp) world=*pp;
  }
  if(world){
    try {
      const ogre_unordered_set<Character*>::type& chars=world->getCharacterUpdateList();
      for(ogre_unordered_set<Character*>::type::const_iterator i=chars.begin();i!=chars.end();++i){
        ProcessCharacter(*i);
        ProcessTraderShop(world,*i,false);
      }
    } catch (...) {}
  }
  SaveDb();
}

float HookGetStat(const CharStats* s,StatsEnumerated st,bool unmodified) {
  float base=g_getStatOrig?g_getStatOrig(s,st,unmodified):0;
  if(g_statProbe && s && s->me==g_statProbeWho){
    PGP::ProfessionStat q=MapStat(st);
    if(q!=PGP::STAT_NONE){ if(unmodified) ++g_statProbeRaw[q]; else ++g_statProbeMod[q]; }
  }
  if(unmodified || !s || !s->me) return base;
  PGP::ProfessionStat p=MapStat(st);
  if(p==PGP::STAT_NONE) return base;
  float pct=EquippedBonus(s->me,p);
  return PGP::EffectiveStatValue(base,pct,false,150.0f);
}

float HookInventoryWeight(Inventory* inv) {
  float base = g_inventoryWeightOrig ? g_inventoryWeightOrig(inv) : 0.0f;
  if (!g_cfg.enabled || !inv || base <= 0.0f) return base;
  RootObject* owner = 0;
  try { owner = inv->getOwner(); } catch (...) { return base; }
  ContainerItem* pack = dynamic_cast<ContainerItem*>(owner);
  if (!pack || !pack->isEquipped) return base;
  PGP::ItemDescriptor pd = Describe(pack);
  std::vector<PGP::ItemTag> tags = TagsFor(pd);
  if (!HasTag(tags,PGP::TAG_PACK_ORE) && !HasTag(tags,PGP::TAG_PACK_CROP) &&
      !HasTag(tags,PGP::TAG_PACK_CONSTRUCTION) && !HasTag(tags,PGP::TAG_PACK_MEDICAL) &&
      !HasTag(tags,PGP::TAG_PACK_TRADE) && !HasTag(tags,PGP::TAG_PACK_TECH) &&
      !HasTag(tags,PGP::TAG_PACK_HAULING)) return base;
  float raw=0.0f, adjusted=0.0f;
  try {
    const lektor<Item*>& items=inv->getAllItems();
    for(unsigned int i=0;i<items.size();++i){
      if(!items[i]) continue;
      float w=items[i]->getItemWeight();
      raw+=w;
      adjusted+=w*PGP::SpecialistPackItemWeightMultiplier(tags,items[i]->getName(),BaseId(items[i]),items[i]->isTradeItem);
    }
  } catch (...) { return base; }
  if(raw<=0.0001f) return base;
  return base*(adjusted/raw);
}

void RefreshInventoryOwner(Inventory* inv) {
  if(!g_cfg.enabled || !inv || g_refreshingInventory) return;
  Character* c=0;
  try { c=inv->getCallbackCharacter(); } catch (...) {}
  if(!c) return;
  g_refreshingInventory=true;
  try { ProcessCharacter(c); } catch (...) {}
  g_refreshingInventory=false;
}

void HookInventoryAdd(Inventory* inv,Item* item) {
  if(g_inventoryAddOrig) g_inventoryAddOrig(inv,item);
  RefreshInventoryOwner(inv);
}

void HookInventoryRemove(Inventory* inv,Item* item) {
  if(g_inventoryRemoveOrig) g_inventoryRemoveOrig(inv,item);
  RefreshInventoryOwner(inv);
}

void HookInventoryUpdate(Inventory* inv,Item* item,int amount) {
  if(g_inventoryUpdateOrig) g_inventoryUpdateOrig(inv,item,amount);
  RefreshInventoryOwner(inv);
}

float OperateScale(Building* b, Character* who, PGP::ProfessionStat st) {
  if(!g_cfg.enabled || !g_jobOperateScaling || !who) return 1.0f;
  const float pct=EquippedBonus(who,st);
  return pct==0.0f ? 1.0f : 1.0f+pct/100.0f;
}

void RecordOperate(Building* b, Character* who, float amount, float scaled, float outputNow) {
  EnterCriticalSection(&g_lock);
  OperateStats& o=g_operateStats[b];
  if(!o.hasLast){ o.outputStart=outputNow; o.outputLast=outputNow; o.hasLast=true; try { o.name=b->getName(); } catch (...) {} }
  // getOutput() is the progress of the unit being made; when a unit completes it drops back.
  // Sum the forward progress so whole units and fractions both count.
  if(outputNow>=o.outputLast) o.outputProgress+=outputNow-o.outputLast;
  else o.outputProgress+=outputNow+(1.0f-o.outputLast>0.0f?1.0f-o.outputLast:0.0f);
  ++o.calls; o.amount+=amount; o.scaledAmount+=scaled; o.outputLast=outputNow;
  LeaveCriticalSection(&g_lock);
  if(who && o.worker.empty()){ try { o.worker=who->getName(); } catch (...) {} }
}

void HookProductionOperate(Building* b, Character* who, float amount) {
  const float scaled=amount*OperateScale(b,who,PGP::STAT_LABOURING);
  if(g_productionOperateOrig) g_productionOperateOrig(b,who,scaled);
  float out=0; try { ProductionBuilding* pb=dynamic_cast<ProductionBuilding*>(b); if(pb) out=pb->getOutput(); } catch (...) {}
  RecordOperate(b,who,amount,scaled,out);
}

float HookCritChance(CraftingBuilding* b, Character* smith) {
  const float vanilla=g_critChanceOrig?g_critChanceOrig(b,smith):0.0f;
  if(!g_forceCritical) return vanilla;
  std::ostringstream ss; ss<<"force critical: chance "<<vanilla<<" -> 1";
  try { if(smith) ss<<" smith="<<smith->getName(); if(b) ss<<" bench="<<b->getName(); } catch (...) {}
  Log(ss.str());
  return 1.0f;
}

void HookFarmOperate(Building* b, Character* who, float amount) {
  const float scaled=amount*OperateScale(b,who,PGP::STAT_FARMING);
  if(g_farmOperateOrig) g_farmOperateOrig(b,who,scaled);
  float out=0; try { ProductionBuilding* pb=dynamic_cast<ProductionBuilding*>(b); if(pb) out=pb->getOutput(); } catch (...) {}
  RecordOperate(b,who,amount,scaled,out);
}

// A purchase may hand the buyer a copy of the shop item instead of the same instance. The copy
// would be first seen on the player and roll again as world loot; give it the shop item's
// persistent ID (and drop the record the copy got during the purchase) so the affix the shop
// showed is the affix the buyer gets.
Item* HookBuyItem(Inventory* inv,Item* item,RootObject* sendingTo) {
  const std::string shopId=item?PersistentItemId(item,false):std::string();
  Item* bought=g_buyItemOrig?g_buyItemOrig(inv,item,sendingTo):0;
  if(!bought || bought==item || shopId.empty() || !g_cfg.enabled) return bought;
  try {
    const std::string newId=PersistentItemId(bought,false);
    if(newId==shopId) return bought;
    EnterCriticalSection(&g_lock);
    if(!newId.empty() && newId!=shopId) g_records.erase(newId);
    g_dirty=true;
    LeaveCriticalSection(&g_lock);
    BindPersistentItemId(bought,shopId);
    Character* buyer=dynamic_cast<Character*>(sendingTo);
    if(buyer) RebuildCharacterBonusCache(buyer);
    if(g_cfg.verboseLogging)
      Log(std::string("purchase: kept shop id=")+shopId+" for bought copy runtime="+RuntimeItemKey(bought)+
          (newId.empty()?"":" (dropped copy record "+newId+")"));
  } catch (...) {
    Log("exception keeping persistent id on purchase");
  }
  return bought;
}

void HookCraft(CraftingBuilding* b,Item* item) {
  Character* crafter=0;
  try { crafter=b?b->whosCrafting.getCharacter():0; } catch (...) {}
  Inventory* out=0;
  try { out=b?b->getInventory():0; } catch (...) {}
  std::set<Item*> before;
  if(out){
    std::vector<Item*> v;
    CollectCharacterInventoryItems(out,v);
    before.insert(v.begin(),v.end());
  }

  // Kenshi must finish the item first so ProfessionGear reads the final quality/model.
  if(g_craftOrig) g_craftOrig(b,item);

  // The item that reaches the bench output is not always the Item* passed in (live 2026-10-03:
  // the passed item rolled at tier 6 while the output held another instance of tier 2, which
  // was later picked up unbound and rolled as world loot). Roll every new item in the output,
  // with its final quality, as crafted by the bench's worker.
  int rolled=0;
  if(out){
    std::vector<Item*> after;
    CollectCharacterInventoryItems(out,after);
    for(size_t k=0;k<after.size();++k)
      if(after[k] && !before.count(after[k])){
        ++rolled;
        EnsureRecord(after[k],crafter,true);
      }
  }
  // Nothing new in the output (e.g. it was full): fall back to the passed item.
  if(!rolled && item) EnsureRecord(item,crafter,true);
  if(crafter) RebuildCharacterBonusCache(crafter);
  SaveDb();
}

void TipBase(InventoryItemBase* i,Ogre::vector<StringPair>::type& l){if(g_tipBaseOrig)g_tipBaseOrig(i,l);AppendTip(i,l);}
void TipArmour(InventoryItemBase* i,Ogre::vector<StringPair>::type& l){if(g_tipArmourOrig)g_tipArmourOrig(i,l);AppendTip(i,l);}
void TipContainer(InventoryItemBase* i,Ogre::vector<StringPair>::type& l){if(g_tipContainerOrig)g_tipContainerOrig(i,l);AppendTip(i,l);}
void TipCrossbow(InventoryItemBase* i,Ogre::vector<StringPair>::type& l){if(g_tipCrossbowOrig)g_tipCrossbowOrig(i,l);AppendTip(i,l);}
void TipSword(InventoryItemBase* i,Ogre::vector<StringPair>::type& l){if(g_tipSwordOrig)g_tipSwordOrig(i,l);AppendTip(i,l);}

bool HookSymbol(HMODULE lib,const char* sym,void* detour,void** orig) {
  void* thunk=(void*)GetProcAddress(lib,sym);
  if(!thunk){Log(std::string("missing symbol ")+sym);return false;}
  intptr_t real=KenshiLib::GetRealAddress(thunk);
  if(!real){Log(std::string("no real address ")+sym);return false;}
  bool ok=KenshiLib::AddHook((void*)real,detour,orig)==KenshiLib::SUCCESS;
  Log(std::string(ok?"hooked ":"hook failed ")+sym);
  return ok;
}

void InstallHooks() {
  HMODULE lib=GetModuleHandleA("KenshiLib.dll");
  if(!lib){Log("KenshiLib.dll not loaded");return;}
  HookSymbol(lib,"?update@PlayerInterface@@QEAAXXZ",(void*)HookPlayerUpdate,(void**)&g_playerUpdateOrig);
  HookSymbol(lib,"?getStat@CharStats@@QEBAMW4StatsEnumerated@@_N@Z",(void*)HookGetStat,(void**)&g_getStatOrig);
  HookSymbol(lib,"?addFinishedCraftItem@CraftingBuilding@@QEAAXPEAVItem@@@Z",(void*)HookCraft,(void**)&g_craftOrig);
  HookSymbol(lib,"?getTotalWeight@Inventory@@QEAAMXZ",(void*)HookInventoryWeight,(void**)&g_inventoryWeightOrig);
  HookSymbol(lib,"?_sectionAddItemCallback@Inventory@@UEAAXPEAVItem@@@Z",(void*)HookInventoryAdd,(void**)&g_inventoryAddOrig);
  HookSymbol(lib,"?_sectionRemoveItemCallback@Inventory@@UEAAXPEAVItem@@@Z",(void*)HookInventoryRemove,(void**)&g_inventoryRemoveOrig);
  HookSymbol(lib,"?_sectionUpdateItemCallback@Inventory@@UEAAXPEAVItem@@H@Z",(void*)HookInventoryUpdate,(void**)&g_inventoryUpdateOrig);
  HookSymbol(lib,"?serialiseInInventory@Item@@UEAAPEAVGameData@@PEAVGameDataContainer@@PEAV2@@Z",(void*)HookItemSerialiseInInventory,(void**)&g_itemSerialiseInventoryOrig);
  HookSymbol(lib,"?loadFromSerialiseInInventory@Item@@UEAAXPEAVGameDataContainer@@PEAVGameData@@@Z",(void*)HookItemLoadFromSerialiseInInventory,(void**)&g_itemLoadInventoryOrig);
  HookSymbol(lib,"?buyItem@Inventory@@QEAAPEAVItem@@PEAV2@PEAVRootObject@@@Z",(void*)HookBuyItem,(void**)&g_buyItemOrig);
  HookSymbol(lib,"?operate@ProductionBuilding@@UEAAXPEAVCharacter@@M@Z",(void*)HookProductionOperate,(void**)&g_productionOperateOrig);
  HookSymbol(lib,"?operate@FarmBuilding@@UEAAXPEAVCharacter@@M@Z",(void*)HookFarmOperate,(void**)&g_farmOperateOrig);
  HookSymbol(lib,"?calculateCriticalChance@CraftingBuilding@@QEAAMPEAVCharacter@@@Z",(void*)HookCritChance,(void**)&g_critChanceOrig);
  HookSymbol(lib,"?getTooltipData1@InventoryItemBase@@UEAAXAEAV?$vector@VStringPair@@V?$STLAllocator@VStringPair@@V?$CategorisedAllocPolicy@$0A@@Ogre@@@Ogre@@@std@@@Z",(void*)TipBase,(void**)&g_tipBaseOrig);
  HookSymbol(lib,"?getTooltipData1@Armour@@UEAAXAEAV?$vector@VStringPair@@V?$STLAllocator@VStringPair@@V?$CategorisedAllocPolicy@$0A@@Ogre@@@Ogre@@@std@@@Z",(void*)TipArmour,(void**)&g_tipArmourOrig);
  HookSymbol(lib,"?getTooltipData1@ContainerItem@@UEAAXAEAV?$vector@VStringPair@@V?$STLAllocator@VStringPair@@V?$CategorisedAllocPolicy@$0A@@Ogre@@@Ogre@@@std@@@Z",(void*)TipContainer,(void**)&g_tipContainerOrig);
  HookSymbol(lib,"?getTooltipData1@Crossbow@@UEAAXAEAV?$vector@VStringPair@@V?$STLAllocator@VStringPair@@V?$CategorisedAllocPolicy@$0A@@Ogre@@@Ogre@@@std@@@Z",(void*)TipCrossbow,(void**)&g_tipCrossbowOrig);
  HookSymbol(lib,"?getTooltipData1@Sword@@UEAAXAEAV?$vector@VStringPair@@V?$STLAllocator@VStringPair@@V?$CategorisedAllocPolicy@$0A@@Ogre@@@Ogre@@@std@@@Z",(void*)TipSword,(void**)&g_tipSwordOrig);
}

void LoadConfig() {
  std::ifstream f((g_dir+"\\ProfessionGear.ini").c_str());
  if(!f.is_open()) return;
  std::string line;
  while(std::getline(f,line)){
    line=PGP::Trim(line);
    if(line.empty()||line[0]=='#'||line[0]==';'||line[0]=='[') continue;
    size_t eq=line.find('='); if(eq==std::string::npos) continue;
    std::string k=PGP::Lower(PGP::Trim(line.substr(0,eq)));
    std::string v=PGP::Trim(line.substr(eq+1));
    if(k=="enabled") g_cfg.enabled=(v!="0"&&PGP::Lower(v)!="false");
    else if(k=="autoclassify") g_cfg.autoClassify=(v!="0"&&PGP::Lower(v)!="false");
    else if(k=="verboselogging") g_cfg.verboseLogging=(v!="0"&&PGP::Lower(v)!="false");
    else if(k=="globalchance") g_cfg.globalChance=(float)atof(v.c_str());
    else if(k=="npcrolemultiplier") g_cfg.npcRoleMultiplier=(float)atof(v.c_str());
    else if(k=="playercraftmultiplier") g_cfg.playerCraftMultiplier=(float)atof(v.c_str());
    else if(k=="poornpcmultiplier") g_cfg.poorNpcMultiplier=(float)atof(v.c_str());
    else if(k=="worldlootmultiplier") g_cfg.worldLootMultiplier=(float)atof(v.c_str());
    else if(k=="maxaffixes") g_cfg.maxAffixes=atoi(v.c_str());
    else if(k=="joboperatescaling") g_jobOperateScaling=(v!="0"&&PGP::Lower(v)!="false");
  }
  PGP::NormalizeConfig(g_cfg);
}

void LoadRules() {
  std::ifstream f((g_dir+"\\ProfessionGear.rules").c_str());
  if(!f.is_open()) return;
  std::string line;
  while(std::getline(f,line)){
    line=PGP::Trim(line);
    if(line.empty()||line[0]=='#'||line[0]==';') continue;
    size_t a=line.find('|');
    if(a==std::string::npos) continue;
    std::string op=PGP::Lower(PGP::Trim(line.substr(0,a)));
    size_t b=line.find('|',a+1);
    std::string id=PGP::Lower(PGP::Trim(line.substr(a+1,b==std::string::npos?std::string::npos:b-a-1)));
    if(id.empty()) continue;
    if(op=="exclude") { g_exclusions.insert(id); continue; }
    if(op!="tag" || b==std::string::npos) continue;
    std::string rest=line.substr(b+1);
    std::vector<PGP::ItemTag> tags;
    size_t p=0;
    while(p<=rest.size()){
      size_t c=rest.find(',',p);
      std::string tok=rest.substr(p,c==std::string::npos?std::string::npos:c-p);
      PGP::ItemTag t=PGP::ParseTag(tok);
      if(t!=PGP::TAG_NONE) tags.push_back(t);
      if(c==std::string::npos) break;
      p=c+1;
    }
    if(!tags.empty()) g_overrides[id]=tags;
  }
}

} // anonymous

__declspec(dllexport) void startPlugin() {
  if(g_started) return;
  g_started=true;
  InitializeCriticalSection(&g_lock);
  char path[MAX_PATH]={0};
  GetModuleFileNameA((HMODULE)&__ImageBase,path,MAX_PATH);
  g_dir=DirName(path);
  g_dbPath=g_dir+"\\profession_gear_affixes.tsv";
  g_logPath=g_dir+"\\ProfessionGear.log";
  DeleteFileA(g_logPath.c_str());
  Log("Profession Gear Progression 0.9.1-pretest starting");
  LoadConfig();
  LoadRules();
  {
    std::ostringstream ss;
    ss<<"config enabled="<<(g_cfg.enabled?1:0)
      <<" autoClassify="<<(g_cfg.autoClassify?1:0)
      <<" verboseLogging="<<(g_cfg.verboseLogging?1:0)
      <<" globalChance="<<g_cfg.globalChance
      <<" npcRoleMultiplier="<<g_cfg.npcRoleMultiplier
      <<" playerCraftMultiplier="<<g_cfg.playerCraftMultiplier
      <<" poorNpcMultiplier="<<g_cfg.poorNpcMultiplier
      <<" worldLootMultiplier="<<g_cfg.worldLootMultiplier
      <<" maxAffixes="<<g_cfg.maxAffixes
      <<" jobOperateScaling="<<(g_jobOperateScaling?1:0)
      <<" overrides="<<(unsigned long)g_overrides.size()
      <<" exclusions="<<(unsigned long)g_exclusions.size();
    Log(ss.str());
  }
  LoadDb();
  InstallHooks();
}

BOOL APIENTRY DllMain(HMODULE,DWORD,LPVOID){return TRUE;}
