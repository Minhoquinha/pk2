//#########################
//Pekka Kana 2
//by Janne Kivilahti from Piste Gamez (2003)
//#########################
#include "game/game.hpp"

#include "game/gifts.hpp"
#include "system.hpp"
#include "gfx/text.hpp"
#include "gfx/particles.hpp"
#include "episode.hpp"

#include "PisteSound.hpp"
#include "PisteDraw.hpp"

#include <cstring>

GameClass* Game = nullptr;

GameClass::GameClass(int idx) {

	this->level_id = idx;
	this->from_index = true;

}

GameClass::GameClass(const char* map) {

	strcpy(this->map_path, map);
	this->from_index = false;

}

GameClass::~GameClass() {

	delete map;

}

int GameClass::Start() {

	if (this->started)
		return 1;
	
	this->map = new PK2Kartta();

	if(this->from_index) {

		int index = this->level_id;

		strcpy(this->map_path, Episode->levels_list[index].tiedosto);
		if (Episode->levels_list[index].lapaisty)
			this->repeating = true;
		else
			this->repeating = false;
		
	}

	this->level_clear = false;

	Gifts_Clean(); //Reset gifts
	Sprites_clear(); //Reset sprites
	Prototypes_ClearAll(); //Reset prototypes
	Fadetext_Init(); //Reset fade text

	if (this->Open_Map() == 1)
		PK2_Error("Can't load map");

	this->Calculate_Tiles();

	PSound::set_musicvolume(Settings.music_max_volume);

	this->started = true;

	return 0;
	
}


int GameClass::Calculete_TileMasks(){
	
	BYTE *buffer = nullptr;
	DWORD leveys;
	int x, y;
	BYTE color;

	PDraw::drawimage_start(map->palikat_buffer, buffer, leveys);
	for (int mask=0; mask<BLOCK_MAX_MASKEJA; mask++){
		for (x=0; x<32; x++){
			y=0;
			while (y<31 && (color = buffer[x+(mask%10)*32 + (y+(mask/10)*32)*leveys])==255)
				y++;

			palikkamaskit[mask].alas[x] = y;
		}

		for (x=0; x<32; x++){
			y=31;
			while (y>=0 && (color = buffer[x+(mask%10)*32 + (y+(mask/10)*32)*leveys])==255)
				y--;

			palikkamaskit[mask].ylos[x] = 31-y;
		}
	}
	PDraw::drawimage_end(map->palikat_buffer);

	return 0;
}

//PK2KARTTA::Clean_TileBuffer()
int GameClass::Clean_TileBuffer() {

	BYTE *buffer = NULL;
	DWORD leveys;
	int x,y;

	PDraw::drawimage_start(map->palikat_buffer,*&buffer,(DWORD &)leveys);
	for (y=0;y<480;y++)
		for(x=0;x<320;x++)
			if (buffer[x+y*leveys] == 254)
				buffer[x+y*leveys] = 255;
	PDraw::drawimage_end(map->palikat_buffer);

	return 0;
}

int GameClass::Move_Blocks() {

	this->lasketut_palikat[BLOCK_HISSI_HORI].vasen = (int)cos_table[degree%360];
	this->lasketut_palikat[BLOCK_HISSI_HORI].oikea = (int)cos_table[degree%360];

	this->lasketut_palikat[BLOCK_HISSI_VERT].ala = (int)sin_table[degree%360];
	this->lasketut_palikat[BLOCK_HISSI_VERT].yla = (int)sin_table[degree%360];

	int kytkin1_y = 0,
		kytkin2_y = 0,
		kytkin3_x = 0;

	if (this->button1 > 0) {
		kytkin1_y = 64;

		if (this->button1 < 64)
			kytkin1_y = this->button1;

		if (this->button1 > KYTKIN_ALOITUSARVO-64)
			kytkin1_y = KYTKIN_ALOITUSARVO - this->button1;
	}

	if (this->button2 > 0) {
		kytkin2_y = 64;

		if (this->button2 < 64)
			kytkin2_y = this->button2;

		if (this->button2 > KYTKIN_ALOITUSARVO-64)
			kytkin2_y = KYTKIN_ALOITUSARVO - this->button2;
	}

	if (this->button3 > 0) {
		kytkin3_x = 64;

		if (this->button3 < 64)
			kytkin3_x = this->button3;

		if (this->button3 > KYTKIN_ALOITUSARVO-64)
			kytkin3_x = KYTKIN_ALOITUSARVO - this->button3;
	}

	kytkin1_y /= 2;
	kytkin2_y /= 2;
	kytkin3_x /= 2;

	this->lasketut_palikat[BLOCK_KYTKIN1].ala = kytkin1_y;
	this->lasketut_palikat[BLOCK_KYTKIN1].yla = kytkin1_y;

	this->lasketut_palikat[BLOCK_KYTKIN2_YLOS].ala = -kytkin2_y;
	this->lasketut_palikat[BLOCK_KYTKIN2_YLOS].yla = -kytkin2_y;

	this->lasketut_palikat[BLOCK_KYTKIN2_ALAS].ala = kytkin2_y;
	this->lasketut_palikat[BLOCK_KYTKIN2_ALAS].yla = kytkin2_y;

	this->lasketut_palikat[BLOCK_KYTKIN2].ala = kytkin2_y;
	this->lasketut_palikat[BLOCK_KYTKIN2].yla = kytkin2_y;

	this->lasketut_palikat[BLOCK_KYTKIN3_OIKEALLE].oikea = kytkin3_x;
	this->lasketut_palikat[BLOCK_KYTKIN3_OIKEALLE].vasen = kytkin3_x;
	this->lasketut_palikat[BLOCK_KYTKIN3_OIKEALLE].koodi = BLOCK_HISSI_HORI;

	this->lasketut_palikat[BLOCK_KYTKIN3_VASEMMALLE].oikea = -kytkin3_x;
	this->lasketut_palikat[BLOCK_KYTKIN3_VASEMMALLE].vasen = -kytkin3_x;
	this->lasketut_palikat[BLOCK_KYTKIN3_VASEMMALLE].koodi = BLOCK_HISSI_HORI;

	this->lasketut_palikat[BLOCK_KYTKIN3].ala = kytkin3_x;
	this->lasketut_palikat[BLOCK_KYTKIN3].yla = kytkin3_x;
}

