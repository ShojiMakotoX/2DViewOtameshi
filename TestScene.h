#pragma once
#include "Engine/GameObject.h"
#include "Engine/Model.h"

class Player;
class Text;//前方宣言
//テストシーンを管理するクラス
class TestScene : public GameObject
{
public:
	//コンストラクタ
	//引数：parent  親オブジェクト（SceneManager）
	TestScene(GameObject* parent);

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
	Player* pPlayer_;
};