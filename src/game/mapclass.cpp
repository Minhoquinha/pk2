//#########################
//Pekka Kana 2
//by Janne Kivilahti from Piste Gamez (2003)
//#########################
#include "game/mapclass.hpp"

#include "game/sprites.hpp"
#include "game/game.hpp"
#include "gfx/effect.hpp"
#include "language.hpp"
#include "system.hpp"

#include "engine/PDraw.hpp"
#include "engine/PUtils.hpp"
#include "engine/PInput.hpp"

#include <SDL_rwops.h>

#include <inttypes.h>
#include <cstring>

int aste,
	vesiaste = 0,
	animaatio,
	ajastin1,
	ajastin2,
	ajastin3,
	avaimet;

int ruudun_leveys_palikoina  = 21,
	ruudun_korkeus_palikoina = 16;

struct PK2KARTTA{ // Vanha versio 0.1
	char		versio[8];
	char		nimi[40];
	BYTE		taustakuva;
	BYTE		musiikki;
	BYTE		kartta [640*224];
	BYTE		palikat[320*256];
	BYTE		extrat [640*480];
};

void MapClass_Animoi(int degree, int anim, int aika1, int aika2, int aika3, bool keys){
	aste = degree;
	animaatio = anim;
	ajastin1 = aika1;
	ajastin2 = aika2;
	ajastin3 = aika3;
	avaimet  = keys;
}

void MapClass_Set_Screen_Size(int width, int height){
	ruudun_leveys_palikoina  = width/32 + 1;
	ruudun_korkeus_palikoina = height/32 + 1;
}

//char MapClass::pk2_hakemisto[256] = "";

