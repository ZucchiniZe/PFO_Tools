#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "OfficialData.h"
#include "EntityDefinition.h"
#include "EntityTypeHelper.h"
#include "LineItem.h"
#include "Utils.h"

using namespace std;

OfficialData::OfficialData() {
    FileProcessorMap["Recipes (Crafting).csv"] = &OfficialData::ParseAndStoreCraftingRecipeFile;
    FileProcessorMap["Recipes (Refining).csv"] = &OfficialData::ParseAndStoreRefiningRecipeFile;
    FileProcessorMap["Skills Advancement.csv"] = &OfficialData::ParseAndStoreSkillsAdvancementFile;
    FileProcessorMap["Armor Advancement.csv"] = &OfficialData::ParseAndStoreArmorAdvancementFile;
    FileProcessorMap["Attack Advancement.csv"] = &OfficialData::ParseAndStoreAttackAdvancementFile;
    FileProcessorMap["Bonuses Advancement.csv"] = &OfficialData::ParseAndStoreBonusesAdvancementFile;
    FileProcessorMap["Cantrip Advancement.csv"] = &OfficialData::ParseAndStoreCantripAdvancementFile;
    FileProcessorMap["Defensive Advancement.csv"] = &OfficialData::ParseAndStoreDefensiveAdvancementFile;
    FileProcessorMap["Expendables Advancement.csv"] = &OfficialData::ParseAndStoreExpendablesAdvancementFile;
    FileProcessorMap["Feature Advancement.csv"] = &OfficialData::ParseAndStoreFeatureAdvancementFile;
    FileProcessorMap["Orison Advancement.csv"] = &OfficialData::ParseAndStoreOrisonAdvancementFile;
    FileProcessorMap["Points Advancement.csv"] = &OfficialData::ParseAndStorePointsAdvancementFile;
    FileProcessorMap["Proficiencies.csv"] = &OfficialData::ParseAndStoreProficienciesAdvancementFile;
    FileProcessorMap["Reactive Advancement.csv"] = &OfficialData::ParseAndStoreReactiveAdvancementFile;
    FileProcessorMap["Utility Advancement.csv"] = &OfficialData::ParseAndStoreUtilityAdvancementFile;
}

bool OfficialData::ParseAndStoreSkillsAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreArmorAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreAttackAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreBonusesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreCantripAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreDefensiveAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreExpendablesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
//bool OfficialData::ParseAndStoreFeatureAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreOrisonAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStorePointsAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
//bool OfficialData::ParseAndStoreProficienciesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Feat");}
bool OfficialData::ParseAndStoreReactiveAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreUtilityAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}

bool OfficialData::ParseAndStoreFeatureAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}
bool OfficialData::ParseAndStoreProficienciesAdvancementFile(string fn) {    return this->ParseAndStoreProgressionFile(fn, "Skill");}

// Perception is listed as a skill in Skills Advancement.csv but is refered to as a
// Feat in Utility Advancement.csv (in one of the "Feat LvX" columns.

void OfficialData::Dump() {

    cout << "Have " << Entities.size() << " entities" << endl;

    map<string, EntityDefinition*>::iterator itr;
    for (itr = Entities.begin(); itr != Entities.end(); ++itr) {
	cout << *((*itr).second) << endl;
    }
}

EntityDefinition* OfficialData::GetEntity(string name) {
    map< string, EntityDefinition* >::iterator entityMapEntry;
    entityMapEntry = Entities.find(name);
    if (entityMapEntry == Entities.end()) {
	return NULL;
    } else {
	return (*entityMapEntry).second;
    }
}

