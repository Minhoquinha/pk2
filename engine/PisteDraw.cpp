//#########################
//Pekka Kana 2
//by Janne Kivilahti from Piste Gamez (2003)
//#########################
#include "PisteDraw.hpp"

#include "filters/hqx.h"
#include "PisteInput.hpp"
#include <thread>

#include "PisteFont.hpp"
#include "PisteGui.hpp"

#include <algorithm>
#include <vector>

#include <SDL_image.h>

namespace PDraw {

const char* window_name;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Surface* window_icon; 

SDL_Surface* frameBuffer8 = NULL;
SDL_Palette* game_palette = NULL;

uint8_t* hqx_src = NULL;
uint32_t* hqx_dst = NULL;
uint32_t* game_palette_yuv = NULL;

std::vector<SDL_Surface*> imageList;
std::vector<PFont*> fontList;

int screen_width;
int screen_height;

SDL_Rect screen_dest = {0, 0, 0, 0};
bool screen_fit = false;

bool ready = false;

int fade_speed = 0;
int alpha = 100;

int x_offset = 0;

int findfreeimage(){
    int size = imageList.size();

    for(int i = 0; i < size; i++)
        if(imageList[i] == nullptr)
            return i;

    imageList.push_back(nullptr);
    return size;
}
int findfreefont(){
    int size = fontList.size();

    for(int i = 0; i < size; i++)
        if(fontList[i] == nullptr)
            return i;

    fontList.push_back(nullptr);
    return size;
}

uint32_t rgb_to_yuv(uint32_t r, uint32_t g, uint32_t b) {
    uint32_t y, u, v;

    y = (uint32_t)(0.299*r + 0.587*g + 0.114*b);
    u = (uint32_t)(-0.169*r - 0.331*g + 0.5*b) + 128;
    v = (uint32_t)(0.5*r - 0.419*g - 0.081*b) + 128;
    return (y << 16) + (u << 8) + v;
}
void update_yuv_palette() {
    SDL_Color* colors = game_palette->colors;
    //for (int i = 0; i < 256; i++)
    //    game_palette_yuv[i] = rgb_to_yuv(colors[i].r, colors[i].g, colors[i].b);
    for (int i = 0; i < 256; i++)
        game_palette_yuv[i] = (colors[i].r << 16) + (colors[i].g << 8) + colors[i].b;
}

bool is_fading(){
  if (alpha > 0 && fade_speed < 0)
    return true;

  if (alpha < 100 && fade_speed > 0)
    return true;

  return false;
}
int  fade_out(int speed){
    alpha = 100;
    fade_speed = -speed;
    return 0;
}
int  fade_in(int speed){
    alpha = 0;
    fade_speed = speed;
    return 0;
}
void rotate_palette(BYTE start, BYTE end){
    BYTE i;
    SDL_Color* game_colors = game_palette->colors;
    SDL_Color temp_color = game_colors[end];

    for (i=end;i>start;i--)
        game_colors[i] = game_colors[i-1];

    game_colors[start] = temp_color;
    update_yuv_palette();
}

int image_new(int w, int h){
    int index = findfreeimage();
    if (index == -1) return -1;

    imageList[index] = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);

    SDL_SetColorKey(imageList[index], SDL_TRUE, 255);
    SDL_FillRect(imageList[index], NULL, 255);

    imageList[index]->userdata = (void*)imageList[index]->format->palette;
    imageList[index]->format->palette = game_palette;

    return index;
}
int image_load(const char* filename, bool getPalette){
    int index, i;
    SDL_Palette* pal;

    index = findfreeimage();
    if (index==-1){
        printf("Error - Got index -1");
        return -1;
    }

    imageList[index] = SDL_LoadBMP(filename);
    if(imageList[index]==NULL){
        printf("PD     - Error loading %s\n",filename);
        return -1;
    }
    if(imageList[index]->format->BitsPerPixel != 8){
        printf("PD     - Failed to open %s, just 8bpp indexed images!\n",filename);
        image_delete(index);
        return -1;
    }
    SDL_SetColorKey(imageList[index], SDL_TRUE, 255);

    if(getPalette){
        pal = imageList[index]->format->palette;
        for(i=0;i<256;i++)
            game_palette->colors[i] = pal->colors[i];
        update_yuv_palette();
    }

    imageList[index]->userdata = (void*)imageList[index]->format->palette; //Put allocated pallete in userdata
    imageList[index]->format->palette = game_palette;

    return index;
}
int image_load(int& index, const char* filename, bool getPalette){
    
    image_delete(index);
    index = image_load(filename, getPalette);

    return index;

}

