#include "Player.h"
#include "Engine\\Model.h"
#include "Engine\\Debug.h"
#include "TestScene.h"
#include "Engine\\Input.h"
#include "Ground.h"
#include <cmath>

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

	//短形の当たり判定を表す構造体
	struct CollisionRect
	{
		float left, right, bottom, top;
	};

	//プレイヤーの座標から当たり判定用の短形を生成する
	CollisionRect MakePlayerRect(const XMFLOAT3& position)
	{
		const float foot = position.y - PLAYER_FOOT_OFFSET;
		return { position.x - PLAYER_HALF_WIDTH,position.x + PLAYER_HALF_WIDTH,foot,foot + PLAYER_HEIGHT };
	}
	
	//ブロックの行・列・マップの高さから当たり判定用の短形を生成する
	CollisionRect MakeBlockRect(int row,int col,int mapHeight)
	{
		const float x = col * BLOCK_INTERVAL_X;
		const float y = (mapHeight - 1 - row) * BLOCK_INTERVAL_Y;
		return { x - BLOCK_HALF_WIDTH,x + BLOCK_HALF_WIDTH,y,y + BLOCK_SURFACE_HEIGHT };
	}

	//2つの短形がX軸方向に重なっているか判定
	bool OverlapX(const CollisionRect& a, const CollisionRect& b)
	{
		return a.right > b.left + CONTACT_EPSILON && a.left < b.right - CONTACT_EPSILON;
	}

	//2つの短形がY軸方向に重なっているか判定
	bool OverlapY(const CollisionRect& a, const CollisionRect& b)
	{
		return a.top > b.bottom + CONTACT_EPSILON && a.bottom < b.top - CONTACT_EPSILON;
	}
	
	//プレイヤーの向きに対応する角度
	float P_ANGLE[4] = { 180.0f,0.0f,90.0f,270.0f };

	//プレイヤーの向きに対応する移動ベクトル
	XMVECTOR P_MOVE[4] = { XMVectorSet(0,0,1,0),XMVectorSet(0,0,-1,0),
		XMVectorSet(-1,0,0,0),XMVectorSet(1,0,0,0) };
	
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

}

//コンストラクタの宣言
Player::Player(GameObject* parent)
	:GameObject(parent,"Player"),
	hWalkModel_(-1), 
	hIdleModel_(-1) ,
	ground_(nullptr),
	pstate_(PLAYER_IDLE),
	pdirection_(PLAYER_DOWN),
	turnStartAngle_(0.0f),
	turnEndAngle_(0.0f),
	turnEndDirection_(PLAYER_DOWN),
	currentSpeed_(0.0f),
	turnFrame_(0.0f),
	jumpVelocity_(0.0f),
	isGrounded_(true)
{
	
}

void Player::Initialize()
{
	hWalkModel_ = Model::Load("Walking.fbx");
	Model::SetAnimFrame(hWalkModel_, 0, 67, 1.0);
	transform_.position_ = START_POS;

	hIdleModel_ = Model::Load("Idle.fbx");
	Model::SetAnimFrame(hIdleModel_, 0, 600, 1.0);
	SphereCollider* collision = new SphereCollider(XMFLOAT3(0, 0.25f, 0), 0.5f);
	AddCollider(collision);
}

