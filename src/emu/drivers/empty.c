/*************************************************************************

    empty.c

    Empty driver.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**************************************************************************/

#include "emu.h"
#include "render.h"
#include "uimenu.h"


/*************************************
 *
 *  Machine "start"
 *
 *************************************/

static MACHINE_START( empty )
{
	/* force the UI to show the game select screen */
	ui_menu_force_game_select(machine, render_container_get_ui());
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( empty )

	MDRV_MACHINE_START(empty)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	//MDRV_SCREEN_SIZE(640,480)
	//MDRV_SCREEN_VISIBLE_AREA(0,639, 0,479)
	//DAV HACK
        MDRV_SCREEN_SIZE(400,300)
        MDRV_SCREEN_VISIBLE_AREA(0,399, 0,299)
        //MDRV_SCREEN_SIZE(1280,720)
        //MDRV_SCREEN_VISIBLE_AREA(0,1279, 0,719)
        //MDRV_SCREEN_SIZE(1920,1200)
        //MDRV_SCREEN_VISIBLE_AREA(0,1919, 0,1199)
        //MDRV_SCREEN_SIZE(10,10)
        //MDRV_SCREEN_VISIBLE_AREA(0,9, 0,9)
	MDRV_SCREEN_REFRESH_RATE(30)
MACHINE_DRIVER_END

//"640x480" "640x400" "512x384" "480x300" "400x300" "320x240" "320x200"


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( empty )
	ROM_REGION( 0x10, "user1", 0 )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2007, empty, 0, empty, 0, 0, ROT0, "MAME", "No Driver Loaded", 0 )