int OfficialData::ProcessSpreadsheetDir(string directoryName) {
    DIR *dp;
    struct dirent *dirp;

    dp = opendir(directoryName.c_str());
    if (dp == NULL) {
	cout << "ERROR(" << errno << ") opening " << directoryName << endl;
	return errno;
    }

    while ((dirp = readdir(dp))) {
	string filepath;
	struct stat filestat;


	string fname = dirp->d_name;
	filepath = directoryName + "/" + fname;

	if (stat( filepath.c_str(), &filestat)) continue;
	if (S_ISDIR( filestat.st_mode)) continue;

	if (FileProcessorMap[fname]) {
	    (this->*FileProcessorMap[fname])(filepath);
	} else {
	    cout << "no handler for " << fname << endl;
	}	
    }
    closedir(dp);

    cout << "Done processing spreadsheet data; have " << Entities.size() << " entities" << endl;

    return 0;
}

bool OfficialData::ParseAndStoreProgressionFile(string fn, string t) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

    cout << "+++ RUNNING: OfficialData::ParseAndStoreSkillFile(" << fn << ")" << endl;

    EntityTypeHelper* typeHelper = EntityTypeHelper::Instance();

    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	cerr << "failed to read file " << fn << " errno: " << errno << endl;
	return false;
    }
    string line;
    int line_num = 0;
    while(getline(fin, line)) {
	++line_num;
	// there is only one line per skill with each line having the data out to the max skill (121 total fields)
	vector<string> fields = Utils::SplitCommaSeparatedValuesWithQuotedFields(line.c_str());
	
	// SlotName
	//Exp Lv1
	//Category Lv1
	//Feat Lv1
	//Achievement Lv1
	//AbilityReq Lv1
	//AbilityBonus Lv1
	//Exp Lv2
	//Category Lv2
	//Feat Lv2
	//Achievement Lv2
	//AbilityReq Lv2
	//AbilityBonus Lv2

	string skillName = fields[0];
	if (line_num == 1) {
	    cout << "skipping first line w/ first field: " << skillName << endl;
	    continue;
	}
	
	// see if something else already added an entity for this (eg, if the entity was listed
	// as a component for another Entity)
	map< string, EntityDefinition* >::iterator entityMapEntry;
	EntityDefinition *entity;
	entityMapEntry = Entities.find(skillName);
	if (entityMapEntry != Entities.end()) {
	    entity = (*entityMapEntry).second;
	    printf("%c %2d: %22s -> %d\n", '.', line_num, fields[0].c_str(), (int)fields.size());
	    assert(entity->ProcessedSpreadsheetDefinition == false);
	} else {
	    printf("%c %2d: %22s -> %d\n", '+', line_num, fields[0].c_str(), (int)fields.size());

	    entity = new EntityDefinition();
	    entity->Name = skillName;

	    // this has an unfortunate side effect of poluting the typeHelper's id space
	    // because when we process the files in random order, we might add crafting skills
	    // here w/ an incomplete type.  EG: Skills.Weaver.  Then when we prcess the
	    // refining crafting recipe file, the type for this entity will change to
	    // Skills.Refining.Weaver.  Not sure what to do about this yet.
	    list<string> typeFields;
	    typeFields.push_back(t);
	    typeFields.push_back(skillName);
	    entity->Type = typeHelper->GetType(typeFields);

	    Entities[skillName] = entity;
	}
	    
	// hack here to keep track of known errors in the data
	if (entity->Requirements.size() != 0) {
	    if (fn == "official_data/Utility Advancement.csv" && skillName == "Channel Smite") {
		cout << "KNOWN DATA ERROR: Utility Advancement.csv has two rows for Channel Smite" << endl;
	    } else {
		// skills are ranked - always ranked - but we won't add the ranks of requirements or
		// provides until we process them here
		assert (entity->Requirements.size() == 0);
	    }
	}

	int idx = 1;
	int rank = 1;
	for (idx = 1, rank = 1; idx < fields.size(); idx += 6, ++rank) {

	    // [  0 ][  1  ][  2  ][  3  ][  4  ][  5  ][  6  ][  7  ][  8  ][  9  ][ 10  ][ 11  ][ 12  ]
	    // [name][1_exp][1_cat][1_fea][1_ach][1_abi][1_ab+][2_exp][2_cat][2_fea][2_ach][2_abi][2_ab+]
	    // say: idx = 7 and size = 11 (error).  11(sz) - 7(ix) = 4
	    if ((fields.size() - idx) < 5) {
		cout << "ERROR: in " << fn << ", line " << line_num << ", " << skillName 
		     << " rank " << rank << " has incomplete data - skipping" << endl;
		continue;
	    }

	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));

	    if (rank == 1) {
		// we will index into the requirements based on the rank since the first rank
		// is 1, we need to add an empty/dummy requirement list at rank 0 so that
		// Requirements[1] gets the first rank's requirements.
		entity->Requirements.push_back(*(new list<LineItem*>));
		entity->Provides.push_back(*(new list<LineItem*>));
	    }
	    list<LineItem*> *reqs = &(entity->Requirements.back());
	    list<LineItem*> *pros = &(entity->Requirements.back());
	    
	    // add the exp requirement
	    string entityName = "ExperiencePoint";
	    LineItem *req = new LineItem();
	    req->Entity = Entities[entityName];
	    if (req->Entity == NULL) {
		list<string> typeFields;
		typeFields.push_back(entityName);
		
		req->Entity = new EntityDefinition();
		req->Entity->Name = entityName;
		req->Entity->Type = typeHelper->GetType(typeFields);
		Entities[entityName] = req->Entity;
	    }
	    req->Quantity = atoi(fields[idx].c_str());
	    reqs->push_back(req);

	    // add the previous rank as a requirement
	    if (rank > 1) {
		req = new LineItem(entity, (rank - 1));
		reqs->push_back(req);
	    }

	    // add the achievement point
	    string reqStr = fields[idx+1];
	    string label = "AchievementPoint";
	    string errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    cout << "ERROR: failed to parse this " << label << " requirement string: [" << reqStr << "]; err:" << errMsg << endl;
		} else {
		    reqs->push_back(required);
		}
	    }

	    // add the feat requirements
	    reqStr = fields[idx+2];
	    label = "Skill";
	    errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    cout << "ERROR: failed to parse this " << label << " requirement string: [" << reqStr << "]; err:" << errMsg << endl;
		} else {
		    reqs->push_back(required);
		}
	    }

	    
	    // add the achievement requirements
	    reqStr = fields[idx+3];
	    label = "Skill";
	    errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    cout << "ERROR: failed to parse this " << label << " requirement string: [" << reqStr << "]; err:" << errMsg << endl;
		} else {
		    reqs->push_back(required);
		}
	    }

	    // add the ability score requirements
	    reqStr = fields[idx+4];
	    label = "AbilityScore";
	    errMsg = "";
	    if (reqStr.size() > 0) {
		LineItem *required = ParseRequirementString(reqStr, label, errMsg);
		if (required == NULL) {
		    cout << "ERROR: failed to parse this " << label << " requirement string: [" << reqStr << "]; err:" << errMsg << endl;
		} else {
		    reqs->push_back(required);
		}
	    }
	}
	
	// for (int idx = 0; idx < fields.size(); ++idx) { cout << fields[idx] << endl; }
    }

    fin.close();

    return true;
}


