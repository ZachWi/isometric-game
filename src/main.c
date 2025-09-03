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

// Get a random number from 0 to 255
int randInt(int rmin, int rmax) {
	return rand() % rmax + rmin;
}

// Window dimensions
static const int width = 800;
static const int height = 600;

const int tile_w = 96; // diamond width
const int tile_h = 48; // diamond height
const int origin_x = (width / 2) - 48;
const int origin_y = 120;



void isometric(int gx, int gy, int *sx, int *sy){
	*sx = origin_x + (gx - gy) * (tile_w / 2);
	*sy = origin_y + (gx + gy) * (tile_h / 2);
}

int main(int argc, char **argv) {
	uint32_t lastTime = SDL_GetTicks();
	srand(time(NULL)); 
	IMG_Init(IMG_INIT_PNG);
	CHECK_ERROR(SDL_Init(SDL_INIT_VIDEO) != 0, SDL_GetError());

	SDL_Window *window = SDL_CreateWindow("iso", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
	CHECK_ERROR(window == NULL, SDL_GetError());

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);    
	CHECK_ERROR(renderer == NULL, SDL_GetError());

	SDL_SetRenderDrawColor(renderer, 3, 182, 252, 255);
	SDL_Rect srcRect = {0, 0, 32, 32};

	SDL_Surface* greensurf = IMG_Load("assets/greencube.png");
	SDL_Texture* greentext = SDL_CreateTextureFromSurface(renderer, greensurf);
	SDL_Surface* purplesurf = IMG_Load("assets/purplecube.png");
	SDL_Texture* purpletext = SDL_CreateTextureFromSurface(renderer, purplesurf);
	SDL_Surface* greysurf = IMG_Load("assets/greycube.png");
	SDL_Texture* greytext = SDL_CreateTextureFromSurface(renderer, greysurf);
	SDL_Surface* backsurf = IMG_Load("assets/background.png");
	SDL_Texture* backtext = SDL_CreateTextureFromSurface(renderer, backsurf);
	SDL_FreeSurface(greensurf);
	SDL_FreeSurface(purplesurf);
	SDL_FreeSurface(greysurf);
	SDL_FreeSurface(backsurf);

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
		if (scan <= 96){
			scan = 96;
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
		// draw
		for (int by = 0; by < 20; by++) {
			for (int bx = 0; bx <= 26; bx++) {
				SDL_Rect backRect = { (bx * 96) + scan - 96, (by * 96) + scan - 96, tile_w, 96 };
				SDL_RenderCopy(renderer, backtext, &srcRect, &backRect);
			}
		}
		for (int gy = 0; gy < 6; gy++) {
			for (int gx = 0; gx < 6; gx++) {
				int sx, sy;
				isometric(gx, gy, &sx, &sy);
				SDL_Rect destRect = { sx, sy + dist, tile_w, 96 };
				SDL_Texture* tex = ((gx + gy) % 2 == 0) ? greentext : purpletext;
				SDL_RenderCopy(renderer, tex, &srcRect, &destRect);
			}
		}
		isometric(gx, gy, &cx, &cy);
		SDL_Rect charRect = { cx, cy - 48, tile_w, 96 };

		SDL_RenderCopy(renderer, greytext, &srcRect, &charRect);

		// Show what was drawn
		SDL_RenderPresent(renderer);
	}

	// Release resources
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
