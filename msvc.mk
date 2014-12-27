### This Makefile was written for nmake. ###
GETOPT_DIR        = getopt
GETOPT_REPOSITORY = https://github.com/koturn/$(GETOPT_DIR).git
GETOPT_LIBS_DIR   = $(GETOPT_DIR)/lib
GETOPT_LIB        = getopt$(DBG_SUFFIX).lib
GETOPT_LDLIBS     = /libpath:$(GETOPT_LIBS_DIR) $(GETOPT_LIB)
GETOPT_INCS       = /Igetopt/include/

TERMUTIL_DIR        = TermUtil
TERMUTIL_REPOSITORY = https://github.com/koturn/$(TERMUTIL_DIR).git
TERMUTIL_LIBS_DIR   = $(TERMUTIL_DIR)/lib
TERMUTIL_LIB        = termutil.lib
TERMUTIL_LDLIBS     = /libpath:$(TERMUTIL_LIBS_DIR) $(TERMUTIL_LIB)
TERMUTIL_INCS       = /Itermutil/include/

!if "$(CRTDLL)" == "true"
CRTLIB = /MD$(DBG_SUFFIX)
!else
CRTLIB = /MT$(DBG_SUFFIX)
!endif

!if "$(DEBUG)" == "true"
DBG_SUFFIX         = d
MSVCDBG_DIR        = msvcdbg
MSVCDBG_REPOSITORY = https://github.com/koturn/$(MSVCDBG_DIR).git
MSVCDBG_INCS       = /Imsvcdbg/

COPTFLAGS   = /Od /GS /Zi $(CRTLIB)
LDOPTFLAGS  = /Od /GS /Zi $(CRTLIB)
MSVC_MACROS = /D_DEBUG /D_CRTDBG_MAP_ALLOC /D_USE_MATH_DEFINES
DEPENDENT_LIBRARIES = $(GETOPT_LIBS_DIR)/$(GETOPT_LIB) $(TERMUTIL_LIBS_DIR)/$(TERMUTIL_LIB) $(MSVCDBG_DIR)/NUL

!else
COPTFLAGS   = /Ox /GL $(CRTLIB)
LDOPTFLAGS  = /Ox /GL $(CRTLIB)
MSVC_MACROS = /DNDEBUG /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS \
              /D_USE_MATH_DEFINES
DEPENDENT_LIBRARIES = $(GETOPT_LIBS_DIR)/$(GETOPT_LIB) $(TERMUTIL_LIBS_DIR)/$(TERMUTIL_LIB)
!endif


CC       = cl
RM       = del /F
MAKE     = $(MAKE) /nologo
GIT      = git
INCS     = $(GETOPT_INCS) $(TERMUTIL_INCS) $(MSVCDBG_INCS)
MACROS   = $(MSVC_MACROS)
CFLAGS   = /nologo $(COPTFLAGS) /W4 /c $(INCS) $(MACROS)
LDFLAGS  = /nologo $(LDOPTFLAGS)
LDLIBS   = /link $(GETOPT_LDLIBS) $(TERMUTIL_LDLIBS)
TARGET   = minesweeper.exe
OBJ      = $(TARGET:.exe=.obj)
SRC      = $(TARGET:.exe=.c)
MAKEFILE = msvc.mk


.SUFFIXES: .c .obj .exe
.obj.exe:
	$(CC) $(LDFLAGS) $** /Fe$@ $(LDLIBS)
.c.obj:
	$(CC) $(CFLAGS) $** /Fo$@


all: $(DEPENDENT_LIBRARIES) $(TARGET)

$(TARGET): $(OBJ)

$(OBJ): $(SRC)

$(GETOPT_LIBS_DIR)/$(GETOPT_LIB):
	@if not exist $(@D)/NUL \
		$(GIT) clone $(GETOPT_REPOSITORY)
	cd $(GETOPT_DIR)  &  $(MAKE) /f $(MAKEFILE)  &  cd $(MAKEDIR)

$(TERMUTIL_LIBS_DIR)/$(TERMUTIL_LIB):
	@if not exist $(@D)\NUL \
		$(GIT) clone $(TERMUTIL_REPOSITORY)
	cd $(TERMUTIL_DIR)  &  $(MAKE) /f $(MAKEFILE)  &  cd $(MAKEDIR)

$(MSVCDBG_DIR)/NUL:
	@if not exist $(@D)/NUL \
		$(GIT) clone $(MSVCDBG_REPOSITORY)


clean:
	$(RM) $(TARGET) $(OBJ)
cleanobj:
	$(RM) $(OBJ)
