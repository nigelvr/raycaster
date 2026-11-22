#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <tuple>
#include <cmath>
#include <algorithm>
#include <charconv>
#include <limits>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// using namespace std;

// Colors
#define RED 0xFF0000FF
#define WHITE 0xFFFFFFFF
#define GREY 0x00808080
#define CYAN  0xFF00FFFF
#define GREEN 0xFF00FF00
#define BLUE  0xFFFF0000

// Game config
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 480
#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define BOX_WIDTH 25
#define BOX_HEIGHT 20

constexpr double CAMERA_HEIGHT = 0.5;
constexpr double HORIZON_Y = SCREEN_HEIGHT / 2.0 + 80.0;

const double infinity = std::numeric_limits<double>::infinity();


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
  {1,0,0,0,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Even more

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

void draw_pixel(vec2 v, uint32_t *pixbuf, uint32_t color, int pitchBytes) {
    int x = floor(v.x);
    int y = floor(v.y);
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }
    pixbuf[y*(pitchBytes/sizeof(uint32_t))+x] = color;
}

void draw_line(vec2 u, vec2 v, uint32_t *pixbuf, int pitchBytes, uint32_t color=RED) {
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
        
        draw_pixel(u, pixbuf, color, pitchBytes);
        
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

class Player {
public:
    vec2 pos;
    double angle;
    Player(vec2 p, double a) : pos(p), angle(a) {}

    vec2 forwardDir() {
        return vec2(sin(angle), cos(angle));
    }

    void turnLeft() {
        angle = fmod(angle+0.01*player_speed, 2*M_PI);
    }

    void turnRight() {
        angle = fmod(angle-0.01*player_speed, 2*M_PI);
    }

    void moveForward() {
        vec2 rayDir = forwardDir();
        vec2 tmpVec = vec2(SCREEN_WIDTH,SCREEN_HEIGHT).min(pos + player_speed*rayDir);
        if (!world_map[(int)tmpVec.x/BOX_WIDTH][(int)tmpVec.y/BOX_HEIGHT]) {
            pos = tmpVec;
        }
    }

    void moveBackward() {
        vec2 rayDir = forwardDir();
        vec2 tmpVec = vec2(0,0).max(pos - player_speed*rayDir);
        if (!world_map[(int)tmpVec.x/BOX_WIDTH][(int)tmpVec.y/BOX_HEIGHT]) {
            pos = tmpVec;
        }
    }

    void strafeLeft() {
        vec2 rayDir = forwardDir();
        vec2 orth(-rayDir.y, rayDir.x);
        vec2 tmpVec = vec2(0,0).max(pos - player_speed*orth);
        if (!world_map[(int)tmpVec.x/BOX_WIDTH][(int)tmpVec.y/BOX_HEIGHT]) {
            pos = tmpVec;
        }
    }

    void strafeRight() {
        vec2 rayDir = forwardDir();
        vec2 orth(rayDir.y, -rayDir.x);
        vec2 tmpVec = vec2(0,0).max(pos - player_speed*orth);
        if (!world_map[(int)tmpVec.x/BOX_WIDTH][(int)tmpVec.y/BOX_HEIGHT]) {
            pos = tmpVec;
        }
    }
private:
    double rotSpeed=0.04;
    double walkSpeed = 1;
    int player_speed=4;
};

bool newdda(double cameraX, Player &player, double *perpWallDist, int *rMapX, int *rMapY, double *wallx, double *wally, int *side) {  
      
    double px = player.pos.x/(double)BOX_WIDTH;
    double py = player.pos.y/(double)BOX_HEIGHT;
    vec2 rayDir = vec2(sin(player.angle), cos(player.angle)) + cameraX*vec2(-cos(player.angle), sin(player.angle));
    int mapX = int(px);
    int mapY = int(py);
    if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
        return false;
    }
    double rayT = 0;

    // Our ray is represented by R(t) = (px,py) + t(rayDir.x, rayDir.y)

    // Initialize the ray's starting position in the map grid
    int stepX = 0, stepY = 0;
    double tdX = infinity, tdY = infinity; // time-distances to the nearest x and y grid lines
    if (rayDir.x > 0) {
        stepX = 1;
        tdX = (mapX + 1 - px)/rayDir.x;
    } else if (rayDir.x < 0) {
        stepX = -1;
        tdX = (px - mapX)/-rayDir.x;
    }
    if (rayDir.y > 0) {
        stepY = 1;
        tdY = (mapY + 1 - py)/rayDir.y;
    } else if (rayDir.y < 0) {
        stepY = -1;
        tdY = (py - mapY)/-rayDir.y;
    }

    while (1) {
        if (isinf(tdX) && isinf(tdY)) {
            return false;
        }
        if (tdX < tdY) {
            *side = 0;
            auto t = tdX;
            rayT += t;
            px = mapX + (stepX > 0 ? 1 : 0);
            py += t*rayDir.y;
            mapX += stepX;
        } else {
            *side = 1;
            auto t = tdY;
            rayT += t;
            px += t*rayDir.x;
            py = mapY + (stepY > 0 ? 1 : 0);
            mapY += stepY;
        }
        if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) {
            return {};
        }
        if (rayDir.x > 0) {
            tdX = (mapX + 1 - px)/rayDir.x;
        } else if (rayDir.x < 0) {
            tdX = (px - mapX)/-rayDir.x;
        } else {
            tdX = infinity;
        }
        if (rayDir.y > 0) {
            tdY = (mapY + 1 - py)/rayDir.y;
        } else if (rayDir.y < 0) {
            tdY = (py - mapY)/-rayDir.y;
        } else {
            tdY = infinity;
        }
        if (world_map[mapX][mapY]) {
            break;
        }
    }

    *rMapX = mapX;
    *rMapY = mapY;
    *perpWallDist = rayT;
    *wallx = px;
    *wally = py;
    return true;
}



