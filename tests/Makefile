# Host-Tests: mit g++ auf dem PC bauen und ausführen, ohne ESP32-Toolchain.
#
#   make -C tests          baut und startet die Tests (Rückgabewert ≠ 0 bei Fehlschlag)
#   make -C tests clean    entfernt den Build-Ordner
#
# Firmware-Module, die getestet werden, kommen in FIRMWARE_SRCS (reines C++
# ohne Arduino-Abhängigkeit, z. B. ../src/frame_align.cpp).

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -g -Wall -Wextra
CPPFLAGS += -I. -I../src -Ithird_party -DTEST_IMAGE_DIR='"$(CURDIR)/images"'

BUILD_DIR     := build
FIRMWARE_SRCS :=
TEST_SRCS     := $(wildcard *.cpp)
OBJS          := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(TEST_SRCS) $(notdir $(FIRMWARE_SRCS))) \
                 $(BUILD_DIR)/stb_image.o
BIN           := $(BUILD_DIR)/host_tests

vpath %.cpp . $(sort $(dir $(FIRMWARE_SRCS)))

.PHONY: test clean
test: $(BIN)
	./$(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lm

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c -o $@ $<

# Fremdcode ohne Warnungen übersetzen
$(BUILD_DIR)/stb_image.o: third_party/stb_image_impl.c | $(BUILD_DIR)
	$(CC) -O2 -w -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
