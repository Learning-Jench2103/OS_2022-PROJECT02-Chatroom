CXX := g++
CXXFLAGS := -std=c++11 -pthread
INCLUDE_DIR := include
SRC_DIR := src
BIN_DIR := bin
CPP_HEADERS := $(wildcard $(INCLUDE_DIR)/*.hpp)
SERVER_SRC := server.cc
CLIENT_SRC := client.cc

all: $(BIN_DIR)/server $(BIN_DIR)/client
	@echo "Build successfully!"
$(BIN_DIR)/server: $(CPP_HEADERS) $(SRC_DIR)/$(SERVER_SRC)
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(SRC_DIR)/$(SERVER_SRC) -I $(INCLUDE_DIR) -o $@
$(BIN_DIR)/client: $(CPP_HEADERS) $(SRC_DIR)/$(CLIENT_SRC)
	@mkdir -p $(BIN_DIR)
	@$(CXX) $(CXXFLAGS) $(SRC_DIR)/$(CLIENT_SRC) -I $(INCLUDE_DIR) -o $@

.PHONY: clean
clean:
	@rm -rf $(BIN_DIR)