enum Mode {
    MAP,
    FOV
};

class Renderer {
public:
    SDL_Renderer *renderer;
    SDL_Texture *fov_tex;
    uint32_t *fov_pixels;
    SDL_Surface *sky_surface;
    SDL_Surface *wall_surface;
    SDL_Surface *floor_surface;


    Renderer(SDL_Window *window) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        map_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        player_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 40, 40);
        SDL_SetTextureBlendMode(map_tex, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(player_tex, SDL_BLENDMODE_BLEND);
        fov_tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        wall_surface = loadSurface("wall.bmp");
        floor_surface = loadSurface("floor.bmp");
        sky_surface = loadSurface("sky.png");
    }

    ~Renderer() {
        SDL_DestroyTexture(map_tex);
        SDL_DestroyTexture(player_tex);
        SDL_DestroyTexture(fov_tex);
        SDL_FreeSurface(wall_surface);
        SDL_FreeSurface(floor_surface);
        SDL_FreeSurface(sky_surface);
        SDL_DestroyRenderer(renderer);
    }

    void renderMap(Player &player, Mode mode) {
        SDL_RenderClear(renderer);
        SDL_Rect r;
        r.w = 40;
        r.h = 40;
        if (mode == MAP) {
            SDL_RenderCopy(renderer, map_tex, NULL, NULL);
            r.x = floor(player.pos.x);
            r.y = floor(player.pos.y);
            SDL_RenderCopy(renderer, player_tex, NULL, &r);
        } else {
            SDL_RenderCopy(renderer, fov_tex, NULL, NULL);
        }
    }

    // only needs to be called once - the map background
    void draw_2d_map() {
        int mapPitch;
        SDL_LockTexture(map_tex, NULL, (void **)&map_pixels, &mapPitch);

        for (int i = 0; i < MAP_WIDTH; i++) {
            for (int y=0; y < SCREEN_HEIGHT; y++) {
                draw_pixel(vec2(i*BOX_WIDTH, y), map_pixels, WHITE, mapPitch);
            }
            for (int j = 0; j < MAP_HEIGHT; j++) {
                for (int x=0; x < SCREEN_WIDTH; x++) {
                    draw_pixel(vec2(x, j*BOX_HEIGHT), map_pixels, WHITE, mapPitch);
                }
                if (world_map[i][j]) {
                    int pad = 1;
                    auto v = vec2(i*BOX_WIDTH, j*BOX_HEIGHT);
                    for (int i = v.x+pad; i < v.x+BOX_WIDTH-1; i++) {
                        for (int j = v.y+pad; j < v.y+BOX_HEIGHT-1; j++) {
                            draw_pixel(vec2(i,j), map_pixels, GREY, mapPitch);
                        }
                    }
                }
            }
        }
        
        SDL_UnlockTexture(map_tex);
    }

    void draw_fov(Player &player) {
        int pitch;
        SDL_LockTexture(fov_tex, NULL, (void **)&fov_pixels, &pitch);
        memset(fov_pixels, 0, SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint32_t));

        bool wallSurfaceLocked = SDL_MUSTLOCK(wall_surface);
        if (wallSurfaceLocked && SDL_LockSurface(wall_surface) != 0) {
            std::cerr << "Could not lock wall: " << SDL_GetError() << std::endl;
            SDL_UnlockTexture(fov_tex);
            return;
        }
        bool skySurfaceLocked = SDL_MUSTLOCK(sky_surface);
        if (skySurfaceLocked && SDL_LockSurface(sky_surface) != 0) {
            std::cerr << "Could not lock sky.png: " << SDL_GetError() << std::endl;
            if (wallSurfaceLocked) {
                SDL_UnlockSurface(wall_surface);
            }
            SDL_UnlockTexture(fov_tex);
            return;
        }

        bool floorSurfaceLocked = SDL_MUSTLOCK(floor_surface);
        if (floorSurfaceLocked && SDL_LockSurface(floor_surface) != 0) {
            std::cerr << "Could not lock floor.bmp: " << SDL_GetError() << std::endl;
            if (wallSurfaceLocked) {
                SDL_UnlockSurface(wall_surface);
            }
            if (skySurfaceLocked) {
                SDL_UnlockSurface(sky_surface);
            }
            SDL_UnlockTexture(fov_tex);
            return;
        }

        int mapX, mapY;
        double cameraX, perpWallDist;
        const int horizon = SCREEN_HEIGHT / 2 + 80;
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            cameraX = 2*(x/(double)SCREEN_WIDTH) - 1;

            double wallx, wally;
            int side;
            bool hit = newdda(cameraX, player, &perpWallDist, &mapX, &mapY, &wallx, &wally, &side);
            int wallStart = horizon;
            int drawEnd = horizon;
            
            // Draw the sky
            // We choose not to draw the sky for now
            /*
            double skyU = (player.angle - atan(cameraX)) / (2 * M_PI);
            skyU -= floor(skyU);
            int skyTexX = std::min((int)(skyU * sky_surface->w), sky_surface->w - 1);
            for (int y = 0; y < wallStart; y++) {
                int skyTexY = y * sky_surface->h / horizon;
                auto *row = reinterpret_cast<Uint32 *>(
                    static_cast<Uint8 *>(sky_surface->pixels)
                    + skyTexY * sky_surface->pitch
                );
                Uint8 r, g, b, a;
                SDL_GetRGBA(row[skyTexX], sky_surface->format, &r, &g, &b, &a);
                uint32_t pixelColor = SDL_MapRGBA(sky_surface->format, r, g, b, a);
                draw_pixel(vec2(x, y), fov_pixels, pixelColor, pitch);
            } */
            
            if (hit) {
                const double maxLineHeight = 2.0 * SCREEN_HEIGHT;
                const double projectedHeight = perpWallDist > 0
                    ? SCREEN_HEIGHT / perpWallDist
                    : maxLineHeight;
                int lineHeight = (int)std::min(projectedHeight, maxLineHeight);
                wallStart = std::max(0, -lineHeight / 2 + horizon);
                drawEnd = std::min(SCREEN_HEIGHT - 1, lineHeight / 2 + horizon);
            }

            // draw the wall
            for (int y = wallStart; y <= drawEnd; y++) {
                double wallV = (double)(y - wallStart) / (drawEnd - wallStart);
                double wallU = side == 0
                ? wally - floor(wally)
                : wallx - floor(wallx);
                int wallTexX = std::clamp(
                    (int)(wallU * wall_surface->w),
                    0,
                    wall_surface->w - 1
                );
                int wallTexY = std::clamp(
                    (int)(wallV * wall_surface->h),
                    0,
                    wall_surface->h - 1
                );
                auto *row = reinterpret_cast<Uint32 *>(
                    static_cast<Uint8 *>(wall_surface->pixels)
                    + wallTexY * wall_surface->pitch
                );
                Uint8 r, g, b, a;
                SDL_GetRGBA(row[wallTexX], wall_surface->format, &r, &g, &b, &a);
                uint32_t pixelColor = SDL_MapRGBA(wall_surface->format, r, g, b, a);
                draw_pixel(vec2(x, y), fov_pixels, pixelColor, pitch);
            }

            // draw the floor
            vec2 forward = vec2(sin(player.angle), cos(player.angle));
            const vec2 cameraPlane(-forward.y, forward.x);
            for (int y = drawEnd + 1; y < SCREEN_HEIGHT; y++) {
                // distance from the wall to the floor
                double distance = (SCREEN_HEIGHT * 0.5) / (y - horizon);
                const vec2 rayDir = forward + cameraX * cameraPlane;
                double floorX = player.pos.x + distance * rayDir.x;
                double floorY = player.pos.y + distance * rayDir.y;

                int floorTexX = (int)(floorX * floor_surface->w) % floor_surface->w;
                int floorTexY = (int)(floorY * floor_surface->h) % floor_surface->h;

                auto *row = reinterpret_cast<Uint32 *>(
                    static_cast<Uint8 *>(floor_surface->pixels)
                    + floorTexY * floor_surface->pitch
                );
                Uint8 r, g, b, a;
                SDL_GetRGBA(row[floorTexX], floor_surface->format, &r, &g, &b, &a);
                uint32_t pixelColor = SDL_MapRGBA(floor_surface->format, r, g, b, a);
                draw_pixel(vec2(x, y), fov_pixels, pixelColor, pitch);
            }
        }
        if (skySurfaceLocked) {
            SDL_UnlockSurface(sky_surface);
        }
        if (wallSurfaceLocked) {
            SDL_UnlockSurface(wall_surface);
        }
        if (floorSurfaceLocked) {
            SDL_UnlockSurface(floor_surface);
        }
        SDL_UnlockTexture(fov_tex);
    }

    void drawPixels(Player &player) {
        draw_player(player);
    }


