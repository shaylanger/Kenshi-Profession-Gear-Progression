
#include "ProfessionGearCore.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

static int fails=0;
static int passes=0;
static void Check(bool v,const char* name){
  if(!v){++fails;std::cerr<<"FAIL: "<<name<<"\n";}
  else ++passes;
}
static bool Eq(float a,float b,float e=.001f){return std::fabs(a-b)<=e;}

static bool HasTag(const std::vector<PGP::ItemTag>& v,PGP::ItemTag t){
  return std::find(v.begin(),v.end(),t)!=v.end();
}
static bool HasStat(const std::vector<PGP::ProfessionStat>& v,PGP::ProfessionStat t){
  return std::find(v.begin(),v.end(),t)!=v.end();
}

int main(){
  using namespace PGP;

  Check(Lower("AbC")=="abc","lower");
  Check(Trim("  x \r\n")=="x","trim");
  Check(ParseStat("Farming")==STAT_FARMING,"parse farming");
  Check(ParseStat("armor smithing")==STAT_ARMOUR_SMITH,"parse armour alias");
  Check(ParseStat("nope")==STAT_NONE,"parse unknown stat");
  Check(ParseTag("tool_mining")==TAG_TOOL_MINING,"parse mining tag");
  Check(ParseTag("PACK_TECH")==TAG_PACK_TECH,"parse pack tag");
  Check(ParseTag("PACK_GENERIC")==TAG_PACK_GENERIC,"parse generic pack tag");
  Check(ParseTag("ASSASSIN_GEAR")==TAG_ASSASSIN_GEAR,"parse assassin tag");
  Check(ParseTag("garbage")==TAG_NONE,"parse unknown tag");

  Check(QualityTier(.0f)==0,"tier 0");
  Check(QualityTier(.149f)==0,"tier 0 upper");
  Check(QualityTier(.15f)==1,"tier 1 lower");
  Check(QualityTier(.299f)==1,"tier 1 upper");
  Check(QualityTier(.30f)==2,"tier 2 lower");
  Check(QualityTier(.479f)==2,"tier 2 upper");
  Check(QualityTier(.48f)==3,"tier 3 lower");
  Check(QualityTier(.659f)==3,"tier 3 upper");
  Check(QualityTier(.66f)==4,"tier 4 lower");
  Check(QualityTier(.819f)==4,"tier 4 upper");
  Check(QualityTier(.82f)==5,"tier 5 lower");
  Check(QualityTier(.939f)==5,"tier 5 upper");
  Check(QualityTier(.94f)==6,"tier 6 lower");
  Check(QualityTier(5.0f)==6,"tier 6 clamp high");

  Check(TierAffixCap(0)==1 && TierAffixCap(1)==1 && TierAffixCap(2)==1,"low tiers cap one affix");
  Check(TierAffixCap(3)==2 && TierAffixCap(4)==2,"medium tiers cap two affixes");
  Check(TierAffixCap(5)==3 && TierAffixCap(6)==3,"top tiers cap three affixes");
  Check(TierAffixCap(-9)==1 && TierAffixCap(99)==3,"affix cap clamps tier");

  RuleConfig badCfg;
  badCfg.globalChance=-2; badCfg.npcRoleMultiplier=-1; badCfg.playerCraftMultiplier=-1;
  badCfg.poorNpcMultiplier=-1; badCfg.worldLootMultiplier=-1; badCfg.maxAffixes=99;
  NormalizeConfig(badCfg);
  Check(Eq(badCfg.globalChance,0)&&Eq(badCfg.npcRoleMultiplier,0)&&Eq(badCfg.playerCraftMultiplier,0),"config negative multipliers clamped");
  Check(Eq(badCfg.poorNpcMultiplier,0)&&Eq(badCfg.worldLootMultiplier,0)&&badCfg.maxAffixes==3,"config world/poor/max clamped");
  badCfg.maxAffixes=0; NormalizeConfig(badCfg); Check(badCfg.maxAffixes==1,"config min affix clamp");

  float lastChance=-1;
  for(int t=0;t<=6;++t){
    float lo=0,hi=0; TierRange(t,lo,hi);
    Check(lo>0 && hi>=lo,"tier range valid");
    float chance=TierAffixChance(t);
    Check(chance>=lastChance,"tier chance nondecreasing");
    lastChance=chance;
  }
  float lo=0,hi=0; TierRange(-99,lo,hi); Check(Eq(lo,2)&&Eq(hi,4),"tier range clamp low");
  TierRange(99,lo,hi); Check(Eq(lo,16)&&Eq(hi,25),"tier range clamp high");

  std::map<std::string,std::vector<ItemTag> > overrides;
  std::set<std::string> exclusions;
  ItemDescriptor item; item.quality=.5f;

  item.baseId="hoe"; item.name="Iron Hoe"; item.category="tool";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_FARMING),"classify hoe");
  item.baseId="pick"; item.name="Heavy Pickaxe";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_MINING),"classify pickaxe");
  item.baseId="lab"; item.name="Research Lab Coat";
  {std::vector<ItemTag> x=Classify(item,overrides,exclusions);Check(HasTag(x,TAG_BODY_RESEARCH),"classify lab coat body");Check(HasTag(x,TAG_TOOL_RESEARCH),"classify lab coat research");}
  item.baseId="eng"; item.name="Engineer Tool Belt";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_ENGINEERING),"classify engineer");
  item.baseId="robot"; item.name="Robotics Tool";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_ROBOTICS),"classify robotics");
  item.baseId="med"; item.name="Field Medic Pack";
  {std::vector<ItemTag> x=Classify(item,overrides,exclusions);Check(HasTag(x,TAG_TOOL_MEDIC),"classify medic");Check(HasTag(x,TAG_PACK_MEDICAL),"classify medic pack");}
  item.baseId="ws"; item.name="Weapon Smith Hammer";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_WEAPON_SMITH),"classify weapon smith");
  item.baseId="as"; item.name="Armour Smith Hammer";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_ARMOUR_SMITH),"classify armour smith");
  item.baseId="cs"; item.name="Crossbow Smith Tools";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_CROSSBOW_SMITH),"classify crossbow smith");
  item.baseId="cook"; item.name="Chef Cooking Tool";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TOOL_COOKING),"classify cooking");
  item.baseId="farmhat"; item.name="Straw Hat";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_HEAD_FARMING),"classify straw hat");
  item.baseId="minerhat"; item.name="Miner Helmet";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_HEAD_MINING),"classify miner helmet");
  item.baseId="workgloves"; item.name="Industrial Work Gloves";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_GLOVES_WORK),"classify work gloves");
  item.baseId="workboots"; item.name="Industrial Work Boots";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_BOOTS_WORK),"classify work boots");
  item.baseId="travelboots"; item.name="Scout Runner Boots";
  {std::vector<ItemTag> x=Classify(item,overrides,exclusions);Check(HasTag(x,TAG_BOOTS_TRAVEL),"classify travel boots");Check(HasTag(x,TAG_SCOUT_GEAR),"classify scout boots");}
  item.baseId="orepack"; item.name="Mining Ore Pack";
  {std::vector<ItemTag> x=Classify(item,overrides,exclusions);Check(HasTag(x,TAG_PACK_ORE),"classify ore pack");}
  item.baseId="croppack"; item.name="Farm Crop Pack";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_PACK_CROP),"classify crop pack");
  item.baseId="buildpack"; item.name="Builder Construction Pack";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_PACK_CONSTRUCTION),"classify construction pack");
  item.baseId="techpack"; item.name="Research Tech Pack";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_PACK_TECH),"classify tech pack");
  item.baseId="tradepack"; item.name="Caravan Trade Pack";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_PACK_TRADE),"classify trade pack");
  item.baseId="turret"; item.name="Turret Gunner Visor";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_TURRET_GEAR),"classify turret gear");
  item.baseId="stealth"; item.name="Stealth Infiltrator Hood";
  Check(HasTag(Classify(item,overrides,exclusions),TAG_STEALTH_GEAR),"classify stealth gear");
  item.baseId="sword"; item.name="Katana"; item.category="weapon";
  Check(Classify(item,overrides,exclusions).empty(),"ordinary sword no profession class");

  ItemDescriptor greenHat; greenHat.baseId="mod.green_hat"; greenHat.name="Green Hat"; greenHat.slot="head"; greenHat.armour=true;
  {std::vector<ItemTag> x=Classify(greenHat,overrides,exclusions);Check(HasTag(x,TAG_WORKWEAR_GENERIC),"plain modded hat becomes generic workwear");}
  {std::vector<ProfessionStat> x=AllowedStats(Classify(greenHat,overrides,exclusions));Check(HasStat(x,STAT_FARMING)&&HasStat(x,STAT_LABOURING)&&HasStat(x,STAT_ENGINEERING)&&HasStat(x,STAT_COOKING),"generic workwear has contextual profession pool");}

  ItemDescriptor rags; rags.baseId="mod.rags"; rags.name="Worker Rags"; rags.armour=true;
  {std::vector<ItemTag> x=Classify(rags,overrides,exclusions);Check(HasTag(x,TAG_WORKWEAR_GENERIC),"worker rags generic workwear");}

  ItemDescriptor goggles; goggles.baseId="mod.goggles"; goggles.name="Green-Tint Goggles"; goggles.slot="head"; goggles.armour=true;
  {std::vector<ItemTag> x=Classify(goggles,overrides,exclusions);Check(HasTag(x,TAG_GOGGLES_GENERIC),"plain modded goggles generic precision");}
  {std::vector<ProfessionStat> x=AllowedStats(Classify(goggles,overrides,exclusions));Check(HasStat(x,STAT_PERCEPTION)&&HasStat(x,STAT_SCIENCE)&&HasStat(x,STAT_ENGINEERING)&&HasStat(x,STAT_ROBOTICS)&&HasStat(x,STAT_TURRETS),"goggles multi-context pool");}

  ItemDescriptor shoes; shoes.baseId="mod.running_shoes"; shoes.name="Running Shoes"; shoes.slot="boots"; shoes.armour=true;
  {std::vector<ItemTag> x=Classify(shoes,overrides,exclusions);Check(HasTag(x,TAG_BOOTS_TRAVEL),"running shoes infer athletics context from name");}

  ItemDescriptor combat; combat.baseId="mod.farmer_sword"; combat.name="Farmer's Sword"; combat.weapon=true; combat.weaponLevel=50;
  {std::vector<ItemTag> x=Classify(combat,overrides,exclusions);Check(HasTag(x,TAG_TOOL_FARMING),"farmer named weapon can carry farming context");}
  ItemDescriptor farmersSword=combat; farmersSword.baseId="mod.farmers_sword"; farmersSword.name="Farmers Sword";
  {std::vector<ItemTag> x=Classify(farmersSword,overrides,exclusions);Check(HasTag(x,TAG_TOOL_FARMING),"farmers plural weapon carries farming context");}

  ItemDescriptor pitchfork; pitchfork.baseId="mod.pitchfork"; pitchfork.name="Rusty Pitchfork"; pitchfork.weapon=true; pitchfork.weaponLevel=25;
  {std::vector<ItemTag> x=Classify(pitchfork,overrides,exclusions);Check(HasTag(x,TAG_TOOL_FARMING),"pitchfork weapon class gets farming context");}

  ItemDescriptor toolWeapon; toolWeapon.baseId="mod.pickaxe"; toolWeapon.name="Industrial Pickaxe"; toolWeapon.weapon=true; toolWeapon.weaponLevel=40;
  {std::vector<ItemTag> x=Classify(toolWeapon,overrides,exclusions);Check(HasTag(x,TAG_TOOL_MINING),"tool-like weapon allowed by explicit semantics");}

  ItemDescriptor legendary=toolWeapon; legendary.baseId="legendary"; legendary.legendary=true; legendary.weaponLevel=100;
  Check(Classify(legendary,overrides,exclusions).empty(),"legendary item classification blocked");

  ItemDescriptor uniqueClothing; uniqueClothing.baseId="unique.rags"; uniqueClothing.name="Hero's Work Rags"; uniqueClothing.armour=true; uniqueClothing.legendary=true;
  Check(Classify(uniqueClothing,overrides,exclusions).empty(),"explicitly unique non-weapon item classification blocked");

  Check(WeaponGradeRank(5)==0,"weapon rank rusted junk");
  Check(WeaponGradeRank(10)==1,"weapon rank rusting blade");
  Check(WeaponGradeRank(40)==7,"weapon rank catun3");
  Check(WeaponGradeRank(60)==10,"weapon rank mk3");
  Check(WeaponGradeRank(70)==11,"weapon rank edge1");
  Check(WeaponGradeRank(80)==13,"weapon rank edge3");
  Check(WeaponGradeRank(100)==14,"weapon rank meitou");

  ItemDescriptor wg; wg.weapon=true; wg.weaponLevel=5;
  Check(ProgressionTier(wg)==0,"weapon rusted affix tier");
  wg.weaponLevel=40; Check(ProgressionTier(wg)==3,"weapon catun3 affix tier");
  wg.weaponLevel=60; Check(ProgressionTier(wg)==4,"weapon mk3 affix tier");
  wg.weaponLevel=75; Check(ProgressionTier(wg)==5,"weapon edge2 affix tier");
  wg.weaponLevel=80; Check(ProgressionTier(wg)==6,"weapon edge3 affix tier");

  item.baseId="special"; item.name="Katana";
  overrides["special"].push_back(TAG_TOOL_FARMING);
  {std::vector<ItemTag> x=Classify(item,overrides,exclusions);Check(x.size()==1&&x[0]==TAG_TOOL_FARMING,"override precedence");}
  exclusions.insert("special");
  Check(Classify(item,overrides,exclusions).empty(),"exclusion precedence");
  overrides.clear(); exclusions.clear();

  std::vector<ItemTag> tags;
  tags.push_back(TAG_TOOL_FARMING);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(s.size()==1&&s[0]==STAT_FARMING,"farming stat pool");}
  tags.clear();tags.push_back(TAG_TOOL_RESEARCH);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_SCIENCE)&&HasStat(s,STAT_ROBOTICS)&&!HasStat(s,STAT_FARMING),"research stat pool");}
  tags.clear();tags.push_back(TAG_TOOL_ENGINEERING);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_ENGINEERING)&&HasStat(s,STAT_LABOURING),"engineering stat pool");}
  tags.clear();tags.push_back(TAG_BODY_SMITH);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_ARMOUR_SMITH)&&HasStat(s,STAT_WEAPON_SMITH),"smith stat pool");}
  tags.clear();tags.push_back(TAG_BOOTS_TRAVEL);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_ATHLETICS)&&HasStat(s,STAT_PERCEPTION),"travel stat pool");}
  tags.clear();tags.push_back(TAG_STEALTH_GEAR);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_STEALTH)&&HasStat(s,STAT_LOCKPICKING),"stealth stat pool");}
  tags.clear();tags.push_back(TAG_ASSASSIN_GEAR);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_STEALTH)&&HasStat(s,STAT_ASSASSINATION)&&!HasStat(s,STAT_FARMING),"assassin stat pool");}
  tags.clear();tags.push_back(TAG_THIEF_GEAR);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_STEALTH)&&HasStat(s,STAT_LOCKPICKING)&&HasStat(s,STAT_THIEVERY),"thief stat pool");}
  tags.clear();tags.push_back(TAG_SWIM_GEAR);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(s.size()==1&&s[0]==STAT_SWIMMING,"swim stat pool");}
  tags.clear();tags.push_back(TAG_PACK_HAULING);
  {std::vector<ProfessionStat> s=AllowedStats(tags);Check(HasStat(s,STAT_ATHLETICS),"hauling pack utility pool");}

  ItemDescriptor assassin; assassin.baseId="vanilla.assassin_rags"; assassin.name="Assassin's Rags"; assassin.armour=true;
  {std::vector<ItemTag> x=Classify(assassin,overrides,exclusions);Check(HasTag(x,TAG_ASSASSIN_GEAR),"classify assassin rags");}
  ItemDescriptor ninja; ninja.baseId="vanilla.ninja_rags"; ninja.name="Ninja Rags"; ninja.armour=true;
  {std::vector<ItemTag> x=Classify(ninja,overrides,exclusions);Check(HasTag(x,TAG_STEALTH_GEAR),"classify ninja rags stealth");}
  ItemDescriptor thief; thief.baseId="mod.thief_coat"; thief.name="Thief's Coat"; thief.armour=true;
  {std::vector<ItemTag> x=Classify(thief,overrides,exclusions);Check(HasTag(x,TAG_THIEF_GEAR),"classify thief gear");}
  ItemDescriptor swim; swim.baseId="mod.diving_suit"; swim.name="Diving Suit"; swim.armour=true;
  {std::vector<ItemTag> x=Classify(swim,overrides,exclusions);Check(HasTag(x,TAG_SWIM_GEAR),"classify swimming gear");}
  ItemDescriptor cargo; cargo.baseId="mod.cargo_frame"; cargo.name="Nomad Cargo Frame"; cargo.container=true;
  {std::vector<ItemTag> x=Classify(cargo,overrides,exclusions);Check(HasTag(x,TAG_PACK_HAULING),"classify hauling cargo frame");}

  ItemDescriptor genericPack; genericPack.baseId="vanilla.medium_backpack"; genericPack.name="Medium Backpack"; genericPack.container=true;
  {std::vector<ItemTag> x=Classify(genericPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_GENERIC)&&!HasTag(x,TAG_PACK_HAULING),"generic backpack gets utility but not specialist hauling");}
  ItemDescriptor minersPack; minersPack.baseId="mod.miners_backpack"; minersPack.name="Miners Backpack"; minersPack.container=true;
  {std::vector<ItemTag> x=Classify(minersPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_ORE)&&!HasTag(x,TAG_PACK_GENERIC),"miners backpack becomes ore specialist");}
  ItemDescriptor farmersPack; farmersPack.baseId="mod.farmers_backpack"; farmersPack.name="Farmers Backpack"; farmersPack.container=true;
  {std::vector<ItemTag> x=Classify(farmersPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_CROP)&&!HasTag(x,TAG_PACK_GENERIC),"farmers backpack becomes crop specialist");}
  ItemDescriptor medicPack; medicPack.baseId="mod.field_medic_backpack"; medicPack.name="Field Medic Backpack"; medicPack.container=true;
  {std::vector<ItemTag> x=Classify(medicPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_MEDICAL)&&!HasTag(x,TAG_PACK_GENERIC),"medic backpack becomes medical specialist");}
  ItemDescriptor engineerPack; engineerPack.baseId="mod.engineer_pack"; engineerPack.name="Engineer's Pack"; engineerPack.container=true;
  {std::vector<ItemTag> x=Classify(engineerPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_CONSTRUCTION)&&!HasTag(x,TAG_PACK_GENERIC),"engineer pack becomes construction specialist");}
  ItemDescriptor researchPack; researchPack.baseId="mod.research_satchel"; researchPack.name="Researcher's Satchel"; researchPack.container=true;
  {std::vector<ItemTag> x=Classify(researchPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_TECH)&&!HasTag(x,TAG_PACK_GENERIC),"research satchel becomes tech specialist");}
  ItemDescriptor traderPack; traderPack.baseId="vanilla.trader_backpack"; traderPack.name="Trader's Wooden Backpack"; traderPack.container=true;
  {std::vector<ItemTag> x=Classify(traderPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_TRADE)&&!HasTag(x,TAG_PACK_GENERIC),"trader backpack gets trade specialization");}
  ItemDescriptor thievesPack; thievesPack.baseId="vanilla.small_thieves_backpack"; thievesPack.name="Small Thieves Backpack"; thievesPack.container=true;
  {std::vector<ItemTag> x=Classify(thievesPack,overrides,exclusions);Check(HasTag(x,TAG_THIEF_GEAR)&&HasTag(x,TAG_PACK_GENERIC),"vanilla thieves backpack combines thief and generic pack utility");}
  ItemDescriptor sandals; sandals.baseId="vanilla.wooden_sandals"; sandals.name="Wooden Sandals"; sandals.armour=true; sandals.slot="boots";
  {std::vector<ItemTag> x=Classify(sandals,overrides,exclusions);Check(HasTag(x,TAG_BOOTS_TRAVEL),"wooden sandals travel utility");}
  ItemDescriptor doctorMask; doctorMask.baseId="uwe.plague_doctor"; doctorMask.name="Plague Doctor Mask"; doctorMask.armour=true;
  {std::vector<ItemTag> x=Classify(doctorMask,overrides,exclusions);Check(HasTag(x,TAG_TOOL_MEDIC)&&!HasTag(x,TAG_WORKWEAR_GENERIC),"doctor mask stays medic-specific");}
  ItemDescriptor assassinRags2; assassinRags2.baseId="uwe.assassin_rags"; assassinRags2.name="Assassin's Rags"; assassinRags2.armour=true;
  {std::vector<ItemTag> x=Classify(assassinRags2,overrides,exclusions);Check(HasTag(x,TAG_ASSASSIN_GEAR)&&!HasTag(x,TAG_WORKWEAR_GENERIC),"assassin rags not generic workwear");}
  ItemDescriptor mechanicalBlade; mechanicalBlade.baseId="uwe.mechanical_blade"; mechanicalBlade.name="Mechanical Blade"; mechanicalBlade.weapon=true; mechanicalBlade.weaponLevel=50;
  Check(Classify(mechanicalBlade,overrides,exclusions).empty(),"mechanical weapon does not false-match mechanic profession");
  ItemDescriptor assassinBlade; assassinBlade.baseId="mod.assassin_blade"; assassinBlade.name="Assassin's Blade"; assassinBlade.weapon=true; assassinBlade.weaponLevel=50;
  {std::vector<ItemTag> x=Classify(assassinBlade,overrides,exclusions);Check(HasTag(x,TAG_ASSASSIN_GEAR),"assassin named weapon allowed utility role");}
  ItemDescriptor thiefDagger; thiefDagger.baseId="mod.thief_dagger"; thiefDagger.name="Thief's Dagger"; thiefDagger.weapon=true; thiefDagger.weaponLevel=40;
  {std::vector<ItemTag> x=Classify(thiefDagger,overrides,exclusions);Check(HasTag(x,TAG_THIEF_GEAR),"thief named weapon allowed utility role");}
  ItemDescriptor scoutSword; scoutSword.baseId="mod.scout_sword"; scoutSword.name="Scout Sword"; scoutSword.weapon=true; scoutSword.weaponLevel=40;
  {std::vector<ItemTag> x=Classify(scoutSword,overrides,exclusions);Check(HasTag(x,TAG_SCOUT_GEAR),"scout named weapon allowed utility role");}
  ItemDescriptor runningShoes; runningShoes.baseId="mod.running_shoes_2"; runningShoes.name="Running Shoes"; runningShoes.armour=true; runningShoes.slot="boots";
  {std::vector<ItemTag> x=Classify(runningShoes,overrides,exclusions);Check(HasTag(x,TAG_BOOTS_TRAVEL)&&!HasTag(x,TAG_TOOL_FARMING),"shoes do not false-match hoe");}
  ItemDescriptor dragonArmour; dragonArmour.baseId="mod.dragon_armour"; dragonArmour.name="Dragon Armour"; dragonArmour.armour=true;
  Check(!HasTag(Classify(dragonArmour,overrides,exclusions),TAG_WORKWEAR_GENERIC),"dragon does not false-match rag");
  ItemDescriptor mineralMask; mineralMask.baseId="mod.mineral_mask"; mineralMask.name="Mineral Dust Mask"; mineralMask.armour=true;
  Check(!HasTag(Classify(mineralMask,overrides,exclusions),TAG_TOOL_MINING),"mineral does not false-match miner");

  // Live false positive (2026-10-02): rebirth.mod "Staff" (52302) became TOOL_FARMING because its
  // description says "Popular with poor farmers". Weapon descriptions are flavour; names decide.
  ItemDescriptor staff; staff.baseId="52302-rebirth.mod"; staff.name="Staff"; staff.weapon=true; staff.weaponLevel=20;
  staff.description="-Length 23\nPopular with poor farmers, pacifists and drifters.  It's usually seen as a weapon for those who can't afford a blade.";
  Check(Classify(staff,overrides,exclusions).empty(),"staff with farmer flavour description gets no profession tag");
  ItemDescriptor bardiche; bardiche.baseId="55394-ArkWeaponPack.mod"; bardiche.name="Bardiche"; bardiche.weapon=true; bardiche.weaponLevel=30;
  bardiche.description="Good weapon for guards, either to slay thieves or to take on armoured opponents.";
  Check(Classify(bardiche,overrides,exclusions).empty(),"bardiche 'slay thieves' description is not thief gear");
  ItemDescriptor hatchet; hatchet.baseId="2757887-WeaponExpansion_Ronin.mod"; hatchet.name="Ronin Hatchet"; hatchet.weapon=true; hatchet.weaponLevel=30;
  hatchet.description="Drawing inspiration from some of the more commonly found robotic tools.";
  Check(Classify(hatchet,overrides,exclusions).empty(),"weapon description mentioning robotics is not a robotics tool");
  ItemDescriptor scythe; scythe.baseId="55416-ArkWeaponPack.mod"; scythe.name="Scythe"; scythe.weapon=true; scythe.weaponLevel=30;
  scythe.description="Harvesting tool, might work on people.";
  {std::vector<ItemTag> x=Classify(scythe,overrides,exclusions);Check(HasTag(x,TAG_TOOL_FARMING),"scythe weapon is a farming tool by name");}
  ItemDescriptor farmerSwordDesc; farmerSwordDesc.baseId="mod.farmers_sword_desc"; farmerSwordDesc.name="Farmer's Sword"; farmerSwordDesc.weapon=true; farmerSwordDesc.weaponLevel=40;
  farmerSwordDesc.description="An ordinary blade.";
  {std::vector<ItemTag> x=Classify(farmerSwordDesc,overrides,exclusions);Check(HasTag(x,TAG_TOOL_FARMING),"farmer-named weapon still farming with plain description");}
  ItemDescriptor doctorCoat; doctorCoat.baseId="mod.coat"; doctorCoat.name="Long Coat"; doctorCoat.armour=true;
  doctorCoat.description="Favoured by field medics.";
  {std::vector<ItemTag> x=Classify(doctorCoat,overrides,exclusions);Check(HasTag(x,TAG_TOOL_MEDIC),"armour description still classifies (non-weapon)");}
  ItemDescriptor tradersPack; tradersPack.baseId="1498-gamedata.base"; tradersPack.name="Traders Backpack Medium"; tradersPack.container=true;
  {std::vector<ItemTag> x=Classify(tradersPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_TRADE)&&!HasTag(x,TAG_PACK_GENERIC),"vanilla Traders Backpack is a trade pack");}
  ItemDescriptor oldTradersPack; oldTradersPack.baseId="1019-gamedata.base"; oldTradersPack.name="Old Traders backpack small"; oldTradersPack.container=true;
  {std::vector<ItemTag> x=Classify(oldTradersPack,overrides,exclusions);Check(HasTag(x,TAG_PACK_TRADE),"vanilla Old Traders backpack is a trade pack");}

  Check(Eq(WealthFromBestSkill(0),0)&&Eq(WealthFromBestSkill(-5),0),"no skill is poor");
  Check(WealthFromBestSkill(5)<.15f&&WealthFromBestSkill(11.9f)<.15f,"very low skill counts as poor");
  Check(WealthFromBestSkill(12)>=.15f&&WealthFromBestSkill(40)>=.15f,"ordinary skill is not poor");
  Check(Eq(WealthFromBestSkill(80),1)&&Eq(WealthFromBestSkill(100),1),"skill 80+ is full wealth");
  {
    ItemDescriptor hat; hat.baseId="poor.hat"; hat.name="Worker Rags"; hat.quality=.5f; hat.armour=true;
    std::vector<ItemTag> ht=Classify(hat,overrides,exclusions);
    RuleConfig half; half.globalChance=1.0f; half.poorNpcMultiplier=0.0f;
    RoleProfile poorNpc; poorNpc.primary=STAT_FARMING; poorNpc.wealth01=WealthFromBestSkill(5);
    int poorRolls=0;
    for(int i=0;i<200;++i){ std::ostringstream k; k<<"poor-"<<i; if(!RollAffixes(hat,poorNpc,half,ht,k.str(),(unsigned)i,false).affixes.empty()) ++poorRolls; }
    Check(poorRolls==0,"very low-skill NPC gear is suppressed by PoorNpcMultiplier");
  }
  {
    // Tests 102-104 as rates over many instances with the shipped multipliers (GlobalChance 1).
    ItemDescriptor hoe2; hoe2.baseId="rate.hoe"; hoe2.name="Iron Hoe"; hoe2.quality=.5f;
    std::vector<ItemTag> ft; ft.push_back(TAG_TOOL_FARMING);
    RuleConfig shipped;  // defaults: npcRole 1.35, poor 0.20
    RoleProfile specialist; specialist.primary=STAT_FARMING; specialist.wealth01=.7f;
    RoleProfile other; other.primary=STAT_MEDIC; other.wealth01=.7f;
    RoleProfile slave=other; slave.slave=true;
    RoleProfile poor=other; poor.wealth01=WealthFromBestSkill(4);
    int nSpec=0,nOther=0,nSlave=0,nPoor=0; const int N=4000;
    for(int i=0;i<N;++i){
      std::ostringstream k; k<<"rate-"<<i;
      if(!RollAffixes(hoe2,specialist,shipped,ft,k.str(),(unsigned)i,false).affixes.empty()) ++nSpec;
      if(!RollAffixes(hoe2,other,shipped,ft,k.str(),(unsigned)i,false).affixes.empty()) ++nOther;
      if(!RollAffixes(hoe2,slave,shipped,ft,k.str(),(unsigned)i,false).affixes.empty()) ++nSlave;
      if(!RollAffixes(hoe2,poor,shipped,ft,k.str(),(unsigned)i,false).affixes.empty()) ++nPoor;
    }
    // tier 3 chance .60: specialist .81 (x1.35), other .60, slave/poor .12 (x0.20)
    Check(nSpec>nOther*1.2,"104 matching specialist rolls more often than an unrelated NPC");
    Check(nSlave<nOther*0.3,"102 slave gear is strongly suppressed");
    Check(nPoor<nOther*0.3,"103 very low-skill NPC gear is strongly suppressed");
    Check(nOther>N*0.5 && nOther<N*0.7,"unrelated NPC rolls near the tier chance");
  }

  ItemDescriptor tobacco; tobacco.baseId="live.chewing_tobacco"; tobacco.name="Chewing Tobacco"; tobacco.description="Workers may chew these rags of tobacco"; tobacco.equippable=false;
  {std::vector<ItemTag> x=Classify(tobacco,overrides,exclusions); AffixRecord rr=RollAffixes(tobacco,RoleProfile(),RuleConfig(),x,"tobacco",7,false); Check(rr.affixes.empty(),"non-equippable tobacco never rolls profession affix");}
  ItemDescriptor bolts; bolts.baseId="live.bolts"; bolts.name="Bolts [Regulars]"; bolts.description="Regular crossbow ammunition"; bolts.equippable=false;
  {std::vector<ItemTag> x=Classify(bolts,overrides,exclusions); AffixRecord rr=RollAffixes(bolts,RoleProfile(),RuleConfig(),x,"bolts",7,false); Check(rr.affixes.empty(),"non-equippable bolts never roll profession affix");}
  ItemDescriptor firstAid; firstAid.baseId="209-gamedata.base"; firstAid.name="Basic First Aid Kit"; firstAid.equippable=false;
  {std::vector<ItemTag> x=Classify(firstAid,overrides,exclusions); AffixRecord rr=RollAffixes(firstAid,RoleProfile(),RuleConfig(),x,"aid",7,false); Check(rr.affixes.empty(),"non-equippable first aid kit never rolls profession affix");}
  ItemDescriptor medicalSupplies; medicalSupplies.baseId="582-gamedata.base"; medicalSupplies.name="Medical Supplies"; medicalSupplies.equippable=false;
  {std::vector<ItemTag> x=Classify(medicalSupplies,overrides,exclusions); AffixRecord rr=RollAffixes(medicalSupplies,RoleProfile(),RuleConfig(),x,"medsup",7,false); Check(rr.affixes.empty(),"non-equippable medical supplies never roll profession affix");}

  Check(Eq(EffectiveStatValue(50,20,false,150),60),"effective stat percent");
  Check(EffectiveStatValue(50,20,true,150)==50,"unmodified bypass");
  Check(EffectiveStatValue(140,20,false,150)==150,"effective stat cap");
  Check(EffectiveStatValue(10,-200,false,150)==0,"effective stat floor");
  Check(Eq(EffectiveStatValue(50,20,false,0),60),"effective stat no cap");

  tags.clear();tags.push_back(TAG_PACK_ORE);
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Raw Iron","raw_iron",false),.25f),"ore pack raw iron");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Copper Ore","copper_ore",false),.25f),"ore pack copper");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Bread","bread",false),1.0f),"ore pack unrelated");
  tags.clear();tags.push_back(TAG_PACK_CROP);
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Wheatstraw","wheat",false),.30f),"crop pack wheat");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Cactus","cactus",false),.30f),"crop pack cactus");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Iron Plate","plate",false),1.0f),"crop pack unrelated");
  tags.clear();tags.push_back(TAG_PACK_CONSTRUCTION);
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Building Materials","bm",false),.35f),"construction pack materials");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Iron Plate","plate",false),.35f),"construction pack plates");
  tags.clear();tags.push_back(TAG_PACK_MEDICAL);
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Advanced First Aid Kit","med",false),.35f),"medical pack aid");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Splint Kit","splint",false),.35f),"medical pack splint");
  tags.clear();tags.push_back(TAG_PACK_TECH);
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"AI Core","ai_core",false),.35f),"tech pack ai core");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Ancient Science Book","book",false),.35f),"tech pack book");
  tags.clear();tags.push_back(TAG_PACK_TRADE);
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Luxury Goods","lux",true),.55f),"trade pack trade item");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Luxury Goods","lux",false),1.0f),"trade pack nontrade");
  tags.clear();tags.push_back(TAG_PACK_HAULING);
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Building Materials","bm",false),.75f),"hauling pack general cargo");
  Check(Eq(SpecialistPackItemWeightMultiplier(tags,"Foodcube","food",false),.75f),"hauling pack all-cargo support");

  ItemDescriptor hoe; hoe.baseId="mod.hoe"; hoe.name="Iron Hoe"; hoe.quality=.5f; hoe.stackable=false;
  RuleConfig cfg; cfg.globalChance=100.0f;
  RoleProfile farmer; farmer.primary=STAT_FARMING; farmer.wealth01=.7f;

  ItemDescriptor contextRags; contextRags.baseId="mod.worker_rags"; contextRags.name="Worker Rags"; contextRags.quality=.5f;
  std::vector<ItemTag> contextTags=Classify(contextRags,overrides,exclusions);
  AffixRecord farmerRags=RollAffixes(contextRags,farmer,cfg,contextTags,"farmer-rags",99,false);
  Check(farmerRags.affixes.size()==1&&farmerRags.affixes[0].stat==STAT_FARMING,"generic workwear narrows fully to farmer context");
  RoleProfile labourer; labourer.primary=STAT_LABOURING; labourer.wealth01=.7f;
  AffixRecord labourRags=RollAffixes(contextRags,labourer,cfg,contextTags,"labour-rags",99,false);
  Check(!labourRags.affixes.empty()&&labourRags.affixes[0].stat==STAT_LABOURING,"same workwear base uses labourer context on another instance");
  RoleProfile medicCtx; medicCtx.primary=STAT_MEDIC; medicCtx.wealth01=.7f;
  AffixRecord medicRags=RollAffixes(contextRags,medicCtx,cfg,contextTags,"medic-rags",99,false);
  Check(!medicRags.affixes.empty()&&medicRags.affixes[0].stat==STAT_MEDIC,"generic workwear supports field medic context");
  RoleProfile cookCtx; cookCtx.primary=STAT_COOKING; cookCtx.wealth01=.7f;
  AffixRecord cookRags=RollAffixes(contextRags,cookCtx,cfg,contextTags,"cook-rags",99,false);
  Check(!cookRags.affixes.empty()&&cookRags.affixes[0].stat==STAT_COOKING,"generic workwear supports cook context");
  RoleProfile engineerCtx; engineerCtx.primary=STAT_ENGINEERING; engineerCtx.wealth01=.7f;
  AffixRecord engineerRags=RollAffixes(contextRags,engineerCtx,cfg,contextTags,"engineer-rags",99,false);
  Check(!engineerRags.affixes.empty()&&engineerRags.affixes[0].stat==STAT_ENGINEERING,"generic workwear supports engineering context");
  RoleProfile noRole;
  AffixRecord contextlessRags=RollAffixes(contextRags,noRole,cfg,contextTags,"contextless-rags",99,false);
  Check(contextlessRags.affixes.empty(),"generic workwear without any recognized source context gets no roll");

  RoleProfile worldLoot; worldLoot.worldLootSource=true; worldLoot.primary=STAT_NONE; worldLoot.wealth01=.7f;
  RuleConfig worldCfg=cfg; worldCfg.worldLootMultiplier=1.0f;
  AffixRecord worldRags=RollAffixes(contextRags,worldLoot,worldCfg,contextTags,"world-rags",99,false);
  Check(!worldRags.affixes.empty(),"world loot generic workwear can roll profession gear");
  Check(HasStat(AllowedStats(contextTags),worldRags.affixes[0].stat),"world loot roll stays inside legal item pool");
  worldCfg.worldLootMultiplier=0.0f;
  AffixRecord disabledWorldRags=RollAffixes(contextRags,worldLoot,worldCfg,contextTags,"world-rags-off",99,false);
  Check(disabledWorldRags.affixes.empty(),"world loot multiplier can disable contextless exploration rolls");

  RoleProfile traderStock; traderStock.traderSource=true; traderStock.primary=STAT_NONE; traderStock.wealth01=.7f;
  AffixRecord traderRags=RollAffixes(contextRags,traderStock,cfg,contextTags,"trader-rags",99,false);
  Check(!traderRags.affixes.empty(),"generic workwear in trader stock can roll profession gear");
  Check(HasStat(AllowedStats(contextTags),traderRags.affixes[0].stat),"trader generic roll stays inside item context pool");

  ItemDescriptor contextGoggles; contextGoggles.baseId="mod.plain_goggles"; contextGoggles.name="Plain Goggles"; contextGoggles.quality=.5f; contextGoggles.armour=true;
  std::vector<ItemTag> gTags=Classify(contextGoggles,overrides,exclusions);
  RoleProfile researcherCtx; researcherCtx.primary=STAT_SCIENCE; researcherCtx.wealth01=.7f;
  AffixRecord researchGoggles=RollAffixes(contextGoggles,researcherCtx,cfg,gTags,"research-goggles",77,false);
  Check(researchGoggles.affixes.size()==1&&researchGoggles.affixes[0].stat==STAT_SCIENCE,"generic goggles narrow fully to researcher context");
  tags.clear();tags.push_back(TAG_TOOL_FARMING);
  AffixRecord a=RollAffixes(hoe,farmer,cfg,tags,"instance-a",1234,false);
  Check(!a.affixes.empty(),"forced roll produces affix");
  Check(a.affixes[0].stat==STAT_FARMING,"farming contextual roll");
  Check(a.affixes[0].percent>=7&&a.affixes[0].percent<=12,"tier3 magnitude");
  AffixRecord a2=RollAffixes(hoe,farmer,cfg,tags,"instance-a",1234,false);
  Check(SerializeRecord(a)==SerializeRecord(a2),"deterministic same identity");
  AffixRecord b=RollAffixes(hoe,farmer,cfg,tags,"instance-b",1234,false);
  Check(SerializeRecord(a)!=SerializeRecord(b),"different identity variance");

  ItemDescriptor stack=hoe;stack.stackable=true;
  Check(RollAffixes(stack,farmer,cfg,tags,"stack",1,false).affixes.empty(),"stackable excluded");
  ItemDescriptor uniqueRoll=hoe;uniqueRoll.legendary=true;
  Check(RollAffixes(uniqueRoll,farmer,cfg,tags,"unique",1,false).affixes.empty(),"unique item roll excluded");
  RuleConfig disabled=cfg;disabled.enabled=false;
  Check(RollAffixes(hoe,farmer,disabled,tags,"off",1,false).affixes.empty(),"disabled no roll");
  RuleConfig zero=cfg;zero.globalChance=0;
  Check(RollAffixes(hoe,farmer,zero,tags,"zero",1,false).affixes.empty(),"zero chance no roll");

  ItemDescriptor high=hoe;high.quality=.90f;
  tags.clear();tags.push_back(TAG_TOOL_RESEARCH);tags.push_back(TAG_TOOL_ROBOTICS);
  RoleProfile researcher;researcher.primary=STAT_SCIENCE;researcher.wealth01=.8f;
  AffixRecord highRoll=RollAffixes(high,researcher,cfg,tags,"high",777,false);
  Check(highRoll.tier==5,"high quality tier5");
  Check(highRoll.affixes.size()<=3,"tier5 up to three affixes");
  for(size_t i=0;i<highRoll.affixes.size();++i)
    Check(highRoll.affixes[i].percent>=13&&highRoll.affixes[i].percent<=20,"tier5 magnitude");

  RuleConfig one=cfg;one.maxAffixes=1;
  AffixRecord oneRoll=RollAffixes(high,researcher,one,tags,"one",123,false);
  Check(oneRoll.affixes.size()<=1,"maxAffixes one");

  std::string serial=SerializeRecord(a);
  AffixRecord parsed;
  Check(ParseRecord(serial,parsed),"parse serialized");
  Check(parsed.instanceKey==a.instanceKey&&parsed.baseId==a.baseId&&parsed.tier==a.tier,"roundtrip header");
  Check(parsed.affixes.size()==a.affixes.size(),"roundtrip affix count");
  Check(!ParseRecord("bad",parsed),"reject malformed");
  Check(!ParseRecord("x\ty\t1\t999:10",parsed),"reject invalid stat id");
  Check(!ParseRecord("\ty\t1\t1:10",parsed),"reject empty identity");
  Check(!ParseRecord("x\t\t1\t1:10",parsed),"reject empty base id");
  Check(!ParseRecord("x\ty\t99\t1:10",parsed),"reject invalid tier");
  Check(ParseRecord("x\ty\t1\t",parsed)&&parsed.affixes.empty(),"parse empty affix record");
  Check(ParseRecord("x\ty\t1\t\r",parsed)&&parsed.affixes.empty()&&parsed.tier==1,"parse empty affix record with CRLF line end (live sidecar)");
  Check(ParseRecord("x\ty\t2\t10:8.5\r",parsed)&&parsed.affixes.size()==1&&Eq(parsed.affixes[0].percent,8.5f),"parse affix record with CRLF line end");

  std::vector<AffixRecord> rs;
  AffixRecord r1;r1.affixes.push_back(Affix(STAT_FARMING,5));rs.push_back(r1);
  AffixRecord r2;r2.affixes.push_back(Affix(STAT_FARMING,7));r2.affixes.push_back(Affix(STAT_SCIENCE,4));rs.push_back(r2);
  Check(Eq(AggregatePercent(rs,STAT_FARMING),12),"aggregate same stat");
  Check(Eq(AggregatePercent(rs,STAT_SCIENCE),4),"aggregate other stat");
  Check(Eq(AggregatePercent(rs,STAT_MEDIC),0),"aggregate absent stat");

  unsigned int seed=42;
  bool sawLow=false,sawHigh=false;
  for(int i=0;i<5000;++i){
    float u=UnitRoll(seed);
    Check(u>=0&&u<=1,"rng bounded");
    if(u<.1f)sawLow=true;
    if(u>.9f)sawHigh=true;
  }
  Check(sawLow&&sawHigh,"rng distribution reaches both tails");
  Check(Hash32("abc")==Hash32("abc"),"hash deterministic");
  Check(Hash32("abc")!=Hash32("abd"),"hash distinguishes nearby strings");

  for(int s=STAT_LABOURING;s<=STAT_THIEVERY;++s)
    Check(IsProfessionStat((ProfessionStat)s),"profession stat recognized");
  Check(!IsProfessionStat(STAT_NONE),"none not profession stat");

  if(fails){std::cerr<<fails<<" failure(s), "<<passes<<" passed\n";return 1;}
  std::cout<<"ProfessionGearCore tests passed: "<<passes<<" checks\n";
  return 0;
}
