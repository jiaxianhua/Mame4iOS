//============================================================
//
//  droidinput.c - Implementation of MAME input routines
//
//  Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  MAME4DROID MAME4iOS by David Valdeita (Seleuco)
//
//============================================================

#ifdef ANDROID
#include <android/log.h>
#endif

#include "osdepend.h"
#include "emu.h"
#include "uimenu.h"

#include "myosd.h"
#include "netplay.h"

enum
{
	KEY_ESCAPE,
	KEY_1,
	KEY_2,
	KEY_LOAD,
	KEY_SAVE,
	KEY_PGUP,
	KEY_PGDN,
	KEY_SERVICE,
	KEY_TOTAL
};

enum
{
	STATE_NORMAL,
	STATE_LOADSAVE
};

enum  {
    NP_EXIT=0x1,
    NP_SAVE=1<<2,
    NP_LOAD=1<<3
};

static int mystate = STATE_NORMAL;
static int B_pressed[4] ={0,0,0,0};
static int old_B_pressed[4] = {0,0,0,0};
static int enabled_autofire[4] = {0,0,0,0};

// a single input device
static input_device *keyboard_device;

// the states
static int keyboard_state[KEY_TOTAL];
static int joy_buttons[4][8];
static int joy_axis[4][2];
static int joy_hats[4][4];

static int lightgun_axis[4][2];

static int mouse_axis[4][2];

static int poll_ports = 0;
static int fire[4] = {0,0,0,0};

static void my_poll_ports(running_machine *machine);
static long joystick_read(int i);
float joystick_read_analog(int n, char axis);
static INT32 my_get_state(void *device_internal, void *item_internal);
static INT32 my_axis_get_state(void *device_internal, void *item_internal);

