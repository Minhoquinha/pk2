//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#include "engine/platform.hpp"

#include "engine/PDraw.hpp"
#include "engine/PInput.hpp"
#include "engine/PSound.hpp"
#include "engine/PUtils.hpp"
#include "engine/PLang.hpp"
#include "engine/PLog.hpp"
#include "engine/PFile.hpp"

namespace Piste {

void init(int width, int height, const char* name, const char* icon);
void terminate();

void loop(int (*GameLogic)());
void stop();
float get_fps();

void set_debug(bool set);
void ignore_frame();
bool is_ready();

}
