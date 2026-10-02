
#include "ProfessionGearCore.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
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

  ItemDescriptor pitchfork; pitchfork.baseId="mod.pitchfork"; pitchfork.name="Rusty Pitchfork"; pitchfork.weapon=true; pitchfork.weaponLevel=25;
  {std::vector<ItemTag> x=Classify(pitchfork,overrides,exclusions);Check(HasTag(x,TAG_TOOL_FARMING),"pitchfork weapon class gets farming context");}

  ItemDescriptor toolWeapon; toolWeapon.baseId="mod.pickaxe"; toolWeapon.name="Industrial Pickaxe"; toolWeapon.weapon=true; toolWeapon.weaponLevel=40;
  {std::vector<ItemTag> x=Classify(toolWeapon,overrides,exclusions);Check(HasTag(x,TAG_TOOL_MINING),"tool-like weapon allowed by explicit semantics");}

  ItemDescriptor legendary=toolWeapon; legendary.baseId="legendary"; legendary.legendary=true; legendary.weaponLevel=100;
  Check(Classify(legendary,overrides,exclusions).empty(),"legendary item classification blocked");

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

  ItemDescriptor hoe; hoe.baseId="mod.hoe"; hoe.name="Iron Hoe"; hoe.quality=.5f; hoe.stackable=false;
  RuleConfig cfg; cfg.globalChance=100.0f;
  RoleProfile farmer; farmer.primary=STAT_FARMING; farmer.wealth01=.7f;

  ItemDescriptor contextRags; contextRags.baseId="mod.worker_rags"; contextRags.name="Worker Rags"; contextRags.quality=.5f;
  std::vector<ItemTag> contextTags=Classify(contextRags,overrides,exclusions);
  AffixRecord farmerRags=RollAffixes(contextRags,farmer,cfg,contextTags,"farmer-rags",99,false);
  Check(!farmerRags.affixes.empty()&&farmerRags.affixes[0].stat==STAT_FARMING,"generic workwear uses farmer context for first affix");
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
  Check(contextlessRags.affixes.empty(),"loose generic workwear without context gets no profession roll");

  RoleProfile traderStock; traderStock.traderSource=true; traderStock.primary=STAT_NONE; traderStock.wealth01=.7f;
  AffixRecord traderRags=RollAffixes(contextRags,traderStock,cfg,contextTags,"trader-rags",99,false);
  Check(!traderRags.affixes.empty(),"generic workwear in trader stock can roll profession gear");
  Check(HasStat(AllowedStats(contextTags),traderRags.affixes[0].stat),"trader generic roll stays inside item context pool");

  ItemDescriptor contextGoggles; contextGoggles.baseId="mod.plain_goggles"; contextGoggles.name="Plain Goggles"; contextGoggles.quality=.5f;
  std::vector<ItemTag> gTags=Classify(contextGoggles,overrides,exclusions);
  RoleProfile researcherCtx; researcherCtx.primary=STAT_SCIENCE; researcherCtx.wealth01=.7f;
  AffixRecord researchGoggles=RollAffixes(contextGoggles,researcherCtx,cfg,gTags,"research-goggles",77,false);
  Check(!researchGoggles.affixes.empty()&&researchGoggles.affixes[0].stat==STAT_SCIENCE,"goggles use researcher context for first affix");
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
  RuleConfig disabled=cfg;disabled.enabled=false;
  Check(RollAffixes(hoe,farmer,disabled,tags,"off",1,false).affixes.empty(),"disabled no roll");
  RuleConfig zero=cfg;zero.globalChance=0;
  Check(RollAffixes(hoe,farmer,zero,tags,"zero",1,false).affixes.empty(),"zero chance no roll");

  ItemDescriptor high=hoe;high.quality=.90f;
  tags.clear();tags.push_back(TAG_TOOL_RESEARCH);tags.push_back(TAG_TOOL_ROBOTICS);
  RoleProfile researcher;researcher.primary=STAT_SCIENCE;researcher.wealth01=.8f;
  AffixRecord highRoll=RollAffixes(high,researcher,cfg,tags,"high",777,false);
  Check(highRoll.tier==5,"high quality tier5");
  Check(highRoll.affixes.size()<=2,"max two affixes");
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
  Check(ParseRecord("x\ty\t1\t",parsed)&&parsed.affixes.empty(),"parse empty affix record");

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
