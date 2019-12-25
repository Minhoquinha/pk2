
#include "screens.hpp"

#include "text.hpp"
#include "language.hpp"
#include "game.hpp"
#include "gifts.hpp"
#include "gui.hpp"
#include "episode.hpp"
#include "sfx.hpp"
#include "sprites.hpp"

#include "PisteDraw.hpp"
#include "PisteInput.hpp"

bool siirry_pistelaskusta_karttaan = false;

int pistelaskuvaihe = 0;
int pistelaskudelay = 0;

DWORD bonuspisteet = 0;
DWORD aikapisteet = 0;
DWORD energiapisteet = 0;
DWORD esinepisteet = 0;
DWORD pelastuspisteet = 0;

int PK_Draw_ScoreCount(){
	char luku[20];
	int vali = 20;
	int my = 0, x, y;
	int kuutio, aste;
	int	vari = 0, kerroin;

	PDraw::screen_fill(0);
	PDraw::image_clip(kuva_tausta, 0, 0);

	for (aste=0;aste<18;aste++) {

		kerroin = (int)(cos_table[(degree+aste*3)%180]);

		x = (int)(sin_table[(degree+aste*10)%360]*2)+kerroin;
		y = (int)(cos_table[(degree+aste*10)%360]*2);//10 | 360 | 2
		//PDraw::image_clip(kuva_peli,320+x,240+y,157,46,181,79);
		kuutio = (int)(sin_table[(degree+aste*3)%360]);
		if (kuutio < 0) kuutio = -kuutio;

		PDraw::screen_fill(320-x,240-y,320-x+kuutio,240-y+kuutio,VARI_TURKOOSI+8);
	}
	for (aste=0;aste<18;aste++) {

		x = (int)(sin_table[(degree+aste*10)%360]*3);
		y = (int)(cos_table[(degree+aste*10)%360]*3);//10 | 360 | 3
		//PDraw::image_clip(kuva_peli,320+x,240+y,157,46,181,79);
		kuutio = (int)(sin_table[(degree+aste*2)%360])+18;
		if (kuutio < 0) kuutio = -kuutio;//0;//
		if (kuutio > 100) kuutio = 100;

		//PDraw::screen_fill(320+x,240+y,320+x+kuutio,240+y+kuutio,VARI_TURKOOSI+10);
		PDraw::image_cutcliptransparent(kuva_peli, 247, 1, 25, 25, 320+x, 240+y, kuutio, 32);
	}

	PDraw::font_write(fontti4,tekstit->Hae_Teksti(PK_txt.score_screen_title),100+2,72+2);
	PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_title),100,72);
	PDraw::font_write(fontti4,tekstit->Hae_Teksti(PK_txt.score_screen_level_score),100+2,102+2);
	vali = PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_level_score),100,102);//250,80
	fake_pisteet = bonuspisteet + aikapisteet + energiapisteet + esinepisteet + pelastuspisteet;
	ltoa(fake_pisteet,luku,10);
	PDraw::font_write(fontti4,luku,400+2,102+2);
	PDraw::font_write(fontti2,luku,400,102);
	my = 0;

	PDraw::font_write(fontti4,tekstit->Hae_Teksti(PK_txt.score_screen_bonus_score),100+2,192+2+my);
	vali = PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_bonus_score),100,192+my);
	ltoa(bonuspisteet,luku,10);
	PDraw::font_write(fontti4,luku,400+2,192+2+my);
	PDraw::font_write(fontti2,luku,400,192+my);
	my += 30;

	PDraw::font_write(fontti4,tekstit->Hae_Teksti(PK_txt.score_screen_time_score),100+2,192+2+my);
	vali = PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_time_score),100,192+my);
	ltoa(aikapisteet,luku,10);
	PDraw::font_write(fontti4,luku,400+2,192+2+my);
	PDraw::font_write(fontti2,luku,400,192+my);
	my += 30;

	PDraw::font_write(fontti4,tekstit->Hae_Teksti(PK_txt.score_screen_energy_score),100+2,192+2+my);
	vali = PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_energy_score),100,192+my);
	ltoa(energiapisteet,luku,10);
	PDraw::font_write(fontti4,luku,400+2,192+2+my);
	PDraw::font_write(fontti2,luku,400,192+my);
	my += 30;

	PDraw::font_write(fontti4,tekstit->Hae_Teksti(PK_txt.score_screen_item_score),100+2,192+2+my);
	vali = PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_item_score),100,192+my);
	ltoa(esinepisteet,luku,10);
	PDraw::font_write(fontti4,luku,400+2,192+2+my);
	PDraw::font_write(fontti2,luku,400,192+my);
	my += 30;

	x = 110;
	y = 192 + my;

	for (int i = 0; i < MAX_GIFTS; i++)
		if (Gifts_Get(i) != -1)	{
			//PK2Sprite_Prototyyppi* prot = Gifts_GetProtot(i);
			Gifts_Draw(i, x, y);
			//prot->Piirra(x - prot->leveys/2, y - prot->korkeus / 2, 0);
			x += 38;
		}

	my += 10;

	if (pistelaskuvaihe >= 4){
		PDraw::font_write(fontti4,tekstit->Hae_Teksti(PK_txt.score_screen_total_score),100+2,192+2+my);
		vali = PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_total_score),100,192+my);//250,80
		ltoa(pisteet,luku,10);
		PDraw::font_write(fontti4,luku,400+2,192+2+my);
		PDraw::font_write(fontti2,luku,400,192+my);
		my += 25;

		if (jakso_uusi_ennatys) {
			PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_new_level_hiscore),100+rand()%2,192+my+rand()%2);
			my += 25;
		}
		if (jakso_uusi_ennatysaika) {
			PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_new_level_best_time),100+rand()%2,192+my+rand()%2);
			my += 25;
		}
		if (episodi_uusi_ennatys) {
			PDraw::font_write(fontti2,tekstit->Hae_Teksti(PK_txt.score_screen_new_episode_hiscore),100+rand()%2,192+my+rand()%2);
			my += 25;
		}
	}

	if (Draw_Menu_Text(true,tekstit->Hae_Teksti(PK_txt.score_screen_continue),100,430)){
		music_volume = 0;
		siirry_pistelaskusta_karttaan = true;
		PDraw::fade_out(PDraw::FADE_SLOW);
		//next_screen = SCREEN_MAP;
	}

	PK_Draw_Cursor(hiiri_x, hiiri_y);

	return 0;
}


