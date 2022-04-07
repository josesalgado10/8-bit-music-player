/*
 * Created: 4/13/2020 12:17:07 PM
 * Author : Jose
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "avr.h"
#include "avr.c"
#include "lcd.h"
#include "lcd.c"

#define modifier 0.00003

#define A 38
#define As 36
#define B 34
#define C 32
#define Cs 30
#define D 28
#define Ds 27
#define E 25
#define F 24
#define Fs 23
#define G 21
#define Gs 20

#define W 8 // 4 // 16
#define H 4 // 2 // 9
#define Q 2 // 1.5 // 4
#define Eth 1.5 // 1 // 3

struct note {
	int freq;
	int duration;
};

const struct note BASEBALL[] = {{C*2, Q}, {C, Q}, {A, Q}, {G*2, Q}, {E*2, Q}, {G*2, H}, {D*2, H},
								{C*2, H}, {C, Q}, {A, Q}, {G*2, Q}, {E*2, Q}, {G*2, W},
								{A, Q}, {Gs*2, Q}, {A, Q}, {E*2, Q}, {F*2, Q}, {G*2, Q}, {A, H}, {F*2, Q}, {D*2, H},
								{A, Q}, {A, Q}, {A, Q}, {B, Q}, {C, Q}, {D, Q}, {B, Q}, {A, Q}, {G*2, Q},
								{E*2, Q}, {D*2, Q}, {C*2, H}, {C, Q}, {A, Q}, {G*2, Q}, {E*2, Q}, {G*2, H}, {D*2, H}, {D*2, Q}, {C*2, Q}, {D*2, Q}, {E*2, Q}, {F*2, Q}, {G*2, Q}, {A, H},
								{A, Q}, {B, Q}, {C, H}, {C, H}, {C, Q}, {B, Q}, {A, Q}, {G*2, Q}, {Fs*2, Q}, {G*2, Q}, {A, H}, {B, H}, {C, W}};

const struct note LOWRIDER1[] = {{B, Q}, {B, Q}, {B, Q}, {B, Q}, {B, Q}, {C, Q}, {D, Q}};
const struct note LOWRIDER2[] = {{G*2, Q}, {B, H}, {C, Q}, {B, H}, {G*2, H}};
const struct note LOWRIDER3[] = {{F*2, Q}, {F*2, Q}, {F*2, Q}, {F*2, Q}, {F*2, Q}};
const struct note LOWRIDER4[] =	{{F*2, Q}, {G*2, W}};

const struct note TWINKLE[] = {{C*2, Q}, {C*2, Q}, {G*2, Q}, {G*2, Q}, {A, Q}, {A, Q}, {G*2, H},
							   {F*2, Q}, {F*2, Q}, {E*2, Q}, {E*2, Q}, {D*2, Q}, {D*2, Q}, {C*2, H},
							   {G*2, Q}, {G*2, Q}, {F*2, Q}, {F*2, Q}, {E*2, Q}, {E*2, Q}, {D*2, H},
							   {G*2, Q}, {G*2, Q}, {F*2, Q}, {F*2, Q}, {E*2, Q}, {E*2, Q}, {D*2, H},
							   {C*2, Q}, {C*2, Q}, {G*2, Q}, {G*2, Q}, {A, Q}, {A, Q}, {G*2, H},
							   {F*2, Q}, {F*2, Q}, {E*2, Q}, {E*2, Q}, {D*2, Q}, {D*2, Q}, {C*2, H}};

int stop = 0;
int tempo = 1; // 0 S, 1 N, 2 F
int pitch = 1; // 0 L, 1 N, 2 H

// # to stop
// * to play
// 0 to quit

int main(void)
{
	lcd_init();
	display_lcd();
	
	SET_BIT(DDRB,3);
	while (1) 
    {
		avr_wait_msec(200);
		int key = get_key();
		switch (key)
		{
			case 1:
				play_track1(&stop);
				display_lcd();
				break;
			case 2:
				play_track2(&stop);
				display_lcd();
				break;
			case 3:
				play_track3(&stop);
				display_lcd();
				break;
			case 4: // increase tempo
				if (tempo < 2)
					tempo++;
					display_lcd();
				break;
			case 8: // decrease tempo
				if (tempo > 0)
					tempo--;
					display_lcd();
				break;
			case 12: // increase pitch
				if (pitch < 2)
					pitch++;
					display_lcd();
				break;
			case 16: // decrease pitch
				if (pitch > 0)
					pitch--;
					display_lcd();
				break;
			default:
				break;
		}
    }
}

int get_key(void)
/* 0 -> no key | 1-16 -> key */
{
	int r, c;
	for (r = 0; r < 4; ++r)
	{
		for (c = 0; c < 4; ++c)
		{
			if (is_pressed(r, c))
			{
				return (r * 4 + c) + 1;
			}
		}
	}
	return 0;
}

