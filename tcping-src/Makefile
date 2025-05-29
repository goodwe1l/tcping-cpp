#### 自动检测编译环境
UNAME_S := $(shell uname -s)

## Windows (Visual C++)
ifeq ($(OS),Windows_NT)
    CC = cl
    CFLAGS = -EHsc -Os
    LFLAGS = ws2_32.lib /MACHINE:X86 /Fe$@
    OBJEXT = obj
    TARGET = tcping.exe
endif

## Linux
ifeq ($(UNAME_S),Linux)
    CC = g++
    CFLAGS = -Wall -O2 -std=c++11
    LFLAGS = -o $@
    OBJEXT = o
    TARGET = tcping
endif

## macOS
ifeq ($(UNAME_S),Darwin)
    CC = g++
    CFLAGS = -Wall -O2 -std=c++11
    LFLAGS = -o $@
    OBJEXT = o
    TARGET = tcping
endif


.SUFFIXES: .cpp .$(OBJEXT)

## 编译规则
ifeq ($(OS),Windows_NT)
# Windows 编译规则
.cpp.$(OBJEXT):
	$(CC) -c $(CFLAGS) $<
else
# Linux/macOS 编译规则
.cpp.$(OBJEXT):
	$(CC) -c $(CFLAGS) $< -o $*.o
endif


#### Major targets #####################################################

all: $(TARGET)

$(TARGET): main.$(OBJEXT) tcping.$(OBJEXT) base64.$(OBJEXT) tee.$(OBJEXT) ws-util.$(OBJEXT)
ifeq ($(OS),Windows_NT)
	$(CC) $(CFLAGS) $^ $(LFLAGS)
else
	$(CC) $(CFLAGS) $^ $(LFLAGS)
endif

ctags:
	ctags *.cpp *.h

ifeq ($(OS),Windows_NT)
# Windows 清理命令
clean:
	del *.$(OBJEXT) *.bak

distclean: clean
	del *.exe *.swp
else
# Linux/macOS 清理命令
clean:
	rm -f *.$(OBJEXT) *.bak

distclean: clean
	rm -f tcping *.swp
endif


#### Common dependencies ###############################################

main.$(OBJEXT): main.cpp platform.h
tcping.$(OBJEXT): tcping.cpp tcping.h platform.h
ws-util.$(OBJEXT): ws-util.cpp ws-util.h platform.h
tee.$(OBJEXT): tee.cpp tee.h platform.h
base64.$(OBJEXT): base64.cpp base64.h


#### tcping ##############################################

TCPING_OBJS = tcping.$(OBJEXT) main.$(OBJEXT) \
		ws-util.$(OBJEXT)
tcping.exe: $(TCPING_OBJS)
	$(CC) $(TCPING_OBJS) $(LFLAGS)

tcping.$(OBJEXT): tcping.cpp ws-util.h


