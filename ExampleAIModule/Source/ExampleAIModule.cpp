#include "ExampleAIModule.h" 

#include "../Base.h"

#include <unordered_map>
#include <list>

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

void AI::onStart() {
	mainBase = new Base();
	m_bases.push_back(mainBase);

	mainBase->init(this);
	initStartOrder(mainBase);

	Broodwar->setLocalSpeed(10);
	//Enable flags
	Broodwar->enableFlag(Flag::UserInput);
	//Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//Start analyzing map data
	BWTA::readMap();
	analyzed = false;
	analysis_just_finished = false;
	//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL); //Threaded version
	AnalyzeThread();

	for (auto u : Broodwar->self()->getUnits()) {
		if (u->getType().isBuilding()) {
			reservedMinerals += u->getType().mineralPrice();
			reservedGas += u->getType().gasPrice();
		}

		mainBase->m_waiting[u->getType()]++;
	}
}

void AI::onEnd(bool isWinner) {
	delete mainBase;
	if (isWinner) {
		Broodwar->sendText("I won!");
	}
}

Position AI::findGuardPoint() {
	//Get the chokepoints linked to our home region
	std::set<BWTA::Chokepoint*> chokepoints = home->getChokepoints();
	double min_length = 10000;
	BWTA::Chokepoint* choke = NULL;

	//Iterate through all chokepoints and look for the one with the smallest gap (least width)
	for (std::set<BWTA::Chokepoint*>::iterator c = chokepoints.begin(); c != chokepoints.end(); c++) {
		double length = (*c)->getWidth();
		if (length < min_length || choke == NULL) {
			min_length = length;
			choke = *c;
		}
	}

	return choke->getCenter();
}

Base* AI::getClosestBase(Position p) {
	return mainBase;
}

void AI::initStartOrder(Base* base) {
	if (Broodwar->self()->getRace() == BWAPI::Races::Terran) {
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_Supply_Depot));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_Barracks));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_Marine, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_Marine, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_SCV, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_Supply_Depot, true));
		base->m_startOrder.push_back(Action(TRAIN_BUILD, UnitTypes::Terran_Marine));

	} else if (Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Probe });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Probe });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Probe });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Probe });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Pylon });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Probe });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Probe });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Probe });
		base->m_startOrder.push_back(Action{ TRAIN_BUILD, UnitTypes::Protoss_Gateway });
	} else {

	}
}

bool AI::canAfford(BWAPI::UnitType u) {
	return ((u.mineralPrice() + reservedMinerals <= Broodwar->self()->minerals()) && (u.gasPrice() + reservedGas <= Broodwar->self()->gas()));
}

BWAPI::Unit AI::getClosestMineral(BWAPI::Unit u) {
	return Broodwar->getClosestUnit(u->getPosition(), BWAPI::Filter::IsMineralField, 512);
}

void AI::onFrame() {

	BWAPI::Error e = Broodwar->getLastError();
	//if (e != BWAPI::Errors::Insufficient_Supply)
	//	Broodwar->printf("%ss", e.getName().c_str());

	drawStats();
	//Call every 100:th frame
	if (Broodwar->getFrameCount() % 100 == 0) {
		mainBase->controll();
	}

	//Draw lines around regions, chokepoints etc.
	if (analyzed) {
		drawTerrainData();
	}
}

void AI::onSendText(std::string text) {
	if (text == "/show players") {
		showPlayers();
	} else if (text == "/show forces") {
		showForces();
	} else {
		Broodwar->printf("You typed '%s'!", text.c_str());
		Broodwar->sendText("%s", text.c_str());
	}
}

void AI::onReceiveText(BWAPI::Player player, std::string text) {
	Broodwar->printf("%s said '%s'", player->getName().c_str(), text.c_str());
}

void AI::onPlayerLeft(BWAPI::Player player) {
	Broodwar->sendText("%s left the game.", player->getName().c_str());
}

void AI::onNukeDetect(BWAPI::Position target) {
	if (target != Positions::Unknown) {
		Broodwar->printf("Nuclear Launch Detected at (%d,%d)", target.x, target.y);
	} else {
		Broodwar->printf("Nuclear Launch Detected");
	}
}

