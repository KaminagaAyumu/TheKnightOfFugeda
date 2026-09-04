#include "PlayerAttackResource.h"
#include <cassert>

PlayerAttackResource::PlayerAttackResource()
{
}

PlayerAttackResource::~PlayerAttackResource()
{
}

void PlayerAttackResource::ConvertAttackData()
{
	// データの数を取得する
	size_t size = GetDataCount();

	for (size_t i = 0; i < size; ++i)
	{
		// データを読み込む
		int id = Read<int>(static_cast<int>(i), 0);
		int startFrame = Read<int>(static_cast<int>(i), 1);
		int activeFrame = Read<int>(static_cast<int>(i), 2);
		int comboFrame = Read<int>(static_cast<int>(i), 3);
		int animChangeFrame = Read<int>(static_cast<int>(i), 4);
		int cancelFrame = Read<int>(static_cast<int>(i), 5);
		int nextID = Read<int>(static_cast<int>(i), 6);
		std::wstring nextAnim = Read<std::wstring>(static_cast<int>(i), 7);

		// 攻撃データを作成
		std::shared_ptr<AttackData> data = std::make_shared<AttackData>();
		// データをセットする
		data->SetData(static_cast<AttackData::AttackID>(id), startFrame, activeFrame, comboFrame, animChangeFrame,cancelFrame, static_cast<AttackData::AttackID>(nextID), nextAnim);
		m_pAttackDatas[static_cast<AttackData::AttackID>(id)] = data;
	}

	// データを開放する
	ReleaseRawData();
}

AttackData* PlayerAttackResource::GetAttackData(AttackData::AttackID id)
{
	auto it = m_pAttackDatas.find(id);
	if (it != m_pAttackDatas.end())
	{
		return it->second.get();
	}
	else
	{
		assert(false && "PlayerAttackResource : IDに対応する攻撃データが見つかりませんでした");
	}
	return nullptr;
}
