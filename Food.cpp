#include "Food.h"
#include "Engine\\Model.h"
#include "Engine\\SphereCollider.h"
#include "TestScene.h"
#include "Ground.h"

Food::Food(GameObject*parent)
	:GameObject(parent,"Food"),type_(FOODTYPE_NORMAL),hModel_(-1),score_(0)
{
}

Food::~Food()
{
}

void Food::Initialize()
{
	transform_.scale_ = { 0.3f,0.3f,0.3f };

	
}

void Food::Update()
{
	if (type_ ==  FoodType::FOODTYPE_POWER)
	{
		transform_.rotate_.y += 1.0f;
	}
}

void Food::Draw()
{
	Model::SetTransform(hModel_, transform_);
	Model::Draw(hModel_);
}

void Food::Release()
{
}

void Food::SetFoodType(FoodType type)//餌についての情報
{
	type_ = type;
	if (type_ ==  FoodType::FOODTYPE_NORMAL)
	{
		SphereCollider* collision = new SphereCollider(XMFLOAT3(0, 1.0, 0), 0.35f);
		AddCollider(collision);
		hModel_ = Model::Load("item.fbx");
		transform_.scale_ = { 1.5f,1.5f,1.5f };
		score_ = 1;
		
	}
	else if (type_ ==  FoodType::FOODTYPE_POWER)
	{
		SphereCollider* collision = new SphereCollider(XMFLOAT3(0, 0.5, 0), 0.7f);
		AddCollider(collision);
		hModel_ = Model::Load("bigitem.fbx");
		transform_.scale_ = { 0.25f,0.25f,0.25f };
		score_ =  5;
		
	}
}

void Food::OnCollision(GameObject* pTarget)//餌と当たった時の反応
{
	if (pTarget->GetObjectName() == "Player")
	{
	TestScene* testScene = dynamic_cast<TestScene*>(GetParent()->GetParent());
	testScene->AddScore(score_);//スコア加算
	testScene->DeleteEsa(1);//餌の数減らす
	
		KillMe();
	}
	
}
