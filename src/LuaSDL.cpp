#include <iostream>
#include <ctime>
#include <fstream>
#include <string>

#include <include/SDL.h>
#include <include/SDL_image.h>
#include <include/lua.hpp>

#include "LuaSDL.hpp"

#pragma region main
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
lua_State* L = NULL;

bool SDLInited = false, IMGInited = false;
bool running = false;

const Uint8* keys;

double dt = 0.0, lt = 0.0;

Color bgColor = { 0,0,0,1 };

static void loop()
{
    running = true;

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) running = false;

            keys = SDL_GetKeyboardState(NULL);
        }

        // call the "update(dt)" function from lua code
        Update();

        // draw background
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderClear(renderer);

        // call the "render()" function from lua code
        Render();

        // make changements visible
        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char** argv)
{
    L = luaL_newstate();
    luaL_openlibs(L);

    LoadEngine(L);

    luaL_dofile(L, "main.lua");

    if (SDLInited)
    {
        loop();
    }

    QuitAll();
    return 0;
}

void QuitSDL()
{
    if (renderer != NULL)
        SDL_DestroyRenderer(renderer);
    if (window != NULL)
        SDL_DestroyWindow(window);
    if (SDLInited)
        SDL_Quit();
    if (IMGInited)
        IMG_Quit();
}
void QuitAll()
{
    QuitSDL();
    if (L != NULL) lua_close(L);
}

void Update()
{
    double now = ((double)clock()) / (double)CLOCKS_PER_SEC;
    dt = now - lt;
    lt = now;

    if (lua_getglobal(L, "update") > 0)
    {
        lua_pushnumber(L, dt);
        lua_call(L, 1, 0);
    }
}
void Render()
{
    if (lua_getglobal(L, "render") > 0)
    {
        lua_call(L, 0, 0);
    }
}
#pragma endregion

#pragma region engine
static const luaL_Reg Engine[] = {
    {"StartSDL", StartSDL},

    // window infos
    {"GetWindowDim", GetWindowDim},
    {"SetWindowDim", SetWindowDim},
    {"GetWindowPos", GetWindowPos},
    {"SetWindowPos", SetWindowPos},

    // user inputs
    {"IsMouseButtonDown", IsMouseButtonDown},
    {"IsMouseButtonReleased", IsMouseButtonReleased},

    {"IsKeyDown", IsKeyDown},
    {"IsKeyReleased", IsKeyReleased},

    {"GetMousePixPos", GetMousePixPos},
    {"GetMousePos", GetMousePos},


    // data types
    {"newImage", newImage},
    {"newColor", newColor},

    // colors thing
    {"SetBgColor", SetBgColor},
    {"GetBgColor", GetBgColor},
    {"SetDrawColor", SetDrawColor},
    {"GetDrawColor", GetDrawColor},

    // drawing
    {"DrawRect", DrawRect},
    {"FillRect", FillRect},
    {"DrawPixel", DrawPixel},
    {"DrawImage", DrawImage},
    {NULL, NULL}
};
static const luaL_Reg Color_m[] = {
    {"__newindex", ColorSet},
    {"__index", ColorGet},
    {"__tostring", ColorToString},
    {NULL, NULL}
};
static const luaL_Reg Image_m[] = {
    {"path", ImagePath},
    {NULL, NULL}
};

static void LoadEngine(lua_State* L)
{
    Uint32 imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (IMG_Init(imgFlags) != imgFlags)
    {
        std::cout << "Can't initialize SDL_image :\n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    IMGInited = true;

    luaL_newmetatable(L, COLOR_TYPE_NAME);
    luaL_setfuncs(L, Color_m, 0);

    luaL_newmetatable(L, IMAGE_TYPE_NAME);
    luaL_setfuncs(L, Image_m, 0);

    luaL_newlib(L, Engine);
    lua_setglobal(L, ENGINENAME);
}

static int StartSDL(lua_State* L)
{
    if (SDLInited) return 0;
    int argc = lua_gettop(L);
    const char* title = (argc > 0) ? lua_tostring(L, 1) : ENGINENAME;
    int width = (argc > 1) ? (int)lua_tonumber(L, 2) : 800;
    int height = (argc > 2) ? (int)lua_tonumber(L, 3) : 600;
    int x = (argc > 3) ? (int)lua_tonumber(L, 4) : SDL_WINDOWPOS_CENTERED;
    int y = (argc > 4) ? (int)lua_tonumber(L, 5) : SDL_WINDOWPOS_CENTERED;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "Can't initialize SDL :\n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    SDLInited = true;
    window = SDL_CreateWindow(title, x, y, width, height, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        std::cout << "Can't create window : \n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        std::cout << "Can't create renderer : \n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }

    return 0;
}

// window infos
static int GetWindowDim(lua_State* L)
{
    if (!SDLInited) return 0;
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
}
static int SetWindowDim(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, integer, 1);
    luaL_checkArgType(L, integer, 2);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    w = (int)lua_tointeger(L, 1);
    h = (int)lua_tointeger(L, 2);
    SDL_SetWindowSize(window, w, h);
    return 0;
}
static int GetWindowPos(lua_State* L)
{
    if (!SDLInited) return 0;
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    lua_pushnumber(L, y);
    lua_pushnumber(L, x);
    return 2;
}
static int SetWindowPos(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, integer, 1);
    luaL_checkArgType(L, integer, 2);

    int x, y;
    SDL_GetWindowSize(window, &x, &y);
    x = (int)lua_tointeger(L, 1);
    y = (int)lua_tointeger(L, 2);
    SDL_SetWindowPosition(window, x, y);
    return 0;
}

