#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

enum Structure {BARRACKS, TOWER, MINE, EMPTY};
enum Unit {QUEEN, KNIGHT, ARCHER, GIANT, NONE};

int arch_knit_barrack_difference = 2;
int arch_knit_troop_difference = 2;

int gold = 0;

int cost_knight = 80;
int cost_archer = 100;
int cost_giant = 140;

int max_of_sites = 6;
int current_of_sites = 0;

int towers_per_mine = 2;
int target_range = 500;

int close_distance_sqr = 50000;

class Site;
class Queen;
class Knight;
class Archer;
class Giant;

map<int, Site*> sites;

Site* GetSiteWithId(int id);
Unit GetUnitType(int type);
Structure GetStructureType(int type);
Site* FindClosestEmptySite(int x, int y);
string MoveToSite(Site* site);
string BuildMineAtSite(Site* site);
string BuildTowerAtSite(Site* site);
string BuildBarracksAtSite(Site* site, Unit unit);
string train(vector<Site*> &archer_barracks, vector<Site*> &knight_barracks);
bool EnemyKnightsClose(vector<Knight*> &enemyKnights, Queen* queen);

class Site
{
public:
	Site(int id, int x, int y, int r) {
		this->id = id;
		this->x = x;
		this->y = y;
		this->radius = r;
		
		this->gold_deposits = -1;
		this->owner = -1;
		this->structure = EMPTY;
		this->max_mine = -1;
		this->rate = -1;
		this->HP = -1;
		this->train_cooldown = -1;
		this->last_turn_range = -1;
		this->range = -1;
		this->unit = NONE;
	};

	int id;
	int x;
	int y;
	int radius;

	int gold_deposits;
	int max_mine;
	Structure structure;
	int owner;

	// P1
	int rate;
	int HP;
	int train_cooldown;
	// Mine: rate (gold/turn)
	// Tower: HP
	// Barracks: train_cooldown

	// P2
	int last_turn_range;
	int range;
	Unit unit;

	// Tower: range
	// Barracks: Unit type: 0 - Knight, 1 - Archer, 2 - Giant

	void Update(int new_gold, int max_size, Structure new_struct, int new_owner, int param1, int param2) {
		gold_deposits = new_gold;
		max_mine = max_size;
		structure = new_struct;

		owner = new_owner;
		last_turn_range = range;

		rate = -1;
		HP = -1;
		train_cooldown = -1;
		range = -1;
		unit = NONE;
		
		if (new_struct == MINE) {
			rate = param1;
		} else if (new_struct == TOWER) {
			HP = param1;
			if (range < param2) {
			}
			range = param2;
		} else if (new_struct == BARRACKS) {
			train_cooldown = param1;
			unit = GetUnitType(param2);
		}
	}

	bool IsMine() {
		return structure == MINE;
	}

	bool IsTower() {
		return structure == TOWER;
	}

	bool IsBarracks() {
		return structure == BARRACKS;
	}

	bool IsEmpty() {
		return structure == EMPTY;
	}

	bool IsMaxMineLevel() {
		if (!IsMine()) 
			return false;

		return rate >= max_mine;
	}

	bool IsMaxMineLevelInNextTurn() {
		if (!IsMine()) 
			return false;

		return rate == max_mine - 1;
	}

	bool MaxRangeReached() {
		if (last_turn_range == range)
			return true;
		return false;
	}

};

class Queen {
public:
	Queen(int x, int y, int hp) {
		this->x = x;
		this->y = y;
		this->HP = hp;
	}
	int x;
	int y;
	int HP;
};

class Knight {
public:
	Knight(int x, int y, int hp) {
		this->x = x;
		this->y = y;
		this->HP = hp;
	}
	int x;
	int y;
	int HP;
};

class Archer {
public:
	Archer(int x, int y, int hp) {
		this->x = x;
		this->y = y;
		this->HP = hp;
	}
	int x;
	int y;
	int HP;
};

class Giant {
public:
	Giant(int x, int y, int hp) {
		this->x = x;
		this->y = y;
		this->HP = hp;
	}
	int x;
	int y;
	int HP;
};

