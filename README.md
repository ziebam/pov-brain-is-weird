# pov: brain is weird

simple toy to showcase the persistence of vision phenomenon discussed in [this video](https://www.youtube.com/watch?v=TdTMeNXCnTs).

try starting the below demo. when it's running, you will seemingly notice lines flickering on the screen, but they disappear as soon as you pause or take a screenshot.

https://github.com/ziebam/pov-brain-is-weird/assets/31964869/1f517c27-9b61-4cb3-a452-a46d5d4b7cea


# build

you can build this project locally on Windows (sorry Linux/macOS folks).

requirements: `gcc` and `ar` available in the path.

steps:

1. clone the repo
2. `gcc ./nob.c -o nob.exe`
3. `./nob.exe`
4. `./build/pov-brain-is-weird.exe`

# credits

`nob` -- amazing, minimalist build system by [Tsoding](https://github.com/tsoding), license included in `nob.h`

`raylib` -- amazing, minimalist game-making library, [license](./raylib/raylib-5.0/LICENSE) included in the repository as it's built statically

window icon -- taken from [Tsoding](https://github.com/tsoding)'s stream/Discord server with his persmission: [click!](https://twitchemotes.com/channels/110240192)

inspiration -- [this video](https://www.youtube.com/watch?v=TdTMeNXCnTs) and its unironically awesome comment section
