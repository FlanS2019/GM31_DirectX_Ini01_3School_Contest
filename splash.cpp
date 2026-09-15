//splash.cpp
#include "main.h"
#include "splash.h"
#include "manager.h"
#include "splashLogo.h"

void Splash::Init()
{
	Manager::AddGameObject<SplashLogo>();
}

void Splash::Uninit()
{
}
