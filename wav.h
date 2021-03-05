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

#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct wav_t wav_t;

size_t wav_get_num_channels    (const wav_t *w);
size_t wav_get_bytes_per_sample(const wav_t *w);
size_t wav_get_length          (const wav_t *w);
size_t wav_get_sample_rate     (const wav_t *w);
void wav_set_num_channels    (wav_t *w, int num_chn);
void wav_set_bytes_per_sample(wav_t *w, int bytes);
void wav_set_sample_rate     (wav_t *w, int rate);

// True: OK, False: EOF
bool wav_get_sample(wav_t *w, void **buffers);
void wav_add_sample(wav_t *w, void **buffers);

// Write cuurent contents into a .wav file
void wav_write(wav_t *w);

wav_t *wav_open(const char *file_name, const char* mode);
void   wav_close(wav_t *w);