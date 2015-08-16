#ifndef SOUNDFILE_IO_H
#define SOUNDFILE_IO_H

extern int soundfile_io_load(char *fn, unsigned long *samplerate,
	float **buf, unsigned long *nsamples);

extern int soundfile_io_save(char *fn, unsigned long samplerate,
	const float *buf, unsigned long nsamples);


#endif
