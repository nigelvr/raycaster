#include <vector>
#include <iostream>
#include <tuple>
#include <cmath>
#include <SDL2/SDL.h>

using namespace std;

#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define RED 0xFF0000FF
#define WHITE 0xFFFFFFFF
#define GREY 0x00808080
#define BLUE 0x000047AB
#define CYAN 0xFF00FFFF
#define GREEN 0x0000A36C

const int box_width = 26;
const int box_height = 20;
const int wall_height = 200;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *map_tex;
uint32_t *map_pixels;
int pitch;

double playerX=140;
double playerY=20;
double playerDirX=-1;
double playerDirY=0;
double rotSpeed=0.04;
double walkSpeed = 1;
double player_angle=0;//M_PI/2;
int player_speed=4;
SDL_Texture *player_tex;
uint32_t *player_pixels;

SDL_Texture *fov_tex;
uint32_t *fov_pixels;
vector<pair<pair<int,int>, double>> fov;

enum Mode {
    MAP,
    FOV
};

Mode mode = MAP;

const int world_map[MAP_WIDTH][MAP_HEIGHT] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,4,4,4,4,3,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

void draw_pixel(int x, int y, uint32_t *pixbuf, uint32_t color) {
    pixbuf[y*(pitch/sizeof(uint32_t))+x] = color;
}

void fill_rect(int x, int y, int width, int height, uint32_t color, uint32_t *target, int pad=1) {
    for (int i = x+pad; i < x+width-1; i++) {
        for (int j = y+pad; j < y+height-1; j++) {
            draw_pixel(i, j, target, color);
        }
    }
}

int cell_value(int x, int y) {
    return world_map[x/box_width][y/box_height];
}

uint32_t color_map(int k) {
    switch (k) {
        case 1:
            return GREY;
        case 2:
            return RED;
        case 3:
            return BLUE;
        case 4:
            return CYAN;
    }
    return GREEN;
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t *pixbuf, uint32_t color=RED) {
    int length = (int)sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx > dy ? dx : -dy;
    int last_x=x0, last_y=y0;
    int orig_x = x0, orig_y = y0;
    while (x0 != x1 || y0 != y1) {
        if (x0 <= 0 || x0 >= SCREEN_WIDTH || y0 <= 0 || y0 >= SCREEN_HEIGHT) {
            break;
        }
        if (length > 0 && (orig_x-x0)*(orig_x-x0) + (orig_y-y0)*(orig_y-y0) >= length*length) {
            break;
        }
        
        draw_pixel(x0, y0, pixbuf, color);
        
        if (err > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (err < dy) {
            err += dx;
            y0 += sy;
        }
        last_x = x0;
        last_y = y0;
    }
}

void draw_2d_map() {
    SDL_LockTexture(map_tex, NULL, (void **)&map_pixels, &pitch);

    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int y=0; y < SCREEN_HEIGHT; y++) {
            draw_pixel(i*box_width, y, map_pixels, WHITE);
        }
        for (int j = 0; j < MAP_HEIGHT; j++) {
            for (int x=0; x < SCREEN_WIDTH; x++) {
                draw_pixel(x, j*box_height, map_pixels, WHITE);
            }
            if (world_map[i][j]) {
                fill_rect(i*box_width, j*box_height, box_width, box_height, color_map(world_map[i][j]), map_pixels);
            }
        }
    }
    
    SDL_UnlockTexture(map_tex);
}

// returns true if time to quit
// otherwise, processes the event and returns false
bool pollAndHandleEvent() {
    SDL_Event event;
    int x,y;
    if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
        return true;
    }
    double rayDirX = sin(player_angle);
    double rayDirY = cos(player_angle);
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                return true;
            case SDLK_SPACE:
                if (mode == FOV) {
                    mode = MAP;
                } else {
                    mode = FOV;
                }
                break;
            case SDLK_DOWN:
                // x = floor(playerX - sin(player_angle)*player_speed);
                // y = floor(playerY + cos(player_angle)*player_speed);
                x = floor(playerX - rayDirX*player_speed);
                y = floor(playerY - rayDirY*player_speed);

                if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT
                    || world_map[x/box_width][y/box_height]) {
                    return false;
                }

                playerX = x;
                playerY = y;
                break;
            case SDLK_UP:
                x = floor(playerX + rayDirX*player_speed);
                y = floor(playerY + rayDirY*player_speed);

                if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT
                    || world_map[x/box_width][y/box_height]) {
                    return false;
                }

                playerX = x;
                playerY = y;
                break;
            case SDLK_LEFT:
                player_angle = fmod(player_angle+0.01*player_speed, 2*M_PI);
                break;
            case SDLK_RIGHT:
                player_angle = fmod(player_angle-0.01*player_speed, 2*M_PI);
                break;
        }
    }
    return false;
}

