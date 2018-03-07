CC=arm-linux-gnueabihf-gcc
CXX=arm-linux-gnueabihf-g++
RM=rm -rf

LIB = periphery.a
SRCS = src/periphery/mmio.cpp  src/periphery/spi.cpp  src/periphery/serial.cpp  src/periphery/i2c.cpp

SRCDIR = src/periphery
OBJDIR = obj

TEST_PROGRAMS = $(addprefix build/,$(notdir $(basename $(wildcard src/test/*.cpp))))

#$(info TEST_PROGRAMS="${TEST_PROGRAMS}")

################################################################################

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

COMMIT_ID := $(shell git describe --abbrev --always --tags --dirty 2>/dev/null || echo "")

CXXFLAGS += -std=c++11 -fPIC
CXXFLAGS += -Wall -Wextra -Werror -Wpedantic $(DEBUG)
#CXXFLAGS += -DPERIPHERY_VERSION_COMMIT=\"$(COMMIT_ID)\"
CXXFLAGS += -Iinclude

LDFLAGS +=

################################################################################

.PHONY: all
all: $(LIB)

.PHONY: release
release: CXXFLAGS += -O2
release: all

.PHONY: tests
tests: $(TEST_PROGRAMS)

.PHONY: clean
clean:
	$(RM) $(LIB) $(OBJDIR)

################################################################################

build/%: src/test/%.cpp $(LIB)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< $(LIB) -o $@

################################################################################

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LIB): $(OBJECTS)
	ar rcs $(LIB) $(OBJECTS)

$(OBJDIR)/%.o: CXXFLAGS += -c
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@