private:
    SDL_Texture *player_tex;
    uint32_t *player_pixels=NULL;
    SDL_Texture *map_tex;
    uint32_t *map_pixels;

    SDL_Surface *loadSurface(std::string filename) {
        SDL_Surface *convertedSurface;
        SDL_Surface *tmpSurface = IMG_Load(filename.c_str());
        if (tmpSurface == nullptr) {
            std::cerr << "Could not load " << filename << ": " << SDL_GetError() << std::endl;
            SDL_Quit();
            return nullptr;
        }
        convertedSurface = SDL_ConvertSurfaceFormat(tmpSurface, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(tmpSurface);
        if (convertedSurface == nullptr) {
            std::cerr << "Could not convert " << filename << ": " << SDL_GetError() << std::endl;
            SDL_Quit();
            return nullptr;
        }
        return convertedSurface;
    }

    void draw_player(Player &player) {
        int player_pitch;
        SDL_LockTexture(player_tex, NULL, (void **)&player_pixels, &player_pitch);
        memset(player_pixels, 0, 40*40*sizeof(uint32_t));
        for (int i = 20; i < 30; i++) {
            for (int j = 20; j < 30; j++) {
                draw_pixel(vec2(i,j), player_pixels, RED, player_pitch);
            }
        }
        draw_line(vec2(20,20), (vec2(20,20) + 20*player.forwardDir()).round(), player_pixels, player_pitch, RED);
        SDL_UnlockTexture(player_tex);
    }


};



