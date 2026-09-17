#pragma once
#include "Engine\\GameObject.h"
#include "Engine\\Text.h"

class Title :
    public GameObject
{
public:
	Title(GameObject* parent);
	//初期化
	void Initialize() override;
	//更新
	void Update() override;
	//描画
	void Draw() override;
	//開放
	void Release() override;
	
private:
	Text* pText_;
};

