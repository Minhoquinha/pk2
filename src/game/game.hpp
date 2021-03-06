//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/types.hpp"

#include "game/mapclass.hpp"
#include "game/sprites.hpp"
#include "game/blocks.hpp"

const int TIME_FPS = 100;
const int INFO_TIME = 700;

class GameClass {

	public:

		int level_id = -1;
		MapClass* map = nullptr;
		std::string map_file;

		PK2BLOCK     palikat[300];
		PK2BLOCK     lasketut_palikat[150];//150
		PK2BLOCKMASK palikkamaskit[BLOCK_MAX_MASKS];

		bool game_over = false;
		bool level_clear = false;
		bool repeating = false;

		u32 exit_timer = 0;

		u32 timeout = 0;
		int increase_time = 0;
		int seconds = 0;
		bool has_time = false;

		int button_moving = 0;
		int button1 = 0;
		int button2 = 0;
		int button3 = 0;

		u32 score = 0;
		u32 score_increment = 0;

		u32 apples_count = 0;
		u32 apples_got = 0;

		int vibration = 0;

		int camera_x;
		int camera_y;
		double dcamera_x;
		double dcamera_y;
		double dcamera_a;
		double dcamera_b;

		bool paused = false;

		int keys = 0;

		int info_timer = 0;
		char info[80] = " ";

		int item_pannel_x = -215;

		GameClass(int idx);
		GameClass(std::string map_file);
		~GameClass();

		int Start();
		int Finnish();

		int Move_Blocks();
		void Show_Info(const char *text);

		bool isStarted();

	private:
		
		bool started = false;
		bool from_index;
		
		int Calculete_TileMasks();
		int Clean_TileBuffer();

		int Calculate_Tiles();
		int Open_Map();

};

extern GameClass* Game;

// TODO - remove
bool Draw_Menu_Text(bool active, const char *teksti, int x, int y);
//