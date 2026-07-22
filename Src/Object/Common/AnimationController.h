#pragma once
#include <string>
#include <map>

class AnimationController
{

public:

	// アニメーションデータ
	struct Animation
	{
		int model = -1;				// モデルハンドルID
		int attachNo = -1;			// フレーム番号
		int animIndex = 0;			// アニメーションインデックス
		float speed = 0.0f;			// 再生速度
		float totalTime = 0.0f;		// アニメーション時間
		float step = 0.0f;			// 現在のステップ
		float blendStep = 0.0f;		// ブレンド時間
		float blendRate = 0.0f;		// ブレンド率
		float weight = 0.0f;		// 重み
		bool isPriority = false;	// 優先するかどうか
		bool isLoop = false;		// ループするかどうか
	};

	// コンストラクタ
	AnimationController(int modelId);

	// デストラクタ
	~AnimationController(void);

	// 外部FBXからアニメーション追加
	void Add(int type, float speed, const std::string path);
	
	// 同じFBX内のアニメーションを準備
	void AddInFbx(int type, float speed, int animIndex);

	// アニメーション再生
	void Play(int type, bool isLoop = true, float blendTime = 0.5f);

	// 更新
	void Update(void);

	// 解放
	void Release(void);

	// 再生中のアニメーション
	int GetPlayType(void) const;

	// 再生終了
	bool IsEnd(int animType) const;

	// 再生中のアニメーション情報を取得
	const Animation& GetPlayAnim(void) const;

private:

	// アニメーションするモデルのハンドルID
	int modelId_;

	// 種類別のアニメーションデータ
	std::map<int, Animation> animations_;

	// アニメーションをブレンドしているかどうか
	bool isBlending_;

	// ブレンド時間
	float blendTime_;

	// 現在、優先度の高いアニメーション種別
	int priorityType_;

	// アニメーション追加の共通処理
	void Add(int type, float speed, Animation& animation);

	// アニメーションをアタッチする
	int AttachAnim(const Animation& animation) const;

};
