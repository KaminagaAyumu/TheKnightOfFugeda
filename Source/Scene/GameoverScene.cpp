#include "GameoverScene.h"
#include "GameScene.h"
#include "TitleScene.h"
#include "SelectScene.h"
#include "SceneController.h"
#include "../Utility/Game.h"
#include "../Utility/Input.h"
#include "../MyLib/Physics.h"
#include "../MyLib/ObjectFactory.h"
#include "../MyLib/ObjectManager.h"
#include "../Common/Effect/EffectManager.h"
#include "DxLib.h"

namespace
{
	// シーン遷移関連
	constexpr int kFadeInterval = 60; // フェードを行う時間
	constexpr int kMaxFadeRate = 255; // フェード進行率の最大値
	constexpr unsigned int kFadeInColor = 0xffffff; // フェードインの色
	constexpr unsigned int kFadeOutColor = 0xffffff; // フェードアウトの色

	constexpr int kCursorMoveIndex = 1;	// カーソルが動く値

	const Vector2Int kSelectListPos = { Game::kScreenWidth / 2, Game::kScreenHeight - 350 };
	const Vector2Int kSelectListSize = { Game::kScreenWidth / 3, 250 };
}

GameoverScene::GameoverScene(SceneController& controller, int stageNo) :
	SceneBase(controller),
	m_update(&GameoverScene::FadeInUpdate),
	m_draw(&GameoverScene::FadeDraw),
	m_stageNo(stageNo)
{
}

GameoverScene::~GameoverScene()
{
}

void GameoverScene::Init()
{
	m_frameCount = kFadeInterval;
	m_fadeColor = kFadeInColor;

	m_pUISelectList = MyLib::ObjectFactory::CreateUISelectList(kSelectListPos, MyLib::Renderer::FontType::Small);
	m_pUISelectList->Init();
	m_pSelectList = m_pUISelectList->GetComponent<MyLib::UISelectList>();
	if (auto selectList = m_pSelectList.lock())
	{

		selectList->SetSize(kSelectListSize);

		selectList->AddOption(L"リトライ", [this]()
			{
				// 最後のゲームシーンに戻る
				MyLib::ObjectManager::GetInstance().End();
				m_sceneController.ResetScene(std::make_shared<GameScene>(m_sceneController, m_stageNo));
			});
		selectList->AddOption(L"ステージセレクトに戻る", [this]()
			{
				MyLib::ObjectManager::GetInstance().End();
				m_sceneController.ResetScene(std::make_shared<SelectScene>(m_sceneController));

			});
		selectList->AddOption(L"タイトルに戻る", [this]()
			{
				MyLib::ObjectManager::GetInstance().End();
				m_sceneController.ResetScene(std::make_shared<TitleScene>(m_sceneController));
			});
	}

	MyLib::ObjectManager::GetInstance().SetUpdate(false);

	MyLib::Physics::GetInstance().StopUpdate();

	EffectManager::GetInstance().StopUpdate();
}

void GameoverScene::End()
{
	m_pUISelectList->End();

	MyLib::ObjectManager::GetInstance().SetUpdate(true);

	MyLib::Physics::GetInstance().StartUpdate();

	EffectManager::GetInstance().StartUpdate();
}

void GameoverScene::Update()
{
	(this->*m_update)();
}

void GameoverScene::Draw() const
{
	(this->*m_draw)();
}

void GameoverScene::FadeInUpdate()
{
	m_frameCount--;

	if (m_frameCount <= 0)
	{
		m_update = &GameoverScene::NormalUpdate;
		m_draw = &GameoverScene::NormalDraw;
		return;
	}
}

void GameoverScene::FadeOutUpdate()
{
	m_frameCount++;

	if (m_frameCount >= kFadeInterval)
	{
		//m_sceneController.ResetScene(std::make_shared<GameScene>(m_sceneController));

		auto selectList = m_pSelectList.lock();
		selectList->TriggerSelect();
		return;
	}
}

void GameoverScene::NormalUpdate()
{
	Input& input = Input::GetInstance();

	if (input.IsTriggered("Up"))
	{
		auto selectList = m_pSelectList.lock();
		selectList->MoveCursor(-kCursorMoveIndex);
	}

	if (input.IsTriggered("Down"))
	{
		auto selectList = m_pSelectList.lock();
		selectList->MoveCursor(kCursorMoveIndex);
	}


	if (input.IsTriggered("OK"))
	{
		m_update = &GameoverScene::FadeOutUpdate;
		m_draw = &GameoverScene::FadeDraw;

		//auto selectList = m_pSelectList.lock();
		return;
	}
}

void GameoverScene::FadeDraw() const
{
	// フェード率の計算 開始時: 0.0f  終了時: 1.0f
	auto rate = static_cast<float>(m_frameCount) / static_cast<float>(kFadeInterval);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(kMaxFadeRate * rate));
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, m_fadeColor, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameoverScene::NormalDraw() const
{
}