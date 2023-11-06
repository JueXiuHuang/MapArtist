# How to install MapArtist
1. Clone [botcraft](https://github.com/adepierre/Botcraft.git)
2. Clone [MapArtist](https://github.com/JueXiuHuang/MapArtist.git)
3. Cd to botcraft execute below command to build botcraft, it will take a few minutes to run
```bash
cmake -S . -B build -DBOTCRAFT_USE_OPENGL_GUI=ON -DBOTCRAFT_USE_IMGUI=ON -G "MinGW Makefiles" -DCMAKE_CXX_FLAGS="-Wa,-mbig-obj" -DBOTCRAFT_GAME_VERSION=1.20.1

cmake --build build

cmake --install build --prefix "build/output"
  ```
4. In `botcraft/build/output` folder, you can find these folders: `bin`, `include`, `lib`, put files in `bin` to `MapArtist/bin`, move `include`&`lib` to `MapArtist/Botcraft`
5. Cd to `MapArtist` folder, execute below command to update/get submodule we need.
```bash
git submodule update --init --recursive
```
6. In `MapArtist` folder, build it with below command
```bash
cmake -S . -B build -G "MinGW Makefiles"

cmake --build build
```
6. Executable file will in `MapArtist/bin`, use `mapArtist.exe --help` to see available arguments.

# Commands
All commands should add **"bot"** prefix. For example `bot hungry`.
## hungry
Check if player is hungry or not.
## stop all / <user_name>
Stop all players' or specific player's working progress.
## start all / <user_name>
Ask specific player or all players start to work.
## bar
Display working progress bar.
## csafe
Execute csafe command.
## cmd
Execute command in minecraft.
For example `cmd gamemode survival` is equivalent to `/gamemode survival`.
## assign
Define specific player's working column. Default value is 0.
Example: `assign chishin 2`.
## worker
Define max worker number. Default value is 1.
Example: `worker 3`.
## duty
Display all players' set assign value and worker value.
## default
Reset all players' assign & worker value
## ingot
Display current emerald <-> villager ingot exchange rate.
## channel
Display player's current channel.
## move <x> <y> <z>
Ask player move to specified position with self defined method.
## bmove <x> <y> <z>
Ask player move to specified position with botcraft GoTo.