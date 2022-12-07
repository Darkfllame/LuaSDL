#include <iostream>
#include <ctime>
#include <fstream>
#include <string>

#include <signal.h>

#include <include/SDL.h>
#include <include/SDL_image.h>
#include <include/SDL_mixer.h>

#include <include/lua.hpp>

#include "LuaSDL.hpp"

#pragma region Main
// the window
SDL_Window* window = NULL;
// the renderer of the window
SDL_Renderer* renderer = NULL;
// the lua state (for lua host)
lua_State* L = NULL;

bool SDLInited = false, IMGInited = false, MIXInited = false;
bool running = false;

const Uint8* keys;

double dt = 0.0, lt = 0.0;

Color bgColor = { 0,0,0,255 };

int pmain(lua_State* L)
{
    int argc = lua_tointeger(L, 1);
    char** argv = (char**)lua_touserdata(L, 2);

    lua_gc(L, LUA_GCGEN, 0, 0);

    LoadEngine(L);

    luaL_dofile(L, "main.lua");

    if (SDLInited)
    {
        loop();
    }

    return 0;
}
int main(int argc, char** argv) {
    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcfunction(L, pmain);
    lua_pushinteger(L, (lua_Integer)argc);
    lua_pushlightuserdata(L, argv);
    lua_pcall(L, 2, 0, 0);

    QuitAll();
    return 0;
}

void loop()
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


    if (lua_getglobal(L, "update") == LUA_TFUNCTION)
    {
        lua_pushnumber(L, dt);
        lua_pcall(L, 1, 0, 0);
    }
}
void Render()
{
    if (lua_getglobal(L, "render") == LUA_TFUNCTION)
    {
        lua_pcall(L, 0, 0, 0);
    }
}
#pragma endregion

#pragma region Engine
static const luaL_Reg Engine_t[] = {
    {"Start", LuaSDL_Start},
    {"Copy", LuaSDL_Copy},
    {"PollEvents", LuaSDL_PollEvents},
    {NULL, NULL}
};
static const luaL_Reg Engine_Window_t[] = {
    {"GetSize", LuaSDL_Window_GetSize},
    {"SetSize", LuaSDL_Window_SetSize},
    {"GetPos", LuaSDL_Window_GetPos},
    {"SetPos", LuaSDL_Window_SetPos},
    {NULL, NULL}
};
static const luaL_Reg Engine_Input_t[] = {
    {"IsKeyDown", LuaSDL_Input_IsKeyDown},
    {"IsKeyReleased", LuaSDL_Input_IsKeyReleased},

    {"IsMouseButtonDown", LuaSDL_Input_IsMouseButtonDown},
    {"IsMouseButtonReleased", LuaSDL_Input_IsMouseButtonReleased},

    {"GetMousePos", LuaSDL_Input_GetMousePos},
    {NULL, NULL}
};
static const luaL_Reg Engine_Background_t[] = {
    {"SetColor", LuaSDL_Background_SetColor},
    {"GetColor", LuaSDL_Background_GetColor},
    {NULL, NULL}
};
static const luaL_Reg Engine_Drawing_t[] = {
    // colors thing
    {"SetColor", LuaSDL_Drawing_SetColor},
    {"GetColor", LuaSDL_Drawing_GetColor},

    // drawing
    {"DrawRect", LuaSDL_Drawing_DrawRect},
    {"FillRect", LuaSDL_Drawing_FillRect},
    {"DrawPixel", LuaSDL_Drawing_DrawPixel},
    {"DrawImage", LuaSDL_Drawing_DrawImage},
    {NULL, NULL}
};

static const luaL_Reg Color_t[] = {
    {"new", Color_new},
    {NULL, NULL}
};
static const luaL_Reg Image_t[] = {
    {"new", Image_new},
    {NULL, NULL}
};
static const luaL_Reg Sound_t[] = {
    {"new", Sound_new},
    {NULL, NULL}
};

