#include <DxLib.h>

#include "ItemController.h"
#include "GameObject.h"
#include "Primitive.h"
#include "Collider.h"
#include "DrawManager.h"
#include "GameWindow.h"
#include "GameTime.h"
#include "EffectUtillity.h"

namespace
{
	//落ちる速さ（ピクセル/秒）。画面の上から下まで約4秒で、取りに行くか迷える長さにする
	constexpr float FallSpeed = 120.0f;
	//画面の下からこれだけ出たら消す。半分見えている間に消えないようにするため
	constexpr float DestroyMargin = 30.0f;

	constexpr int RingColor = 0xFFFFFF;
	constexpr int RingThickness = 2;
	//本体の色の上でも読めるように、頭文字は黒で描く
	constexpr int LetterColor = 0x000000;
	//強化のオーラと同じ光り方にして、「取ると強化につながる物」だと分かるようにする
	constexpr int AuraLevel = 1;
	constexpr float AuraRadiusOffset = 6.0f;

	//本体に描く強化の頭文字。色が見分けにくい人でも種類が分かるようにする
	const char* GetBuffInitial(BuffType type)
	{
		switch (type)
		{
		case BuffType::Attack:  return "A";
		case BuffType::Defense: return "D";
		case BuffType::Speed:   return "S";
		case BuffType::Predict: return "P";
		case BuffType::Heal:    return "H";
		default:                return "";
		}
	}
}

void ItemController::Initialize(ServiceLocator& locator)
{
	_shape = GetOwner()->GetComponent<Primitive>();
	_collider = GetOwner()->GetComponent<Collider>();
	_drawManager = locator.drawManager;
	_bounds = &locator.gameWindow->GetBounds();
}

void ItemController::Update(float deltaTime)
{
	Vector2& position = GetOwner()->GetTransform().Position;
	position.y += FallSpeed * deltaTime;

	//取られずに画面の下まで落ちたら消す
	if (position.y > _bounds->height + DestroyMargin)
		GetOwner()->Destroy();
}

void ItemController::Draw()
{
	_drawManager->Register(DrawLayer::Enemy, [this] { DrawBody(); });
}

void ItemController::OnPickup(GameObject& picker)
{
	//同じフレームのうちに、もう一度拾われないようにする
	_collider->SetCollisionEnabled(false);
	GetOwner()->Destroy();
}

void ItemController::DrawBody() const
{
	const Vector2& position = GetOwner()->GetTransform().Position;
	const float radius = _shape->GetRadius();
	//機体のオーラやHUDと同じ色にして、取る前にどの強化か分かるようにする
	const int bodyColor = GetBuffColor(_buffType);

	//外側から順に、脈打つ光、塗りつぶした本体、白い縁、頭文字の順に重ねる
	//本体の大きさは当たり判定の半径と同じにして、見た目どおりの位置で拾えるようにする
	EffectUtillity::DrawAura(position, radius + AuraRadiusOffset, bodyColor, AuraLevel, Time::GetWorldTime());
	DrawCircle((int)position.x, (int)position.y, (int)radius, bodyColor, TRUE);
	//白い縁で、暗い背景や敵の弾と重なっても形が分かるようにする
	DrawCircle((int)position.x, (int)position.y, (int)radius, RingColor, FALSE, RingThickness);

	//頭文字を本体の真ん中に置く
	const char* letter = GetBuffInitial(_buffType);
	const int letterWidth = GetDrawStringWidth(letter, static_cast<int>(strlen(letter)));
	DrawString((int)position.x - letterWidth / 2, (int)position.y - GetFontSize() / 2, letter, LetterColor);
}
