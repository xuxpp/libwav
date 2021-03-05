/**
 * Copyright (c) 2021 Yuyang Qi
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "wav.h"
// Assume LITTLE_ENDIAN

#define BUF_INIT_SIZE 1024 * 1024 // 1MB
#define WAV_RIFF_CHUNK_ID       'FFIR'
#define WAV_FORMAT_CHUNK_ID     ' tmf'
#define WAV_FACT_CHUNK_ID       'tcaf'
#define WAV_DATA_CHUNK_ID       'atad'
#define WAV_WAVE_ID             'EVAW'

typedef struct
{
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint32_t format;
    uint32_t subchunk1_id;
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t subchunk2_id;
    uint32_t subchunk2_size;
} header_t;

struct wav_t
{
    FILE    *fp;
    uint8_t *f_buf;
    size_t   f_len;        // In Bytes
    size_t   mem_size;     // In Bytes
    size_t   sample_count; // For single channel

    header_t *h;
    uint8_t  *data;
    size_t    n_channels;
    size_t    bytes_per_sample;
    size_t    bytes_per_sample_all_chn;
    size_t    data_len;
};

static void parse_header(wav_t *w)
{
    w->n_channels               =  w->h->num_channels;
    w->bytes_per_sample         =  w->h->bits_per_sample / 8;
    w->bytes_per_sample_all_chn =  w->bytes_per_sample * w->n_channels;
    w->data_len                 =  w->h->subchunk2_size;
}

static void reallocate_memory(wav_t *w, size_t new_size)
{
    w->mem_size = new_size;
    void *new_ptr = realloc(w->f_buf, w->mem_size);
    if (new_ptr)
    {
        w->f_buf = new_ptr;
        w->h = (header_t *)w->f_buf;
        w->data = &w->f_buf[sizeof(header_t)];
    }
    else
    {
        fprintf(stderr, "Failed to allocate more memory, quitting.\n");
        wav_close(w);
        exit(1);
    }
}

size_t wav_get_num_channels    (const wav_t *w) { return w->n_channels;       }
size_t wav_get_bytes_per_sample(const wav_t *w) { return w->bytes_per_sample; }
size_t wav_get_length          (const wav_t *w) { return w->data_len;         }
size_t wav_get_sample_rate     (const wav_t *w) { return w->h->sample_rate;   }

void wav_set_num_channels(wav_t *w, int num_chn)
{
    w->n_channels = num_chn;
    w->bytes_per_sample_all_chn = w->n_channels * w->bytes_per_sample;
}

void wav_set_bytes_per_sample(wav_t *w, int bytes)
{
    w->bytes_per_sample = bytes;
    w->bytes_per_sample_all_chn = w->n_channels * w->bytes_per_sample;
}

void wav_set_sample_rate(wav_t *w, int rate)
{
    w->h->sample_rate = rate;
    w->h->byte_rate   = w->h->sample_rate * w->n_channels * w->bytes_per_sample;
}

bool wav_get_sample(wav_t *w, void **buffers)
{
    size_t sample_idx = w->sample_count * w->bytes_per_sample_all_chn;
    if (sample_idx >= w->data_len)
        return false;

    for (size_t i = 0; i < w->n_channels; i++)
        memcpy(buffers[i], &w->data[sample_idx + i * w->bytes_per_sample], w->bytes_per_sample);

    w->sample_count++;
    return true;
}

void wav_add_sample(wav_t *w, void **buffers)
{
    // Do we need more memory?
    if (w->f_len + w->bytes_per_sample_all_chn > w->mem_size)
        reallocate_memory(w, w->mem_size + 1024 * 1024);

    const size_t offset = w->sample_count * w->bytes_per_sample_all_chn;
    for (size_t i = 0; i < w->n_channels; i++)
        memcpy(&w->data[offset + i * w->bytes_per_sample], buffers[i], w->bytes_per_sample);

    w->sample_count++;
    w->data_len += w->bytes_per_sample_all_chn;
    w->f_len    += w->bytes_per_sample_all_chn;
}

void wav_write(wav_t *w)
{
    // Init header http://soundfile.sapp.org/doc/WaveFormat/
    w->h->chunk_id        = WAV_RIFF_CHUNK_ID;
    w->h->chunk_size      = 36 + w->data_len;
    w->h->format          = WAV_WAVE_ID;
    w->h->subchunk1_id    = WAV_FORMAT_CHUNK_ID;
    w->h->subchunk1_size  = 16;
    w->h->audio_format    = 1;
    w->h->num_channels    = w->n_channels;
    w->h->sample_rate     = w->h->sample_rate == 0 ? 48000 : w->h->sample_rate;
    w->h->byte_rate       = w->h->sample_rate * w->n_channels * w->bytes_per_sample;
    w->h->block_align     = w->n_channels * w->bytes_per_sample;
    w->h->bits_per_sample = w->bytes_per_sample * 8;
    w->h->subchunk2_id    = WAV_DATA_CHUNK_ID;
    w->h->subchunk2_size  = w->data_len;
    fwrite(w->f_buf, 1, w->f_len, w->fp);
}

wav_t *wav_open(const char *file_name, const char* mode)
{
    wav_t *w = calloc(1, sizeof(*w));
    w->mem_size = BUF_INIT_SIZE;
    w->f_buf = calloc(1, w->mem_size);

    w->fp = fopen(file_name, mode);
    w->h = (header_t *)w->f_buf;

    if (strcmp(mode, "r") == 0)
    {
        // Get file size:
        fseek(w->fp , 0, SEEK_END);
        w->f_len = ftell(w->fp);
        rewind(w->fp);

        // Reallocate memory if the file is too big
        if (w->f_len > w->mem_size)
            reallocate_memory(w, w->f_len);

        // Read entire file
        size_t act_read = fread(w->f_buf, 1, w->f_len, w->fp);
        if (act_read != w->f_len)
            fprintf(stderr, "Unable to read all bytes! Exp: %ld, Act: %ld\n", w->f_len, act_read);

        parse_header(w);
    }
    if (strcmp(mode, "w") == 0 || strcmp(mode, "w+") == 0)
        w->f_len = sizeof(header_t);

    w->data = &w->f_buf[sizeof(header_t)];

    return w;
}

void wav_close(wav_t *w)
{
    if (w)
    {
        fclose(w->fp);
        free(w->f_buf);
        free(w);
    }
}