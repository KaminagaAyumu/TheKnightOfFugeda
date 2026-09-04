#include "GameScene.h"
#include "ResultScene.h"
#include "PauseScene.h"
#include "GameoverScene.h"
#include "LoadingScene.h"
#include "../Utility/Input.h"
#include "SceneController.h"
#include "../Utility/File/FileManager.h"
#include "../Utility/File/ImageFile.h"
#include "../Utility/CSV/PlayerAttackResource.h"
#include "../Utility/CSV/TextResource.h"
#include "../Utility/CSV/EventResource.h"
#include "../Object/Player/Player.h"
#include "../Object/Enemy/EnemyManager.h"
#include "../Object/Camera/PlayerCamera.h"
#include "../Object/UI/LockOnMarkerUI.h"
#include "../Object/UI/LifeUI.h"
#include "../Object/Stage.h"
#include "../Object/Skybox.h"
#include "../MyLib/MyMath.h"
#include "../MyLib/MyString.h"
#include "../MyLib/Renderer.h"
#include "../MyLib/ObjectFactory.h"
#include "../MyLib/ObjectManager.h"
#include "../MyLib/Component/Transform.h"
#include "../MyLib/Component/Draw/Drawable3D.h"
#include "../MyLib/Component/Draw/UI/UITelop.h"
#include "../Main/Application.h"
#include "../Utility/Game.h"
#include "../Common/Sound/SoundManager.h"
#include "../Common/Event/EventSensors.h"
#include "../Common/Event/EventControls.h"
#include "../Common/Event/EventManager.h"
#include "../Common/Event/EventStructs.h"
#include "DxLib.h"
#include <cassert>

namespace
{
	// シーン遷移関連
	constexpr int kFadeInterval = 60; // フェードを行う時間
	constexpr int kMaxFadeRate = 255; // フェード進行率の最大値
	constexpr unsigned int kFadeInColor = 0xffffff; // フェードインの色
	constexpr unsigned int kFadeOutColor = 0xffffff; // フェードアウトの色

	const Vector2Int kTelopPos = { Game::kScreenWidth / 2, Game::kScreenHeight / 2 };
	const Vector2Int kTextPos = { Game::kScreenWidth / 2, 50 };

	const Vector3 kLightDir = { 0.0f,-1.5f,0.0f };
	const Vector3 kShadowMapAreaMin = { -61.0f,-1.0f,-65.0f };
	const Vector3 kShadowMapAreaMax = { 61.0f,20.0f,65.0f };

	// ロックオンを行う最大距離
	constexpr float kLockOnMaxDistance = 15.0f;

	constexpr float kLockOnStickThreshold = 0.7f;

	const Vector2Int kLifeUILeft = { 70, 60 };

	constexpr int kLifeUIMargin = 80;

	// ファイルを読み込む際のパスの最大サイズ(文字数)
	constexpr size_t kFilePathMax = 256;
}

GameScene::GameScene(SceneController& controller, int stageNo) :
	SceneBase(controller),
	m_update(&GameScene::FadeInUpdate),
	m_draw(&GameScene::FadeDraw),
	m_gameTime(0), 
	m_stageNo(stageNo),
	m_isGameStart(false),
	m_isLockOnChange(true)
{
}

GameScene::~GameScene()
{
	// 念のため終了処理を呼ぶようにする
	//End();
}

