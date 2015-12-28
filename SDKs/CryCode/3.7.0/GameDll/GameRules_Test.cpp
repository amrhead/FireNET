#include "StdAfx.h"
#include "GameRules.h"


using namespace GameTesting;

//--------------------------------------------------------------------------------
CRY_TEST_FIXTURE(GameRulesTestFixture, GameBaseTestFixture, GameTestSuite)
{
	CGameRules &GetGameRules() { return dummyGameRules; }

private:

	CGameRules dummyGameRules;
};

//--------------------------------------------------------------------------------
CRY_TEST_WITH_FIXTURE(TestGameRulesOnCollision, GameRulesTestFixture)
{
	IGameRules::SGameCollision dummyEvent;
	dummyEvent.pCollision = NULL;

	ASSERT_IS_TRUE(GetGameRules().OnCollision(dummyEvent));
}
