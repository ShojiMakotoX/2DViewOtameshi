#include "Player.h"
#include "Engine\\Model.h"
#include "Engine\\Debug.h"
#include "TestScene.h"
#include "Engine\\Input.h"
#include "Ground.h"

namespace
{
	//プレイヤー移動に関する定数
	const float MAX_SPEED = 0.2f;//最大移動速度
	const float BASE_SPEED = 0.1f;//アニメーション速度1.0になる基準速度
	const float ACCRATATE = 0.005f;//移動入力中の加速度
	const float FRICTION = 0.008f;//入力を話した時の減速度
	const float BRAKE = 0.02f;//進行方向と逆方向を入力したときの減速度
	const float TURN_FRAME = 10.0f;//方向転換に書けるフレーム数
	const float BLOCK_INTERVAL_X = 2.0f;//マップ1マス分のワールドサイズ

	const XMFLOAT3 START_POS = { 15.0f,0.75f,0.5f };//プレイヤー初期座標

	//ジャンプ
	const float JUMP_POWER = 0.2f;//ジャンプ開始時の上向き速度
	const float GRAVITY = 0.01f;//1フレームごとに減少する垂直速度
	const float AIR_CONTROL = 0.5f;//空中での加速・減速の強さ（地上比）

	//ブロックの配置間隔
	const float BLOCK_INTERVAL_Y = 1.0f;//ブロック1マス分の縦方向の間隔
	const float BLOCK_HALF_WIDTH = 0.99375f;//ブロックの当たり判定の半分の幅
	const float BLOCK_SURFACE_HEIGHT = 0.75f;//ブロック上面の高さ

	//プレイヤーの判定（原点を足元に）
	const float PLAYER_FOOT_OFFSET = 0.0f;//プレイヤー足元の補正値
	//身長はマップの縦2マスで、判定はワールド座標定義
	const float PLAYER_HEIGHT = BLOCK_INTERVAL_Y * 2.0f;//プレイヤーの当たり判定高さ
	const float PLAYER_MODEL_HEIGHT = 3.76537f;//プレイヤーモデルの元の高さ（スケールを計算するときに必要）
	const float PLAYER_MODEL_SCALE = PLAYER_HEIGHT / PLAYER_MODEL_HEIGHT;
	//横幅は従来の判定幅を描画モデルと同じ割合で
	const float PLAYER_HALF_WIDTH = 0.4f * PLAYER_MODEL_SCALE;//プレイヤーの当たり判定の半分幅
	const float CONTACT_EPSILON = 0.0001f;//当たり判定誤差吸収用。
	const float WALL_WALK_SPEED = 1.0f;//壁戻しの歩行速度

	struct CollisionRect
	{
		float left, right, bottom, top;
	};



	//enum
	


	
	
	std::vector<std::vector<int>>gmap;

	float P_ANGLE[4] = { 180.0f,0.0f,90.0f,270.0f };
	XMVECTOR P_MOVE[4] = { XMVectorSet(0,0,1,0),XMVectorSet(0,0,-1,0),
		XMVectorSet(-1,0,0,0),XMVectorSet(1,0,0,0) };
	

	//float TURN_FRAME = 30.0f;//回転にかかるフレーム数
	float AdujustAngle(float angle)
	{
		if (angle >= 180.0f)
		{
			angle -= 360.0f;
		}
		else if (angle < -180.0f)
		{
			angle += 360.0f;
		}
		return angle;
	}
	//float diff = 0.0f;//開始角度から、目標角度までの回転量（何度回転するか）
	//float halfangle = 180.0f;
	//float fullangle = 360.0f;

}

Player::Player(GameObject* parent)
	:GameObject(parent,"Player"), hWalkModel_(-1), hIdleModel_(-1) ,ground_(nullptr){
	//swordDirには、初期方向として、ローカルモデルの剣の根っこから
	//先端までのベクトルとして（0,1,0)を代入しておく
	//初期位置は原点
}

void Player::Initialize()
{
	hWalkModel_ = Model::Load("Walking.fbx");
	Model::SetAnimFrame(hWalkModel_, 0, 67, 1.0);
	transform_.position_ = { 0.5f,0.0,0.5f };

	hIdleModel_ = Model::Load("Idle.fbx");
	Model::SetAnimFrame(hIdleModel_, 0, 600, 1.0);
	SphereCollider* collision = new SphereCollider(XMFLOAT3(0, 0.25, 0), 0.5f);
	AddCollider(collision);
}

