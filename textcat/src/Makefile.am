lib_LTLIBRARIES = libtextcat.la
include_HEADERS = textcat.h
libtextcat_la_SOURCES = textcat.c ngrams.c memory.c handlers.c


bin_PROGRAMS = tc_learn

tc_learn_SOURCES = learn.c
tc_learn_LDADD   = libtextcat.la
