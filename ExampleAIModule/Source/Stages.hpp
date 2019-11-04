#pragma once
#include <BWAPI.h>
#include <map>
#include <string>

struct SubStage {
	std::unordered_map<int, int> m_demand;

};

struct Stage {
	std::vector<SubStage> m_subStage;
	int curSubStage = 0;

	void update(std::unordered_map<int, int> &m_current);
};

class StageHolder {

public:
	static void InitStages();
	static Stage getStage();

};