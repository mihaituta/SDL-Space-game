#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_thread.h>
#include <thread>
#include "windows.h"
#include "time.h"
#include <iostream>
#include <conio.h>
using namespace std;

#define MAX_BULLETS 1000
#define MAX_ENEMY 10
#define fps 60
#define sWidth 1920
#define sHeight 1080

typedef struct
{
	float x, y, dy;
	int currentSprite, hbSprite, lifeSprite, life, lives, currentScore;
	SDL_Texture *playership;
	SDL_Texture *enemyship; 
	SDL_Texture *healthbar;
	bool scoreChanged;
	bool gameover;
	bool visible = true;
	bool alive = true;
}Ship;

SDL_Window *window;
SDL_Renderer* renderer;
SDL_Surface *eship[5];

//enemy array
Ship enemy[MAX_ENEMY];
Ship player;
typedef struct {
	float x, y, dx;
}Bullet;

//bullets arrays
Bullet *playerBullets[MAX_BULLETS] = { NULL };
Bullet *enemyBullets[MAX_BULLETS] = { NULL };

int Delay[MAX_ENEMY] = { NULL };
int DelayX[MAX_ENEMY] = { NULL };
int DelayY[MAX_ENEMY] = { NULL };

//texture declarations
SDL_Texture *background = NULL;
SDL_Texture *laserTexture = NULL;
SDL_Texture *enemylaserTexture = NULL;
SDL_Texture *playerlife = NULL;
SDL_Texture *text = NULL;
SDL_Texture *textGO = NULL;
SDL_Surface *textSurface = NULL;
SDL_Surface *textSurfaceGO = NULL;
TTF_Font *font = NULL;
TTF_Font *fontGO = NULL;
SDL_Color color = { 255,255,255 };

SDL_Surface* bg = NULL;
SDL_Surface* ship = NULL;
SDL_Surface* laser = NULL;
SDL_Surface* enemylaser = NULL;
SDL_Surface* lives = NULL;
SDL_Surface* hb = NULL;

//sounds
Mix_Music *bgm;
Mix_Chunk *bullet_shot;
Mix_Chunk *explosion;
 
int enemyNr = 5;
int prevTime = 0;
int curentTime = 0;
float deltaTime = 0;
int playerBulletSpeed = 750;
int enemyBulletSpeed = 530;
int bgspeed = 100;
int timeGameOver = 0;
bool go = false;
int done = 0;
char buffer[15];
int enemyDirectionX[MAX_ENEMY], enemyDirectionY[MAX_ENEMY], enemySpeedX[MAX_ENEMY], enemySpeedY[MAX_ENEMY];
int enemySpeedMaxX = 150;
int enemySpeedMinX = 50;
int enemySpeedMaxY = 100;
int enemySpeedMinY = 50;
int directionTimeX = 3000;
int directionTimeY = 1000;
int textW = 0, textH = 0, textWGO = 0, textHGO = 0;
int timecheck;
int bgX = 0, bgY = 0;
int globalTime = 0;
int textx = 1660, texty = 40;

void addLaser(float x, float y, float dx, Bullet* bullet[])
{
	int found = -1;
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (bullet[i] == NULL)
		{
			found = i;
			break;
		}
	}

	if (found >= 0)
	{
		int i = found;
		bullet[i] = (Bullet*)malloc(sizeof(Bullet));
		bullet[i]->x = x;
		bullet[i]->y = y;
		bullet[i]->dx = dx;
	}
}

void destroyLaser(int i, Bullet* bullet[])
{
	if (bullet[i])
	{
		free(bullet[i]);
		bullet[i] = NULL;
	}
}

void addEnemy() {
	enemy[enemyNr].x = rand() % ((sWidth - 120) - 20) + 20;
	enemy[enemyNr].y = rand() % (-99 - (-200)) + (-200);
	enemy[enemyNr].currentSprite = 0;
	enemy[enemyNr].alive = true;
	enemy[enemyNr].life = 2;
	enemy[enemyNr].visible = true;
	enemy[enemyNr].enemyship = SDL_CreateTextureFromSurface(renderer, eship[rand() % 5]);
}

