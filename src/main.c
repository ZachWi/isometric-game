#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Utility macros
#define CHECK_ERROR(test, message) \
    do { \
        if((test)) { \
            fprintf(stderr, "%s\n", (message)); \
            exit(1); \
        } \
    } while(0)

// Get a random number from min to max
int randInt(int rmin, int rmax) {
    return rand() % (rmax - rmin + 1) + rmin;
}

// Function to get the resource path for assets
char* getResourcePath(const char *subDir) {
    // Get the base path (where the executable is located)
    char *basePath = SDL_GetBasePath();
    if (basePath) {
        // If we're running from the build directory, we need to go up to the project root
        char *buildPos = strstr(basePath, "build/");
        if (buildPos) {
            *buildPos = '\0'; // Truncate at the build directory
        }
        
        // Calculate the length needed for the new path
        size_t pathLen = strlen(basePath) + strlen("assets/") + strlen(subDir) + 2;
        char *path = (char*)malloc(pathLen);
        if (path) {
            snprintf(path, pathLen, "%sassets/%s", basePath, subDir);
        }
        SDL_free(basePath);
        return path;
    }
    return NULL;
}

// Window dimensions
static const int width = 800;
static const int height = 600;

const int tile_w = 96; // diamond width
const int tile_h = 48; // diamond height
const int origin_x = (width / 2) - 48;
const int origin_y = 120;

void isometric(int gx, int gy, int *sx, int *sy) {
    *sx = origin_x + (gx - gy) * (tile_w / 2);
    *sy = origin_y + (gx + gy) * (tile_h / 2);
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    char* fullPath = getResourcePath(path);
    if (!fullPath) {
        fprintf(stderr, "Could not get resource path for: %s\n", path);
        return NULL;
    }
    
    printf("Loading texture from: %s\n", fullPath);
    SDL_Surface* surface = IMG_Load(fullPath);
    free(fullPath);
    
    if (!surface) {
        fprintf(stderr, "Could not load image: %s\n", IMG_GetError());
        return NULL;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        return NULL;
    }
    
    return texture;
}

int main(int argc, char **argv) {
    uint32_t lastTime = SDL_GetTicks();
    srand(time(NULL)); 
    
    CHECK_ERROR(SDL_Init(SDL_INIT_VIDEO) != 0, SDL_GetError());
    CHECK_ERROR(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG, IMG_GetError());

    SDL_Window *window = SDL_CreateWindow("iso", SDL_WINDOWPOS_UNDEFINED, 
                                         SDL_WINDOWPOS_UNDEFINED, width, height, 
                                         SDL_WINDOW_OPENGL);
    CHECK_ERROR(window == NULL, SDL_GetError());

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 
                                               SDL_RENDERER_ACCELERATED | 
                                               SDL_RENDERER_PRESENTVSYNC);    
    CHECK_ERROR(renderer == NULL, SDL_GetError());

    SDL_SetRenderDrawColor(renderer, 3, 182, 252, 255);
    SDL_Rect srcRect = {0, 0, 32, 32};

    // Load textures with proper path handling
    SDL_Texture* greentext = loadTexture(renderer, "greencube.png");
    SDL_Texture* purpletext = loadTexture(renderer, "purplecube.png");
    SDL_Texture* greytext = loadTexture(renderer, "greycube.png");
    SDL_Texture* backtext = loadTexture(renderer, "background.png");
    
    // Check if all textures loaded successfully
    if (!greentext || !purpletext || !greytext || !backtext) {
        fprintf(stderr, "Failed to load one or more textures. Exiting.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    int gx = 0;
    int gy = 0;
    int cx, cy;
    int scan = 0;
    int dist = 0;
    bool running = true;
    SDL_Event event;
    
    while(running) {
        uint32_t currentTime = SDL_GetTicks();
        float deltaTime = (float)(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        scan += 100.0f * deltaTime;
        if (scan >= 96){
            scan = 0;
        }
        
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
            } else if(event.type == SDL_KEYDOWN) {
                const char *key = SDL_GetKeyName(event.key.keysym.sym);
                if(strcmp(key, "Q") == 0) {
                    running = false;
                }
                if(strcmp(key, "A") == 0 && gy < 5) {
                    gy += 1;
                }
                if(strcmp(key, "D") == 0 && gy > 0) {
                    gy -= 1;
                }
                if(strcmp(key, "S") == 0 && gx < 5) {
                    gx += 1;
                }
                if(strcmp(key, "W") == 0 && gx > 0) {
                    gx -= 1;
                }
            }
        }

        // Clear screen
        SDL_RenderClear(renderer); 
        
        // Draw background
        for (int by = 0; by < 20; by++) {
            for (int bx = 0; bx <= 26; bx++) {
                SDL_Rect backRect = { (bx * 96) + scan - 96, (by * 96) + scan - 96, tile_w, 96 };
                SDL_RenderCopy(renderer, backtext, &srcRect, &backRect);
            }
        }
        
        // Draw tiles
        for (int ty = 0; ty < 6; ty++) {
            for (int tx = 0; tx < 6; tx++) {
                int sx, sy;
                isometric(tx, ty, &sx, &sy);
                SDL_Rect destRect = { sx, sy + dist, tile_w, 96 };
                SDL_Texture* tex = ((tx + ty) % 2 == 0) ? greentext : purpletext;
                SDL_RenderCopy(renderer, tex, &srcRect, &destRect);
            }
        }
        
        // Draw character
        isometric(gx, gy, &cx, &cy);
        SDL_Rect charRect = { cx, cy - 48, tile_w, 96 };
        SDL_RenderCopy(renderer, greytext, &srcRect, &charRect);

        // Show what was drawn
        SDL_RenderPresent(renderer);
    }

    // Release resources
    SDL_DestroyTexture(greentext);
    SDL_DestroyTexture(purpletext);
    SDL_DestroyTexture(greytext);
    SDL_DestroyTexture(backtext);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
