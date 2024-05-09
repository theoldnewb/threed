#include <SDL3/SDL_log.h>
#include <SDL3/SDL_version.h>


void
hello_sdl3()
{
    SDL_Version compiled ;
    SDL_Version linked ;
    SDL_VERSION(&compiled) ;
    SDL_GetVersion(&linked) ;
    SDL_Log("compiled with: %u, %u, %u", compiled.major, compiled.minor, compiled.patch) ;
    SDL_Log("linking with: %u, %u, %u", linked.major, linked.minor, linked.patch) ;
    SDL_Log("SDL_GetRevision()=%s", SDL_GetRevision()) ;
}


int
main(
    int     argc
,   char *  argv[]
)
{
    SDL_Log("Hello, World!") ;
    hello_sdl3() ;

    return 0 ;
}
