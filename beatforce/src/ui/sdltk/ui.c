#include "SDL_Window.h"
#include "wndmgr.h"

int UI_Init()
{
    SDL_WidgetInit();
    WNDMGR_Init(); 
}


int UI_Main()
{
    SDLTK_Main(); /* main loop */
}
