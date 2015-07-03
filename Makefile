SOURCES = $(wildcard */*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

CXX=g++
RM=rm -f
DEBUG=-g
CPPFLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
-Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self \
-Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept \
-Woverloaded-virtual -Wredundant-decls -Wshadow \
-Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 \
-Wswitch-default -Wundef -Werror -Wno-unused
CXXFLAGS = -I. $(CPPFLAGS) -W -Wall -g -std=c++1y

LDFLAGS = $(shell pkg-config) -lpthread
MAIN=rpc
LIB=lib$(MAIN).a

OBJS=$(SERVER_OBJS) $(CLIENT_OBJS)

all: $(MAIN)

$(MAIN): $(OBJECTS)
	ar cvq $(LIB) $(OBJECTS)

%.o: %.cpp %.hpp
	@echo Compiling $<...
	@$(CXX) -o $@ -c $(CXXFLAGS) $<

clean:
	$(RM) $(OBJECTS) $(LIB)
