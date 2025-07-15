CXX = c++

CXXFLAGS += -g -Wall -Werror -Wextra -std=c++98 -MMD -MP

CPPFLAGS += -I./includes

LDFLAGS +=

LDLIBS +=

NAME = $B/webserv

S = ./src

O = ./obj

B = ./bin

D = ./dep

FILES = main \
		Server \
		WebServer \
		Tokenizer \
		SyntaxException \
		Response \
		Request \
		HttpMethods \

SRCS = $(addsuffix .cpp, $(addprefix $S/, $(FILES)))

OBJS = $(SRCS:$S/%.cpp=$O/%.o)

DEPS = $(SRCS:$S/%.cpp=$D/%.d)

RM = rm -rf

DEBUGGER = gdb

AUTHOR = buozcan

AUTHOR2 = faata

SHELL = /bin/bash
