#include <vector>
#include <iostream>
#include <tuple>
#include <cmath>
#include <SDL2/SDL.h>

using namespace std;

// Colors
#define RED 0xFF0000FF
#define WHITE 0xFFFFFFFF
#define GREY 0x00808080
#define BLUE 0x000047AB
#define CYAN 0xFF00FFFF
#define GREEN 0x0000A36C

// Game config
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAP_WIDTH 24
#define MAP_HEIGHT 24
const int box_width = 26;
const int box_height = 20;
const int wall_height = 200;

// Game globals
SDL_Texture *map_tex;
uint32_t *map_pixels;
int pitch;

// More game globals
double playerX=140;
double playerY=20;
double playerDirX=-1;
double playerDirY=0;
double rotSpeed=0.04;
double walkSpeed = 1;
double player_angle=0;
int player_speed=4;
SDL_Texture *player_tex;
uint32_t *player_pixels;

// Even more
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
  {1,0,0,0,0,3,0,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
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

template <class T>
class vec2 {
public:
    T x,y;
    vec2(T x0, T y0) {
        x = x0;
        y = y0;
    }

    int dist_squared(vec2 u) {
        return (x-u.x)*(x-u.x) + (y-u.y)*(y-u.y);
    }

    vec2 roundDown() {
        return vec2(floor(x), floor(y));
    }

};

vec2<double> playerPos(140,20);

void draw_pixel(vec2<int> v, uint32_t *pixbuf, uint32_t color) {
    pixbuf[v.y*(pitch/sizeof(uint32_t))+v.x] = color;
}

void fill_rect(vec2<int> v, int width, int height, uint32_t color, uint32_t *target, int pad=1) {
    for (int i = v.x+pad; i < v.x+width-1; i++) {
        for (int j = v.y+pad; j < v.y+height-1; j++) {
            draw_pixel(vec2(i,j), target, color);
        }
    }
}

int cell_value(vec2<int> v) {
    return world_map[v.x/box_width][v.y/box_height];
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

void draw_line(vec2<int> u, vec2<int> v, uint32_t *pixbuf, uint32_t color=RED) {
    int length_squared = (v.x-u.x)*(v.x-u.x) + (v.y-u.y)*(v.y-u.y);
    int dx = abs(v.x-u.x);
    int dy = abs(v.y-u.y);
    int sx = u.x < v.x ? 1 : -1;
    int sy = u.y < v.y ? 1 : -1;
    int err = dx > dy ? dx : -dy;
    vec2 orig_u = vec2(u.x, u.y);

    while (u.x != v.x || u.y != v.y) {
        if (u.x <= 0 || u.x >= SCREEN_WIDTH || u.y <= 0 || u.y >= SCREEN_HEIGHT) {
            break;
        }
        if (length_squared > 0 && orig_u.dist_squared(u) >= length_squared) {
            break;
        }
        
        draw_pixel(u, pixbuf, color);
        
        if (err > -dx) {
            err -= dy;
            u.x += sx;
        }
        if (err < dy) {
            err += dx;
            u.y += sy;
        }
    }
}

void draw_2d_map() {
    SDL_LockTexture(map_tex, NULL, (void **)&map_pixels, &pitch);

    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int y=0; y < SCREEN_HEIGHT; y++) {
            draw_pixel(vec2(i*box_width, y), map_pixels, WHITE);
        }
        for (int j = 0; j < MAP_HEIGHT; j++) {
            for (int x=0; x < SCREEN_WIDTH; x++) {
                draw_pixel(vec2(x, j*box_height), map_pixels, WHITE);
            }
            if (world_map[i][j]) {
                fill_rect(vec2(i*box_width, j*box_height), box_width, box_height, color_map(world_map[i][j]), map_pixels);
            }
        }
    }
    
    SDL_UnlockTexture(map_tex);
}

vec2<double> computeRayDir() {
    return vec2<double>(sin(player_angle), cos(player_angle));
}

double min(double x, double y) {
    return x >= y ? y : x;
}

double max(double x, double y) {
    return x >= y ? x : y;
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
                x = max(0, floor(playerPos.x - rayDirX*player_speed));
                y = max(floor(playerPos.y - rayDirY*player_speed), 0);

                if (world_map[x/box_width][y/box_height]) {
                    goto done;
                }

                playerPos.x = x;
                playerPos.y = y;
                break;
            case SDLK_UP:
                x = min(SCREEN_WIDTH, floor(playerPos.x + rayDirX*player_speed));
                y = min(SCREEN_HEIGHT, floor(playerPos.y + rayDirY*player_speed));

                if (world_map[x/box_width][y/box_height]) {
                    goto done;
                }

                playerPos.x = x;
                playerPos.y = y;
                break;
            case SDLK_LEFT:
                player_angle = fmod(player_angle+0.01*player_speed, 2*M_PI);
                break;
            case SDLK_RIGHT:
                player_angle = fmod(player_angle-0.01*player_speed, 2*M_PI);
                break;
        }
    }
done:
    return false;
}

void draw_player() {
    SDL_LockTexture(player_tex, NULL, (void **)&player_pixels, &pitch);
    memset(player_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint32_t));

    fill_rect(vec2<int>(floor(playerPos.x), floor(playerPos.y)), 10, 10, RED, player_pixels);
    
    double rayDirX = sin(player_angle);
    double rayDirY = cos(player_angle);

    draw_line(vec2<int>(floor(playerPos.x), floor(playerPos.y)), vec2<int>(floor(playerPos.x + 30*rayDirX), floor(playerPos.y + 30*rayDirY)), player_pixels, RED);
    SDL_UnlockTexture(player_tex);
}

void draw_fov() {
    SDL_LockTexture(fov_tex, NULL, (void **)&fov_pixels, &pitch);
    memset(fov_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint32_t));
    int hit, stepX, stepY, side;
    double sideDistX, sideDistY;
    double px = playerPos.x/(double)box_width;
    double py = playerPos.y/(double)box_height;
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
            draw_pixel(vec2(x, y), fov_pixels, color_map(world_map[mapX][mapY]));
        }
    }
    SDL_UnlockTexture(fov_tex);
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("sdl app", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    map_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    player_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    fov_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(map_tex, SDL_BLENDMODE_NONE);
    SDL_SetTextureBlendMode(player_tex, SDL_BLENDMODE_BLEND);

    draw_2d_map();

    SDL_Event event;
    bool quit = false;
    while (1) {
       quit = pollAndHandleEvent();
        if (quit) {
            break;
        }

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