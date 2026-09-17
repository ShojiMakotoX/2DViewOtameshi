#include "Title.h"
#include "Engine\\Input.h"
#include "Engine\\SceneManager.h"



Title::Title(GameObject* parent)
	:GameObject(parent,"Title")
{
}

void Title::Initialize()
{
	pText_ = new Text;
	HRESULT result = pText_->Initialize();
}

void Title::Update()
{
	if (Input::IsKey(DIK_SPACE))
	{
		SceneManager* pSceneManager = (SceneManager*)FindObject("SceneManager");
		pSceneManager->ChangeScene(SCENE_ID_TEST);
	}
}

void Title::Draw()
{
	pText_->Draw(550, 300, "Maze Hunt\n");
	pText_->Draw(500, 400, "Press Space Start!\n");
	
}

void Title::Release()
{
	//pText_->Release();
}