int GameClass::Calculate_Tiles() {
	PK2BLOCK palikka;

	for (int i=0;i<150;i++){
		palikka = this->lasketut_palikat[i];

		palikka.vasen  = 0;
		palikka.oikea  = 0;//32
		palikka.yla	   = 0;
		palikka.ala    = 0;//32

		palikka.koodi  = i;

		if ((i < 80 || i > 139) && i != 255){
			palikka.tausta = false;

			palikka.oikealle	= BLOCK_SEINA;
			palikka.vasemmalle	= BLOCK_SEINA;
			palikka.ylos		= BLOCK_SEINA;
			palikka.alas		= BLOCK_SEINA;

			// Erikoislattiat

			if (i > 139){
				palikka.oikealle	= BLOCK_TAUSTA;
				palikka.vasemmalle	= BLOCK_TAUSTA;
				palikka.ylos		= BLOCK_TAUSTA;
				palikka.alas		= BLOCK_TAUSTA;
			}

			// L�pik�velt�v� lattia

			if (i == BLOCK_ESTO_ALAS){
				palikka.oikealle	= BLOCK_TAUSTA;
				palikka.ylos		= BLOCK_TAUSTA;
				palikka.alas		= BLOCK_SEINA;
				palikka.vasemmalle	= BLOCK_TAUSTA;
				palikka.ala -= 27;
			}

			// M�et

			if (i > 49 && i < 60){
				palikka.oikealle	= BLOCK_TAUSTA;
				palikka.ylos		= BLOCK_SEINA;
				palikka.alas		= BLOCK_SEINA;
				palikka.vasemmalle	= BLOCK_TAUSTA;
				palikka.ala += 1;
			}

			// Kytkimet

			if (i >= BLOCK_KYTKIN1 && i <= BLOCK_KYTKIN3){
				palikka.oikealle	= BLOCK_SEINA;
				palikka.ylos		= BLOCK_SEINA;
				palikka.alas		= BLOCK_SEINA;
				palikka.vasemmalle	= BLOCK_SEINA;
			}
		}
		else{
			palikka.tausta = true;

			palikka.oikealle	= BLOCK_TAUSTA;
			palikka.vasemmalle	= BLOCK_TAUSTA;
			palikka.ylos		= BLOCK_TAUSTA;
			palikka.alas		= BLOCK_TAUSTA;
		}

		if (i > 131 && i < 140)
			palikka.vesi = true;
		else
			palikka.vesi = false;

		this->lasketut_palikat[i] = palikka;
	}

	Move_Blocks();

	return 0;
}

int GameClass::Open_Map() {
	
	char path[PE_PATH_SIZE] = "";
	Episode->Get_Dir(path);

	if (map->Lataa(path, map_path) == 1){
		printf("PK2    - Error loading map '%s' at '%s'\n", map_path, path);
		return 1;
	}

	timeout = map->aika;

	if (timeout > 0)
		has_time = true;
	else
		has_time = false;

	if (strcmp(map->versio,"1.2") == 0 || strcmp(map->versio,"1.3") == 0)
		if (Prototypes_GetAll() == 1)
			return 1;

	Calculete_TileMasks();

	if (Clean_TileBuffer() == 1)
		return 1;

	map->Place_Sprites();
	map->Select_Start();
	map->Calculate_Edges();

	keys = map->Count_Keys();
	
	Sprites_start_directions();

	Particles_Clear();
	Particles_LoadBG(map);

	if ( strcmp(map->musiikki, "") != 0 ) {
		char music_path[PE_PATH_SIZE] = "";
		Episode->Get_Dir(music_path);
		strcat(music_path, map->musiikki);
		if (PSound::start_music(music_path) != 0) {

			printf("Can't load '%s'. ", music_path);
			strcpy(music_path, "music/");
			strcat(music_path, map->musiikki);
			printf("Trying '%s'.\n", music_path);

			if (PSound::start_music(music_path) != 0) {

				printf("Can't load '%s'. Trying 'music/song01.xm'.\n", music_path);

				if (PSound::start_music("music/song01.xm") != 0) {
					PK2_Error("Can't find song01.xm");
				}
			}
		}
	}
	return 0;
}

void GameClass::Show_Info(const char *text) {

	if (strcmp(text, info) != 0 || info_timer == 0) {

		strcpy(info, text);
		info_timer = INFO_TIME;
	
	}
}

bool GameClass::isStarted() {

	return started;

}