int Screen_ScoreCount_Init() {
	GUI_Change(UI_CURSOR);
	if (Settings.isWide)
		PDraw::set_xoffset(80);
	else
		PDraw::set_xoffset(0);
	PDraw::screen_fill(0);

	PDraw::image_delete(kuva_tausta);
	kuva_tausta = PDraw::image_load("gfx/menu.bmp", true);
	PK_MenuShadow_Create(kuva_tausta, 640, 480, 30);

	jakso_uusi_ennatys = false;
	jakso_uusi_ennatysaika = false;
	episodi_uusi_ennatys = false;

	// Lasketaan pelaajan kokonaispisteet etuk�teen
	DWORD temp_pisteet = 0;
	temp_pisteet += jakso_pisteet;
	temp_pisteet += timeout * 5;
	temp_pisteet += Player_Sprite->energia * 300;
	for (int i = 0; i < MAX_GIFTS; i++)
		if (Gifts_Get(i) != -1)
			temp_pisteet += Gifts_GetProtot(i)->pisteet + 500;

	//if (jaksot[jakso_indeksi_nyt].lapaisty)
	//if (jaksot[jakso_indeksi_nyt].jarjestys == jakso-1)
	pisteet += temp_pisteet;

	if (uusinta)
		pisteet -= temp_pisteet;

	fake_pisteet = 0;
	pistelaskuvaihe = 0;
	pistelaskudelay = 30;
	bonuspisteet = 0,
	aikapisteet = 0,
	energiapisteet = 0,
	esinepisteet = 0,
	pelastuspisteet = 0;

	char pisteet_tiedosto[PE_PATH_SIZE] = "scores.dat";
	int vertailun_tulos;

	/* Tutkitaan onko pelaajarikkonut kent�n piste- tai nopeusenn�tyksen */
	vertailun_tulos = EpisodeScore_Compare(jakso_indeksi_nyt, temp_pisteet, kartta->aika - timeout, false);
	if (vertailun_tulos > 0)
	{
		EpisodeScore_Save(pisteet_tiedosto);
	}

	/* Tutkitaan onko pelaaja rikkonut episodin piste-enn�tyksen */
	vertailun_tulos = EpisodeScore_Compare(0, pisteet, 0, true);
	if (vertailun_tulos > 0)
		EpisodeScore_Save(pisteet_tiedosto);

	music_volume = Settings.music_max_volume;

	siirry_pistelaskusta_karttaan = false;

	PDraw::fade_in(PDraw::FADE_FAST);
}

