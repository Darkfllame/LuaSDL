#ifndef LUASDL_HPP
#define LUASDL_HPP

#define ENGINENAME "LuaSDL"

#define COLOR_TYPE_NAME "Color"
#define IMAGE_TYPE_NAME "Image"
#define SOUND_TYPE_NAME "Sound"

// engine types
typedef struct Image
{
	SDL_Surface* surf;
	const char* path;
} Image;
typedef struct Color
{
	Uint8 r, g, b, a;
} Color;
typedef struct Sound
{
	Mix_Chunk* snd;
	const char* path;
	int channel;
} Sound;

void loop();

void QuitSDL(), QuitAll();
void Update(), Render();

static const int maxChannels = MIX_CHANNELS;
static bool channels[maxChannels] = {
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	0
};

// engine macros
#define luaL_checkArgType(L, type, arg) \
	luaL_argcheck(L, lua_is##type##(L, arg), arg, (std::string(#type " expected, got ") + lua_typename(L, arg)).c_str());

// engine functions
void LoadEngine(lua_State* L);

// start SDL
// args : name(string), width(number), height(number), (optional) x(number), (optional) y(number)
// return (nil)
static int LuaSDL_Start(lua_State* L);
// poll events
// args :
// return (nil)
static int LuaSDL_PollEvents(lua_State* L);

// return the window's dimention
// args :
// return width(number), height(number)
static int LuaSDL_Window_GetSize(lua_State* L);
// set the window's dimention
// args : width(number), height(number)
// return (nil)
static int LuaSDL_Window_SetSize(lua_State* L);
// return the window's position
// args :
// return x(number), y(number)
static int LuaSDL_Window_GetPos(lua_State* L);
// set the window's position
// args : x(number), y(number)
// return (nil)
static int LuaSDL_Window_SetPos(lua_State* L);

// get wether given mouse button is down
// args : button(integer)
// return boolean
static int LuaSDL_Input_IsMouseButtonDown(lua_State* L);
// get wether given mouse button is released
// args : button(integer)
// return boolean
static int LuaSDL_Input_IsMouseButtonReleased(lua_State* L);

// return if given key is pressed
// args : key (string)
// return boolean
static int LuaSDL_Input_IsKeyDown(lua_State* L);
// return if given key is released
// args : key (string)
// return boolean
static int LuaSDL_Input_IsKeyReleased(lua_State* L);

// returns the mouse position in pixel coordinates
// args :
// return number, number
static int LuaSDL_Input_GetMousePos(lua_State* L);

// create a new image
// args : path(string)
// return Image
static int Image_new(lua_State* L);
// the __newindex metamethod for Image datatype
static int ImageSet(lua_State* L);
// the __index metamethod for Image datatype
static int ImageGet(lua_State* L);
static int ImageToString(lua_State* L);
static int ImageGC(lua_State* L);
static inline int lua_isimage(lua_State* L, int idx)
{
	return lua_isuserdata(L, idx) & (luaL_checkudata(L, idx, IMAGE_TYPE_NAME) != NULL);
}

// reload given image with filename
static void reloadImage(Image* img, const char* fn);

// create a new image
// args : r(number),g(number),b(number),(optional, default : 255) a(number)
// return Color
static int Color_new(lua_State* L);
// the __newindex metamethod for Color datatype
static int ColorSet(lua_State* L);
// the __index metamethod for Color datatype
static int ColorGet(lua_State* L);
// the __mul metamethod for Color datatype
static int ColorMul(lua_State* L);
// the __div metamethod for Color datatype
static int ColorDiv(lua_State* L);
static int ColorToString(lua_State* L);
static inline int lua_iscolor(lua_State* L, int idx)
{
	return lua_isuserdata(L, idx) & (luaL_checkudata(L, idx, COLOR_TYPE_NAME) != NULL);
}

// create a new sound
// args : path (string)
// return Sound
static int Sound_new(lua_State* L);
// the __newindex metamethod for Sound datatype
static int SoundSet(lua_State* L);
// the __index metamethod for Sound datatype
static int SoundGet(lua_State* L);
// play the given sound
// args :
// return nil
static int Sound_PlaySound(lua_State* L);
// pause the given sound
// args :
// return nil
static int Sound_PauseSound(lua_State* L);
// stop the given sound
// args :
// return nil
static int Sound_StopSound(lua_State* L);
// return wether given sound is playing or not
// args :
// return boolean
static int Sound_IsSoundPlaying(lua_State* L);
// return wether given sound is paused or not
// args :
// return boolean
static int Sound_IsSoundPaused(lua_State* L);
static int SoundToString(lua_State* L);
static int SoundGC(lua_State* L);
static int lua_issound(lua_State* L, int idx)
{
	return lua_isuserdata(L, idx) & (luaL_checkudata(L, idx, SOUND_TYPE_NAME) != NULL);
}

static void reloadSound(Sound* snd, const char* fn);

// copy given values
// args : ... (any)
// return any
static int LuaSDL_Copy(lua_State* L);

// set the background color
// args : color(Color)
// return (nil)
static int LuaSDL_Background_SetColor(lua_State* L);
// get the background color
// args :
// return Color
static int LuaSDL_Background_GetColor(lua_State* L);
// set the current draw color
// args : color(Color)
// return (nil)
static int LuaSDL_Drawing_SetColor(lua_State* L);
// get the current draw color
// args :
// return color(Color)
static int LuaSDL_Drawing_GetColor(lua_State* L);

// draw a rectange (outline)
// args : x(number), y(number), width(number), height(number)
// return (nil)
static int LuaSDL_Drawing_DrawRect(lua_State* L);
// draw a rectange
// args : x(number), y(number), width(number), height(number)
// return (nil)
static int LuaSDL_Drawing_FillRect(lua_State* L);
// draw a pixel
// args : x(number), y(number)
// return (nil)
static int LuaSDL_Drawing_DrawPixel(lua_State* L);
// draw an image
// args : image(Image), x(integer), y(integer)
// return (nil)
static int LuaSDL_Drawing_DrawImage(lua_State* L);
#endif