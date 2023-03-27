// SPDX-License-Identifier: zlib-acknowledgement

// i2s used for DAC data to audio codec, ADC data from codec

// streaming data just means update as it comes in (as opposed to batched, e.g. once every hour)
// e.g. SPI stream

// if have distinct input and output, double buffering required

// https://www.youtube.com/watch?v=VDhmVrbSpqA 
// https://github.com/pms67/FilterLib
// https://github.com/TjRichmond/RichEffectsSynth/tree/b5a469b09fde933ffd37c7a89b8aa7e8a19353a5

#define SAMPLE_TIME_MS_USB  20
#define SAMPLE_TIME_MS_BAR  125
#define SAMPLE_TIME_MS_LED  250
#define SAMPLE_TIME_MS_ATT   10

