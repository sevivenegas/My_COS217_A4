#--------------------------------------------------------------------
# Makefile                                                        
# Author: Sevastian Venegas & Neha Ayyalapu                                                
#--------------------------------------------------------------------

CC=gcc217

all: ft

clean:
  rm -f ft *.o

clobber: clean
  rm -f ft_client.o *~

ft: ft.o ft_client.o nodeFT.o path.o dynarray.o
  $(CC) ft.o ft_client.o nodeFT.o path.o dynarray.o -o ft

ft_client.o: ft_client.c ft.h a4def.h
  $(CC) -c ft_client.c

ft.o: ft.c ft.h dynarray.h nodeFT.h path.h a4def.h
  $(CC) -c ft.c

nodeFT.o: nodeFT.c nodeFT.h path.h dynarray.h a4def.h
  $(CC) -c nodeFT.c

path.o: path.c path.h dynarray.h a4def.h
  $(CC) -c path.c

dynarray.o: dynarray.c dynarray.h 
  $(CC) -c dynarray.c