#include "../../stdafx.h"
#include "SettingsManager.h"
#include <wininet.h>
#pragma comment(lib, "wininet")

double SettingsArray[20][200][100];

SettingsManger Settings;
int currentgroup = 1;
int oldgroup = -1;
int weaponconfigTab = Group_Default;

bool SettingsManger::weaponconfigs()
{
	if (Hacks.LocalWeapon->isSniper())
	{
		if (Hacks.LocalWeapon->isAWP())
		{
			currentgroup = Group_AWP;
		}
		else if (Hacks.LocalWeapon->isScout())
		{
			currentgroup = Group_Scout;
		}
		else if (Hacks.LocalWeapon->isDak())
		{
			currentgroup = Group_Othersnipers;
		}
	}
	if (Hacks.LocalWeapon->isShoot())
	{
		currentgroup = Group_Shotgun;
	}
	else if (Hacks.LocalWeapon->isRifle())
	{
		currentgroup = Group_Rifle;
	}
	if (Hacks.LocalWeapon->isPistol())
	{
		if (Hacks.LocalWeapon->isDEAGLE())
		{
			currentgroup = Group_DEAGLE;
		}
		if (Hacks.LocalWeapon->isBERETTA())
		{
			currentgroup = Group_BERETTA;
		}
		if (Hacks.LocalWeapon->isBUSPS())
		{
			currentgroup = Group_BUSPS;
		}
		if (Hacks.LocalWeapon->isCZ75())
		{
			currentgroup = Group_CZ75;
		}
		if (Hacks.LocalWeapon->isrevolver())
		{
			currentgroup = Group_revolver;
		}
	}
	if (currentgroup != oldgroup)
	{
		oldgroup = currentgroup;
		weaponconfigTab = currentgroup;
		return true;
	}
	else
	{
		return false;
	}
}

void SettingsManger::SetSetting(int Tab, int Setting, double Value)
{
	int someweaponvalue = 0;
	if (Tab == Tab_Legitbot)
	{
		if (Settings.GetSetting(Tab_Misc, Misc_WhichWeapon) == 0 || Settings.GetSetting(Tab_Misc, Misc_WhichWeapon) == 9)
		{
			if (Hacks.LocalWeapon && Hacks.LocalPlayer->isAlive())
			{
				someweaponvalue = currentgroup;
			}
			else
			{
				someweaponvalue = currentgroup;
			}
		}
		else
		{
			someweaponvalue = currentgroup;
		}
	}
	SettingsArray[Tab][Setting][someweaponvalue] = Value;
}

double SettingsManger::GetSetting(int Tab, int Setting)
{
	int someweaponvalue = 0;
	if (Tab == Tab_Legitbot)
	{
		if (Hacks.LocalWeapon && Hacks.LocalPlayer->isAlive())
		{
			someweaponvalue = currentgroup;
		}
		else
		{
			someweaponvalue = currentgroup;
		}
	}

	return SettingsArray[Tab][Setting][someweaponvalue];
}

double SettingsManger::GetSettingLegit(int Tab, int Setting)
{
	int someweaponvalue = 0;
	if (Tab == Tab_Legitbot)
	{
		if (Settings.GetSetting(Tab_Misc, Misc_WhichWeapon) == 0 || Settings.GetSetting(Tab_Misc, Misc_WhichWeapon) == 9)
		{
			if (Hacks.LocalWeapon && Hacks.LocalPlayer->isAlive())
			{
				someweaponvalue = currentgroup;
			}
			else
			{
				someweaponvalue = currentgroup;
			}
		}
		else
		{
			someweaponvalue = currentgroup;
		}
	}


	return SettingsArray[Tab][Setting][someweaponvalue];
}

double SettingsManger::GetMenuSetting(int Tab, int Setting)
{
	int someweaponvalue = 0;
	if (Tab == Tab_Legitbot)
	{
		if (Settings.GetSetting(Tab_Misc, Misc_WhichWeapon) == 0 || Settings.GetSetting(Tab_Misc, Misc_WhichWeapon) == 9)
		{
			if (Hacks.LocalWeapon && Hacks.LocalPlayer->isAlive())
			{
				someweaponvalue = currentgroup;
			}
			else
			{
				someweaponvalue = currentgroup;
			}
		}
		else
		{
			someweaponvalue = currentgroup;
		}
	}
	return SettingsArray[Tab][Setting][someweaponvalue];
}

