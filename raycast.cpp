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
#define BLUE 0xFF0000FF
#define CYAN 0xFF00FFFF
#define GREEN 0x0000A36C

// Game config
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define BOX_WIDTH 26
#define BOX_HEIGHT 20

// Game globals
SDL_Texture *map_tex;
uint32_t *map_pixels;
int pitch;

// More game globals
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
  {1,0,0,0,0,4,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,2,2,4,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

double myMin(double x, double y) {
    return x >= y ? y : x;
}

double myMax(double x, double y) {
    return x >= y ? x : y;
}

class vec2 {
public:
    double x,y;
    vec2(double x0, double y0) {
        x = x0;
        y = y0;
    }

    vec2() {}

    int dist_squared(vec2 u) {
        return (x-u.x)*(x-u.x) + (y-u.y)*(y-u.y);
    }

    vec2 round() {
        return vec2(floor(x), floor(y));
    }

    vec2 max(vec2 u) {
        return vec2(myMax(x, u.x), myMax(y, u.y));
    }

    vec2 min(vec2 u) {
        return vec2(myMin(x, u.x), myMin(y, u.y));
    }
};

vec2 operator+(vec2 a, vec2 b) {
    return vec2(a.x+b.x, a.y+b.y);
}

vec2 operator-(vec2 a, vec2 b) {
    return vec2(a.x-b.x, a.y-b.y);
}

vec2 operator*(double scalar, vec2 a) {
    return vec2(scalar*a.x, scalar*a.y);
}

vec2 operator/(vec2 a, double scalar) {
    return vec2(a.x/scalar, a.y/scalar);
}

bool operator==(vec2 a, vec2 b) {
    return a.x == b.x && a.y == b.y;
}

bool operator!=(vec2 a, vec2 b) {
    return !(a==b);
}

vec2 playerPos(140,20);

void draw_pixel(vec2 v, uint32_t *pixbuf, uint32_t color) {
    int x = floor(v.x);
    int y = floor(v.y);
    pixbuf[y*(pitch/sizeof(uint32_t))+x] = color;
}

void draw_pixel_with_pitch(vec2 v, uint32_t *pixbuf, uint32_t color, int p) {
    int x = floor(v.x);
    int y = floor(v.y);
    pixbuf[y*(p/sizeof(uint32_t))+x] = color;
}

void fill_rect(vec2 v, int width, int height, uint32_t color, uint32_t *target, int pad=1) {
    for (int i = v.x+pad; i < v.x+width-1; i++) {
        for (int j = v.y+pad; j < v.y+height-1; j++) {
            draw_pixel(vec2(i,j), target, color);
        }
    }
}

int cell_value(vec2 v) {
    int x = floor(v.x);
    int y = floor(v.y);
    return world_map[x/BOX_WIDTH][y/BOX_HEIGHT];
}

uint32_t colorMap(int k) {
    switch (k) {
        case 1:
            return GREY; // ok
        case 2:
            return RED; // OK
        case 3:
            return BLUE;
        case 4:
            return CYAN;
    }
    return GREEN;
}

void draw_line(vec2 u, vec2 v, uint32_t *pixbuf, uint32_t color=RED, int p=pitch) {
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
        
        draw_pixel_with_pitch(u, pixbuf, color, p);
        
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
            draw_pixel(vec2(i*BOX_WIDTH, y), map_pixels, WHITE);
        }
        for (int j = 0; j < MAP_HEIGHT; j++) {
            for (int x=0; x < SCREEN_WIDTH; x++) {
                draw_pixel(vec2(x, j*BOX_HEIGHT), map_pixels, WHITE);
            }
            if (world_map[i][j]) {
                fill_rect(vec2(i*BOX_WIDTH, j*BOX_HEIGHT), BOX_WIDTH, BOX_HEIGHT, colorMap(world_map[i][j]), map_pixels);
            }
        }
    }
    
    SDL_UnlockTexture(map_tex);
}

vec2 computeRayDir() {
    return vec2(sin(player_angle), cos(player_angle));
}