MapClass::MapClass(){

	this->palikat_buffer = -1;
	this->taustakuva_buffer = -1;
	this->palikat_vesi_buffer = -1;

	strcpy(this->versio, PK2KARTTA_VIIMEISIN_VERSIO);
	strcpy(this->palikka_bmp,"blox.bmp");
	strcpy(this->taustakuva, "default.bmp");
	strcpy(this->musiikki,   "default.xm");

	strcpy(this->nimi,  "untitled");
	strcpy(this->tekija,"unknown");

	this->jakso		= 0;
	this->ilma		= ILMA_NORMAALI;
	this->kytkin1_aika = KYTKIN_ALOITUSARVO;
	this->kytkin2_aika = KYTKIN_ALOITUSARVO;
	this->kytkin3_aika = KYTKIN_ALOITUSARVO;
	this->pelaaja_sprite = 0;
	this->aika		= 0;
	this->extra		= PK2KARTTA_EXTRA_EI;
	this->tausta	= TAUSTA_STAATTINEN;

	this->x = 0;
	this->y = 0;
	this->icon = 0;

	memset(this->taustat, 255, sizeof(taustat));
	memset(this->seinat , 255, sizeof(seinat));
	memset(this->spritet, 255, sizeof(spritet));
	memset(this->reunat,  0,   sizeof(reunat));

	for (int i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
		strcpy(this->protot[i],"");

	this->taustakuva_buffer = PDraw::image_new(640,480);
	this->palikat_buffer = PDraw::image_new(PK2KARTTA_BLOCK_PALETTI_LEVEYS,PK2KARTTA_BLOCK_PALETTI_KORKEUS);
	this->palikat_vesi_buffer = PDraw::image_new(PK2KARTTA_BLOCK_PALETTI_LEVEYS,32); //water

	PDraw::image_fill(this->taustakuva_buffer,255);
	PDraw::image_fill(this->palikat_buffer,255);
	PDraw::image_fill(this->palikat_buffer,255);
}

MapClass::MapClass(const MapClass &kartta){

	this->palikat_buffer = -1;
	this->taustakuva_buffer = -1;
	this->palikat_vesi_buffer = -1;

	strcpy(this->versio,		kartta.versio);
	strcpy(this->palikka_bmp,	kartta.palikka_bmp);
	strcpy(this->taustakuva,	kartta.taustakuva);
	strcpy(this->musiikki,		kartta.musiikki);

	strcpy(this->nimi,			kartta.nimi);
	strcpy(this->tekija,		kartta.tekija);

	this->jakso			= kartta.jakso;
	this->ilma			= kartta.ilma;
	this->kytkin1_aika	= kartta.kytkin1_aika;
	this->kytkin2_aika	= kartta.kytkin2_aika;
	this->kytkin3_aika	= kartta.kytkin3_aika;
	this->pelaaja_sprite = kartta.pelaaja_sprite;
	this->aika			= kartta.aika;
	this->extra			= kartta.extra;
	this->tausta		= kartta.tausta;

	this->x				= kartta.x;
	this->y				= kartta.y;
	this->icon			= kartta.icon;

	int i;

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->taustat[i] = kartta.taustat[i];

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->seinat[i] = kartta.seinat[i];

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->spritet[i] = kartta.spritet[i];

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->reunat[i] = kartta.reunat[i];

	for (i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
		strcpy(this->protot[i],kartta.protot[i]);


	PDraw::image_copy(kartta.taustakuva_buffer,this->taustakuva_buffer);
	PDraw::image_copy(kartta.palikat_buffer,this->palikat_buffer);
	PDraw::image_copy(kartta.palikat_vesi_buffer,this->palikat_vesi_buffer);
}

MapClass::~MapClass(){
	PDraw::image_delete(this->palikat_buffer);
	PDraw::image_delete(this->taustakuva_buffer);
	PDraw::image_delete(this->palikat_vesi_buffer);
}

MAPREC MapClass::LaskeTallennusAlue(BYTE *lahde, BYTE *&kohde){

	int x,y;
	int kartan_vasen = PK2KARTTA_KARTTA_LEVEYS,//PK2KARTTA_KARTTA_LEVEYS/2,
		kartan_oikea = 0,
		kartan_yla	 = PK2KARTTA_KARTTA_KORKEUS,//PK2KARTTA_KARTTA_KORKEUS/2,
		kartan_ala	 = 0,
		kartan_korkeus = 0,
		kartan_leveys = 0;

	MAPREC rajat = {0,0,0,0};

	// tutkitaan kartan reunimmaiset tilet ja asetetaan reunat niiden mukaan
	for (y=0;y<PK2KARTTA_KARTTA_KORKEUS;y++) {
		for (x=0;x<PK2KARTTA_KARTTA_LEVEYS;x++)	{
			if (lahde[x+y*PK2KARTTA_KARTTA_LEVEYS] != 255) {
				if (x < kartan_vasen)
					kartan_vasen = x;
				if (y < kartan_yla)
					kartan_yla = y;
				if (x > kartan_oikea)
					kartan_oikea = x;
				if (y > kartan_ala)
					kartan_ala = y;
			}
		}
	}

	kartan_leveys = kartan_oikea - kartan_vasen;
	kartan_korkeus = kartan_ala - kartan_yla;

	// onko kartta tyhja?
	if (kartan_leveys < 0 || kartan_korkeus < 0) {
		kartan_vasen = kartan_yla = 0;
		kartan_ala = kartan_oikea = 1;
		kartan_leveys = kartan_oikea - kartan_vasen;
		kartan_korkeus = kartan_ala - kartan_yla;
	}

	kohde = new BYTE[kartan_leveys*kartan_korkeus];
	BYTE tile;

	for (y=0;y<kartan_korkeus;y++){
		for (x=0;x<kartan_leveys;x++){
			tile = lahde[(x+kartan_vasen)+(y+kartan_yla)*PK2KARTTA_KARTTA_LEVEYS];
			kohde[x+y*kartan_leveys] = tile;
			if (x==0 || y==0 || x==kartan_leveys-1 || y==kartan_korkeus-1)
				lahde[(x+kartan_vasen)+(y+kartan_yla)*PK2KARTTA_KARTTA_LEVEYS] = 104;
		}
	}

	rajat.left = kartan_vasen;
	rajat.top  = kartan_yla;
	rajat.right = kartan_oikea;
	rajat.bottom= kartan_ala;

	return rajat;
}

MAPREC MapClass::LaskeTallennusAlue(BYTE *alue){

	DWORD x,y;
	DWORD kartan_vasen		= PK2KARTTA_KARTTA_LEVEYS,
		  kartan_oikea		= 0,
		  kartan_yla		= PK2KARTTA_KARTTA_KORKEUS,
		  kartan_ala		= 0;

	MAPREC rajat = {0,0,0,0};

	// tutkitaan kartan reunimmaiset tilet ja asetetaan reunat niiden mukaan
	for (y=0;y<PK2KARTTA_KARTTA_KORKEUS;y++) {
		for (x=0;x<PK2KARTTA_KARTTA_LEVEYS;x++)	{
			if (alue[x+y*PK2KARTTA_KARTTA_LEVEYS] != 255) {

				if (x < kartan_vasen) {
					kartan_vasen = x;
					//alue[x+y*PK2KARTTA_KARTTA_LEVEYS] = 101;
				}
				if (y < kartan_yla) {
					kartan_yla = y;
					//alue[x+y*PK2KARTTA_KARTTA_LEVEYS] = 102;
				}
				if (x > kartan_oikea) {
					kartan_oikea = x;
					//alue[x+y*PK2KARTTA_KARTTA_LEVEYS] = 103;
				}
				if (y > kartan_ala) {
					kartan_ala = y;
					//alue[x+y*PK2KARTTA_KARTTA_LEVEYS] = 104;
				}
			}
		}
	}

	// onko kartta tyhja?
	if (kartan_oikea < kartan_vasen || kartan_ala < kartan_yla) {
		kartan_vasen = 0;
		kartan_yla	 = 0;
		kartan_ala   = 1;
		kartan_oikea = 1;
	}

	rajat.left = kartan_vasen;
	rajat.top  = kartan_yla;
	rajat.right = kartan_oikea;
	rajat.bottom= kartan_ala;
	return rajat;
}

void MapClass::LueTallennusAlue(BYTE *lahde, MAPREC alue, int kohde){

	int x,y;
	int kartan_vasen   = alue.left,
		kartan_oikea   = alue.right,
		kartan_yla     = alue.top,
		kartan_ala     = alue.bottom,
		kartan_korkeus = kartan_oikea - kartan_vasen,
		kartan_leveys  = kartan_ala - kartan_yla;

	BYTE tile;
	if (lahde != NULL && kohde != 0)	{
		for (y=0;y<kartan_korkeus;y++) {
			for (x=0;x<kartan_leveys;x++) {
				tile = lahde[x+y*kartan_leveys];
				if (kohde == 1)
					taustat[(x+kartan_vasen)+(y+kartan_yla)*PK2KARTTA_KARTTA_LEVEYS] = tile;
				if (kohde == 2)
					seinat[(x+kartan_vasen)+(y+kartan_yla)*PK2KARTTA_KARTTA_LEVEYS] = tile;
				if (kohde == 3)
					spritet[(x+kartan_vasen)+(y+kartan_yla)*PK2KARTTA_KARTTA_LEVEYS] = tile;
			}
		}
	}
}

int MapClass::Tallenna(char *filename){
	char luku[8]; //Size can't be changed
	DWORD i;

	SDL_RWops* file = SDL_RWFromFile(filename, "w");

	strcpy(this->versio, PK2KARTTA_VIIMEISIN_VERSIO);

	//tiedosto->write ((char *)this, sizeof (*this));

	SDL_RWwrite(file, this->versio,      sizeof(versio), 1);
	SDL_RWwrite(file, this->palikka_bmp, sizeof(palikka_bmp), 1);
	SDL_RWwrite(file, this->taustakuva,  sizeof(taustakuva), 1);
	SDL_RWwrite(file, this->musiikki,    sizeof(musiikki), 1);
	SDL_RWwrite(file, this->nimi,        sizeof(nimi), 1);
	SDL_RWwrite(file, this->tekija,      sizeof(tekija), 1);

	//itoa(this->jakso,luku,10);
	sprintf(luku, "%i", this->jakso);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->ilma,luku,10);
	sprintf(luku, "%i", this->ilma);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->kytkin1_aika,luku,10);
	sprintf(luku, "%" PRIu32, this->kytkin1_aika);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->kytkin2_aika,luku,10);
	sprintf(luku, "%" PRIu32, this->kytkin2_aika);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->kytkin3_aika,luku,10);
	sprintf(luku, "%" PRIu32, this->kytkin3_aika);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->aika,luku,10);
	sprintf(luku, "%i", this->aika);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->extra,luku,10);
	sprintf(luku, "%i", this->extra);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->tausta,luku,10);
	sprintf(luku, "%i", this->tausta);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->pelaaja_sprite,luku,10);
	sprintf(luku, "%i", this->pelaaja_sprite);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->x,luku,10);
	sprintf(luku, "%i", this->x);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->y,luku,10);
	sprintf(luku, "%i", this->y);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//itoa(this->icon,luku,10);
	sprintf(luku, "%i", this->icon);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	int protoja = 0;

	for (i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
		if (strlen(this->protot[i]) > 0)
			protoja++;

	//itoa(protoja,luku,10);
	sprintf(luku, "%i", protoja);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	for (i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
		if (strlen(this->protot[i]) > 0)
			SDL_RWwrite(file, this->protot[i], sizeof(this->protot[i]), 1);

	// laske alue

	//BYTE *alue_taustat = NULL, *alue_seinat = NULL, *alue_spritet = NULL;
	MAPREC alue = {0,0,0,0};
	DWORD /*koko, aloituskohta,*/ leveys, korkeus, x, y;
	DWORD offset_x,offset_y;
	char tile[1];

	// taustat
	alue = LaskeTallennusAlue(this->taustat);
	leveys = alue.right - alue.left;
	korkeus = alue.bottom - alue.top;
	offset_x = alue.left;
	offset_y = alue.top;

	sprintf(luku, "%" PRIu32, offset_x);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	sprintf(luku, "%" PRIu32, offset_y);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	sprintf(luku, "%" PRIu32, leveys);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	sprintf(luku, "%" PRIu32, korkeus);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	for (y=offset_y;y<=offset_y+korkeus;y++) {	// Kirjoitetaan alue tiedostoon tile by tile
		for (x=offset_x;x<=offset_x+leveys;x++) {
			tile[0] = this->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS];
			SDL_RWwrite(file, tile, sizeof(tile), 1);
		}
	}

	// seinat
	alue = LaskeTallennusAlue(this->seinat);
	leveys = alue.right - alue.left;
	korkeus = alue.bottom - alue.top;
	offset_x = alue.left;
	offset_y = alue.top;
	//ltoa(offset_x,luku,10);
	sprintf(luku, "%" PRIu32, offset_x);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	sprintf(luku, "%" PRIu32, offset_y);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	sprintf(luku, "%" PRIu32, leveys);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	sprintf(luku, "%" PRIu32, korkeus);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku)); //TODO - MAKE A FUNCTION TO DO THIS
	for (y=offset_y;y<=offset_y+korkeus;y++) {	// Kirjoitetaan alue tiedostoon tile by tile
		for (x=offset_x;x<=offset_x+leveys;x++) {
			tile[0] = this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS];
			SDL_RWwrite(file, tile, sizeof(tile), 1);
		}
	}

	// spritet
	alue = LaskeTallennusAlue(this->spritet);
	leveys = alue.right - alue.left;
	korkeus = alue.bottom - alue.top;
	offset_x = alue.left;
	offset_y = alue.top;

	//ltoa(offset_x,luku,10);
	sprintf(luku, "%" PRIu32, offset_x);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//ltoa(offset_y,luku,10);
	sprintf(luku, "%" PRIu32, offset_y);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//ltoa(leveys,luku,10);
	sprintf(luku, "%" PRIu32, leveys);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));

	//ltoa(korkeus,luku,10);
	sprintf(luku, "%" PRIu32, korkeus);
	SDL_RWwrite(file, luku, sizeof(luku), 1);
	memset(luku, 0, sizeof(luku));
	for (y=offset_y;y<=offset_y+korkeus;y++) {	// Kirjoitetaan alue tiedostoon tile by tile
		for (x=offset_x;x<=offset_x+leveys;x++) {
			tile[0] = this->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS];
			SDL_RWwrite(file, tile, sizeof(tile), 1);
		}
	}

	SDL_RWclose(file);

	return 0;
}