void droid_ios_init_input(running_machine *machine)
{

	memset(keyboard_state,0,sizeof(keyboard_state));
	memset(joy_buttons,0,sizeof(joy_buttons));
	memset(joy_axis,0,sizeof(joy_axis));
	memset(joy_hats,0,sizeof(joy_hats));
    


	input_device_class_enable(machine, DEVICE_CLASS_LIGHTGUN, TRUE);
	input_device_class_enable(machine, DEVICE_CLASS_JOYSTICK, TRUE);
        if(myosd_mouse)
           input_device_class_enable(machine, DEVICE_CLASS_MOUSE, TRUE);

	for (int i = 0; i < 4; i++)
	{
		char name[10];
		input_device *devinfo;

		snprintf(name, 10, "Joy %d", i + 1);
		devinfo = input_device_add(machine, DEVICE_CLASS_JOYSTICK, name, NULL);
		input_device_item_add(devinfo, "Bt_A", &joy_buttons[i][0], ITEM_ID_BUTTON1, my_get_state);
		input_device_item_add(devinfo, "Bt_B", &joy_buttons[i][1], ITEM_ID_BUTTON2, my_get_state);
		input_device_item_add(devinfo, "Bt_C", &joy_buttons[i][2], ITEM_ID_BUTTON3, my_get_state);
		input_device_item_add(devinfo, "Bt_D", &joy_buttons[i][3], ITEM_ID_BUTTON4, my_get_state);
		input_device_item_add(devinfo, "Bt_E", &joy_buttons[i][4], ITEM_ID_BUTTON5, my_get_state);
		input_device_item_add(devinfo, "Bt_F", &joy_buttons[i][5], ITEM_ID_BUTTON6, my_get_state);

		input_device_item_add(devinfo, "Coin", &joy_buttons[i][6], ITEM_ID_BUTTON7, my_get_state);
		input_device_item_add(devinfo, "Start", &joy_buttons[i][7], ITEM_ID_BUTTON8, my_get_state);

		input_device_item_add(devinfo, "X Axis", &joy_axis[i][0], ITEM_ID_XAXIS, my_axis_get_state);
		input_device_item_add(devinfo, "Y Axis", &joy_axis[i][1], ITEM_ID_YAXIS, my_axis_get_state);

		input_device_item_add(devinfo, "D-Pad Up", &joy_hats[i][0],(input_item_id)( ITEM_ID_HAT1UP+i*4), my_get_state);
		input_device_item_add(devinfo, "D-Pad Down", &joy_hats[i][1],(input_item_id)( ITEM_ID_HAT1DOWN+i*4), my_get_state);
		input_device_item_add(devinfo, "D-Pad Left", &joy_hats[i][2], (input_item_id)(ITEM_ID_HAT1LEFT+i*4), my_get_state);
		input_device_item_add(devinfo, "D-Pad Right", &joy_hats[i][3], (input_item_id)(ITEM_ID_HAT1RIGHT+i*4), my_get_state);

	}

	keyboard_device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, "Keyboard", NULL);
	if (keyboard_device == NULL)
		fatalerror("Error creating keyboard device");

	input_device_item_add(keyboard_device, "Exit", &keyboard_state[KEY_ESCAPE], ITEM_ID_ESC, my_get_state);

	input_device_item_add(keyboard_device, "1", &keyboard_state[KEY_1], ITEM_ID_1, my_get_state);
	input_device_item_add(keyboard_device, "2", &keyboard_state[KEY_2], ITEM_ID_2, my_get_state);

	input_device_item_add(keyboard_device, "Load", &keyboard_state[KEY_LOAD], ITEM_ID_F7, my_get_state);
	input_device_item_add(keyboard_device, "Save", &keyboard_state[KEY_SAVE], ITEM_ID_F8, my_get_state);

	input_device_item_add(keyboard_device, "PGUP", &keyboard_state[KEY_PGUP], ITEM_ID_PGUP, my_get_state);
	input_device_item_add(keyboard_device, "PGDN", &keyboard_state[KEY_PGDN], ITEM_ID_PGDN, my_get_state);

	input_device_item_add(keyboard_device, "Service", &keyboard_state[KEY_SERVICE], ITEM_ID_F2, my_get_state);

	for (int i = 0; i < 4; i++)
	{
	    char name[10];
            snprintf(name, 10, "Lightgun %d", i + 1);

	    input_device *lightgun_device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, name, NULL);
	    if (lightgun_device == NULL)
		fatalerror("Error creating lightgun device");

	   input_device_item_add(lightgun_device, "X Axis", &lightgun_axis[i][0], ITEM_ID_XAXIS, my_axis_get_state);
  	   input_device_item_add(lightgun_device, "Y Axis", &lightgun_axis[i][1], ITEM_ID_YAXIS, my_axis_get_state);
        }

	for (int i = 0; i < 4; i++ && myosd_mouse)
	{
	    char name[10];
            snprintf(name, 10, "Mouse %d", i + 1);

	    input_device *mouse_device = input_device_add(machine, DEVICE_CLASS_MOUSE, name, NULL);
	    if (mouse_device == NULL)
		fatalerror("Error creating mouse device");

	   input_device_item_add(mouse_device, "X Axis", &mouse_axis[i][0], ITEM_ID_XAXIS, my_axis_get_state);
  	   input_device_item_add(mouse_device, "Y Axis", &mouse_axis[i][1], ITEM_ID_YAXIS, my_axis_get_state);
        }
    
        poll_ports = 1;
}

