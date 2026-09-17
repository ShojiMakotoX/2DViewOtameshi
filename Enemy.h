#pragma once
#include "Engine\\GameObject.h"

class Ground;

class Enemy :
    public GameObject
{
public:
	Enemy(GameObject* parent);
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
	int hModel_;//歩きアニメーションのモデルハンドル
	Ground* ground_;
};

