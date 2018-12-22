
################################################################################
#                                   ACHTUNG!                                   #
################################################################################
# Este Makefile é agnóstico com relação ao seu sistema operacional. No         #
# entanto, ele assume que você esteja tentando compilar seu programa em um     #
# destes sistemas e compiladores:                                              #
#                                                                              #
# - Windows, com MinGW ou MinGW-w64 e GLFW incluído nas pastas de `include` e  #
#   `lib` da sua instalação, apropriadas para a versão do compilador;          #
#                                                                              #
# - Linux, qualquer arquitetura, com GCC ou Clang instalado, e GLFW >= 3.2     #
#   (possivelmente instalável através dos pacotes `libglfw3` e `libglfw3-dev`  #
#   em distribuições baseadas em Debian;                                       #
#                                                                              #
# - Mac OS X, qualquer arquitetura, com Clang instalado (possivelmente sob um  #
#   alias `g++`), assim como o GLFW defidamente instalado -- talvez seja       #
#   necessário alterar as LDFLAGS deste Makefile.                              #
#                                                                              #
################################################################################

# Variáveis-padrão
CXX        =
CXXFLAGS   = -Wall -Wpedantic -g
OBJFLAG    = -c
OUTFLAG    = -o
LDFLAGS    =
SRCS       = $(wildcard *.cpp)
OBJS       = $(SRCS:%.cpp=%.o)
BIN        = automaton


# Linkagem de OpenGL e definição do compilador baseada em SO
ifeq ($(OS), Windows_NT)
	CXX     := g++
	LDFLAGS += -lglfw -lopengl32
else
	UNAME    := $(shell uname -s)
	ifeq ($(UNAME), Linux)
		CXX     := c++
		LDFLAGS := -lglfw -lGL
	endif

	ifeq ($(UNAME), Darwin)
		CXX     := clang++
		LDFLAGS += -lglfw -framework OpenGL
	endif
endif

###############################################################

# Targets que não propriamente produzem arquivos
.PHONY: all clean

# Demais targets
all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(OUTFLAG) $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(OBJFLAG) $^ $(OUTFLAG) $@

clean:
	rm -f *.o $(BIN) *~
