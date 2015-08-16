/* Written by Christian Vogel <vogelchr@vogel.cx> in 2015. This program
is placed in the public domain to the largest extent possible, I do
not claim any rights to this code. */

#include "soundfile_io.h"

#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int soundfile_io_load(char *fn, unsigned long *samplerate, float **buf, unsigned long *nsamples)
{
	SNDFILE *f;
	SF_INFO sfinfo;
	sf_count_t w,i;
	float *p;
	float *one_frame=NULL;

	*buf=NULL;

	bzero(&sfinfo, sizeof(sfinfo));
	if (!(f=sf_open(fn, SFM_READ, &sfinfo))) {
		fprintf(stderr, "%s: %s (libsndfile error)\n", fn,
			sf_strerror(NULL));
		return -1;
	}

	*nsamples = sfinfo.frames;
	*samplerate = sfinfo.samplerate;

	if (!(*buf = malloc(sfinfo.frames * sizeof(float)))) {
		perror("malloc()");
		goto error_out;
	}

	if (!(one_frame = malloc(sizeof(float)*sfinfo.frames))) {
		perror("malloc()");
		goto error_out;
	}

	p = *buf;
	for (w=sfinfo.frames; w; w--) {
		i = sf_readf_float(f, one_frame, 1);
		if (i != 1) {
			fprintf(stderr,"%s: %s (libsndfile error)\n",
				fn, sf_strerror(f));
			goto error_out;
		}
		*p++ = one_frame[0];
	}
	sf_close(f);

	return 0;
error_out:
	if (f)
		sf_close(f);
	if (*buf)
		free(*buf);
	if (one_frame)
		free(one_frame);
	return -1;
}


int soundfile_io_save(char *fn, unsigned long samplerate, const float *buf, unsigned long nsamples)
{
	SNDFILE *f;
	SF_INFO sfinfo;
	sf_count_t frames;

	bzero(&sfinfo, sizeof(sfinfo));

	sfinfo.frames = frames = nsamples;
	sfinfo.samplerate = samplerate;
	sfinfo.channels = 1;
	sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;

	if (!(f=sf_open(fn, SFM_WRITE, &sfinfo))) {
		fprintf(stderr, "%s: %s (libsndfile error)\n", fn,
			sf_strerror(NULL));
		return -1;
	}

	if (sf_writef_float(f, buf, nsamples) != frames) {
		fprintf(stderr,"%s: %s (libsndfile error)\n",
			fn, sf_strerror(f));
		goto error_out;
	}

	sf_close(f);

	return 0;
error_out:
	if (f)
		sf_close(f);

	return -1;
}
