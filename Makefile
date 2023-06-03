CLANG_MAJOR_VERSION=$(shell clang --version | grep -Eo 'version [0-9]+' | cut -d' ' -f2)
ifeq ($(MAKECMDGOALS),cover)
	CXXFLAGS+=-O0
	CXXFLAGS+=-fcoverage-mapping
	CXXFLAGS+=-fprofile-instr-generate
	CXXFLAGS+=-g
	LDFLAGS+=-fprofile-instr-generate
else
	CXXFLAGS+=-Oz
	LDFLAGS+=-Wl,-s
endif
CXX=clang++
CXXFLAGS+=-Wall
CXXFLAGS+=-Werror
CXXFLAGS+=-Wextra
CXXFLAGS+=-fno-exceptions
CXXFLAGS+=-fno-rtti
CXXFLAGS+=-iquote$(PWD)/inc
CXXFLAGS+=-pedantic
ifeq ($(shell test $(CLANG_MAJOR_VERSION) -lt 13 && echo $$?),0)
	CXXFLAGS+=-Wno-unknown-attributes
	CXXFLAGS+=-std=c++20
else
	CXXFLAGS+=-std=c++2b
endif
HEADERS+=$(wildcard inc/*.hh)
LD=clang++
LDFLAGS+=-fuse-ld=lld
OBJECTS=$(SOURCES:%.cc=%.o)
PROFILE_COV=$(TARGET).cov
PROFILE_DATA=$(TARGET).profdata
PROFILE_LOG=$(TARGET).proflog
PROFILE_RAW=$(TARGET).profraw
SOURCES+=$(wildcard src/*.cc)
TARGET=dumpelf
TARGET_PCH=$(TARGET_PCH_SOURCE:%.hh=%.hh.pch)
TARGET_PCH_SOURCE=inc/stdafx.hh

all: $(TARGET)

clean:
	$(RM) $(OBJECTS) $(PROFILE_COV) $(PROFILE_DATA) $(PROFILE_LOG) $(PROFILE_RAW) $(TARGET)

cover: $(PROFILE_COV)

distclean: clean
	$(RM) $(TARGET_PCH)

.PHONY: all clean cover distclean

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -include-pch $(TARGET_PCH)
	@mv $(notdir $(<:%.cc=%.o)) src/

$(OBJECTS): $(HEADERS) $(TARGET_PCH)

$(PROFILE_COV): $(PROFILE_DATA)
	llvm-cov show $(TARGET) -instr-profile=$< $(HEADERS) $(SOURCES) > $@

$(PROFILE_DATA): $(PROFILE_RAW)
	llvm-profdata merge -o $@ -sparse $^

$(PROFILE_RAW): $(TARGET)
	env LLVM_PROFILE_FILE=$@ ./$(TARGET) $(TARGET) > $(PROFILE_LOG)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(TARGET_PCH): $(TARGET_PCH_SOURCE)
	$(CXX) $(CXXFLAGS) -o $@ -x c++-header $^