int Screen_ScoreCount(){
	PK_Draw_ScoreCount();

	degree = 1 + degree % 360;

	// PISTELASKU

	int energia = Player_Sprite->energia;

	if (pistelaskudelay == 0){
		if (bonuspisteet < jakso_pisteet){
			pistelaskuvaihe = 1;
			pistelaskudelay = 0;
			bonuspisteet += 10;

			if (degree%7==1)
				Play_MenuSFX(pistelaskuri_aani, 70);

			if (bonuspisteet >= jakso_pisteet){
				bonuspisteet = jakso_pisteet;
				pistelaskudelay = 50;
			}

		} else if (timeout > 0){
			pistelaskuvaihe = 2;
			pistelaskudelay = 0;
			aikapisteet+=5;
			timeout--;

			if (degree%10==1)
				Play_MenuSFX(pistelaskuri_aani, 70);

			if (timeout == 0)
				pistelaskudelay = 50;

		} else if (Player_Sprite->energia > 0){
			pistelaskuvaihe = 3;
			pistelaskudelay = 10;
			energiapisteet+=300;
			Player_Sprite->energia--;

			Play_MenuSFX(pistelaskuri_aani, 70);

		} else if (Gifts_Count() > 0){
			pistelaskuvaihe = 4;
			pistelaskudelay = 30;
			for (int i = 0; i < MAX_GIFTS; i++)
				if (Gifts_Get(i) != -1) {
					esinepisteet += Gifts_GetProtot(i)->pisteet + 500;
					//esineet[i] = NULL;
					Play_MenuSFX(hyppy_aani, 100);
					break;
				}
		}
		else pistelaskuvaihe = 5;
	}

	if (pistelaskudelay > 0)
		pistelaskudelay--;

	if (siirry_pistelaskusta_karttaan && !PDraw::is_fading()){
		/*tarkistetaan oliko viimeinen jakso*/

		if (jakso_indeksi_nyt == EPISODI_MAX_LEVELS-1) { // ihan niin kuin joku tekisi n�in monta jaksoa...
			next_screen = SCREEN_END;
		}
		else if (jaksot[jakso_indeksi_nyt+1].jarjestys == -1)
			    next_screen = SCREEN_END;
		else // jos ei niin palataan karttaan...
		{
			next_screen = SCREEN_MAP;
			//episode_started = false;
			//menu_nyt = MENU_MAIN;
		}
	}

	if (key_delay > 0)
		key_delay--;

	if (key_delay == 0){
		if (PisteInput_Keydown(PI_RETURN) && pistelaskuvaihe == 5){
			siirry_pistelaskusta_karttaan = true;
			music_volume = 0;
			PDraw::fade_out(PDraw::FADE_SLOW);
			key_delay = 20;
		}

		if (PisteInput_Keydown(PI_RETURN) && pistelaskuvaihe < 5){
			pistelaskuvaihe = 5;
			bonuspisteet = jakso_pisteet;
			aikapisteet += timeout*5;
			timeout = 0;
			energiapisteet += Player_Sprite->energia * 300;
			Player_Sprite->energia = 0;
			for (int i=0;i<MAX_GIFTS;i++)
				if (Gifts_Get(i) != -1)
					esinepisteet += Gifts_GetProtot(i)->pisteet + 500;
			
			Gifts_Clean(); //TODO esineita = 0;

			key_delay = 20;
		}
	}

	return 0;
}