void draw_player() {
    SDL_LockTexture(player_tex, NULL, (void **)&player_pixels, &pitch);
    memset(player_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint32_t));
    fill_rect(playerX, playerY, 10, 10, RED, player_pixels);
    // draw_line(playerX, playerY, floor(playerX + 30*sin(player_angle)), floor(playerY - 30*cos(player_angle)), player_pixels, RED);
    double rayDirX = sin(player_angle);
    double rayDirY = cos(player_angle);

    draw_line(playerX, playerY, floor(playerX + 30*rayDirX), floor(playerY + 30*rayDirY), player_pixels, RED);
    SDL_UnlockTexture(player_tex);
}

void draw_fov() {
    SDL_LockTexture(fov_tex, NULL, (void **)&fov_pixels, &pitch);
    memset(fov_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint32_t));
    int hit, stepX, stepY, side;
    double sideDistX, sideDistY;
    double px = playerX/(double)box_width;
    double py = playerY/(double)box_height;
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        double cameraX = 2*(x/(double)SCREEN_WIDTH) - 1;
        double dirX = sin(player_angle);
        double dirY = cos(player_angle);
        double planeX = -cos(player_angle);
        double planeY = sin(player_angle);

        double rayDirX = dirX + planeX * cameraX;
        double rayDirY = dirY + planeY * cameraX;

        int mapX = int(px);
        int mapY = int(py);

        printf("At %d %d - ", mapX, mapY);

        double deltaDistX = (rayDirX == 0) ? 1e30 : abs(1/rayDirX);
        double deltaDistY = (rayDirY == 0) ? 1e30 : abs(1/rayDirY);
        double perpWallDist;

        hit = 0;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (px - mapX)*deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1 - px) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (py - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1 - py) * deltaDistY;
        }

        while (!hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (world_map[mapX][mapY]) {
                hit=1;
                break;
            }
        }

        printf("hit %d %d\n", mapX, mapY);

        if (!side) {
            perpWallDist = sideDistX - deltaDistX;
        } else {
            perpWallDist = sideDistY - deltaDistY;
        }

        int lineHeight = (int)(SCREEN_HEIGHT/perpWallDist);
        // calculate lowest and highest pixel to fill in current stripe
        int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2 + 80;
        if (drawStart < 0) {
            drawStart = 0;
        }
        int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2 + 80;
        if (drawEnd >= SCREEN_HEIGHT) {
            drawEnd = SCREEN_HEIGHT - 1;
        }

        // vertical line
        for (int y = drawStart; y <= drawEnd; y++) {
            draw_pixel(x, y, fov_pixels, color_map(world_map[mapX][mapY]));
        }
    }
    SDL_UnlockTexture(fov_tex);
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("sdl app", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    map_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    player_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    fov_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(map_tex, SDL_BLENDMODE_NONE);
    SDL_SetTextureBlendMode(player_tex, SDL_BLENDMODE_BLEND);

    SDL_Event event;
    bool quit = false;
    while (1) {
       quit = pollAndHandleEvent();
        if (quit) {
            break;
        }

        draw_2d_map();
        draw_player();
        draw_fov();

        SDL_RenderClear(renderer);

        if (mode == MAP) {
            SDL_RenderCopy(renderer, map_tex, NULL, NULL);
            SDL_RenderCopy(renderer, player_tex, NULL, NULL);
        } else {
            SDL_RenderCopy(renderer, fov_tex, NULL, NULL);
        }
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60FPS
    
    }

    return 0;
}