static const luaL_Reg Color_mt[] = {
    {"__index", ColorGet},
    {"__newindex", ColorSet},
    {"__tostring", ColorToString},
    {NULL, NULL}
};
static const luaL_Reg Image_mt[] = {
    {"__index", ImageGet},
    {"__newindex", ImageSet},
    {"__tostring", ImageToString},
    {"__gc", ImageGC},
    {NULL, NULL}
};
static const luaL_Reg Sound_mt[] = {
    {"__index", SoundGet},
    {"__newindex", SoundSet},
    {"__tostring", SoundToString},
    {"__gc", SoundGC},

    {"Play", Sound_PlaySound},
    {"Pause", Sound_PauseSound},
    {"Stop", Sound_StopSound},
    {"IsPlaying", Sound_IsSoundPlaying},
    {"IsPaused", Sound_IsSoundPaused},
    {NULL, NULL}
};

void LoadEngine(lua_State* L)
{
    Uint32 imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (IMG_Init(imgFlags) != imgFlags)
    {
        std::cout << "Can't initialize SDL_image :\n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    IMGInited = true;
    Uint32 mixFlags = MIX_INIT_MP3 | MIX_INIT_OGG;
    if (Mix_Init(mixFlags) != mixFlags)
    {
        std::cout << "Can't initialize SDL_mixer :\n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    MIXInited = true;


    // datatypes metatables
    luaL_newmetatable(L, COLOR_TYPE_NAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, Color_mt, 0);

    luaL_newmetatable(L, IMAGE_TYPE_NAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, Image_mt, 0);

    luaL_newmetatable(L, SOUND_TYPE_NAME);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, Sound_mt, 0);


    // [ENGINENAME]
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Engine_t, 0);

    // [ENGINENAME].Window
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Engine_Window_t, 0);
    lua_setfield(L, -2, "Window");
    // [ENGINENAME].Input
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Engine_Input_t, 0);
    lua_setfield(L, -2, "Input");
    // [ENGINENAME].Background
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Engine_Background_t, 0);
    lua_setfield(L, -2, "Background");
    // [ENGINENAME].Drawing
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Engine_Drawing_t, 0);
    lua_setfield(L, -2, "Drawing");

    lua_setglobal(L, ENGINENAME);

    // [COLOR_TYPE_NAME]
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Color_t, 0);
    lua_setglobal(L, COLOR_TYPE_NAME);
    // [IMAGE_TYPE_NAME]
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Image_t, 0);
    lua_setglobal(L, IMAGE_TYPE_NAME);
    // [SOUND_TYPE_NAME]
    lua_createtable(L, 0, 0);
    luaL_setfuncs(L, Sound_t, 0);
    lua_setglobal(L, SOUND_TYPE_NAME);
}

static int LuaSDL_Start(lua_State* L)
{
    if (SDLInited) return 0;
    int argc = lua_gettop(L);
    const char* title = (argc > 0 && !lua_isnoneornil(L, 1)) ? lua_tostring(L, 1) : ENGINENAME;
    int width = (argc > 1 && !lua_isnoneornil(L, 2)) ? (int)lua_tonumber(L, 2) : 800;
    int height = (argc > 2 && !lua_isnoneornil(L, 3)) ? (int)lua_tonumber(L, 3) : 600;
    int x = (argc > 3 && !lua_isnoneornil(L, 4)) ? (int)lua_tonumber(L, 4) : SDL_WINDOWPOS_CENTERED;
    int y = (argc > 4 && !lua_isnoneornil(L, 5)) ? (int)lua_tonumber(L, 5) : SDL_WINDOWPOS_CENTERED;
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
static int LuaSDL_PollEvents(lua_State* L)
{
    int argc = lua_gettop(L);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) running = false;

        keys = SDL_GetKeyboardState(NULL);
    }

    return 0;
}