int events(SDL_Window *window, Ship *player)
{
	SDL_Event event;
	int done = 0;

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_WINDOWEVENT_CLOSE)
			if (window)
			{
				SDL_DestroyWindow(window);
				window = NULL;
				done = 1;
			}
		
		if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				done = 1;
				break;
			}
		}
		if (event.type == SDL_QUIT) {
					done = 1;
				}
		 if (event.type == SDL_MOUSEMOTION) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			SDL_ShowCursor(SDL_DISABLE);
			 
			player->x = event.motion.x-50;
			player->y = event.motion.y-50;
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (SDL_BUTTON_LEFT) {
				if (player->alive) {
					player->currentSprite = 1;
					addLaser(player->x + 45, player->y - 25, 1 * deltaTime*playerBulletSpeed,playerBullets);
					Mix_PlayChannel(-1, bullet_shot, 0);
				}
			}
		}
		else if (event.type == SDL_MOUSEBUTTONUP) {
			if (SDL_BUTTON_LEFT) {
				if (player->alive) {
					player->currentSprite = 0;
				}
		   }
		}
	}
	return done;
}

void render(SDL_Renderer *renderer, Ship *player)
{
	//clear screen
	SDL_RenderClear(renderer);

	//render background
	SDL_Rect rectbg1 = { bgX,bgY,sWidth,sHeight };
	SDL_Rect rectbg2 = { bgX,bgY - sHeight,sWidth,sHeight };
	SDL_RenderCopy(renderer, background, NULL, &rectbg1);
	SDL_RenderCopy(renderer, background, NULL, &rectbg2);

	//render player lives
	SDL_Rect lifeRect = { 162 * player->lifeSprite,0,162,38 };
	SDL_Rect rect = { 30, 30, 163,38 };
	SDL_RenderCopy(renderer, playerlife, &lifeRect, &rect);

	//render healthbar
	SDL_Rect eSrcRect = { 180 * player->hbSprite, 0, 180, 26 };
	SDL_Rect recthb = { 20, 80, 180, 26 };
	SDL_RenderCopy(renderer, player->healthbar, &eSrcRect, &recthb);

	//render enemy
	for (int i = 0; i < enemyNr; i++)
	{
		if (enemy[i].visible) {
			SDL_Rect eSrcRect = { 100 * enemy[i].currentSprite, 0, 100, 100 };
			SDL_Rect rect = { enemy[i].x, enemy[i].y, 100, 100 };
			SDL_RenderCopy(renderer, enemy[i].enemyship, &eSrcRect, &rect);
		}
	}

	//render player
	if (player->visible) {
		SDL_Rect srcRect = { 100 * player->currentSprite, 0, 100, 100 };
		SDL_Rect rect = { player->x, player->y, 100, 100 };
		SDL_RenderCopy(renderer, player->playership, &srcRect, &rect);
	}

	//render bullet
	for (int i = 0; i < MAX_BULLETS; i++) if (playerBullets[i])
	{
		SDL_Rect rect = { playerBullets[i]->x, playerBullets[i]->y, 11, 46 };
		SDL_RenderCopy(renderer, laserTexture, NULL, &rect);
	}

	for (int i = 0; i < MAX_BULLETS; i++) if (enemyBullets[i])
	{
		SDL_Rect rect = { enemyBullets[i]->x, enemyBullets[i]->y, 13, 43 };
		SDL_RenderCopy(renderer, enemylaserTexture, NULL, &rect);
	}
	
	//render score
	SDL_Rect textrect = { textx,texty,textW,textH };
	SDL_RenderCopy(renderer, text, NULL, &textrect);

	//render game over
	if (go) {
		SDL_Rect textrectGO = { sWidth/2 - textWGO/2,sHeight/2 - textHGO,textWGO,textHGO };
		SDL_RenderCopy(renderer, textGO, NULL, &textrectGO);
	}

   //render
	SDL_RenderPresent(renderer);
}

