#include "ResultScene.h"
#include "TitleScene.h"
#include "SelectScene.h"
#include "GameScene.h"
#include "SceneController.h"
#include "../Utility/Input.h"
#include "../Common/Model.h"
#include "../Geometry/Vector3.h"
#include "../Utility/Binary/TerrainResource.h"
#include "../Utility/Game.h"
#include "../MyLib/ObjectFactory.h"
#include "../MyLib/ObjectManager.h"
#include "../Common/Effect/EffectManager.h"
#include "../Common/Sound/SoundManager.h"
#include "../Object/Skybox.h"
#include <cassert>
#include "DxLib.h"

namespace
{
	// シーン遷移関連
	constexpr int kFadeInterval = 60; // フェードを行う時間
	constexpr int kMaxFadeRate = 255; // フェード進行率の最大値
	constexpr unsigned int kFadeInColor = 0xffffff; // フェードインの色
	constexpr unsigned int kFadeOutColor = 0xffffff; // フェードアウトの色

	constexpr int kCursorMoveIndex = 1;	// カーソルが動く値

	const Vector2Int kTextPos = { Game::kScreenWidth / 2, 80 };
	const Vector2Int kTextTimePos = { Game::kScreenWidth / 2, 220 };

	const Vector2Int kSelectListPos = { Game::kScreenWidth / 2, Game::kScreenHeight - 200 };
	const Vector2Int kSelectListSize = { 500, 300 };
}

ResultScene::ResultScene(SceneController& controller, int gameTime, int stageNo) :
	SceneBase(controller),
	m_update(&ResultScene::FadeInUpdate),
	m_draw(&ResultScene::FadeDraw),
	m_gameTime(gameTime),
	m_stageNo(stageNo)
{
}

ResultScene::~ResultScene()
{
	// 念のため終了処理を呼ぶようにする
	//End();
}

void ResultScene::Init()
{
	m_frameCount = kFadeInterval;
	m_fadeColor = kFadeInColor;
	//printfDx("ResultScene:初期化");

	m_pSkybox = std::dynamic_pointer_cast<Skybox>(MyLib::ObjectFactory::CreateSkybox());
	m_pSkybox->Init(Skybox::Type::Evening);

	m_pUIText = MyLib::ObjectFactory::CreateUIText(kTextPos, MyLib::Renderer::FontType::Header);
	m_pUIText->Init();
	m_pText = m_pUIText->GetComponent<MyLib::UIText>();
	if (auto text = m_pText.lock())
	{
		text->SetTextColor(0xaa4400);
		text->SetText(L"クリア!");
	}
	
	m_pUITextTime = MyLib::ObjectFactory::CreateUIText(kTextTimePos, MyLib::Renderer::FontType::Large);
	m_pUITextTime->Init();
	m_pTextTime = m_pUITextTime->GetComponent<MyLib::UIText>();
	if (auto text = m_pTextTime.lock())
	{
		text->SetTextColor(0xaa4400);
		text->SetText(L"クリアタイム:" + std::to_wstring(m_gameTime / 60));
	}

	m_pUISelectList = MyLib::ObjectFactory::CreateUISelectList(kSelectListPos, MyLib::Renderer::FontType::Small);
	m_pUISelectList->Init();
	m_pSelectList = m_pUISelectList->GetComponent<MyLib::UISelectList>();
	if (auto selectList = m_pSelectList.lock())
	{

		selectList->SetSize(kSelectListSize);

		selectList->AddOption(L"リトライ", [this]()
			{
				// ゲームシーンに戻る
				m_sceneController.ChangeScene(std::make_shared<GameScene>(m_sceneController, m_stageNo));
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

	auto& soundManager = SoundManager::GetInstance();
	soundManager.LoadSoundClip("TitleBGM", L"Data/File/Sound/BGM/title.ogg", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("SelectBGM", L"Data/File/Sound/BGM/select.mp3", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("GameBGM", L"Data/File/Sound/BGM/stage1.ogg", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("ResultBGM", L"Data/File/Sound/BGM/result.mp3", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("OK", L"Data/File/Sound/SE/ok.mp3", SoundBus::SE, 1.0f, false);
	soundManager.LoadSoundClip("Cursor", L"Data/File/Sound/SE/cursor.mp3", SoundBus::SE, 1.0f, false);
	soundManager.LoadSoundClip("Open", L"Data/File/Sound/SE/open.mp3", SoundBus::SE, 1.0f, false);
}

void ResultScene::End()
{
	//printfDx("ResultScene:終了");
	m_pSkybox->End();
	m_pUIText->End();
	m_pUITextTime->End();
	m_pUISelectList->End();

	auto& soundManager = SoundManager::GetInstance();
	soundManager.DeleteSoundClip("TitleBGM");
	soundManager.DeleteSoundClip("SelectBGM");
	soundManager.DeleteSoundClip("GameBGM");
	soundManager.DeleteSoundClip("ResultBGM");
	soundManager.DeleteSoundClip("OK");
	soundManager.DeleteSoundClip("Cursor");
	soundManager.DeleteSoundClip("Open");
}

void ResultScene::Update()
{
	(this->*m_update)();
}

void ResultScene::Draw() const
{
	(this->*m_draw)();
#ifdef _DEBUG
	DrawString(0, 0, L"ResultScene", GetColor(255, 255, 255));
	DrawFormatString(0, 16, GetColor(255, 255, 255), L"FRAME:%d", m_frameCount);
#endif // _DEBUG

}

void ResultScene::FadeInUpdate()
{
	m_frameCount--;

	if (m_frameCount <= 0)
	{
		m_update = &ResultScene::NormalUpdate;
		m_draw = &ResultScene::NormalDraw;
		return;
	}
}

void ResultScene::FadeOutUpdate()
{
	m_frameCount++;

	if (m_frameCount >= kFadeInterval)
	{
		auto selectList = m_pSelectList.lock();

		if (selectList->IsMatchedCursor(L"リトライ"))
		{
			SoundManager::GetInstance().CrossFadeBGM("GameBGM", 1.0f);
		}
		else if (selectList->IsMatchedCursor(L"ステージセレクトに戻る"))
		{
			SoundManager::GetInstance().CrossFadeBGM("SelectBGM", 1.0f);
		}
		else if (selectList->IsMatchedCursor(L"タイトルに戻る"))
		{
			SoundManager::GetInstance().CrossFadeBGM("TitleBGM", 1.0f);
		}

		selectList->TriggerSelect();
		return;
	}
}

void ResultScene::NormalUpdate()
{
	Input& input = Input::GetInstance();

	if (input.IsTriggered("Up"))
	{
		SoundManager::GetInstance().Play("Cursor", 1.0f, true);
		auto selectList = m_pSelectList.lock();
		selectList->MoveCursor(-kCursorMoveIndex);
	}

	if (input.IsTriggered("Down"))
	{
		SoundManager::GetInstance().Play("Cursor", 1.0f, true);
		auto selectList = m_pSelectList.lock();
		selectList->MoveCursor(kCursorMoveIndex);
	}


	if (input.IsTriggered("OK"))
	{
		// 決定した際のSEを再生
		SoundManager::GetInstance().Play("OK", 1.0f, false);
		m_update = &ResultScene::FadeOutUpdate;
		m_draw = &ResultScene::FadeDraw;
		return;
	}
}

void ResultScene::FadeDraw() const
{
	// フェード率の計算 開始時: 0.0f  終了時: 1.0f
	auto rate = static_cast<float>(m_frameCount) / static_cast<float>(kFadeInterval);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(kMaxFadeRate * rate));
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, m_fadeColor, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void ResultScene::NormalDraw() const
{
}