void my_poll_ports(running_machine *machine)
{
    const input_field_config *field;
	const input_port_config *port;
    if(poll_ports)
    {
        int way8 = 0;
        int counter = 0;
        myosd_num_buttons = 0;
        myosd_light_gun = 0;
        for (port = machine->m_portlist.first(); port != NULL; port = port->next())
        {
            for (field = port->fieldlist; field != NULL; field = field->next)
            {
                if (field->player!=0)continue;
                if(field->type == IPT_BUTTON1)
                    if(myosd_num_buttons<1)myosd_num_buttons=1;
                if(field->type == IPT_BUTTON2)
                    if(myosd_num_buttons<2)myosd_num_buttons=2;
                if(field->type == IPT_BUTTON3)
                    if(myosd_num_buttons<3)myosd_num_buttons=3;
                if(field->type == IPT_BUTTON4)
                    if(myosd_num_buttons<4)myosd_num_buttons=4;
                if(field->type == IPT_BUTTON5)
                    if(myosd_num_buttons<5)myosd_num_buttons=5;
                if(field->type == IPT_BUTTON6)
                    if(myosd_num_buttons<6)myosd_num_buttons=6;
                if(field->type == IPT_JOYSTICKRIGHT_UP)//dual stick is mapped as buttons
                    if(myosd_num_buttons<4)myosd_num_buttons=4;
                if(field->type == IPT_POSITIONAL)//positional is mapped with two last buttons
                    if(myosd_num_buttons<6)myosd_num_buttons=6;
                if(field->type == IPT_JOYSTICK_UP || field->type == IPT_JOYSTICK_DOWN || field->type == IPT_JOYSTICKLEFT_UP || field->type == IPT_JOYSTICKLEFT_DOWN)
                    way8= 1;
                if(field->type == IPT_AD_STICK_X || field->type == IPT_LIGHTGUN_X || field->type == IPT_MOUSE_X ||
                   field->type == IPT_TRACKBALL_X || field->type == IPT_PEDAL)
                    way8= 1;
                if(field->type == IPT_DIAL || field->type == IPT_PADDLE || field->type == IPT_POSITIONAL ||
                   field->type == IPT_DIAL_V || field->type == IPT_PADDLE_V || field->type == IPT_POSITIONAL_V)
                    counter++;
                if(field->type == IPT_LIGHTGUN_X)
                   myosd_light_gun = 1;
            }
        }
        poll_ports=0;
        
        //8 si analog o lightgun o up o down
        if (myosd_num_ways!=4){
            if(way8 || counter>1)
                myosd_num_ways = 8;
            else
                myosd_num_ways = 2;
        }
        printf("Num Buttons %d\n",myosd_num_buttons);
        printf("Num WAYS %d\n",myosd_num_ways);
    }
    
}

long joystick_read(int i)
{
    netplay_t *handle = netplay_get_handle();
    
    if(handle->has_connection)
    {
        long res = 0;
        if(i==0)
        {
            if(handle->player1)
                res = handle->state.digital;
            else
                res = handle->peer_state.digital;
        }
        else if(i==1)
        {
            if(handle->player1)
                res = handle->peer_state.digital;
            else
                res = handle->state.digital;
        }
        return res;
    }
    else
    {
        return  myosd_joystick_read(i);
    }
}

float joystick_read_analog(int n, char axis)
{
    netplay_t *handle = netplay_get_handle();
    
    if(handle->has_connection)
    {
        float res = 0.0;
        if(n==0)
        {
            if(handle->player1)
                res = (float) (axis=='x' ? handle->state.analog_x : handle->state.analog_y);
            else
                res = (float) (axis=='x' ? handle->peer_state.analog_x : handle->peer_state.analog_y);
        }
        else if(n==1)
        {
            if(handle->player1)
                res = (float) (axis=='x' ? handle->peer_state.analog_x : handle->peer_state.analog_y);
            else
                res = (float) (axis=='x' ? handle->state.analog_x : handle->state.analog_y);
        }
        return res;
    }
    else
    {
        return  myosd_joystick_read_analog(n,axis);
    }
}