int MapClass::Lataa(char *polku, char *nimi){
	
	char path[PE_PATH_SIZE];
	strcpy(path, polku);
	strcat(path, nimi);

	char versio[8];

	SDL_RWops* file = SDL_RWFromFile(path, "r");
	if (file == nullptr){
		return 1;
	}

	SDL_RWread(file, versio, sizeof(versio), 1);
	SDL_RWclose(file);

	int ok = 2;

	printf("PK2MAP - Loading %s, version %s\n", path, versio);

	if (strcmp(versio,"1.3")==0) {
		this->LataaVersio13(path);
		ok = 0;
	}
	if (strcmp(versio,"1.2")==0) {
		this->LataaVersio12(path);
		ok = 0;
	}
	if (strcmp(versio,"1.1")==0) {
		this->LataaVersio11(path);
		ok = 0;
	}
	if (strcmp(versio,"1.0")==0) {
		this->LataaVersio10(path);
		ok = 0;
	}
	if (strcmp(versio,"0.1")==0) {
		this->LataaVersio01(path);
		ok = 0;
	}

	Lataa_PalikkaPaletti(polku, this->palikka_bmp);
	Lataa_Taustakuva(polku, this->taustakuva);

	return(ok);
}

int MapClass::Lataa_Pelkat_Tiedot(char *polku, char *nimi){
	char path[PE_PATH_SIZE];
	strcpy(path, polku);
	strcat(path, nimi);

	char versio[8];

	SDL_RWops* file = SDL_RWFromFile(path, "r");
	if (file == nullptr){
		return 1;
	}

	SDL_RWread(file, versio, sizeof(versio), 1);
	SDL_RWclose(file);

	if (strcmp(versio,"1.3")==0)
		this->LataaVersio13(path);

	if (strcmp(versio,"1.2")==0)
		this->LataaVersio12(path);

	if (strcmp(versio,"1.1")==0)
		this->LataaVersio11(path);

	if (strcmp(versio,"1.0")==0)
		this->LataaVersio10(path);

	if (strcmp(versio,"0.1")==0)
		this->LataaVersio01(path);

	return(0);
}

