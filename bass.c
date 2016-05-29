#include "bass.h"

#include <portaudio/portaudio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

PaStream *stream = NULL;
float *table = NULL;
bool playing = false;
int n_buffers = 0;
int buffer_index = 0;

static void init_pa(void);
static void init_table(float frequency, float volume, float duration);
static int cb(const void *in, void *out, unsigned long fpb,
              const PaStreamCallbackTimeInfo *time_info,
              PaStreamCallbackFlags status, void *p);

void init_bass(float frequency, float volume, float duration)
{
    init_pa();
    init_table(frequency, volume, duration);
    Pa_OpenDefaultStream(&stream, 0, 2, paFloat32,
                         BASS_SAMPLE_RATE, BASS_BUFFER_SIZE, cb, NULL);
    Pa_StartStream(stream);
}

static void init_pa(void)
{
    FILE *orig_stderr; // ALSA puts garbage. I don't know to portably shut it.
    orig_stderr = stderr;
    stderr = tmpfile();
    Pa_Initialize();
    fflush(stderr);
    fclose(stderr);
    stderr = orig_stderr;
}

static void init_table(float frequency, float volume, float duration)
{
    int actual_size = duration * BASS_SAMPLE_RATE;
    n_buffers = actual_size / BASS_BUFFER_SIZE + 1;
    int n_samples = n_buffers * BASS_BUFFER_SIZE;
    table = malloc(n_samples * sizeof(*table));
    float radians = (M_PI * 2 * frequency * duration) / actual_size;
    for (int i = 0; i < actual_size; ++i)
    {
        float one_to_zero = 1 - (float) i / actual_size;
        table[i] = sin(i * radians) * volume * one_to_zero;
    }
    for (int i = actual_size; i < n_samples; ++i) table[i] = 0;
}

void exit_bass(void)
{
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    free(table);
    table = NULL;
    playing = false;
}

void drop_bass(void)
{
    playing = true;
    buffer_index = 0;
}

static int cb(const void *in, void *out, unsigned long fpb,
              const PaStreamCallbackTimeInfo *time_info,
              PaStreamCallbackFlags status, void *p)
{
    (void) in;
    (void) fpb;
    (void) time_info;
    (void) status;
    (void) p;
    float *output = out;

    if (playing)
    {
        int base_index = buffer_index * BASS_BUFFER_SIZE;
        for (int i = 0; i < BASS_BUFFER_SIZE; ++i)
        {
            output[i * 2 + 0] = table[base_index + i];
            output[i * 2 + 1] = table[base_index + i];
        }
        ++buffer_index;
        if (buffer_index >= n_buffers) playing = false;
    }
    else for (int i = 0; i < BASS_BUFFER_SIZE * 2; ++i) output[i] = 0;
    return paContinue;
}
