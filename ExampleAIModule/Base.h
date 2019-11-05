#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include "Source/Stages.hpp"

class AI;

enum ActionType {
	TRAIN_BUILD,
};

struct Action {
	Action(ActionType type, BWAPI::UnitType unitType, bool allowSkip = false) : m_type(type), m_unitType(unitType), m_allowSkip(allowSkip){}

	ActionType m_type;
	BWAPI::UnitType m_unitType;
	bool m_allowSkip = false;
};

enum BaseStatus {
	Functional,
	Attacked,
	Lost,
};

struct Base {
	AI* m_ai;

	int stage = 0;
	BaseStatus m_baseStatus = Functional;

	BWAPI::Unitset workers;
	BWAPI::Unitset army;
	BWAPI::Unitset buildings;
	BWAPI::Unitset all;
	BWAPI::Unit m_closestEnemy;

	std::vector<Action> m_startOrder;
	std::unordered_map<BWAPI::Unit, BWAPI::UnitType> m_buildingIssued;

	Stage m_currStage;
	std::unordered_map<int, int> m_current;
	std::unordered_map<int, int> m_waiting;
	std::unordered_map<BWAPI::Unit, Action> m_BuildActions;

	bool isInited = false;

	Base();
	void init(AI* ai);
	void addUnit(BWAPI::Unit& u, bool justBuilt = true);
	void requestBuild(BWAPI::UnitType u);

	void removeUnit(BWAPI::Unit& u);
	BWAPI::Unit getWorker();
	bool buildBuilding(BWAPI::UnitType u);
	bool buildAddon(BWAPI::UnitType u);
	BWAPI::Unit getClosestMineral();

	bool trainUnit(BWAPI::UnitType u); 
	void controll();
	void informAttack(Base* base, BWAPI::Unit u);
	void enemyDetected(BWAPI::Unit u);
	void expand();
};
