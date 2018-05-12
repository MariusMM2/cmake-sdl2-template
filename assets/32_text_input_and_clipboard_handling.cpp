//Using SDL, SDL_image, SDL_ttf, standard IO, strings and string streams
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cstdio>
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
	bool loadFromFile(const std::string &path);
#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(const std::string &textureText, SDL_Color textColor);
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

//The dot that will move around on the screen
class Dot
{
public:
	//The dimensions of the dot
	static const int DOT_WIDTH = 20;
	static const int DOT_HEIGHT = 20;
	//Maximum axis velocity of the dot
	static const int DOT_VEL = 10;
	//Initializes the variables
	Dot();
	//Takes key presses and adjusts the dot's velocity
	void handleEvent(SDL_Event& e);
	//Moves the dot and checks collision
	void move();
	//Shows the dot on the screen
	void render();
private:
	//The X and Y offsets of the dot
	int mPosX, mPosY;
	//The velocity of the dot
	int mVelX, mVelY;
};

//Starts up SDL and creates window
bool init();
//Loads media
bool loadMedia();
//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
//The surface contained by the window
SDL_Renderer* gRenderer = NULL;
//Globally used font
TTF_Font *gFont = NULL;
//Scene texture
LTexture gDotTexture;
LTexture gBGTexture;

int main(int argc, char* args[])
{
	//Start up SDL and create window
	printf("Initializing...\n");
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		printf("Loading media...\n");
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
			Dot dot;

			//The background scrolling offset
			int scrollingOffset = 0;

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
				dot.move();

				//Scroll background
				--scrollingOffset;
				if (scrollingOffset < -gBGTexture.getWidth())
				{
					scrollingOffset = 0;
				}

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//Render background
				gBGTexture.render(scrollingOffset, 0);
				gBGTexture.render(scrollingOffset+ gBGTexture.getWidth(), 0);
				//Render objects
				dot.render();

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
	if (!gDotTexture.loadFromFile("31_scrolling_backgrounds/dot.bmp"))
	{
		printf("Failed to load dot texture!\n");
		return false;
	}

	//Load background texture
	if (!gBGTexture.loadFromFile("31_scrolling_backgrounds/bg.png"))
	{
		printf("Failed to load background texture!\n");
		return false;
	}

	return true;
}

void close()
{
	//Free loaded images
	gDotTexture.free();
	gBGTexture.free();

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

double distanceSquared(int x1, int y1, int x2, int y2)
{
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return deltaX * deltaX + deltaY * deltaY;
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

bool LTexture::loadFromFile(const std::string &path)
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
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0x00, 0xFF, 0xFF));

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
bool LTexture::loadFromRenderedText(const std::string &textureText, SDL_Color textColor)
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

Dot::Dot()
{
	//Initialize the offsets
	mPosX = 0;
	mPosY = 0;
	//Initialize the velocity
	mVelX = 0;
	mVelY = 0; 
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

void Dot::move()
{
	//Move the dot left or right
	mPosX += mVelX;

	//If the dot went too far to the left or right
	if ((mPosX < 0) || (mPosX + DOT_WIDTH > SCREEN_WIDTH))
	{
		//Move back
		mPosX -= mVelX;
	}

	//Move the dot up or down
	mPosY += mVelY;

	//If the dot collided went too far up or down
	if ((mPosY < 0) || (mPosY + DOT_HEIGHT > SCREEN_HEIGHT))
	{
		//Move back
		mPosY -= mVelY;
	}
}

void Dot::render()
{
	//Shot the dot
	gDotTexture.render(mPosX, mPosY);
}
