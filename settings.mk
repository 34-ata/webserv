CXX = c++

CXXFLAGS += -Wall -Werror -Wextra -std=c++98

CPPFLAGS += -I./includes

LDFLAGS +=

LDLIBS +=

NAME = webserv

S = ./src

O = ./obj

B = ./bin

SRCS = $S/main.cpp \
	   $S/Socket.cpp 

OBJS = $(SRCS:$S/%.cpp=$O/%.o)

RM = rm -rf

AUTHOR = buozcan

AUTHOR2 = faata