LineItem* OfficialData::ParseRequirementString(string reqStr, string entityTypeName, string &errMsg) {
    // TODO: MEMORY LEAKS HERE
    // if this function encounters an error (and returns NULL) midway through parsing
    // a requirement, we will leak the already-created LineItems and Logic entities.

    vector<string> andedGroups = Utils::SplitCommaSeparatedValuesWithQuotedFields(reqStr.c_str());
    if (andedGroups.size() < 1) {
	errMsg = "topLevelZero";
	return NULL;
    }

    vector<string>::iterator groupEntry;
    list<LineItem*> andedLineItems;
    for (groupEntry = andedGroups.begin(); groupEntry != andedGroups.end(); ++groupEntry) {
	if (strstr((*groupEntry).c_str(), " or ")) {
	    list<string> orEntityLongName = { string("LogicOr") };
	    EntityDefinition *orEntity = new EntityDefinition();
	    orEntity->Name = "LogicAnd";
	    orEntity->Type = EntityTypeHelper::Instance()->GetType(orEntityLongName);
	    orEntity->Requirements.push_back(*(new list<LineItem*>));
	    
	    vector<string> orReqs = Utils::SplitDelimitedValues((*groupEntry).c_str(), " or ");
	    vector<string>::iterator orEntry;
	    for (orEntry = orReqs.begin(); orEntry != orReqs.end(); ++orEntry) {
		LineItem *newReq = BuildLineItemFromKeyEqualsVal((*orEntry), entityTypeName);
		if (newReq == NULL) {
		    errMsg = "failed to parse 'or' requirement";
		    return NULL;
		}
		orEntity->Requirements[0].push_back(newReq);
	    }
	    andedLineItems.push_back(new LineItem(orEntity, 1));
	    
	} else {
	    LineItem *newReq = BuildLineItemFromKeyEqualsVal((*groupEntry), entityTypeName);
	    if (newReq == NULL) {
		errMsg = "failed to parse 'and' requirement";
		return NULL;
	    }
	    andedLineItems.push_back(newReq);
	}
    }
    if (andedLineItems.size() < 0) {
	errMsg = "failed to parse anything";
	return NULL;
    }

    if (andedLineItems.size() == 1) {
	return andedLineItems.front();
    } else {
	list<string> andEntityLongName = { string("LogicAnd") };
	EntityDefinition *andEntity = new EntityDefinition();
	andEntity->Name = "LogicAnd";
	andEntity->Type = EntityTypeHelper::Instance()->GetType(andEntityLongName);
	andEntity->Requirements.push_back(*(new list<LineItem*>));

	list<LineItem*>::iterator groupEntry;
	for (groupEntry = andedLineItems.begin(); groupEntry != andedLineItems.end(); ++groupEntry) {
	    andEntity->Requirements[0].push_back(*groupEntry);
	}
	LineItem *group = new LineItem(andEntity, 1);
	return group;
    }
}


