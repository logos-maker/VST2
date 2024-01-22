// More examples with the RST framework can be found at https://github.com/logos-maker/RST and the plugin graphic system ikiGUI can be found at https://github.com/logos-maker/ikiGUI
#include <stdint.h>		// For variable declaration names.
#include <stdlib.h>		// For malloc function.
#include <stdio.h>		// For text in window
#include "../libs/ikigui.h"	// cross platform audio plugin GUI library for tiled graphics and animations.
#include "../libs/rst.h"	// definitions for making audio plugins compatible with the ABI.

//**************************************************************************************
//    Plugin settings - Info that is uniqe to your plug that the host want's to know

char brand_name[]   = "DSC";    		// Place your brand name inside ""
char product_name[] = "DSC-THELAY";		// Place your plug name inside ""
#define VERSION_NUMBER_OF_THIS_SPECIFIC_PLUG 1  // Version number for this plug is set to 1. Increase number for your plug when you make improvements.
#define TYPE_OF_PLUG EFFECT_UNIT 		// Set this to EFFECT_UNIT or SYNTHESIZER
#define NUMBER_OF_PRESETS 3			// Number of presets inside the plug.
#define NO_MIDI					// If NO_MIDI is defined, the MIDI handling is turned of, and this only makes sense for a EFFECT_UNIT  <----- Turns of MIDI system function:  MIDI_in()

//**************************************************************************************************************************************************
//   Global variables/data for the plug -  You can only have things here that do not change and is the same for all instances/copies of your plug
//**************************************************************************************************************************************************
  /* Place your global variables here - not anything that's can have different state in different plug instances */

//******************************
//   Plugin GUI declarations

  /* Place your declations for your GUI here */

// The definitions for substitions of names
#define PARAMETER_COL 5
#define PARAMETER_ROW 1
#define NUMBER_OF_PARAMETERS PARAMETER_COL * PARAMETER_ROW  // Uniqe number of parameters in plug.
#define H_DISTANCE 64
#define V_DISTANCE 64
#define PLUG_WIDTH  350
#define PLUG_HEIGHT 90

// The graphic art (a normal BMP file converted to array declarations), is in the following files...
#include "../gfx/knob.h"	// Embedded graphics for knobs in 32bit AARRGGBB BMP format.
#include "../gfx/bg.h"	// Embedded graphics for a background picture in 332bit AARRGGBB BMP format.
ikigui_image knob_anim;	// Global graphics for knobs.
ikigui_image bg;	// Global graphics for background art.


//***************************************************************************************************
//   Plugin data struct  -  Things that uniqe for each plug instance that is needed for this plug
//***************************************************************************************************

struct data{     // Things that uniqe for each plug instance that is needed for this plug.
	// For audio algorithm
	float delaybuffer[2][100000]; // stereo buffer for delay
	float filt_buff[2][16];	  // stereo buffer for filter
	int delaytap;		  // the dalay tap for the delay
	// For graphics
	ikigui_window mywin; // A plugin window declaration.
	ikigui_map knob_map; // A tilemap declaration.
} data;
#include "../libs/rst_framework_head.c"	// Has to be below 'struct data'.


//*****************************
//   User defined functions  
//*****************************
  /* Place your functions here */


//*****************************************************
//   Patch information - Parameters, Names & Values
//*****************************************************

//*********************
//   Plugin presets
struct preset presets[NUMBER_OF_PRESETS] = {	// the preset presets inside the plug. Change NUMBER_OF_PRESETS if changing the number of presets.
	{"SHORT",  0.1,0.9,0.3,0.2,0.3},// First preset
	{"LONG",   0.7,0.7,0.7,0.1,0.2},// Second preset
	{"BYPASS", 0.7,0.7,0.7,0.1,0.0},// Third preset 
};

//***********************************************
//   Names of all user parameters in the plug
void getParameterName(int32_t index,  char* ptr){ // Names of all user parameters in the plug. Max 7 characters for each name(8 if the implicit \n is included), but the spec is futile. Hosts allows longer names for compatibiliy reasons as almost no plugins follows this limit.
        switch(index){ // Copy the name of the the right paramter to be displayed in the host.
                case  0: strcpy(ptr, "DELAY");          break; // Name of the first  parameter is between ""
                case  1: strcpy(ptr, "OVERDRIVE");      break; // Name of the second parameter is between ""
                case  2: strcpy(ptr, "CUTOFF");		break; // ...and so on.
                case  3: strcpy(ptr, "FEEDBACK");       break; // 
                case  4: strcpy(ptr, "DRY/WET");        break;

                default: strcpy(ptr, "???");	 break; // A default name, reminding to add create any missing case for some parameter.
        }
};

//********************************************
//   Generate parameter text to show in DAW
void getParameterText(plug_instance *plug,int32_t index,char* ptr){ if(NULL!=gcvt(plug->pth.knob[index], 6, ptr)) return; }; // Host whant the indexed parameter value in text.


//**************************
//   GUI related functions
//**************************