int image_copy(int src_i, int dst_i){
    if(src_i < 0 || dst_i < 0) return -1;
    SDL_FillRect(imageList[dst_i], NULL, 255);
    SDL_BlitSurface(imageList[src_i], NULL, imageList[dst_i], NULL);
    return 0;
}
int image_cut(int ImgIndex, int x, int y, int w, int h){ //Create a new image from a existing image
    RECT area;
    area.x = x; area.y = y;
    area.w = (w <= 0) ? imageList[ImgIndex]->w : w; //If 0 get the entire image
    area.h = (h <= 0) ? imageList[ImgIndex]->h : h;
    return image_cut(ImgIndex, area);
}
int image_cut(int ImgIndex, RECT area){
    int index;

    index = findfreeimage();
    if (index==-1){
        printf("PD     - PisteDraw has run out of free images!");
        return -1;
    }

    imageList[index] = SDL_CreateRGBSurface(0, area.w, area.h, 8, 0, 0, 0, 0);

    SDL_SetColorKey(imageList[index], SDL_TRUE, 255);
    SDL_FillRect(imageList[index], NULL, 255);

    imageList[index]->userdata = (void*)imageList[index]->format->palette;
    imageList[index]->format->palette = game_palette;

    SDL_BlitScaled(imageList[ImgIndex], (SDL_Rect*)&area, imageList[index], NULL);

    return index;
}
int image_clip(int index, int x, int y){
    SDL_Rect dstrect;

    dstrect.x = x + x_offset;
    dstrect.y = y;

    SDL_BlitSurface(imageList[index], NULL, frameBuffer8, &dstrect);

    return 0;
}
int image_cliptransparent(int index, int x, int y, int alpha){
    RECT srcrect, dstrect;
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.w = imageList[index]->w;
    srcrect.h = imageList[index]->h;
    dstrect.x = x;
    dstrect.y = y;
    return image_cutcliptransparent(index, srcrect, dstrect, alpha);
}
int image_cutclip(int index, int dstx, int dsty, int srcx, int srcy, int oikea, int ala){ //TODO - fix names
    RECT src = {(DWORD)srcx, (DWORD)srcy, (DWORD)oikea-srcx, (DWORD)ala-srcy};
    RECT dst = {(DWORD)dstx, (DWORD)dsty, (DWORD)oikea-srcx, (DWORD)ala-srcy};
    image_cutclip(index, src, dst);
    return 0;
}
int image_cutclip(int index, RECT srcrect, RECT dstrect){
    dstrect.x += x_offset;
    SDL_BlitSurface(imageList[index], (SDL_Rect*)&srcrect, frameBuffer8, (SDL_Rect*)&dstrect);
    return 0;
}
int image_cutcliptransparent(int index, RECT srcrect, RECT dstrect, int alpha){
    return image_cutcliptransparent(index, srcrect, dstrect, alpha, 0); //TODO implement
}

int image_cutcliptransparent(int index, RECT src, RECT dst, int alpha, int colorsum){
	return image_cutcliptransparent(index, src.x, src.y, src.w, src.h,
	    dst.x, dst.y, alpha, colorsum);
}

