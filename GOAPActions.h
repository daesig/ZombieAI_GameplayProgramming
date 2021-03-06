#pragma once
#include <unordered_map>
#include "SteeringHelpers.h"
#include "IExamInterface.h"
#include "structs.h"
#include <unordered_map>

class Agent;
class Blackboard;
class GOAPPlanner;
class IExamInterface;
class WorldState;

class GOAPAction
{
public:
	GOAPAction(GOAPPlanner* pPlanner, const std::string& effectName);
	virtual ~GOAPAction();

	// Plan the action
	virtual bool Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) { return true; };
	// Prepare the action
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) {};
	// Perform the action
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt) { return true; };

	std::vector<GOAPProperty*> GetPreconditions() { return m_Preconditions; };
	std::vector<GOAPProperty*> GetEffects() { return m_Effects; };
	bool HasEffect(GOAPProperty* pPrecondition);

	float GetCost()const { return m_Cost; };
	virtual Elite::Vector2 GetMoveLocation() { return moveTarget.Position; };

	virtual bool RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const { return false; }; // If yes, the GoTo state will be ran before going into Perform
	virtual bool IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const { return true; };

	virtual std::string ToString() { return m_EffectName; };
protected:
	std::string m_EffectName{ "Undefined effect" };
	WorldState* m_pWorldState = nullptr;
	// Conditions that have to be met in order to start this action
	std::vector<GOAPProperty*> m_Preconditions;
	// Effects that will be applied to the world state after completing this action
	std::vector<GOAPProperty*> m_Effects;

	// Cost of the action
	float m_Cost = 10.f;

	// Requires the FSM to move to a location before performing
	bool m_RequiresMovement = false;
	TargetData moveTarget{};

	virtual void InitPreConditions(GOAPPlanner* pPlanner) = 0;
	virtual void InitEffects(GOAPPlanner* pPlanner) = 0;
	virtual void ApplyEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;
	virtual bool CheckPreConditions(GOAPPlanner* pPlanner) const { return true; }; // List of pre defined pre conditions that have to be met
	virtual bool CheckProceduralPreconditions(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) { return true; }; // Procedually check the world for conditions

	// Cleanup
	virtual void Cleanup();
};

class GOAPSurvive final : public GOAPAction
{
public:
	GOAPSurvive(GOAPPlanner* pPlanner);
	virtual bool Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
private:
	virtual void InitPreConditions(GOAPPlanner* pPlanner);
	virtual void InitEffects(GOAPPlanner* pPlanner);
};

class GOAPConsumeFood final : public GOAPAction
{
public:
	GOAPConsumeFood(GOAPPlanner* pPlanner);
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt);
	virtual bool IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;
private:
	virtual void InitPreConditions(GOAPPlanner* pPlanner);
	virtual void InitEffects(GOAPPlanner* pPlanner);
	virtual void ApplyEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;

	bool m_Consumed = false;
};

class GOAPConsumeMedkit final : public GOAPAction
{
public:
	GOAPConsumeMedkit(GOAPPlanner* pPlanner);
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt);
	virtual bool IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;
private:
	virtual void InitPreConditions(GOAPPlanner* pPlanner);
	virtual void InitEffects(GOAPPlanner* pPlanner);
	virtual void ApplyEffects(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;

	bool m_Consumed = false;
};

class GOAPSearchItem : public GOAPAction
{
public:
	GOAPSearchItem(GOAPPlanner* pPlanner, const std::string effectName = "GOAPSearchItem");
	virtual bool Plan(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt);
	virtual bool RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const { return false; };
	virtual bool IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;
protected:
	std::vector<Elite::Vector2>* m_pHouseCornerLocations = nullptr;
	std::vector<ExploredHouse>* m_pHouseLocations = nullptr;
	std::list<EntityInfo>* m_pItemsOnGround = nullptr;
	Agent* m_pAgent = nullptr;
private:
	Elite::Vector2 m_selectedLocation{};
	float m_ArrivalRange = 3.5f;
	int m_ItemsToLootBeforeHouseRevisit = 28;
	Elite::Vector2 m_HouseGoalPos{};
	std::vector<SpottedPurgeZone> m_PurgesZonesInSight{};

	Elite::Vector2 m_ItemLootedPosition{};

	float m_ChooseSeekLocationTimer{};
	float m_ChooseSeekLocationTime{ .25f };

	ExploredHouse* m_AgentHouse = nullptr;

	// testing
	float m_IsDoneTime = 4.f;
	float m_IsDoneTimer = 0.f;

	virtual void InitPreConditions(GOAPPlanner* pPlanner);
	virtual void InitEffects(GOAPPlanner* pPlanner);
	void ChooseSeekLocation(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	bool CheckArrival(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	void RemoveExploredCornerLocations(HouseInfo& houseInfo);
};

class GOAPSearchForFood final : public GOAPSearchItem
{
public:
	GOAPSearchForFood(GOAPPlanner* pPlanner);
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt);
	virtual bool RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const { return false; };
	virtual bool IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;
private:
	virtual void InitPreConditions(GOAPPlanner* pPlanner);
	virtual void InitEffects(GOAPPlanner* pPlanner);
};

class GOAPSearchForMedkit final : public GOAPSearchItem
{
public:
	GOAPSearchForMedkit(GOAPPlanner* pPlanner);
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard);
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt);
	virtual bool RequiresMovement(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const { return false; };
	virtual bool IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const;
private:
	virtual void InitPreConditions(GOAPPlanner* pPlanner);
	virtual void InitEffects(GOAPPlanner* pPlanner);
};

class GOAPFastHouseScout final : public GOAPAction
{
public:
	GOAPFastHouseScout(GOAPPlanner* pPlanner);
	virtual void Setup(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) override;
	virtual bool Perform(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard, float dt) override;
	virtual bool IsDone(IExamInterface* pInterface, GOAPPlanner* pPlanner, Blackboard* pBlackboard) const override;
private:
	virtual void InitPreConditions(GOAPPlanner* pPlanner) override;
	virtual void InitEffects(GOAPPlanner* pPlanner) override;

	std::vector<ExploredHouse>* m_pHouseLocations = nullptr;
	std::vector<Elite::Vector2>* m_pHouseCornerLocations = nullptr;
	int m_PositionsToCheck{ 36 };
	int m_Cycles{ 3 };
	float m_OffcycleAngleOffset{ 5.f };
	float m_DistanceFromAgent{ 25.f };
	float m_DistanceIncreasePerCycle{ 45.f };
	float m_IgnoreLocationDistance{ 5.f };
	WorldInfo m_WorldInfo{};

	// Debug
	std::vector<Line>* m_pScoutedVectors = nullptr;
};