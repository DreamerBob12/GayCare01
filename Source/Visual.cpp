#include "Visuals.h"
#include "Interfaces.h"
#include "RenderManager.h"




void CVisuals::Init()
{

}

void CVisuals::Move(CUserCmd *pCmd, bool &bSendPacket) {}

void CVisuals::Draw()
{

	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());


	if (Menu::Window.VisualsTab.OtherNoScope.GetState() && pLocal->IsAlive() && pLocal->IsScoped())
		NoScopeCrosshair();

	// Recoil Crosshair
	if (Menu::Window.VisualsTab.OtherSpreadCrosshair.GetState())
		SpreadCrosshair();

}

void CVisuals::NoScopeCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;

	IClientEntity* pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (GameUtils::IsSniper(pWeapon))
	{
		Render::Line(MidX - 1000, MidY, MidX + 1000, MidY, Color(0, 0, 0, 255));
		Render::Line(MidX, MidY - 1000, MidX, MidY + 1000, Color(0, 0, 0, 255));

	}
}



void CVisuals::SpreadCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	IClientEntity* WeaponEnt = Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (!hackManager.pLocal()->IsAlive())
		return;

	if (!GameUtils::IsBallisticWeapon(pWeapon))
		return;

	if (pWeapon == nullptr)
		return;

	int xs;
	int ys;
	Interfaces::Engine->GetScreenSize(xs, ys);
	xs /= 2; ys /= 2;

	auto accuracy = pWeapon->GetInaccuracy() * 550.f; //3000

	Render::DrawFilledCircle(Vector2D(xs, ys), Color(21, 21, 21, 124), accuracy, 60);

	if (pLocal->IsAlive())
	{
		if (pWeapon)
		{

			float inaccuracy = pWeapon->GetInaccuracy() * 1000;
			char buffer4[64];
			sprintf_s(buffer4, "Spread:  %.00f", inaccuracy);

			if (inaccuracy < 15)
			Render::Text(xs + accuracy + 4, ys, Color(5, 255, 80, 255), Render::Fonts::ESP, buffer4);
			else if (inaccuracy >= 15 && inaccuracy < 100)
			Render::Text(xs + accuracy + 4, ys, Color(255, 160, 20, 255), Render::Fonts::ESP, buffer4);
			else
			Render::Text(xs + accuracy + 4, ys, Color(255, 10, 110, 255), Render::Fonts::ESP, buffer4);
		}

	}
	else
	{

		Render::Text(10, 70, Color(180, 20, 255, 255), Render::Fonts::ESP, "Spread: --");
	}

}

void CVisuals::DrawCrosshair()
{

	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;

	Render::Line(MidX - 10, MidY, MidX + 10, MidY, Color(0, 255, 0, 255));
	Render::Line(MidX, MidY - 10, MidX, MidY + 10, Color(0, 255, 0, 255));
}