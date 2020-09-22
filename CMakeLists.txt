cmake_minimum_required(VERSION 3.10)

project(inventory LANGUAGES C)
set(C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_compile_options(${GTK_CFLAGS_OTHER})

add_executable(inventory "main.c"
                         "product.c"
                         "inventory.c"
                         "uitools.c"
                         "mainwindow.c"
                         "editdialog.c"
                         "product.h"
                         "inventory.h"
                         "uitools.h"
                         "mainwindow.h"
                         "editdialog.h")

target_link_libraries(inventory ${GTK_LIBRARIES})