int image_cutcliptransparent(int index, DWORD src_x, DWORD src_y, DWORD src_w, DWORD src_h,
						 DWORD dst_x, DWORD dst_y, int alpha, BYTE colorsum){
    BYTE *imagePix = nullptr;
    BYTE *screenPix = nullptr;
    BYTE color1, color2;
    DWORD imagePitch, screenPitch;
    int posx, posy;

    int x_start = dst_x + x_offset;
    int x_end = dst_x + src_w;
    int y_start = dst_y;
    int y_end = dst_y + src_h;

    if (alpha > 100) alpha = 100;
    if (alpha < 0) alpha = 0;
    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_end > screen_width) x_end = screen_width;
    if (y_end > screen_height) y_end = screen_height;
    if (x_start > x_end || y_start > y_end) return -1;
    
    drawimage_start(index, *&imagePix, (DWORD &)imagePitch);
    drawscreen_start(*&screenPix, (DWORD &)screenPitch);
    for (posx = x_start; posx < x_end; posx++)
        for (posy = y_start; posy < y_end; posy++) {
            color1 = imagePix[(posx-x_start+src_x)+imagePitch*(posy-y_start+src_y)];
            if (color1 != 255) {
                color2 = screenPix[posx+screenPitch*posy];
                screenPix[posx+screenPitch*posy] = blend_colors(color1, color2, alpha) + colorsum;
            }
        }
    drawscreen_end();
    drawimage_end(index);
    return 0;
}
void image_getsize(int index, int& w, int& h){
    w = imageList[index]->w;
    h = imageList[index]->h;
}
int image_fliphori(int index){
    int i, h, w, p;
    BYTE* pix_array;

    if(index < 0) return -1;

    h = imageList[index]->h;
    w = imageList[index]->w;
    p = imageList[index]->pitch;

    SDL_LockSurface(imageList[index]);

    pix_array  = (BYTE*)(imageList[index]->pixels);

    for(i=0; i<h*p; i+=p)
        std::reverse(&pix_array[i],&pix_array[i + w]);

    SDL_UnlockSurface(imageList[index]);
    return 0;
}
int image_snapshot(int index){
    SDL_FillRect(imageList[index], NULL, 0);
    return SDL_BlitSurface(frameBuffer8, NULL, imageList[index], NULL);
}
int image_delete(int& index){
    if(index < 0) return -1;
    if (imageList[index] == NULL) return -1;
    imageList[index]->format->palette = (SDL_Palette*)imageList[index]->userdata; //Return to the original pallete
    SDL_FreeSurface(imageList[index]);
    imageList[index] = NULL;
    index = -1;
    return 0;
}

int image_fill(int index, BYTE color){
    return image_fill(index, 0, 0, imageList[index]->w, imageList[index]->h, color);
}
int image_fill(int index, int posx, int posy, int oikea, int ala, BYTE color){
    SDL_Rect r = {posx, posy, oikea-posx, ala-posy};
    return SDL_FillRect(imageList[index], &r, color);
}
int screen_fill(BYTE color){
    return SDL_FillRect(frameBuffer8, NULL, color);
}
int screen_fill(int posx, int posy, int oikea, int ala, BYTE color){
    SDL_Rect r = {posx + x_offset, posy, oikea-posx, ala-posy};
    return SDL_FillRect(frameBuffer8, &r, color);
}
void set_mask(int x, int y, int w, int h){
    SDL_Rect r = {x, y, w, h};
    SDL_SetClipRect(frameBuffer8, &r);
}

int drawscreen_start(BYTE* &pixels, DWORD &pitch){
    pixels = (BYTE*)frameBuffer8->pixels;
    pitch = frameBuffer8->pitch;
    return SDL_LockSurface(frameBuffer8);
}
int drawscreen_end(){
    SDL_UnlockSurface(frameBuffer8);
    return 0;
}
int drawimage_start(int index, BYTE* &pixels, DWORD &pitch){
    pixels = (BYTE*)imageList[index]->pixels;
    pitch = imageList[index]->pitch;
    return SDL_LockSurface(imageList[index]);
}
int drawimage_end(int index){
    SDL_UnlockSurface(imageList[index]);
    return 0;
}
BYTE blend_colors(BYTE color, BYTE colBack, int alpha){
    int result;

    if(alpha>100) alpha = 100;
    if(alpha<0) alpha = 0;

    result = color%32;
    result = (result*alpha)/100;
    result += colBack%32;
    if(result>31) result = 31;

    return (BYTE)result;//+32*col
}

