/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, SDL_ttf, standard IO, strings and string streams
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <vector>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();
	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);
#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
#endif
	//Deallocates texture
	void free();
	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);
	//Set blending
	void setBlendMode(SDL_BlendMode blending);
	//Set alpha modulation
	void setAlpha(Uint8 alpha);
	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
	//Gets image dimensions
	int getWidth();
	int getHeight();
private:
	//The actual hardware texture
	SDL_Texture * mTexture;
	//Image dimensions
	int mWidth;
	int mHeight;
};

class Dot
{
public:
	//The dimensions of the dot
	static const int DOT_WIDTH = 20;
	static const int DOT_HEIGHT = 20;
	//Maximum axis velocity of the dot
	static const int DOT_VEL = 1;
	//Initializes the variables
	Dot(int x, int y);
	//Takes key presses and adjusts the dot's velocity
	void handleEvent(SDL_Event& e);
	//Moves the dot and checks collision
	void move(std::vector<SDL_Rect>& wall);
	//Shows the dot on the screen
	void render();
	//Gets the collision boxes
	std::vector<SDL_Rect>& getColliders();
private:
	//The X and Y offsets of the dot
	int mPosX, mPosY;
	//The velocity of the dot
	int mVelX, mVelY;
	//Dot's collision box
	std::vector<SDL_Rect> mColliders;
	//Moves the collision boxes relative to the dot's offset
	void shiftColliders();
};

//Starts up SDL and creates window
bool init();
//Loads media
bool loadMedia();
//Frees media and shuts down SDL
void close();
//Box collision detector
bool checkCollision(std::vector<SDL_Rect>& a, std::vector<SDL_Rect>& b);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
//The surface contained by the window
SDL_Renderer* gRenderer = NULL;
//Globally used font
TTF_Font *gFont = NULL;
//Scene texture
LTexture gDotTexture;

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//The dot that will be moving around on the screen
			Dot dot(0, 0);

			//The dot that will be collided against
			Dot otherDot(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);

			//While application is running
			while (!quit)
			{
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}

					//Handle input for the dot
					dot.handleEvent(e);
				}

				//Move the dot and check collision
				dot.move(otherDot.getColliders());

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//Render dots
				dot.render();
				otherDot.render();

				//Update screen 
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}

bool init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	//Check for joysticks
	if (SDL_NumJoysticks() < 1)
	{
		printf("Warning: No joysticks connected!\n");
	} 
	else
	{
		//Load joystick
#ifdef gGameController
		gGameController = SDL_JoystickOpen(0);
		if (gGameController == NULL)
		{
			printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Get controller haptic device
			gControllerHaptic = SDL_HapticOpenFromJoystick(gGameController);
			if (gControllerHaptic == NULL)
			{
				printf("Warning: Controller does not support haptics! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				//Get initialize rumble
				if (SDL_HapticRumbleInit(gControllerHaptic) < 0)
				{
					printf("Warning: Unable to initialize rumble! SDL Error: %s\n", SDL_GetError());
				}
			}
		}

#endif
	}

	//Create window
	gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Create renderer for window
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (gRenderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

#ifdef SDL_IMAGE_H_
	//Initialize PNG  loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}
#endif	

#ifdef _SDL_TTF_H
	//Initialize SDL_tff
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}
#endif

#ifdef SDL_MIXER_H_
	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		return false;
	}

#endif // _SDL_MIXER_H


	return true;
}

bool loadMedia()
{
	//Load dot texture
	if (!gDotTexture.loadFromFile("26_motion/dot.bmp"))
	{
		printf("Failed to load dot texture!\n");
		return false;
	}

	return true;
}

