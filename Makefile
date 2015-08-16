CFLAGS=-Wall -Wextra -Os -ggdb
LDFLAGS=
LIBS=-ljack -lsndfile -lm

OBJS = jack_play_and_record.o jack_interface.o soundfile_io.o

all : jack_play_and_record

jack_play_and_record : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

ifneq ($(MAKECMDGOALS), clean)
include $(OBJS:.o=.d)
endif

%.d : %.c
	$(CC) $(CPPFLAGS) -o $@ -MM $^

.PHONY : clean
clean :
	rm -f *~ *.d *.o *.bak jack_play_and_record
