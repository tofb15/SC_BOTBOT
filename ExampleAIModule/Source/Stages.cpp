#include "Stages.hpp"

using namespace BWAPI;
static std::unordered_map<std::string, Stage> m_stages;


void StageHolder::InitStages() {

	if (!m_stages.empty())
		return;

	Stage stage;
	SubStage sub;
	//1
	sub.m_demand[UnitTypes::Terran_SCV] = 10;
	sub.m_demand[UnitTypes::Terran_Barracks] = 1;
	sub.m_demand[UnitTypes::Terran_Supply_Depot] = 3;
	sub.m_demand[UnitTypes::Terran_Marine] = 3;
	stage.m_subStage.push_back(sub);
	//2
	sub.m_demand[UnitTypes::Terran_SCV] = 29;
	sub.m_demand[UnitTypes::Terran_Barracks] = 2;
	sub.m_demand[UnitTypes::Terran_Academy] = 1;
	sub.m_demand[UnitTypes::Terran_Supply_Depot] = 5;
	sub.m_demand[UnitTypes::Terran_Marine] = 15;
	sub.m_demand[UnitTypes::Terran_Refinery] = 1;
	sub.m_demand[UnitTypes::Resource_Vespene_Geyser] = 3;
	stage.m_subStage.push_back(sub);
	//3
	sub.m_demand[UnitTypes::Terran_SCV] = 29;
	sub.m_demand[UnitTypes::Terran_Barracks] = 2;
	sub.m_demand[UnitTypes::Terran_Academy] = 1;
	sub.m_demand[UnitTypes::Terran_Supply_Depot] = 8;
	sub.m_demand[UnitTypes::Terran_Marine] = 20;
	sub.m_demand[UnitTypes::Terran_Medic] = 5;
	sub.m_demand[UnitTypes::Terran_Firebat] = 10;
	sub.m_demand[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 2;
	sub.m_demand[UnitTypes::Terran_Factory] = 1;
	sub.m_demand[UnitTypes::Terran_Machine_Shop] = 1;
	sub.m_demand[UnitTypes::Buildings] = 1;
	stage.m_subStage.push_back(sub);
	//4
	sub.m_demand[UnitTypes::Terran_SCV] = 29;
	sub.m_demand[UnitTypes::Terran_Barracks] = 4;
	sub.m_demand[UnitTypes::Terran_Academy] = 1;
	sub.m_demand[UnitTypes::Terran_Supply_Depot] = 13;
	sub.m_demand[UnitTypes::Terran_Marine] = 30;
	sub.m_demand[UnitTypes::Terran_Medic] = 5;
	sub.m_demand[UnitTypes::Terran_Firebat] = 25;
	sub.m_demand[UnitTypes::Terran_Siege_Tank_Tank_Mode] = 15;
	sub.m_demand[UnitTypes::Terran_Factory] = 2;

	stage.m_subStage.push_back(sub);

	m_stages["General"] = stage;
}

Stage StageHolder::getStage() {
	return m_stages["General"];
}

void Stage::update(std::unordered_map<int, int>& m_current) {

	int left = 0;

	for (size_t i = curSubStage; i < m_subStage.size(); i++) {
		left = 0;
		for (std::pair<int, int> e : m_subStage[i].m_demand) {
			if (e.second > m_current[e.first]) {
				left++;
				if (left >= 3) {
					curSubStage = i;
					return;
				}
			}
		}
	}

	curSubStage = m_subStage.size() - 1;
	return;
}