int font_create(int image, int x, int y, int char_w, int char_h, int count){
    int index;

    index = findfreefont();
    if (index==-1) {
        printf("PD    - PisteDraw has run out of free fonts!");
        return -1;
    }

  fontList[index] = new PFont(image, x, y, char_w, char_h, count);
  return index;
}
int font_create(char* path, char* file){
    printf("PD     - Creating font with path %s e file %s\n",path,file);
    int index;

    index = findfreefont();
    if (index == -1){
        printf("PD     - PisteDraw has run out of free fonts!");
        return -1;
    }

    fontList[index] = new PFont();
    if (fontList[index]->load(path,file) == -1){
        printf("PD     - PisteDraw can't load a font from file!\n");
        delete fontList[index];
        return -1;
    }

  return index;
}
int font_write(int font_index, const char* text, int x, int y){
    if (font_index < 0) return 1;
    return fontList[font_index]->write(x, y, text);
}
int font_writealpha(int font_index, const char* text, int x, int y, BYTE alpha){
    return fontList[font_index]->write_trasparent(x + x_offset, y, text, alpha);
}

int set_filter(const char* filter){
    if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,filter) == SDL_TRUE)
        return 0;

    return 1;
}
void set_fullscreen(bool set){
    #ifndef __ANDROID__
    if(set)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    else {
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, screen_width, screen_height);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED);
        //TODO - adjust dst_rect too and turn off filters
    }

    adjust_screen();
    #endif
}
void adjust_screen(){
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    float screen_prop = (float)w / h;
    float buff_prop   = (float)screen_width / screen_height;
    if (buff_prop > screen_prop) {
        screen_dest.w = w;
        screen_dest.h = (int)(h / buff_prop);
        screen_dest.x = 0;
        screen_dest.y = (int)((h - screen_dest.h) / 2);
    }
    else {
        screen_dest.w = (int)(buff_prop * h);
        screen_dest.h = h;
        screen_dest.x = (int)((w - screen_dest.w) / 2);
        screen_dest.y = 0;
    }
}
void fit_screen(bool fit){
    screen_fit = fit;
}
void change_resolution(int w, int h) {
    if (w == screen_width && h == screen_height)
        return;

    frameBuffer8->format->palette = (SDL_Palette *)frameBuffer8->userdata;
    SDL_FreeSurface(frameBuffer8);

    
    frameBuffer8 = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);
    frameBuffer8->userdata = (void *)frameBuffer8->format->palette;
    frameBuffer8->format->palette = game_palette;
    SDL_SetColorKey(frameBuffer8, SDL_TRUE, 255);
    SDL_FillRect(frameBuffer8, NULL, 255);

    SDL_Rect r = {0, 0, w, h};
    SDL_SetClipRect(frameBuffer8, &r);

    screen_width = w;
    screen_height = h;
    SDL_SetWindowSize(window, w, h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED);
    
    adjust_screen();
    SDL_RenderClear(renderer);

}

void get_windowposition(int* x, int* y) {
    SDL_GetWindowPosition(window, x, y);
}

int get_xoffset() {
    return x_offset;
}
void set_xoffset(int x) {
    x_offset = x;
}

