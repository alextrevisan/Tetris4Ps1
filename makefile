include setup.mk

# Project target name
TARGET		= Tetris

#TOOLS
#MKPSXISO    = tools/mkpsxiso.exe
MKPSXISO_XML= iso_build/Tetris4Ps1.xml
BIN_FOLDER  = bin
ISO_FOLDER  = iso

#EMULATOR
#EMUBIN = D:\\Games\\Emuladores\\Playstation\\ePSXe.exe
#EMU_CMD = $(EMUBIN) -nogui -loadbin iso/$(TARGET).cue


EMUBIN = D:\\Dev\\DevEngine3D\\pcsx-redux\\pcsx-redux.exe
#EMUBIN = tools\\NOPSX.EXE bin/$(TARGET).exe
#EMU_CMD = $(EMUBIN) -iso iso\\$(TARGET).cue
#EMU_CMD = $(NOPS) -exe bin/$(TARGET).exe com4 -fast

#EMU_CMD = $(EMUBIN) iso/$(TARGET).bin

EMUBIN = ../DuckStation/duckstation-qt-x64-ReleaseLTCG.exe
EMU_CMD = $(EMUBIN) bin/$(TARGET).exe


#EMU_CMD = $(MCOMMS) -dev com4 run bin/$(TARGET).exe

# Searches for C, C++ and S (assembler) files in local directory
CFILES		= $(notdir $(wildcard *.c))
CPPFILES 	= $(notdir $(wildcard *.cpp))
AFILES		= $(notdir $(wildcard *.s))

#UI
UI_DIR = ui
UI =  $(notdir $(wildcard $(UI_DIR)/*.s))
UIFILES = $(addprefix build/$(UI_DIR)/,$(UI:.s=.o)) \

#ENGINE
ENGINE_DIR = engine
ENGINE =  $(notdir $(wildcard $(ENGINE_DIR)/*.cpp))
ENGINEFILES = $(addprefix build/$(ENGINE_DIR)/,$(ENGINE:.cpp=.o))

#MAIN
OFILES		= $(addprefix build/,$(CFILES:.c=.o)) \
			$(addprefix build/,$(CPPFILES:.cpp=.o)) \
			$(addprefix build/,$(AFILES:.s=.o)) 



# Project specific include and library directories
# (use -I for include dirs, -L for library dirs)
INCLUDE	 	+=	-I. -Iscenario -I$(ENGINE_DIR) -I$(UMM_DIR) -Iuclibc++ -Ips1c++
LIBDIRS		+=

# Libraries to link
LIBS		+=

# C compiler flags
CFLAGS		+= -g

# C++ compiler flags
CPPFLAGS	+= -std=c++20
						
# Assembler flags
AFLAGS		+=

# Linker flags
LDFLAGS		+=

all: $(OFILES) $(UIFILES) $(ENGINEFILES)
	@mkdir -p $(BIN_FOLDER)
	$(LD) $(LDFLAGS_EXE) $(LIBDIRS) $(OFILES) $(UIFILES) $(ENGINEFILES) $(LIBS)  -o bin/$(TARGET)
	$(ELF2X) -q $(BIN_FOLDER)/$(TARGET) $(BIN_FOLDER)/$(TARGET).exe
	$(ELF2X) -q $(BIN_FOLDER)/$(TARGET) $(BIN_FOLDER)/$(TARGET)
	
iso: all
	@mkdir -p $(ISO_FOLDER)
	@rm -rf $(ISO_FOLDER)/*.cue $(ISO_FOLDER)/*.bin
	@$(MKPSXISO) $(MKPSXISO_XML)
	@mv *.cue $(ISO_FOLDER)
	@mv *.bin $(ISO_FOLDER)

run: iso
	$(EMU_CMD)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(AFLAGS) $(CPPFLAGS) $(INCLUDE) -c $< -o $@

build/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(AFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf build $(BIN_FOLDER) $(ISO_FOLDER) *.bin *.cue