void Player::Update()
{
	/*transform_.rotate_.y +=1;
	static float angle = 0.0;
	angle = angle + 0.3f;
	XMMATRIX scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX rotateX = XMMatrixRotationX(XMConvertToRadians(angle));
	XMMATRIX rotate = XMMatrixRotationY(XMConvertToRadians(angle));
	XMMATRIX translate = XMMatrixTranslation(1.0f, 0.0f, 0.0f);

	SetWorldMatrix(scale *  rotate * translate);*/
	XMVECTOR pos = XMLoadFloat3(&transform_.position_);
	XMVECTOR move = XMVectorSet(0, 0, 0, 0);
	const float SPEED = 0.1f;
	float angle = 0.0f;
	static float turnFrame = 0.0f;//回転中のフレーム数を管理する変数

	if (pstate != PLAYER_STATE::PLAYER_TURN)
	{
		pstate = PLAYER_STATE::PLAYER_IDLE;
	}
	PLAYER_DIRECTION olddir = pdirection;//今の向きを入れる

	if (pstate != PLAYER_STATE::PLAYER_TURN)//ターン中はキー入力受け付けない
	{
		if (Input::IsKey(DIK_LEFT))
		{

			pdirection = PLAYER_DIRECTION::PLAYER_LEFT;
			pstate = PLAYER_STATE::PLAYER_WALK;

		}
		if (Input::IsKey(DIK_RIGHT))
		{

			pdirection = PLAYER_DIRECTION::PLAYER_RIGHT;
			pstate = PLAYER_STATE::PLAYER_WALK;
		}
		if (Input::IsKey(DIK_UP))
		{

			pdirection = PLAYER_DIRECTION::PLAYER_UP;
			pstate = PLAYER_STATE::PLAYER_WALK;
		}
		if (Input::IsKey(DIK_DOWN))
		{

			pdirection = PLAYER_DIRECTION::PLAYER_DOWN;
			pstate = PLAYER_STATE::PLAYER_WALK;
		}
	}
		if (olddir != pdirection)
		{
			//回転
			pstate = PLAYER_STATE::PLAYER_TURN;
			turnFrame = 0.0f;
			turnStartAngle = P_ANGLE[olddir];
			float diff = AdujustAngle(P_ANGLE[pdirection] - P_ANGLE[olddir]);
			turnEndDirection = pdirection;
			turnEndAngle =turnStartAngle+diff;
		}
		 
		if (pstate == PLAYER_STATE::PLAYER_TURN)
		{
			//回転処理
			//angleを30フレーム使って新しいangleに切り替え
			//古いものからちょっとずつ足してって…

			turnFrame += 1.5f;
			float t = turnFrame / TURN_FRAME;//0から1.0

			if (t > 1.0f)
			{
				t = 1.0f;//1.0を超えないようにする（保険）
			}
			//最短方向に回転するように角度差を補正。
			
			/*if (diff > halfangle)
			{
				diff -= fullangle;
			}
			if (diff < -halfangle)
			{
				diff += fullangle;
			}*/
			
			angle = turnStartAngle + (turnEndAngle-turnStartAngle)*t;//開始角度から回転量を保管率だけ進めた現在の角度を求める
			transform_.rotate_.y = angle;

			//30フレーム経過したら回転終了
			if (turnFrame >= TURN_FRAME)
			{
				pdirection = turnEndDirection;
				transform_.rotate_.y = angle;
				pstate = PLAYER_STATE::PLAYER_WALK;

				return;//早期リターン
			}
		}
		else if (pstate != PLAYER_STATE::PLAYER_IDLE)
		{
			move = P_MOVE[pdirection];
			angle = P_ANGLE[pdirection];
			transform_.rotate_.y = angle;
		}
		pos = pos + SPEED * move;
		XMStoreFloat3(&transform_.position_, pos);
		XMFLOAT3 wpos = transform_.position_;

		//壁オブジェクトに食い込んでいたら戻す
		gmap = ground_->GetMapData();//マップを取得
		int mapX = (int)((wpos.x + 10.0f) / 2);
		int mapZ = (int)((10.0f - wpos.z) / 2);
	
		if (gmap[mapZ][mapX]==1)
		{
			pos = pos - SPEED * move;
			XMStoreFloat3(&transform_.position_, pos);
		}

}
	


void Player::Draw()
{
	//transform_.scale_ = { 0.01,0.01,0.01 };
	//transform_.position_ = { 0,-0.5, 0 };

	if (pstate == PLAYER_STATE::PLAYER_IDLE)
	{
		
		Model::SetTransform(hIdleModel_, transform_);
		Model::Draw(hIdleModel_);
	}
	else if (pstate == PLAYER_STATE::PLAYER_WALK|| pstate == PLAYER_STATE::PLAYER_TURN)
	{
		
		Model::SetTransform(hWalkModel_, transform_);
		Model::Draw(hWalkModel_);
	}
	
}


void Player::Release()
{
}

void Player::OnCollision(GameObject* pTarget)
{
	
}