int init(int width, int height, const char* name, const char* icon) {
    if (ready) return -1;

    if (game_palette == NULL) {
        game_palette = SDL_AllocPalette(256);
        for(int i = 0; i < 256; i++)
            game_palette->colors[i] = {(Uint8)i,(Uint8)i,(Uint8)i,(Uint8)i};
    }

    if (game_palette_yuv == NULL) {
        game_palette_yuv = new uint32_t[256];
        update_yuv_palette();
    }

    window_name = name;
    #ifdef __ANDROID__

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    auto Width = DM.w;
    auto Height = DM.h;
    auto x = Height;
    if(Width < Height){
        Height = Width;
        Width = x;
    }

    window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_SHOWN);

    #else

    window = SDL_CreateWindow(name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);

    window_icon = SDL_LoadBMP(icon);
    SDL_SetWindowIcon(window, window_icon);

    #endif

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderClear(renderer);

    frameBuffer8 = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    frameBuffer8->userdata = (void *)frameBuffer8->format->palette;
    frameBuffer8->format->palette = game_palette;
    SDL_SetColorKey(frameBuffer8, SDL_TRUE, 255);
    SDL_FillRect(frameBuffer8, NULL, 255);

    PGui::init(width, height, renderer);

    SDL_Rect r = {0, 0, width, height};
    SDL_SetClipRect(frameBuffer8, &r);

    screen_width = width;
    screen_height = height;
    adjust_screen();

    ready = true;
    return 0;
}
void clear_fonts() {
    int size = fontList.size();

    for (int i = 0; i < size; i++) {
        if (fontList[i] != nullptr)
            delete fontList[i];
        fontList[i] = NULL;
    }
}
int terminate(){
    if (!ready) return -1;

    int i, j;

    int size = imageList.size();

    for (i = 0; i < size; i++)
        if (imageList[i] != NULL) {
            j = i;
            image_delete(j);
        }

    clear_fonts();

    SDL_FreePalette(game_palette);
    delete game_palette_yuv;

    frameBuffer8->format->palette = (SDL_Palette *)frameBuffer8->userdata;
    SDL_FreeSurface(frameBuffer8);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    if(window_icon != NULL) SDL_FreeSurface(window_icon);

    ready = false;
    return 0;
}

//SDL_Surface* hqx_dst_s = SDL_CreateRGBSurfaceWithFormat(0, 800, 480, 32, SDL_PIXELFORMAT_RGBA32);
uint8_t* hqx_src8 = (uint8_t*)malloc(800 * 480 * 2);
uint32_t* hqx_doneb = (uint32_t*)malloc(800 * 480 * 4 * sizeof(uint32_t));
uint32_t* hqx_dst2 = (uint32_t*)malloc(800 * 480 * 4 * sizeof(uint32_t));
bool hqx_done = true;

void draw_thread_f() {

    while(1) {

        while(hqx_done);

        int w = frameBuffer8->w;
        int h = frameBuffer8->h;
        
        int s_pitch = frameBuffer8->pitch;
        int d_pitch = 2 * w * sizeof(uint32_t);
        
        hq2x_32( hqx_src8, s_pitch, hqx_dst2, d_pitch, game_palette_yuv, w, h);
        memcpy(hqx_doneb, hqx_dst2, 800 * 480 * 4 * sizeof(uint32_t));
        hqx_done = true;
    }
}

std::thread draw_thread(draw_thread_f);

void update(bool draw){
    if(!ready) return;

    if(draw){
        SDL_Texture* texture;
        BYTE alpha2 = (BYTE)(alpha*255/100);

        if (PisteInput_Keydown(PI_H)) {
            int w = frameBuffer8->w;
            int h = frameBuffer8->h;
            int d_pitch = 2 * w * sizeof(uint32_t);
            memcpy(hqx_src8, frameBuffer8->pixels, frameBuffer8->pitch * h);
            SDL_Surface* s = SDL_CreateRGBSurfaceWithFormatFrom(hqx_doneb, w*2, h*2, 32, d_pitch, SDL_PIXELFORMAT_RGB888);
            texture = SDL_CreateTextureFromSurface(renderer, s);
            SDL_FreeSurface(s);

            hqx_done = false;

            //free(hqx_dst);

        } else {
            texture = SDL_CreateTextureFromSurface(renderer,frameBuffer8);
        }
        
        SDL_SetTextureColorMod(texture,alpha2,alpha2,alpha2);

        SDL_RenderClear(renderer);

        if(screen_fit)
            SDL_RenderCopy(renderer, texture, NULL, NULL);
        else
            SDL_RenderCopy(renderer, texture, NULL, &screen_dest);

        PGui::draw(alpha2);

        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(texture);
    }

    if (is_fading()){
        alpha += fade_speed;
        if(alpha < 0) alpha = 0;
        if(alpha > 255) alpha = 255;
    }

    SDL_Rect r = {0, 0, x_offset, screen_height}; // Fill the unused borders
    SDL_FillRect(frameBuffer8, &r, 0);
    r.x = screen_width - x_offset;
    SDL_FillRect(frameBuffer8, &r, 0);
}


}