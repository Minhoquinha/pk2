#include "text.hpp"

#include "game.hpp"

#include "PisteDraw.hpp"

#include <cstring>

PK2FADETEXT fadetekstit[MAX_FADETEKSTEJA];
int fadeteksti_index = 0;

//TODO - lang menu
char langlist[60][PE_PATH_SIZE];
char langmenulist[10][PE_PATH_SIZE];
int langlistindex = 0;
int totallangs = 0;

//Fonts
int fontti1;
int fontti2;
int fontti3;
int fontti4;
int fontti5;

PisteLanguage *tekstit;

int Load_Fonts() {
	int ind_font = 0,
		ind_path = 0;

	PDraw::clear_fonts();
	ind_path = tekstit->Hae_Indeksi("font path");

	ind_font = tekstit->Hae_Indeksi("font small font");
	if (ind_path == -1 || ind_font == -1) {
        fontti1 = PDraw::font_create("language/fonts/", "ScandicSmall.txt");
		if (fontti1 == -1)
			PK2_Error("Can't create font 1 from ScandicSmall.txt");
	}
	else {
        fontti1 = PDraw::font_create(tekstit->Hae_Teksti(ind_path), tekstit->Hae_Teksti(ind_font));
		if (fontti1 == -1)
			PK2_Error("Can't create font 1");
	}

	ind_font = tekstit->Hae_Indeksi("font big font normal");
	if (ind_path == -1 || ind_font == -1) {
        fontti2 = PDraw::font_create("language/fonts/", "ScandicBig1.txt");
		if (fontti2 == -1)
			PK2_Error("Can't create font 1 from ScandicBig1.txt");
	}
	else {
        fontti2 = PDraw::font_create(tekstit->Hae_Teksti(ind_path), tekstit->Hae_Teksti(ind_font));
		if (fontti2 == -1) {
			PK2_Error("Can't create font 2");
		}
	}

	ind_font = tekstit->Hae_Indeksi("font big font hilite");
	if (ind_path == -1 || ind_font == -1) {
        fontti3 = PDraw::font_create("language/fonts/", "ScandicBig2.txt");
		if (fontti3 == -1) {
			PK2_Error("Can't create font 3 from ScandicBig2.txt");
		}
	}
	else {
        fontti3 = PDraw::font_create(tekstit->Hae_Teksti(ind_path), tekstit->Hae_Teksti(ind_font));
		if (fontti3 == -1) {
			PK2_Error("Can't create font 3");
		}
	}

	ind_font = tekstit->Hae_Indeksi("font big font shadow");
	if (ind_path == -1 || ind_font == -1) {
        fontti4 = PDraw::font_create("language/fonts/", "ScandicBig3.txt");
		if (fontti4 == -1) {
			PK2_Error("Can't create font 4 from ScandicBig3.txt");
		}
	}
	else {
        fontti4 = PDraw::font_create(tekstit->Hae_Teksti(ind_path), tekstit->Hae_Teksti(ind_font));
		if (fontti4 == -1) {
			PK2_Error("Can't create font 4");
		}
	}

	return 0;
}

int CreditsText_Draw(char *text, int font, int x, int y, DWORD start, DWORD end, DWORD time){
	int pros = 100;
	if (time > start && time < end) {

		if (time - start < 100)
			pros = time - end;

		if (end - time < 100)
			pros = end - time;

		if (pros > 0) {
			if (pros < 100)
				PDraw::font_writealpha(font,text,x,y,pros);
			else
				PDraw::font_write(font,text,x,y);
		}

	}
	return 0;
}

int Wavetext_Draw(char *teksti, int fontti, int x, int y){
	int pituus = strlen(teksti);
	int vali = 0;
	char kirjain[3] = " \0";
	int ys, xs;

	if (pituus > 0){
		for (int i=0;i<pituus;i++){
			ys = (int)(sin_table[((i+degree)*8)%360])/7;
			xs = (int)(cos_table[((i+degree)*8)%360])/9;
			kirjain[0] = teksti[i];
			PDraw::font_write(fontti4,kirjain,x+vali-xs+3,y+ys+3);
			vali += PDraw::font_write(fontti,kirjain,x+vali-xs,y+ys);
		}
	}
	return vali;
}

int WavetextSlow_Draw(char *teksti, int fontti, int x, int y){
	int pituus = strlen(teksti);
	int vali = 0;
	char kirjain[3] = " \0";
	int ys, xs;

	if (pituus > 0){
		for (int i=0;i<pituus;i++){
			ys = (int)(sin_table[((i+degree)*4)%360])/9;
			xs = (int)(cos_table[((i+degree)*4)%360])/11;
			kirjain[0] = teksti[i];

			if (Settings.lapinakyvat_menutekstit)
				vali += PDraw::font_writealpha(fontti,kirjain,x+vali-xs,y+ys,75);
			else{
				PDraw::font_write(fontti4,kirjain,x+vali-xs+1,y+ys+1);
				vali += PDraw::font_write(fontti,kirjain,x+vali-xs,y+ys);
			}


		}
	}
	return vali;
}

void Fadetext_Init(){
	for (int i=0;i<MAX_FADETEKSTEJA;i++)
		fadetekstit[i].ajastin = 0;
}

void Fadetext_New(int fontti, char *teksti, DWORD x, DWORD y, DWORD ajastin, bool ui){
	fadetekstit[fadeteksti_index].fontti = fontti;
	strcpy(fadetekstit[fadeteksti_index].teksti,teksti);
	fadetekstit[fadeteksti_index].x = x;
	fadetekstit[fadeteksti_index].y = y;
	fadetekstit[fadeteksti_index].ajastin = ajastin;
	fadetekstit[fadeteksti_index].ui = ui;
	fadeteksti_index++;

	if (fadeteksti_index >= MAX_FADETEKSTEJA)
		fadeteksti_index = 0;
}

int Fadetext_Draw(){
	int pros;
	int x, y;

	for (int i=0; i<MAX_FADETEKSTEJA; i++)
		if (fadetekstit[i].ajastin > 0){
			if (fadetekstit[i].ajastin > 50)
				pros = 100;
			else
				pros = fadetekstit[i].ajastin * 2;

			x = fadetekstit[i].ui ? fadetekstit[i].x : fadetekstit[i].x - Game::camera_x;
			y = fadetekstit[i].ui ? fadetekstit[i].y : fadetekstit[i].y - Game::camera_y;

			if (Settings.lapinakyvat_objektit && pros < 100)
				PDraw::font_writealpha(fadetekstit[i].fontti, fadetekstit[i].teksti, x, y, pros);
			else
				PDraw::font_write(fadetekstit[i].fontti, fadetekstit[i].teksti, x, y);

		}
	return 0;
}

void Fadetext_Update(){
	for (int i=0;i<MAX_FADETEKSTEJA;i++)
		if (fadetekstit[i].ajastin > 0){
			fadetekstit[i].ajastin--;

			if (fadetekstit[i].ajastin%2 == 0)
				fadetekstit[i].y--;

			if (fadetekstit[i].x < Game::camera_x || fadetekstit[i].x > Game::camera_x + screen_width ||
				fadetekstit[i].y < Game::camera_y || fadetekstit[i].y > Game::camera_y + screen_height)
				if(!fadetekstit[i].ui) fadetekstit[i].ajastin = 0;
		}
}