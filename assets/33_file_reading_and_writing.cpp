//Using SDL, SDL_image, SDL_ttf, standard IO, strings and string streams
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cstdio>
#include <string>
#include <sstream>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TOTAL_DATA = 10;

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
	//The texture
    LTexture gDotTexture;
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
LTexture gPromptTextTexture;
//Scene texture
LTexture gDataTextures[TOTAL_DATA];

//Data points
Sint32 gData[TOTAL_DATA];

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

			//Set text color as black
            SDL_Color textColor = {0, 0, 0, 0xFF};
            SDL_Color highlightColor = {0xFF, 0, 0, 0xFF};

			//The current input point
       		int currentData = 0;
            std::string inputText = "Some Text";

            //Enable text input
            SDL_StartTextInput();

			//While application is running
			while (!quit)
			{
			    //The rerender text flag
                bool renderText = false;

				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
					else if(e.type == SDL_KEYDOWN)
                    {
                        switch(e.key.keysym.sym)
						{
							//Previous data entry
							case SDLK_UP:
								//Rerender previous entry input point
								gDataTextures[currentData].loadFromRenderedText(std::to_string( (long long)gData[ currentData ] ), textColor);
								--currentData;
								if(currentData < 0)
								{
									currentData = TOTAL_DATA - 1;
								}

								//Rerender current entry input point
								gDataTextures[currentData].loadFromRenderedText(std::to_string( (long long)gData[ currentData ] ), highlightColor);
								break;

								//Next data entry
							case SDLK_DOWN:
								//Rerender previous entry input point
								gDataTextures[currentData].loadFromRenderedText(std::to_string( (long long)gData[ currentData ] ), textColor);
								++currentData;
								if(currentData == TOTAL_DATA)
								{
									currentData = 0;
								}

								//Rerender current entry input point
								gDataTextures[currentData].loadFromRenderedText(std::to_string( (long long)gData[ currentData ] ), highlightColor);
								break;

								//Decrement input point
							case SDLK_LEFT:
								--gData[currentData];
								gDataTextures[currentData].loadFromRenderedText(std::to_string( (long long)gData[ currentData ] ), highlightColor);
								break;

								//Increment input point
							case SDLK_RIGHT:
								++gData[currentData];
								gDataTextures[ currentData ].loadFromRenderedText( std::to_string( (long long)gData[ currentData ] ), highlightColor );
								break;
						}
                    }
                    else if(e.type == SDL_TEXTINPUT)
                    {
                        //Not copy or pasting
                        if(!((e.text.text[0] == 'c' ||
                              e.text.text[ 0 ] == 'C' ) &&
                             (e.text.text[ 0 ] == 'v' ||
                              e.text.text[ 0 ] == 'V' )
                             && SDL_GetModState() & KMOD_CTRL))
                        {
                            //Append character
                            inputText += e.text.text;
                            renderText = true;
                        }
                    }
				}

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//Render text textures
                gPromptTextTexture.render((SCREEN_WIDTH - gPromptTextTexture.getWidth()) / 2, 0);
				for (int i = 0; i < TOTAL_DATA; ++i) {
					gDataTextures[i].render(( SCREEN_WIDTH - gDataTextures[ i ].getWidth() ) / 2, gPromptTextTexture.getHeight() + gDataTextures[ 0 ].getHeight() * i);
				}

				//Update screen
				SDL_RenderPresent(gRenderer);
			}

            //Disable text input
            SDL_StopTextInput();
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
	//Text rendering color
	SDL_Color textColor = {0, 0, 0, 0xFF};
	SDL_Color highlightColor = {0xFF, 0, 0, 0xFF};

    gFont = TTF_OpenFont("33_file_reading_and_writing/lazy.ttf", 28);
    if (gFont == NULL)
    {
        printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }
    else
    {
        //Render the prompt
        if(!gPromptTextTexture.loadFromRenderedText("Enter Data:", textColor))
        {
            printf("Failed to render prompt text!\n");
            return false;
        }
    }

	//Open file for reading in binary
	SDL_RWops* file = SDL_RWFromFile("33_file_reading_and_writing/nums.bin", "r+b");

    //File does not exist
	if(file == NULL)
	{
		printf("Warning: Unable to open file! SDL Error: %s\n", SDL_GetError());

		//Create file for writing
		file = SDL_RWFromFile("33_file_reading_and_writing/nums.bin", "w+b");
		if(file != NULL)
		{
			printf("New file created!\n");

			//Initialize data
			for (int i = 0; i < TOTAL_DATA; ++i)
			{
				gData[i] = 0;
				SDL_RWwrite(file, &gData[i], sizeof(Sint32), 1);
			}

			//Close file handler
			SDL_RWclose(file);
		}
		else
		{
			printf("Error: Unable to create file! SDL Error: %s\n", SDL_GetError());
			return false;
		}
	}
	//File exists
	else
	{
		//Load data
		printf("Reading file...!\n");
		for (int i = 0; i < TOTAL_DATA; ++i)
		{
			SDL_RWread(file, &gData[i], sizeof(Sint32), 1);
		}

		//Close file handler
		SDL_RWclose(file);
	}

	//Initialize data textures
	gDataTextures[0].loadFromRenderedText(std::to_string((long long)gData[0]), highlightColor);
	for (int i = 1; i < TOTAL_DATA; ++i) {
		gDataTextures[i].loadFromRenderedText(std::to_string((long long)gData[i]), textColor);
	}

	return true;
}

void close()
{
	//Open data for writing
	SDL_RWops* file = SDL_RWFromFile("33_file_reading_and_writing/nums.bin", "w+b");
	if(file != NULL)
	{
		//Save data
		for (int i = 0; i < TOTAL_DATA; ++i) {
			SDL_RWwrite(file, &gData[i], sizeof(Sint32), 1);
		}

		//Close file handler
		SDL_RWclose(file);
	}
	else
	{
		printf("Error: Unable to save file! %s\n", SDL_GetError());
	}

	//Free loaded images
    gPromptTextTexture.free();
	for (auto &gDataTexture : gDataTextures) {
		gDataTexture.free();
	}

    //Free global font
    TTF_CloseFont(gFont);

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
