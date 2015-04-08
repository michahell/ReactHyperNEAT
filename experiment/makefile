# =================== COMPILER ====================

# compiler flags:
#  -g     							adds debugging information to the executable file
#  -w 	  							suppresses all warnings...
#  -wAll  							turn on all warnings
#  =Wno-error						do not consider warnings as errors (NEAT is riddled with warnings/errors..)
#  --print-search-dirs	prints paths searched by gcc
#  -fmessage-length=0 	error messages are not truncated, shown fully on single line 
#  -std=c++11						use C++11...
#  -lstdc++							link the C++11 library...
#  libstdc++						GNU/Linux standard C++ lib

CFLAGS = -g -stdlib=libstdc++ -Wall -Wno-error -ferror-limit=1000 -fmessage-length=0

PLATFORM_CFLAG = -DMACOS
EXTRA_CFLAGS = -mmacosx-version-min=10.5 -arch i386 -arch x86_64

# compiler
CC = gcc

# current dir
DIR_CURRENT = $(shell pwd)
DIR_HYPERNEAT = $(DIR_CURRENT)/../HyperNEAT


# ================== LIBRARIES ==================

# include tinyXML lib(s)
# LIB_TINYXML = -L/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/tinyxmldll/out
LIB_TINYXML = -L$(DIR_HYPERNEAT)/tinyxmldll/out
# include HyperNEAT lib(s)
# LIB_HYPERNEAT = -L/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/NE/HyperNEAT/out
LIB_HYPERNEAT = -L$(DIR_HYPERNEAT)/NE/HyperNEAT/out
# include boost lib
# LIB_BOOST = -L/Users/michahell/Documents/projects_c++/HyperNEAT/boost_1_57_0/stage/lib
LIB_BOOST = -L$(DIR_HYPERNEAT)/boost_1_57_0/stage/lib
# include WeBots lib
LIB_WEBOTS = -L/Applications/Webots6.3.0/lib -lController
# include Zlib (tiny XML needs this)
# LIB_ZLIB = -L/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/zlib/out -lz
LIB_ZLIB = -L$(DIR_HYPERNEAT)/zlib/out -lz

LIBS = $(LIB_TINYXML) $(LIB_HYPERNEAT) $(LIB_BOOST) $(LIB_WEBOTS) $(LIB_ZLIB)




# ================== INCLUDES ==================

# include WeBots source
# INC_WEBOTS = -I/Applications/Webots/include/controller/c
INC_WEBOTS = -I/Applications/Webots6.3.0/include/controller/c
# include boost source
# INC_BOOST = -I/Users/michahell/Documents/SDKS/boost_1_57_0/boost/
INC_BOOST = -I$(DIR_HYPERNEAT)/boost_1_57_0/
# include NEAT
# INC_NEAT = -I/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/NE/HyperNEAT/NEAT/include
INC_NEAT = -I$(DIR_HYPERNEAT)/NE/HyperNEAT/NEAT/include
# include HyperNEAT
# INC_HYPERNEAT = -I/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/NE/HyperNEAT/Hypercube_NEAT/include
INC_HYPERNEAT = -I$(DIR_HYPERNEAT)/NE/HyperNEAT/Hypercube_NEAT/include
# include tinyXML
# INC_TXML = -I/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/tinyxmldll/include
INC_TXML = -I$(DIR_HYPERNEAT)/tinyxmldll/include
# include JGTL (Jason Gauci Templating Library?)
# INC_JGTL = -I/Users/michahell/Documents/projects_c++/experimentSuite/HyperNEAT/JGTL/include
INC_JGTL = -I$(DIR_HYPERNEAT)/JGTL/include
# combine libs and include dirs into macro's

HEADERPATH = $(INC_WEBOTS) $(INC_BOOST) $(INC_BOOST_EVERT) $(INC_NEAT) $(INC_HYPERNEAT) $(INC_TXML) $(INC_JGTL)




# =================== FLAGS ====================