// user inputs
static int IsMouseButtonDown(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, integer, 1);

    Uint32 btn = SDL_GetMouseState(NULL, NULL);

    int button = (int)lua_tointeger(L, 1);

    lua_pushboolean(L, btn & SDL_BUTTON(button));

    return 1;
}
static int IsMouseButtonReleased(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, integer, 1);

    Uint32 btn = SDL_GetMouseState(NULL, NULL);

    int button = (int)lua_tointeger(L, 1);

    lua_pushboolean(L, !(btn & SDL_BUTTON(button)));

    return 1;
}

static int IsKeyDown(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, string, 1);

    const char* key = lua_tostring(L, 1);

    lua_pushboolean(L, 0);
    if (keys[SDL_GetScancodeFromName(key)])
        lua_pushboolean(L, 1);

    return 1;
}
static int IsKeyReleased(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, string, 1);

    const char* key = lua_tostring(L, 1);

    lua_pushboolean(L, 0);
    if (!keys[SDL_GetScancodeFromName(key)])
        lua_pushboolean(L, 1);

    return 1;
}

static int GetMousePixPos(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    lua_pushinteger(L, mx);
    lua_pushinteger(L, my);

    return 2;
}
static int GetMousePos(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);

    int mx, my;
    int width, height;
    SDL_GetMouseState(&mx, &my);
    SDL_GetWindowSize(window, &width, &height);

    lua_pushinteger(L, (int)(mx / width));
    lua_pushinteger(L, (int)(my / height));

    return 2;
}

// data types
static int newImage(lua_State* L)
{
    if (!IMGInited) return 0;
    int argc = lua_gettop(L);
    const char* fn = (argc > 0) ? lua_tostring(L, 1) : "";
    Image* image = (Image*)lua_newuserdata(L, sizeof(Image*));
    image->surf = IMG_Load(fn);
    if (image->surf == NULL)
    {
        std::cout << "Can't create image : \n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    image->path = fn;
    luaL_getmetatable(L, IMAGE_TYPE_NAME);
    lua_setmetatable(L, -2);
    return 1;
}
static int ImagePath(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, image, 1);

    lua_pushstring(L, ((Image*)lua_touserdata(L, 1))->path);

    return 1;
}

static int newColor(lua_State* L)
{
    int argc = lua_gettop(L);

    Color* col = (Color*)lua_newuserdata(L, sizeof(Color));
    col->r = (argc > 0) ? (Uint8)lua_tonumber(L, 1) : 0;
    col->g = (argc > 1) ? (Uint8)lua_tonumber(L, 2) : 0;
    col->b = (argc > 2) ? (Uint8)lua_tonumber(L, 3) : 0;
    col->a = (argc > 3) ? (Uint8)lua_tonumber(L, 4) : 255;
    luaL_getmetatable(L, COLOR_TYPE_NAME);
    lua_setmetatable(L, -2);

    return 1;
}
static int ColorSet(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, color, 1);
    luaL_checkArgType(L, string, 2);
    luaL_checkArgType(L, integer, 3);

    Color* col = (Color*)lua_touserdata(L, 1);
    char name = lua_tostring(L, 2)[0];
    long long val = lua_tointeger(L, 3);

    if (name == 'r' || name == 'g' || name == 'b' || name == 'a')
    {
        if (name == 'r')
        {
            col->r = (Uint8)val;
        }
        else if (name == 'g')
        {
            col->g = (Uint8)val;
        }
        else if (name == 'b')
        {
            col->b = (Uint8)val;
        }
        else if (name == 'a')
        {
            col->a = (Uint8)val;
        }
    }
    
    return 0;
}
static int ColorGet(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, color, 1);
    luaL_checkArgType(L, string, 2);

    Color* col = (Color*)lua_touserdata(L, 1);
    char name = lua_tostring(L, 2)[0];
    long long val = 0;

    if (name == 'r' || name == 'g' || name == 'b' || name == 'a')
    {
        if (name == 'r')
        {
            val = col->r;
        }
        else if (name == 'g')
        {
            val = col->g;
        }
        else if (name == 'b')
        {
            val = col->b;
        }
        else if (name == 'a')
        {
            val = col->a;
        }

        lua_pushinteger(L, val);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}
