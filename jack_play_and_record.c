/* based heavily on thru_client.c and simple_client.c from
   the jack audio connection kit examples */

#include "jack_interface.h"
#include "soundfile_io.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>


enum process_state { WAIT_STARTUP, COPYING, FINISHED };

/* this will be accessed by our buffer_process callback to write out,
   copy in samples from/to jack */
static float *sample_buf=NULL;
static unsigned long sample_count=0;

/* these are accessed in a mixed fashion between the process thread and
   our main function, hence marked "volatile", that's good enough for now */
static volatile unsigned long sample_ptr=0;
static volatile unsigned long start_at_sample=96000;
static volatile unsigned long curr_pos=0;
static volatile enum process_state process_state;

int
buffer_process(unsigned long nsamples, const float *inbuf, float *outbuf)
{
        unsigned long ncopy;

        if (process_state == FINISHED) {
                curr_pos += nsamples;
                return 0;
        }

        /* wait for copying to start... */
        if (process_state == WAIT_STARTUP) {
                /* not yet copying... */
                if (curr_pos + nsamples < start_at_sample + 1) {
                        curr_pos += nsamples;
                        return 0;
                }

                process_state = COPYING;

                /* check if we have to skip a few samples at start */
                if (curr_pos < start_at_sample) {
                        unsigned long skip;

                        skip = start_at_sample - curr_pos;

                        curr_pos += skip;
                        nsamples -= skip;
                        inbuf += skip;
                        outbuf += skip;
                }
        }

        ncopy = sample_count - sample_ptr;
        if (ncopy > nsamples)
                ncopy = nsamples;

        memcpy(outbuf, sample_buf+sample_ptr, sizeof(float)*ncopy);
        memcpy(sample_buf+sample_ptr, inbuf, sizeof(float)*ncopy);
        sample_ptr += ncopy;

        if (sample_ptr == sample_count)
                process_state = FINISHED;

        curr_pos += nsamples;
        return 0;
}

void
usage(char *argv0)
{
        printf("Usage: %s [options] infile.wav outfile.wav\n", argv0);
}

int
main(int argc, char **argv)
{
        int i;
        unsigned long file_sr;

        while ((i=getopt(argc, argv, "i:o:h")) != -1) {
                switch (i) {
                case 'h':
                        usage(argv[0]);
                        exit(1);
                }
        }

        if (argc != optind+2) {
                usage(argv[0]);
                fprintf(stderr,"Specify exactly one input, one output file!\n");
                exit(1);
        }

        if (soundfile_io_load(argv[optind], &file_sr, &sample_buf, &sample_count))
                exit(1);

        fprintf(stderr, "Loaded %lu samples from %s.\n",
                sample_count,argv[optind]);

        jack_interface_set_cb(buffer_process);

        if (jack_interface_connect(argv[0], NULL) == -1)
                exit(1);

        printf("Current sample rate is %lu Hz.\n",
                jack_interface_framerate());

        while (1) {
                unsigned long ctr;

                if (!jack_interface_is_alive(&ctr))
                        break;

                printf("Jack connected, state is %d, counter is %lu\n",
                        process_state, curr_pos);
                sleep(1);

                if (process_state == 2)
                        break;
        }

        if (soundfile_io_save(argv[optind+1], file_sr, sample_buf, sample_count))
                exit(1);

        exit(0);
}
