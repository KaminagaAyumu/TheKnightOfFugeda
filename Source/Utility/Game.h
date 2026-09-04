#pragma once

// ゲーム全体で使用する定数
namespace Game
{
	// 画面情報
	constexpr int kScreenWidth = 1280;
	constexpr int kScreenHeight = 720;
	constexpr int kColorBitNum = 32;

	// 更新情報
	constexpr int	kOneFrameNanoSec = 16667; // 1フレームのナノ秒(60FPS)

	// エフェクトの最大数
	constexpr int kEffectMaxNum = 8000; // Effekseerで画面に表示できる最大パーティクル数

	// 当たり判定の八分木分割のレベル
	constexpr uint32_t kOctreeLevel = 2;
	// ステージの最大値、最小値(ステージによって変わるため今後変更する可能性がある)
	constexpr float kMinStageSize = -180.0f;
	constexpr float kMaxStageSize = 180.0f;
}
