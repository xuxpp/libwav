#include <stdio.h>
#include <stddef.h>

#include "wav.h"

int main()
{
    /*****************  Write a WAV file  *****************/
    // Write to a wav file with some PCM samples
    wav_t *w_write = wav_open("/tmp/test_wav.wav", "w+");
    wav_set_sample_rate     (w_write, 48000);
    wav_set_num_channels    (w_write, 8);
    wav_set_bytes_per_sample(w_write, 3);

    // Add 10 samples
    for (size_t i = 0; i < 10; i++)
    {
        int buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        // Array of pointers to per-channel sample buffers
        void *channels[8] =
        {
            (char *)&buf[0],
            (char *)&buf[1],
            (char *)&buf[2],
            (char *)&buf[3],
            (char *)&buf[4],
            (char *)&buf[5],
            (char *)&buf[6],
            (char *)&buf[7],
        };
        // Add 1 sample
        wav_add_sample(w_write, channels);
    }
    // Write to file
    wav_write(w_write);
    wav_close(w_write);
    printf("Created wav file at /tmp/test_wav.wav with:\n");
    printf("Sample rate       : 48000\n");
    printf("Number of channels: 8\n");
    printf("Bytes per sample  : 3\n");

    /*****************  Read a WAV file  *****************/
    printf("----------------------------------------------\n");
    wav_t *w_read = wav_open("/tmp/test_wav.wav", "r");
    printf("Read wav file at /tmp/test_wav.wav with:\n");
    printf("Sample rate       : %ld\n",  wav_get_sample_rate     (w_read));
    printf("Number of channels: %ld\n",  wav_get_num_channels    (w_read));
    printf("Bytes per sample  : %ld\n",  wav_get_bytes_per_sample(w_read));

    int buf[8] = {0};
    // Array of pointers to per-channel sample buffers
    void *channels[8] =
    {
        (char *)&buf[0],
        (char *)&buf[1],
        (char *)&buf[2],
        (char *)&buf[3],
        (char *)&buf[4],
        (char *)&buf[5],
        (char *)&buf[6],
        (char *)&buf[7],
    };
    size_t sample_cnt = 0;
    // Read all samples
    while (wav_get_sample(w_read, channels))
    {
        printf("Sample %ld: %d, %d, %d, %d, %d, %d, %d, %d\n", sample_cnt++, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
    }
    wav_close(w_read);

    return 0;
}