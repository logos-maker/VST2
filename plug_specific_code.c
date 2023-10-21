// In this C file, you make the changes specific to your plug

//*********************
//     Plugin GUI
//*********************

// The graphic art (in BMP format) is converted to array declarations, in the following files...
#include "gfx/knob.h"	// Embedded graphics for knobs in 32bit AARRGGBB BMP format.
#include "gfx/bg.h"	// Embedded graphics for background in 32bit AARRGGBB BMP format.
#define PLUG_WIDTH 350
#define PLUG_HEIGHT 90

// Structs to hold graphic art, uniqe to this plug (one for each picture to be included). .
ikigui_image knob_anim;	// Raw global source graphics holder for knobs
ikigui_image bg;	// Raw global source graphics holder for background

void draw_graphics(plug_instance *plug){ 	   		// The daw calls this when it wants to redraw the editor  
	ikigui_image_draw(&plug->dat.mywin.frame,&bg, 0, 0); 	// Draw gackground.
	ikigui_map_draw(&plug->dat.knob_map,0,10,10);		// Draw knobs.
}
void prepare_graphics(plug_instance *plug,void *ptr){	// The daw calls this when it wants to open the editor window.
	ikigui_bmp_include(&knob_anim,knob_array);						// Load knob graphics.
	ikigui_bmp_include(&bg,bg_array);							// Load background graphics.
	ikigui_map_init(&plug->dat.knob_map, &plug->dat.mywin.frame,&knob_anim,5,1,64,56);	// Set columns and rows of knobs in the tile array, and tile width and hight.
}
void destroy_graphics(plug_instance *plug,void *ptr){

}


//*********************
//   Plugin settings
//*********************

char brand_name[]   = "DSC";    // Place your brand name inside ""
char product_name[] = "THELAY";	// Place your plug name inside ""
#define VERSION_NUMBER_OF_THIS_SPECIFIC_PLUG 1  // Version number for this plug is set to 1. Increase number for your plug when you make improvements.
#define NUMBER_OF_PRESETS 3			// Number of presets inside the plug.
#define TYPE_OF_PLUG EFFECT_UNIT // Set this to EFFECT_UNIT or SYNTHESIZER

struct preset presets[NUMBER_OF_PRESETS] = {	// the preset presets inside the plug. Change NUMBER_OF_PRESETS if changing the number of presets.
	{"SHORT",  0.1,0.9,0.3,0.2,0.3},// First preset
	{"LONG",   0.7,0.7,0.7,0.1,0.2},// Second preset
	{"BYPASS", 0.7,0.7,0.7,0.1,0.0},// Third preset
};
void getParameterName(int32_t index,  char* ptr){ // Names of all user parameters in the plug. Max 7 characters for each name(8 if the implicit \n is included), but the spec is futile.
        switch(index){ // Copy the name of the the right paramter to be displayed in the host.
                case  0: strcpy(ptr, "DELAY");          break; // Name of the first  parameter is between ""
                case  1: strcpy(ptr, "OVERDRIVE");      break; // Name of the second parameter is between ""
                case  2: strcpy(ptr, "CUTOFF");		break; // Is "to long" but works. Hosts allows longer names for compatibiliy reasons.
                case  3: strcpy(ptr, "FEEDBACK");       break; // ...as almost no plugins follows this limit.
                case  4: strcpy(ptr, "DRY/WET");        break;
                default: strcpy(ptr, "???");		break; // A default name, reminding to add create any missing case for some parameter.
        }
};
void getParameterText(plug_instance *plug,int32_t index,char* ptr){ if(NULL!=gcvt(plug->pth.knob[index], 6, ptr)) return; }; // Host whant the plug indexed parameter value in text. if(NULL is for not giving compilation warnings).


//*********************
//  Plugin algothithm
//*********************

// Function is called by the host to make your plug process the audio buffer. Here the 'audio in' gets processed and sent to audio out. Audio levels is between -1 to +1
// plugin specific variables used inside this function, is placed inside the plug_instance struct in generic_fx_code.c starting at line 18.
void plugProcessSamplesFloat32(plugHeader *vstPlugin, float **inputs, float **outputs, int32_t sampleFrames){ plug_instance *plug = (plug_instance*)vstPlugin->object;
	enum { PAR_DELAY,PAR_OVERDR,PAR_CUTOFF,PAR_FEEDBK,PAR_DRY}; // Create enumeration on words from 0 to 4 to get the right parameters.
        float cutoff = (plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]); // log4 knob value
        for(int j = 0; j < sampleFrames; j++){ // Loop trough all the samples in buffer. Put your audio algorithm inside here....
		float filt_in, filt_out;
		for(char i = 0; i < 2 ; i++){ // stereo handling
		        filt_in = inputs[i] [j] * (plug->pth.knob[PAR_OVERDR]*4); // AMP
			if(filt_in>1)filt_in=1;else if(filt_in<-1)filt_in=-1; // CLAMP
		        for(int k = 0 ; k < 5 ; k++){	plug->dat.filt_buff[i][k+1] = ((filt_in - plug->dat.filt_buff[i][k]) * cutoff) + plug->dat.filt_buff[i][k]; } // LOWPASS FILTER
		        plug->dat.filt_buff[i][0] = filt_out = plug->dat.filt_buff[i][5]; // FILTER FEEDBACK
		        plug->dat.delaybuffer[i] [plug->dat.delaytap] = ( plug->dat.delaybuffer[i] [plug->dat.delaytap] * (plug->pth.knob[PAR_FEEDBK] ) ) +  ( filt_out * (1-plug->pth.knob[PAR_FEEDBK]) ) ;
		        outputs[i][j]   = inputs[i][j]* (1-plug->pth.knob[PAR_DRY]) + plug->dat.delaybuffer[i][plug->dat.delaytap +1] * plug->pth.knob[PAR_DRY]; // OUTPUT STAGE
		}
		plug->dat.delaytap++; if(plug->dat.delaytap > (int)(plug->pth.knob[PAR_DELAY] *(50000-1))) plug->dat.delaytap = 0; // Loop the delay tap
        }
}
