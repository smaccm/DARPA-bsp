/*
 * Copyright 2016, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(D61_BSD)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <platsupport/delay.h>
#include <camkes.h>

double duty_cycle = 0.75;

void playNote(double freqHz, int durationMs) {
    if (freqHz == 0.0) {
        ps_mdelay(durationMs);
        return;
    }

    double periodUs = 1000000.0 / freqHz;
    int count = 1000 * durationMs / periodUs;
    for (int i = 0; i < count; i++) {
        gpio_spi_can1_cs(1);
        ps_udelay(duty_cycle * periodUs);
        gpio_spi_can1_cs(0);
        ps_udelay((1 - duty_cycle) * periodUs);
    }
}

void playSong(char *song) {
    int i = 0;
    int len = strlen(song);

    while (song[i] != ':') {
        i++;
    }
    printf("Playing: %.*s\n", i, song);
    i += 3;

    int default_duration = atoi(&song[i]);
    while (song[i] != ',') {
        i++;
    }
    i += 3;

    int default_octave = song[i] - '0';
    i += 4;
    
    int bpm = atoi(&song[i]);
    while (song[i] != ':') {
        i++;
    }
    i++;

    while (i < len) {
        int duration = default_duration;
        if ('0' <= song[i] && song[i] <= '9') {
            duration = atoi(&song[i]);
        }
        while ('0' <= song[i] && song[i] <= '9') {
            i++;
        }

        int dot = 0;
        if (song[i] == '.') {
            dot = 1;
            i++;
        }

        // http://electronic-setup.blogspot.com/2010/11/nokia-rttl-frequencies-hz.html
        double freq = 0;
        if (song[i+1] == '#') {
            switch (song[i]) {
            case 'a': freq = 233.082; break;
            case 'c': freq = 277.183; break;
            case 'd': freq = 311.127; break;
            case 'f': freq = 369.994; break;
            case 'g': freq = 415.305; break;
            }
            i += 2;
        } else {
            switch (song[i]) {
            case 'a': freq = 220.000; break;
            case 'b': freq = 246.942; break;
            case 'c': freq = 261.626; break;
            case 'd': freq = 293.665; break;
            case 'e': freq = 329.628; break;
            case 'f': freq = 349.228; break;
            case 'g': freq = 391.995; break;
            }
            i++;
        }

        if (song[i] == '.') {
            dot = 1;
            i++;
        }

        int octave = default_octave;
        if ('0' <= song[i] && song[i] <= '9') {
            octave = atoi(&song[i]);
        }
        while ('0' <= song[i] && song[i] <= '9') {
            i++;
        }

        if (song[i] == '.') {
            dot = 1;
            i++;
        }

        float real_freq = freq;
        while (octave > 4) {
            real_freq *= 2;
            octave--;
        }

        int real_duration = 4 * 60 * 1000 / (bpm * duration);
        if (dot) {
            real_duration *= 1.5;
        }

        playNote(real_freq, real_duration);
        i++;
    }
}

#define RIDE "Ride:d=4,o=5,b=240:a,32e,8a,c6,8p,a,8p,c6,32a,8c6,e6,8p,c6,8p,e6,32c6,8e6,g6,8p,g,8p,c6,32g,8c6,2e6,p"

#define SMB "smb:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,16p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g.,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16c7,16p,16c7,16c7,p,16g6,16f#6,16f6,16d#6,16p,16e6,16p,16g#,16a,16c6,16p,16a,16c6,16d6,8p,16d#6,8p,16d6,8p,16c6"

#define MI "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d"

int run(void) {
    // Close enough
    ps_cpufreq_hint(668 * 1000 * 1000);

    while (1) {
        playSong(RIDE);
        ps_sdelay(3);
        playSong(SMB);
        ps_sdelay(3);
        playSong(MI);
        ps_sdelay(3);
    }
}