// window infos
static int LuaSDL_Window_GetSize(lua_State* L)
{
    if (!SDLInited) return 0;
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    lua_pushnumber(L, w);
    lua_pushnumber(L, h);
    return 2;
}
static int LuaSDL_Window_SetSize(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, number, 1);
    luaL_checkArgType(L, number, 2);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    w = (int)lua_tonumber(L, 1);
    h = (int)lua_tonumber(L, 2);
    SDL_SetWindowSize(window, w, h);
    return 0;
}
static int LuaSDL_Window_GetPos(lua_State* L)
{
    if (!SDLInited) return 0;
    int x, y;
    SDL_GetWindowPosition(window, &x, &y);
    lua_pushnumber(L, y);
    lua_pushnumber(L, x);
    return 2;
}
static int LuaSDL_Window_SetPos(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, number, 1);
    luaL_checkArgType(L, number, 2);

    int x, y;
    SDL_GetWindowSize(window, &x, &y);
    x = (int)lua_tonumber(L, 1);
    y = (int)lua_tonumber(L, 2);
    SDL_SetWindowPosition(window, x, y);
    return 0;
}

// user inputs
static int LuaSDL_Input_IsKeyDown(lua_State* L)
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
static int LuaSDL_Input_IsKeyReleased(lua_State* L)
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


static int LuaSDL_Input_IsMouseButtonDown(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, number, 1);

    Uint32 btn = SDL_GetMouseState(NULL, NULL);

    int button = (int)lua_tonumber(L, 1);

    lua_pushboolean(L, btn & SDL_BUTTON(button));

    return 1;
}
static int LuaSDL_Input_IsMouseButtonReleased(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, number, 1);

    Uint32 btn = SDL_GetMouseState(NULL, NULL);

    int button = (int)lua_tonumber(L, 1);

    lua_pushboolean(L, !(btn & SDL_BUTTON(button)));

    return 1;
}

static int LuaSDL_Input_GetMousePos(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);

    int mx, my;
    SDL_GetMouseState(&mx, &my);

    lua_pushnumber(L, mx);
    lua_pushnumber(L, my);

    return 2;
}

