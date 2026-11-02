# Host-Tests: mit g++ auf dem PC bauen und ausführen, ohne ESP32-Toolchain.
#
#   make -C tests          baut und startet die Tests (Rückgabewert ≠ 0 bei Fehlschlag)
#   make -C tests clean    entfernt den Build-Ordner
#
# Alle Logik-Module unter ../src/core/ (auch in Unterordnern) werden automatisch
# mitgebaut, siehe docs/adr/0001-logic-host-buildable.md. Bindet eines davon
# Arduino- oder ESP-Header ein, scheitert der Build.

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -g -Wall -Wextra
CPPFLAGS += -I. -I../src -Ithird_party -DTEST_IMAGE_DIR='"$(CURDIR)/images"'

BUILD_DIR := build
CORE_DIR  := ../src/core
CORE_SRCS := $(shell find $(CORE_DIR) -name '*.cpp')
TEST_SRCS := $(wildcard *.cpp)
OBJS      := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(TEST_SRCS)) \
             $(patsubst $(CORE_DIR)/%.cpp,$(BUILD_DIR)/core/%.o,$(CORE_SRCS)) \
             $(BUILD_DIR)/stb_image.o
BIN       := $(BUILD_DIR)/host_tests

.PHONY: test clean
test: $(BIN)
	./$(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lm

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<

$(BUILD_DIR)/core/%.o: $(CORE_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<

# Fremdcode ohne Warnungen übersetzen
$(BUILD_DIR)/stb_image.o: third_party/stb_image_impl.c
	@mkdir -p $(@D)
	$(CC) -O2 -w -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
