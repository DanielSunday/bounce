#ifndef BASS_H
#define BASS_H

#define BASS_SAMPLE_RATE 44100
#define BASS_BUFFER_SIZE   256

void init_bass(float frequency, float volume, float duration);
void exit_bass(void);
void drop_bass(void);

#endif // BASS_H