class Game {
public:
    int ok=1;
    SDL_Window *window;
    Renderer *renderer;

    Game() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
            ok = 0;
        }
        window = SDL_CreateWindow("sdl app", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        renderer = new Renderer(window);
    }

    int mainloop(bool screenshot, int capture_width, int capture_height) {
        bool quit = false;
        renderer->draw_2d_map();
        long captureCount = 0;
        while (1) {
        quit = pollAndHandleEvent();
            if (quit) {
                break;
            }

            renderer->drawPixels(player);
            renderer->draw_fov(player);
            
            renderer->renderMap(player, mode);

            if (screenshot && mode == FOV) {
                int w, h;
                if (SDL_GetRendererOutputSize(renderer->renderer, &w, &h) != 0) {
                    std::cerr << "Could not get renderer size: " << SDL_GetError() << std::endl;
                    return 1;
                }
                SDL_Surface* screenshot_surface = SDL_CreateRGBSurfaceWithFormat(
                    0, 2 * w, h, 32, SDL_PIXELFORMAT_ARGB8888
                );
                if (!screenshot_surface) {
                    std::cerr << "Could not create screenshot surface: " << SDL_GetError() << std::endl;
                    return 1;
                }
                SDL_Surface* scaled_surface = SDL_CreateRGBSurfaceWithFormat(
                    0, capture_width, capture_height, 32, SDL_PIXELFORMAT_ARGB8888
                );
                if (!scaled_surface) {
                    std::cerr << "Could not create scaled screenshot: " << SDL_GetError() << std::endl;
                    SDL_FreeSurface(screenshot_surface);
                    return 1;
                }
                renderer->renderMap(player, MAP);
                if (SDL_RenderReadPixels(renderer->renderer, NULL, SDL_PIXELFORMAT_ARGB8888, screenshot_surface->pixels, screenshot_surface->pitch) != 0) {
                    std::cerr << "Could not capture map: " << SDL_GetError() << std::endl;
                    SDL_FreeSurface(scaled_surface);
                    SDL_FreeSurface(screenshot_surface);
                    return 1;
                }
                renderer->renderMap(player, FOV);
                auto *fov_pixels = static_cast<Uint8 *>(screenshot_surface->pixels) + w * sizeof(Uint32);
                if (SDL_RenderReadPixels(renderer->renderer, NULL, SDL_PIXELFORMAT_ARGB8888, fov_pixels, screenshot_surface->pitch) != 0 ||
                    SDL_SetSurfaceBlendMode(screenshot_surface, SDL_BLENDMODE_NONE) != 0 ||
                    SDL_BlitScaled(screenshot_surface, NULL, scaled_surface, NULL) != 0 ||
                    SDL_SaveBMP(scaled_surface, ("cap/screenshot" + std::to_string(captureCount) + ".bmp").c_str()) != 0) {
                    std::cerr << "Could not save screenshot: " << SDL_GetError() << std::endl;
                    SDL_FreeSurface(scaled_surface);
                    SDL_FreeSurface(screenshot_surface);
                    return 1;
                }
                SDL_FreeSurface(scaled_surface);
                SDL_FreeSurface(screenshot_surface);
                captureCount++;
            }

            SDL_RenderPresent(renderer->renderer);

            //SDL_Delay(16); // ~60FPS
        }
        return 0;
    }

    ~Game() {
        SDL_DestroyWindow(window);
        delete renderer;
        SDL_Quit();
        
    }