void loadSettings(std::string settings)
{
	for (int i = 0; i < settings.size(); i++)
	{
		settings[i] = (settings[i] - 1) / 2;
	}
	// sets up settings char to a vector looks like this "0:3:2:2" ie tab-setting-value
	std::string tovector = settings;
	std::stringstream ss(tovector);
	std::istream_iterator< std::string > begin(ss);
	std::istream_iterator< std::string > end;
	std::vector< std::string > settingarray(begin, end);
	//Gets how many settings to set
	int size = settingarray.size();
	for (auto i = 0; i < size - 1; i++)
	{
		// Gets the setting we are dealing with
		string meme = settingarray[i];
		//Converst the string to char array
		for (auto i = 0; i < meme.length(); i++)
		{
			if (meme[i] == ':')
				meme[i] = ' ';
		}
		vector< double > indi;
		stringstream ss(meme);
		double temp;
		while (ss >> temp)
			indi.push_back(temp);
		//0:1:2:8
		// Sets the setting
		SettingsArray[static_cast< int >(indi[0])][static_cast< int >(indi[1])][static_cast< int >(indi[2])] = static_cast< double >(indi[3]);
	}
}

void savesets()
{
	int number = Settings.GetSetting(Tab_Misc, Config_type);
	if (CreateDirectory("C:/JesusCry", nullptr) || ERROR_ALREADY_EXISTS == GetLastError())
	{
	}
	std::string savetype = "c:/JesusCry/JesusCry_";
	savetype.append(to_string(number));
	savetype.append(".fl");

	string txt;
	ifstream file(savetype);

	if (file.is_open())
		while (file.good())
			getline(file, txt);
	file.close();
	//#ifdef DEBUG 
	//cout « txt « endl; 

	//#endif // DEBUG 

	loadSettings(txt);
}

void writefile(std::string string)
{
	int number = Settings.GetSetting(Tab_Misc, Config_type);

	if (CreateDirectory("C:/JesusCry/", nullptr) || ERROR_ALREADY_EXISTS == GetLastError())
	{
	}
	std::string savetype = "c:/JesusCry/JesusCry_";
	savetype.append(to_string(number));
	savetype.append(".fl");
	ofstream myfile;
	myfile.open(savetype);
	myfile << string;
	myfile.close();
	//cout « 1 « endl; 
	//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)upload, 0, 0, 0); 
	//cout « 2 « endl; 
}

std::string settingstostring()
{
	std::string stra;
	int val = MAXVALUEMENU;

	for (int i = 0; i < val; i++)
	{
		int mena = 0;
		switch (i)
		{
		case 0:
			mena = MAXVALUELEGIT;
			break;
		case 1:
			mena = MAXVALUERAGE;
			break;
		case 2:
			mena = MAXVALUEVISUALS;
			break;
		case 3:
			mena = MAXVALUEMISC;
			break;
		case 4:
			mena = MAXVALUECOLORS;
			break;
		case 5:
			mena = MAXVALUESKINS;
			break;


		}
		for (int a = 0; a < mena; a++)
		{
			double settingval;
			if (i == 0)
			{
				for (int c = 0; c < 80; c++)
				{
					stra.append(to_string(i));
					stra.append(":");
					stra.append(to_string(a));
					stra.append(":");
					settingval = SettingsArray[i][a][c];
					stra.append(to_string(c));
					stra.append(":");
					stra.append(to_string(settingval));
					stra.append(" ");
					stra.append("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
				}
			}
			else
			{
				stra.append(to_string(i));
				stra.append(":");
				stra.append(to_string(a));
				stra.append(":");
				settingval = SettingsArray[i][a][0];
				stra.append(to_string(0));
				stra.append(":");
				stra.append(to_string(settingval));
				stra.append(" ");
				stra.append("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
			}
		}
	}
	//#ifdef DEBUG
	//cout << "Saving" << endl;

	for (int i = 0; i < stra.size(); i++)
	{
		stra[i] = stra[i] * 2 + 1;
	}

	//cout << stra << endl;
	//#endif
	writefile(stra);

	return stra;
}
void junk12()
{
	float dLSmfDWTAi = 2039656837874; dLSmfDWTAi = 66602585424300; if (dLSmfDWTAi = 95026813754581) dLSmfDWTAi = 71500606760902; dLSmfDWTAi = 54502663622768; dLSmfDWTAi = 36227685450266;
	if (dLSmfDWTAi = 5644387150060)dLSmfDWTAi = 37545813105476; dLSmfDWTAi = 43108647949178;
	if (dLSmfDWTAi = 10570629039930)dLSmfDWTAi = 37545813105476; dLSmfDWTAi = 43108647949178;
	if (dLSmfDWTAi = 10570629039930)dLSmfDWTAi = 37545813105476; dLSmfDWTAi = 43108647949178;
	if (dLSmfDWTAi = 10570629039930)dLSmfDWTAi = 37545813105476; dLSmfDWTAi = 43108647949178;
	if (dLSmfDWTAi = 10570629039930)dLSmfDWTAi = 37545813105476; dLSmfDWTAi = 43108647949178; dLSmfDWTAi = 52972187895;
}