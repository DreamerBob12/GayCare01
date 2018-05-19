#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" 
#include "Configuration.hpp"
#include "Interfaces.h"
#include "CRC32.h"
#include "xostrash.h"
#include "GUI.h"


#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 520

mirrorwindow Menu::Window;

void KnifeApplyCallbk()
{
	static ConVar* Meme = Interfaces::CVar->FindVar("cl_fullupdate");
	Meme->nFlags &= ~FCVAR_CHEAT;
	Interfaces::Engine->ClientCmd_Unrestricted("cl_fullupdate");
}
void GloveApplyCallbk()
{
	static ConVar* Meme = Interfaces::CVar->FindVar("cl_fullupdate");
	Meme->nFlags &= ~FCVAR_CHEAT;
	Interfaces::Engine->ClientCmd_Unrestricted("cl_fullupdate");
}

struct Config_t {
	int id;
	std::string name;
};

std::vector<Config_t> configs;

void save_callback()
{
	switch (Menu::Window.MiscTab.ConfigBox.GetIndex())
	{
	case 0:
		GUI.SaveWindowState(&Menu::Window, "legit.xml");
		break;
	case 1:
		GUI.SaveWindowState(&Menu::Window, "scout.xml");
		break;
	case 2:
		GUI.SaveWindowState(&Menu::Window, "auto.xml");
		break; 
	case 3: GUI.SaveWindowState(&Menu::Window, "duals.xml"); break;
	case 4: GUI.SaveWindowState(&Menu::Window, "deagle.xml"); break; 
	case 5: GUI.SaveWindowState(&Menu::Window, "r8.xml"); break;
	}
}

void load_callback()
{
	switch (Menu::Window.MiscTab.ConfigBox.GetIndex())
	{
	case 0:
		GUI.LoadWindowState(&Menu::Window, "legit.xml");
		break;
	case 1:
		GUI.LoadWindowState(&Menu::Window, "scout.xml");
		break;
	case 2:
		GUI.LoadWindowState(&Menu::Window, "auto.xml");
		break;
	case 3:
		GUI.LoadWindowState(&Menu::Window, "duals.xml");
		break;
	case 4:
		GUI.LoadWindowState(&Menu::Window, "deagle.xml");
		break;
	case 5:
		GUI.LoadWindowState(&Menu::Window, "r8.xml");
		break;
	}
}

void UnLoadCallbk()
{
	DoUnload = true;
}

void mirrorwindow::Setup()
{
	int w, h;
	int centerW, centerh, topH;
	Interfaces::Engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;


	SetPosition(320, 60);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle("");

	RegisterTab(&RageBotTab);
	RegisterTab(&LegitBotTab);
	RegisterTab(&VisualsTab);

	RegisterTab(&MiscTab);
	RegisterTab(&colourtab);
	RegisterTab(&aabuild);

	RegisterTab(&SkinchangerTab); 

	RECT Client = GetClientArea();
	Client.bottom -= 29;

	RageBotTab.Setup();
	LegitBotTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();

	colourtab.Setup();
	aabuild.Setup();
	SkinchangerTab.Setup(); 


//-------------------Broken-Configs-------------------//


/*
	MiscTab.RegisterControl(&SaveButton);
	MiscTab.RegisterControl(&LoadButton);
	MiscTab.RegisterControl(&ConfigBox);

	LegitBotTab.RegisterControl(&SaveButton);
	LegitBotTab.RegisterControl(&LoadButton);
	LegitBotTab.RegisterControl(&ConfigBox);

	RageBotTab.RegisterControl(&SaveButton);
	RageBotTab.RegisterControl(&LoadButton);
	RageBotTab.RegisterControl(&ConfigBox);

	VisualsTab.RegisterControl(&SaveButton);
	VisualsTab.RegisterControl(&LoadButton);
	VisualsTab.RegisterControl(&ConfigBox);

	SkinchangerTab.RegisterControl(&SaveButton);
	SkinchangerTab.RegisterControl(&LoadButton);
	SkinchangerTab.RegisterControl(&ConfigBox);

	colourtab.RegisterControl(&SaveButton);
	colourtab.RegisterControl(&LoadButton);
	colourtab.RegisterControl(&ConfigBox);
	*/
	




#pragma endregion
}