void logic(Ship *player)
{
	bgY += 1 *bgspeed *deltaTime;
	if (bgY >= sHeight) 
		bgY = 0;

	//limit player to not leave screen edges
	if (player->x > sWidth-120)
		player->x = sWidth-120;
	else if (player->x < 20)
		player->x = 20;

	if (player->y < 20)
		player->y = 20;
	else if (player->y > sHeight - 150)
		player->y = sHeight - 150;

	for (int i = 0; i < enemyNr; i++) { 
		if (enemy[i].x > sWidth-120)
			enemyDirectionX[i] = 0;
		else if (enemy[i].x < 20)
			enemyDirectionX[i] = 1;
	}

	//respawn enemies random when they get to the bottom edge of screen
	for (int i = 0; i < enemyNr; i++) {
		if (enemy[i].alive) {
			enemy[i].y++;
			if (enemy[i].y > sHeight) {
				enemy[i].y = rand() % (-99 - (-150)) + (-150);
				enemy[i].x = rand() % ((sWidth-120) - 20) + 20;
				SDL_DestroyTexture(enemy[i].playership);
				enemy[i].enemyship = SDL_CreateTextureFromSurface(renderer, eship[rand() % 5]);
			}
		}
	}

	float Timp = SDL_GetTicks();

	//add enemy bullet
	for (int i = 0; i < enemyNr; i++)
		if (enemy[i].alive && enemy[i].visible && Timp > Delay[i]) {
			Delay[i] = Timp + (rand() % (1400 - 500) + 500);
			addLaser(enemy[i].x + 43, enemy[i].y + 60, -1 * deltaTime*enemyBulletSpeed,enemyBullets);
		    Mix_PlayChannel(-1, bullet_shot, 0);
		}

	

	//set enemy directions randomly for X and Y axes, the speed of the ship and for how long it will move before chosing again left or right
	for (int i = 0; i < enemyNr; i++) {
		if (Timp > DelayX[i]) {
			DelayX[i] = Timp + rand() % directionTimeX;  
			enemyDirectionX[i] = rand() % 2;
			enemySpeedX[i] = rand() % (enemySpeedMaxX - enemySpeedMinX) + enemySpeedMinX;
		}
		
		if (Timp > DelayY[i]) {
			DelayY[i] = Timp + (rand() % directionTimeY);
			enemyDirectionY[i] = rand() % 2;
			enemySpeedY[i] = rand() % (enemySpeedMaxY - enemySpeedMinY) + enemySpeedMinY;
		}
	}

	//move enemy ship based on what direction it choses 
	for (int i = 0; i < enemyNr; i++) {
		if (enemyDirectionX[i] == 0)
			enemy[i].x -= enemySpeedX[i] * deltaTime;
		else if (enemyDirectionX[i] == 1)
			enemy[i].x += enemySpeedX[i] * deltaTime;

		if (enemyDirectionY[i] == 0)
			enemy[i].y -= (enemySpeedY[i] - 50) * deltaTime;
		else if (enemyDirectionY[i] == 1)
			enemy[i].y += enemySpeedY[i] * deltaTime;
	}

	//collision betwen player's laser and enemies
	for (int i = 0; i < MAX_BULLETS; i++) if (playerBullets[i])
	{
		playerBullets[i]->y -= playerBullets[i]->dx;
		for (int j = 0; j < enemyNr; j++) {
			if (enemy[j].alive && (playerBullets[i]->x > enemy[j].x && playerBullets[i]->x < enemy[j].x + 97 && playerBullets[i]->y <= enemy[j].y + 60 && playerBullets[i]->y >= enemy[j].y + 40))
			{
				enemy[j].life -= 1;
				if (enemy[j].life <= 0) {
					Mix_PlayChannel(2, explosion, 0);
					enemy[j].alive = false;
					player->currentScore += 100;
					player->scoreChanged = true;
				}

			}
		}
	}

	if (player->currentScore == 100)
		textx = 1660;
	else if (player->currentScore > 100)
		textx = 1645;
	if (player->currentScore == 1000)
		textx = 1640;  
	else if (player->currentScore > 1000)
		textx = 1625; 
	if (player->currentScore == 10000 || player->currentScore == 10100)
		textx = 1615;  
	else if (player->currentScore > 10100)
		textx = 1610; 

	sprintf(buffer, "SCORE %d", player->currentScore);

	////collision betwen enemy's bullet and player
	for (int i = 0; i < MAX_BULLETS; i++) if (enemyBullets[i]) {
		enemyBullets[i]->y -= enemyBullets[i]->dx;
		if (player->alive && (enemyBullets[i]->x > player->x && enemyBullets[i]->x < player->x + 97 && enemyBullets[i]->y >= player->y + 15 && enemyBullets[i]->y <= player->y + 30))
		{
			player->life -= 25;
			player->hbSprite++;
			if (player->life <= 0) {
				player->lives--;
				player->life = 100;
				player->lifeSprite++;
				player->hbSprite = 0;
				if (player->lives <= 0) {
					Mix_PlayChannel(2, explosion, 0);
					player->alive = false;
					player->hbSprite = 4;
					player->lifeSprite = 3;
				 }
			  }
			destroyLaser(i, enemyBullets);
		}
	}

	for (int i = 0; i < MAX_BULLETS; i++) if (enemyBullets[i]) {
		if (enemyBullets[i]->y > sHeight)
			destroyLaser(i, enemyBullets);
	}

	
	//player's ship explodes when life = 0
	if (player->alive == false)
	{
		if (player->currentSprite < 2)
			player->currentSprite = 2;
		else if (player->currentSprite >= 2)
		{
			player->currentSprite++;
			if (player->currentSprite > 17)
			{
				player->alive = false;
				player->visible = false;
			}
		}
	}

	//destroy player's laser when it hits enemy
	for (int j = 0; j < enemyNr; j++) {
		for (int i = 0; i < MAX_BULLETS; i++) if (playerBullets[i])
		{
			if (enemy[j].alive && (playerBullets[i]->x > enemy[j].x && playerBullets[i]->x < enemy[j].x + 97 && playerBullets[i]->y <= enemy[j].y + 60 && playerBullets[i]->y >= enemy[j].y + 40))
			{
				destroyLaser(i, playerBullets);
			}
		}
	}

	for (int i = 0; i < MAX_BULLETS; i++) if (playerBullets[i])
	{
		if (playerBullets[i]->y < -20)
			destroyLaser(i,playerBullets);
	}

	//run explosion animation when enemy's life is 0 and reset status
	for (int j = 0; j < enemyNr; j++) {
		if (enemy[j].alive == false)
		{
			if (enemy[j].currentSprite < 1)
				enemy[j].currentSprite = 1;
			else if (enemy[j].currentSprite >= 1)
			{
				enemy[j].currentSprite++;
				if (enemy[j].currentSprite > 16)
				{
					enemy[j].currentSprite = 0;
					enemy[j].alive = true;
					enemy[j].y = -100;
					enemy[j].x = rand() % sWidth;
					SDL_DestroyTexture(enemy[j].playership);
					enemy[j].enemyship = SDL_CreateTextureFromSurface(renderer, eship[rand() % 5]);
					if (player->currentScore <= 1500) {
						enemy[j].life = 2;	
					}
					else if (player->currentScore <= 3000) {
						enemy[j].life = 3;
						addEnemy();
						enemyNr = 6;
						enemySpeedMaxX = 200;
						enemySpeedMinX = 100;
						enemySpeedMaxY = 200;
					    enemySpeedMinY = 80;
						playerBulletSpeed = 800;
						enemyBulletSpeed = 600;
					}
					else if (player->currentScore <= 5000) {
						enemy[j].life = 4;
						addEnemy();
						enemyNr = 7;
						enemySpeedMaxX = 250;
						enemySpeedMinX = 150;
						enemySpeedMaxY = 250;
						enemySpeedMinY = 100;
						playerBulletSpeed = 900;
						enemyBulletSpeed = 800;
						directionTimeX = 2500;
						directionTimeY = 800;
					}
					else if (player->currentScore <= 7000) {
						enemy[j].life = 5;
						addEnemy();
						enemyNr = 8;
						enemySpeedMaxX = 350;
						enemySpeedMinX = 250;
						enemySpeedMaxY = 350;
						enemySpeedMinY = 200;
						playerBulletSpeed = 1100;
						enemyBulletSpeed = 900;
						directionTimeX = 2000;
						directionTimeY = 400;
					}
					else if (player->currentScore <= 10000) {
						enemy[j].life = 5;
						enemyNr = 10;
						addEnemy();
						addEnemy();
					}
				}
			}
		}
	}

	if (player->alive == false) {
		if(!player->gameover)
		timeGameOver = SDL_GetTicks() + 2000;
		if (SDL_GetTicks() > timeGameOver)
		{
			textx = sWidth/2- textW/2;
			texty = sHeight/2 + textHGO/2;
			for (int i = 0; i < enemyNr; i++) {
				enemy[i].currentSprite = 0;
				enemy[i].alive = false;
				enemy[i].visible = false;
				go = true;
				for (int j = 0; j < MAX_BULLETS; j++)
					if (enemy[i].visible == false) {
						destroyLaser(j,enemyBullets);
				 }
			 }
		  }
	   player->gameover = true;
	}
}