void close()
{
	//Free loaded images
	gDotTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(std::vector<SDL_Rect>& a, std::vector<SDL_Rect>& b)
{
	//The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;


	for (Uint8 Abox = 0; Abox < a.size(); Abox++)
	{
		//Calculate the sides of rect A
		leftA = a[Abox].x;
		rightA = a[Abox].x + a[Abox].w;
		topA = a[Abox].y;
		bottomA = a[Abox].y + a[Abox].h;

		for (Uint8 Bbox = 0; Bbox < b.size(); Bbox++)
		{
			//Calculate the sides of rect B
			leftB = b[Bbox].x;
			rightB = b[Bbox].x + b[Bbox].w;
			topB = b[Bbox].y;
			bottomB = b[Bbox].y + b[Bbox].h;

			//If no sides from A are outside of B
			if (!((bottomA <= topB) || (topA >= bottomB) || (rightA <= leftB) || (leftA >= rightB)))
			{
				//a collision is detected
				return true;
			}
		}
	}

	//If neither set of collision boxes touched
	return false;
}

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0xFF, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success 
	mTexture = newTexture;
	return mTexture != NULL;
}
#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
	if (textSurface == NULL)
	{
		printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		return false;
	}

	//Create texture from surface pixels
	mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
	if (mTexture == NULL)
	{
		printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Get image dimensions
	mWidth = textSurface->w;
	mHeight = textSurface->h;

	//Get rid of old surface
	SDL_FreeSurface(textSurface);

	return true;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Dot::Dot(int x, int y)
{
	//Initialize the offsets
	mPosX = x;
	mPosY = y;
	//Set collision box dimension
	mColliders.resize(11);
	//Initialize the velocity
	mVelX = 0;
	mVelY = 0; 
	
	//Initialize the collision boxes' width and height 
	mColliders[ 0 ].w = 6; 
	mColliders[ 0 ].h = 1; 

	mColliders[ 1 ].w = 10; 
	mColliders[ 1 ].h = 1; 

	mColliders[ 2 ].w = 14; 
	mColliders[ 2 ].h = 1; 

	mColliders[ 3 ].w = 16; 
	mColliders[ 3 ].h = 2; 

	mColliders[ 4 ].w = 18; 
	mColliders[ 4 ].h = 2; 

	mColliders[ 5 ].w = 20; 
	mColliders[ 5 ].h = 6; 

	mColliders[ 6 ].w = 18; 
	mColliders[ 6 ].h = 2; 

	mColliders[ 7 ].w = 16;
	mColliders[ 7 ].h = 2; 

	mColliders[ 8 ].w = 14;
	mColliders[ 8 ].h = 1; 

	mColliders[ 9 ].w = 10;
	mColliders[ 9 ].h = 1;

	mColliders[ 10 ].w = 6;
	mColliders[ 10 ].h = 1;
	//Initialize colliders relative to position 
	shiftColliders();
}

void Dot::handleEvent(SDL_Event & e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY -= DOT_VEL; break;
		case SDLK_DOWN: mVelY += DOT_VEL; break;
		case SDLK_LEFT: mVelX -= DOT_VEL; break;
		case SDLK_RIGHT: mVelX += DOT_VEL; break;
		}
	}
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY += DOT_VEL; break;
		case SDLK_DOWN: mVelY -= DOT_VEL; break;
		case SDLK_LEFT: mVelX += DOT_VEL; break;
		case SDLK_RIGHT: mVelX -= DOT_VEL; break;
		}
	}
}

void Dot::move(std::vector<SDL_Rect>& otherColliders)
{
	//Move the dot left or right
	mPosX += mVelX;
	shiftColliders();

	//If the dot collided went too far to the left or right
	if ((mPosX < 0) || (mPosX + DOT_WIDTH > SCREEN_WIDTH) || checkCollision(mColliders, otherColliders))
	{
		//Move back
		mPosX -= mVelX;
		shiftColliders();
	}

	mPosY += mVelY;
	shiftColliders();

	//If the dot collided went too far up or down
	if ((mPosY < 0) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT) || checkCollision(mColliders, otherColliders))
	{
		//Move back
		mPosY -= mVelY;
		shiftColliders();
	}
}

void Dot::render()
{
	//Shot the dot
	gDotTexture.render(mPosX, mPosY);
}

void Dot::shiftColliders()
{
	//The row offset
	int r = 0;

	//Go through the dot's collision boxes
	for (Uint8 set = 0; set < mColliders.size(); set++)
	{
		//Center the collision box
		mColliders[set].x = mPosX + (DOT_WIDTH - mColliders[set].w)/2;
		//Set the collision box at its row offset
		mColliders[set].y = mPosY + r;
		//Move the row offset down the height of the collision box
		r += mColliders[set].h;
	}
}

std::vector<SDL_Rect>& Dot::getColliders()
{
	return mColliders;
}