void Player::Update()
{
	//方向転換中でなければ、いったん待機状態へ
	//HandleInput()で移動入力があればWALKへ
	if (pstate_ != PLAYER_TURN)
	{
		pstate_ = PLAYER_IDLE;
	}

	bool isBraking = HandleInput();
	if (UpdateTurn())
	{
		UpdateJump();
		return;
	}

	XMVECTOR pos = XMLoadFloat3(&transform_.position_);
	XMVECTOR move = XMVectorSet(0, 0, 0, 0);

	//移動入力中
	if (pstate_ == PLAYER_WALK)
	{
		//空中で加速を弱くする(if文のやつ…だよな、これ）
		float accel = isGrounded_
			? ACCRATATE
			: ACCRATATE * AIR_CONTROL;

		currentSpeed_ += accel;

		if (currentSpeed_ > MAX_SPEED)
		{
			currentSpeed_ = MAX_SPEED;
		}
		move = P_MOVE[pdirection_];
		transform_.rotate_.y = P_ANGLE[pdirection_];
	}
	//移動入力がない場合は
	else
	{
		if (currentSpeed_ > 0.0f)
		{
			float decel;//減速処理
			if (isGrounded_)
			{
				decel = isBraking ? BRAKE : FRICTION;
			}
			else
			{
				//空中では減速を弱く
				decel = FRICTION * AIR_CONTROL;
			}
			currentSpeed_ -= decel;
			if (currentSpeed_ < 0.0f)
			{
				currentSpeed_ = 0.0f;
			}
			//入力を話しても減速中は今までの方向へ
			move = P_MOVE[pdirection_];
		}
	}

	//水平移動
	pos = pos + currentSpeed_ * move;
	XMStoreFloat3(&transform_.position_, pos);

	//衝突で速度がゼロになったかを解決前後で確認
	const float speedBeforeCollision = currentSpeed_;
	ResolveWallCollision(pos, move);
	const bool blockedByWall = speedBeforeCollision > 0.0f && currentSpeed_ == 0.0f;

	//実際の移動速度と壁に向かって歩くアニメの速度を分離
	//入力を離した場合や逆方向へのブレーキ中は再度固定しない
	const bool pushingWall = blockedByWall && pstate_ == PLAYER_WALK;
	const float walkAnimSpeed = pushingWall
		? WALL_WALK_SPEED
		: currentSpeed_ / BASE_SPEED;

	Model::SetAnimSpeed(hWalkModel_, walkAnimSpeed);

	//ジャンプ・重力・ブロックへの着地
	UpdateJump();
}
bool Player::HandleInput()
{
	bool isBraking = false;

	PLAYER_DIRECTION olddir = pdirection_;//今の向きを入れる

	if (pstate_ != PLAYER_TURN)
	{
		//停止中
		if (currentSpeed_ == 0.0f && isGrounded_)
		{
			if (Input::IsKey(DIK_LEFT))
			{

				pdirection_ = PLAYER_DIRECTION::PLAYER_LEFT;
				pstate_ = PLAYER_STATE::PLAYER_WALK;

			}
			if (Input::IsKey(DIK_RIGHT))
			{

				pdirection_ = PLAYER_DIRECTION::PLAYER_RIGHT;
				pstate_ = PLAYER_STATE::PLAYER_WALK;
			}
		}
		//移動中、または空中の場合
		else
		{
			if (Input::IsKey(DIK_LEFT))
			{
				if (pdirection_ == PLAYER_LEFT)
				{
					pstate_ = PLAYER_STATE::PLAYER_WALK;
				}
				else if (pdirection_ == PLAYER_RIGHT)
				{
					//稚樹で逆方向入力したときだけブレーキを
					isBraking = isGrounded_;
				}
			}
			if (Input::IsKey(DIK_RIGHT))
			{
				if (pdirection_ == PLAYER_RIGHT)
				{
					pstate_ = PLAYER_STATE::PLAYER_WALK;
				}
				else if (pdirection_ == PLAYER_LEFT)
				{
					//稚樹で逆方向入力したときだけブレーキを
					isBraking = isGrounded_;
				}
			}
		}
	}
	//ジャンプ開始
	if (Input::IsKeyDown(DIK_SPACE) && isGrounded_)
	{
		jumpVelocity_ = JUMP_POWER;
		isGrounded_ = false;
	}
	//方向転換
	if (olddir != pdirection_)
	{
		//回転
		pstate_ = PLAYER_TURN;
		turnFrame_ = 0.0f;
		turnStartAngle_ = P_ANGLE[olddir];
		//最短方向へ回転するための角度差補正
		float diff = AdujustAngle(P_ANGLE[pdirection_] - P_ANGLE[olddir]);
		turnEndDirection_ = pdirection_;
		turnEndAngle_ = turnStartAngle_ + diff;
	}
	return isBraking;

}
//方向転換処理
//TURN_FRAME フレーム掛けて回転
bool Player::UpdateTurn()
{
	if (pstate_ != PLAYER_TURN)
	{
		return false;
	}
		//回転処理
		//angleを30フレーム使って新しいangleに切り替え
		//古いものからちょっとずつ足してって…

		turnFrame_ += 1.0f;

		float t = min(turnFrame_ / TURN_FRAME, 1.0f);//0から1.0

		transform_.rotate_.y = turnStartAngle_ + (turnEndAngle_ - turnStartAngle_) * t;


		//30フレーム経過したら回転終了
		if (turnFrame_ >= TURN_FRAME)
		{
			pdirection_ = turnEndDirection_;
			transform_.rotate_.y = P_ANGLE[pdirection_];
			pstate_ = PLAYER_WALK;
		}
		return true;
}
//ジャンプ・重力・ブロックへの着地処理
void Player::UpdateJump()
{
	if (ground_ == nullptr)
	{
		return;
	}

	const auto& gmap = ground_->GetMapData();
	const int mapHeight = static_cast<int>(gmap.size());
	const CollisionRect before = MakePlayerRect(transform_.position_);


	if (isGrounded_)
	{
		//既存仕様の常設床　穴を作る場合はこの床もマップで管理
		bool supported = transform_.position_.y <= START_POS.y + CONTACT_EPSILON;
		float supportY = START_POS.y;
		for (int row = 0;row < mapHeight;++row)
		{

			for (int col = 0;col < static_cast<int>(gmap[row].size());++col)
			{
				if (gmap[row][col] != 1)
				{
					continue;
				}
				const CollisionRect block = MakeBlockRect(row, col, mapHeight);
				if (OverlapX(before, block) && std::fabs(before.bottom - block.top) <= CONTACT_EPSILON)
				{
					supported = true;
					supportY = block.top + PLAYER_FOOT_OFFSET;
				}

			}
		}
		if (supported)
		{
			transform_.position_.y = supportY;
			jumpVelocity_ = 0.0f;
			return;
		}
		isGrounded_ = false;
		jumpVelocity_ = 0.0f;

	}
	const float dy = jumpVelocity_;
	transform_.position_.y += dy;
	jumpVelocity_ -= GRAVITY;
	const CollisionRect after = MakePlayerRect(transform_.position_);

	float reslovedY = transform_.position_.y;
	bool hit = false;

	//移動前後で面をまたいだ画を調べ、最初に接触する面で止める
	for (int row = 0;row < mapHeight;++row)
	{
		for (int col = 0;col < static_cast<int>(gmap[row].size());++col)
		{
			if (gmap[row][col] != 1)
			{
				continue;
			}
			const CollisionRect block = MakeBlockRect(row, col, mapHeight);
			if (!OverlapX(after, block))
			{
				continue;
			}

			if (dy <= 0.0f && before.bottom >= block.top - CONTACT_EPSILON && after.bottom <= block.top)
			{
				const float y = block.top + PLAYER_FOOT_OFFSET;
				if (!hit || y > reslovedY)
				{
					reslovedY = y;
				}
				hit = true;

			}


		}
	}
	//常設床も着地候補に
	if (dy <= 0.0f && reslovedY <= START_POS.y)
	{
		reslovedY = START_POS.y;
		hit = true;
	}
	transform_.position_.y = reslovedY;
	if (hit)
	{
		jumpVelocity_ = 0.0f;
		//頭突きでは接地させない。次の更新から重力で落下
		isGrounded_ = dy <= 0.0f;
	}

}

