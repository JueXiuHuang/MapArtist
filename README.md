You need to compile botcraft file on your own.

# TODO lists
- [ ] Add Microsoft login utility
- [ ] A* optimize (走斜的+垂直降落) -> submodule
- [ ] Contorl bot through chat or dc

# Botcraft build
`cmake -S . -B build -DBOTCRAFT_USE_OPENGL_GUI=ON -DBOTCRAFT_USE_IMGUI=ON -G "MinGW Makefiles" -DCMAKE_CXX_FLAGS="-Wa,-mbig-obj"`

`cmake --build build`

`cmake --install build --prefix "build/output"`
