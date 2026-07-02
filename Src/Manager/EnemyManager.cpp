#include "EnemyManager.h"
#include <string>
#include <fstream>
#include "../Application.h"
#include "../Utility/SchoolUtility.h"
#include "../Object/Collider/ColliderBase.h"
#include "../Object/Actor/Character/Enemy/EnemyRat.h"
#include "../Object/Actor/Character/Enemy/EnemyRobot.h"

EnemyManager::EnemyManager(void)
{
	enemies_.clear();
}

EnemyManager::~EnemyManager(void)
{
}

void EnemyManager::Init(void)
{
	// エネミーデータの読み込み
	LoadCsvData();

}

void EnemyManager::Update(void)
{
	for (auto& enemy : enemies_)
	{
		enemy->Update();
	}
}

void EnemyManager::Draw(void)
{
	for (auto& enemy : enemies_)
	{
		enemy->Draw();
	}
}

void EnemyManager::Release(void)
{
	for (auto& enemy : enemies_)
	{
		enemy->Release();
		delete enemy;
	}
	enemies_.clear();
}

void EnemyManager::AddHitCollider(const ColliderBase* hitCollider)
{
	for (auto& enemy : enemies_)
	{
		enemy->AddHitCollider(hitCollider);
	}
}

void EnemyManager::SetTargetTransform(const Transform* targetTransform)
{
	for (auto& enemy : enemies_)
	{
		enemy->SetTargetTransform(targetTransform);
	}
}

void EnemyManager::LoadCsvData(void)
{
	// ファイルの読込
	
	// ifs = input file stream
	std::ifstream ifs = std::ifstream(Application::PATH_CSV + "EnemyData.csv");
	if (!ifs)
	{
		// エラーが発生
		return;
	}
	// ファイルを１行ずつ読み込む
	std::string line;

	// 1行の文字情報
	std::vector<std::string> strSplit;

	// 1行を1文字の動的配列に分割
	bool isHeader = true;
	while (getline(ifs, line))
	{
		if (isHeader)
		{
			isHeader = false;
			continue;
		}
		// １行をカンマ区切りで分割
		strSplit = SchoolUtility::Split(line, ',');
		EnemyBase* enemy = nullptr;
		
		// 構造体に合わせて読込データを格納
		EnemyBase::EnemyData data = EnemyBase::EnemyData();
		int idx = 0;

		// ID
		// stoi = string to int
		data.id = stoi(strSplit[idx++]);

		// 種別
		data.type = static_cast<EnemyBase::TYPE>(stoi(strSplit[idx++]));

		// HP
		data.hp = stoi(strSplit[idx++]);

		// 初期座標
		// stof = string to float
		data.defaultPos =
		{
		stof(strSplit[idx++]),
		stof(strSplit[idx++]),
		stof(strSplit[idx++])
		};

		// 移動可能範囲
		data.radius = stof(strSplit[idx++]);

		// 巡回ﾎﾟｲﾝﾄ
		std::string points = strSplit[idx++];
		if (points != "-")
		{
			std::vector<std::string> datas = SchoolUtility::Split(points, '#');
			for (auto& poss : datas)
			{
				std::vector<std::string> xyz = SchoolUtility::Split(poss, '|');
				VECTOR tmp = VECTOR();
				tmp.x = stof(xyz[0]);
				tmp.y = stof(xyz[1]);
				tmp.z = stof(xyz[2]);
				data.wayPoints.emplace_back(tmp);
			}
		}

		// エネミー生成
		Create(data);
	}

	ifs.close();

}

EnemyBase* EnemyManager::Create(const EnemyBase::EnemyData& data)
{
	EnemyBase* enemy = nullptr;

	// 種別に応じたエネミー生成
	switch (data.type)
	{
	case EnemyBase::TYPE::RAT:
		enemy = new EnemyRat(data);
		break;
	case EnemyBase::TYPE::ROBOT:
		enemy = new EnemyRobot(data);
		break;
	default:
		break;
	}

	if (enemy != nullptr)
	{
		enemy->Init();
		enemies_.emplace_back(enemy);
	}

	return enemy;
}