void droid_ios_poll_input(running_machine *machine)
{
    my_poll_ports(machine);

    long _pad_status = joystick_read(0);

    netplay_t *handle = netplay_get_handle();
    int netplay = handle->has_connection && handle->has_begun_game;

    if (mystate == STATE_NORMAL)
    {

        keyboard_state[KEY_1] = 0;
        keyboard_state[KEY_2] = 0;

        if (myosd_exitGame || handle->state.ext & NP_EXIT || handle->peer_state.ext & NP_EXIT)
        {
            if (netplay)
            {
                if (myosd_in_menu)
                {
                    if (handle->state.ext & NP_EXIT || handle->peer_state.ext & NP_EXIT)
                    {
                        keyboard_state[KEY_ESCAPE] = 0x80;
                        myosd_ext_status &= ~ NP_EXIT;
                    }
                    else
                    {
                        myosd_ext_status |= NP_EXIT;
                    }
                }
                else
                {
                    handle->has_connection = 0;
                    handle->has_begun_game = 0;
                    keyboard_state[KEY_ESCAPE] = 0x80;
                }
            }
            else
            {
                handle->has_begun_game = 0;//ensure for auto disconnect
                keyboard_state[KEY_ESCAPE] = 0x80;
            }

            myosd_exitGame = 0;
            handle->state.ext &= ~ NP_EXIT;
            handle->peer_state.ext &= ~ NP_EXIT;
        }
        else
        {
            keyboard_state[KEY_ESCAPE] = 0;

            if (myosd_reset_filter==1) {
                myosd_exitGame= 1;
            }
        }

        if (myosd_service && !myosd_in_menu && !netplay)
        {
            keyboard_state[KEY_SERVICE] = 0x80;
            myosd_service = 0;
        }
        else
        {
            keyboard_state[KEY_SERVICE] = 0;
        }

        keyboard_state[KEY_LOAD] =  0;
        keyboard_state[KEY_SAVE] =  0;

        if (myosd_savestate || handle->state.ext & NP_SAVE || handle->peer_state.ext & NP_SAVE)
        {
            if (netplay)
            {
                if (handle->state.ext & NP_SAVE || handle->peer_state.ext & NP_SAVE)
                {
                    keyboard_state[KEY_SAVE] =  0x80;
                    mystate = STATE_LOADSAVE;
                    myosd_ext_status &= ~ NP_SAVE;
                }
                else
                {
                    myosd_ext_status |= NP_SAVE;
                }
            }
            else
            {
                keyboard_state[KEY_SAVE] =  0x80;
                mystate = STATE_LOADSAVE;
            }

            handle->state.ext &= ~ NP_SAVE;
            handle->peer_state.ext &= ~ NP_SAVE;
            myosd_savestate = 0;
            myosd_fs_counter = 60 * 5;
            return;
        }

        if (myosd_loadstate || handle->state.ext & NP_LOAD || handle->peer_state.ext & NP_LOAD)
        {
            if (netplay)
            {
                if (handle->state.ext & NP_LOAD || handle->peer_state.ext & NP_LOAD)
                {
                    keyboard_state[KEY_LOAD] =  0x80;
                    mystate = STATE_LOADSAVE;
                    myosd_ext_status &= ~ NP_LOAD;
                }
                else
                {
                    myosd_ext_status |= NP_LOAD;
                }
            }
            else
            {
                keyboard_state[KEY_LOAD] =  0x80;
                mystate = STATE_LOADSAVE;
            }

            handle->state.ext &= ~ NP_LOAD;
            handle->peer_state.ext &= ~ NP_LOAD;
            myosd_loadstate = 0;
            myosd_fs_counter = 60 * 5;
            return;
        }

        for (int i=0; i<4; i++)
        {
            if (i!=0  && myosd_in_menu==1 && myosd_num_of_joys <=1)//to avoid mapping issues when pxasp1 is active
                break;

            _pad_status = joystick_read(i);

            if (i==0)
            {
                if (!myosd_inGame && !myosd_in_menu)
                {
                    keyboard_state[KEY_PGUP] = ((_pad_status & MYOSD_LEFT) != 0) ? 0x80 : 0;
                    keyboard_state[KEY_PGDN] = ((_pad_status & MYOSD_RIGHT) != 0) ? 0x80 : 0;
                }
                else
                {
                    keyboard_state[KEY_PGUP] = 0;
                    keyboard_state[KEY_PGDN] = 0;
                }

                if ((_pad_status & MYOSD_START /*& MYOSD_SELECT*/)  &&  (_pad_status /*& MYOSD_A*/ & MYOSD_L1) && myosd_saveload_combo)
                {
                    myosd_savestate = 1;
                    break;
                }
                if ((_pad_status /*& MYOSD_SELECT*/ & MYOSD_START)  &&  (_pad_status /*& MYOSD_A*/ & MYOSD_R1) && myosd_saveload_combo)
                {
                    myosd_loadstate = 1;
                    break;
                }
            }

            // lo cambio de 0 a i...
            if (joystick_read_analog(i, 'x') == 0 && joystick_read_analog(i, 'y')==0)
            {
                joy_hats[i][0] = ((_pad_status & MYOSD_UP) != 0) ? 0x80 : 0;
                joy_hats[i][1] = ((_pad_status & MYOSD_DOWN) != 0) ? 0x80 : 0;
                joy_hats[i][2] = ((_pad_status & MYOSD_LEFT) != 0) ? 0x80 : 0;
                joy_hats[i][3] = ((_pad_status & MYOSD_RIGHT) != 0) ? 0x80 : 0;
                joy_axis[i][0] = 0;
                joy_axis[i][1] = 0;

                lightgun_axis[i][0] = (int)(lightgun_x[myosd_pxasp1 ? 0 : i] *  32767 *  2 );
                lightgun_axis[i][1] = (int)(lightgun_y[myosd_pxasp1 ? 0 : i] *  32767 * -2 );

                mouse_axis[i][0] = (int)(mouse_x[myosd_pxasp1 ? 0 : i] * INPUT_RELATIVE_PER_PIXEL);
                mouse_axis[i][1] = (int)(mouse_y[myosd_pxasp1 ? 0 : i] * INPUT_RELATIVE_PER_PIXEL);
            }
            else
            {

                joy_axis[i][0] = (int)(joystick_read_analog(i, 'x') *  32767 *  2 );
                joy_axis[i][1] = (int)(joystick_read_analog(i, 'y') *  32767 * -2 );
                joy_hats[i][0] = 0;
                joy_hats[i][1] = 0;
                joy_hats[i][2] = 0;
                joy_hats[i][3] = 0;

                lightgun_x[i] = 0;
                lightgun_y[i] = 0;
                lightgun_axis[i][0] = 0;
                lightgun_axis[i][1] = 0;

                mouse_x[i] = 0;
                mouse_y[i] = 0;
                mouse_axis[i][0] = 0;
                mouse_axis[i][1] = 0;
            }

            if (myosd_inGame && !myosd_in_menu && myosd_autofire && !netplay)
            {
                old_B_pressed[i] = B_pressed[i];
                B_pressed[i] = _pad_status & MYOSD_B;

                if (!old_B_pressed[i] && B_pressed[i])
                {
                    enabled_autofire[i] = !enabled_autofire[i];
                }
            }
            else
                enabled_autofire[i] = false;

            if (enabled_autofire[i])//AUTOFIRE
            {
                int value  = 0;
                switch (myosd_autofire) {
                case 1:                    value = 1;                    break;
                case 2:                    value = 2;                    break;
                case 3:                    value = 4;                    break;
                case 4:                    value = 6;                    break;
                case 5:                    value = 8;                    break;
                case 6:                    value = 10;                    break;
                case 7:                    value = 13;                    break;
                case 8:                    value = 16;                    break;
                case 9:                    value = 20;                    break;
                default:
                    value = 6;                    break;
                    break;
                }

                joy_buttons[i][0] = fire[i]++ >=value ? 0x80 : 0;
                if (fire[ i] >= value*2)
                    fire[ i] = 0;

            }
            else
                joy_buttons[i][0]  = ((_pad_status & MYOSD_B) != 0) ? 0x80 : 0;

            joy_buttons[i][1]  = ((_pad_status & MYOSD_X) != 0) ? 0x80 : 0;
            joy_buttons[i][2]  = ((_pad_status & MYOSD_A) != 0) ? 0x80 : 0;
            joy_buttons[i][3]  = ((_pad_status & MYOSD_Y) != 0) ? 0x80 : 0;
            joy_buttons[i][4]  = ((_pad_status & MYOSD_L1) != 0) ? 0x80 : 0;
            joy_buttons[i][5]  = ((_pad_status & MYOSD_R1) != 0) ? 0x80 : 0;

            if (i!=0  && (myosd_num_of_joys==1 || myosd_num_of_joys==0) && !netplay)
                continue;

            joy_buttons[i][6]  = ((_pad_status & MYOSD_SELECT ) != 0) ? 0x80 : 0;
            joy_buttons[i][7]  = ((_pad_status & MYOSD_START  ) != 0) ? 0x80 : 0;
        }

        for (int i=0; i<4; i++)
        {
            mouse_x[i] = 0;
            mouse_y[i] = 0;
        }
    }
    else if (mystate == STATE_LOADSAVE)
    {

        keyboard_state[KEY_ESCAPE] = 0;
        keyboard_state[KEY_1] = 0;
        keyboard_state[KEY_2] = 0;

        if (myosd_exitGame || handle->state.ext & NP_EXIT || handle->peer_state.ext & NP_EXIT)
        {
            if (netplay)
            {
                if (handle->state.ext & NP_EXIT || handle->peer_state.ext & NP_EXIT)
                {
                    keyboard_state[KEY_ESCAPE] = 0x80;
                    mystate = STATE_NORMAL;
                    myosd_ext_status &= ~ NP_EXIT;
                }
                else
                {
                    myosd_ext_status |= NP_EXIT;
                }
            }
            else
            {
                keyboard_state[KEY_ESCAPE] = 0x80;
                mystate = STATE_NORMAL;
            }

            myosd_exitGame = 0;
            handle->state.ext &= ~ NP_EXIT;
            handle->peer_state.ext &= ~ NP_EXIT;
        }

        if ((_pad_status & MYOSD_B) != 0)
        {
            keyboard_state[KEY_1] = 0x80;
            mystate = STATE_NORMAL;
            do {}while (myosd_joystick_read(0) & MYOSD_B);
        }

        if ((_pad_status & MYOSD_X) != 0)
        {
            keyboard_state[KEY_2] = 0x80;
            mystate = STATE_NORMAL;
            do {}while (myosd_joystick_read(0) & MYOSD_X);
        }

    }
    else {/*???*/}

}

