#pragma once
#include "Engine/GameObject.h"
#include "Engine\\SphereCollider.h"

class Ground;

class Player :
    public GameObject
{
public:
	//コンストラクタ
	//引数：parent  親オブジェクト（SceneManager）
	Player(GameObject* parent);
	//初期化
	void Initialize() override;
	//更新
	void Update() override;
	//描画
	void Draw() override;
	//開放
	void Release() override;
	void SetGround(Ground* ground) { ground_ = ground; }
	void OnCollision(GameObject* pTarget)override;
private:

	enum PLAYER_STATE
	{
		PLAYER_IDLE,
		PLAYER_WALK,
		PLAYER_TURN,
		PLAYER_STATE_MAX//状態の数
	};
	
	enum PLAYER_DIRECTION
	{
		PLAYER_UP,
		PLAYER_DOWN,
		PLAYER_LEFT,
		PLAYER_RIGHT,
		PLAYER_DIRECTION_MAX//方向の数
	};

	bool HandleInput();
	bool UpdateTurn();
	void UpdateJump();
	void ResolveWallCollision(XMVECTOR& pos, const XMVECTOR& move);

	int hWalkModel_;//歩きアニメーションのモデルハンドル
	int hIdleModel_;//待機アニメーションのモデルハンドル
	Ground* ground_;

	//状態変数
	PLAYER_STATE  pstate_;
	PLAYER_DIRECTION pdirection_;

	float turnStartAngle_;//開始角度
	float turnEndAngle_;//終了角度

	float currentSpeed_;
	float turnFrame_;

	float jumpVelocity_;
	bool isGrounded_;

};