// data types
static int Image_new(lua_State* L)
{
    if (!IMGInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, string, 1);
    const char* fn = lua_tostring(L, 1);

    Image* img = (Image*)lua_newuserdata(L, sizeof(Image));
    if (img == NULL)
    {
        std::cout << "Can't create image : \n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    reloadImage(img, fn);

    luaL_getmetatable(L, IMAGE_TYPE_NAME);
    lua_setmetatable(L, -2);

    return 1;
}
static int ImageSet(lua_State* L)
{
    int argc = lua_gettop(L);
    Image* img = (Image*)lua_touserdata(L, 1);
    const char* k = lua_tostring(L, 2);
    const char* v = lua_tostring(L, 3);

    lua_pushstring(L, "path");

    if (lua_compare(L, 3, 4, LUA_OPEQ)) {
        reloadImage(img, v);
    }

    return 0;
}
static int ImageGet(lua_State* L)
{
    int argc = lua_gettop(L);
    Image* img = (Image*)lua_touserdata(L, 1);

    lua_pushstring(L, "path");      //3
    lua_pushstring(L, "width");     //4
    lua_pushstring(L, "height");    //5

    lua_pushnil(L);

    if (lua_compare(L, 2, 3, LUA_OPEQ))
        lua_pushstring(L, img->path);
    else if (lua_compare(L, 2, 4, LUA_OPEQ))
        lua_pushnumber(L, img->surf->clip_rect.w);
    else if (lua_compare(L, 2, 5, LUA_OPEQ))
        lua_pushnumber(L, img->surf->clip_rect.h);

    return 1;
}
static int ImageToString(lua_State* L)
{
    int argc = lua_gettop(L);

    Image* img = (Image*)lua_touserdata(L, 1);

    lua_pushfstring(L, IMAGE_TYPE_NAME " %s", img->path);

    return 1;
}
static int ImageGC(lua_State* L)
{
    int argc = lua_gettop(L);
    Image* img = (Image*)lua_touserdata(L, 1);

    SDL_DestroyTexture(SDL_CreateTextureFromSurface(renderer, img->surf));
    return 0;
}

static void reloadImage(Image* img, const char* fn)
{
    img->surf = IMG_Load(fn);
    if (img->surf == NULL)
    {
        std::cout << "Can't create image : \n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    img->path = fn;
}

static int Color_new(lua_State* L)
{
    int argc = lua_gettop(L);

    Color* col = (Color*)lua_newuserdata(L, sizeof(Color));
    if (col == NULL)
    {
        std::cout << "Can't create color :\n" << std::endl;
        QuitAll();
        exit(1);
    }
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
    Color* col = (Color*)lua_touserdata(L, 1);
    const char* k = lua_tostring(L, 2);
    int v = (int)lua_tonumber(L, 3);

    lua_pushstring(L, "r");
    lua_pushstring(L, "g");
    lua_pushstring(L, "b");
    lua_pushstring(L, "a");

    if (lua_compare(L, 2, 4, LUA_OPEQ))
        col->r = (Uint8)v;
    else if (lua_compare(L, 2, 5, LUA_OPEQ))
        col->g = (Uint8)v;
    else if (lua_compare(L, 2, 6, LUA_OPEQ))
        col->b = (Uint8)v;
    else if (lua_compare(L, 2, 7, LUA_OPEQ))
        col->a = (Uint8)v;

    return 0;
}
static int ColorGet(lua_State* L)
{
    int argc = lua_gettop(L);
    Color* col = (Color*)lua_touserdata(L, 1);
    const char* k = lua_tostring(L, 2);

    lua_pushstring(L, "r");
    lua_pushstring(L, "g");
    lua_pushstring(L, "b");
    lua_pushstring(L, "a");

    lua_pushnil(L);

    if (lua_compare(L, 2, 3, LUA_OPEQ))
        lua_pushnumber(L, (double)col->r);
    else if (lua_compare(L, 2, 4, LUA_OPEQ))
        lua_pushnumber(L, (double)col->g);
    else if (lua_compare(L, 2, 5, LUA_OPEQ))
        lua_pushnumber(L, (double)col->b);
    else if (lua_compare(L, 2, 6, LUA_OPEQ))
        lua_pushnumber(L, (double)col->a);

    return 1;
}
static int ColorMul(lua_State* L)
{
    int argc = lua_gettop(L);
    Color* col = (Color*)lua_touserdata(L, 1);
    double n = lua_tonumber(L, 2);

    Color* finalcol = (Color*)lua_newuserdata(L, sizeof(Color));
    if (col == NULL)
    {
        std::cout << "Can't create color :\n" << std::endl;
        QuitAll();
        exit(1);
    }
    finalcol->r = (Uint8)((double)col->r * n / 255.0);
    finalcol->g = (Uint8)((double)col->g * n / 255.0);
    finalcol->b = (Uint8)((double)col->b * n / 255.0);
    finalcol->a = (Uint8)((double)col->a * n / 255.0);
    luaL_setmetatable(L, COLOR_TYPE_NAME);

    return 1;
}
static int ColorDiv(lua_State* L)
{
    int argc = lua_gettop(L);
    Color* col = (Color*)lua_touserdata(L, 1);
    double n = lua_tonumber(L, 2);

    Color* finalcol = (Color*)lua_newuserdata(L, sizeof(Color));
    if (col == NULL)
    {
        std::cout << "Can't create color :\n" << std::endl;
        QuitAll();
        exit(1);
    }
    finalcol->r = (Uint8)(((double)col->r / 255.0) / n);
    finalcol->g = (Uint8)(((double)col->g / 255.0) / n);
    finalcol->b = (Uint8)(((double)col->b / 255.0) / n);
    finalcol->a = (Uint8)(((double)col->a / 255.0) / n);
    luaL_setmetatable(L, COLOR_TYPE_NAME);

    return 1;
}
static int ColorToString(lua_State* L)
{
    int argc = lua_gettop(L);

    Color* col = (Color*)lua_touserdata(L, 1);

    lua_pushfstring(L, COLOR_TYPE_NAME " %d, %d, %d, %d", col->r, col->g, col->b, col->a);

    return 1;
}

static int Sound_new(lua_State* L)
{
    if (!MIXInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, string, 1);
    const char* fn = lua_tostring(L, 1);

    Sound* snd = (Sound*)lua_newuserdata(L, sizeof(Sound));
    if (snd == NULL)
    {
        std::cout << "Can't create sound :\n" << std::endl;
        QuitAll();
        exit(1);
    }
    reloadSound(snd, fn);

    snd->channel = -1;

    for (int i = 0; i < maxChannels; i++)
    {
        if (channels[i] == false)
        {
            snd->channel = i;
            channels[i] = true;
        }
    }

    if (snd->channel == -1)
    {
        std::cout << "trying to make a new sound be all channels are taken\ntip:10 playing sounds max" << std::endl;
        QuitAll();
        exit(1);
    }

    luaL_getmetatable(L, SOUND_TYPE_NAME);
    lua_setmetatable(L, -2);

    return 1;
}
static int SoundSet(lua_State* L)
{
    int argc = lua_gettop(L);
    Sound* snd = (Sound*)lua_touserdata(L, 1);
    const char* k = lua_tostring(L, 2);
    const char* v = lua_tostring(L, 3);

    lua_pushstring(L, "path");

    if (lua_compare(L, 2, 3, LUA_OPEQ))
        reloadSound(snd, v);

    return 0;
}
static int SoundGet(lua_State* L)
{
    int argc = lua_gettop(L);
    Sound* snd = (Sound*)lua_touserdata(L, 1);
    const char* k = lua_tostring(L, 2);

    lua_pushstring(L, "path");

    lua_pushnil(L);

    if (lua_compare(L, 2, 3, LUA_OPEQ))
        lua_pushstring(L, snd->path);

    return 1;
}
static int Sound_PlaySound(lua_State* L)
{
    if (!MIXInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, sound, 1);
    Sound* snd = (Sound*)lua_touserdata(L, 1);
    int ch = snd->channel;

    Mix_PlayChannel(ch, snd->snd, 0);

    return 0;
}
static int Sound_PauseSound(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, sound, 1);
    Sound* snd = (Sound*)lua_touserdata(L, 1);
    int ch = snd->channel;

    Mix_Pause(ch);

    return 0;
}
static int Sound_StopSound(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, sound, 1);
    Sound* snd = (Sound*)lua_touserdata(L, 1);
    int ch = snd->channel;

    Mix_Resume(ch);

    return 0;
}
static int Sound_IsSoundPlaying(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, sound, 1);
    Sound* snd = (Sound*)lua_touserdata(L, 1);
    int ch = snd->channel;

    lua_pushboolean(L, Mix_Playing(ch));

    return 1;
}
static int Sound_IsSoundPaused(lua_State* L)
{
    int argc = lua_gettop(L);
    luaL_checkArgType(L, sound, 1);
    Sound* snd = (Sound*)lua_touserdata(L, 1);
    int ch = snd->channel;

    lua_pushboolean(L, Mix_Paused(ch));

    return 1;
}
static int SoundToString(lua_State* L)
{
    int argc = lua_gettop(L);

    Sound* snd = (Sound*)lua_touserdata(L, 1);

    lua_pushfstring(L, SOUND_TYPE_NAME " %s", snd->path);

    return 1;
}
static int SoundGC(lua_State* L)
{
    int argc = lua_gettop(L);
    Sound* snd = (Sound*)lua_touserdata(L, 1);

    channels[snd->channel] = false;
    return 0;
}

