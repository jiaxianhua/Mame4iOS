//============================================================
//
//  myosd.c - Implementation of osd stuff
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  MAME4DROID MAME4iOS by David Valdeita (Seleuco)
//
//============================================================

#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>

#ifndef __MYOSD_H__
#define __MYOSD_H__

#if defined(__cplusplus)
extern "C" {
#endif

enum  { MYOSD_UP=0x1,       MYOSD_LEFT=0x4,       MYOSD_DOWN=0x10,  MYOSD_RIGHT=0x40,
        MYOSD_START=1<<8,   MYOSD_SELECT=1<<9,    MYOSD_L1=1<<10,    MYOSD_R1=1<<11,
        MYOSD_A=1<<12,      MYOSD_B=1<<13,        MYOSD_X=1<<14,    MYOSD_Y=1<<15,
        MYOSD_VOL_UP=1<<23, MYOSD_VOL_DOWN=1<<22, MYOSD_PUSH=1<<27, MYOSD_ESC=1<<28 };
    
#define MAX_FILTER_KEYWORD 30
#define MAX_GAME_NAME 14
#define NETPLAY_PORT 55435
#define MAX_ROM_PATH 247
        
extern unsigned short *myosd_screen15;
extern int  myosd_fps;
extern int  myosd_showinfo;
extern int  myosd_sleep;
extern int  myosd_inGame;
extern int  myosd_exitGame;
extern int  myosd_pause;
extern int  myosd_exitPause;
extern int  myosd_autosave;
extern int  myosd_cheat;
extern int  myosd_sound_value;
extern int  myosd_frameskip_value;
extern int  myosd_throttle;
extern int  myosd_savestate;
extern int  myosd_loadstate;
extern int  myosd_waysStick;
extern int  myosd_video_width;
extern int  myosd_video_height;
extern int  myosd_vis_video_width;
extern int  myosd_vis_video_height;
extern int  myosd_res_width;
extern int  myosd_res_height;
extern int  myosd_in_menu;
extern int  myosd_auto_res;
extern int  myosd_res;
extern int  myosd_force_pxaspect;
extern int  myosd_num_of_joys;
extern int  myosd_video_threaded;
extern int  myosd_service;
extern int  myosd_num_buttons;
extern int  myosd_light_gun;
extern int  myosd_fs_counter;
extern int  myosd_saveload_combo;

extern unsigned long myosd_pad_status;
    
extern float joy_analog_x[4];
extern float joy_analog_y[4];

extern float lightgun_x[4];
extern float lightgun_y[4];

extern float mouse_x[4];
extern float mouse_y[4];

extern int myosd_mouse;

extern unsigned short myosd_ext_status;
    
extern int myosd_last_game_selected;

extern int myosd_filter_favorites;
extern int myosd_filter_clones;
extern int myosd_filter_not_working;
extern int myosd_filter_manufacturer;
extern int myosd_filter_gte_year;
extern int myosd_filter_lte_year;
extern int myosd_filter_driver_source;
extern int myosd_filter_category;
extern char myosd_filter_keyword[MAX_FILTER_KEYWORD];
extern int myosd_reset_filter;

extern int myosd_num_ways;
    
extern int myosd_vsync;
extern int myosd_refresh;
extern int myosd_dbl_buffer;
extern int myosd_rgb;
extern int myosd_autofire;
extern int myosd_hiscore;
    
extern int myosd_vector_bean2x;
extern int myosd_vector_antialias;
extern int myosd_vector_flicker;
    
extern int  myosd_speed;
extern int  myosd_pxasp1;
    
extern char myosd_selected_game[MAX_GAME_NAME];

extern void myosd_init(void);
extern void myosd_deinit(void);
extern void myosd_video_flip(void);
extern unsigned long myosd_joystick_read(int n);
extern float myosd_joystick_read_analog(int n, char axis);
extern void myosd_set_video_mode(int width,int height,int vis_width, int vis_height);
extern void myosd_closeSound(void);
extern void myosd_openSound(int rate,int stereo);
extern void myosd_sound_play(void *buff, int len);
extern void myosd_check_pause(void);
    
extern const char *myosd_array_main_manufacturers[];
extern const char *myosd_array_years[];
extern const char *myosd_array_main_driver_source[];
extern const char *myosd_array_categories[];

extern char myosd_game[MAX_GAME_NAME];
extern char myosd_rompath[MAX_ROM_PATH];
extern char myosd_version[16];
extern char myosd_bios[16];

extern char *myosd_category;
    
#if defined(__cplusplus)
}
#endif

#endif
