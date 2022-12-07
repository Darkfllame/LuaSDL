#ifndef LUASDL_HPP
#define LUASDL_HPP

#define ENGINENAME "LuaSDL"
#define COLOR_TYPE_NAME "Color"
#define IMAGE_TYPE_NAME "Image"

void QuitSDL(), QuitAll();
void Update(), Render();

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

// engine macros
#define luaL_checkArgType(L, type, arg) \
	luaL_argcheck(L, lua_is##type##(L, arg), arg, (std::string(#type " expected, got") + lua_typename(L, arg)).c_str());

// engine functions
static void LoadEngine(lua_State* L);

// start SDL
// args : name(string), width(number), height(number), (optional) x(number), (optional) y(number)
// return (nil)
static int StartSDL(lua_State* L);

// return the window's dimention
// args :
// return width(number), height(number)
static int GetWindowDim(lua_State* L);
// set the window's dimention
// args : width(number), height(number)
// return (nil)
static int SetWindowDim(lua_State* L);
// return the window's position
// args :
// return x(number), y(number)
static int GetWindowPos(lua_State* L);
// set the window's position
// args : x(number), y(number)
// return (nil)
static int SetWindowPos(lua_State* L);

// get wether given mouse button is down
// args : button(integer)
// return boolean
static int IsMouseButtonDown(lua_State* L);
// get wether given mouse button is released
// args : button(integer)
// return boolean
static int IsMouseButtonReleased(lua_State* L);

// return if given key is pressed
// args : key (string)
// return boolean
static int IsKeyDown(lua_State* L);
// return if given key is released
// args : key (string)
// return boolean
static int IsKeyReleased(lua_State* L);

// returns the mouse position in pixels
// args :
// return integer, integer
static int GetMousePixPos(lua_State* L);
// returns the mouse position in percents
// args :
// return number, number
static int GetMousePos(lua_State* L);

// create a new image
// args : path(string)
// return Image
static int newImage(lua_State* L);
// return the path of the image
// args :
// return string 
static int ImagePath(lua_State* L);
static inline int lua_isimage(lua_State* L, int idx)
{
	return lua_isuserdata(L, idx) & (luaL_checkudata(L, idx, IMAGE_TYPE_NAME) != NULL);
}

// create a new image
// args : r(number),g(number),b(number),(optional, default : 255) a(number)
// return Color
static int newColor(lua_State* L);
// set a value of the color
// args : any, any
// return nil
static int ColorSet(lua_State* L);
// return or the green value of the color
// args : any
// return any
static int ColorGet(lua_State* L);
// convert color to a string
// args :
// return string
static int ColorToString(lua_State* L);
static inline int lua_iscolor(lua_State* L, int idx)
{
	return lua_isuserdata(L, idx) & (luaL_checkudata(L, idx, COLOR_TYPE_NAME) != NULL);
}

// set the background color
// args : color(Color)
// return (nil)
static int SetBgColor(lua_State* L);
// get the background color
// args :
// return Color
static int GetBgColor(lua_State* L);
// set the current draw color
// args : color(Color)
// return (nil)
static int SetDrawColor(lua_State* L);
// get the current draw color
// args :
// return color(Color)
static int GetDrawColor(lua_State* L);

// draw a rectange (outline)
// args : x(number), y(number), width(number), height(number)
// return (nil)
static int DrawRect(lua_State* L);
// draw a rectange
// args : x(number), y(number), width(number), height(number)
// return (nil)
static int FillRect(lua_State* L);
// draw a pixel
// args : x(number), y(number)
// return (nil)
static int DrawPixel(lua_State* L);
// draw a rectange
// args : image(Image), x(number), y(number)
// return (nil)
static int DrawImage(lua_State* L);
#endif