int loadsurfaces()
{
	bg = IMG_Load("bg2.png");
	hb = IMG_Load("healthbar1.png");
	lives = IMG_Load("playerlifex.png");
	enemylaser = IMG_Load("enemylaser.png");
	laser = IMG_Load("laser2.png");
	eship[0] = IMG_Load("enemyship.png");
	eship[1] = IMG_Load("enemyorange.png");
	eship[2] = IMG_Load("enemygreen.png");
	eship[3] = IMG_Load("enemypurple.png");
	eship[4] = IMG_Load("enemyblue.png");
	ship = IMG_Load("myship.png");
	font = TTF_OpenFont("font2.ttf", 40);
	fontGO = TTF_OpenFont("font2.ttf", 80);
	bgm = Mix_LoadMUS("imaginedragons.mp3");
	bullet_shot = Mix_LoadWAV("laser1.ogg");
	explosion = Mix_LoadWAV("explosion.wav");

	return 0;
}

int loadtextures()
{
	background = SDL_CreateTextureFromSurface(renderer, bg);
	SDL_FreeSurface(bg);

	player.playership = SDL_CreateTextureFromSurface(renderer, ship);
	SDL_FreeSurface(ship);

	for (int i = 0; i < enemyNr; i++) {
		int p = rand() % 5;
		enemy[i].enemyship = SDL_CreateTextureFromSurface(renderer, eship[p]);
	}

	laserTexture = SDL_CreateTextureFromSurface(renderer, laser);

	enemylaserTexture = SDL_CreateTextureFromSurface(renderer, enemylaser);
	SDL_FreeSurface(laser);

	playerlife = SDL_CreateTextureFromSurface(renderer, lives);
	SDL_FreeSurface(lives);

	player.healthbar = SDL_CreateTextureFromSurface(renderer, hb);
	SDL_FreeSurface(hb);

	
	sprintf(buffer, "SCORE %d", player.currentScore);
	textSurface = TTF_RenderText_Solid(font, buffer, color);

	text = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);

	textSurfaceGO = TTF_RenderText_Solid(fontGO, "Game Over", color);
	textGO = SDL_CreateTextureFromSurface(renderer, textSurfaceGO);
	SDL_FreeSurface(textSurfaceGO);
	SDL_QueryTexture(textGO, NULL, NULL, &textWGO, &textHGO);

	Mix_VolumeMusic(1); //70
	Mix_VolumeChunk(bullet_shot, 1); //12
	Mix_VolumeChunk(explosion, 1); //75
	return 0;
}

