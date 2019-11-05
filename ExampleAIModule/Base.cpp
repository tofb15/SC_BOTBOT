#include "Base.h"
#include "Utills.h"
#include "Source/ExampleAIModule.h"



Base::Base() {


}

void Base::init(AI* ai) {

	StageHolder::InitStages();
	m_currStage = StageHolder::getStage();

	if (isInited)
		return;

	m_ai = ai;
	isInited = true;

	//if (Broodwar->self()->getRace() == BWAPI::Races::Terran) {
	//	m_demand[BWAPI::UnitTypes::Terran_SCV] = 20;
	//	m_demand[BWAPI::UnitTypes::Terran_Supply_Depot] = 5;
	//	m_demand[BWAPI::UnitTypes::Terran_Barracks] = 2;
	//	m_demand[BWAPI::UnitTypes::Terran_Marine] = 50;
	//	m_demand[BWAPI::UnitTypes::Terran_Refinery] = 1;
	//} else if (Broodwar->self()->getRace() == BWAPI::Races::Protoss) {
	//	m_demand[BWAPI::UnitTypes::Protoss_Probe] = 20;
	//	m_demand[BWAPI::UnitTypes::Protoss_Pylon] = 5;
	//	m_demand[BWAPI::UnitTypes::Protoss_Gateway] = 2;
	//	m_demand[BWAPI::UnitTypes::Protoss_Zealot] = 50;
	//	m_demand[BWAPI::UnitTypes::Protoss_Assimilator] = 1;
	//} else {
	//	m_demand[BWAPI::UnitTypes::Zerg_Drone] = 20;
	//	m_demand[BWAPI::UnitTypes::Zerg_Overlord] = 5;
	//	m_demand[BWAPI::UnitTypes::Zerg_Hatchery] = 2;
	//	m_demand[BWAPI::UnitTypes::Zerg_Zergling] = 50;
	//	m_demand[BWAPI::UnitTypes::Zerg_Extractor] = 1;
	//}
}

void Base::addUnit(BWAPI::Unit& u, bool justBuilt) {
	if (u->getType().getRace() == BWAPI::Races::Zerg)
		Broodwar->printf("Added: %ss", u->getType().getName().c_str());

	if (u->getType() == BWAPI::UnitTypes::Zerg_Larva || u->getType() == BWAPI::UnitTypes::Zerg_Egg) {
		m_waiting[u->getType()]++;
		return;
	}

	all.insert(u);

	if (u->getType().isWorker()) {
		workers.insert(u);
		u->gather(m_ai->getClosestMineral(u));
	} else if (u->getType().isBuilding()) {
		buildings.insert(u);
		if (u->isBeingConstructed()) {
			m_buildingIssued.erase(u->getBuildUnit());
		}
	} else {
		army.insert(u);
	}

	m_current[u->getType()]++;
	//if (justBuilt) {
	//	m_waiting[u->getType()]--;
	//}
}

void Base::requestBuild(BWAPI::UnitType u) {
	if (m_waiting[u] < 1 && m_currStage.m_subStage[m_currStage.curSubStage].m_demand[u] - m_current[u] < 1) {
		bool constructing = false;
		for (auto e : buildings) {
			if (e->getType() == u && e->isBeingConstructed()) {
				constructing = true;
				break;
			}
		}

		if (!constructing) {
			m_currStage.m_subStage[m_currStage.curSubStage].m_demand[u]++;
		}
	}
}

void Base::removeUnit(BWAPI::Unit& u) {
	all.erase(u);
	buildings.erase(u);
	army.erase(u);
	workers.erase(u);

	m_current[u->getType()]--;
}

BWAPI::Unit Base::getWorker() {
	for (auto e : workers) {
		if (e->isIdle() && e->isCompleted()) {
			return e;
		}
	}

	for (auto e : workers) {
		if (e->isGatheringMinerals() && e->isCompleted()) {
			return e;
		}
	}

	for (auto e : workers) {
		if (!e->isConstructing() && e->isCompleted()) {
			return e;
		}
	}
}

bool Base::buildBuilding(BWAPI::UnitType u) {

	if (!m_ai->canAfford(u))
		return false;

	int i = 0;
	TilePosition buildpos = TilePositions::Invalid;
	while (buildpos == TilePositions::Invalid && i < 5) {
		buildpos = Broodwar->getBuildLocation(u, TilePosition(buildings.getPosition()), 64 + i * 20);
		i++;
	}

	if (buildpos != TilePositions::Invalid) {

		BWAPI::Unit worker = getWorker();

		if (worker->build(u, buildpos)) {
			Broodwar->printf("Reserving for: %ss", u.getName().c_str());
			m_ai->reservedMinerals += u.mineralPrice();
			m_ai->reservedGas += u.gasPrice();

			m_buildingIssued[worker] = u;

			m_waiting[u]++;
			return true;
		}
	} else {
		Broodwar->printf("No valid build position found.");
	}
	return false;
}