static int ColorToString(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, color, 1);

    Color* col = (Color*)lua_touserdata(L, 1);

    lua_pushfstring(L, ENGINENAME".Color %d, %d, %d, %d", col->r, col->g, col->b, col->a);

    return 1;
}

// color thing
static int SetBgColor(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, userdata, 1);

    bgColor = *(Color*)lua_touserdata(L, 1);

    return 0;
}
static int GetBgColor(lua_State* L)
{
    if (!SDLInited) return 0;
    Color* col = (Color*)lua_newuserdata(L, sizeof(Color));
    col->r = bgColor.r;
    col->g = bgColor.g;
    col->b = bgColor.b;
    col->a = bgColor.a;

    luaL_getmetatable(L, COLOR_TYPE_NAME);
    lua_setmetatable(L, -2);

    return 1;
}
static int SetDrawColor(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, userdata, 1);

    Color* col = (Color*)lua_touserdata(L, 1);

    SDL_SetRenderDrawColor(renderer, col->r, col->g, col->b, col->a);
    return 0;
}
static int GetDrawColor(lua_State* L)
{
    if (!SDLInited) return 0;
    Color* col = (Color*)lua_newuserdata(L, sizeof(Color));
    SDL_GetRenderDrawColor(renderer, &col->r, &col->g, &col->b, &col->a);

    luaL_getmetatable(L, COLOR_TYPE_NAME);
    lua_setmetatable(L, -2);

    return 1;
}

// drawing
static int DrawRect(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, number, 1);
    luaL_checkArgType(L, number, 2);
    luaL_argcheck(L,
        lua_isnumber(L, 3) || lua_isnil(L, 3),
        3,
        std::string("number expected, got ", lua_typename(L, 3)).c_str()
    );

    int x = (argc > 0) ? (int)lua_tonumber(L, 1) : 0;
    int y = (argc > 1) ? (int)lua_tonumber(L, 2) : 0;
    // if no more arguments
    if (argc <= 2) {
        SDL_RenderDrawPoint(renderer, x, y);
        return 0;
    }

    int w = (argc > 2) ? (int)lua_tonumber(L, 3) : 0;
    int h = (argc > 3) ? (int)lua_tonumber(L, 4) : 0;

    SDL_Rect rect = { x,y,w,h };

    SDL_RenderDrawRect(renderer, &rect);

    return 0;
}
static int FillRect(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, number, 1);
    luaL_checkArgType(L, number, 2);
    luaL_argcheck(L,
        lua_isnumber(L, 3) || lua_isnil(L, 3),
        3,
        std::string("number expected, got ", lua_typename(L, 3)).c_str()
    );

    int x = (argc > 0) ? (int)lua_tonumber(L, 1) : 0;
    int y = (argc > 1) ? (int)lua_tonumber(L, 2) : 0;
    // if no more arguments
    if (argc <= 2) {
        SDL_RenderDrawPoint(renderer, x, y);
        return 0;
    }

    int w = (argc > 2) ? (int)lua_tonumber(L, 3) : 0;
    int h = (argc > 3) ? (int)lua_tonumber(L, 4) : 0;

    SDL_Rect rect = { x,y,w,h };

    SDL_RenderFillRect(renderer, &rect);

    return 0;
}
static int DrawPixel(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, number, 1);
    luaL_checkArgType(L, number, 2);

    int x = (argc > 0) ? (int)lua_tonumber(L, 1) : 0;
    int y = (argc > 0) ? (int)lua_tonumber(L, 2) : 0;

    SDL_RenderDrawPoint(renderer, x, y);

    return 0;
}
static int DrawImage(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, userdata, 1);
    luaL_checkArgType(L, integer, 2);
    luaL_checkArgType(L, integer, 3);

    Image* img = (Image*)lua_touserdata(L, 1);
    SDL_Surface* image = img->surf;
    int x = (argc > 1) ? (int)lua_tonumber(L, 2) : 0;
    int y = (argc > 2) ? (int)lua_tonumber(L, 3) : 0;

    const SDL_Rect* drawRect = &image->clip_rect;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, image);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    SDL_RenderCopy(renderer, tex, NULL, drawRect);

    return 0;
}
#pragma endregion