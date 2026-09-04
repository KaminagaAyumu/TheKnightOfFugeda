#include "EnemyResource.h"
#include "EnemyData.h"

void EnemyResource::ConvertEnemyData()
{
	// データの数を取得する
	size_t size = GetDataCount();

	for(size_t i = 0; i < size; ++i)
	{
		// データを読み込む
		int type = Read<int>(static_cast<int>(i), 0);
		float posX = Read<float>(static_cast<int>(i), 1);
		float posY = Read<float>(static_cast<int>(i), 2);
		float posZ = Read<float>(static_cast<int>(i), 3);
		// 敵データを作成
		std::shared_ptr<EnemyData> data = std::make_shared<EnemyData>();
		data->SetData(static_cast<EnemyManager::EnemyType>(type), Vector3{ posX, posY, posZ });
		m_enemyDatas.push_back(data);
	}

	// データを開放する
	ReleaseRawData();
}
