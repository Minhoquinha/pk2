//#########################
//Pekka Kana 2
//by Janne Kivilahti from Piste Gamez (2003)
//#########################
#include "system.hpp"

#include "engine/PUtils.hpp"
#include "engine/PDraw.hpp"
#include "settings.hpp"

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

int screen_width  = 800;
int screen_height = 480;

std::string data_path;

int game_assets = -1;
int game_assets2 = -1;
int bg_screen = -1;

int key_delay = 0;

double cos_table[360];
double sin_table[360];
int degree = 0;
int degree_temp = 0;

bool test_level = false;
bool dev_mode = false;

bool doublespeed = false;

float fps = 0;
bool show_fps = false;

bool PK2_error = false;
const char* PK2_error_msg = nullptr;

int PK2_Error(const char* msg) {
	PK2_error = true;
	PK2_error_msg = msg;

	return 0;
}

void Calculate_SinCos(){

	for ( int i = 0; i < 360; i++ ) {
	
		cos_table[i] = cos(M_PI*2*i/180) * 33;
		sin_table[i] = sin(M_PI*2*i/180) * 33;
	
	}

}

void Draw_Cursor(int x, int y){

	PDraw::image_cutclip(game_assets,x,y,621,461,640,480);
	
}