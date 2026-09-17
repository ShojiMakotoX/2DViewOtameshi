#include "CLEAR.h"
#include "Engine\\Input.h"
#include "Engine\\SceneManager.h"

CLEAR::CLEAR(GameObject* parent)
	:GameObject(parent,"CLEAR")
{
}

void CLEAR::Initialize()
{
	pText_ = new Text;
	pText_->Initialize();
}

void CLEAR::Update()
{
	if (Input::IsKey(DIK_R))
	{
		SceneManager* pSceneManager = (SceneManager*)FindObject("SceneManager");
		pSceneManager->ChangeScene(SCENE_ID_TITLE);
	}
}

void CLEAR::Draw()
{
	pText_->Draw(550, 300, "EXCELLENT!\n");
	pText_->Draw(500, 400, "Press Space R to Title.\n");
}

void CLEAR::Release()
{
	//pText_->Release();
}
