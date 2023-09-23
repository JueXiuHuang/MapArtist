You need to compile botcraft file on your own.

# TODO lists
- [ ] Optimize TaskPrioritize algorithm (BFS->DFS)
- [x] Optimize collect material algorithm
- [ ] Add Microsoft login utility

# Botcraft build
`cmake -S . -B build -DBOTCRAFT_USE_OPENGL_GUI=ON -DBOTCRAFT_USE_IMGUI=ON -G "MinGW Makefiles" -DCMAKE_CXX_FLAGS="-Wa,-mbig-obj"`

`cmake --build build`

`cmake --install build --prefix "build/output"`