int MapClass::LataaVersio01(char *filename){

	PK2KARTTA kartta;

	SDL_RWops* file = SDL_RWFromFile(filename, "r");
	if (file == nullptr) {
		return 1;
	}

	SDL_RWread(file, &kartta, sizeof(PK2KARTTA), 1);
	SDL_RWclose(file);

	strcpy(this->versio, PK2KARTTA_VIIMEISIN_VERSIO);
	strcpy(this->palikka_bmp,"blox.bmp");
	strcpy(this->taustakuva, "default.bmp");
	strcpy(this->musiikki,   "default.xm");

	strcpy(this->nimi,  "v01");
	strcpy(this->tekija,"unknown");

	this->aika		= 0;
	this->extra		= 0;
	this->tausta	= kartta.taustakuva;

	for (int i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->seinat[i] = kartta.kartta[i%PK2KARTTA_KARTTA_LEVEYS + (i/PK2KARTTA_KARTTA_LEVEYS) * 640];

	memset(this->taustat,255, sizeof(taustat));

	memset(this->spritet,255, sizeof(spritet));

	return(0);
}
int MapClass::LataaVersio10(char *filename){
	
	MapClass *kartta = new MapClass();

	SDL_RWops* file = SDL_RWFromFile(filename, "r");
	if (file == nullptr) {
		return 1;
	}

	SDL_RWread(file, &kartta, sizeof(PK2KARTTA), 1);
	SDL_RWclose(file);

	strcpy(this->versio,		kartta->versio);
	strcpy(this->palikka_bmp,	kartta->palikka_bmp);
	strcpy(this->taustakuva,	kartta->taustakuva);
	strcpy(this->musiikki,		kartta->musiikki);

	strcpy(this->nimi,			kartta->nimi);
	strcpy(this->tekija,		kartta->tekija);

	this->aika			= kartta->aika;
	this->extra			= kartta->extra;
	this->tausta		= kartta->tausta;

	for (int i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->taustat[i] = kartta->taustat[i];

	for (int i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->seinat[i] = kartta->seinat[i];

	for (int i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->spritet[i] = kartta->spritet[i];


	//Lataa_PalikkaPaletti(kartta->palikka_bmp);
	//Lataa_Taustakuva(kartta->taustakuva);

	//delete kartta;

	return(0);
}
int MapClass::LataaVersio11(char *filename){
	int virhe = 0;

	SDL_RWops* file = SDL_RWFromFile(filename, "r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->taustat, 255, sizeof(this->taustat));
	memset(this->seinat , 255, sizeof(this->seinat));
	memset(this->spritet, 255, sizeof(this->spritet));

	SDL_RWread(file, this->versio,      sizeof(char),    5);
	SDL_RWread(file, this->palikka_bmp, sizeof(char),   13);
	SDL_RWread(file, this->taustakuva,  sizeof(char),   13);
	SDL_RWread(file, this->musiikki,    sizeof(char),   13);
	SDL_RWread(file, this->nimi,        sizeof(char),   40);
	SDL_RWread(file, this->tekija,      sizeof(char),   40);
	SDL_RWread(file, &this->aika,       sizeof(int),     1);
	SDL_RWread(file, &this->extra,      sizeof(BYTE),    1);
	SDL_RWread(file, &this->tausta,     sizeof(BYTE),    1);
	SDL_RWread(file, this->taustat,     sizeof(taustat), 1);
	if (SDL_RWread(file, this->seinat,  sizeof(seinat),  1) != PK2KARTTA_KARTTA_KOKO)
		virhe = 2;
	SDL_RWread(file, this->spritet,     sizeof(spritet), 1);

	SDL_RWclose(file);

	int i;

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		if (seinat[i] != 255)
			seinat[i] -= 50;

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		if (taustat[i] != 255)
			taustat[i] -= 50;

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		if (spritet[i] != 255)
			spritet[i] -= 50;

	//Lataa_PalikkaPaletti(this->palikka_bmp);
	//Lataa_Taustakuva(this->taustakuva);

	return (virhe);
}
int MapClass::LataaVersio12(char *filename){

	char luku[8];
	
	SDL_RWops* file = SDL_RWFromFile(filename, "r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->taustat, 255, sizeof(this->taustat));
	memset(this->seinat , 255, sizeof(this->seinat));
	memset(this->spritet, 255, sizeof(this->spritet));

	for (int i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
		strcpy(this->protot[i],"");

	//tiedosto->read ((char *)this, sizeof (*this));
	SDL_RWread(file, versio,      sizeof(versio), 1);
	SDL_RWread(file, palikka_bmp, sizeof(palikka_bmp), 1);
	SDL_RWread(file, taustakuva,  sizeof(taustakuva), 1);
	SDL_RWread(file, musiikki,    sizeof(musiikki), 1);
	SDL_RWread(file, nimi,        sizeof(nimi), 1);
	SDL_RWread(file, tekija,      sizeof(tekija), 1);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->jakso = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->ilma = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->kytkin1_aika = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->kytkin2_aika = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->kytkin3_aika = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->aika = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->extra = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->tausta = atoi(luku);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->pelaaja_sprite = atoi(luku);

	SDL_RWread(file, taustat, sizeof(taustat), 1);
	SDL_RWread(file, seinat,  sizeof(seinat), 1);
	SDL_RWread(file, spritet, sizeof(spritet), 1);

	SDL_RWread(file, protot, sizeof(protot[0]), PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA);

	SDL_RWclose(file);

	//Lataa_PalikkaPaletti(this->palikka_bmp);
	//Lataa_Taustakuva(this->taustakuva);

	return 0;
}
int MapClass::LataaVersio13(char *filename){

	char luku[8];
	DWORD i;

	SDL_RWops* file = SDL_RWFromFile(filename, "r");
	if (file == nullptr) {
		return 1;
	}

	memset(this->taustat, 255, sizeof(this->taustat));
	memset(this->seinat , 255, sizeof(this->seinat));
	memset(this->spritet, 255, sizeof(this->spritet));

	for (i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
		strcpy(this->protot[i],"");

	SDL_RWread(file, versio,      sizeof(versio), 1);
	SDL_RWread(file, palikka_bmp, sizeof(palikka_bmp), 1);
	SDL_RWread(file, taustakuva,  sizeof(taustakuva), 1);
	SDL_RWread(file, musiikki,    sizeof(musiikki), 1);
	SDL_RWread(file, nimi,        sizeof(nimi), 1);
	SDL_RWread(file, tekija,      sizeof(tekija), 1);

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->jakso = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->ilma = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->kytkin1_aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->kytkin2_aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->kytkin3_aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->aika = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->extra = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->tausta = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->pelaaja_sprite = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->x = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->y = atoi(luku);
	memset(luku, 0, sizeof(luku));

	SDL_RWread(file, luku, sizeof(luku), 1);
	this->icon = atoi(luku);
	memset(luku, 0, sizeof(luku));

	DWORD lkm;
	SDL_RWread(file, luku, sizeof(luku), 1);
	lkm = (int)atoi(luku);

	SDL_RWread(file, protot, sizeof(protot[0]), lkm);

	DWORD leveys, korkeus;
	DWORD offset_x, offset_y;

	// taustat
	SDL_RWread(file, luku, sizeof(luku), 1); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1); leveys   = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1); korkeus  = atol(luku); memset(luku, 0, sizeof(luku));
	for (DWORD y = offset_y; y <= offset_y + korkeus; y++) {
		DWORD x_start = offset_x + y * PK2KARTTA_KARTTA_LEVEYS;
		SDL_RWread(file, &taustat[x_start], 1, leveys + 1);
	}

	// seinat
	SDL_RWread(file, luku, sizeof(luku), 1); offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1); offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1); leveys   = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1); korkeus  = atol(luku); memset(luku, 0, sizeof(luku));
	for (DWORD y = offset_y; y <= offset_y + korkeus; y++) {
		DWORD x_start = offset_x + y * PK2KARTTA_KARTTA_LEVEYS;
		SDL_RWread(file, &seinat[x_start], 1, leveys + 1);
	}

	//spritet
	SDL_RWread(file, luku, sizeof(luku), 1);; offset_x = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1);; offset_y = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1);; leveys   = atol(luku); memset(luku, 0, sizeof(luku));
	SDL_RWread(file, luku, sizeof(luku), 1);; korkeus  = atol(luku); memset(luku, 0, sizeof(luku));
	for (DWORD y = offset_y; y <= offset_y + korkeus; y++) {
		DWORD x_start = offset_x + y * PK2KARTTA_KARTTA_LEVEYS;
		SDL_RWread(file, &spritet[x_start], 1, leveys + 1);
	}

	SDL_RWclose(file);

	//Lataa_PalikkaPaletti(this->palikka_bmp);
	//Lataa_Taustakuva(this->taustakuva);

	return 0;
}

void MapClass::Tyhjenna(){
	strcpy(this->versio, PK2KARTTA_VIIMEISIN_VERSIO);
	strcpy(this->palikka_bmp,"blox.bmp");
	strcpy(this->taustakuva, "default.bmp");
	strcpy(this->musiikki,   "default.xm");

	strcpy(this->nimi,  "untitled");
	strcpy(this->tekija,"unknown");

	this->jakso			= 0;
	this->ilma			= ILMA_NORMAALI;
	this->kytkin1_aika	= KYTKIN_ALOITUSARVO;
	this->kytkin2_aika	= KYTKIN_ALOITUSARVO;
	this->kytkin3_aika	= KYTKIN_ALOITUSARVO;
	this->pelaaja_sprite = 0;
	this->aika		= 0;
	this->extra		= PK2KARTTA_EXTRA_EI;
	this->tausta	= PK2KARTTA_TAUSTAKUVA_EI;

	this->x = 0;
	this->y = 0;
	this->icon = 0;

	memset(this->taustat, 255, sizeof(taustat));
	memset(this->seinat,  255, sizeof(seinat));
	memset(this->spritet, 255, sizeof(spritet));

	for (int i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
		strcpy(this->protot[i],"");

	//PDraw::image_fill(this->palikat_buffer,255);
	//PDraw::image_fill(this->taustakuva_buffer,255);
}

MapClass &MapClass::operator = (const MapClass &kartta){
	if (this == &kartta) return *this;

	strcpy(this->versio,		kartta.versio);
	strcpy(this->palikka_bmp,	kartta.palikka_bmp);
	strcpy(this->taustakuva,	kartta.taustakuva);
	strcpy(this->musiikki,		kartta.musiikki);

	strcpy(this->nimi,			kartta.nimi);
	strcpy(this->tekija,		kartta.tekija);

	this->aika		= kartta.aika;
	this->extra		= kartta.extra;
	this->tausta	= kartta.tausta;

	int i;

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->seinat[i] = kartta.seinat[i];

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->taustat[i] = kartta.taustat[i];

	for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
		this->spritet[i] = kartta.spritet[i];

	PDraw::image_copy(kartta.taustakuva_buffer,this->taustakuva_buffer);
	PDraw::image_copy(kartta.palikat_buffer,this->palikat_buffer);

	return *this;
}

int MapClass::Lataa_Taustakuva(char *polku, char *filename){
	int i;
	char file[PE_PATH_SIZE];
	
	strcpy(file,polku);
	strcat(file,filename);

	if (!PisteUtils_Find(file)){
		//strcpy(file,MapClass::pk2_hakemisto);
		strcpy(file,"gfx/scenery/");
		strcat(file,filename);
		if (!PisteUtils_Find(file)) return 1;
	}

	i = PDraw::image_load(file,true);
	if(i == -1) return 2;
	PDraw::image_copy(i,this->taustakuva_buffer);
	PDraw::image_delete(i);

	strcpy(this->taustakuva,filename);

	BYTE *buffer = NULL;
	DWORD leveys;
	int x,y;
	int color;

	PDraw::drawimage_start(taustakuva_buffer,*&buffer,(DWORD &)leveys);

	for (x=0;x<640;x++)
		for (y=0;y<480;y++)
		{
			color = buffer[x+y*leveys];

			if (color == 255)
				color--;

			buffer[x+y*leveys] = color;
		}

	PDraw::drawimage_end(taustakuva_buffer);

	return 0;
}

int MapClass::Lataa_PalikkaPaletti(char *polku, char *filename){
	int i;
	int img;
	char file[PE_PATH_SIZE];
	strcpy(file,"");
	strcpy(file,polku);
	strcat(file,filename);

	if (!PisteUtils_Find(file)){
		//strcpy(file,MapClass::pk2_hakemisto);
		strcpy(file,"gfx/tiles/");
		strcat(file,filename);
		if (!PisteUtils_Find(file))
			return 1;
	}

	img = PDraw::image_load(file,false);
	if(img == -1) return 2;
	PDraw::image_copy(img,this->palikat_buffer);
	PDraw::image_delete(img);

	PDraw::image_delete(this->palikat_vesi_buffer); //Delete last water buffer
	this->palikat_vesi_buffer = PDraw::image_cut(this->palikat_buffer,0,416,320,32);

	strcpy(this->palikka_bmp,filename);
	return 0;
}

int MapClass::Lataa_TaustaMusiikki(char *filename){

	return 0;
}

void MapClass::Kopioi(MapClass &kartta){
	if (this != &kartta){
		strcpy(this->versio,		kartta.versio);
		strcpy(this->palikka_bmp,	kartta.palikka_bmp);
		strcpy(this->taustakuva,	kartta.taustakuva);
		strcpy(this->musiikki,		kartta.musiikki);

		strcpy(this->nimi,			kartta.nimi);
		strcpy(this->tekija,		kartta.tekija);

		this->jakso			= kartta.jakso;
		this->ilma			= kartta.ilma;
		this->kytkin1_aika	= kartta.kytkin1_aika;
		this->kytkin2_aika	= kartta.kytkin2_aika;
		this->kytkin3_aika	= kartta.kytkin3_aika;
		this->pelaaja_sprite = kartta.pelaaja_sprite;
		this->aika		= kartta.aika;
		this->extra		= kartta.extra;
		this->tausta	= kartta.tausta;

		int i;

		for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
			this->seinat[i] = kartta.seinat[i];

		for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
			this->taustat[i] = kartta.taustat[i];

		for (i=0;i<PK2KARTTA_KARTTA_KOKO;i++)
			this->spritet[i] = kartta.spritet[i];

		for (i=0;i<PK2KARTTA_KARTTA_MAX_PROTOTYYPPEJA;i++)
			strcpy(this->protot[i],kartta.protot[i]);

		PDraw::image_fill(palikat_buffer,255);
		PDraw::image_fill(taustakuva_buffer,0);

		PDraw::image_copy(kartta.taustakuva_buffer,this->taustakuva_buffer);
		PDraw::image_copy(kartta.palikat_buffer,this->palikat_buffer);
		PDraw::image_copy(kartta.palikat_vesi_buffer,this->palikat_vesi_buffer);
	}
}

/* Pekka Kana 2 funcitons ---------------------------------------------------------------*/


// Put in GameClass
void MapClass::Place_Sprites() {

	Sprites_clear();
	Sprites_add(this->pelaaja_sprite, 1, 0, 0, MAX_SPRITEJA, false);

	for (int x = 0; x < PK2KARTTA_KARTTA_LEVEYS; x++) {
		for (int y = 0; y < PK2KARTTA_KARTTA_KORKEUS; y++) {

			int sprite = this->spritet[x+y*PK2KARTTA_KARTTA_LEVEYS];

			if (sprite != 255 && Prototypes_List[sprite].korkeus > 0) {

				char* name = Prototypes_List[sprite].nimi;
				if (strcmp(name, "big apple") == 0 || strcmp(name, "big apple 2") == 0)
					Game->apples_count++;

				Sprites_add(sprite, 0, x*32, y*32 - Prototypes_List[sprite].korkeus+32, MAX_SPRITEJA, false);
				
			}
		}
	}

	Sprites_sort_bg();

}

void MapClass::Select_Start() {
	double  pos_x = 320,
			pos_y = 196;
	int		alkujen_maara = 0, alku = 0,
			x, y;

	for (x=0; x<PK2KARTTA_KARTTA_KOKO; x++)
		if (this->seinat[x] == BLOCK_ALOITUS)
			alkujen_maara ++;

	if (alkujen_maara > 0){
		alku = rand()%alkujen_maara + 1;
		alkujen_maara = 1;

		for (x=0; x < PK2KARTTA_KARTTA_LEVEYS; x++)
			for (y=0; y < PK2KARTTA_KARTTA_KORKEUS; y++)
				if (this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS] == BLOCK_ALOITUS){
					if (alkujen_maara == alku){
						pos_x = x*32;
						pos_y = y*32;
					}

					alkujen_maara ++;
				}
	}

	Player_Sprite->x = pos_x + Player_Sprite->tyyppi->leveys/2;
	Player_Sprite->y = pos_y - Player_Sprite->tyyppi->korkeus/2;

	Game->camera_x = (int)Player_Sprite->x;
	Game->camera_y = (int)Player_Sprite->y;
	Game->dcamera_x = Game->camera_x;
	Game->dcamera_y = Game->camera_y;

}


int MapClass::Count_Keys() {
	BYTE sprite;
	DWORD x;

	int keys = 0;

	for (x=0; x < PK2KARTTA_KARTTA_KOKO; x++){
		sprite = this->spritet[x];
		if (sprite != 255)
			if (Prototypes_List[sprite].avain && 
				Prototypes_List[sprite].tuhoutuminen != TUHOUTUMINEN_EI_TUHOUDU)

				keys++;
	}

	return keys;
}

void MapClass::Change_SkullBlocks() {
	BYTE front, back;
	DWORD x,y;

	for (x=0; x<PK2KARTTA_KARTTA_LEVEYS; x++)
		for (y=0; y<PK2KARTTA_KARTTA_KORKEUS; y++){
			front = this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS];
			back  = this->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS];

			if (front == BLOCK_KALLOSEINA){
				this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;
				if (back != BLOCK_KALLOSEINA)
					Effect_SmokeClouds(x*32+24,y*32+6);

			}

			if (back == BLOCK_KALLOTAUSTA && front == 255)
				this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS] = BLOCK_KALLOSEINA;
		}

	//Put in game
	Game->vibration = 90;//60
	PisteInput_Vibrate();

	//Game->Show_Info(tekstit->Get_Text(PK_txt.game_locksopen));

	Calculate_Edges();
}