int loadResources(void* data) {
	loadsurfaces();
	loadtextures();

	return 0;
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	TTF_Init();

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}
	srand(time(NULL));

	//create window
	window = SDL_CreateWindow("FIREHIVE'S GAME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sWidth, sHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);

	//create render
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	player.x = 450;
	player.y = 660;
	player.currentSprite = 0;
	player.hbSprite = 0;
	player.lifeSprite = 0;
	player.life = 100;
	player.lives = 3;
	player.alive = true;
	player.visible = true;
	player.currentScore = 0;
	player.scoreChanged = false;
	player.gameover = false;

	for (int i = 0; i < enemyNr; i++) {
		enemy[i].x = rand() % ((sWidth - 120) - 20) + 20;
		enemy[i].y = rand() % (-99 - (-200)) + (-200);
		enemy[i].currentSprite = 0;
		enemy[i].alive = true;
		enemy[i].life = 2;
		enemy[i].visible = true;
	}

	 SDL_Thread* thread = SDL_CreateThread(loadResources, "loadResources", (void*)NULL);
	 timecheck = SDL_GetTicks();
	 SDL_Delay(300);

	Uint32 tick;
 
	while (!done)
	{
		 tick = SDL_GetTicks();
	     prevTime = curentTime;
	     curentTime = SDL_GetTicks();
	     deltaTime = (curentTime - prevTime) / 1000.0f;
		
		//check if there are any events
		done = events(window, &player);
		logic(&player);
		

		if (Mix_PlayingMusic() == 0)
		{
			//start music
			Mix_PlayMusic(bgm, 1);
		}

		textSurface = TTF_RenderText_Solid(font, buffer, color);
		
		if (player.scoreChanged == true) {
			SDL_DestroyTexture(text);
			text = SDL_CreateTextureFromSurface(renderer, textSurface);
			player.scoreChanged = false;
		}
	 
		SDL_QueryTexture(text, NULL, NULL, &textW, &textH);
		SDL_FreeSurface(textSurface);
		render(renderer, &player);

		if ((1000 / fps) > SDL_GetTicks() - tick) {
			SDL_Delay(1000 / fps - (SDL_GetTicks() - tick));
		}
	}

	SDL_DestroyWindow(window);
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(player.playership);
	for (int i = 0; i < enemyNr; i++) {
		SDL_DestroyTexture(enemy[i].enemyship);
	}
	SDL_DestroyTexture(playerlife);
	SDL_DestroyTexture(laserTexture);
	SDL_DestroyTexture(enemylaserTexture);
	SDL_DestroyTexture(player.healthbar);
	SDL_DestroyTexture(text);
	SDL_DestroyRenderer(renderer);
	TTF_CloseFont(font);

	Mix_FreeMusic(bgm);
	Mix_FreeChunk(bullet_shot);
	Mix_FreeChunk(explosion);

	for (int i = 0; i < MAX_BULLETS; i++) {
		destroyLaser(i, playerBullets);
		destroyLaser(i, enemyBullets);
	}
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	return 0;
}