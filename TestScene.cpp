#include "TestScene.h"
#include "Player.h"
#include "Ground.h"
#include "Engine\\Camera.h"
#include "Engine\\Text.h"
#include "Enemy.h"
#include "Food.h"
#include "Engine\\SceneManager.h"

namespace
{
	int myScore = 0;
	int myEsa;

	Ground* pGround;
	
}

//コンストラクタ
TestScene::TestScene(GameObject * parent)
	: GameObject(parent, "TestScene")
{
}

//初期化
void TestScene::Initialize()
{	
	//pWp = Instantiate<Weapon>(this);
	Player*pPlayer = Instantiate <Player>(this);
	pGround = Instantiate<Ground>(this);
	Enemy* eEnemy = Instantiate<Enemy>(this);
	pPlayer->SetGround(pGround);
	eEnemy->SetGround(pGround);
	myEsa = pGround->GetEsaCount();

	Camera::SetPosition({ 0,10,-20 });
	Camera::SetTarget({0,0,0});

	pText_ = new Text;
	pText_ -> Initialize();

}

//更新
void TestScene::Update()
{
	if (myEsa == 0)
	{
		SceneManager* pSceneManager = (SceneManager*)FindObject("SceneManager");
		pSceneManager->ChangeScene(SCENE_ID_CLEAR);
	}
}

//描画
void TestScene::Draw()
{
	std::string scrText;
	std::string scrText2;
	scrText = "SCORE:" + std::to_string(myScore);
	pText_->Draw(20, 20, scrText.c_str());
	scrText2 = "Esa:" + std::to_string(myEsa);
	pText_->Draw(1000, 20, scrText2.c_str());
}
//開放
void TestScene::Release()
{
	//pText_->Release();//テキスト開放
}

void TestScene::AddScore(int score)
{
	myScore += score;
}

void TestScene::DeleteEsa(int esa)
{
	myEsa -= esa;
}

