
# Aria Kafie 828109926
# Anosh Taraporevala 827939860

#Makefile

#.RECIPEPREFIX +=

# Specify compiler
CC = g++
# Compiler flags, if you want debug info, add -g
CCFLAGS = -std=c++17 -g3 -c
CFLAGS = -g3 -c

# object files
OBJS = main.o log.o

# Program name
PROGRAM = dineseating

# The program depends upon its object files
$(PROGRAM) : $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS)

main.o : main.cpp log.h consumer.h producer.h shared_data.h
	$(CC) $(CCFLAGS) main.cpp

log.o : log.cpp log.h
	$(CC) $(CFLAGS) log.cpp

# Once things work, people frequently delete their object files.
# If you use "make clean", this will do it for you.
# As we use gnuemacs which leaves auto save files termintating
# with ~, we will delete those as well.
clean :
	rm -rf $(OBJS) *~ $(PROGRAM)