void MapClass::Open_Locks() {
	BYTE palikka;
	DWORD x,y;

	for (x=0; x < PK2KARTTA_KARTTA_LEVEYS; x++)
		for (y=0; y < PK2KARTTA_KARTTA_KORKEUS; y++){
			palikka = this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS];
			if (palikka == BLOCK_LUKKO){
				this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;
				Effect_SmokeClouds(x*32+6,y*32+6);
			}
		}

	//Put in game
	Game->vibration = 90;//60
	PisteInput_Vibrate();

	Game->Show_Info(tekstit->Get_Text(PK_txt.game_locksopen));

	Calculate_Edges();

}

void MapClass::Calculate_Edges(){
	BYTE tile1, tile2, tile3;
	bool edge = false;

	memset(this->reunat, false, sizeof(this->reunat));

	for (int x=1;x<PK2KARTTA_KARTTA_LEVEYS-1;x++)
		for (int y=0;y<PK2KARTTA_KARTTA_KORKEUS-1;y++){
			edge = false;

			tile1 = this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS];

			if (tile1 > BLOCK_LOPETUS)
				this->seinat[x+y*PK2KARTTA_KARTTA_LEVEYS] = 255;

			tile2 = this->seinat[x+(y+1)*PK2KARTTA_KARTTA_LEVEYS];

			if (tile1 > 79 || tile1 == BLOCK_ESTO_ALAS) tile1 = 1; else tile1 = 0;
			if (tile2 > 79) tile2 = 1; else tile2 = 0;

			if (tile1 == 1 && tile2 == 1){
				tile1 = this->seinat[x+1+(y+1)*PK2KARTTA_KARTTA_LEVEYS];
				tile2 = this->seinat[x-1+(y+1)*PK2KARTTA_KARTTA_LEVEYS];

				if (tile1 < 80  && !(tile1 < 60 && tile1 > 49)) tile1 = 1; else tile1 = 0;
				if (tile2 < 80  && !(tile2 < 60 && tile2 > 49)) tile2 = 1; else tile2 = 0;

				if (tile1 == 1){
					tile3 = this->seinat[x+1+y*PK2KARTTA_KARTTA_LEVEYS];
					if (tile3 > 79 || (tile3 < 60 && tile3 > 49) || tile3 == BLOCK_ESTO_ALAS)
						edge = true;
				}

				if (tile2 == 1){
					tile3 = this->seinat[x-1+y*PK2KARTTA_KARTTA_LEVEYS];
					if (tile3 > 79 || (tile3 < 60 && tile3 > 49) || tile3 == BLOCK_ESTO_ALAS)
						edge = true;
				}

				if (edge){
					this->reunat[x+y*PK2KARTTA_KARTTA_LEVEYS] = true;
					//this->taustat[x+y*PK2KARTTA_KARTTA_LEVEYS] = 49; //Debug
				}
			}
		}
}

