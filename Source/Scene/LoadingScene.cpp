#include "LoadingScene.h"
#include "SceneController.h"
#include "../Utility/Game.h"
#include "../Utility/CSV/ResourceManifestData.h"
#include "DxLib.h"
#include <cassert>

LoadingScene::LoadingScene(std::function<std::shared_ptr<SceneBase>()> m_nextSceneFactory, std::wstring filePath, SceneController& controller, TransitionType transitionType) :
	SceneBase(controller),
	m_nextSceneFactory(m_nextSceneFactory),
	m_filePath(filePath),
	m_transitionType(transitionType)
{
}

LoadingScene::~LoadingScene()
{
}

void LoadingScene::Init()
{
	ResourceManifestData manifest;

	if (manifest.Load(m_filePath))
	{
		manifest.ConvertManifestData();

		auto& fileManager = FileManager::GetInstance();

		for (const auto& request : manifest.GetResourceRequests())
		{
			if (request.type == FileManager::FileType::Image)
			{
				fileManager.ReserveImage(request.path.c_str(), request.isEternal);
			}
			else if (request.type == FileManager::FileType::Model)
			{
				fileManager.ReserveModel(request.path.c_str(), request.isEternal);
			}
			else
			{
				assert(false && "LoadingScene:不明なリソースの種類です");
			}
		}
	}
	else
	{
		assert(false && "LoadingScene:リソースマニフェストの読み込みに失敗しました");
	}

	
}

void LoadingScene::End()
{
}

void LoadingScene::Update()
{
	if (!FileManager::GetInstance().IsLoadingComplete()) return;

	auto next = m_nextSceneFactory();
	switch (m_transitionType)
	{
	case LoadingScene::TransitionType::Change:
		m_sceneController.ChangeScene(next);
		break;
	case LoadingScene::TransitionType::Push:
		m_sceneController.PushScene(next);
		break;
	case LoadingScene::TransitionType::Reset:
		m_sceneController.ResetScene(next);
		break;
	default:
		break;
	}
}

void LoadingScene::Draw() const
{
	// ローディング画面の描画処理をここに追加する
	// 例: "Loading..."と表示する
	DrawString(100, 100, L"Loading...", GetColor(255, 255, 255));
	DrawFormatString(100, 120, 0xffffff, L"%f パーセント", FileManager::GetInstance().GetLoadProgress() * 100.0f);
}
