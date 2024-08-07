/******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include "OsApi.h"
#include "EventLib.h"
#include "LuaEngine.h"
#include "OpenOceansPPClassifier.h"

/******************************************************************************
 * DEFINES
 ******************************************************************************/

#define LUA_OPENOCEANSPP_LIBNAME    "openoceanspp"

/******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************/

/*----------------------------------------------------------------------------
 * openoceanspp_version
 *----------------------------------------------------------------------------*/
int openoceanspp_version (lua_State* L)
{
    lua_pushstring(L, BINID);
    lua_pushstring(L, BUILDINFO);
    return 2;
}

/*----------------------------------------------------------------------------
 * openoceanspp_open
 *----------------------------------------------------------------------------*/
int openoceanspp_open (lua_State *L)
{
    static const struct luaL_Reg openoceanspp_functions[] = {
        {"version",             openoceanspp_version},
        {"classifier",          OpenOceansPPClassifier::luaCreate},
        {NULL,                  NULL}
    };

    /* Set Library */
    luaL_newlib(L, openoceanspp_functions);

    return 1;
}

/******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************/

extern "C" {
void initopenoceanspp (void)
{
    /* Initialize Modules */
    OpenOceansPPClassifier::init();

    /* Extend Lua */
    LuaEngine::extend(LUA_OPENOCEANSPP_LIBNAME, openoceanspp_open);

    /* Indicate Presence of Package */
    LuaEngine::indicate(LUA_OPENOCEANSPP_LIBNAME, BINID);

    /* Display Status */
    print2term("%s plugin initialized (%s)\n", LUA_OPENOCEANSPP_LIBNAME, BINID);
}

void deinitopenoceanspp (void)
{
}
}
