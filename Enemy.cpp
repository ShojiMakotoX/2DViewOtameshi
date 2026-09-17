#include "Enemy.h"
#include "Ground.h"
#include "Player.h"
#include "Engine\\Model.h"

namespace
{
	
	std::vector<std::vector<int>>gmap;
	enum ENEMY_DIRECTION//敵の方向変換に必要そう
	{
		ENEMY_UP,
		ENEMY_DOWN,
		ENEMY_LEFT,
		ENEMY_RIGHT,
		ENEMY_DIRECTION_MAX//方向の数
	};
	ENEMY_DIRECTION edir = ENEMY_LEFT;//最初に敵が向く方向を決定する
	XMVECTOR E_MOVE[4] = { XMVectorSet(0,0,1,0),XMVectorSet(0,0,-1,0),
		XMVectorSet(-1,0,0,0),XMVectorSet(1,0,0,0) };
}
Enemy::Enemy(GameObject* parent)
	:GameObject(parent,"Enemy"),hModel_(-1)
{
}

void Enemy::Initialize()
{
	hModel_ = Model::Load("Enemy.fbx");
	Model::SetAnimFrame(hModel_, 0, 67, 1.0);
	transform_.position_ = { 4.0f,1.0f,1.0f };
	transform_.scale_ = { 1.0f,1.0f,1.0f };

	
}

void Enemy::Update()
{
	XMVECTOR pos = XMLoadFloat3(&transform_.position_);
	XMVECTOR move = XMVectorSet(0, 0, 0, 0);
	const float SPEED = 0.1f;
	/*float angle = 0.0f;
	static float turnFrame = 0.0f;*/

	move = E_MOVE[edir];

	pos = pos + SPEED * move;
	XMStoreFloat3(&transform_.position_, pos);
	XMFLOAT3 wpos = transform_.position_;

	gmap = ground_->GetMapData();//マップを取得
	int mapX = (int)((wpos.x + 10.0f)/2.0f);
	int mapZ = (int)((10.0f - wpos.z)/2.0f);

	
	
	if (gmap[mapZ][mapX] == 1)
	{
		pos = pos - SPEED * move;
		XMStoreFloat3(&transform_.position_, pos);
		edir = (ENEMY_DIRECTION)(rand() % ENEMY_DIRECTION_MAX);
	}
}

void Enemy::Draw()
{
	Model::SetTransform(hModel_, transform_);
	Model::Draw(hModel_);
}

void Enemy::Release()
{
}

void Enemy::OnCollision(GameObject* pTarget)
{
}
