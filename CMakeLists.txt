cmake_minimum_required(VERSION 3.20)
project(TP_SO C)

set(CMAKE_C_STANDARD 99)

add_executable(TP_SO main.c cliente.c medico.c balcao.c classificador.c cliente.h medico.h balcao.h)