void mouse_handling(plug_instance *plug){		// Mouse handling
        ikigui_get_events(&plug->dat.mywin);		// update window events
	struct mouse* m = &plug->dat.mywin.mouse ;	// Make a short hand name, for the code below

	if(m->left_click){ // Mouse down event
                plug->knob_selected = ikigui_mouse_pos(&plug->dat.knob_map, m->x -10, m->y -10);
                if(-1 != plug->knob_selected){ // if mouse pointer was over a tile
                        m->pressed = 1; // That we has sent to the host that we have grabbed something
                        plug->hostcall(&plug->plughead, dawAutomateStart, plug->knob_selected, 0, 0, 0); // Tell host we grabed the knob 
                }
        }
        if(m->pressed){ // Change pressed knob according to relative mouse movement.
                float temp = plug->pth.knob[plug->knob_selected] + (float)(plug->dat.mywin.mouse.old_y - plug->dat.mywin.mouse.y) * 0.01; // Relative mouse movement 
                if(0 > temp)            plug->pth.knob[plug->knob_selected] = 0; // knob can't go below 0.
                else if(1 < temp)       plug->pth.knob[plug->knob_selected] = 1; // knob can't go above 1.
                else                    plug->pth.knob[plug->knob_selected] = temp ; // the new knob value.
                
                plug->hostcall(&plug->plughead, dawAutomate,   plug->knob_selected, 0, 0, plug->pth.knob[plug->knob_selected]); // send new knob value to the DAW.
        }
        if(m->left_release && m->pressed){// Release of mouse button when previus informed to the host
                m->pressed = 0;
                plug->hostcall(&plug->plughead, dawAutomateEnd,   plug->knob_selected, 0, 0, 0); // Tell the DAW that we released the knob.
        }
        for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++ ){ // Update the tile map, with all knob values.
                plug->dat.knob_map.map[i] = (char)(plug->pth.knob[i] * plug->dat.knob_map.max_index ); // Select animation frame for knob value.
        }
}
void draw_graphics(plug_instance *plug){			// The DAW calls this when it wants to redraw the editor...
	ikigui_image_draw(&plug->dat.mywin.frame,&bg, 0, 0);	// Draw background.
	ikigui_map_draw(&plug->dat.knob_map,0,10,10);		// Draw knobs.
}
void prepare_graphics(plug_instance *plug,void *ptr){	// The DAW calls this when it wants to open the editor window...
	// Image composite of a background image for the plug
	ikigui_image_empty(&bg, PLUG_WIDTH,PLUG_HEIGHT);
	ikigui_draw_gradient(&bg,0x00eeeedd, 0x00999999);
	ikigui_bmp_include(&bg,bg_array); 

	// For the knob animation
	ikigui_bmp_include(&knob_anim,knob_array); // Load knob graphics.						
	ikigui_map_init(&plug->dat.knob_map, &plug->dat.mywin.frame,&knob_anim,0,H_DISTANCE,V_DISTANCE,64,64,PARAMETER_COL,PARAMETER_ROW); // Set columns and rows of knobs in the tile array, and tile width and hight.
}
void destroy_graphics(plug_instance *plug,void *ptr){	// When the DAW closes the window...

}


//***********************************************
//   Some functions for administative purposes
//***********************************************

void set_samplerate(plug_instance *plug){		// Is called by the DAW when it gives you the samplerate your plug needs to use...

}
void audioplugOpen(plugHeader *plugin){ 		// Is executed when the plug opens
	plug_instance *plug = (plug_instance*)plugin->object;
}
void audioplugClose(plugHeader *plugin){ 		// Is executed when the plug going to be closed
	plug_instance *plug = (plug_instance*)plugin->object;
	/* Place your code here thats going to run before the instance is going to be cloased */
}

//***********************************************
//   Audio & MIDI port communciation functions  
//***********************************************

//***********************************************************************************************************************************************************************************
//   Plugin algorithm - audio_in_out_float() The DAW calls this function to make the plugin process audio in buffer and fill with audio out data. Audio levels is between -1 to +1
void audio_in_out_float(plugHeader *plugin, float **inputs, float **outputs, int32_t sampleFrames){ plug_instance *plug = (plug_instance*)plugin->object;

	enum { PAR_DELAY,PAR_OVERDR,PAR_CUTOFF,PAR_FEEDBK,PAR_DRY}; // Create enumeration on words from 0 to 4 to get the right parameters.
        float cutoff = (plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]); // log4 knob value
        for(int j = 0; j < sampleFrames; j++){ // Loop trough all the samples in buffer. Put your audio algorithm inside here....
		float filt_in, filt_out;
		for(char i = 0; i < 2 ; i++){ // Stereo handling
		        filt_in = inputs[i] [j] * (plug->pth.knob[PAR_OVERDR]*4); // AMP
			if(filt_in>1)filt_in=1;else if(filt_in<-1)filt_in=-1; // CLAMP
		        for(int k = 0 ; k < 5 ; k++){	plug->dat.filt_buff[i][k+1] = ((filt_in - plug->dat.filt_buff[i][k]) * cutoff) + plug->dat.filt_buff[i][k]; } // LOWPASS FILTER
		        plug->dat.filt_buff[i][0] = filt_out = plug->dat.filt_buff[i][5]; // FILTER FEEDBACK
		        plug->dat.delaybuffer[i] [plug->dat.delaytap] = ( plug->dat.delaybuffer[i] [plug->dat.delaytap] * (plug->pth.knob[PAR_FEEDBK] ) ) +  ( filt_out * (1-plug->pth.knob[PAR_FEEDBK]) ) ;
		        outputs[i][j]   = inputs[i][j] * (1-plug->pth.knob[PAR_DRY]) + plug->dat.delaybuffer[i][plug->dat.delaytap +1] * plug->pth.knob[PAR_DRY]; // OUTPUT STAGE
		}
		plug->dat.delaytap++; if(plug->dat.delaytap > (int)(plug->pth.knob[PAR_DELAY] *(50000-1))) plug->dat.delaytap = 0; // Loop the delay tap
        }

}

#include "../libs/rst_framework.c"