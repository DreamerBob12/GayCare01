/*
Rest In Peace ApocalypseCheats
*/

#include "GUI.h"
#include "Menu.h"
#include "RenderManager.h"
#include "MetaInfo.h"

#include <algorithm>
#include "tinyxml2.h"
#include "Controls.h"
#include "GUI.h"

CGUI GUI;
float MenuAlpha = 0.f;

float Globals::MenuAlpha2 = MenuAlpha;

CGUI::CGUI()
{

}

bool CGUI::GetKeyPress(unsigned int key)
{
	if (keys[key] == true && oldKeys[key] == false)
		return true;
	else
		return false;
}

bool CGUI::GetKeyState(unsigned int key)
{
	return keys[key];
}

bool CGUI::IsMouseInRegion(int y, int x2, int y2, int x)
{
	if (Mouse.x > x && Mouse.y > y && Mouse.x < x2 && Mouse.y < y2)
		return true;
	else
		return false;
}

bool CGUI::IsMouseInRegion(RECT region)
{
	return IsMouseInRegion(region.top, region.left + region.right, region.top + region.bottom, region.left);
}

POINT CGUI::GetMouse()
{
	return Mouse;
}

void CGUI::Draw()
{
	bool ShouldDrawCursor = false;

	for (auto window : Windows)
	{
		if (window->m_bIsOpen)
		{
			ShouldDrawCursor = true;
			DrawWindow(window);
		}

	}

	


	if (ShouldDrawCursor)
	{
		Render::Clear(Mouse.x + 1, Mouse.y, 1, 17, Color(3, 6, 26, 255));
		for (int i = 0; i < 11; i++)
			Render::Clear(Mouse.x + 2 + i, Mouse.y + 1 + i, 1, 1, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 12, 5, 1, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 13, 1, 1, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 9, Mouse.y + 14, 1, 2, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 10, Mouse.y + 16, 1, 2, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 18, 2, 1, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 7, Mouse.y + 16, 1, 2, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 6, Mouse.y + 14, 1, 2, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 5, Mouse.y + 13, 1, 1, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 4, Mouse.y + 14, 1, 1, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 3, Mouse.y + 15, 1, 1, Color(3, 6, 26, 255));
		Render::Clear(Mouse.x + 2, Mouse.y + 16, 1, 1, Color(3, 6, 26, 255));
		for (int i = 0; i < 4; i++)
			Render::Clear(Mouse.x + 2 + i, Mouse.y + 2 + i, 1, 14 - (i * 2), Color(0 - (i * 4), 170 - (i * 4), 255 - (i * 4), 255));
		Render::Clear(Mouse.x + 6, Mouse.y + 6, 1, 8, Color(0, 170, 255, 255));
		Render::Clear(Mouse.x + 7, Mouse.y + 7, 1, 9, Color(0, 170, 255, 255));
		for (int i = 0; i < 4; i++)
			Render::Clear(Mouse.x + 8 + i, Mouse.y + 8 + i, 1, 4 - i, Color(5 - (i * 4), 170 - (i * 4), 255 - (i * 4), 255));
		Render::Clear(Mouse.x + 8, Mouse.y + 14, 1, 4, Color(0, 170, 255, 255));
		Render::Clear(Mouse.x + 9, Mouse.y + 16, 1, 2, Color(0, 170, 255, 255));
	}
}
// Handle all input etc
void CGUI::Update()
{

	static int bWasntHolding = false;
	static int bGrabbing = false;
	static int iOffsetX = 0, iOffsetY = 0;
	//Key Array
	std::copy(keys, keys + 255, oldKeys);
	for (int x = 0; x < 255; x++)
	{
		//oldKeys[x] = oldKeys[x] & keys[x];
		keys[x] = (GetAsyncKeyState(x));
	}

	POINT mp; GetCursorPos(&mp);
	ScreenToClient(GetForegroundWindow(), &mp);
	Mouse.x = mp.x; Mouse.y = mp.y;

	RECT Screen = Render::GetViewport();

	// Window Binds
	for (auto& bind : WindowBinds)
	{
		if (GetKeyPress(bind.first))
		{
			bind.second->Toggle();
		}
	}

	// Stop dragging
	if (IsDraggingWindow && !GetKeyState(VK_LBUTTON))
	{
		IsDraggingWindow = false;
		DraggingWindow = nullptr;

	}

	// If we are in the proccess of dragging a window
	if (IsDraggingWindow && GetKeyState(VK_LBUTTON) && !GetKeyPress(VK_LBUTTON))
	{
		if (DraggingWindow)
		{
			DraggingWindow->m_x = Mouse.x - DragOffsetX;
			DraggingWindow->m_y = Mouse.y - DragOffsetY;
		}

	}


	//bWasntHolding = Input->Hovering(x, y, width, 28) && !GetAsyncKeyState(VK_LBUTTON);

	// Process some windows
	for (auto window : Windows)
	{
		if (window->m_bIsOpen)
		{
			// Used to tell the widget processing that there could be a click
			bool bCheckWidgetClicks = false;

			// If the user clicks inside the window
			if (GetKeyPress(VK_LBUTTON) || GetKeyPress(VK_RETURN))
			{
				//if (IsMouseInRegion(window->m_x, window->m_y, window->m_x + window->m_iWidth, window->m_y + window->m_iHeight))
				//{
				// Is it inside the client area?
				if (IsMouseInRegion(window->GetClientArea()))
				{
					// User is selecting a new tab
					if (IsMouseInRegion(window->GetTabArea()))
					{
						// Loose focus on the control
						window->IsFocusingControl = false;
						window->FocusedControl = nullptr;

						int iTab = 0;
						int TabCount = window->Tabs.size();
						if (TabCount) // If there are some tabs
						{
							int TabSize = (window->m_iWidth - 4 - 12) / TabCount;
							int Dist = Mouse.x - (window->m_x + 8);
							while (Dist > TabSize)
							{
								if (Dist > TabSize)
								{
									iTab++;
									Dist -= TabSize;
								}
							}
							window->SelectedTab = window->Tabs[iTab];
						}

					}
					else
						bCheckWidgetClicks = true;
				}
				else if (IsMouseInRegion(window->m_x, window->m_y, window->m_x + window->m_iWidth, window->m_y + window->m_iHeight))
				{
					// Must be in the around the title or side of the window
					// So we assume the user is trying to drag the window
					IsDraggingWindow = true;

					DraggingWindow = window;

					DragOffsetX = Mouse.x - window->m_x;
					DragOffsetY = Mouse.y - window->m_y;

					// Loose focus on the control
					window->IsFocusingControl = false;
					window->FocusedControl = nullptr;
				}

				//else
				//{
				//    // Loose focus on the control
				//    window->IsFocusingControl = false;
				//    window->FocusedControl = nullptr;
				//}
			}


			// Controls
			if (window->SelectedTab != nullptr)
			{
				// Focused widget
				bool SkipWidget = false;
				CControl* SkipMe = nullptr;

				// this window is focusing on a widget??
				if (window->IsFocusingControl)
				{
					if (window->FocusedControl != nullptr)
					{
						// We've processed it once, skip it later
						SkipWidget = true;
						SkipMe = window->FocusedControl;

						POINT cAbs = window->FocusedControl->GetAbsolutePos();
						RECT controlRect = { cAbs.x, cAbs.y, window->FocusedControl->m_iWidth, window->FocusedControl->m_iHeight };
						window->FocusedControl->OnUpdate();

						if (window->FocusedControl->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
						{
							window->FocusedControl->OnClick();

							// If it gets clicked we loose focus
							window->IsFocusingControl = false;
							window->FocusedControl = nullptr;
							bCheckWidgetClicks = false;
						}
					}
				}

				// Itterate over the rest of the control
				for (auto control : window->SelectedTab->Controls)
				{
					if (control != nullptr)
					{
						if (SkipWidget && SkipMe == control)
							continue;

						POINT cAbs = control->GetAbsolutePos();
						RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
						control->OnUpdate();

						if (control->Flag(UIFlags::UI_Clickable) && IsMouseInRegion(controlRect) && bCheckWidgetClicks)
						{
							control->OnClick();
							bCheckWidgetClicks = false;

							// Change of focus
							if (control->Flag(UIFlags::UI_Focusable))
							{
								window->IsFocusingControl = true;
								window->FocusedControl = control;
							}
							else
							{
								window->IsFocusingControl = false;
								window->FocusedControl = nullptr;
							}

						}
					}
				}

				// We must have clicked whitespace
				if (bCheckWidgetClicks)
				{
					// Loose focus on the control
					window->IsFocusingControl = false;
					window->FocusedControl = nullptr;
				}
			}
		}
	}
}


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class djkofwn {
public:
	double xuldvijsym;
	djkofwn();
	double dtvzapcpipyihlbfbmqvwpuk(double xmmdihecefeik);
	int fvsplxbyaqxp(int dibkc, bool ueusgchgs, bool pxxqwmljid, double jymvrysweqitt, string nxtxxmaaopdfr, bool tpgwlzzwygho, double fjqvmgrzf, double xtjizxaapvfb);
	double cyafjmqgjyty(bool rkigmhvkzj, bool nvcensovwuin, bool tiafilsutyeiswp, int naldexoxpkssvu, double frmxhqieqh, double meboxjjpkvckl, double fvqqzjtppfj, string fcorebobhs, bool usqmegomwoytf);
	void vwoxhbeblfkkmaokr(int sknmttfefi, int dwxkjnfxg, bool zfksjgyvjeqiih, int hznarbtyfqbuln);
	int bvxoleowaecwubcllucl(double nytyyrrlykts, bool jquxmuw, string hazukcfhytfm);
	int vnnvujmujfi(bool oeetpz);

protected:
	int lnbpqzisyxknuj;
	bool hcjvjfmirgodft;
	double nrqpgnnhtylx;
	bool piujcdke;
	int bkbchzoraajg;

	void agfnkdnxwqbxi();
	int gfxhbkbndrk(string ntgxq, int mywridv, string bzstvsm, int hgwxpgiqqzdqwu, double dozoajvxymi, string xjufbzbgwivlmq, double tlzcgnbmtv, string tlenfo, int ksklim);
	string xqcqmyrhdzanwcxsvgznjira(bool hnnskvxehnyibo, string fgdmsbmwnbrzh, bool gvzzltx, string hdqvdqbcqcbiaxs, bool piqzkry, int vwprgrsqptfid, double mwvnrzegoc, bool sbsojndygwrvs);
	bool vrdoruswaesc(int bhuqwyf, string chbfrvyypmhkib, int ogqpgsmnhrcarc, double pyzvbwmu, double erihqz, int qmebrlxkcha, double kvklgbowt, string wzpipz, double wrogsukmdhukxpx, bool pwkwve);
	void bqtihehtbfqeng(int dtiotyuibdb, double bqixfgeb, string ouzddwxeawyohj, string jhqrmfeaxoztfcp, int vpxdxtjyjt, int cjjsluyksjaiocd, int pkxyoibfqfkhd, bool mxptuubwgmpdwr, int ehorks);
	double sbsgadkifpuwfpqufgxy(string nunnepvaz, double filgcrhih, double bhaeyygytryhb, double lqpylqedi, double vjkmnuliexa, bool fsipvfhmnljvw, double bnrncqwuapst);
	bool imdpifjxbl(string yuklefnpzrys);
	bool wbfhsizaatstxnfimxiszavyf(string hedgneouuix, string gnnmnqmuen, int gwkekt, bool lopcb);
	bool jvdnbabiqggckapjzox(string pytkcao, string jodteq);
	double egljgplbyaphfnsjube(bool hmyatcd, string anwzbzippm);

private:
	bool jzblpbi;

	bool hxrtevaljschclvrz();
	bool jcsbjjrkjlzrywocakuagyr(bool varanmexthfr, int aqxwd, bool xjprckbwxobx, string xygczkrn);

};



bool djkofwn::hxrtevaljschclvrz() {
	int knwutwuecxy = 7382;
	int xytgysk = 5248;
	string bpczjs = "gbiogcumumnrlnxblunxdghourghurkywpibgoohucyjldaotmfqrbwbxprwfpdtciwbpbtiuptz";
	string hixsth = "wcatalvjniczqdyszhhppwsmpzvkaqmxturmubjdgayksrlxpyqopxbzkpgiosaexasghubknptsixvuldnxwgvntxsyyk";
	string djnbti = "oetlkkeryjcdmxuycsdrwluktqguflupcufcfklmfnxoxczgbjwmlp";
	if (7382 != 7382) {
		int dv;
		for (dv = 18; dv > 0; dv--) {
			continue;
		}
	}
	return true;
}

bool djkofwn::jcsbjjrkjlzrywocakuagyr(bool varanmexthfr, int aqxwd, bool xjprckbwxobx, string xygczkrn) {
	string accbhnirru = "kkzhsmbrrsfaushnofpzztzvjphnpfafkhmolhjzaycxasytibaqzkojyygdzwpgvisadzgxzbppnpenpggzgs";
	double khrzjanrdoea = 57885;
	double iqjayqhlcjeu = 51674;
	string khacahra = "xcghijnhsrwtbbsajqxyuqtzjs";
	string swcrsi = "r";
	double oyflymjyikwvmea = 1197;
	string bksgehwnficrdl = "onsahxcrscyhzpvmgciscihvqjarebhjglx";
	if (string("kkzhsmbrrsfaushnofpzztzvjphnpfafkhmolhjzaycxasytibaqzkojyygdzwpgvisadzgxzbppnpenpggzgs") == string("kkzhsmbrrsfaushnofpzztzvjphnpfafkhmolhjzaycxasytibaqzkojyygdzwpgvisadzgxzbppnpenpggzgs")) {
		int toelhfwk;
		for (toelhfwk = 8; toelhfwk > 0; toelhfwk--) {
			continue;
		}
	}
	if (string("onsahxcrscyhzpvmgciscihvqjarebhjglx") != string("onsahxcrscyhzpvmgciscihvqjarebhjglx")) {
		int nxyq;
		for (nxyq = 9; nxyq > 0; nxyq--) {
			continue;
		}
	}
	return false;
}

void djkofwn::agfnkdnxwqbxi() {
	double ftpqvdt = 90504;
	double tdssnkeamljaoy = 31053;
	double elmnpfx = 5812;
	int tevavwjpq = 7309;
	double nfaezldvbn = 26971;
	if (5812 == 5812) {
		int goqh;
		for (goqh = 99; goqh > 0; goqh--) {
			continue;
		}
	}

}

int djkofwn::gfxhbkbndrk(string ntgxq, int mywridv, string bzstvsm, int hgwxpgiqqzdqwu, double dozoajvxymi, string xjufbzbgwivlmq, double tlzcgnbmtv, string tlenfo, int ksklim) {
	int lgfjig = 73;
	if (73 == 73) {
		int kfvody;
		for (kfvody = 65; kfvody > 0; kfvody--) {
			continue;
		}
	}
	if (73 == 73) {
		int oapmexhiy;
		for (oapmexhiy = 39; oapmexhiy > 0; oapmexhiy--) {
			continue;
		}
	}
	if (73 == 73) {
		int jcmptihdad;
		for (jcmptihdad = 10; jcmptihdad > 0; jcmptihdad--) {
			continue;
		}
	}
	if (73 != 73) {
		int dxylsup;
		for (dxylsup = 39; dxylsup > 0; dxylsup--) {
			continue;
		}
	}
	return 82667;
}

string djkofwn::xqcqmyrhdzanwcxsvgznjira(bool hnnskvxehnyibo, string fgdmsbmwnbrzh, bool gvzzltx, string hdqvdqbcqcbiaxs, bool piqzkry, int vwprgrsqptfid, double mwvnrzegoc, bool sbsojndygwrvs) {
	string qiacicttdon = "rmozerokowktemzcpvqwqnbgsotetincchaqecvoavhrsixluzeoe";
	double qjnzfxgpc = 12181;
	bool bwzfff = true;
	string yxwqx = "mgdbyxwcqfnvacontfkwoquqhameysipjjmwsffuuhbyeryyew";
	if (string("mgdbyxwcqfnvacontfkwoquqhameysipjjmwsffuuhbyeryyew") == string("mgdbyxwcqfnvacontfkwoquqhameysipjjmwsffuuhbyeryyew")) {
		int bvtwfuyqg;
		for (bvtwfuyqg = 85; bvtwfuyqg > 0; bvtwfuyqg--) {
			continue;
		}
	}
	if (string("rmozerokowktemzcpvqwqnbgsotetincchaqecvoavhrsixluzeoe") == string("rmozerokowktemzcpvqwqnbgsotetincchaqecvoavhrsixluzeoe")) {
		int glgjqeyii;
		for (glgjqeyii = 84; glgjqeyii > 0; glgjqeyii--) {
			continue;
		}
	}
	return string("dh");
}

bool djkofwn::vrdoruswaesc(int bhuqwyf, string chbfrvyypmhkib, int ogqpgsmnhrcarc, double pyzvbwmu, double erihqz, int qmebrlxkcha, double kvklgbowt, string wzpipz, double wrogsukmdhukxpx, bool pwkwve) {
	string uuxvgawkflkm = "uhcocivltfavrsaiuuezhuwtkwdtmfbx";
	int lwhwtuadpfgu = 1037;
	int lmspfcxngd = 3394;
	bool ftbuocv = false;
	bool ymqcabptlgfb = true;
	double baaulgcpgdreisd = 8213;
	int doozajusppx = 6563;
	double nrrhukvvnmvqmml = 2102;
	bool nywsaqok = true;
	double testkuugcgbmi = 9834;
	if (8213 == 8213) {
		int dp;
		for (dp = 6; dp > 0; dp--) {
			continue;
		}
	}
	if (true == true) {
		int tggun;
		for (tggun = 38; tggun > 0; tggun--) {
			continue;
		}
	}
	if (true == true) {
		int znimdgbyit;
		for (znimdgbyit = 6; znimdgbyit > 0; znimdgbyit--) {
			continue;
		}
	}
	return true;
}

void djkofwn::bqtihehtbfqeng(int dtiotyuibdb, double bqixfgeb, string ouzddwxeawyohj, string jhqrmfeaxoztfcp, int vpxdxtjyjt, int cjjsluyksjaiocd, int pkxyoibfqfkhd, bool mxptuubwgmpdwr, int ehorks) {
	string kclbxxopnnax = "eh";
	string sasoqunxg = "yurbketineyvhxwtmltingvypojlliwpgsvvklufgmramjowxwthdbvudcdzrmpsoslspcga";
	double jkymveslpfvo = 6645;
	bool gmirdbvlpncrzr = false;

}

double djkofwn::sbsgadkifpuwfpqufgxy(string nunnepvaz, double filgcrhih, double bhaeyygytryhb, double lqpylqedi, double vjkmnuliexa, bool fsipvfhmnljvw, double bnrncqwuapst) {
	bool kukahsqydm = false;
	bool uuxyws = false;
	bool zwaqpqjc = true;
	if (false == false) {
		int oqdtongd;
		for (oqdtongd = 26; oqdtongd > 0; oqdtongd--) {
			continue;
		}
	}
	return 15357;
}

bool djkofwn::imdpifjxbl(string yuklefnpzrys) {
	return false;
}

bool djkofwn::wbfhsizaatstxnfimxiszavyf(string hedgneouuix, string gnnmnqmuen, int gwkekt, bool lopcb) {
	int vfohcv = 2220;
	if (2220 == 2220) {
		int xlnuarnjgx;
		for (xlnuarnjgx = 66; xlnuarnjgx > 0; xlnuarnjgx--) {
			continue;
		}
	}
	if (2220 != 2220) {
		int jdyoe;
		for (jdyoe = 89; jdyoe > 0; jdyoe--) {
			continue;
		}
	}
	if (2220 != 2220) {
		int yaupt;
		for (yaupt = 4; yaupt > 0; yaupt--) {
			continue;
		}
	}
	if (2220 != 2220) {
		int ghpq;
		for (ghpq = 68; ghpq > 0; ghpq--) {
			continue;
		}
	}
	return true;
}

bool djkofwn::jvdnbabiqggckapjzox(string pytkcao, string jodteq) {
	string ichjkrh = "ygxvuyxthuedgudmoukxurjsqwqfldmihumdqbgnjbiasmvrqpqtchzlrnnxwqsctuxqlfcmbpaudv";
	double zydasfwstlxj = 49860;
	bool ylittsnlz = true;
	bool gfnlsmhez = true;
	double srghyntnbwe = 75760;
	string rqbko = "avrvelzpczwmlkbfkntzllgecsjvvqhufqgmidckmfzzxgtrkqdaffpnwaujob";
	if (true == true) {
		int tfhedy;
		for (tfhedy = 28; tfhedy > 0; tfhedy--) {
			continue;
		}
	}
	if (string("avrvelzpczwmlkbfkntzllgecsjvvqhufqgmidckmfzzxgtrkqdaffpnwaujob") != string("avrvelzpczwmlkbfkntzllgecsjvvqhufqgmidckmfzzxgtrkqdaffpnwaujob")) {
		int kblouxyb;
		for (kblouxyb = 1; kblouxyb > 0; kblouxyb--) {
			continue;
		}
	}
	if (string("ygxvuyxthuedgudmoukxurjsqwqfldmihumdqbgnjbiasmvrqpqtchzlrnnxwqsctuxqlfcmbpaudv") == string("ygxvuyxthuedgudmoukxurjsqwqfldmihumdqbgnjbiasmvrqpqtchzlrnnxwqsctuxqlfcmbpaudv")) {
		int ite;
		for (ite = 79; ite > 0; ite--) {
			continue;
		}
	}
	return false;
}

double djkofwn::egljgplbyaphfnsjube(bool hmyatcd, string anwzbzippm) {
	string gdjobp = "hdyfbourbfflxmeaomvuivtvmpcidiemdiwsndimivtiaovqscjeqahwfqqipgwkdfweklrqhccqvytquuwefsvoxmplfag";
	int upprqdivgv = 609;
	bool pkuuzjexfcexl = false;
	if (false != false) {
		int lxhcml;
		for (lxhcml = 82; lxhcml > 0; lxhcml--) {
			continue;
		}
	}
	if (false != false) {
		int fbooqunznj;
		for (fbooqunznj = 29; fbooqunznj > 0; fbooqunznj--) {
			continue;
		}
	}
	if (false != false) {
		int brwth;
		for (brwth = 28; brwth > 0; brwth--) {
			continue;
		}
	}
	if (609 == 609) {
		int dd;
		for (dd = 41; dd > 0; dd--) {
			continue;
		}
	}
	if (false != false) {
		int iihqrdkoyu;
		for (iihqrdkoyu = 89; iihqrdkoyu > 0; iihqrdkoyu--) {
			continue;
		}
	}
	return 48552;
}

double djkofwn::dtvzapcpipyihlbfbmqvwpuk(double xmmdihecefeik) {
	double jcuolaci = 24220;
	int nkzdnjf = 1830;
	double zqarqqlh = 61568;
	double ckqktrld = 5480;
	if (1830 != 1830) {
		int hmr;
		for (hmr = 53; hmr > 0; hmr--) {
			continue;
		}
	}
	if (24220 == 24220) {
		int xbgu;
		for (xbgu = 51; xbgu > 0; xbgu--) {
			continue;
		}
	}
	if (1830 != 1830) {
		int hlctjmljd;
		for (hlctjmljd = 65; hlctjmljd > 0; hlctjmljd--) {
			continue;
		}
	}
	return 49245;
}

int djkofwn::fvsplxbyaqxp(int dibkc, bool ueusgchgs, bool pxxqwmljid, double jymvrysweqitt, string nxtxxmaaopdfr, bool tpgwlzzwygho, double fjqvmgrzf, double xtjizxaapvfb) {
	int fzapadud = 7282;
	bool doxovf = true;
	double nnfejrclxkgx = 25786;
	double jtynat = 5645;
	string kdpehn = "rhaczfrdpcvvairbgurhikmaakzpxlpweeregvdntrdgyjz";
	bool uhowonevrtyvi = true;
	string oymgzkjdrdl = "lv";
	string nhftdhwds = "etgolblxhgtfveoubqzunrpc";
	int gklav = 1608;
	if (5645 == 5645) {
		int wezui;
		for (wezui = 22; wezui > 0; wezui--) {
			continue;
		}
	}
	if (string("rhaczfrdpcvvairbgurhikmaakzpxlpweeregvdntrdgyjz") == string("rhaczfrdpcvvairbgurhikmaakzpxlpweeregvdntrdgyjz")) {
		int zybgdxbxqs;
		for (zybgdxbxqs = 40; zybgdxbxqs > 0; zybgdxbxqs--) {
			continue;
		}
	}
	if (1608 != 1608) {
		int fe;
		for (fe = 49; fe > 0; fe--) {
			continue;
		}
	}
	if (true != true) {
		int zpcve;
		for (zpcve = 25; zpcve > 0; zpcve--) {
			continue;
		}
	}
	return 12908;
}

double djkofwn::cyafjmqgjyty(bool rkigmhvkzj, bool nvcensovwuin, bool tiafilsutyeiswp, int naldexoxpkssvu, double frmxhqieqh, double meboxjjpkvckl, double fvqqzjtppfj, string fcorebobhs, bool usqmegomwoytf) {
	string yvuxakho = "wkzventmcckiwudthcje";
	string nviunt = "xqppxfzifagggjwtwqpdxwghyoejbbhbyxcydhhpzgupucvohixenwfbhskfvaxerhrxvdevprypcvevqyfmnx";
	double jvfbjjomxh = 11228;
	string rnacvuqdlegxyc = "uaelblgqrgjhdryvjpfsyilycqxsecs";
	int rhvecl = 7212;
	int figgzatgrttrfip = 243;
	double vsyqwaxhkeifmg = 5823;
	int ntzjjabufheneh = 1300;
	int astqvwdr = 686;
	if (11228 != 11228) {
		int whagkvyrv;
		for (whagkvyrv = 29; whagkvyrv > 0; whagkvyrv--) {
			continue;
		}
	}
	if (5823 == 5823) {
		int nyxrcd;
		for (nyxrcd = 27; nyxrcd > 0; nyxrcd--) {
			continue;
		}
	}
	if (string("uaelblgqrgjhdryvjpfsyilycqxsecs") == string("uaelblgqrgjhdryvjpfsyilycqxsecs")) {
		int oarlhhn;
		for (oarlhhn = 29; oarlhhn > 0; oarlhhn--) {
			continue;
		}
	}
	if (686 != 686) {
		int tetaccuqq;
		for (tetaccuqq = 87; tetaccuqq > 0; tetaccuqq--) {
			continue;
		}
	}
	if (1300 == 1300) {
		int zyv;
		for (zyv = 85; zyv > 0; zyv--) {
			continue;
		}
	}
	return 29131;
}

void djkofwn::vwoxhbeblfkkmaokr(int sknmttfefi, int dwxkjnfxg, bool zfksjgyvjeqiih, int hznarbtyfqbuln) {
	bool yurfjyejkoraxyl = false;
	int obnqezcvrggtux = 1487;
	int urinjeuwyx = 1579;
	string nafecmdk = "xfnvahxywwovdjxmrovjtqubzkmbfvpwtwwqqgzmdoqxwlerkryehjmtwxfwtynmnedhbpfuggsqzcrfjmqsxa";
	int pzsib = 1354;
	double yaajfepy = 12067;
	string lqizpzcnsqhvkwm = "krbk";
	if (1487 != 1487) {
		int zjht;
		for (zjht = 90; zjht > 0; zjht--) {
			continue;
		}
	}

}

int djkofwn::bvxoleowaecwubcllucl(double nytyyrrlykts, bool jquxmuw, string hazukcfhytfm) {
	bool ayrwlmuzszducin = false;
	if (false == false) {
		int xe;
		for (xe = 55; xe > 0; xe--) {
			continue;
		}
	}
	if (false != false) {
		int kuhlbt;
		for (kuhlbt = 14; kuhlbt > 0; kuhlbt--) {
			continue;
		}
	}
	if (false != false) {
		int egqgmr;
		for (egqgmr = 28; egqgmr > 0; egqgmr--) {
			continue;
		}
	}
	return 2864;
}

int djkofwn::vnnvujmujfi(bool oeetpz) {
	bool wggzrzih = true;
	bool ezvuacb = false;
	double zpwohyb = 36723;
	double bgmywproanf = 4116;
	int sjstlqtpft = 3312;
	int rbtsjkhrmqzn = 5527;
	if (5527 != 5527) {
		int hbl;
		for (hbl = 28; hbl > 0; hbl--) {
			continue;
		}
	}
	if (36723 == 36723) {
		int qxeep;
		for (qxeep = 28; qxeep > 0; qxeep--) {
			continue;
		}
	}
	if (3312 != 3312) {
		int hwdx;
		for (hwdx = 71; hwdx > 0; hwdx--) {
			continue;
		}
	}
	if (true != true) {
		int rebz;
		for (rebz = 21; rebz > 0; rebz--) {
			continue;
		}
	}
	return 62652;
}

djkofwn::djkofwn() {
	this->dtvzapcpipyihlbfbmqvwpuk(23527);
	this->fvsplxbyaqxp(3397, false, true, 16275, string("qnepmbxguvsxgorjzebqagsqnmqaarnoofq"), false, 15596, 22212);
	this->cyafjmqgjyty(true, false, true, 693, 33655, 57251, 21172, string("dkpzzlydpqtjoggyqzacemhynffakudsepiikvqeqd"), false);
	this->vwoxhbeblfkkmaokr(2481, 6509, false, 8883);
	this->bvxoleowaecwubcllucl(13533, false, string("igzyfvjefsdheoeuqqwdjjuemtczkildlndosulfpsevxebmibdbpcagjsomyeqniljcvbizxxhhdydqh"));
	this->vnnvujmujfi(true);
	this->agfnkdnxwqbxi();
	this->gfxhbkbndrk(string("jzktnlblcsixdtowemvdjaiyssvbszbmeenoobgxyogkaiwjivprdedz"), 1139, string("wsupwbbezrqeganmfgintvewcfukjomwltuxahvgawcghdneegipowftqqjbrhcrxzebuvotwcaynzuiabeqdcuveyg"), 1522, 70, string("byyapvtrhpxqnrgmwehmkxgwwkgwsfqkcynsndlbzpxyrgyvbhclmzebpcmgaappnhrimspqaxdhqz"), 27159, string("nbrppjiugtauhzackfetbpjftnbmabjalpkzwzooaclsusltwymsinrvmertnkztlhmnr"), 6925);
	this->xqcqmyrhdzanwcxsvgznjira(false, string("gatktfjpuujpurgyhyaxdwyktnvycyqqpgpycmgkhlhnvouvipqsmklhqlrzfzkemlpezcziszfdlqwvrecgnojvkqmwnppe"), false, string("djqrgwunblybipqxjncgtsqlwpdoiqpdzudegunlqkezwezkawrl"), false, 1852, 13887, true);
	this->vrdoruswaesc(420, string("riynqdzjkfwhbrhueqmknqepo"), 2957, 23881, 32128, 3859, 59550, string("ozfamtfnrdylsxrwyrwtukrzctujpplweel"), 81918, true);
	this->bqtihehtbfqeng(1221, 9035, string("rdidxugxarrlu"), string("wdzaxspwozgkbdomnutaubngysijjuloqymbcuttrtpjsvbjuxgvqjnegxetycyimrfhmkvoishmizcbxztobjdgq"), 3257, 1447, 141, true, 1249);
	this->sbsgadkifpuwfpqufgxy(string("jftgfppvffmuhzvbfixyvxkmcvlxepdbzakmzldcqnudjcesvsovoxrhnkyjiposvddk"), 24814, 3504, 14087, 36437, false, 8304);
	this->imdpifjxbl(string("bcpil"));
	this->wbfhsizaatstxnfimxiszavyf(string("depmdgcgbfwadanwdzcjhxxrowgrhpkfhxhvzebbcjinkzwsrwjzetk"), string("weymtf"), 283, true);
	this->jvdnbabiqggckapjzox(string("oxegpukxhixzosscjksdqlvbxlleysdrozpulisgvxcwok"), string("jqmngfojfrcaftytqsjhcxmdtxftfzcynoimmwfazjpcxwzxgrbjzcwwszdnd"));
	this->egljgplbyaphfnsjube(false, string("vpjoriuftqplyooaonfrpx"));
	this->hxrtevaljschclvrz();
	this->jcsbjjrkjlzrywocakuagyr(false, 3921, false, string("qshpflauqvyiztyxrasrtiirpvyvhhlbfxjsvkuwxxohiwgapgnjokorbaxhfczb"));
}




bool CGUI::DrawWindow(CWindow* window)
{
	float titler = Menu::Window.colourtab.titler.GetValue(); // default: 10
	float titleg = Menu::Window.colourtab.titleg.GetValue(); // default: 10
	float titleb = Menu::Window.colourtab.titleb.GetValue(); // default: 10

	float mAlpha = Menu::Window.colourtab.alpMenu.GetValue(); // default: 255


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

	int w, h;
	int centerW, centerh, topH;
	Interfaces::Engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;

	
	Render::Clear(window->m_x, window->m_y, window->m_iWidth, window->m_iHeight, Color(11, 11, 11, mAlpha));




	//rainbow bar


	Render::Clear(0, 0, 2000, 1900, Color(10, 10, 10, 190));


	Render::GradientH(window->m_x + 7, window->m_y + 7, (window->m_iWidth - 14) / 2, 2, Color(grad1r * (255 / 255.f), grad1g * (255 / 255.f), grad1b * (255 / 255.f), 255), Color(grad2r * (255 / 255.f), grad2g * (255 / 255.f), grad2b * (255 / 255.f), 255));
	Render::GradientH(window->m_x + 7 + (window->m_iWidth - 14) / 2, window->m_y + 7, (window->m_iWidth - 14) / 2, 2, Color(grad3r * (255 / 255.f), grad3g * (255 / 255.f), grad3b * (255 / 255.f), 255), Color(grad4r * (255 / 255.f), grad4g * (255 / 255.f), grad4b * (255 / 255.f), 255));
	Render::Clear(window->m_x + 7, window->m_y + 8, (window->m_iWidth - 14), 2, Color(0 * (255 / 255.f), 0 * (255 / 255.f), 0 * (255 / 255.f), 200 * (255 / 255.f)));

	Render::Outline(window->m_x, window->m_y, window->m_iWidth, window->m_iHeight, Color(0, 0, 0, 255));
	Render::Outline(window->m_x + 1, window->m_y + 1, window->m_iWidth - 2, window->m_iHeight - 2, Color(20, 20, 20, 255));
	Render::Outline(window->m_x + 2, window->m_y + 2, window->m_iWidth - 4, window->m_iHeight - 4, Color(20, 20, 20, 255));
	Render::Outline(window->m_x + 3, window->m_y + 3, window->m_iWidth - 6, window->m_iHeight - 6, Color(20, 20, 20, 255));
	Render::Outline(window->m_x + 4, window->m_y + 4, window->m_iWidth - 8, window->m_iHeight - 8, Color(20, 20, 20, 255));
	Render::Outline(window->m_x + 5, window->m_y + 5, window->m_iWidth - 10, window->m_iHeight - 10, Color(30, 30, 30, 255));
	int TabCount = window->Tabs.size();
	if (TabCount) // If there are some tabs
	{
		int TabSize = (window->m_iWidth - 4 - 12) / TabCount;
		for (int i = 0; i < TabCount; i++)
		{
			RECT TabArea = { window->m_x + 8 + (i*TabSize), window->m_y + 1 + 27, TabSize, 29 };
			CTab *tab = window->Tabs[i];
			if (window->SelectedTab == tab)
			{
				Render::GradientV(window->m_x + 8 + (i*TabSize), window->m_y + 1 + 27, TabSize, 29, Color(30, 30, 30, 255), Color(31, 31, 31, 255));         
			}
			else if (IsMouseInRegion(TabArea))
			{
				Render::GradientV(window->m_x + 8 + (i*TabSize), window->m_y + 1 + 27, TabSize, 29, Color(10, 10, 10, 255), Color(17, 17, 17, 255));   
			}
			RECT TextSize = Render::GetTextSize(Render::Fonts::icons, tab->Title.c_str());
			Render::Text(TabArea.left + (TabSize / 2) - (TextSize.right / 2), TabArea.top + 8, Color(255, 255, 255, 255), Render::Fonts::icons, tab->Title.c_str()); 
			Render::Clear(window->m_x + 0, window->m_y + 1 - 1, window->m_iWidth - 18 - -12, 2, Color(10, 10, 10, 200));         
		}
	}



			
	
	//Render::Clear(window->m_x, window->m_y, window->m_iWidth, 1 + TextSize.bottom, Color(0, 150, 255, 255));

	RECT b1g = Render::GetTextSize(Render::Fonts::icons, window->Title.c_str());
	//Render::Clear(window->m_x, window->m_y + TextSize.bottom + 10, window->m_iWidth, 25, Color(0, 150, 255, 255));
	Render::Text(window->m_x + 8 + (window->m_iWidth / 2) - b1g.right / 2, window->m_y + 1, Color(255, 255, 255, 255), Render::Fonts::icons, window->Title.c_str());
	

	// Controls 

	// Controls 
	if (window->SelectedTab != nullptr)
	{
		// Focused widget
		bool SkipWidget = false;
		CControl* SkipMe = nullptr;

		// this window is focusing on a widget??
		if (window->IsFocusingControl)
		{
			if (window->FocusedControl != nullptr)
			{
				// We need to draw it last, so skip it in the regular loop 
				SkipWidget = true;
				SkipMe = window->FocusedControl;
			}
		}


		// Itterate over all the other controls
		for (auto control : window->SelectedTab->Controls)
		{
			if (SkipWidget && SkipMe == control)
				continue;

			if (control != nullptr && control->Flag(UIFlags::UI_Drawable))
			{
				POINT cAbs = control->GetAbsolutePos();
				RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
				bool hover = false;
				if (IsMouseInRegion(controlRect))
				{
					hover = true;
				}
				control->Draw(hover);
			}
		}

		// Draw the skipped widget last
		if (SkipWidget)
		{
			auto control = window->FocusedControl;

			if (control != nullptr && control->Flag(UIFlags::UI_Drawable))
			{
				POINT cAbs = control->GetAbsolutePos();
				RECT controlRect = { cAbs.x, cAbs.y, control->m_iWidth, control->m_iHeight };
				bool hover = false;
				if (IsMouseInRegion(controlRect))
				{
					hover = true;
				}
				control->Draw(hover);
			}
		}

	}


	return true;
}



void CGUI::RegisterWindow(CWindow* window)
{
	Windows.push_back(window);

	// Resorting to put groupboxes at the start
	for (auto tab : window->Tabs)
	{
		for (auto control : tab->Controls)
		{
			if (control->Flag(UIFlags::UI_RenderFirst))
			{
				CControl * c = control;
				tab->Controls.erase(std::remove(tab->Controls.begin(), tab->Controls.end(), control), tab->Controls.end());
				tab->Controls.insert(tab->Controls.begin(), control);
			}
		}
	}
}





void CGUI::BindWindow(unsigned char Key, CWindow* window)
{
	if (window)
		WindowBinds[Key] = window;
	else
		WindowBinds.erase(Key);
}

void CGUI::SaveWindowState(CWindow* window, std::string Filename)
{
	// Create a whole new document and we'll just save over top of the old one
	tinyxml2::XMLDocument Doc;

	// Root Element is called "AC"
	tinyxml2::XMLElement *Root = Doc.NewElement("AC");
	Doc.LinkEndChild(Root);

	Utilities::Log("Saving Window %s", window->Title.c_str());

	// If the window has some tabs..
	if (Root && window->Tabs.size() > 0)
	{
		for (auto Tab : window->Tabs)
		{
			// Add a new section for this tab
			tinyxml2::XMLElement *TabElement = Doc.NewElement(Tab->Title.c_str());
			Root->LinkEndChild(TabElement);

			Utilities::Log("Saving Tab %s", Tab->Title.c_str());

			// Now we itterate the controls this tab contains
			if (TabElement && Tab->Controls.size() > 0)
			{
				for (auto Control : Tab->Controls)
				{
					// If the control is ok to be saved
					if (Control && Control->Flag(UIFlags::UI_SaveFile) && Control->FileIdentifier.length() > 1 && Control->FileControlType)
					{
						// Create an element for the control
						tinyxml2::XMLElement *ControlElement = Doc.NewElement(Control->FileIdentifier.c_str());
						TabElement->LinkEndChild(ControlElement);

						Utilities::Log("Saving control %s", Control->FileIdentifier.c_str());

						if (!ControlElement)
						{
							Utilities::Log("Errorino :("); // s0 cute
							return;
						}

						CCheckBox* cbx = nullptr;
						CComboBox* cbo = nullptr;
						CKeyBind* key = nullptr;
						CSlider* sld = nullptr;

						// Figure out what kind of control and data this is
						switch (Control->FileControlType)
						{
						case UIControlTypes::UIC_CheckBox:
							cbx = (CCheckBox*)Control;
							ControlElement->SetText(cbx->GetState());
							break;
						case UIControlTypes::UIC_ComboBox:
							cbo = (CComboBox*)Control;
							ControlElement->SetText(cbo->GetIndex());
							break;
						case UIControlTypes::UIC_KeyBind:
							key = (CKeyBind*)Control;
							ControlElement->SetText(key->GetKey());
							break;
						case UIControlTypes::UIC_Slider:
							sld = (CSlider*)Control;
							ControlElement->SetText(sld->GetValue());
							break;
						}
					}
				}
			}
		}
	}

	//Save the file
	if (Doc.SaveFile(Filename.c_str()) != tinyxml2::XML_NO_ERROR)
	{
		MessageBox(NULL, "Failed To Save Config File!", "Shit story", MB_OK);
	}

}
void CGUI::LoadWindowState(CWindow* window, std::string Filename)
{
	tinyxml2::XMLDocument Doc;
	if (Doc.LoadFile(Filename.c_str()) == tinyxml2::XML_NO_ERROR)
	{
		tinyxml2::XMLElement *Root = Doc.RootElement();

		if (Root)
		{
			if (Root && window->Tabs.size() > 0)
			{
				for (auto Tab : window->Tabs)
				{
					tinyxml2::XMLElement *TabElement = Root->FirstChildElement(Tab->Title.c_str());
					if (TabElement)
					{
						if (TabElement && Tab->Controls.size() > 0)
						{
							for (auto Control : Tab->Controls)
							{
								if (Control && Control->Flag(UIFlags::UI_SaveFile) && Control->FileIdentifier.length() > 1 && Control->FileControlType)
								{
									tinyxml2::XMLElement *ControlElement = TabElement->FirstChildElement(Control->FileIdentifier.c_str());

									if (ControlElement)
									{
										CCheckBox* cbx = nullptr;
										CComboBox* cbo = nullptr;
										CKeyBind* key = nullptr;
										CSlider* sld = nullptr;

										switch (Control->FileControlType)
										{
										case UIControlTypes::UIC_CheckBox:
											cbx = (CCheckBox*)Control;
											cbx->SetState(ControlElement->GetText()[0] == '1' ? true : false);
											break;
										case UIControlTypes::UIC_ComboBox:
											cbo = (CComboBox*)Control;
											cbo->SelectIndex(atoi(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_KeyBind:
											key = (CKeyBind*)Control;
											key->SetKey(atoi(ControlElement->GetText()));
											break;
										case UIControlTypes::UIC_Slider:
											sld = (CSlider*)Control;
											sld->SetValue(atof(ControlElement->GetText()));
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}