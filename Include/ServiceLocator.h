#pragma once

class InputManager;
class BulletManager;
class GameWindow;
class PlayerProvider;
class ColliderRegistry;
class SceneManager;
class GameObjectFactory;
class DrawManager;
class SoundManager;

//各クラスが共通で使うサービスへの参照をまとめたもの
//実体はGameManagerが持つ
struct ServiceLocator
{
	InputManager* input;
	GameWindow* gameWindow;
	BulletManager* bulletManager;
	PlayerProvider* playerProvider;
	ColliderRegistry* colliderRegistry;
	SceneManager* sceneManager;
	GameObjectFactory* factory;
	DrawManager* drawManager;
	SoundManager* soundManager;
};