/* Kartanpiirtorutiineja ----------------------------------------------------------------*/
//Anim Fire
void MapClass::Animoi_Tuli(void){
	BYTE *buffer = NULL;
	DWORD leveys;
	int x,y;
	int color;

	PDraw::drawimage_start(palikat_buffer,*&buffer,(DWORD &)leveys);

	for (x=128;x<160;x++)
		for (y=448;y<479;y++)
		{
			color = buffer[x+(y+1)*leveys];

			if (color != 255)
			{
				color %= 32;
				color = color - rand()%4;

				if (color < 0)
					color = 255;
				else
				{
					if (color > 21)
						color += 128;
					else
						color += 64;
				}
			}

			buffer[x+y*leveys] = color;
		}

	if (ajastin1 < 20)
	{
		for (x=128;x<160;x++)
			buffer[x+479*leveys] = rand()%15+144;
	}
	else
		for (x=128;x<160;x++)
			buffer[x+479*leveys] = 255;

	PDraw::drawimage_end(palikat_buffer);
}
//Anim
void MapClass::Animoi_Vesiputous(void){
	BYTE *buffer = NULL;
	DWORD leveys;
	int x,y,plus;
	int color,color2;

	BYTE temp[32*32];

	PDraw::drawimage_start(palikat_buffer,*&buffer,(DWORD &)leveys);

	for (x=32;x<64;x++)
		for (y=416;y<448;y++)
			temp[x-32+(y-416)*32] = buffer[x+y*leveys];

	color2 = (temp[0]/32)*32;	// mahdollistaa erivriset vesiputoukset

	for (x=32;x<64;x++)
	{
		plus = rand()%2+2;//...+1
		for (y=416;y<448;y++)
		{
			color = temp[x-32+(y-416)*32];

			if (color != 255)	// mahdollistaa eri leveyksiset vesiputoukset
			{
				color %= 32;
				if (color > 10)//20
					color--;
				if (rand()%40 == 1)
					color = 11+rand()%11;//15+rand()%8;//9+rand()%5;
				if (rand()%160 == 1)
					color = 30;
				buffer[x + (416+(y+plus)%32)*leveys] = color+color2;
			}
			else
				buffer[x + (416+(y+plus)%32)*leveys] = color;
		}
	}

	PDraw::drawimage_end(palikat_buffer);
}
//Anim
void MapClass::Animoi_Vedenpinta(void){
	BYTE *buffer = NULL;
	DWORD leveys;
	int x,y;

	BYTE temp[32];

	PDraw::drawimage_start(palikat_buffer,*&buffer,(DWORD &)leveys);

	for (y=416;y<448;y++)
		temp[y-416] = buffer[y*leveys];

	for (y=416;y<448;y++)
	{
		for (x=0;x<31;x++)
		{
			buffer[x+y*leveys] = buffer[x+1+y*leveys];
		}
	}

	for (y=416;y<448;y++)
		buffer[31+y*leveys] = temp[y-416];

	PDraw::drawimage_end(palikat_buffer);
}
//Anim water
void MapClass::Animoi_Vesi(void){
	BYTE *buffer_lahde = NULL, *buffer_kohde = NULL;
	DWORD leveys_lahde, leveys_kohde;
	int x, y, color1, color2,
		d1 = vesiaste / 2, d2;
	int sini, cosi;
	int vx,vy;
	int i;


	PDraw::drawimage_start(palikat_buffer,		*&buffer_kohde,(DWORD &)leveys_kohde);
	PDraw::drawimage_start(palikat_vesi_buffer,*&buffer_lahde,(DWORD &)leveys_lahde);

	for (y=0;y<32;y++){
		d2 = d1;

		for (x=0;x<32;x++){
			sini = int((y+d2/2)*11.25)%360;
			cosi = int((x+d1/2)*11.25)%360;
			sini = (int)sin_table[sini];
			cosi = (int)cos_table[cosi];

			vy = (y+sini/11)%32;
			vx = (x+cosi/11)%32;

			if (vy < 0){
				vy = -vy;
				vy = 31-(vy%32);
			}

			if (vx < 0){
				vx= -vx;
				vx = 31-(vx%32);
			}

			color1 = buffer_lahde[64+vx+vy*leveys_lahde];
			buffer_lahde[32+x+y*leveys_lahde] = color1;
			d2 = 1 + d2 % 360;
		}

		d1 = 1 + d1 % 360;
	}

	int vy2;

	for (int p=2;p<5;p++){
		i = p*32;

		for (y=0;y<32;y++){
			//d2 = d1;
			vy = y*leveys_lahde;
			vy2 = (y+416)*leveys_kohde;

			for (x=0;x<32;x++){
				vx = x+vy;
				color1 = buffer_lahde[32+vx];
				color2 = buffer_lahde[ i+vx];
				buffer_kohde[i+x+vy2] = (color1 + color2*2) / 3;
			}
		}
	}
	PDraw::drawimage_end(palikat_buffer);
	PDraw::drawimage_end(palikat_vesi_buffer);
}