void AI::onUnitDiscover(BWAPI::Unit unit) {
	//Broodwar->sendText("A %s [%x] has been discovered at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
	Base* closestBase = getClosestBase(unit->getPosition());
	int dist = closestBase->buildings.getPosition().getDistance(unit->getPosition());
	if (dist < 1024) {
		for (auto e : m_bases) {
			e->informAttack(closestBase, unit);
		}
	}
}

void AI::onUnitEvade(BWAPI::Unit unit) {
	//Broodwar->sendText("A %s [%x] was last accessible at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

void AI::onUnitShow(BWAPI::Unit unit) {
	//Broodwar->sendText("A %s [%x] has been spotted at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

void AI::onUnitHide(BWAPI::Unit unit) {
	//Broodwar->sendText("A %s [%x] was last seen at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}

void AI::onUnitCreate(BWAPI::Unit unit) {
	if (unit->getPlayer() == Broodwar->self()) {
		Base* base = getClosestBase(unit->getPosition());

		m_unitBase[unit] = base;
		base->addUnit(unit);

		if (unit->getType().isBuilding()) {
			reservedMinerals -= unit->getType().mineralPrice();
			reservedGas -= unit->getType().gasPrice();
		}
		
		base->m_waiting[unit->getType()]--;
	}
}

void AI::onUnitDestroy(BWAPI::Unit unit) {
	if (unit->getPlayer() == Broodwar->self()) {
		Base* base = m_unitBase[unit];
		if (base) {
			base->removeUnit(unit);
		} else {
			Broodwar->printf("Error removeing unit from base.");
		}

		//Broodwar->sendText("My unit %s [%x] has been destroyed at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
	} else {
		//Broodwar->sendText("Enemy unit %s [%x] has been destroyed at (%d,%d)", unit->getType().getName().c_str(), unit, unit->getPosition().x, unit->getPosition().y);
	}
}

void AI::onUnitMorph(BWAPI::Unit unit) {
	//Broodwar->sendText("A %s [%x] has been morphed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);

	if (unit->getPlayer() == Broodwar->self()) {
		Base* base = getClosestBase(unit->getPosition());
		m_unitBase[unit] = base;
		base->addUnit(unit);
		base->m_waiting[unit->getType()]--;
	}
}

void AI::onUnitRenegade(BWAPI::Unit unit) {
	//Broodwar->sendText("A %s [%x] is now owned by %s",unit->getType().getName().c_str(),unit,unit->getPlayer()->getName().c_str());
}

void AI::onSaveGame(std::string gameName) {
	//Broodwar->printf("The game was saved to \"%s\".", gameName.c_str());
}

DWORD WINAPI AnalyzeThread() {
	BWTA::analyze();

	//Self start location only available if the map has base locations
	if (BWTA::getStartLocation(BWAPI::Broodwar->self()) != NULL) {
		home = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
	}
	//Enemy start location only available if Complete Map Information is enabled.
	if (BWTA::getStartLocation(BWAPI::Broodwar->enemy()) != NULL) {
		enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
	}
	analyzed = true;
	analysis_just_finished = true;
	return 0;
}

void AI::drawStats() {
	int line = 0;
	int idle = 0;
	for (auto e : mainBase->workers) {
		if (e->isIdle())
			idle++;
	}
	Broodwar->drawTextScreen(5, 16 * line++, "Idle Workers: %d", idle);
	Broodwar->drawTextScreen(5, 16 * line++, "Workers: %d", mainBase->workers.size());
	Broodwar->drawTextScreen(5, 16 * line++, "Reserved Resources");
	Broodwar->drawTextScreen(5, 16 * line++, "- Minerals: %d", reservedMinerals);
	Broodwar->drawTextScreen(5, 16 * line++, "- Gas: %d", reservedGas);

	if (!mainBase->m_startOrder.empty()) {
		//int line = 0;
		Broodwar->drawTextScreen(5, 16 * line++, "Build Order Start");
		for (auto u : mainBase->m_startOrder) {
			if (u.m_type == TRAIN_BUILD) {
				Broodwar->drawTextScreen(5, 16 * line++, "-%ss", u.m_unitType.getName().c_str());
			}
		}
	} else {
		Broodwar->drawTextScreen(5, 16 * line++, "Base");
		std::map<UnitType, int> unitTypeCounts;
		for (auto u : mainBase->buildings) {
			if (unitTypeCounts.find(u->getType()) == unitTypeCounts.end()) {
				unitTypeCounts.insert(std::make_pair(u->getType(), 0));
			}
			unitTypeCounts.find(u->getType())->second++;
		}

		for (auto u : mainBase->army) {
			if (unitTypeCounts.find(u->getType()) == unitTypeCounts.end()) {
				unitTypeCounts.insert(std::make_pair(u->getType(), 0));
			}
			unitTypeCounts.find(u->getType())->second++;
		}

		for (std::map<UnitType, int>::iterator i = unitTypeCounts.begin(); i != unitTypeCounts.end(); i++) {
			Broodwar->drawTextScreen(5, 16 * line++, "- %d %ss", i->second, i->first.getName().c_str());
		}

		Broodwar->drawTextScreen(5, 16 * line++, "Demand");

		for (std::pair<BWAPI::UnitType, int> e : mainBase->m_currStage.m_subStage[mainBase->m_currStage.curSubStage].m_demand) {
			if (e.second - mainBase->m_current[e.first] != 0) {
				Broodwar->drawTextScreen(5, 16 * line++, "- %ss : %d (%d, %d, %d)", e.first.getName().c_str(), (e.second - mainBase->m_current[e.first]), e.second, mainBase->m_current[e.first], mainBase->m_waiting[e.first]);
			}
		}
	}
}

void AI::drawTerrainData() {
	//Iterate through all the base locations, and draw their outlines.
	for (auto bl : BWTA::getBaseLocations()) {
		TilePosition p = bl->getTilePosition();
		Position c = bl->getPosition();
		//Draw outline of center location
		Broodwar->drawBox(CoordinateType::Map, p.x * 32, p.y * 32, p.x * 32 + 4 * 32, p.y * 32 + 3 * 32, Colors::Blue, false);
		//Draw a circle at each mineral patch
		for (auto m : bl->getStaticMinerals()) {
			Position q = m->getInitialPosition();
			Broodwar->drawCircle(CoordinateType::Map, q.x, q.y, 30, Colors::Cyan, false);
		}
		//Draw the outlines of vespene geysers
		for (auto v : bl->getGeysers()) {
			TilePosition q = v->getInitialTilePosition();
			Broodwar->drawBox(CoordinateType::Map, q.x * 32, q.y * 32, q.x * 32 + 4 * 32, q.y * 32 + 2 * 32, Colors::Orange, false);
		}
		//If this is an island expansion, draw a yellow circle around the base location
		if (bl->isIsland()) {
			Broodwar->drawCircle(CoordinateType::Map, c.x, c.y, 80, Colors::Yellow, false);
		}
	}
	//Iterate through all the regions and draw the polygon outline of it in green.
	for (auto r : BWTA::getRegions()) {
		BWTA::Polygon p = r->getPolygon();
		for (int j = 0; j < (int)p.size(); j++) {
			Position point1 = p[j];
			Position point2 = p[(j + 1) % p.size()];
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Green);
		}
	}
	//Visualize the chokepoints with red lines
	for (auto r : BWTA::getRegions()) {
		for (auto c : r->getChokepoints()) {
			Position point1 = c->getSides().first;
			Position point2 = c->getSides().second;
			Broodwar->drawLine(CoordinateType::Map, point1.x, point1.y, point2.x, point2.y, Colors::Red);
		}
	}
}

void AI::showPlayers() {
	for (auto p : Broodwar->getPlayers()) {
		Broodwar->printf("Player [%d]: %s is in force: %s", p->getID(), p->getName().c_str(), p->getForce()->getName().c_str());
	}
}

void AI::showForces() {
	for (auto f : Broodwar->getForces()) {
		BWAPI::Playerset players = f->getPlayers();
		Broodwar->printf("Force %s has the following players:", f->getName().c_str());
		for (auto p : players) {
			Broodwar->printf("  - Player [%d]: %s", p->getID(), p->getName().c_str());
		}
	}
}

void AI::onUnitComplete(BWAPI::Unit unit) {
	//Broodwar->sendText("A %s [%x] has been completed at (%d,%d)",unit->getType().getName().c_str(),unit,unit->getPosition().x,unit->getPosition().y);
}