void CRageBotTab::Setup()
{

	SetTitle("Rage");

	ActiveLabel.SetPosition(40, 2);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(10, 2);
	RegisterControl(&Active);
#pragma region RageBot
#pragma region Aimbot

	AimbotGroup.SetPosition(10, 20);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(340, 395);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceCheckBox("Enable", this, &AimbotEnable);

	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceCheckBox("Auto Fire", this, &AimbotAutoFire);

	AccuracyRecoil.SetFileId("acc_norecoil");
	AccuracyRecoil.AddItem("Off");
	AccuracyRecoil.AddItem("No Recoil");
	AccuracyRecoil.AddItem("(!) No Spread");
	AimbotGroup.PlaceOtherControl("Accuracy Control", this, &AccuracyRecoil);

	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.SetValue(35.f);
	AimbotGroup.PlaceOtherControl("FOV Range", this, &AimbotFov);

	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceCheckBox("Silent", this, &AimbotSilentAim);


	posadjust.SetFileId("fakelagfixlmao");
	AimbotGroup.PlaceCheckBox("Lag Correction", this, &posadjust);

	ragetrack.SetFileId("acc_backtrklmao");
	AimbotGroup.PlaceOtherControl("Force Backtrack", this, &ragetrack);

	AWPAtBody.SetFileId("aim_awpatbody");
	AimbotGroup.PlaceCheckBox("AWP Body Aim", this, &AWPAtBody);

	AutoRevolver.SetFileId("aim_autorevolver");
	AimbotGroup.PlaceCheckBox("Auto Revolver", this, &AutoRevolver);

	AimbotAimStep.SetFileId("aim_aimstep");
	AimbotGroup.PlaceCheckBox("Casual Rage", this, &AimbotAimStep);

	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("Closest To Crosshair");
	TargetSelection.AddItem("Distance");
	TargetSelection.AddItem("Lowest Health");
	TargetSelection.AddItem("Threat");
	TargetSelection.AddItem("Next Shot");
	AimbotGroup.PlaceOtherControl("Selection", this, &TargetSelection);

	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("None");
	TargetHitbox.AddItem("Head");
	TargetHitbox.AddItem("Low");
	TargetHitbox.AddItem("Medium");
	TargetHitbox.AddItem("High");
	AimbotGroup.PlaceOtherControl("Hitbox", this, &TargetHitbox);

	TargetMultipoint.SetFileId("tgt_multipoint");  // causes crashes  
	AimbotGroup.PlaceCheckBox("Multipoint", this, &TargetMultipoint);

	TargetPointscale.SetFileId("tgt_pointscale");
	TargetPointscale.SetBoundaries(0.f, 7.f);
	TargetPointscale.SetValue(4.9f);
	AimbotGroup.PlaceOtherControl("Aim Height", this, &TargetPointscale);

	BaimIfUnderXHealth.SetFileId("acc_BaimIfUnderXHealthidfkwtf");
	BaimIfUnderXHealth.SetBoundaries(0, 100);
	BaimIfUnderXHealth.SetValue(0);
	AimbotGroup.PlaceOtherControl("bAim if HP is under", this, &BaimIfUnderXHealth);

	xaneafterX.SetFileId("acc_baimafterXshots");
	xaneafterX.SetBoundaries(0, 20);
	xaneafterX.SetValue(3);
	AimbotGroup.PlaceOtherControl("bAim After X Shots", this, &xaneafterX);

#pragma endregion Aimbot Controls Get Setup in here

#pragma region Accuracy
	AccuracyGroup.SetPosition(370, 185);
	AccuracyGroup.SetText("Other");
	AccuracyGroup.SetSize(310, 230);
	RegisterControl(&AccuracyGroup);


	AccuracyAutoWall.SetFileId("acc_awall");
	AccuracyGroup.PlaceCheckBox("AutoWall", this, &AccuracyAutoWall);

	AccuracyMinimumDamage.SetFileId("acc_mindmg");
	AccuracyMinimumDamage.SetBoundaries(1.f, 99.f);
	AccuracyMinimumDamage.SetValue(1.f);
	AccuracyGroup.PlaceOtherControl("Minimum Damage", this, &AccuracyMinimumDamage);

	AccuracyAutoScope.SetFileId("acc_scope");
	AccuracyGroup.PlaceCheckBox("Auto Scope", this, &AccuracyAutoScope);


	AimbotResolver.SetFileId("acc_aaaaaaaaaaaaa");
	AimbotResolver.AddItem("Off");
	AimbotResolver.AddItem("Simple"); // The most basic thing you can ask for, but it works rather well.
	AimbotResolver.AddItem("Prediction"); // Simple brute based off certain events. It's not a pure brute tho.
	AimbotResolver.AddItem("Smart"); // Zeus resolver. 
	AimbotResolver.AddItem("(!) NoSpread"); // meh, 6.5/10
	AccuracyGroup.PlaceOtherControl("Resolver", this, &AimbotResolver);

	positioncorrect.SetFileId("interp_fix");
	AccuracyGroup.PlaceCheckBox("Position Correction", this, &positioncorrect);

	forceback.SetFileId("acc_force");
	forceback.SetBoundaries(0, 20);
	forceback.SetValue(4);
	AccuracyGroup.PlaceOtherControl("Brute After X Shots", this, &forceback);

	AimbotBaimOnKey.SetFileId("baim_onkeypress");
	AccuracyGroup.PlaceOtherControl("Force bAim", this, &AimbotBaimOnKey);

	AccuracyHitchance.SetFileId("acc_chance");
	AccuracyHitchance.SetBoundaries(0, 100);
	AccuracyHitchance.SetValue(0);
	AccuracyGroup.PlaceOtherControl("Hitchance", this, &AccuracyHitchance);

	baimfactor.SetFileId("acc_baimfact");
	baimfactor.AddItem("None");
	baimfactor.AddItem("Innacurate");
	baimfactor.AddItem("Fake");
	baimfactor.AddItem("Forceful");
	AccuracyGroup.PlaceOtherControl("Prefer bAim If", this, &baimfactor);


#pragma endregion  Accuracy controls get Setup in here


#pragma region AntiAim
	AntiAimGroup.SetPosition(370, 10);
	AntiAimGroup.SetText("Anti-Aim");
	AntiAimGroup.SetSize(310, 165);
	RegisterControl(&AntiAimGroup);

	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceCheckBox("Enable", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("Off");
	AntiAimPitch.AddItem("Safe Down");
	AntiAimPitch.AddItem("Safe Up");
	AntiAimPitch.AddItem("Down");
	AntiAimPitch.AddItem("Up");
	AntiAimPitch.AddItem("Nuiger Down");
	AntiAimPitch.AddItem("Divine down");
	AntiAimPitch.AddItem("Custom");
	AntiAimGroup.PlaceOtherControl("Pitch", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("Off");
	AntiAimYaw.AddItem("Static");
	AntiAimYaw.AddItem("Twitch");
	AntiAimYaw.AddItem("Jitter");
	AntiAimYaw.AddItem("Lowerbody");
	AntiAimYaw.AddItem("Magic Lowerbody");
	AntiAimYaw.AddItem("KeyBased");
	AntiAimYaw.AddItem("Spinbot");
	AntiAimYaw.AddItem("Distorted");
	AntiAimYaw.AddItem("Custom");
	AntiAimGroup.PlaceOtherControl("Real Yaw", this, &AntiAimYaw);

	FakeYaw.SetFileId("fake_aa");
	FakeYaw.AddItem("Off");
	FakeYaw.AddItem("Static");
	FakeYaw.AddItem("Twitch");
	FakeYaw.AddItem("Jitter");
	FakeYaw.AddItem("Magic Lowerbody");
	FakeYaw.AddItem("Jolt");
	FakeYaw.AddItem("Spinbot");
	FakeYaw.AddItem("Distorted");
	FakeYaw.AddItem("Custom");
	AntiAimGroup.PlaceOtherControl("Fake Yaw", this, &FakeYaw);

	FlipAA.SetFileId("flipaa");
	AntiAimGroup.PlaceCheckBox("Flip if Hit", this, &FlipAA);

	ManualAA.SetFileId("manualaa");
	AntiAimGroup.PlaceOtherControl("Manual Switch", this, &ManualAA);



#pragma endregion  AntiAim controls get setup in here
#pragma endregion RageBot
}

void CLegitBotTab::Setup()
{
	SetTitle("Legit");  //pFont 2012

	ActiveLabel.SetPosition(40, 1);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetPosition(10, 1);
	RegisterControl(&Active);

#pragma region Aimbot
	AimbotGroup.SetPosition(1, 20);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(220, 200);
	RegisterControl(&AimbotGroup);

	AimbotEnable.SetFileId("aim_legitenable");
	AimbotGroup.PlaceCheckBox("Enable", this, &AimbotEnable);

	AimbotBacktrack.SetFileId("legit_backtrack");
	AimbotGroup.PlaceCheckBox("Backtrack Ticks", this, &AimbotBacktrack);

	TickModulation.SetFileId("tick_modulate");
	TickModulation.SetBoundaries(0.1f, 13.f);
	TickModulation.SetValue(13.f);
	AimbotGroup.PlaceOtherControl("Tick Range", this, &TickModulation);

	AimbotAutoFire.SetFileId("aim_legtautofire");
	AimbotGroup.PlaceCheckBox("Auto Fire", this, &AimbotAutoFire);

	AimbotFriendlyFire.SetFileId("aim_friendlegtfire");
	AimbotGroup.PlaceCheckBox("Friendly Fire", this, &AimbotFriendlyFire);

	AimbotSmokeCheck.SetFileId("otr_smokeweedcheck");
	AimbotGroup.PlaceCheckBox("Smoke Check", this, &AimbotSmokeCheck);


	AimbotKeyBind.SetFileId("aim_onkeylmao");
	AimbotGroup.PlaceOtherControl("Key Bind", this, &AimbotKeyBind);

#pragma endregion Aimbot shit

#pragma region Triggerbot
	TriggerGroup.SetPosition(230, 20);
	TriggerGroup.SetText("Triggerbot");
	TriggerGroup.SetSize(240, 200);
	RegisterControl(&TriggerGroup);

	TriggerEnable.SetFileId("trig_enable");
	TriggerGroup.PlaceCheckBox("Enable", this, &TriggerEnable);

	TriggerKeyPress.SetFileId("trig_onkey");
	TriggerGroup.PlaceCheckBox("On Key Press", this, &TriggerKeyPress);

	TriggerKeyBind.SetFileId("trig_key");
	TriggerGroup.PlaceOtherControl("Key Bind", this, &TriggerKeyBind);

	TriggerRecoil.SetFileId("trig_recoil");
	TriggerGroup.PlaceCheckBox("Recoil Control", this, &TriggerRecoil);

	TriggerSmokeCheck.SetFileId("trig_smokecheck");
	TriggerGroup.PlaceCheckBox("Smoke Check", this, &TriggerSmokeCheck);

	TriggerDelay.SetFileId("trig_time");
	TriggerDelay.SetBoundaries(0, 100);
	TriggerDelay.SetValue(1);
	TriggerGroup.PlaceOtherControl("Delay", this, &TriggerDelay);

#pragma endregion Triggerbot stuff

#pragma region TriggerbotFilter
	TriggerFilterGroup.SetPosition(480, 20);
	TriggerFilterGroup.SetText("Filter");
	TriggerFilterGroup.SetSize(200, 200);
	RegisterControl(&TriggerFilterGroup);

	TriggerHead.SetFileId("trig_head");
	TriggerFilterGroup.PlaceCheckBox("Head", this, &TriggerHead);

	TriggerChest.SetFileId("trig_chest");
	TriggerFilterGroup.PlaceCheckBox("Chest", this, &TriggerChest);

	TriggerStomach.SetFileId("trig_stomach");
	TriggerFilterGroup.PlaceCheckBox("Stomach", this, &TriggerStomach);

	TriggerArms.SetFileId("trig_arms");
	TriggerFilterGroup.PlaceCheckBox("Arms", this, &TriggerArms);

	TriggerLegs.SetFileId("trig_legs");
	TriggerFilterGroup.PlaceCheckBox("Legs", this, &TriggerLegs);

	TriggerTeammates.SetFileId("trig_teammates");
	TriggerFilterGroup.PlaceCheckBox("Friendly Fire", this, &TriggerTeammates);

#pragma endregion TriggerbotFilter stuff

#pragma region Main Weapon
	WeaponMainGroup.SetPosition(1, 230);
	WeaponMainGroup.SetText("Rifles");
	WeaponMainGroup.SetSize(220, 130);
	RegisterControl(&WeaponMainGroup);

	WeaponMainHitbox.SetFileId("main_hitbox");
	WeaponMainHitbox.AddItem("Head");
	WeaponMainHitbox.AddItem("Neck");
	WeaponMainHitbox.AddItem("Chest");
	WeaponMainHitbox.AddItem("Stomach");
	WeaponMainHitbox.AddItem("Multihitbox");
	WeaponMainGroup.PlaceOtherControl("Hitbox", this, &WeaponMainHitbox);

	WeaponMainSpeed.SetFileId("main_speed");
	WeaponMainSpeed.SetBoundaries(0.f, 100.f);
	WeaponMainSpeed.SetValue(1.f);
	WeaponMainGroup.PlaceOtherControl("Max Speed", this, &WeaponMainSpeed);

	WeaponMainFoV.SetFileId("main_fov");
	WeaponMainFoV.SetBoundaries(0.f, 30.f);
	WeaponMainFoV.SetValue(3.5f);
	WeaponMainGroup.PlaceOtherControl("Field of View", this, &WeaponMainFoV);

	WeaponMainRecoil.SetFileId("main_recoil");
	WeaponMainRecoil.SetBoundaries(0.f, 2.f);
	WeaponMainRecoil.SetValue(1.00f);
	WeaponMainGroup.PlaceOtherControl("Recoil Control", this, &WeaponMainRecoil);

	WeaponMainAimtime.SetValue(0);
	WeaoponMainStartAimtime.SetValue(0);

#pragma endregion

#pragma region Pistols
	WeaponPistGroup.SetPosition(230, 230);
	WeaponPistGroup.SetText("Pistols");
	WeaponPistGroup.SetSize(240, 130);
	RegisterControl(&WeaponPistGroup);

	WeaponPistHitbox.SetFileId("pist_hitbox");
	WeaponPistHitbox.AddItem("Head");
	WeaponPistHitbox.AddItem("Neck");
	WeaponPistHitbox.AddItem("Chest");
	WeaponPistHitbox.AddItem("Stomach");
	WeaponPistHitbox.AddItem("Multihitbox");
	WeaponPistGroup.PlaceOtherControl("Hitbox", this, &WeaponPistHitbox);

	WeaponPistSpeed.SetFileId("pist_speed");
	WeaponPistSpeed.SetBoundaries(0.f, 100.f);
	WeaponPistSpeed.SetValue(1.0f);
	WeaponPistGroup.PlaceOtherControl("Max Speed", this, &WeaponPistSpeed);

	WeaponPistFoV.SetFileId("pist_fov");
	WeaponPistFoV.SetBoundaries(0.f, 30.f);
	WeaponPistFoV.SetValue(3.f);
	WeaponPistGroup.PlaceOtherControl("Field of View", this, &WeaponPistFoV);

	WeaponPistRecoil.SetFileId("pist_recoil");
	WeaponPistRecoil.SetBoundaries(0.f, 2.f);
	WeaponPistRecoil.SetValue(1.00f);
	WeaponPistGroup.PlaceOtherControl("Recoil Control", this, &WeaponPistRecoil);

	WeaponPistAimtime.SetValue(0);
	WeaoponPistStartAimtime.SetValue(0);

#pragma endregion

#pragma region Snipers
	WeaponSnipGroup.SetPosition(480, 230);
	WeaponSnipGroup.SetText("Snipers");
	WeaponSnipGroup.SetSize(200, 130);
	RegisterControl(&WeaponSnipGroup);

	WeaponSnipHitbox.SetFileId("snip_hitbox");
	WeaponSnipHitbox.AddItem("Head");
	WeaponSnipHitbox.AddItem("Neck");
	WeaponSnipHitbox.AddItem("Chest");
	WeaponSnipHitbox.AddItem("Stomach");
	WeaponSnipHitbox.AddItem("Multihitbox");
	WeaponSnipGroup.PlaceOtherControl("Hitbox", this, &WeaponSnipHitbox);

	WeaponSnipSpeed.SetFileId("snip_speed");
	WeaponSnipSpeed.SetBoundaries(0.f, 100.f);
	WeaponSnipSpeed.SetValue(1.5f);
	WeaponSnipGroup.PlaceOtherControl("Max Speed", this, &WeaponSnipSpeed);

	WeaponSnipFoV.SetFileId("snip_fov");
	WeaponSnipFoV.SetBoundaries(0.f, 30.f);
	WeaponSnipFoV.SetValue(2.f);
	WeaponSnipGroup.PlaceOtherControl("Field of View", this, &WeaponSnipFoV);

	WeaponSnipRecoil.SetFileId("snip_recoil");
	WeaponSnipRecoil.SetBoundaries(0.f, 2.f);
	WeaponSnipRecoil.SetValue(1.00f);
	WeaponSnipGroup.PlaceOtherControl("Recoil Control", this, &WeaponSnipRecoil);

	WeaponSnipAimtime.SetValue(0);
	WeaoponSnipStartAimtime.SetValue(0);

	/*
	#pragma region MPs
	WeaponMpGroup.SetPosition(490, 350);
	WeaponMpGroup.SetText("MPs");
	WeaponMpGroup.SetSize(240, 136);
	RegisterControl(&WeaponMpGroup);

	WeaponMpHitbox.SetFileId("mps_hitbox");
	WeaponMpHitbox.AddItem("Head");
	WeaponMpHitbox.AddItem("Neck");
	WeaponMpHitbox.AddItem("Chest");
	WeaponMpHitbox.AddItem("Stomach");
	WeaponMpHitbox.AddItem("Multihitbox");
	WeaponMpGroup.PlaceOtherControl("Hitbox", this, &WeaponMpHitbox);

	WeaponMpSpeed.SetFileId("mps_speed");
	WeaponMpSpeed.SetBoundaries(0.f, 100.f);
	WeaponMpSpeed.SetValue(1.0f);
	WeaponMpGroup.PlaceOtherControl("Max Speed", this, &WeaponMpSpeed);

	WeaponMpFoV.SetFileId("mps_fov");
	WeaponMpFoV.SetBoundaries(0.f, 30.f);
	WeaponMpFoV.SetValue(4.f);
	WeaponMpGroup.PlaceOtherControl("Field of View", this, &WeaponMpFoV);

	WeaponMpRecoil.SetFileId("mps_recoil");
	WeaponMpRecoil.SetBoundaries(0.f, 2.f);
	WeaponMpRecoil.SetValue(1.00f);
	WeaponMpGroup.PlaceOtherControl("Recoil Control", this, &WeaponMpRecoil);

	WeaponMpAimtime.SetValue(0);
	WeaoponMpStartAimtime.SetValue(0);
	#pragma endregion
	*/
}

void CVisualTab::Setup()
{
	SetTitle("Visuals");

	ActiveLabel.SetPosition(40, 1);
	ActiveLabel.SetText("Active");
	RegisterControl(&ActiveLabel);

	Active.SetFileId("active");
	Active.SetState(true);
	Active.SetPosition(10, 1);
	RegisterControl(&Active);

#pragma region Options
	BoxGroup.SetText("ESP");
	BoxGroup.SetPosition(20, 15);
	BoxGroup.SetSize(173, 350);
	RegisterControl(&BoxGroup);

	OptionsBox.SetFileId("otr_showbox");
	OptionsBox.AddItem("Off");
	OptionsBox.AddItem("Genuine");
//	OptionsBox.AddItem("Classic");
	BoxGroup.PlaceOtherControl("Box Esp", this, &OptionsBox);

//	OptionsName.SetFileId("opt_name");
//	BoxGroup.PlaceCheckBox("Name", this, &OptionsName);

	OptionsWeapon.SetFileId("opt_weapon");
	BoxGroup.PlaceCheckBox("Weapon", this, &OptionsWeapon);

	OptionHealthEnable.SetFileId("opt_health");
	BoxGroup.PlaceCheckBox("Health", this, &OptionHealthEnable);

	OptionsArmor.SetFileId("opt_armor");
	BoxGroup.PlaceCheckBox("Armor", this, &OptionsArmor);

	OptionsSkeleton.SetFileId("opt_bone");
	BoxGroup.PlaceCheckBox("Skeleton", this, &OptionsSkeleton);

	OptionsGlow.SetFileId("opt_glow");
	BoxGroup.PlaceCheckBox("Glow", this, &OptionsGlow);

	EntityGlow.SetFileId("opt_entityglow");
	BoxGroup.PlaceCheckBox("Entity Glow", this, &EntityGlow);

	ResolverInfo.SetFileId("opt_resolverinfo");
	BoxGroup.PlaceCheckBox("Resolver Info", this, &ResolverInfo);

	BacktrackingLol.SetFileId("opt_backdot");
	BoxGroup.PlaceCheckBox("View Backtrack", this, &BacktrackingLol);

	OptionsCompRank.SetFileId("opt_comprank");
	BoxGroup.PlaceCheckBox("Rank", this, &OptionsCompRank);

	SpecList.SetFileId("otr_speclist");
	BoxGroup.PlaceCheckBox("Spectators", this, &SpecList);

	Logs.SetFileId("otr_logs");
	BoxGroup.PlaceCheckBox("Logs", this, &Logs);






	ChamsGroup.SetText("Model");
	ChamsGroup.SetPosition(210, 7);  //-23
	ChamsGroup.SetSize(200, 110);
	RegisterControl(&ChamsGroup);

	OptionsChams.SetFileId("opt_chams");
	OptionsChams.AddItem("Off");
	OptionsChams.AddItem("Normal");
	OptionsChams.AddItem("Flat");
	ChamsGroup.PlaceOtherControl("Chams", this, &OptionsChams);

	OtherNoHands.SetFileId("otr_hands");
	OtherNoHands.AddItem("Off");
	OtherNoHands.AddItem("None");
	OtherNoHands.AddItem("Ghost");
	OtherNoHands.AddItem("Wire (!)");
	OtherNoHands.AddItem("Chams");
	OtherNoHands.AddItem("Rainbow");
	ChamsGroup.PlaceOtherControl("Hands", this, &OtherNoHands);

	ChamsVisibleOnly.SetFileId("opt_chamsvisonly");
	ChamsGroup.PlaceCheckBox("WallHack", this, &ChamsVisibleOnly);

	pLocalOpacity.SetFileId("otr_plocal_opacity");
	ChamsGroup.PlaceCheckBox("Transparent", this, &pLocalOpacity);


	OptionsGroup.SetText("Misc");
	OptionsGroup.SetPosition(431, 7);
	OptionsGroup.SetSize(250, 70);
	RegisterControl(&OptionsGroup);

	OtherHitmarker.SetFileId("otr_hitmarker");
	OptionsGroup.PlaceCheckBox("Hitmarker", this, &OtherHitmarker);

	HitmarkerSound.SetFileId("otr_hitmarkersound");
	HitmarkerSound.AddItem("Off");
	HitmarkerSound.AddItem("Default");
	HitmarkerSound.AddItem("Metalic");
	HitmarkerSound.AddItem("Lil Pump");
	OptionsGroup.PlaceOtherControl("Hitsound", this, &HitmarkerSound);




	RemovalsGroup.SetText("Removals");
	RemovalsGroup.SetPosition(210, 250);
	RemovalsGroup.SetSize(200, 115);
	RegisterControl(&RemovalsGroup);




	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	RemovalsGroup.PlaceCheckBox("Remove Visual Recoil", this, &OtherNoVisualRecoil);

	OtherNoFlash.SetFileId("otr_noflash");
	RemovalsGroup.PlaceCheckBox("Remove Flash", this, &OtherNoFlash);

	OtherNoSmoke.SetFileId("otr_nosmoke");
	RemovalsGroup.PlaceCheckBox("Remove Smoke", this, &OtherNoSmoke);

	OtherNoScope.SetFileId("otr_noscope");
	RemovalsGroup.PlaceCheckBox("Remove Scope", this, &OtherNoScope);

#pragma endregion Setting up the Options controls

#pragma region Filters
	FiltersGroup.SetText("Filters");
	FiltersGroup.SetPosition(431, 105);
	FiltersGroup.SetSize(250, 180);
	RegisterControl(&FiltersGroup);

	FiltersAll.SetFileId("ftr_all");
	FiltersGroup.PlaceCheckBox("All", this, &FiltersAll);

	FiltersPlayers.SetFileId("ftr_players");
	FiltersGroup.PlaceCheckBox("Players", this, &FiltersPlayers);

	FiltersSelf.SetFileId("ftr_self");
	FiltersGroup.PlaceCheckBox("Client", this, &FiltersSelf);

	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	FiltersGroup.PlaceCheckBox("Enemies Only", this, &FiltersEnemiesOnly);

	FiltersWeapons.SetFileId("ftr_weaps");
	FiltersGroup.PlaceCheckBox("Weapons", this, &FiltersWeapons);

	FiltersNades.SetFileId("ftr_nades");
	FiltersGroup.PlaceCheckBox("Explosives", this, &FiltersNades);

	FiltersC4.SetFileId("ftr_c4");
	FiltersGroup.PlaceCheckBox("C4", this, &FiltersC4);

#pragma endregion Setting up the Filters controls

#pragma region Other
	OtherGroup.SetText("Draw");
	OtherGroup.SetPosition(431, 295); //
	OtherGroup.SetSize(250, 120);
	RegisterControl(&OtherGroup);

	colmodupdate.SetFileId("otr_night");
	OtherGroup.PlaceLabledControl("  Force Refresh", this, &colmodupdate);

	Watermark.SetFileId("otr_watermark");
	Watermark.SetState(true);
	OtherGroup.PlaceCheckBox("Draw Watermark", this, &Watermark);

	colmodr.SetFileId("night_amm");
	OtherGroup.PlaceLabledControl("  World Mod", this, &colmodr);
	colmodr.SetBoundaries(0.01f, 1.00f);
	colmodr.SetValue(0.15f);

	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 70.f);
	OtherFOV.SetValue(0.f);
	OtherGroup.PlaceOtherControl("Fov Changer", this, &OtherFOV);




	RenderGroup.SetText("Render");
	RenderGroup.SetPosition(210, 130);
	RenderGroup.SetSize(200, 110);
	RegisterControl(&RenderGroup);


	OtherSpreadCrosshair.SetFileId("otr_spreadhair");
	RenderGroup.PlaceCheckBox("Show Innacuracy", this, &OtherSpreadCrosshair);


	FakeAngleChams.SetFileId("fakechams");
	RenderGroup.PlaceCheckBox("FakeY Ghost", this, &FakeAngleChams);

	AALines.SetFileId("opt_aalines");
	RenderGroup.PlaceCheckBox("Yaw Lines", this, &AALines);

	OptionsAimSpot.SetFileId("opt_aimspot");
	OptionsAimSpot.AddItem("Off");
	OptionsAimSpot.AddItem("Head");
	OptionsAimSpot.AddItem("Main");
	RenderGroup.PlaceOtherControl("Aim Points", this, &OptionsAimSpot);


#pragma endregion Setting up the Other controls
}

void ColourTab::Setup()
{
	SetTitle("Colour");


	//---------------------------ColourESP---------------------------//
	esp.SetPosition(5, 15);
	esp.SetSize(210, 160);
	esp.SetText("ESP");
	RegisterControl(&esp);

	TespR.SetFileId("tEsp1");
	TespR.SetBoundaries(0, 255);
	TespR.SetValue(255);
	esp.PlaceOtherControl("T: Red", this, &TespR);

	TespG.SetFileId("tEsp2");
	TespG.SetBoundaries(0, 255);
	TespG.SetValue(255);
	esp.PlaceOtherControl("T: Green", this, &TespG);

	TespB.SetFileId("tEsp3");
	TespB.SetBoundaries(0, 255);
	TespB.SetValue(255);
	esp.PlaceOtherControl("T: Blue", this, &TespB);

	CTespR.SetFileId("ctEsp1");
	CTespR.SetBoundaries(0, 255);
	CTespR.SetValue(255);
	esp.PlaceOtherControl("CT: Red", this, &CTespR);

	CTespG.SetFileId("ctEsp2");
	CTespG.SetBoundaries(0, 255);
	CTespG.SetValue(255);
	esp.PlaceOtherControl("CT: Green", this, &CTespG);

	CTespB.SetFileId("ctEsp3");
	CTespB.SetBoundaries(0, 255);
	CTespB.SetValue(255);
	esp.PlaceOtherControl("CT: Blue", this, &CTespB);

	//---------------------------ColourChams1---------------------------//

	cham3.SetPosition(475, 15);
	cham3.SetSize(210, 90);
	cham3.SetText("Chams (1)");
	RegisterControl(&cham3);

	vchamtr.SetFileId("chamt1");
	vchamtr.SetBoundaries(0, 255);
	vchamtr.SetValue(255);
	cham3.PlaceOtherControl("T InVis: Red", this, &vchamtr);

	vchamtg.SetFileId("chamt2");
	vchamtg.SetBoundaries(0, 255);
	vchamtg.SetValue(0);
	cham3.PlaceOtherControl("T InVis: Green", this, &vchamtg);

	vchamtb.SetFileId("chamt3");
	vchamtb.SetBoundaries(0, 255);
	vchamtb.SetValue(255);
	cham3.PlaceOtherControl("T InVis: Blue", this, &vchamtb);

	//---------------------------ColourChams2---------------------------//

	cham4.SetPosition(475, 115);
	cham4.SetSize(210, 90);
	cham4.SetText("Chams (2)");
	RegisterControl(&cham4);

	vchamctr.SetFileId("chamct1");
	vchamctr.SetBoundaries(0, 255);
	vchamctr.SetValue(40);
	cham4.PlaceOtherControl("CT InVis: Red", this, &vchamctr);

	vchamctg.SetFileId("chamct2");
	vchamctg.SetBoundaries(0, 255);
	vchamctg.SetValue(255);
	cham4.PlaceOtherControl("CT InVis: Green", this, &vchamctg);

	vchamctb.SetFileId("chamct3");
	vchamctb.SetBoundaries(0, 255);
	vchamctb.SetValue(0);
	cham4.PlaceOtherControl("CT InVis: Blue", this, &vchamctb);	
	
	//---------------------------ColourChams3---------------------------//

	cham.SetPosition(475, 215);
	cham.SetSize(210, 90);
	cham.SetText("Chams (3)");
	RegisterControl(&cham);

	chamtr.SetFileId("ichamt1");
	chamtr.SetBoundaries(0, 255);
	chamtr.SetValue(255);
	cham.PlaceOtherControl("T Vis: Red", this, &chamtr);

	chamtg.SetFileId("ichamt2");
	chamtg.SetBoundaries(0, 255);
	chamtg.SetValue(0);
	cham.PlaceOtherControl("T Vis: Green", this, &chamtg);

	chamtb.SetFileId("ichamt3");
	chamtb.SetBoundaries(0, 255);
	chamtb.SetValue(255);
	cham.PlaceOtherControl("T Vis: Blue", this, &chamtb);

	//---------------------------ColourChams4---------------------------//

	cham2.SetPosition(475, 315);
	cham2.SetSize(210, 90);
	cham2.SetText("Chams (4)");
	RegisterControl(&cham2);

	chamctr.SetFileId("ichamct1");
	chamctr.SetBoundaries(0, 255);
	chamctr.SetValue(0);
	cham2.PlaceOtherControl("CT Vis: Red", this, &chamctr);

	chamctg.SetFileId("ichamct2");
	chamctg.SetBoundaries(0, 255);
	chamctg.SetValue(255);
	cham2.PlaceOtherControl("CT Vis: Green", this, &chamctg);

	chamctb.SetFileId("ichamct3");
	chamctb.SetBoundaries(0, 255);
	chamctb.SetValue(0);
	cham2.PlaceOtherControl("CT Vis: Blue", this, &chamctb);

	//---------------------------Watermark---------------------------//

	watermarkG.SetPosition(5, 190);
	watermarkG.SetSize(210, 90);
	watermarkG.SetText("Dreamware");
	RegisterControl(&watermarkG);

	markr.SetFileId("mark1");
	markr.SetBoundaries(0, 255);
	markr.SetValue(230);
	watermarkG.PlaceOtherControl("'V4' Red:", this, &markr);

	markg.SetFileId("mark2");
	markg.SetBoundaries(0, 255);
	markg.SetValue(20);
	watermarkG.PlaceOtherControl("'V4' Green:", this, &markg);

	markb.SetFileId("mark3");
	markb.SetBoundaries(0, 255);
	markb.SetValue(210);
	watermarkG.PlaceOtherControl("'V4' Blue:", this, &markb);
	
	//---------------------------Title---------------------------//
	gtitle.SetPosition(5, 290);
	gtitle.SetSize(210, 95);
	gtitle.SetText("Controls");
	RegisterControl(&gtitle);

	titler.SetFileId("title1");
	titler.SetBoundaries(10, 255);
	titler.SetValue(10);
	gtitle.PlaceOtherControl("Control Red:", this, &titler);

	titleg.SetFileId("title2");
	titleg.SetBoundaries(10, 255);
	titleg.SetValue(255);
	gtitle.PlaceOtherControl("Control Green:", this, &titleg);

	titleb.SetFileId("title3");
	titleb.SetBoundaries(10, 255);
	titleb.SetValue(10 );
	gtitle.PlaceOtherControl("Control Blue:", this, &titleb);

	//---------------------------Rainbow---------------------------//

	grainbow.SetPosition(220, 80);
	grainbow.SetSize(250, 305);
	grainbow.SetText("Rainbow Bar");
	RegisterControl(&grainbow);

	//----out-left----//
	grad1r.SetFileId("grad1r");
	grad1r.SetBoundaries(0, 255);
	grad1r.SetValue(190);
	grainbow.PlaceOtherControl("Exterior Left: Red", this, &grad1r);

	grad1g.SetFileId("grad1g");
	grad1g.SetBoundaries(0, 255);
	grad1g.SetValue(5);
	grainbow.PlaceOtherControl("Exterior Left: Green", this, &grad1g);

	grad1b.SetFileId("grad1b");
	grad1b.SetBoundaries(0, 255);
	grad1b.SetValue(220);
	grainbow.PlaceOtherControl("Exterior Left: Blue", this, &grad1b);

	//----in-left----//
	grad2r.SetFileId("grad2r");
	grad2r.SetBoundaries(0, 255);
	grad2r.SetValue(185);
	grainbow.PlaceOtherControl("Interior Left: Red", this, &grad2r);

	grad2g.SetFileId("grad2g");
	grad2g.SetBoundaries(0, 255);
	grad2g.SetValue(0);
	grainbow.PlaceOtherControl("Interior Left: Green", this, &grad2g);

	grad2b.SetFileId("grad2b");
	grad2b.SetBoundaries(0, 255);
	grad2b.SetValue(255);
	grainbow.PlaceOtherControl("Interior Left: Blue", this, &grad2b);

	//----in-right----//
	grad3r.SetFileId("grad3r");
	grad3r.SetBoundaries(0, 255);
	grad3r.SetValue(160);
	grainbow.PlaceOtherControl("Interior Right: Red", this, &grad3r);

	grad3g.SetFileId("grad3g");
	grad3g.SetBoundaries(0, 255);
	grad3g.SetValue(0);
	grainbow.PlaceOtherControl("Interior Right: Green", this, &grad3g);

	grad3b.SetFileId("grad3b");
	grad3b.SetBoundaries(0, 255);
	grad3b.SetValue(255);
	grainbow.PlaceOtherControl("Interior Right: Blue", this, &grad3b);

	//----out-right----//
	grad4r.SetFileId("grad4r");
	grad4r.SetBoundaries(0, 255);
	grad4r.SetValue(255);
	grainbow.PlaceOtherControl("Exterior Right: Red", this, &grad4r);

	grad4g.SetFileId("grad4g");
	grad4g.SetBoundaries(0, 255);
	grad4g.SetValue(25);
	grainbow.PlaceOtherControl("Exterior Right: Green", this, &grad4g);

	grad4b.SetFileId("grad4b");
	grad4b.SetBoundaries(0, 255);
	grad4b.SetValue(190);
	grainbow.PlaceOtherControl("Exterior Right: Blue", this, &grad4b);

	//---------------------------Alpha---------------------------//
	mAlpha.SetPosition(220, 15);
	mAlpha.SetSize(250, 60);
	mAlpha.SetText("Alpha");
	RegisterControl(&mAlpha);

	alpEsp.SetFileId("espA");
	alpEsp.SetBoundaries(0, 255);
	alpEsp.SetValue(255);
	mAlpha.PlaceOtherControl("ESP: Alpha", this, &alpEsp);

	alpMenu.SetFileId("menuA");
	alpMenu.SetBoundaries(0, 255);
	alpMenu.SetValue(255);
	mAlpha.PlaceOtherControl("Menu: Alpha", this, &alpMenu);


};
// = (Menu::Window.aabuild.pitch);

void Caabuild::Setup()
{
	SetTitle("AA buidler");
	pitchctrl.SetPosition(5, 15);
	pitchctrl.SetSize(220, 50);
	pitchctrl.SetText("Pitch");
	RegisterControl(&pitchctrl);
	//--------------------------Pitch-Pick--------------------------//
	pitchpick.SetFileId("pitchpick");
	pitchpick.AddItem("Static");
	pitchpick.AddItem("Jitter");
	pitchpick.AddItem("Safe Fake");
	pitchpick.AddItem("UnSafe Fake");
	pitchctrl.PlaceOtherControl("Pitch Type", this, &pitchpick);

	//--------------------------Static-Pitch--------------------------//

	pitchctrl2.SetPosition(5, 75);
	pitchctrl2.SetSize(220, 50);
	pitchctrl2.SetText("Static Pitch");
	RegisterControl(&pitchctrl2);

	pitchbase.SetFileId("custompitch");
	pitchbase.SetBoundaries(0, 89);
	pitchbase.SetValue(0);
	pitchctrl2.PlaceOtherControl("Pitch:", this, &pitchbase);

	//--------------------------Jiitery-Pitch--------------------------// 

	pitchctrl3.SetPosition(5, 135);
	pitchctrl3.SetSize(220, 70);
	pitchctrl3.SetText("Jitter Pitch");
	RegisterControl(&pitchctrl3);

	pitch_jitter_from.SetFileId("customJitpitch");
	pitch_jitter_from.SetBoundaries(0, 89);
	pitch_jitter_from.SetValue(0);
	pitchctrl3.PlaceOtherControl("Jitter From:", this, &pitch_jitter_from);

	pitch_jitter_to.SetFileId("customJitpitch2");
	pitch_jitter_to.SetBoundaries(0, 89);
	pitch_jitter_to.SetValue(80);
	pitchctrl3.PlaceOtherControl("Jitter To:", this, &pitch_jitter_to);

	//--------------------------MMfake-Pitch--------------------------//

	pitchctrl4.SetPosition(5, 220);
	pitchctrl4.SetSize(220, 70);
	pitchctrl4.SetText("Safe Fake Pitch");
	RegisterControl(&pitchctrl4);

	pitch_safe_fake.SetFileId("customfakepitch");
	pitch_safe_fake.SetBoundaries(0, 89);
	pitch_safe_fake.SetValue(0);
	pitchctrl4.PlaceOtherControl("Fake:", this, &pitch_safe_fake);

	pitch_safe_real.SetFileId("customfakepitch2");
	pitch_safe_real.SetBoundaries(0, 89);
	pitch_safe_real.SetValue(80);
	pitchctrl4.PlaceOtherControl("Real:", this, &pitch_safe_real);


	//--------------------------NoSpread-Pitch--------------------------//

	pitchctrl5.SetPosition(5, 300);
	pitchctrl5.SetSize(220, 70);
	pitchctrl5.SetText("NoSpread Fake:");
	RegisterControl(&pitchctrl5);

	pitch_unsafe_fake.SetFileId("customfakepitch3");
	pitch_unsafe_fake.SetBoundaries(0, 1080);
	pitch_unsafe_fake.SetValue(0);
	pitchctrl5.PlaceOtherControl("(!) Fake:", this, &pitch_unsafe_fake);	

	pitch_unsafe_real.SetFileId("customfakepitch4");
	pitch_unsafe_real.SetBoundaries(0, 1080);
	pitch_unsafe_real.SetValue(600);
	pitchctrl5.PlaceOtherControl("(!) Real:", this, &pitch_unsafe_real);


	//--------------------------------------Yaw-pick--------------------------------------//


	yawctrl1.SetPosition(235, 15);
	yawctrl1.SetSize(220, 50);
	yawctrl1.SetText("Yaw");
	RegisterControl(&yawctrl1);

	yawpick.SetFileId("yawpick");
	yawpick.AddItem("Static");
	yawpick.AddItem("Jitter");
	yawpick.AddItem("Lowerbody");
	yawpick.AddItem("Spin");
	yawctrl1.PlaceOtherControl("Yaw Type", this, &yawpick);

	//--------------------------Yaw-Static--------------------------//

	yawctrl2.SetPosition(235, 75);
	yawctrl2.SetSize(220, 50);
	yawctrl2.SetText("Static");
	RegisterControl(&yawctrl2);

	yawbase.SetFileId("yawstat");
	yawbase.SetBoundaries(0, 360); // keep anti UT on. If you go over 180, it will just be left.  
	yawbase.SetValue(180);
	yawctrl2.PlaceOtherControl("Yaw:", this, &yawbase);

	//--------------------------Yaw-Jitter--------------------------//

	yawctrl3.SetPosition(235, 135);
	yawctrl3.SetSize(220, 70);
	yawctrl3.SetText("Jitter");
	RegisterControl(&yawctrl3);

	yaw_jitter_from.SetFileId("yawjit");
	yaw_jitter_from.SetBoundaries(0, 360);
	yaw_jitter_from.SetValue(150);
	yawctrl3.PlaceOtherControl("Jitter From", this, &yaw_jitter_from);

	yaw_jitter_to.SetFileId("yawjit2");
	yaw_jitter_to.SetBoundaries(0, 360);
	yaw_jitter_to.SetValue(200);
	yawctrl3.PlaceOtherControl("Jitter To", this, &yaw_jitter_to);

	//--------------------------Yaw-LBYSwitch--------------------------//

	yawctrl4.SetPosition(235, 220);
	yawctrl4.SetSize(220, 90);
	yawctrl4.SetText("Lowerbody");
	RegisterControl(&yawctrl4);

	yaw_add_jitter.SetFileId("addjitter");
	yawctrl4.PlaceCheckBox("Add Jitter", this, &yaw_add_jitter);

	yaw_switch_from.SetFileId("yawswitch");
	yaw_switch_from.SetBoundaries(0, 360);
	yaw_switch_from.SetValue(270);
	yawctrl4.PlaceOtherControl("Switch From", this, &yaw_switch_from);

	yaw_switch_to.SetFileId("yawjit2");
	yaw_switch_to.SetBoundaries(0, 360);
	yaw_switch_to.SetValue(90);
	yawctrl4.PlaceOtherControl("Switch To", this, &yaw_switch_to);

	//--------------------------Yaw-Spin--------------------------//

	yawctrl5.SetPosition(235, 320);
	yawctrl5.SetSize(220, 50);
	yawctrl5.SetText("Spin");
	RegisterControl(&yawctrl5);

	spinspeed.SetFileId("yawspinspeed");
	spinspeed.SetBoundaries(0, 100);
	spinspeed.SetValue(50);
	yawctrl5.PlaceOtherControl("Velocity", this, &spinspeed); // GayCare premium


   //---------------------------------------Fake-Yaw---------------------------------------//
	fyawctrl1.SetPosition(465, 15);
	fyawctrl1.SetSize(220, 50);
	fyawctrl1.SetText("Fake Yaw");
	RegisterControl(&fyawctrl1);

	fyawpick.SetFileId("fyawpick");
	fyawpick.AddItem("Static");
	fyawpick.AddItem("Jitter");
	fyawpick.AddItem("Lowerbody");
	fyawpick.AddItem("Spin");
	fyawctrl1.PlaceOtherControl("Fake Type", this, &fyawpick);

	//--------------------------FYaw-Static--------------------------//

	fyawctrl2.SetPosition(465, 75);
	fyawctrl2.SetSize(220, 50);
	fyawctrl2.SetText("Static");
	RegisterControl(&fyawctrl2);

	fyawbase.SetFileId("fyawstat");
	fyawbase.SetBoundaries(0, 360); // keep anti UT on. If you go over 180, it will just be left.  
	fyawbase.SetValue(180);
	fyawctrl2.PlaceOtherControl("Fake:", this, &fyawbase);

	//--------------------------FYaw-Jitter--------------------------//

	fyawctrl3.SetPosition(465, 135);
	fyawctrl3.SetSize(220, 70);
	fyawctrl3.SetText("Jitter");
	RegisterControl(&fyawctrl3);

	fyaw_jitter_from.SetFileId("fyawjit");
	fyaw_jitter_from.SetBoundaries(0, 360);
	fyaw_jitter_from.SetValue(200);
	fyawctrl3.PlaceOtherControl("Jitter From", this, &fyaw_jitter_from);

	fyaw_jitter_to.SetFileId("fyawjit2");
	fyaw_jitter_to.SetBoundaries(0, 360);
	fyaw_jitter_to.SetValue(150);
	fyawctrl3.PlaceOtherControl("Jitter To", this, &fyaw_jitter_to);

	//--------------------------FYaw-LBYSwitch--------------------------//

	fyawctrl4.SetPosition(465, 220);
	fyawctrl4.SetSize(220, 90);
	fyawctrl4.SetText("Lowerbody");
	RegisterControl(&fyawctrl4);

	fyaw_add_jitter.SetFileId("faddjitter");
	fyawctrl4.PlaceCheckBox("Add Jitter", this, &fyaw_add_jitter);

	fyaw_switch_from.SetFileId("fyawswitch");
	fyaw_switch_from.SetBoundaries(0, 360);
	fyaw_switch_from.SetValue(90);
	fyawctrl4.PlaceOtherControl("Switch From", this, &fyaw_switch_from);

	fyaw_switch_to.SetFileId("fyawjit2");
	fyaw_switch_to.SetBoundaries(0, 360);
	fyaw_switch_to.SetValue(270);
	fyawctrl4.PlaceOtherControl("Switch To", this, &fyaw_switch_to);

	//--------------------------Yaw-Spin--------------------------//

	fyawctrl5.SetPosition(465, 320);
	fyawctrl5.SetSize(220, 50);
	fyawctrl5.SetText("Spin");
	RegisterControl(&fyawctrl5);

	fspinspeed.SetFileId("fyawspinspeed");
	fspinspeed.SetBoundaries(0, 100);
	fspinspeed.SetValue(50);
	fyawctrl5.PlaceOtherControl("Velocity", this, &fspinspeed); 

	//----------------------------------Fake-Lag----------------------------------//
	fakelag.SetPosition(5, 380);
	fakelag.SetSize(220, 60);
	fakelag.SetText("Fakelag");
	RegisterControl(&fakelag);

	FakeLagTyp.SetFileId("b1gfakelaglmao");
	FakeLagTyp.AddItem("Off");
	FakeLagTyp.AddItem("Minimal");
	FakeLagTyp.AddItem("Moderate");
//	FakeLagTyp.AddItem("Maximal");
	fakelag.PlaceOtherControl("FakeLag", this, &FakeLagTyp);

	antilbyctrl.SetPosition(235, 380);
	antilbyctrl.SetSize(450, 60);
	antilbyctrl.SetText("Anti-LBY");
	RegisterControl(&antilbyctrl);


	Antilby.SetFileId("antilby");
	Antilby.SetBoundaries(0, 180);
	Antilby.SetValue(95);
	antilbyctrl.PlaceOtherControl("Standing", this, &Antilby);   

	moveantilby.SetFileId("antilby2");
	moveantilby.SetBoundaries(0, 180);
	moveantilby.SetValue(120);
	antilbyctrl.PlaceOtherControl("Moving", this, &moveantilby);   
};

/*
CComboBox yawpick; // pick what type of pitch we want

CSlider yawbase; // base
CCheckBox yaw_add_jitter;

CSlider yaw_switch_from; // yaw starting point
CSlider yaw_switch_to; // yaw end point

CSlider yaw_jitter_from; // yaw starting point
CSlider yaw_jitter_to; // yaw end point

CSlider yaw_lby_1;
CSlider yaw_lby_2;
CSlider yaw_lby_3;
CSlider yaw_lby_4;


float grad1r = Menu::Window.colourtab.grad1r.GetValue(); // default: 0
float grad1g = Menu::Window.colourtab.grad1g.GetValue(); // default: 170
float grad1b = Menu::Window.colourtab.grad1b.GetValue(); // default: 255

float grad2r = Menu::Window.colourtab.grad2r.GetValue(); // default: 185
float grad2g = Menu::Window.colourtab.grad2g.GetValue(); // default: 0
float grad2b = Menu::Window.colourtab.grad2b.GetValue(); // default: 255

float grad3r = Menu::Window.colourtab.grad3r.GetValue(); // default: 160
float grad3g = Menu::Window.colourtab.grad3g.GetValue(); // default: 0
float grad3b = Menu::Window.colourtab.grad3b.GetValue(); // default: 255

float grad4r = Menu::Window.colourtab.grad4r.GetValue(); // default: 255
float grad4g = Menu::Window.colourtab.grad4g.GetValue(); // default: 25
float grad4b = Menu::Window.colourtab.grad4b.GetValue(); // default: 190

*/

void CSkinchangerTab::Setup()
{  
	SetTitle("Decoration");

	SkinActive.SetPosition(40, 2);
	SkinActive.SetText("Active");
	RegisterControl(&SkinActive);

	SkinEnable.SetFileId("skin_enable");
	SkinEnable.SetPosition(10, 2);
	RegisterControl(&SkinEnable);

	SkinApply.SetText("Apply");
	SkinApply.SetCallback(KnifeApplyCallbk);
	SkinApply.SetPosition(270, 490);
	SkinApply.SetSize(250, 50);
	RegisterControl(&SkinApply);

#pragma region Knife
KnifeGroup.SetPosition(20, 11);
KnifeGroup.SetText("Knifes");
KnifeGroup.SetSize(270, 55);
RegisterControl(&KnifeGroup);

KnifeModel.SetFileId("knife_model");
KnifeModel.AddItem("Bayonet");
KnifeModel.AddItem("Bowie Knife");
KnifeModel.AddItem("Butterfly Knife");
KnifeModel.AddItem("Falchion Knife");
KnifeModel.AddItem("Flip Knife");
KnifeModel.AddItem("Gut Knife");
KnifeModel.AddItem("Huntsman Knife");
KnifeModel.AddItem("Karambit");
KnifeModel.AddItem("M9 Bayonet");
KnifeModel.AddItem("Shadow Daggers");
KnifeGroup.PlaceLabledControl("Knife", this, &KnifeModel);

KnifeSkin.SetFileId("knife_skin");
KnifeSkin.AddItem("Off");
KnifeSkin.AddItem("Crimson Web");
KnifeSkin.AddItem("Bone Mask");
KnifeSkin.AddItem("Fade");
KnifeSkin.AddItem("Night");
KnifeSkin.AddItem("Blue Steel");
KnifeSkin.AddItem("Stained");
KnifeSkin.AddItem("Case Hardened");
KnifeSkin.AddItem("Slaughter");
KnifeSkin.AddItem("Safari Mesh");
KnifeSkin.AddItem("Boreal Forest");
KnifeSkin.AddItem("Ultraviolet");
KnifeSkin.AddItem("Urban Masked");
KnifeSkin.AddItem("Scorched");
KnifeSkin.AddItem("Rust Coat");
KnifeSkin.AddItem("Tiger Tooth");
KnifeSkin.AddItem("Damascus Steel");
KnifeSkin.AddItem("Damascus Steel");
KnifeSkin.AddItem("Marble Fade");
KnifeSkin.AddItem("Rust Coat");
KnifeSkin.AddItem("Doppler Ruby");
KnifeSkin.AddItem("Doppler Sapphire");
KnifeSkin.AddItem("Doppler Blackpearl");
KnifeSkin.AddItem("Doppler Phase 1");
KnifeSkin.AddItem("Doppler Phase 2");
KnifeSkin.AddItem("Doppler Phase 3");
KnifeSkin.AddItem("Doppler Phase 4");
KnifeSkin.AddItem("Gamma Doppler Phase 1");
KnifeSkin.AddItem("Gamma Doppler Phase 2");
KnifeSkin.AddItem("Gamma Doppler Phase 3");
KnifeSkin.AddItem("Gamma Doppler Phase 4");
KnifeSkin.AddItem("Gamma Doppler Emerald");
KnifeGroup.PlaceLabledControl("Skin", this, &KnifeSkin);
#pragma endregion


#pragma region Snipers
Snipergroup.SetPosition(20, 98);
Snipergroup.SetText("Snipers");
Snipergroup.SetSize(270, 55);
RegisterControl(&Snipergroup);

AWPSkin.SetFileId("awp_skin");
AWPSkin.AddItem("BOOM");
AWPSkin.AddItem("Dragon Lore");
AWPSkin.AddItem("Pink DDPAT");
AWPSkin.AddItem("Fever Dream");
AWPSkin.AddItem("Lightning Strike");
AWPSkin.AddItem("Corticera");
AWPSkin.AddItem("Redline");
AWPSkin.AddItem("Man-o'-war");
AWPSkin.AddItem("Graphite");
AWPSkin.AddItem("Electric Hive");
AWPSkin.AddItem("Pit Viper");
AWPSkin.AddItem("Asiimov");
AWPSkin.AddItem("Oni Taiji");
AWPSkin.AddItem("Medusa");
AWPSkin.AddItem("Sun in Leo");
AWPSkin.AddItem("Hyper Beast");
AWPSkin.AddItem("Elite Build");
Snipergroup.PlaceLabledControl("AWP", this, &AWPSkin);

SSG08Skin.SetFileId("sgg08_skin");
SSG08Skin.AddItem("Dragonfire");
SSG08Skin.AddItem("Blood in the Water");
SSG08Skin.AddItem("Ghost Crusader");
SSG08Skin.AddItem("Detour");
SSG08Skin.AddItem("Abyss");
SSG08Skin.AddItem("Big Iron");
Snipergroup.PlaceLabledControl("SGG 08", this, &SSG08Skin);
/*
SCAR20Skin.SetFileId("scar20_skin");
SCAR20Skin.AddItem("Splash Jam");
SCAR20Skin.AddItem("Emerald");
SCAR20Skin.AddItem("Crimson Web");
SCAR20Skin.AddItem("Cardiac");
SCAR20Skin.AddItem("Bloodsport");
SCAR20Skin.AddItem("Cyrex");
SCAR20Skin.AddItem("Grotto");
Snipergroup.PlaceLabledControl("SCAR-20", this, &SCAR20Skin);

G3SG1Skin.SetFileId("g3sg1_skin");
G3SG1Skin.AddItem("Demeter");
G3SG1Skin.AddItem("Azure Zebra");
G3SG1Skin.AddItem("Green Apple");
G3SG1Skin.AddItem("Orange Kimono");
G3SG1Skin.AddItem("Neon Kimono");
G3SG1Skin.AddItem("Murky");
G3SG1Skin.AddItem("Chronos");
G3SG1Skin.AddItem("Flux");
G3SG1Skin.AddItem("The Executioner");
Snipergroup.PlaceLabledControl("G3SG1", this, &G3SG1Skin);
#pragma endregion
*/

#pragma region Rifles
Riflegroup.SetPosition(20, 194);
Riflegroup.SetText("Rifles");
Riflegroup.SetSize(270, 80);
RegisterControl(&Riflegroup);

AK47Skin.SetFileId("ak47_skin");
AK47Skin.AddItem("First Class");
AK47Skin.AddItem("Red Laminate");
AK47Skin.AddItem("Case Hardened");
AK47Skin.AddItem("Black Laminate");
AK47Skin.AddItem("Fire SerpEntity");
AK47Skin.AddItem("Cartel");
AK47Skin.AddItem("Emerald Pinstripe");
AK47Skin.AddItem("Blue Laminate");
AK47Skin.AddItem("Redline");
AK47Skin.AddItem("Vulcan");
AK47Skin.AddItem("Jaguar");
AK47Skin.AddItem("Jet Set");
AK47Skin.AddItem("Wasteland Rebel");
AK47Skin.AddItem("Orbit Mk01");
AK47Skin.AddItem("Hydroponic");
AK47Skin.AddItem("Aquamarine Revenge");
AK47Skin.AddItem("Frontside Misty");
AK47Skin.AddItem("Point Disarray");
AK47Skin.AddItem("Fuel Injector");
AK47Skin.AddItem("Neon Revolution");
AK47Skin.AddItem("Bloodsport");
Riflegroup.PlaceLabledControl("AK-47", this, &AK47Skin);

M41SSkin.SetFileId("m4a1s_skin");
M41SSkin.AddItem("Dark Water");
M41SSkin.AddItem("Hyper Beast");
M41SSkin.AddItem("Decimator");
M41SSkin.AddItem("VariCamo");
M41SSkin.AddItem("Nitro");
M41SSkin.AddItem("Bright Water");
M41SSkin.AddItem("Atomic Alloy");
M41SSkin.AddItem("Blood Tiger");
M41SSkin.AddItem("Guardian");
M41SSkin.AddItem("Master Piece");
M41SSkin.AddItem("Knight");
M41SSkin.AddItem("Cyrex");
M41SSkin.AddItem("Basilisk");
M41SSkin.AddItem("Icarus Fell");
M41SSkin.AddItem("Hot Rod");
M41SSkin.AddItem("Golden Coi");
M41SSkin.AddItem("Chantico's Fire");
M41SSkin.AddItem("Mecha Industries");
M41SSkin.AddItem("Flashback");
Riflegroup.PlaceLabledControl("M4A1-S", this, &M41SSkin);

M4A4Skin.SetFileId("m4a4_skin");
M4A4Skin.AddItem("Bullet Rain");
M4A4Skin.AddItem("Zirka");
M4A4Skin.AddItem("Asiimov");
M4A4Skin.AddItem("Howl");
M4A4Skin.AddItem("X-Ray");
M4A4Skin.AddItem("Desert-Strike");
M4A4Skin.AddItem("Griffin");
M4A4Skin.AddItem("Dragon King");
M4A4Skin.AddItem("Poseidon");
M4A4Skin.AddItem("Daybreak");
M4A4Skin.AddItem("Evil Daimyo");
M4A4Skin.AddItem("Royal Paladin");
M4A4Skin.AddItem("The BattleStar");
M4A4Skin.AddItem("Desolate Space");
M4A4Skin.AddItem("Buzz Kill");
M4A4Skin.AddItem("Hellfire");
Riflegroup.PlaceLabledControl("M4A4", this, &M4A4Skin);
/*
AUGSkin.SetFileId("aug_skin");
AUGSkin.AddItem("Bengal Tiger");
AUGSkin.AddItem("Hot Rod");
AUGSkin.AddItem("Chameleon");
AUGSkin.AddItem("Torque");
AUGSkin.AddItem("Radiation Hazard");
AUGSkin.AddItem("Asterion");
AUGSkin.AddItem("Daedalus");
AUGSkin.AddItem("Akihabara Accept");
AUGSkin.AddItem("Ricochet");
AUGSkin.AddItem("Fleet Flock");
AUGSkin.AddItem("Syd Mead");
Riflegroup.PlaceLabledControl("      AUG", this, &AUGSkin);

FAMASSkin.SetFileId("famas_skin");
FAMASSkin.AddItem("Djinn");
FAMASSkin.AddItem("Afterimage");
FAMASSkin.AddItem("Doomkitty");
FAMASSkin.AddItem("Spitfire");
FAMASSkin.AddItem("Teardown");
FAMASSkin.AddItem("Hexane");
FAMASSkin.AddItem("Pulse");
FAMASSkin.AddItem("Sergeant");
FAMASSkin.AddItem("Styx");
FAMASSkin.AddItem("Neural Net");
FAMASSkin.AddItem("Survivor");
FAMASSkin.AddItem("Valence");
FAMASSkin.AddItem("Roll Cage");
FAMASSkin.AddItem("Mecha Industries");
Riflegroup.PlaceLabledControl("FAMAS", this, &FAMASSkin);

GALILSkin.SetFileId("galil_skin");
GALILSkin.AddItem("Orange DDPAT");
GALILSkin.AddItem("Eco");
GALILSkin.AddItem("Stone Cold");
GALILSkin.AddItem("Cerberus");
GALILSkin.AddItem("Aqua Terrace");
GALILSkin.AddItem("Chatterbox");
GALILSkin.AddItem("Firefight");
GALILSkin.AddItem("Rocket Pop");
GALILSkin.AddItem("Sugar Rush");
Riflegroup.PlaceLabledControl("GALIL", this, &GALILSkin);

SG553Skin.SetFileId("sg552_skin");
SG553Skin.AddItem("Bulldozer");
SG553Skin.AddItem("Ultraviolet");
SG553Skin.AddItem("Damascus Steel");
SG553Skin.AddItem("Fallout Warning");
SG553Skin.AddItem("Damascus Steel");
SG553Skin.AddItem("Pulse");
SG553Skin.AddItem("Army Sheen");
SG553Skin.AddItem("Traveler");
SG553Skin.AddItem("Fallout Warning");
SG553Skin.AddItem("Cyrex");
SG553Skin.AddItem("Tiger Moth");
SG553Skin.AddItem("Atlas");
Riflegroup.PlaceLabledControl("SG553", this, &SG553Skin);
#pragma endregion

#pragma region MPs
MPGroup.SetPosition(140, 443);
MPGroup.SetText("MPs");
MPGroup.SetSize(376, 173);
RegisterControl(&MPGroup);

MAC10Skin.SetFileId("mac10_skin");
MAC10Skin.AddItem("Fade");
MAC10Skin.AddItem("Neon Rider");
MAC10Skin.AddItem("Ultraviolet");
MAC10Skin.AddItem("Palm");
MAC10Skin.AddItem("Graven");
MAC10Skin.AddItem("Tatter");
MAC10Skin.AddItem("Amber Fade");
MAC10Skin.AddItem("Heat");
MAC10Skin.AddItem("Curse");
MAC10Skin.AddItem("Indigo");
MAC10Skin.AddItem("Commuter");
MAC10Skin.AddItem("Nuclear Garden");
MAC10Skin.AddItem("Malachite");
MAC10Skin.AddItem("Rangeen");
MAC10Skin.AddItem("Lapis Gator");
MPGroup.PlaceLabledControl(" MAC-10", this, &MAC10Skin);

P90Skin.SetFileId("p90_skin");
P90Skin.AddItem("Cold Blooded");
P90Skin.AddItem("Death by Kitty");
P90Skin.AddItem("Shapewood");
P90Skin.AddItem("Emerald Dragon");
P90Skin.AddItem("Asiimov");
P90Skin.AddItem("Virus");
MPGroup.PlaceLabledControl("P90", this, &P90Skin);

UMP45Skin.SetFileId("ump45_skin");
UMP45Skin.AddItem("Blaze");
UMP45Skin.AddItem("Primal Saber");
UMP45Skin.AddItem("Minotaurs Labyrinth");
UMP45Skin.AddItem("Grand Prix");
MPGroup.PlaceLabledControl("UMP-45", this, &UMP45Skin);

BIZONSkin.SetFileId("bizon_skin");
BIZONSkin.AddItem("Blue Streak");
BIZONSkin.AddItem("Antique");
BIZONSkin.AddItem("Judgement of Anubis");
BIZONSkin.AddItem("Osiris");
BIZONSkin.AddItem("Fuel Rod");
MPGroup.PlaceLabledControl("PP-Bizon", this, &BIZONSkin);

MP7Skin.SetFileId("mp7_skin");
MP7Skin.AddItem("Whiteout");
MP7Skin.AddItem("Nemesis");
MP7Skin.AddItem("Impire");
MPGroup.PlaceLabledControl("MP7", this, &MP7Skin);

MP9Skin.SetFileId("mp9_skin");
MP9Skin.AddItem("Ruby Poison Dart");
MP9Skin.AddItem("Hot Rod");
MPGroup.PlaceLabledControl("MP9", this, &MP9Skin);
*/
#pragma endregion

#pragma region Pistols
PistolGroup.SetPosition(300, 98);
PistolGroup.SetText("Pistols");
PistolGroup.SetSize(340, 250);
RegisterControl(&PistolGroup);

GLOCKSkin.SetFileId("glock_skin");
GLOCKSkin.AddItem("Wasteland Rebel");
GLOCKSkin.AddItem("Twillight Galaxy");
GLOCKSkin.AddItem("Fade");
GLOCKSkin.AddItem("Dragon Tattoo");
GLOCKSkin.AddItem("Water Elemental");
GLOCKSkin.AddItem("Royal Legion");
GLOCKSkin.AddItem("Grinder");
GLOCKSkin.AddItem("Reactor");
GLOCKSkin.AddItem("Weasel");
PistolGroup.PlaceLabledControl("Glock", this, &GLOCKSkin);

USPSSkin.SetFileId("usps_skin");
USPSSkin.AddItem("Dark Water");;
USPSSkin.AddItem("Overgrowth");
USPSSkin.AddItem("Caiman");
USPSSkin.AddItem("Blood Tiger");
USPSSkin.AddItem("Serum");
USPSSkin.AddItem("Neo-Noir");
USPSSkin.AddItem("Stainless");
USPSSkin.AddItem("Guardian");
USPSSkin.AddItem("Orion");
USPSSkin.AddItem("Road Rash");
USPSSkin.AddItem("Royal Blue");
USPSSkin.AddItem("Business Class");
USPSSkin.AddItem("Para Green");
USPSSkin.AddItem("Torque");
USPSSkin.AddItem("Kill Confirmed");
USPSSkin.AddItem("Lead Conduit");
USPSSkin.AddItem("Cyrex");
PistolGroup.PlaceLabledControl("USP-S", this, &USPSSkin);

DEAGLESkin.SetFileId("deagle_skin");
DEAGLESkin.AddItem("Blaze");
DEAGLESkin.AddItem("Oxide Blaze");
DEAGLESkin.AddItem("Kumicho Dragon");
DEAGLESkin.AddItem("Sunset Storm");
PistolGroup.PlaceLabledControl("Deagle", this, &DEAGLESkin);

DUALSSkin.SetFileId("duals_skin");
DUALSSkin.AddItem("Dualing Dragons");
DUALSSkin.AddItem("Anodized Navy");
DUALSSkin.AddItem("Duelist");
DUALSSkin.AddItem("Hemoglobin");
DUALSSkin.AddItem("Marina");
DUALSSkin.AddItem("Urban Shock");
PistolGroup.PlaceLabledControl("Duals", this, &DUALSSkin);

RevolverSkin.SetFileId("revolver_skin");
RevolverSkin.AddItem("Fade");
RevolverSkin.AddItem("Crimson Web");
RevolverSkin.AddItem("Amber Fade");
RevolverSkin.AddItem("Reboot");
PistolGroup.PlaceLabledControl("Revolver", this, &RevolverSkin);

FIVESEVENSkin.SetFileId("fiveseven_skin");
FIVESEVENSkin.AddItem("Case Hardened");
FIVESEVENSkin.AddItem("Fowl Play");
FIVESEVENSkin.AddItem("Retrobution");
FIVESEVENSkin.AddItem("Triumvirate");
FIVESEVENSkin.AddItem("Neon Kimono");
FIVESEVENSkin.AddItem("Monkey Business");
FIVESEVENSkin.AddItem("Copper Galaxy");
FIVESEVENSkin.AddItem("Hyper Beast");
PistolGroup.PlaceLabledControl("Five-Seven", this, &FIVESEVENSkin);

TECNINESkin.SetFileId("tec9_skin");
TECNINESkin.AddItem("Terrace");
TECNINESkin.AddItem("Isaac");
TECNINESkin.AddItem("Red Quartz");
TECNINESkin.AddItem("Avalanche");
TECNINESkin.AddItem("Toxic");
TECNINESkin.AddItem("Fuel Injector");
TECNINESkin.AddItem("Re-Entry");
TECNINESkin.AddItem("Bamboo Forest");
TECNINESkin.AddItem("Nuclear Threat");
PistolGroup.PlaceLabledControl("Tec-9", this, &TECNINESkin);

P2000Skin.SetFileId("p2000_skin");
P2000Skin.AddItem("Handgun");
P2000Skin.AddItem("Corticera");
P2000Skin.AddItem("Ocean Foam");
P2000Skin.AddItem("Fire Elemental");
P2000Skin.AddItem("Imperial Dragon");
P2000Skin.AddItem("Ocean Foam");
P2000Skin.AddItem("Amber Fade");
PistolGroup.PlaceLabledControl("P2000", this, &P2000Skin);

P250Skin.SetFileId("p250_skin");
P250Skin.AddItem("Whiteout");
P250Skin.AddItem("Nuclear Threat");
P250Skin.AddItem("Splash");
P250Skin.AddItem("Mehndi");
P250Skin.AddItem("Asiimov");
P250Skin.AddItem("Undertow");
P250Skin.AddItem("Franklin");
P250Skin.AddItem("Supernova");
P250Skin.AddItem("Cartel");
PistolGroup.PlaceLabledControl("P250", this, &P250Skin);

#pragma endregion

#pragma region Skinsettings
SkinsettingsGroup.SetPosition(300, 20); // 	SkinEnable.SetPosition(170, 10);
SkinsettingsGroup.SetText("Customisation");
SkinsettingsGroup.SetSize(270, 55);
RegisterControl(&SkinsettingsGroup);

StatTrakEnable.SetFileId("skin_stattrack");
SkinsettingsGroup.PlaceLabledControl("Stat Trak", this, &StatTrakEnable);


SkinApply.SetText("Set Skins(s)");
SkinApply.SetCallback(KnifeApplyCallbk);
SkinApply.SetPosition(20, 300);
SkinApply.SetSize(270, 106);
RegisterControl(&SkinApply);

#pragma endregion
}



void Menu::SetupMenu()
{
	Window.Setup();

	GUI.RegisterWindow(&Window);
	GUI.BindWindow(VK_INSERT, &Window);
}

void Menu::DoUIFrame()
{


	GUI.Update();
	GUI.Draw();

}

inline void CMiscTab::Setup()
{
	SetTitle("Misc");

	wpnhitchanceGroup.SetPosition(5, 1);
	wpnhitchanceGroup.SetSize(340, 205);
	wpnhitchanceGroup.SetText("Hitchance");
	RegisterControl(&wpnhitchanceGroup);

	WeaponCheck.SetFileId("weapon_chcek");
	wpnhitchanceGroup.PlaceCheckBox("Enable", this, &WeaponCheck);

	scoutChance.SetFileId("scout_chance");
	scoutChance.SetBoundaries(0, 100);
	scoutChance.SetValue(65);
	wpnhitchanceGroup.PlaceOtherControl("Scout", this, &scoutChance);

	AWPChance.SetFileId("awp_chance");
	AWPChance.SetBoundaries(0, 100);
	AWPChance.SetValue(75);
	wpnhitchanceGroup.PlaceOtherControl("AWP", this, &AWPChance);

	AutoChance.SetFileId("auto_chance");
	AutoChance.SetBoundaries(0, 100);
	AutoChance.SetValue(46);
	wpnhitchanceGroup.PlaceOtherControl("Auto", this, &AutoChance);

	RifleChance.SetFileId("rifle_chance");
	RifleChance.SetBoundaries(0, 100);
	RifleChance.SetValue(20);
	wpnhitchanceGroup.PlaceOtherControl("Rifle", this, &RifleChance);

	MPChance.SetFileId("mp_chance");
	MPChance.SetBoundaries(0, 100);
	MPChance.SetValue(4);
	wpnhitchanceGroup.PlaceOtherControl("SMG", this, &MPChance);

	PistolChance.SetFileId("pistol_chance");
	PistolChance.SetBoundaries(0, 100);
	PistolChance.SetValue(25);
	wpnhitchanceGroup.PlaceOtherControl("Pistols", this, &PistolChance);

	zeuschance.SetFileId("zeus_chance");
	zeuschance.SetBoundaries(0, 100);
	zeuschance.SetValue(40);
	wpnhitchanceGroup.PlaceOtherControl("Zeus", this, &zeuschance);



	wpnmindmgGroup.SetPosition(5, 216);
	wpnmindmgGroup.SetSize(340, 180);
	wpnmindmgGroup.SetText("Minimum Damage");
	RegisterControl(&wpnmindmgGroup);

	scoutmindmg.SetFileId("scout_mindmg");
	scoutmindmg.SetBoundaries(0, 100);
	scoutmindmg.SetValue(30);
	wpnmindmgGroup.PlaceOtherControl("Scout", this, &scoutmindmg);

	AWPmindmg.SetFileId("awp_mindmg");
	AWPmindmg.SetBoundaries(0, 100);
	AWPmindmg.SetValue(30);
	wpnmindmgGroup.PlaceOtherControl("AWP", this, &AWPmindmg);

	Automindmg.SetFileId("auto_mindmg");
	Automindmg.SetBoundaries(0, 100);
	Automindmg.SetValue(20);
	wpnmindmgGroup.PlaceOtherControl("Auto", this, &Automindmg);

	Riflemindmg.SetFileId("rifle_mindmg");
	Riflemindmg.SetBoundaries(0, 100);
	Riflemindmg.SetValue(10);
	wpnmindmgGroup.PlaceOtherControl("Rifle", this, &Riflemindmg);

	MPmindmg.SetFileId("mp_mindmg");
	MPmindmg.SetBoundaries(0, 100);
	MPmindmg.SetValue(4);
	wpnmindmgGroup.PlaceOtherControl("SMG", this, &MPmindmg);

	Pistolmindmg.SetFileId("pistol_mindmg");
	Pistolmindmg.SetBoundaries(0, 100);
	Pistolmindmg.SetValue(10);
	wpnmindmgGroup.PlaceOtherControl("Pistols", this, &Pistolmindmg);


	zeusdmg.SetFileId("zeus_mindmg");
	zeusdmg.SetBoundaries(0, 100);
	zeusdmg.SetValue(10);
	wpnmindmgGroup.PlaceOtherControl("Zeus", this, &zeusdmg);

#pragma region Other
	OtherGroup.SetPosition(350, 1);
	OtherGroup.SetSize(335, 400);
	OtherGroup.SetText("Other");
	RegisterControl(&OtherGroup);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.SetState(true);
	OtherGroup.PlaceCheckBox("Anti Untrusted", this, &OtherSafeMode);

	OtherThirdperson.SetFileId("aa_thirdpsr");
	OtherGroup.PlaceCheckBox("Thirdperson", this, &OtherThirdperson);

	ThirdpersonAngle.SetFileId("aa_thirdpersonMode");
	ThirdpersonAngle.AddItem("Real");
	ThirdpersonAngle.AddItem("Fake");
	ThirdpersonAngle.AddItem("LBY");
	OtherGroup.PlaceOtherControl("Thirdperson Angle", this, &ThirdpersonAngle);

	ThirdPersonKeyBind.SetFileId("aa_thirdpersonKey");
	OtherGroup.PlaceOtherControl("Thirdperson Key", this, &ThirdPersonKeyBind);

	AutoPistol.SetFileId("otr_autopistol");
	OtherGroup.PlaceCheckBox("Auto Pistol", this, &AutoPistol);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceCheckBox("Auto Jump", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_strafe");
	OtherAutoStrafe.AddItem("Off");
	OtherAutoStrafe.AddItem("Legit");
	OtherAutoStrafe.AddItem("Rage");
	OtherGroup.PlaceOtherControl("Auto Strafer", this, &OtherAutoStrafe);

	NameChanger.SetFileId("otr_spam");
	NameChanger.AddItem("Off");
	NameChanger.AddItem("Name Spam");
	NameChanger.AddItem("No Name");
	NameChanger.AddItem("Name Steal");
	NameChanger.AddItem("Crasher");
	OtherGroup.PlaceOtherControl("Name Changer", this, &NameChanger);

	OtherClantag.SetFileId("otr_clantag");
	OtherClantag.AddItem("Off");
	OtherClantag.AddItem("Classic");
	OtherClantag.AddItem("Rat");
	OtherClantag.AddItem("Luminus");
	OtherGroup.PlaceOtherControl("ClanTag", this, &OtherClantag);


	OtherAirStuck.SetFileId("otr_astuck");
	OtherGroup.PlaceOtherControl("Air Stuck", this, &OtherAirStuck);


	SetTitle("Settings");

	ConfigBox.SetFileId("cfg_box");
	ConfigBox.AddItem("Legit");
	ConfigBox.AddItem("Scout");
	ConfigBox.AddItem("Auto");
	ConfigBox.AddItem("Duals");
	ConfigBox.AddItem("Deagle");
	ConfigBox.AddItem("R8");
	ConfigBox.SetSize(112, 200);
	ConfigBox.SetPosition(336, 280); // +350
	RegisterControl(&ConfigBox);
	OtherGroup.PlaceLabledControl("", this, &ConfigBox);

	SaveButton.SetText("Save");
	SaveButton.SetCallback(save_callback);
	SaveButton.SetSize(112, 200);
	SaveButton.SetPosition(336, 300);
	RegisterControl(&SaveButton);
	OtherGroup.PlaceLabledControl("", this, &SaveButton);

	LoadButton.SetText("Load");
	LoadButton.SetCallback(load_callback);
	LoadButton.SetSize(112, 200);
	LoadButton.SetPosition(336, 320);
	RegisterControl(&LoadButton);
	OtherGroup.PlaceLabledControl("", this, &LoadButton);

#pragma endregion other random options
}
