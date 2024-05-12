# threed

An attempt of a multi platform C project. Utilizing [CMake](https://cmake.org/), [SDL3](https://github.com/libsdl-org/SDL), [cglm](https://github.com/recp/cglm) and [Vulkan](https://vulkan.lunarg.com/).

We will furthermore use [Python](https://www.python.org/) for shell like functions and other file system operations like creating directories. Otherwise we would need to create several sets of shell scripts for Windows CMD/Powershell or the various Linux shells like sh, csh, bash.

The progression of the project can be followed up here:
[Twitch](https://www.twitch.tv/theoldnewguy) and [YouTube](https://www.youtube.com/@tom-bi1zm)

## Getting started

    git clone --recurse-submodules https://github.com/theoldnewb/threed.git
    cd threed

## Builing third party dependencies

    external/create_third_party.py

Once everything was built properly, a 'tpl' directory should appear.

    dir tpl/
    SDL3 cglm

## Building the threed project itself

    ./rebuild.py

If everything worked out, there should be a 'bin' directory containing the threed executable

    dir bin
    threed

## Running threed

    bin/threed

## Troubleshooting

The Windows build will probably be missing the SDL3.dll.
In that case the SDL3.dll needs to be copied into the threed/bin directory manually.

    cp tpl/SDL/bin/* bin/