static void reloadSound(Sound* snd, const char* fn)
{
    snd->snd = Mix_LoadWAV(fn);
    if (snd->snd == NULL)
    {
        std::cout << "Can't create sound :\n" << SDL_GetError() << std::endl;
        QuitAll();
        exit(1);
    }
    snd->path = fn;
}

static int LuaSDL_Copy(lua_State* L)
{
    int argc = lua_gettop(L);

    for (int i = argc; i >= 1; i--)
    {
        if (lua_isuserdata(L, i))
        {
            void* ud = lua_touserdata(L, i);
            lua_getmetatable(L, i);
            lua_getfield(L, -1, "__name");
            if (lua_tostring(L, -1) == COLOR_TYPE_NAME)
            {
                Color* col = (Color*)ud;
                lua_getglobal(L, COLOR_TYPE_NAME);
                lua_getfield(L, -1, "new");
                lua_pushnumber(L, col->r);   //r
                lua_pushnumber(L, col->g);   //g
                lua_pushnumber(L, col->b);   //b
                lua_pushnumber(L, col->a);   //a
                lua_call(L, 4, 1);
            }
            else if (lua_tostring(L, -1) == IMAGE_TYPE_NAME)
            {
                Image* img = (Image*)ud;
                lua_getglobal(L, IMAGE_TYPE_NAME);
                lua_getfield(L, -1, "new");
                lua_pushstring(L, img->path);   //path
                lua_call(L, 1, 1);
            }
            else if (lua_tostring(L, -1) == SOUND_TYPE_NAME)
            {
                Sound* snd = (Sound*)ud;
                lua_getglobal(L, SOUND_TYPE_NAME);
                lua_getfield(L, -1, "new");
                lua_pushstring(L, snd->path);   //path
                lua_call(L, 1, 1);
            }
        }
        else
            lua_pushvalue(L, i);
    }

    return 1;
}