int is_pressed(int r, int c)
{
	// SET ALL PINS TO N/C
	DDRC = 0;
	PORTC = 0;
	
	// Set "r" PIN to SO
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	
	// Set "c" PIN to W1
	CLR_BIT(DDRC, c + 4);
	SET_BIT(PORTC, c + 4);
	
	// short wait
	avr_wait(1);
	
	// READ PIN at "c"
	if (GET_BIT(PINC, c + 4))
	{
		// if 0 -> return pressed
		return 0;
	}
	// return not_pressed
	return 1;
}

void display_lcd(void)
{
	lcd_clr();
	char buf[17];
	sprintf(buf, "SELECT | T:%d P:%d", tempo, pitch);
	lcd_puts(buf);
	
	lcd_pos(1, 0);
	lcd_puts("1 2 3");
}

void play_note(int frequency, int duration)
{
	int dur = duration;
	int freq = frequency;
	
	// change tempo
	if (tempo == 0)
		dur = dur * 2;
	if (tempo == 2)
		dur = dur / 2;
	
	// change pitch
	if (pitch == 0)
		freq = freq * 2;
	if (pitch == 2)
		freq = freq / 2;
	
	double ThTl = freq * modifier;
	double period = ThTl * 2;
	int k = dur / period;
	int i;
	for (i = 0; i < k; i++)
	{
		SET_BIT(PORTB, 3);
		avr_wait(freq);
		CLR_BIT(PORTB, 3);
		avr_wait(freq);
	}
}

void play_song(const struct note SONG[], int N, int* stop)
{
	int i;
	for (i = 0; i < N; i++)
	{
		int stopped = get_key();
		if (15 == stopped)
		{
			*stop = 1;
			break;
		}
		play_note(SONG[i].freq, SONG[i].duration);
	}	
}

int play_track1(int* stop)
{
	*stop = 0;
	
	lcd_clr();
	char buf[17];
	sprintf(buf, "TRACK: | T:%d P:%d", tempo, pitch);
	lcd_puts(buf);
	lcd_pos(1, 0);
	lcd_puts("1 7THINN STRETCH");
	
	play_song(BASEBALL, sizeof(BASEBALL)/sizeof(BASEBALL[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(1);
		return 1;
	}
}

int play_track2(int* stop)
{
	*stop = 0;
	
	lcd_clr();
	char buf[17];
	sprintf(buf, "TRACK: | T:%d P:%d", tempo, pitch);
	lcd_puts(buf);
	lcd_pos(1, 0);
	lcd_puts("2 LOWRIDER");
	
	play_song(LOWRIDER1, sizeof(LOWRIDER1)/sizeof(LOWRIDER1[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(2);
		return 2;
	}
	avr_wait_msec(500);
	play_song(LOWRIDER2, sizeof(LOWRIDER2)/sizeof(LOWRIDER2[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(2);
		return 2;
	}
	avr_wait_msec(1000);
	play_song(LOWRIDER1, sizeof(LOWRIDER1)/sizeof(LOWRIDER1[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(2);
		return 2;
	}
	avr_wait_msec(500);
	play_song(LOWRIDER2, sizeof(LOWRIDER2)/sizeof(LOWRIDER2[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(2);
		return 2;
	}
	avr_wait_msec(1000);
	play_song(LOWRIDER3, sizeof(LOWRIDER3)/sizeof(LOWRIDER3[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(2);
		return 2;
	}
	avr_wait_msec(500);
	play_song(LOWRIDER3, sizeof(LOWRIDER3)/sizeof(LOWRIDER3[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(2);
		return 2;
	}
	avr_wait_msec(500);
	play_song(LOWRIDER4, sizeof(LOWRIDER4)/sizeof(LOWRIDER4[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(2);
		return 2;
	}
	return 2;
}

int play_track3(int* stop)
{
	*stop = 0;
	
	lcd_clr();
	char buf[17];
	sprintf(buf, "TRACK: | T:%d P:%d", tempo, pitch);
	lcd_puts(buf);
	lcd_pos(1, 0);
	lcd_puts("3 TWINKLE");
	
	play_song(TWINKLE, sizeof(TWINKLE)/sizeof(TWINKLE[0]), stop);
	if (1 == *stop)
	{
		int check = stop_song();
		if (check == 1)
			restart_song(3);
		return 3;
	}
}
int stop_song(void)
{
	lcd_pos(0 ,0);
	lcd_puts("                ");
	lcd_pos(0, 0);
	lcd_puts("STOPPED TRACK:");
	avr_wait_msec(1000);
	
	while (1)
	{
		lcd_pos(0 ,0);
		lcd_puts("                ");
		lcd_pos(0, 0);
		lcd_puts("* PLAY | 0 QUIT");
		int restart = get_key();
		if (13 == restart)
			return 1;
		if (14 == restart)
			return 0;
	}
}

void restart_song(int track)
{
	switch (track)
	{
		case 1:
			play_track1(&stop);
			break;
		case 2:
			play_track2(&stop);
			break;
		case 3:
			play_track3(&stop);
			break;
		default:
			break;
	}
}