void MapClass::Animoi_Virta_Ylos(void){
	BYTE *buffer = NULL;
	DWORD leveys;
	int x,y;

	BYTE temp[32];

	PDraw::drawimage_start(palikat_buffer,*&buffer,(DWORD &)leveys);

	for (x=64;x<96;x++)
		temp[x-64] = buffer[x+448*leveys];

	for (x=64;x<96;x++)
	{
		for (y=448;y<479;y++)
		{
			buffer[x+y*leveys] = buffer[x+(y+1)*leveys];
		}
	}

	for (x=64;x<96;x++)
		buffer[x+479*leveys] = temp[x-64];

	PDraw::drawimage_end(palikat_buffer);
}

int MapClass::Piirra_Taustat(int kamera_x, int kamera_y, bool editor){
	int palikka;
	int px = 0,
		py = 0,
		kartta_x = kamera_x/32,
		kartta_y = kamera_y/32;

	for (int x=-1;x<ruudun_leveys_palikoina;x++){
		for (int y=0;y<ruudun_korkeus_palikoina;y++){
			if (x + kartta_x < 0 || x + kartta_x > PK2KARTTA_KARTTA_LEVEYS) continue;
			if (y + kartta_y < 0 || y + kartta_y > PK2KARTTA_KARTTA_KORKEUS) continue;

			int i = x + kartta_x + (y + kartta_y)*PK2KARTTA_KARTTA_LEVEYS;
			if(i<0 || i >= sizeof(taustat)) continue; //Dont access a not allowed address

			palikka = taustat[i];

			if (palikka != 255){
				px = ((palikka%10)*32);
				py = ((palikka/10)*32);

				if (palikka == BLOCK_ANIM1 || palikka == BLOCK_ANIM2 || palikka == BLOCK_ANIM3 || palikka == BLOCK_ANIM4)
					px += animaatio * 32;

				PDraw::image_cutclip(palikat_buffer, x*32-(kamera_x%32), y*32-(kamera_y%32), px, py, px+32, py+32);
			}
		}
	}

	return 0;
}

