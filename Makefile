CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -pthread
LDFLAGS = -lpcap
TARGET = tpcap
SRC_DIR = src
OBJ_DIR = obj

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g3 -O0 -DDEBUG
	rm -rf $(OBJ_DIR)
else
    CXXFLAGS += -O2 -DNDEBUG
endif

SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/netcap.cpp $(SRC_DIR)/log.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) tpcap

debug:
	$(MAKE) DEBUG=1

.PHONY: all clean debug