//壁との衝突処理
void Player::ResolveWallCollision(XMVECTOR& pos, const XMVECTOR& move)
{
	if (ground_ == nullptr)
	{
		return;
	}
	//Updateで適用した水平移動から、移動前の短形を復元
	XMFLOAT3 oldPosition;
	XMStoreFloat3(&oldPosition, pos - currentSpeed_ * move);
	const float dx = transform_.position_.x - oldPosition.x;
	if (dx == 0.0f)
	{
		return;
	}

	const CollisionRect before = MakePlayerRect(oldPosition);
	const CollisionRect after = MakePlayerRect(transform_.position_);
	const auto& gmap = ground_->GetMapData();
	const int mapHeight = static_cast<int>(gmap.size());
	float resolveX = transform_.position_.x;
	bool hit = false;

	for (int row = 0;row < mapHeight;++row)
	{
		for (int col = 0;col < static_cast<int>(gmap[row].size());++col)
		{
			if (gmap[row][col] != 1)
			{
				continue;
			}
			const CollisionRect block = MakeBlockRect(row, col, mapHeight);
			if (!OverlapY(before, block))
			{
				continue;
			}
			if (dx > 0.0f && before.right <= block.left + CONTACT_EPSILON && after.right >= block.left)
			{
				const float x = block.left - PLAYER_HALF_WIDTH;
				if (!hit || x < resolveX)
				{
					resolveX = x;
				}
				hit = true;
			}
			else if (dx < 0.0f && before.left >= block.right + CONTACT_EPSILON && after.left <= block.right)
			{
				const float x = block.right + PLAYER_HALF_WIDTH;
				if (!hit || x > resolveX)
				{
					resolveX = x;
				}
				hit = true;
			}
		}
	}
	if (hit)
	{
		transform_.position_.x = resolveX;
		pos = XMLoadFloat3(&transform_.position_);
		currentSpeed_ = 0.0f;
	}
}


void Player::Draw()
{
	Transform drawTransform = transform_;
	drawTransform.scale_.x *= PLAYER_MODEL_SCALE;
	drawTransform.scale_.y *= PLAYER_MODEL_SCALE;
	drawTransform.scale_.z *= PLAYER_MODEL_SCALE;

	if (pstate_ == PLAYER_IDLE)
	{
		
		Model::SetTransform(hIdleModel_, transform_);
		Model::Draw(hIdleModel_);
	}
	else if (pstate_ == PLAYER_WALK|| pstate_ == PLAYER_TURN)
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