private:
    Player player{vec2(140, 20), 0};
    Mode mode = MAP;

    // returns true if time to quit
    // otherwise, processes the event and returns false
    bool pollAndHandleEvent() {
        SDL_Event event;
        int x,y;
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT) {
            return true;
        }
        vec2 tmpVec(-1,-1);

        int numKeys, keyCount=0;
        const Uint8 *keyState = SDL_GetKeyboardState(&numKeys);
        for (int i = 0; i < numKeys; i++) {
            if (keyState[i]) {
                keyCount++;
            }
        }

        if (event.type == SDL_KEYDOWN) {
            if (keyCount == 1) {
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
                        player.moveBackward();
                        break;
                    case SDLK_UP:
                        player.moveForward();
                        break;
                    case SDLK_LEFT:
                        player.turnLeft();
                        break;
                    case SDLK_RIGHT:
                        player.turnRight();
                        break;
                }
            } else if (keyCount == 2) {
                vec2 forward = player.forwardDir();
                vec2 orth(-forward.y, forward.x);
                if (keyState[SDL_SCANCODE_LALT]) {
                    if (keyState[SDL_SCANCODE_LEFT]) {
                        player.strafeLeft();
                    } else if (keyState[SDL_SCANCODE_RIGHT]) {
                        player.strafeRight();
                    }
                }
            }
        }

        return false;
    }
};



int main(int argc, char **argv) {
    bool screenshot = false;
    int capture_width = 512, capture_height = 256;
    if (argc >= 2 && !strcmp(argv[1], "screenshot")) {
        screenshot = true;
        if (argc != 2 && argc != 4) {
            std::cerr << "Usage: " << argv[0] << " screenshot [width height]" << std::endl;
            return 1;
        }
        if (argc == 4) {
            auto width = std::from_chars(argv[2], argv[2] + strlen(argv[2]), capture_width);
            auto height = std::from_chars(argv[3], argv[3] + strlen(argv[3]), capture_height);
            if (width.ec != std::errc() || *width.ptr != '\0' || capture_width <= 0 ||
                height.ec != std::errc() || *height.ptr != '\0' || capture_height <= 0) {
                std::cerr << "Screenshot width and height must be positive integers" << std::endl;
                return 1;
            }
        }
        // if directory "cap" doesn't exist, create it
        struct stat st = {0};
        if (stat("cap", &st) == -1) {
            if (mkdir("cap", 0700) != 0) {
                std::cerr << "Could not create cap directory" << std::endl;
                return 1;
            }
        }
    }
   
   Game game;
   if (!game.ok) {
        return 1;
   }

    return game.mainloop(screenshot, capture_width, capture_height);
}
