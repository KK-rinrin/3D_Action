#pragma once
#include <vector>
#include "../Object/Actor/Character/Enemy/EnemyBase.h"

class EnemyBase;
class ColliderBase;
class Transform;
class EnemyManager
{
public:
	// コンストラクタ
	EnemyManager(void);
	// デストラクタ
	~EnemyManager(void);
	
	// 初期化
	void Init(void);
	// 更新
	void Update(void);
	// 描画
	void Draw(void);
	// 解放
	void Release(void);
	
	// エネミー
	const std::vector<EnemyBase*>&GetEnemies(void) const { return enemies_; }
	
	// 衝突対象となるコライダを登録
	void AddHitCollider(const ColliderBase* hitCollider);

	// 追跡対象を設定
	void SetTargetTransform(const Transform* targetTransform);

	// CSVから敵情報の読取を行う
	void LoadCsvData(void);

	// エネミー生成
	EnemyBase* Create(const EnemyBase::EnemyData& data);

private:
	// エネミー
	std::vector<EnemyBase*> enemies_;
};