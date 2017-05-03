//
//  ray0.c
//  
//
//  Created by ekandrot on 9/23/15.
//
//

#include "maths.h"
#include "png_wrapper.h"
#include "coords.h"
#include "world.h"
#include <thread>
#include <png.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "scheduler.h"

SDL_Surface *surface;
SDL_Surface *surfaceR;
static bool _dirty;


#define WIDTH 320
#define HEIGHT 320
#define SUBSAMPLING 5

static ray3 screen_to_ray(double x, double y, double o) {
    ray3 ray;
    ray.origin.x = o/WIDTH;
    ray.origin.y = 0;
    ray.origin.z = 5;
    coord3 lookat;
    lookat.x = (2*(x/WIDTH) - 1);
    lookat.y = (1 - 2*(y/HEIGHT));
    lookat.z = 0;
    ray.direction = unit_vector(lookat - ray.origin);
    return ray;
}


static double srgbEncode(double c) {
    double x = c;
    if (c <= 0.0031308f) {
        x = 12.92f * c;
    } else {
        x = 1.055f * pow(c, 1/2.4) - 0.055f;
    }
    x *= 256;
    if (x<0) {
        x = 0;
    } else if (x>255) {
        x = 255;
    }
    return x;
}

// gamma corrected
static RGBPixel to_srgb(color4 c) {
    // ignore alpha for now
    return {(uint8_t)(srgbEncode(c.r)), (uint8_t)(srgbEncode(c.g)), (uint8_t)(srgbEncode(c.b))};
}

struct params_t {
    uint8_t *pixels;
    double offset;
    world *w;
    int startLine;
    int endLine;
};

static void* test_func(void* param) {
    params_t *p = (params_t*)param;
    for (int y=p->startLine; y<p->endLine; ++y) {
        for (int x=0; x<WIDTH; ++x) {
            color4 summingColor;
            for (int j=0; j<SUBSAMPLING; ++j) {
                for (int i=0; i<SUBSAMPLING; ++i) {
                    ray3 r = screen_to_ray(x+(1.0/(SUBSAMPLING*2))+(1.0/SUBSAMPLING)*i,
                                           y+(1.0/(SUBSAMPLING*2))+(1.0/SUBSAMPLING)*j,
                                           p->offset);
                    color4 c = p->w->cast_ray(r);
                    summingColor += c;
                }
            }
            RGBPixel c = to_srgb(summingColor / sqr(SUBSAMPLING));
            *(p->pixels + y * surface->pitch + x*4 +0 ) = c.red;
            *(p->pixels + y * surface->pitch + x*4 +1 ) = c.green;
            *(p->pixels + y * surface->pitch + x*4 +2 ) = c.blue;
            *(p->pixels + y * surface->pitch + x*4 +3 ) = 255;
        }
    }
    return nullptr;
}

class imager : public worker {
    world *w;
public:
    imager(world *world) : w(world) {}
    void do_work(int work) {
        params_t params;
        if (work & 1) {
            params.pixels = static_cast<uint8_t*>(surface->pixels);
            params.offset = -30;
        } else {
            params.pixels = static_cast<uint8_t*>(surfaceR->pixels);
            params.offset = 30;
        }
        work /= 2;
        params.w = w;
        params.startLine = work;
        params.endLine = work+1;
        test_func((void*)&params);
        _dirty = true;
    }
};


// inclusive
static int rand_range(int a, int b) {
    return a + rand() % (b-a+1);
}



int main(int argc, char** argv) {
    srand(0);
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *textureR;
    SDL_Event event;

    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }
    
    window = SDL_CreateWindow("SDL_CreateTexture",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              1024, 768,
                              SDL_WINDOW_RESIZABLE);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        exit(1);
    }
    
#if (SDL_MAJOR_VERSION > 2) || ((SDL_MAJOR_VERION ==2) && (SDL_MINOR_VERSION > 0) || (SDL_PATCHLEVEL >= 5))
    surface = SDL_CreateRGBSurfaceWithFormat(0, WIDTH, HEIGHT, 32, SDL_PIXELFORMAT_ABGR8888);
    if (surface == nullptr) {
        fprintf(stderr, "SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
        exit(1);
    }
    surfaceR = SDL_CreateRGBSurfaceWithFormat(0, WIDTH, HEIGHT, 32, SDL_PIXELFORMAT_ABGR8888);
    if (surfaceR == nullptr) {
        fprintf(stderr, "SDL_CreateRGBSurfaceWithFormat failed: %s\n", SDL_GetError());
        exit(1);
    }
#else
    Uint32 rmask, gmask, bmask, amask; 
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, rmask, gmask, bmask, amask);
    if (surface == nullptr) {
        fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    surfaceR = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, rmask, gmask, bmask, amask);
    if (surfaceR == nullptr) {
        fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        exit(1);
    }

#endif
    if (SDL_MUSTLOCK(surface)) {
        std::cout << "Surface must be locked..." << std::endl;
        exit(1);
    }

    SDL_memset(surface->pixels, 128, surface->h * surface->pitch);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_memset(surfaceR->pixels, 0, surfaceR->h * surfaceR->pitch);
    textureR = SDL_CreateTextureFromSurface(renderer, surfaceR);
    if (textureR == NULL) {
        fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
    
    world w;
    for (int i=0; i<100; ++i) {
        w.add_sphere(coord3(rand_range(-10,10)/10.0, rand_range(-10,10)/10.0, -rand_range(-10,10)/10.0),
                     rand_range(1,10) / 30.0,
                     {rand_range(0,10)/10.0, rand_range(0,10)/10.0, rand_range(0,10)/10.0, 1});
    }
//    w.add_sphere({0,0,-1}, 0.4, {1,0,0,1});
//    w.add_sphere({0.3,0.3,-1}, 0.3, {0,1,0,1});

    // create the bitmap and raycast to fill it

    _dirty = false;
    imager image(&w);
    scheduler s(&image, surface->h*2);
    s.run();
    
    SDL_Rect r={};
    r.w = WIDTH;
    r.h = HEIGHT;

    SDL_Rect rDstR;
    rDstR.x = WIDTH;
    rDstR.y = 0;
    rDstR.w = WIDTH;
    rDstR.h = HEIGHT;
    while (1) {
        if (_dirty) {
            _dirty = false;
            SDL_DestroyTexture(texture);
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroyTexture(textureR);
            textureR = SDL_CreateTextureFromSurface(renderer, surfaceR);
        }
        
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;
        
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, texture, NULL, &r);
        SDL_RenderCopy(renderer, textureR, NULL, &rDstR);
        SDL_RenderPresent(renderer);
    }
    s.join();

    SDL_FreeSurface(surface);
    surface = NULL;
    
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}


