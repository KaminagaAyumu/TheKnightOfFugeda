#include "UIText.h"
#include "../../../GameObject.h"
#include "../../../Renderer.h"
#include "../../../../Utility/Game.h"
#include <cassert>

MyLib::UIText::UIText(DrawLayer layer) : 
	Drawable2D(layer),
	m_text(L""),
	m_fontHandle(-1),
	m_textColor(0xffffff),
	m_isAlive(true)
{
}

void MyLib::UIText::Init(std::weak_ptr<GameObject> parent)
{
	std::shared_ptr<GameObject> pParent = parent.lock();
	m_pTransform = pParent->GetComponent<Transform>();
	if (!m_pTransform.lock())
	{
		assert(false && "UIText : Transformコンポーネントがありません");
	}
	Renderer::GetInstance().Entry(shared_from_this());
}

void MyLib::UIText::Start()
{
}

void MyLib::UIText::Update()
{
}

void MyLib::UIText::End()
{
}

void MyLib::UIText::Draw() const
{
	if (!IsActive())return;
	if (m_pTransform.expired()) return;
	Vector2Int pos = Vector2Int{ 0,0 };

	if (auto pTransform = m_pTransform.lock())
	{
		pos = pTransform->GetScreenPos();
	}
	const int textW = GetDrawStringWidthToHandle(m_text.c_str(), static_cast<int>(m_text.size()), m_fontHandle);
	const int fontSize = GetFontSizeToHandle(m_fontHandle);

	const int x = pos.x - textW / 2;
	const int y = pos.y - fontSize / 2;

	DrawStringToHandle(x, y, m_text.c_str(), 0xffffff, m_fontHandle);
}
