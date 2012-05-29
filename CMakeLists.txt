CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(mycdb C)

ADD_DEFINITIONS(-ggdb -fsigned-char -Wall -W -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual -Winline -Werror)
ADD_SUBDIRECTORY(src bin)