bool Base::buildAddon(BWAPI::UnitType u) {

	if (!m_ai->canAfford(u))
		return false;

	for (auto e : buildings) {
		if (e->canBuildAddon(u)) {
			Broodwar->printf("Reserving for: %ss", u.getName().c_str());
			m_ai->reservedMinerals += u.mineralPrice();
			m_ai->reservedGas += u.gasPrice();

			//m_buildingIssued[worker] = u;

			e->buildAddon(u);

			m_waiting[u]++;
			return true;
		}
	}

	return false;
}

BWAPI::Unit Base::getClosestMineral() {
	return Broodwar->getClosestUnit(buildings.getPosition(), BWAPI::Filter::IsMineralField);
}

bool Base::trainUnit(BWAPI::UnitType u) {
	if (!m_ai->canAfford(u))
		return false;

	for (auto e : buildings) {
		if (e->canTrain(u)) {
			if (e->getTrainingQueue().size() < 2) {
				if (e->train(u)) {
					m_waiting[u]++;
					return true;
				}
			}

		}
	}

	return false;
}

void Base::controll() {

	m_currStage.update(m_current);

	for (std::pair<BWAPI::Unit, BWAPI::UnitType> e : m_buildingIssued) {
		if (!e.first->isConstructing()) {

			TilePosition buildpos = Broodwar->getBuildLocation(e.second, TilePosition(buildings.getPosition()));
			if (buildpos != TilePositions::Invalid) {
				e.first->build(e.second, buildpos);
			}
		}
	}

	BWTA::Chokepoint* cp = BWTA::getNearestChokepoint(buildings.getPosition());
	Position p;
	if (Broodwar->getRegionAt(cp->getSides().first)->isHigherGround()) {
		p = cp->getSides().first;
	} else {
		p = cp->getSides().second;
	}
	army.attack(p, false);

	if (stage == 0) {
		if (m_startOrder.empty()) {
			stage++;
		} else {
			bool b = false;
			bool stop = false;

			for (int i = 0; !stop && i < m_startOrder.size(); i++) {
				Action a = m_startOrder[i];
				if (a.m_type == TRAIN_BUILD) {
					if (!a.m_unitType.isBuilding()) {
						b = trainUnit(a.m_unitType);
					} else if (!a.m_unitType.isAddon()) {
						if (a.m_unitType == UnitTypes::Terran_Machine_Shop) {
							Broodwar->printf("Terran_Machine_Shop");
						}
						b = buildBuilding(a.m_unitType);
					} else {
						Broodwar->printf("Addon Found");
						b = buildAddon(a.m_unitType);
					}
				}

				if (b) {
					m_startOrder.erase(m_startOrder.begin() + i);
					stop = true;
				} else if (!m_startOrder[i].m_allowSkip) {
					stop = true;
				}
			}
		}
	} else {
		for (std::pair<BWAPI::UnitType, int> e : m_currStage.m_subStage[m_currStage.curSubStage].m_demand) {
			if (e.second > m_current[e.first] + m_waiting[e.first]) {
				if (e.first == UnitTypes::Resource_Vespene_Geyser) {
					for (auto b : buildings) {
						if (b->getType().isRefinery() && b->isCompleted()) {
							Broodwar->printf("Ref");
							Unit w = getWorker();
							w->gather(b);
							break;
						}
					}

				} else if (e.first == UnitTypes::Buildings) {
					expand();
				} else if (!e.first.isBuilding()) {
					//if (e.first == UnitTypes::Terran_Machine_Shop) {
					//	Broodwar->printf("Terran_Machine_Shop");
					//}
					trainUnit(e.first);
				} else if (!e.first.isAddon()) {
					buildBuilding(e.first);
				} else {
					buildAddon(e.first);
				}
			} else {
				//Broodwar->sendText("Im Fine");
			}
		}
	}

	int gasWorkers = 0;
	for (auto u : workers) {
		if (u->isIdle()) {
			u->gather(getClosestMineral());
		} else if (u->isGatheringGas()) {
			gasWorkers++;
		}
	}
	m_current[UnitTypes::Resource_Vespene_Geyser] = gasWorkers;
}

void Base::informAttack(Base* base, BWAPI::Unit u) {
	//if (base == this) {
	//	m_baseStatus = Attacked;
	//	army.attack(u->getPosition());
	//} else if (m_baseStatus != Attacked) {
	//	army.attack(u->getPosition());
	//}
}

void Base::enemyDetected(BWAPI::Unit u) {
	if (m_closestEnemy->isVisible()) {
		if (u->getPosition().getDistance(buildings.getPosition()) < m_closestEnemy->getPosition().getDistance(buildings.getPosition())) {
			m_closestEnemy = u;
		}
	} else {
		m_closestEnemy = u;
	}
}

void Base::expand() {

}
