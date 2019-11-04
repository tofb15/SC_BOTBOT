#pragma once
#include <windows.h>
#include <BWAPI.h>
#include <BWTA.h>

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

using namespace BWAPI;
using namespace BWTA;
struct Base;

class AI : public BWAPI::AIModule
{
public:
	Base* mainBase;
	std::vector<Base*> m_bases;

	std::unordered_map<BWAPI::Unit, Base*> m_unitBase;
	int reservedMinerals = 0;
	int reservedGas = 0;

	//Methods inherited from BWAPI:AIModule
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);

	//Own methods
	void drawStats();
	void drawTerrainData();
	void showPlayers();
	void showForces();
	Position findGuardPoint();
	Base* getClosestBase(Position p);
	void initStartOrder(Base* base);
	bool canAfford(BWAPI::UnitType u);

	BWAPI::Unit getClosestMineral(BWAPI::Unit u);

};
