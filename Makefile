BINDER_SOURCES = $(wildcard src/common/*.cpp src/binder/*.cpp)
BINDER_OBJECTS = $(BINDER_SOURCES:.cpp=.o)

LIB_SOURCES = $(wildcard src/server/*.cpp src/client/*.cpp src/common/*.cpp)
LIB_OBJECTS = $(LIB_SOURCES:.cpp=.o)

CXX = g++
RM = rm -f
DEBUG = -g
CPPFLAGS = -pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
-Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self \
-Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept \
-Woverloaded-virtual -Wredundant-decls -Wshadow \
-Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 \
-Wswitch-default -Wundef -Werror -Wno-unused
CXXFLAGS = -Isrc $(CPPFLAGS) -W -Wall -g -std=c++1y

LDFLAGS = $(shell pkg-config) -lpthread
RPC = rpc
BINDER = binder
LIB = lib$(RPC).a

OBJS = $(SERVER_OBJS) $(CLIENT_OBJS)

all: $(LIB) $(BINDER)

$(BINDER): $(BINDER_OBJECTS)
	@echo Creating $@...
	@$(CXX) -o $@ $(BINDER_OBJECTS) $(LDFLAGS)

$(LIB): $(LIB_OBJECTS)
	ar cvq $(LIB) $(LIB_OBJECTS)

%.o: %.cpp %.hpp
	@echo Compiling $<...
	@$(CXX) -o $@ -c $(CXXFLAGS) $<

clean:
	$(RM) $(LIB_OBJECTS) $(BINDER_OBJECTS) $(LIB) $(BINDER)