int main()
{
	// Map Setup
    int numSites;
    cin >> numSites; cin.ignore();

    for (int i = 0; i < numSites; i++) {
        int siteId;
        int x;
        int y;
        int radius;
        cin >> siteId >> x >> y >> radius; cin.ignore();

        Site* site = new Site(siteId, x, y, radius);
        sites[siteId] = site;
    }

    // Game Loop
    int stage = -1;
    while (1) {
    	// Reading new sites info
        int touchedSite; // -1 if none
        cin >> gold >> touchedSite; cin.ignore();

        // vector<Site*> mySites;
        vector<Site*> knight_barracks;
        vector<Site*> archer_barracks;

        vector<Site*> towers;
        vector<Site*> mines;

        // vector<Site*> enemySites;

        for (int i = 0; i < numSites; i++) {
            int siteId;
            int goldRemaining; // -1 if unknown
            int maxMineSize; // -1 if unknown
            int structureType; // -1 = No structure, 0 = Goldmine, 1 = Tower, 2 = Barracks
            int owner; // -1 = No structure, 0 = Friendly, 1 = Enemy
            int param1;
            int param2;
            cin >> siteId >> goldRemaining >> maxMineSize >> structureType >> owner >> param1 >> param2; cin.ignore();
            
            Site* site = sites[siteId];
            Structure structure = GetStructureType(structureType);
            site->Update(goldRemaining, maxMineSize, structure, owner, param1, param2);

            if (structure == BARRACKS && owner == 0) {
            	if (site->unit == KNIGHT)
            		knight_barracks.push_back(site);
            	else if (site->unit == ARCHER)
            		archer_barracks.push_back(site);
            }

            if (structure == TOWER && owner == 0) {
            	cerr << "Range: " << site->range << endl;
            	towers.push_back(site);
            }

            if (structure == MINE && owner == 0) {
            	mines.push_back(site);
            }

   //          if (owner == 0) {
   //          	cerr << "Site " << siteId << ", at " << site->x << " " << site->y << ": has " \
   //              	 << goldRemaining << ", max size - " << maxMineSize << endl;
			// } else if (goldRemaining >= 0) {
			// 	cerr << "Site " << siteId << ", at " << site->x << " " << site->y << ": has " \
   //              	 << goldRemaining << ", max size - " << maxMineSize << endl;
			// }
        }

        // Reading units info
        int numUnits;
        cin >> numUnits; cin.ignore();

        Queen* my_queen;
        Queen* enemy_queen;

        vector<Knight*> enemy_knights;

        for (int i = 0; i < numUnits; i++) {
            int x;
            int y;
            int owner;
            int unitType; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER, 2 = GIANT
            int health;
            cin >> x >> y >> owner >> unitType >> health; cin.ignore();

            Unit unit = GetUnitType(unitType);
            if (unit == QUEEN) {
            	if (owner == 0)
            		my_queen = new Queen(x, y, health);
            	else 
            		enemy_queen = new Queen(x, y, health);
            }

            if (unit == KNIGHT && owner == 1)
				enemy_knights.push_back(new Knight(x, y, health));
        }

        // Making a strategic decision and generating a command
        Site* closest_empty_site = FindClosestEmptySite(my_queen->x, my_queen->y);

        // Assign stage
        // stage 0 - barracks-knight
        // stage 1 - tower
        // stage 2 - mine
        if (stage == -1) {
        	int num_barracks_knights = knight_barracks.size();
			int num_towers = towers.size(); 
			int num_mines = mines.size();
			bool enemy_close = EnemyKnightsClose(enemy_knights, my_queen);
			cerr << "Enemy status: " << enemy_close << endl;

			int target_towers = num_mines * towers_per_mine;
			cerr << "Flags: B: " << num_barracks_knights << "; num_T " << num_towers << "; tar_T: " << target_towers << "; M:" << num_mines << endl;
			if (enemy_close) {
				stage = 1;
				cerr << "Enemy is close, building tower" << endl;
			} else if (num_barracks_knights < 1)
				stage = 0;
			else if (num_towers == 0 || num_towers < target_towers) 
				stage = 1;
			else 
				stage = 2;
        }

        // Stage 0
        // Build barracks, request first wave of Knights
        if (stage == 0) {
        	cout << BuildBarracksAtSite(closest_empty_site, KNIGHT) << endl;
        	if (touchedSite != -1) {
        		cerr << "Stage 0 complete" << endl;
        		stage = -1;
        	}
        }

        // Stage 1
        // Build Tower
        if (stage == 1) {
        	if (touchedSite != -1 && (GetSiteWithId(touchedSite)->IsEmpty() || GetSiteWithId(touchedSite)->IsTower())) {
	        	Site* site = GetSiteWithId(touchedSite);
	        	if (site->range >= target_range) {
	     			cerr << "Stage 1 is complete" << endl;
	     			stage = -1;
	     			cout << MoveToSite(closest_empty_site) << endl;
	        	} else
	        		cout << BuildTowerAtSite(site) << endl;
		        
	        } else {
	        	cout << MoveToSite(closest_empty_site) << endl;
	        }
        }

        // Stage 2
        // Build mine, upgrade mine to max level
        if (stage == 2) {
        	if (touchedSite != -1 && (GetSiteWithId(touchedSite)->IsEmpty() || GetSiteWithId(touchedSite)->IsMine())) {
	        	Site* site = GetSiteWithId(touchedSite);
	        	if (site->IsMaxMineLevelInNextTurn() || site->IsMaxMineLevel()) {
	     			cerr << "Stage 2 is complete" << endl;
	     			stage = -1;
	     			cout << MoveToSite(closest_empty_site) << endl;
	        	} else if (site->gold_deposits == 0)
	        		cout << BuildTowerAtSite(site) << endl;
	        	else 
	        		cout << BuildMineAtSite(site) << endl;
	        } else {
	        	cout << MoveToSite(closest_empty_site) << endl;
	        }
        }

        
        
        cout << train(archer_barracks, knight_barracks) << endl;
    }
}

