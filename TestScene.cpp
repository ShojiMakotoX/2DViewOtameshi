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

	Ground* pGround;
	const int CAMERA_HEIGHT = 8.0f;
	XMFLOAT3 START_POS = { 15.0f,0.75,0.5f };
	const float END_POS_X = 43.0f;
	
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
	//Enemy* eEnemy = Instantiate<Enemy>(this);
	pPlayer->SetGround(pGround);
	//eEnemy->SetGround(pGround);
	//myEsa = pGround->GetEsaCount();

	Camera::SetPosition({ pPlayer_->GetPosition().x,pPlayer_->GetPosition().y+CAMERA_HEIGHT,-22});
	Camera::SetTarget({ pPlayer_->GetPosition().x,pPlayer_->GetPosition().y+CAMERA_HEIGHT,0 });

	pText_ = new Text;
	pText_ -> Initialize();

}

//更新
void TestScene::Update()
{
	if (pPlayer_->GetPosition().x > START_POS.x && pPlayer_->GetPosition().x < END_POS_X)
	{
		Camera::SetPosition({ pPlayer_->GetPosition().x,pPlayer_->GetPosition().y + CAMERA_HEIGHT,-22 });
		Camera::SetTarget({ pPlayer_->GetPosition().x,pPlayer_->GetPosition().y + CAMERA_HEIGHT,0 });
	}
}

//描画
void TestScene::Draw()
{
	/*std::string scrText;
	std::string scrText2;
	scrText = "SCORE:" + std::to_string(myScore);
	pText_->Draw(20, 20, scrText.c_str());
	scrText2 = "Esa:" + std::to_string(myEsa);
	pText_->Draw(1000, 20, scrText2.c_str());*/
}
//開放
void TestScene::Release()
{
	pText_->Release();//テキスト開放
}


