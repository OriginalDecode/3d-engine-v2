#pragma once
#include "core/Types.h"
#include "core/containers/GrowingArray.h"

class State;
typedef Core::GrowingArray<State*> SubStateContainer;
typedef Core::GrowingArray<SubStateContainer> MainStateContainer;
class StateStack
{
public:
	StateStack() = default;

	enum EGameStateType
	{
		MAIN,
		SUB
	};

	void PopCurrentMainState();
	void PopCurrentSubState();
	void PushState(State* game_state, EGameStateType game_state_type);
	void PauseCurrentState();
	void ResumeCurrentState();
	// void PopState(u32 state_id);

	bool UpdateCurrentState(float dt);

	void Clear();

private:
	MainStateContainer m_GameStates;
	int32 m_MainIndex = 0;
	int32 m_SubIndex = 0;
};
