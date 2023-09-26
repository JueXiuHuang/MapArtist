You need to compile botcraft file on your own.

# TODO lists
- [x] Optimize TaskPrioritize algorithm (BFS->DFS)
- [x] Optimize collect material algorithm
- [ ] Add Microsoft login utility
- [x] 材料會少拿
- [x] sliceDfs可能會少蓋 (最一開始的方塊可能會被跳過)
- [x] 蓋的時候方塊可能被玩家卡住
- [ ] A* optimize (走斜的+垂直降落)

# Botcraft build
`cmake -S . -B build -DBOTCRAFT_USE_OPENGL_GUI=ON -DBOTCRAFT_USE_IMGUI=ON -G "MinGW Makefiles" -DCMAKE_CXX_FLAGS="-Wa,-mbig-obj"`

`cmake --build build`

`cmake --install build --prefix "build/output"`