LineItem* OfficialData::BuildLineItemFromKeyEqualsVal(string kvp, string entityTypeName) {
    string key, value;

    if (!Utils::SplitKeyValueOnChar(kvp.c_str(), "=", key, value)) {
	return NULL;
    }

    EntityDefinition* entity = FindEntity(entityTypeName, key);
    if (entity == NULL) {
	entity = new EntityDefinition();
	entity->Name = key;
	list<string> typeFields;
	typeFields.push_back(entityTypeName);
	typeFields.push_back(key);
	entity->Type = EntityTypeHelper::Instance()->GetType(typeFields);
	Entities[key] = entity;
    }
    
    return new LineItem(entity, atoi(value.c_str()));
}

EntityDefinition* OfficialData::FindEntity(string type, string name) {
    
    // get the short id for this type
    list<string> typeFields;
    typeFields.push_back(type);
    short *tlTypeId = EntityTypeHelper::Instance()->GetType(typeFields);


    map< string, EntityDefinition* >::iterator entityMapEntry;
    EntityDefinition *entity;
    entityMapEntry = Entities.find(name);
    if (entityMapEntry == Entities.end()) {
	return NULL;
    }

    entity = (*entityMapEntry).second;
    if (entity->Type[0] != tlTypeId[0]) {
	list<string> typeNames = EntityTypeHelper::Instance()->GetType(entity->Type);
	string oldType = typeNames.front();
	cout << "ERROR: multiple entities with name [" << name << "]"
	     << " oldType:" << oldType << "; newType:" << type
	     << endl;
    }

    assert(entity->Type[0] == tlTypeId[0]);

    return entity;
    
}

bool OfficialData::ParseAndStoreCraftingRecipeFile(string fn) {
    return this->ParseAndStoreRecipeFile(fn, "Craft");
}

bool OfficialData::ParseAndStoreRefiningRecipeFile(string fn) {
    return this->ParseAndStoreRecipeFile(fn, "Refine");
}

