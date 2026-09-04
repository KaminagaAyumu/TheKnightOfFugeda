#pragma once
#include "SceneBase.h"
#include "../MyLib/Component/Draw/UI/UIText.h"
#include "../MyLib/Component/Draw/UI/UISelectList.h"
#include <memory>

class Skybox;

class ResultScene : public SceneBase
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="controller">シーン遷移に必要なコントローラー</param>
	explicit ResultScene(SceneController& controller, int gameTime, int stageNo);
	// デストラクタ
	virtual ~ResultScene();

	virtual void Init()override;
	virtual void End()override;
	virtual void Update()override;
	virtual void Draw() const override;

private:
	using UpdateFunc_t = void(ResultScene::*)();
	UpdateFunc_t m_update;

	using DrawFunc_t = void(ResultScene::*)()const;
	DrawFunc_t m_draw;

	int m_gameTime;

	int m_stageNo;

	std::shared_ptr<MyLib::GameObject> m_pUIText;
	std::weak_ptr<MyLib::UIText> m_pText;
	
	std::shared_ptr<MyLib::GameObject> m_pUITextTime;
	std::weak_ptr<MyLib::UIText> m_pTextTime;

	std::shared_ptr<MyLib::GameObject> m_pUISelectList;
	std::weak_ptr<MyLib::UISelectList> m_pSelectList;

	std::shared_ptr<Skybox> m_pSkybox;

private:
	void FadeInUpdate();
	void FadeOutUpdate();
	void NormalUpdate();

	void FadeDraw()const;
	void NormalDraw()const;

};