// color thing
static int LuaSDL_Background_SetColor(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, userdata, 1);

    bgColor = *(Color*)lua_touserdata(L, 1);

    return 0;
}
static int LuaSDL_Background_GetColor(lua_State* L)
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
static int LuaSDL_Drawing_SetColor(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, userdata, 1);

    Color* col = (Color*)lua_touserdata(L, 1);

    SDL_SetRenderDrawColor(renderer, col->r, col->g, col->b, col->a);
    return 0;
}
static int LuaSDL_Drawing_GetColor(lua_State* L)
{
    if (!SDLInited) return 0;
    Color* col = (Color*)lua_newuserdata(L, sizeof(Color));
    SDL_GetRenderDrawColor(renderer, &col->r, &col->g, &col->b, &col->a);

    luaL_getmetatable(L, COLOR_TYPE_NAME);
    lua_setmetatable(L, -2);

    return 1;
}

// drawing
static int LuaSDL_Drawing_DrawRect(lua_State* L)
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
static int LuaSDL_Drawing_FillRect(lua_State* L)
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
static int LuaSDL_Drawing_DrawPixel(lua_State* L)
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
static int LuaSDL_Drawing_DrawImage(lua_State* L)
{
    if (!SDLInited) return 0;
    int argc = lua_gettop(L);
    luaL_checkArgType(L, image, 1);
    luaL_checkArgType(L, number, 2);
    luaL_checkArgType(L, number, 3);

    Image* img = (Image*)lua_touserdata(L, 1);
    SDL_Surface* image = img->surf;
    int x = (argc > 1) ? (int)lua_tonumber(L, 2) : 0;
    int y = (argc > 2) ? (int)lua_tonumber(L, 3) : 0;

    SDL_Rect* drawRect = &image->clip_rect;
    drawRect->x = x;
    drawRect->y = y;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, image);

    SDL_RenderCopy(renderer, tex, NULL, drawRect);

    SDL_DestroyTexture(tex);

    return 0;
}
#pragma endregion