Site* GetSiteWithId(int id) {
	return sites[id];
}

// 0 - Knight
// 1 - Archer
// 2 - Giant
Unit GetUnitType(int type) {
	if (type == -1)
		return QUEEN;

	if (type == 0) 
		return KNIGHT;
	
	if (type == 1) 
		return ARCHER;
	
	if (type == 2) 
		return GIANT;

	return NONE;	
};

Structure GetStructureType(int type) {
	if (type == 0) 
		return MINE;
	
	if (type == 1) 
		return TOWER;
	
	if (type == 2) 
		return BARRACKS;

	return EMPTY;	
};


Site* FindClosestEmptySite(int x, int y) {

	if (sites.size() == 0) {
		return nullptr;
	}

	int closest_id = -1;
	double closest_distance = 1e9;

	int i = 0;
	for (map<int, Site*>::iterator it = sites.begin(); it != sites.end(); it++)
	{

		if (!(it->second->IsEmpty())) {
			// ignore all non-empty sites
			continue; 
		}

		double new_distance = (x - it->second->x) * (x - it->second->x) + (y - it->second->y) * (y - it->second->y);
		if (new_distance < closest_distance) {
			closest_distance = new_distance;
			closest_id = it->second->id;
		}
	}

	if (closest_id == -1)
		return nullptr;

	return sites[closest_id];
}

string MoveToSite(Site* site) {
	return "MOVE " + to_string(site->x) + " " + to_string(site->y);
}

string BuildMineAtSite(Site* site) {
	return "BUILD " + to_string(site->id) + " MINE";
}

string BuildTowerAtSite(Site* site) {
	return "BUILD " + to_string(site->id) + " TOWER";
}

string BuildBarracksAtSite(Site* site, Unit unit) {
	string command = "BUILD " + to_string(site->id) + " BARRACKS-";
	if (unit == KNIGHT)
		command += "KNIGHT";
	else if (unit == ARCHER)
		command += "ARCHER";
	else if (unit == GIANT)
		command += "GIANT";

	return command;
}

string train(vector<Site*> &archer_barracks, vector<Site*> &knight_barracks) {
	int archer_size = archer_barracks.size();
	int knight_size = knight_barracks.size();
	// cerr << "Archer Barracks: " << archer_size << endl;
	// cerr << "Knight Barracks: " << knight_size << endl;
	string command = "TRAIN";
	if (archer_size == 0 && knight_size == 0)
		return command;

	// TODO
	// - for now, order 2 archers, 2 knight troops
	int budget = 0;
	// if (train_archers) {
	// 	train_archers = !train_archers;
	// 	for (int i = 0; i < archer_size && i < 2; ++i)
	// 	{
	// 		if (archer_barracks[i].turns_to_spawn > 0)
	// 			continue;

	// 		budget += 100;
	// 		if (budget > total_gold) {
	// 			budget -= 100;
	// 			break;
	// 		}
	// 		cerr << "Archer at: " << archer_barracks[i].id << endl;
	// 		command += " ";
	// 		command += to_string(archer_barracks[i].id);
	// 	}
	// } 

	// if (!train_archers) {
		// train_archers = !train_archers;
	for (int i = 0; i < knight_size; ++i)
	{
		if (knight_barracks[i]->train_cooldown > 0)
			continue;

		budget += 80;
		if (budget >= gold) {
			budget -= 80;
			break;
		}
		cerr << "Knight at: " << knight_barracks[i]->id << endl;
		command += " ";
		command += to_string(knight_barracks[i]->id);
	}
	// }


	cerr << "Command: " << command << ";" << endl;

	return command;
}

bool EnemyKnightsClose(vector<Knight*> &enemyKnights, Queen* queen) {
	cerr << "Checking distance..." << endl;
	if (enemyKnights.empty()) {
		cerr << "Empty vector..." << endl;
		return false;
	}

	for (vector<Knight*>::iterator it = enemyKnights.begin(); it != enemyKnights.end(); it++) {
		int distance = ((*it)->x - queen->x) * ((*it)->x - queen->x) + ((*it)->y - queen->y) * ((*it)->y - queen->y);
		cerr << "Dist to knight at " << (*it)->x << " " << (*it)->y << ": x:" << ((*it)->x - queen->x)\
			 << ", y: " << ((*it)->y - queen->y) << ":: " << distance << endl;
		if (distance <= close_distance_sqr) 
			return true;
	}

	return false;
}