bool OfficialData::ParseAndStoreRecipeFile(string fn, string subtype) {
    // open the file named fn
    // parse the data cells
    // foreach row
    //    make an entity definition and fill it out
    //    store the entity in the global entity map by the name

    cout << "+++ RUNNING: OfficialData::ParseAndStoreRecipeFile(" << fn << ")" << endl;

    EntityTypeHelper* typeHelper = EntityTypeHelper::Instance();

    ifstream fin(fn.c_str());
    if (!fin.is_open()) {
	cerr << "failed to read file " << fn << " errno: " << errno << endl;
	return false;
    }
    string line;
    int line_num = 0;
    while(getline(fin, line)) {
	// this is kind of ugly as the header line is incorrect at this time.  EG, it says:
	// |Component 3 and Number|  Achievement Type|Base Crafting Seconds|Last Updated
	// but a given row will have:
	// |Golden Crystal      |1|||Common          |2400                 |9-18-14

	// so for now, knowledge of the row format will be hardcoded here
	// fieldNum  Name                   Example
	//     0     Recipe                 Recipe
	//     1     Name                   Apprentice's Sunrod
	//     2     ReqSkill               Alchemist
	//     3     SkillRankReq           0
	//     4     Tier                   1
	//     5     Component1Name         Weak Luminous Extract
	//     6     Component1Qty          15
	//     7     Component2Name         Weak Acidic Extract
	//     8     Component2Qty          2
	//     9     Component3Name         Copper Bar
	//    10     Component3Qty          1
	//    11     EMPTY (comp4?)
	//    12     EMPTY (comp4?)
	//    13     Achievement Type       Common
	//    14     Base Crafting Seconds  3000
	//    15     Last Updated           9-18-14

	++line_num;

	// skip the first line - its a header line
	if (line_num < 2) continue;

	stringstream linestream(line);
	string val;
	vector<string> fields;
	while (getline(linestream, val, '|')) {
	    fields.push_back(val);
	}
	if (fields.size() != 16) {
	    cout << "WARNING: bad line in " << fn << ":" << line_num << "; doesn't have 16 fields - has " << fields.size() << ": " << line << endl;
	    continue;
	}
	if (fields[1].size() < 1) {
	    cout << "WARNING: bad line in " << fn << ":" << line_num << "; empty name; " << line << endl;
	    continue;
	}

	string *name = new string(fields[1]);
	int rankInName;
	char *namePart;
	if (Utils::RankInName(name->c_str(), &namePart, rankInName)) {
	    // if (regex_search(name, " \+[0-9]+$")) {
	    // cout << endl << namePart << "; Rank " << rankInName << " (from [" << *name << "])" << endl;
	    delete name;
	    name = new string(namePart);
	    delete namePart;
	    // right now we aren't storing entities for +1 and greater items
	    // there is much we would have to add to support this - right now for ranked
	    // items there is no other Quantity, so the Quantity in the LineItem does double
	    // duty for ranks and actual quantities.  But when we add Quantities of Ranked items
	    // (like 5 x "Steel Wire +1") then we will have to revamp the LineItem class.
	    if (rankInName > 0) {
		delete name;
		continue;
	    }		
	}

	// see if something else already added an entity for this (eg, if the entity was listed
	// as a component for another Entity)
	map< string, EntityDefinition* >::iterator entityMapEntry;
	EntityDefinition *entity;
	entityMapEntry = Entities.find(*name);
	if (entityMapEntry != Entities.end()) {
	    entity = (*entityMapEntry).second;

	    // only add a Requirements and Provides list if we process the Entity from the spreadsheet
	    if (entity->Requirements.size() < 1) {
		entity->Requirements.push_back(*(new list<LineItem*>));
		entity->Provides.push_back(*(new list<LineItem*>));
	    }
	    assert(entity->ProcessedSpreadsheetDefinition == false);
	    cout << ".";
	} else {
	    cout << "+";
	    // I could (should?) check that these fields were set correctly the first time - but lets call that a todo
	    entity = new EntityDefinition();
	    entity->Name = *name;

	    list<string> typeFields;
	    typeFields.push_back("Item");
	    typeFields.push_back(*name);
	    entity->Type = typeHelper->GetType(typeFields);

	    // right now, Items aren't ranked - but we could extend this later to have a +1 item be rank two, etc
	    // still, we need to create the requirements and provides lists for the single rank
	    entity->Requirements.push_back(*(new list<LineItem*>));
	    entity->Provides.push_back(*(new list<LineItem*>));

	    Entities[*name] = entity;
	}
	// TODO FIXME TEST
	// soon I'll add code to consume the secondary, crowd sourced, table that give the yield
	// for refining stuff - for now, in order to do some testing of the goal solution code,
	// I'm going to make it a random number
	if (subtype == "Refine") {
	    entity->CreationIncrement = (rand() % 4) + 1;
	} else {
	    entity->CreationIncrement = 1;
	}
	entity->ProcessedSpreadsheetDefinition = true;
	
	LineItem *req;

	// time requirement
	req = new LineItem();
	req->Entity = Entities["Time"];
	if (req->Entity == NULL) {
	    list<string> typeFields;
	    typeFields.push_back("Time");

	    req->Entity = new EntityDefinition();
	    req->Entity->Name = "Time";
	    req->Entity->Type = typeHelper->GetType(typeFields);
	    Entities["Time"] = req->Entity;
	}
	req->Quantity = atoi(fields[14].c_str());
	entity->Requirements[0].push_back(req);

	// Skill requirement
	string skillName = fields[2];
	int skillLevel = atoi(fields[3].c_str());

	req = new LineItem();
	req->Entity = Entities[skillName];
	if (req->Entity != NULL) {
	    cout << "_";

	    // this is the only file that knows the difference between refine and gather skills
	    // However, this whole distinction is starting to feel hacky.  I'm going to call this
	    // potential technical debt and do it anyway.
	    list<string> typeFields;
	    typeFields.push_back("Skill");
	    typeFields.push_back(subtype);
	    typeFields.push_back(skillName);
	    req->Entity->Type = typeHelper->GetType(typeFields);

	} else {
	    cout << "+";
	    list<string> typeFields;
	    typeFields.push_back("Skill");
	    typeFields.push_back(subtype);
	    typeFields.push_back(skillName);

	    req->Entity = new EntityDefinition();
	    req->Entity->Name = skillName;
	    req->Entity->Type = typeHelper->GetType(typeFields);
	    req->Entity->ProcessedSpreadsheetDefinition = false;
	    Entities[skillName] = req->Entity;
	}
	req->Quantity = skillLevel;
	entity->Requirements[0].push_back(req);
	
	// now all the components - we only have three per recipe right now but I think there
	// is space in here for four.
	for (int componentOffset = 5; componentOffset < 10; componentOffset += 2) {
	    if (fields[componentOffset].size() < 1) { continue; }
	    string componentName = fields[componentOffset];

	    req = new LineItem();
	    req->Entity = Entities[componentName];

	    if (componentName == "Yew Stave") {
		cout << endl << "Yew Stave Component" << endl;
	    }

	    if (req->Entity != NULL) {
		cout << "@";
	    } else {
		cout << "P";
		list<string> typeFields;
		typeFields.push_back("Item");
		typeFields.push_back(componentName);

		req->Entity = new EntityDefinition();
		req->Entity->Name = componentName;
		req->Entity->Type = typeHelper->GetType(typeFields);
		req->Entity->ProcessedSpreadsheetDefinition = false;
		req->Entity->CreationIncrement = 1;
		Entities[componentName] = req->Entity;
	    }
	    req->Quantity = atoi(fields[componentOffset+1].c_str());
	    entity->Requirements[0].push_back(req);	    
	}
	
	delete name;
	// cout << "[" << *name << "] <-> [" << fields[15] << "]" << endl;
    }
    

    fin.close();

    return true;
}