// returns true if time to quit
// otherwise, processes the event and returns false
bool pollAndHandleEvent() {
    SDL_Event event;
    int x,y;
    if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
        return true;
    }
    vec2 rayDir = computeRayDir();
    vec2 tmpVec(-1,-1);
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
                tmpVec = vec2(0,0).max(playerPos - player_speed*rayDir);
                goto update;
            case SDLK_UP:
                tmpVec = vec2(SCREEN_WIDTH,SCREEN_HEIGHT).min(playerPos + player_speed*rayDir);
                goto update;
            case SDLK_LEFT:
                player_angle = fmod(player_angle+0.01*player_speed, 2*M_PI);
                break;
            case SDLK_RIGHT:
                player_angle = fmod(player_angle-0.01*player_speed, 2*M_PI);
                break;
        }
    }
update:
    if (tmpVec != playerPos) {
        if (world_map[(int)tmpVec.x/BOX_WIDTH][(int)tmpVec.y/BOX_HEIGHT]) {
            goto done;
        } else {
            playerPos = tmpVec;
        }
    }
done:
    return false;
}

void draw_player() {
    int player_pitch;
    SDL_LockTexture(player_tex, NULL, (void **)&player_pixels, &player_pitch);
    memset(player_pixels, 0, 40*40*sizeof(uint32_t));
    for (int i = 20; i < 30; i++) {
        for (int j = 20; j < 30; j++) {
            draw_pixel_with_pitch(vec2(i,j), player_pixels, RED, player_pitch);
        }
    }
    vec2 rayDir = computeRayDir();
    draw_line(vec2(20,20), (vec2(20,20) + 20*rayDir).round(), player_pixels, RED, player_pitch);
    SDL_UnlockTexture(player_tex);
}

// Sends a ray from playerPos to cameraX*planeDir and returns the distance
// from the hit wall
void dda(double cameraX, double *perpWallDist, int *rMapX, int *rMapY) {
    double sideDistX, sideDistY;
    double px = playerPos.x/(double)BOX_WIDTH;
    double py = playerPos.y/(double)BOX_HEIGHT;
    vec2 rayDir = vec2(sin(player_angle), cos(player_angle)) + cameraX*vec2(-cos(player_angle), sin(player_angle));
    int mapX = int(px);
    int mapY = int(py);
    double deltaDistX = (rayDir.x == 0) ? 1e30 : abs(1/rayDir.x);
    double deltaDistY = (rayDir.y == 0) ? 1e30 : abs(1/rayDir.y);
    int hit, stepX, stepY, side;

    hit = 0;
    if (rayDir.x < 0) {
        stepX = -1;
        sideDistX = (px - mapX)*deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1 - px) * deltaDistX;
    }

    if (rayDir.y < 0) {
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

    if (!side) {
        *perpWallDist = sideDistX - deltaDistX; 
    } else {
        *perpWallDist = sideDistY - deltaDistY;
    }
    *rMapX = mapX;
    *rMapY = mapY;
}

void draw_fov() {
    SDL_LockTexture(fov_tex, NULL, (void **)&fov_pixels, &pitch);
    memset(fov_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint32_t));
    int mapX, mapY;
    double cameraX, perpWallDist;
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        cameraX = 2*(x/(double)SCREEN_WIDTH) - 1;

        dda(cameraX, &perpWallDist, &mapX, &mapY);

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
        int pad=0;
        if (world_map[mapX][mapY] == 1) {
            pad=20;
        }
        for (int y = drawStart-pad; y <= drawEnd; y++) {
            draw_pixel(vec2(x, y), fov_pixels, colorMap(world_map[mapX][mapY]));
        }
    }
    SDL_UnlockTexture(fov_tex);
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("sdl app", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    map_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    player_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 40, 40);
    fov_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(map_tex, SDL_BLENDMODE_NONE);
    SDL_SetTextureBlendMode(player_tex, SDL_BLENDMODE_BLEND);

    draw_2d_map();

    SDL_Event event;
    bool quit = false;
    SDL_Rect r;
    r.w = 40;
    r.h = 40;
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
            r.x = floor(playerPos.x);
            r.y = floor(playerPos.y);
            SDL_RenderCopy(renderer, player_tex, NULL, &r);
        } else {
            SDL_RenderCopy(renderer, fov_tex, NULL, NULL);
        }
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60FPS
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(map_tex);
    SDL_DestroyTexture(player_tex);
    SDL_DestroyTexture(fov_tex);

    return 0;
}