void GameScene::Init()
{
	m_frameCount = kFadeInterval;
	m_fadeColor = kFadeInColor;

	m_pPlayer = std::dynamic_pointer_cast<Player>(MyLib::ObjectFactory::CreatePlayer());

	m_pPlayer->Init();

	m_enemyManager = std::make_shared<EnemyManager>();
	m_enemyManager->Init(m_stageNo);
	m_enemyManager->CreateEnemy(m_pPlayer);

	m_pStage = std::make_shared<Stage>();

	m_pStage->Init(m_stageNo);

	m_pSkybox = std::dynamic_pointer_cast<Skybox>(MyLib::ObjectFactory::CreateSkybox());

	m_pSkybox->Init(Skybox::Type::Morning);

	m_pCamera = std::dynamic_pointer_cast<PlayerCamera>(MyLib::ObjectFactory::CreateCamera());

	m_pCamera->Init();
	m_pCamera->SetTarget(m_pPlayer->GetComponent<MyLib::Transform>());

	PlayerAttackResource resource;
	if (resource.Load(L"Data/File/CSV/test.csv"))
	{
		resource.ConvertAttackData();
	}

	if (m_textResources.Load(L"Data/File/CSV/text_data.csv"))
	{
		m_textResources.ConvertTextData();
	}

	m_pUITelop = MyLib::ObjectFactory::CretateUITelop(kTelopPos, MyLib::Renderer::FontType::Midium);
	m_pUITelop->Init();
	m_pTelop = m_pUITelop->GetComponent<MyLib::UITelop>();

	m_pUICountDown = MyLib::ObjectFactory::CreateUICountDown(kTelopPos, MyLib::Renderer::FontType::Header);
	m_pUICountDown->Init();
	m_pCountDown = m_pUICountDown->GetComponent<MyLib::UICountDown>();

	m_pUIText = MyLib::ObjectFactory::CreateUIText(kTextPos, MyLib::Renderer::FontType::Large);
	m_pUIText->Init();
	m_pText = m_pUIText->GetComponent<MyLib::UIText>();

	m_pLockOnMarker = std::make_shared<LockOnMarkerUI>();

	m_pLifeUI = std::make_shared<LifeUI>();
	m_pLifeUI->Init(kLifeUILeft, kLifeUIMargin, m_pPlayer->GetMaxLife());
	m_pLifeUI->SetLife(m_pPlayer->GetLife());

	m_pEventSensors = std::make_shared<EventSensors>();
	m_pEventControls = std::make_shared<EventControls>();
	SetEventFunc();

	m_pEventManager = std::make_unique<EventManager>();

	wchar_t eventFilePath[kFilePathMax];
	std::swprintf(eventFilePath, kFilePathMax, L"Data/File/CSV/Event/stage%d_data.csv", m_stageNo);

	EventResource eventResource;
	if (eventResource.Load(eventFilePath))
	{
		eventResource.ConvertEventData();
	}

	m_pEventManager->Init
	(
		eventResource.GetEventDatas(),
		m_pEventControls, m_pEventSensors
	);

	SetLightDirection(kLightDir);
	COLOR_F color = GetLightAmbColor();
	SetLightAmbColor(GetColorF(10.0f, 10.0f, 10.0f, 10.0f));
	MyLib::Renderer::GetInstance().SetShadowMapLightDir(kLightDir);
	MyLib::Renderer::GetInstance().SetShadowMapArea(kShadowMapAreaMin, kShadowMapAreaMax);

	auto& soundManager = SoundManager::GetInstance();
	soundManager.LoadSoundClip("TitleBGM", L"Data/File/Sound/BGM/title.ogg", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("SelectBGM", L"Data/File/Sound/BGM/select.mp3", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("GameBGM", L"Data/File/Sound/BGM/stage1.ogg", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("ResultBGM", L"Data/File/Sound/BGM/result.mp3", SoundBus::BGM, 1.0f, true);
	soundManager.LoadSoundClip("OK", L"Data/File/Sound/SE/ok.mp3", SoundBus::SE, 1.0f, false);
	soundManager.LoadSoundClip("Cursor", L"Data/File/Sound/SE/cursor.mp3", SoundBus::SE, 1.0f, false);
	soundManager.LoadSoundClip("Open", L"Data/File/Sound/SE/open.mp3", SoundBus::SE, 1.0f, false);
	soundManager.LoadSoundClip("LockOn", L"Data/File/Sound/SE/lock_on.mp3", SoundBus::SE, 1.0f, false);
	soundManager.LoadSoundClip("TargetChange", L"Data/File/Sound/SE/target_change.mp3", SoundBus::SE, 1.0f, false);
	soundManager.LoadSoundClip("Pause", L"Data/File/Sound/SE/pause.mp3", SoundBus::SE, 1.0f, false);
}

void GameScene::End()
{
	OutputDebugStringA("End: Player start\n");
	m_pPlayer->End();
	OutputDebugStringA("End: Player done enemy start\n");

	m_enemyManager->End();
	OutputDebugStringA("End: enemy done stage start\n");

	m_pStage->End();
	OutputDebugStringA("End: stage done camera start\n");

	m_pCamera->End();
	OutputDebugStringA("End: camera done  ui telop start\n");

	m_pUITelop->End();
	OutputDebugStringA("End: ui telop done countdown start\n");

	m_pUICountDown->End();
	OutputDebugStringA("End: countdown done UItext start\n");

	m_pUIText->End();
	OutputDebugStringA("End: UItext done skybox start\n");

	m_pSkybox->End();
	OutputDebugStringA("End: skybox done \n");

	auto& soundManager = SoundManager::GetInstance();
	soundManager.DeleteSoundClip("TitleBGM");
	soundManager.DeleteSoundClip("SelectBGM");
	soundManager.DeleteSoundClip("GameBGM");
	soundManager.DeleteSoundClip("ResultBGM");
	soundManager.DeleteSoundClip("OK");
	soundManager.DeleteSoundClip("Cursor");
	soundManager.DeleteSoundClip("Open");
	soundManager.DeleteSoundClip("LockOn");
	soundManager.DeleteSoundClip("TargetChange");
	soundManager.DeleteSoundClip("Pause");
}

void GameScene::Update()
{
	m_pEventManager->Update();

	(this->*m_update)();
}

void GameScene::Draw() const
{
	(this->*m_draw)();

	//DrawGraph(0, 0, m_tex->GetHandle(), true);

	//DrawString(0, 0, L"GameScene", GetColor(255, 255, 255));
	//DrawFormatString(0, 16, GetColor(255, 255, 255), L"FRAME:%d", m_frameCount);
}

void GameScene::FadeInUpdate()
{
	m_pCamera->SetTarget(m_pPlayer->GetComponent<MyLib::Transform>());

	m_frameCount--;
	if (m_frameCount <= 0)
	{
		m_update = &GameScene::NormalUpdate;
		m_draw = &GameScene::NormalDraw;
		return;
	}
}

void GameScene::FadeOutUpdate()
{
	m_frameCount++;
	if (m_frameCount >= kFadeInterval)
	{
		MyLib::ObjectManager::GetInstance().End();
		//m_sceneController.ChangeScene(std::make_shared<ResultScene>(m_sceneController, m_gameTime));
		SoundManager::GetInstance().CrossFadeBGM("ResultBGM", 1.0f);
		m_sceneController.RequestChangeScene(std::make_shared<LoadingScene>([&controller = m_sceneController, this] { return std::make_shared<ResultScene>(controller, m_gameTime, m_stageNo); }, L"Data/File/CSV/Resource/result_scene.csv", m_sceneController, LoadingScene::TransitionType::Change));
		return;
	}
}

void GameScene::NormalUpdate()
{
	m_pLifeUI->SetLife(m_pPlayer->GetLife());

	if (m_pPlayer->IsDied())
	{
		m_sceneController.PushScene(std::make_shared<GameoverScene>(m_sceneController, m_stageNo));
		return;
	}

	Input& input = Input::GetInstance();

	if (m_isGameStart)
	{
		m_gameTime++;

#ifdef _DEBUG

#else
		if (input.IsTriggered("Pause"))
		{
			SoundManager::GetInstance().Play("Pause", 1.0f, true);
			m_sceneController.PushScene(std::make_shared<PauseScene>(m_sceneController));
			return;
		}
#endif
	}

	if (auto text = m_pText.lock())
	{
		text->SetText(L"Time : " + std::to_wstring(m_gameTime / 60));
	}

	//m_pCamera->SetTarget(m_pPlayer->GetComponent<MyLib::Transform>());

	if (input.IsTriggered("XButton") && !m_pCamera->IsLockOn())
	{
		m_pCamera->LookForward();
	}

	Input::XInputData stickData = input.GetXInputData();

	// Lトリガーが押されたら
	if (input.IsTriggeredXInput(false))
	{
		auto target = m_enemyManager->GetNearEnemyTransform(m_pPlayer->GetPos(), m_pCamera->GetEyePos(), m_pCamera->GetFovDegree());
		if (auto pTarget = target.lock())
		{
			SoundManager::GetInstance().Play("LockOn", 1.0f, true);

			m_pCamera->SetLockOnTarget(pTarget);
			// Playerにターゲットをセットする
			m_pPlayer->SetLockOnTarget(pTarget);
			m_pLockOnTarget = target;
			m_pLockOnMarker->Show(pTarget);
		}
	}

	m_pLockOnMarker->Update();

	if (auto pLockOnTarget = m_pLockOnTarget.lock())
	{
		// 右スティックの入力がロックオンを変更する値を超えた場合
		if (fabsf(stickData.rightStick.x) > kLockOnStickThreshold)
		{
			if (m_isLockOnChange)
			{
				bool isLeft = stickData.rightStick.x <= 0.0f;

				auto newTarget = m_enemyManager->ReGetNearEnemyTransform(m_pPlayer->GetPos(), m_pCamera->GetEyePos(), m_pCamera->GetFovDegree(), pLockOnTarget->GetPos(), isLeft);

				// 新たなターゲットが見つかった場合新しいものに変更する
				if (auto pNewTarget = newTarget.lock())
				{
					SoundManager::GetInstance().Play("TargetChange", 1.0f, true);

					m_pCamera->SetLockOnTarget(pNewTarget);
					// Playerにターゲットをセットする
					m_pPlayer->SetLockOnTarget(pNewTarget);
					m_pLockOnTarget = pNewTarget;
					m_pLockOnMarker->Show(pNewTarget);
				}

				m_isLockOnChange = false;
			}
		}
		else
		{
			m_isLockOnChange = true;
		}

		bool shouldRelease = false;

		// Lトリガーが離されたら
		if(input.IsReleasedXInput(false))
		{
			shouldRelease = true;
		}
		else if (Vector3::GetDistance(m_pPlayer->GetPos(), pLockOnTarget->GetPos()) > kLockOnMaxDistance)
		{
			shouldRelease = true;
		}

		if (shouldRelease)
		{
			m_pCamera->ClearLockOn();
			// Playerのターゲット解除
			m_pPlayer->ClearLockOn();
			m_pLockOnTarget.reset();
			m_pLockOnMarker->Hide();
		}
	}
	else if(m_pCamera->IsLockOn())
	{
		m_pCamera->ClearLockOn();
		// Playerのターゲット解除
		m_pPlayer->ClearLockOn();
	}
	else
	{
		m_pLockOnMarker->Hide();
	}


	m_enemyManager->Update();
}

void GameScene::FadeDraw() const
{
	// フェード率の計算 開始時: 0.0f  終了時: 1.0f
	auto rate = static_cast<float>(m_frameCount) / static_cast<float>(kFadeInterval);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(kMaxFadeRate * rate));
	DrawBox(0, 0, Game::kScreenWidth, Game::kScreenHeight, m_fadeColor, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void GameScene::NormalDraw() const
{
}

void GameScene::SetEventFunc()
{
	m_pEventSensors->isTextFinishedFunc = [this]()
		{
			// 敵を倒せ！のようなテキストの一連の流れが終わったらtrueになるようにしたい

			auto pTelop = m_pTelop.lock();
			return pTelop ? pTelop->IsSequenceFinished() : true;
		};

	m_pEventSensors->isCountDownFinishedFunc = [this]()
		{
			// カウントダウンが終わったらtrueになるようにしたい
			auto pCountDown = m_pCountDown.lock();

			return pCountDown ? pCountDown->IsFinished() : true;
		};

	m_pEventSensors->isAllEnemyDeadFunc = [this]()
		{
			return m_enemyManager->IsEnemyDeadAll();
		};

	m_pEventControls->showTextFunc = [this](const std::wstring& id)
		{
			// 敵を倒せ！のようなテキストの描画を始めるようにしたい
			auto pTelop = m_pTelop.lock();
			if (!pTelop) return;

			pTelop->ShowMessage(m_textResources.GetText(id));
		};

	m_pEventControls->startCountDownFunc = [this]()
		{
			// カウントダウンのテキストを始めるようにしたい
			auto pCountDown = m_pCountDown.lock();
			if (!pCountDown) return;
			pCountDown->StartCountDown(3, 60, 45);
		};

	m_pEventControls->setPlayerCanMoveFunc = [this](bool canMove)
		{
			m_pPlayer->SetCanAct(canMove);
			if (canMove)
			{
				m_isGameStart = true;
			}
		};

	m_pEventControls->setEnemyCanActFunc = [this](bool canAct)
		{
			m_enemyManager->SetCanAct(canAct);
		};

	m_pEventControls->goToClearSceneFunc = [this]()
		{
			m_update = &GameScene::FadeOutUpdate;
			m_draw = &GameScene::FadeDraw;
		};
}