DEFINES = -DHCUBE_NOGUI -DTIXML_USE_STL

# FINAL FLAGS -- TO BE USED THROUGHOUT
FLAGS = $(CFLAGS) $(DEFINES) $(PLATFORM_CFLAG)

# LIB FLAGS
LIBFLAGS = -lstdc++ -lz -ltinyxmlpluslib -lboost_filesystem-mt -lboost_random-mt -lboost_system-mt -lNEATLib_d -lHypercube_NEAT_Base_d -lController 




# =================== TARGETS ==================

# helper targets

cleanFirst:
	@# remove controllers
	rm -f controllers/*.o
	rm -f controllers/*.out
	rm -rf controllers/mod_ctrler7_1
	rm -rf controllers/mod_ctrler7_2
	rm -rf controllers/mod_supervisor7
	rm -rf /Applications/Webots6.3.0/resources/controllers/mod_ctrler7_1/
	rm -rf /Applications/Webots6.3.0/resources/controllers/mod_ctrler7_2/
	rm -rf /Applications/Webots6.3.0/resources/controllers/mod_supervisor7/

cleanAfter:
	rm -f controllers/*.o




# experiments




experiment-modular:

	@echo ' '
	@echo 'COMPILING CONTROLLER ...'
	@echo ' '
	$(CC) $(FLAGS) $(EXTRA_CFLAGS) $(HEADERPATH) -DXML1 -c ModHyperNEAT/mod_ctrler7.cpp  -o controllers/mod_ctrler7_1.o
	$(CC) $(FLAGS) $(EXTRA_CFLAGS) $(HEADERPATH) -DXML2 -c ModHyperNEAT/mod_ctrler7.cpp  -o controllers/mod_ctrler7_2.o

	@echo ' '
	@echo 'COMPILING SUPERVISOR ...'
	@echo ' '
	$(CC) $(FLAGS) $(EXTRA_CFLAGS) $(HEADERPATH) -c ModSupervisor/mod_supervisor.cpp -o controllers/mod_supervisor7.o

	@echo ' '
	@echo 'LINKING CONTROLLER...'
	@echo ' '
	$(CC) $(FLAGS) $(EXTRA_CFLAGS) $(LIBS) -DXML1 controllers/mod_ctrler7_1.o $(LIBFLAGS) -o controllers/mod_ctrler7_1.out
	$(CC) $(FLAGS) $(EXTRA_CFLAGS) $(LIBS) -DXML2 controllers/mod_ctrler7_2.o $(LIBFLAGS) -o controllers/mod_ctrler7_2.out

	@echo ' '
	@echo 'LINKING SUPERVISOR...'
	@echo ' '
	$(CC) $(FLAGS) $(EXTRA_CFLAGS) $(LIBS) controllers/mod_supervisor7.o $(LIBFLAGS) -o controllers/mod_supervisor7.out

	@echo ' '
	@echo 'CREATING CONTROLLER DIRS IN WEBOTS FOLDER...'
	@echo ' '
	@cd /Applications/Webots6.3.0/resources/controllers && mkdir -p mod_ctrler7_1 && mkdir -p mod_ctrler7_2 && mkdir -p mod_supervisor7

	@echo ' '
	@echo 'MOVING CONTROLLERS TO WEBOTS FOLDER...'
	@echo ' '
	mv controllers/mod_ctrler7_1.out /Applications/Webots6.3.0/resources/controllers/mod_ctrler7_1/mod_ctrler7_1
	mv controllers/mod_ctrler7_2.out /Applications/Webots6.3.0/resources/controllers/mod_ctrler7_2/mod_ctrler7_2

	@echo ' '
	@echo 'MOVING SUPERVISOR TO WEBOTS FOLDER...'
	@echo ' '
	mv controllers/mod_supervisor7.out /Applications/Webots6.3.0/resources/controllers/mod_supervisor7/mod_supervisor7




# All Target
all: cleanFirst experiment-modular cleanAfter
# all: cleanFirst test-webots cleanAfter
