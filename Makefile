PREFIX=/Users/justin/pusr

DEFINES=-DOLIO_DEBUG
#INCLUDES=
#LIBS=
FLAGS=-g
#FLAGS=-O3

EXE_TARGETS=olio-test
LIB_TARGETS=libolio.a

INSTALL_HEADERS_SUBDIR=olio
INSTALL_HEADERS=array.h buffer.h config.h debug.h error.h fob.h graph.h hash.h list.h phash.h randist.h random.h skiplist.h socket.h string.h varint.h
INSTALL_LIBS=libolio.a
INSTALL_EXES=

# specify srcs (and optionally libs) for each
# target, with upper-cased target name as prefix
# to _SRCS (and _LIBS)

LIBOLIO_A_SRCS=list.c skiplist.c buffer.c error.c config.c array.c hash.c \
	graph.c random.c randist.c phash.c varint.c

OLIO_TEST_SRCS=test.c
OLIO_TEST_LIBS=-L. -lolio #-lgsl -lgslcblas
OLIO_TEST_DEPS=libolio.a
OLIO_TEST_INCLUDES=

include ${PREFIX}/share/Makefile.base