int MapClass::Piirra_Seinat(int kamera_x, int kamera_y, bool editor){
	int palikka;
	int px = 0,
		py = 0,
		ay = 0,
		ax = 0,
		by = 0, bx = 0,
		kartta_x = kamera_x/32,
		kartta_y = kamera_y/32;

	int ajastin1_y = 0,
		ajastin2_y = 0,
		ajastin3_x = 0;

	if (ajastin1 > 0){
		ajastin1_y = 64;

		if (ajastin1 < 64)
			ajastin1_y = ajastin1;

		if (ajastin1 > KYTKIN_ALOITUSARVO-64)
			ajastin1_y = KYTKIN_ALOITUSARVO - ajastin1;
	}

	if (ajastin2 > 0){
		ajastin2_y = 64;

		if (ajastin2 < 64)
			ajastin2_y = ajastin2;

		if (ajastin2 > KYTKIN_ALOITUSARVO-64)
			ajastin2_y = KYTKIN_ALOITUSARVO - ajastin2;
	}

	if (ajastin3 > 0){
		ajastin3_x = 64;

		if (ajastin3 < 64)
			ajastin3_x = ajastin3;

		if (ajastin3 > KYTKIN_ALOITUSARVO-64)
			ajastin3_x = KYTKIN_ALOITUSARVO - ajastin3;
	}


	for (int x=-1; x < ruudun_leveys_palikoina + 1; x++){
		for (int y=-1; y < ruudun_korkeus_palikoina + 1; y++){
			if (x + kartta_x < 0 || x + kartta_x > PK2KARTTA_KARTTA_LEVEYS) continue;
			if (y + kartta_y < 0 || y + kartta_y > PK2KARTTA_KARTTA_KORKEUS) continue;

			int i = x + kartta_x + (y + kartta_y)*PK2KARTTA_KARTTA_LEVEYS;
			if(i<0 || i >= sizeof(seinat)) continue; //Dont access a not allowed address

			palikka = seinat[i];

			if (palikka != 255 && !(!editor && palikka == BLOCK_ESTO_ALAS)){
				px = ((palikka%10)*32);
				py = ((palikka/10)*32);
				ay = 0;
				ax = 0;

				if (!editor){
					if (palikka == BLOCK_HISSI_VERT)
						ay = (int)sin_table[aste%360];

					if (palikka == BLOCK_HISSI_HORI)
						ax = (int)cos_table[aste%360];

					if (palikka == BLOCK_KYTKIN1)
						ay = ajastin1_y/2;

					if (palikka == BLOCK_KYTKIN2_YLOS)
						ay = -ajastin2_y/2;

					if (palikka == BLOCK_KYTKIN2_ALAS)
						ay = ajastin2_y/2;

					if (palikka == BLOCK_KYTKIN2)
						ay = ajastin2_y/2;

					if (palikka == BLOCK_KYTKIN3_OIKEALLE)
						ax = ajastin3_x/2;

					if (palikka == BLOCK_KYTKIN3_VASEMMALLE)
						ax = -ajastin3_x/2;

					if (palikka == BLOCK_KYTKIN3)
						ay = ajastin3_x/2;
				}

				if (palikka == BLOCK_ANIM1 || palikka == BLOCK_ANIM2 || palikka == BLOCK_ANIM3 || palikka == BLOCK_ANIM4)
					px += animaatio * 32;

				PDraw::image_cutclip(palikat_buffer, x*32-(kamera_x%32)+ax, y*32-(kamera_y%32)+ay, px, py, px+32, py+32);
			}
		}
	}

	if (vesiaste%2 == 0)
	{
		Animoi_Tuli();
		Animoi_Vesiputous();
		Animoi_Virta_Ylos();
		Animoi_Vedenpinta();
	}

	if (vesiaste%4 == 0)
	{
		Animoi_Vesi();
		PDraw::rotate_palette(224,239);
	}

	vesiaste = 1 + vesiaste % 320;

	return 0;
}