//============================================================
//  osd_customize_inputport_list
//============================================================

void osd_customize_input_type_list(input_type_desc *typelist)
{
	input_type_desc *typedesc;

	for (typedesc = typelist; typedesc != NULL; typedesc = typedesc->next)
	{
		switch (typedesc->type)
		{
			case IPT_UI_UP:
			case IPT_UI_DOWN:
			case IPT_UI_LEFT:
			case IPT_UI_RIGHT:
			case IPT_UI_CANCEL:
			case IPT_UI_PAGE_UP:
			case IPT_UI_PAGE_DOWN:
				continue;
		}

		if(typedesc->type >= __ipt_ui_start && typedesc->type <= __ipt_ui_end)
			input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);

		if(typedesc->type >= IPT_MAHJONG_A && typedesc->type <= IPT_SLOT_STOP_ALL)
			input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);

	}

	for (typedesc = typelist; typedesc != NULL; typedesc = typedesc->next)
	{

		switch (typedesc->type)
		{
			case IPT_UI_CONFIGURE:
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0));
				break;
			case IPT_COIN1:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0));
				break;
			case IPT_START1:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0));
				break;
			case IPT_COIN2:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 1));
				break;
			case IPT_START2:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 1));
				break;
			case IPT_COIN3:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 2));
				break;
			case IPT_START3:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 2));
				break;
			case IPT_COIN4:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON7, 3));
				break;
			case IPT_START4:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 0), INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				input_seq_append_or(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON8, 3));
				break;
			case IPT_UI_LOAD_STATE:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F7);
				break;
			case IPT_UI_SAVE_STATE:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F8);
				break;
			/*case IPT_UI_CANCEL:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 0));
				break;*/
			case IPT_UI_SELECT:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(/*JOYCODE_BUTTON1*/JOYCODE_BUTTON2, 0));
				break;
			case IPT_UI_UP:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				break;
			case IPT_UI_DOWN:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				break;
			case IPT_UI_LEFT:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 0));
				break;
			case IPT_UI_RIGHT:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				break;
			case IPT_OSD_1:
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(/*JOYCODE_BUTTON2*/JOYCODE_BUTTON1, 0));
				break;
			case IPT_JOYSTICK_UP:
			case IPT_JOYSTICKLEFT_UP:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2UP),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3UP),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4UP),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 3));
				break;
			case IPT_JOYSTICK_DOWN:
			case IPT_JOYSTICKLEFT_DOWN:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2DOWN),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3DOWN),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4DOWN),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 3));
				break;
			case IPT_JOYSTICK_LEFT:
			case IPT_JOYSTICKLEFT_LEFT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2LEFT),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3LEFT),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4LEFT),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 3));
				break;
			case IPT_JOYSTICK_RIGHT:
			case IPT_JOYSTICKLEFT_RIGHT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2RIGHT),1), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3RIGHT),2), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4RIGHT),3), SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 3));
				break;
			case IPT_POSITIONAL:
				if(typedesc->group == IPG_PLAYER1)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 0));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 1));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 2));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
					input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON5, 3));
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON6, 3));
				}
				break;
			case IPT_PADDLE:
			case IPT_TRACKBALL_X:
			case IPT_AD_STICK_X:
			case IPT_LIGHTGUN_X:
			case IPT_DIAL:
				if(typedesc->group == IPG_PLAYER1)
				{				   
                                   //if(typedesc->type==IPT_LIGHTGUN_X)                                   
                                      //input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_X,0),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_X,0));
input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_X,0),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_X,0),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(MOUSECODE_X,0));
                                   //else
                                      //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,0));
                                   input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT),0));
				   input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
				    //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,1));
	                            input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_X,1),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_X,1));
                                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2LEFT),1));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2RIGHT),1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
				    //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,2));
 	                            input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_X,2),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_X,2));
                                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3LEFT),2));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3RIGHT),2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
				    //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_X,3));
	                            input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_X,3),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_X,3));
                                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4LEFT),3));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4RIGHT),3));
				}
				break;
			case IPT_PADDLE_V:
			case IPT_TRACKBALL_Y:
			case IPT_AD_STICK_Y:
			case IPT_LIGHTGUN_Y:
			case IPT_POSITIONAL_V:
			case IPT_DIAL_V:
				if(typedesc->group == IPG_PLAYER1)
				{
                                   //if(typedesc->type==IPT_LIGHTGUN_Y)                                   
                                      //input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_Y,0),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,0));
input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_Y,0),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,0),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(MOUSECODE_Y,0));
                                   //else
                                      //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,0));
                                   input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),0));
				   input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
                                    //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,1));
                                    input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_Y,1),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,1));
                                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2UP),1));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2DOWN),1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
                                    //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,2));
                                    input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_Y,2),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,2));
                                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3UP),2));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3DOWN),2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
                                    //input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,3));
                                    input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD],INPUT_CODE_SET_DEVINDEX(GUNCODE_Y,3),SEQCODE_OR,INPUT_CODE_SET_DEVINDEX(JOYCODE_Y,3));
                                    input_seq_set_1(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4UP),3));
				    input_seq_set_1(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4DOWN),3));
				}
				break;
			case IPT_MOUSE_X:
				if(typedesc->group == IPG_PLAYER1)
				{
                                   input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1LEFT),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 0));
				   input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1RIGHT),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{

					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2LEFT),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 1));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2RIGHT),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
	                                input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3LEFT),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 2));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3RIGHT),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
	                                input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4LEFT),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_LEFT_SWITCH, 3));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4RIGHT),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_X_RIGHT_SWITCH, 3));
				}
				break;
			case IPT_MOUSE_Y:
				if(typedesc->group == IPG_PLAYER1)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1UP),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 0));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT1DOWN),0),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 0));
				}
				else if(typedesc->group == IPG_PLAYER2)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2UP),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 1));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT2DOWN),1),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 1));
				}
				else if(typedesc->group == IPG_PLAYER3)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3UP),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 2));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT3DOWN),2),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 2));
				}
				else if(typedesc->group == IPG_PLAYER4)
				{
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_DECREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4UP),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_UP_SWITCH, 3));
					input_seq_set_3(&typedesc->seq[SEQ_TYPE_INCREMENT],INPUT_CODE_SET_DEVINDEX(STANDARD_CODE(JOYSTICK, 0, SWITCH, NONE, HAT4DOWN),3),SEQCODE_OR, INPUT_CODE_SET_DEVINDEX(JOYCODE_Y_DOWN_SWITCH, 3));
				}
				break;
			case IPT_JOYSTICKRIGHT_UP:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON4, 3));
				break;
			case IPT_JOYSTICKRIGHT_DOWN:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON2, 3));
				break;
			case IPT_JOYSTICKRIGHT_LEFT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON3, 3));
				break;
			case IPT_JOYSTICKRIGHT_RIGHT:
				if(typedesc->group == IPG_PLAYER1)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 0));
				else if(typedesc->group == IPG_PLAYER2)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 1));
				else if(typedesc->group == IPG_PLAYER3)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 2));
				else if(typedesc->group == IPG_PLAYER4)
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], INPUT_CODE_SET_DEVINDEX(JOYCODE_BUTTON1, 3));
				break;
			case IPT_AD_STICK_Z:
			case IPT_START:
			case IPT_SELECT:
				input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
				break;
                
			//default:
				//input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
		}
	}
}

static INT32 my_get_state(void *device_internal, void *item_internal)
{
	UINT8 *keystate = (UINT8 *)item_internal;
	return *keystate;
}

static INT32 my_axis_get_state(void *device_internal, void *item_internal)
{
	INT32 *axisdata = (INT32 *) item_internal;
	return *axisdata;
}

