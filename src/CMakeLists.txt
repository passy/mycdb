ADD_EXECUTABLE(mycdbget mycdb.c mycdbget.c unpack.c)
ADD_EXECUTABLE(mycdbserver mycdbserver.c mycdb.c unpack.c socket_helper.c)
