add_executable(cyberCat_ver2 WIN32
    cyberCat_ver2.cpp
)
target_link_libraries(cyberCat_ver2
    user32
    gdi32
    dwmapi
)