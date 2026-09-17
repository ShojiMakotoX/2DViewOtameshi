#include "Ground.h"
#include "Engine\\Model.h"
#include "Engine\\CsvReader.h"
#include "Food.h"

namespace
{
	using std::vector;
	const float GROUND_WIDTH = 20.0f;//グラウンドの幅
	const float GROUND_Y = 10.0f;
	const float GROUND_Z = 1.0f;
	const float GROUND_ROTATE_X = -90.0f;
	const float BLOCK_INTERVAL_X = 2.0f;
	const float BLOCK_INTERVAL_Y = 1.0f;
}
Ground::Ground(GameObject* parent)
	:GameObject(parent,"Ground"), hModel_(-1),mapWidth_(-1),mapHeight_(-1)
{
	CsvReader csvData;
	csvData.Load("map.csv");//CsVファイル読み込み

	mapWidth_ = csvData.GetWidth();//列数を取得
	mapHeight_ = csvData.GetHeight()/2;//行数を取得


	mapData_ = vector<vector<int>>(mapHeight_,vector<int>(mapWidth_,0));
	//objMap_ = vector<vector<int>>(mapHeight_, vector<int>(mapWidth_, 0));

	for (int x = 0;x < mapWidth_;x++)
	{
		for (int y = 0;y < mapHeight_;y++)
		{
			mapData_[y][x] = csvData.GetValue(x, y);
		}
	}
}

void Ground::Initialize()
{
	hModel_ = Model::Load("masu2.fbx");//1025だとうまく読み込めない可能性あり
	//transform_.rotate_ = XMFLOAT3(-90.0f, 0.0f, 0.0f);
	hModelt_ = Model::Load("Block.fbx");
	
	/*hModelesa_ = Model::Load("item.fbx");
	hModelbigesa_ = Model::Load("bigitem.fbx");*/
	
}

void Ground::Update()
{
}

void Ground::Draw()
{
	Model::SetTransform(hModel_,transform_);
	Model::Draw(hModel_);
	
	for (int i = 0;i < 3;i++)
	{
		transform_.position_ = { GROUND_WIDTH / 2.0f + GROUND_WIDTH * i,GROUND_Y,GROUND_Z };
		transform_.rotate_ = { GROUND_ROTATE_X,0.0f,0.0f };
		Model::SetTransform(hModel_, transform_);
		Model::Draw(hModel_);
	}

	for (int j = 0;j < mapHeight_;j++)
	{
		for (int i = 0;i < mapWidth_;i++)
		{
			if (mapData_[j][i] == 1)
			{
				Transform tr;
				tr.position_ = { i*BLOCK_INTERVAL_X,(mapHeight_-1-j)*BLOCK_INTERVAL_Y,0.0f};//ブロック設置
				Model::SetTransform(hModelt_, tr);
				Model::Draw(hModelt_);
			}

			/*if (objMap_[j][i] == 1)
			{
				Transform tr2;
				tr2.position_ = { -9.0f + i * 2.0f ,0.0f,9.0f - j * 2.0f };
				tr2.scale_ = { 1.5f,1.5f,1.5f };
				Model::SetTransform(hModelesa_, tr2);
				Model::Draw(hModelesa_);
			}
			else if (objMap_[j][i] == 2)
			{
				Transform tr2;
				tr2.position_ = { -9.0f + i * 2.0f ,0.0f,9.0f - j * 2.0f };
				tr2.scale_ = { 0.2f,0.2f,0.2f };
				tr2.rotate_.y += 1.0f;
				Model::SetTransform(hModelbigesa_, tr2);
				Model::Draw(hModelbigesa_);
			}*/

		}
	}
}

void Ground::Release()
{
}


