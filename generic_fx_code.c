// This is an example of an audio effect unit, using the VST2.4 plugin ABI (ABI Application Binary Interface).

#include <stdint.h> // For variable declaration names.
#include <stdbool.h>// For true and false keywords.
#include <stdio.h>  // For sprintf.
#include <stdlib.h> // For malloc function.
#include <string.h> // For memcpy

#include "libs/ikigui.h"	// cross platform audio plugin GUI library for tiled graphics and animations.
#include "libs/rst.h"		// definitions for making VST2 audio plugins compatible with the ABI.

//********************************
//     Plugin data and buffers
//********************************

struct data{     // uniqe variables for this plug
    // For audio algorithm
    float delaybuffer[2][100000]; // stereo buffer for delay
    float filt_buff[2][16];	  // stereo buffer for filter
    int delaytap;		  // the dalay tap for the delay
    // For graphics
    ikigui_map knob_map; // A tilemap declaration
    ikigui_window mywin; // A plugin window declaration
} data;

#define NUMBER_OF_PARAMETERS 5  // Uniqe number of parameters in plug.

//*******************************************************************
// The rest in this file is generic stuff not specific to this plug 
//*******************************************************************

struct patch{ // the patch that the DAW will save and restore when saving and loading audio projects.
    float knob[NUMBER_OF_PARAMETERS];
};

typedef struct{ // general declarations
    struct plugHeader plughead; // It must be first in the struct. Note that each instance has this header.
    plugPtr (*hostcall) (plugHeader* effect, int32_t opcode, int32_t index, plugPtr value, void* ptr, float opt); // VstIntPtr (*audioMasterCallback) i dokumentation
    struct patch pth; // all data to save and restore by host be the op-codes plugGetChunk and plugSetChunk functions.
    struct data dat; // buffers and variables for the audio algorithm.
    struct ERect myrect;
    int program_no; // the current preset number (not used in this plug).
    int pressed ;
    int down_x ;
    int down_y ;
    int old_button_press; // What the mouse buttons was before
    int knob_selected;
} plug_instance;

struct preset{ // general struct used for internal presets in the plug
	char preset_name[24];
	float param[NUMBER_OF_PARAMETERS]; // Maybe not optimal.
};

float samplerate; // Is here only for demonstration purposes.

#include "plug_specific_code.c"

void getProgramName(int32_t index,  char* ptr){	strcpy(ptr,presets[index].preset_name); }; // Copy the preset name and give to the host when asked for.

void setknob(plug_instance* plug,int knob,float value){
	plug->pth.knob[knob] = value ;
	plug->hostcall(&plug->plughead, 0, knob, 0, 0, plug->pth.knob[knob]); // audioMasterAutomate has op-code 0
}

// Function is called by the host to make your plug do and answer different things that the DAW asks for.
plugPtr plugInstructionDecoder(plugHeader *vstPlugin, int32_t opCode, int32_t index, plugPtr value, void *ptr, float opt){ // Pointer to this function is used in the myplugin header
    plug_instance *plug = (plug_instance*)vstPlugin->object;
    switch(opCode){
        case plugEditRedraw:                    
                ikigui_get_events(&plug->dat.mywin); // update window events
                if((plug->old_button_press == 0) & (plug->dat.mywin.mouse.buttons & MOUSE_LEFT)){ // Mouse down event
                        plug->knob_selected = ikigui_mouse_pos(&plug->dat.knob_map, plug->dat.mywin.mouse.x -16, plug->dat.mywin.mouse.y-16);
                        if(-1 != plug->knob_selected){ // if mouse ponter was over a tile
                                plug->pressed = 1;
                                plug->hostcall(&plug->plughead, 43, plug->knob_selected, 0, 0, 0); // Tell host we grabed the knob 
                        }
                }
                if(plug->pressed){ // Change pressed knob according to relative mouse movement.
                        float temp = plug->pth.knob[plug->knob_selected] + (float)(plug->down_y - plug->dat.mywin.mouse.y) * 0.01; 
                        if(0 > temp)            plug->pth.knob[plug->knob_selected] = 0; // knob can't go below 0.
                        else if(1 < temp)       plug->pth.knob[plug->knob_selected] = 1; // knob can't go above 1.
                        else                    plug->pth.knob[plug->knob_selected] = temp ; // the new knob value.
                        
                        plug->hostcall(&plug->plughead, 0,   plug->knob_selected, 0, 0, plug->pth.knob[plug->knob_selected]); // send new knob value to the DAW.
                }

                // values for recognicing changes in mousemovements and mouse buttons.
                plug->old_button_press = plug->dat.mywin.mouse.buttons;  // old value for buttons. For finding changes later on.
                plug->down_x = plug->dat.mywin.mouse.x ;     // old value for x coordinate.
                plug->down_y = plug->dat.mywin.mouse.y ;     // old value for y coodrinate.
                if(plug->pressed && (plug->dat.mywin.mouse.buttons == 0)){ // Release of mouse button
                        plug->pressed = 0;
                        plug->hostcall(&plug->plughead, 44,   plug->knob_selected, 0, 0, 0); // Tell the DAW that we released the knob.
                }
                for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++ ){ // Update the tile map, with all knob values.
                        plug->dat.knob_map.map[i] = (char)(plug->pth.knob[i] * 64) +31; // Select animation frame for knob value.
                }
		
		draw_graphics(plug); // Draw all graphics, handled by plug_specific_code.c

                ikigui_update_window(&plug->dat.mywin); // Update all graphics in the plugin editor
        break;
        case plugEditOpen:{
	    prepare_graphics(plug,ptr); // Allocate everything needed for the new editor window.
	    ikigui_open_plugin_window(&plug->dat.mywin,ptr,PLUG_WIDTH,PLUG_HEIGHT);	// Open the editor window in host.
            return  1;//true;
        }
        break;
	case plugEditGetRect: plug->myrect.bottom = PLUG_HEIGHT; plug->myrect.right = PLUG_WIDTH; *(struct ERect**)ptr = &plug->myrect ; return true;	// Host asks about the editor size.
	case plugGetPlugCategory:	return TYPE_OF_PLUG; // Return 1 if the plug is an effect, or 2 if it's a synthesizer.
        case plugEditClose:             destroy_graphics(plug,ptr);			return true;   // Close plug edit window, not the plug instance.
        case plugGetProductString:      strcpy((char*)ptr, product_name);		return true;   // The name of the plug
        case plugGetVendorString:       strcpy((char*)ptr, brand_name);			return true;   // request for the vendor string (usually used in the UI for plugin grouping)
        case plugSetSampleRate:         samplerate = opt;				return true;   // Host tells plug the samplerate
	case plugSetProgram:		if(plug->program_no == value){ return true;} for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++){ plug->program_no = value; setknob(plug,i,presets[plug->program_no].param[i]); } break;
	case plugGetProgram:		return plug->program_no;			// Return current preset program number 
	case plugGetProgramNameIndexed:	getProgramName(index,(char*)ptr);		return true;	// If the category value is -1 all presets are enumerated linearily. Categorys starts at index 0.
	case plugGetProgramName:	getProgramName(plug->program_no,(char*)ptr);	return true;	// Alternative to plugGetProgramNameIndexed as some hosts use this instead.	
        case plugGetParamName:          getParameterName(index, (char*)ptr);            return true;	// Host whant the plug to transfer the indexed parameter's name. 
        case plugGetParamText:          getParameterText(plug, index, (char*)ptr);	return true;	// 
        case plugGetVersion:		return 2400; // This plugin follows the VST2.4 ABI.
        case plugCanBeAutomated:        return true; // Return true if if the parameter is automatable. The index variable holds the parameter that the hosts asks about. In this example all parameters is automatable.
        case plugOpen:	   		for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++){ setknob(plug,i,presets[plug->program_no].param[i]); } return true; // Load preset 0. OP-Code is sent after the plug starts.
        case plugClose:			free(plug);					return true; // This op-code is sent by host before the plug gets deallocated from the system. To free resources and so on.
        case plugCanDo:			return -1; // Respond: -1 to everything not supported. // example of checking text... if (0 == strcmp(((char*)ptr),"receiveVstEvents"))   return 1;  //receiveVstEvents
        case plugGetChunk:		{ *(void**)ptr = &plug->pth;  return sizeof(struct patch); break; } // Host saves the plug state inside the host.
        case plugSetChunk:		memcpy(&plug->pth, (unsigned char *)ptr, sizeof(struct patch)); return true; // Host loads an old saved state into plug.
        default: break; // ignoring all other opcodes //fprintf(fp, "index:%d value:%ld ptr:%p opt:%f instance:%d \n",index,value,ptr,opt,instances); // For printing op-codes not implemented.
    }
    return 0; // Return that it's not a supported message number by this plug.
}
// These functions is given to the host in the main() function bellow. The host can't call these without the function pointer to them.
void  plugSetParameter(plugHeader *vstPlugin, int32_t index, float parameter){ plug_instance *plug = (plug_instance*)vstPlugin->object; plug->pth.knob[index] = parameter ; }// Host uses this to set parameter in plug
float plugGetParameter(plugHeader *vstPlugin, int32_t index){ plug_instance *plug = (plug_instance*)vstPlugin->object; return (float)(plug->pth.knob[index]); }// Host uses this to get parameter from plug

void* main(hostCallback HostCallback){ // New plug instances is created here. After plug load-in, it's the only known function for the host to run, that will give addresses for more functions for the host to run.
    struct plugHeader myplugin = {                              // Declaration of the struct that a plugin must return to the host.
        .magicNumber =                                          1450406992, // Identifier that it's an audio plug
        .plugInstuctionDecoderFunc =                            plugInstructionDecoder,  // A callback address for Host to send opcodes to plug
        .plugSetParameterFunc =                                 plugSetParameter, // Set new value of automatable parameter.
        .plugGetParameterFunc =                                 plugGetParameter, // Returns current value of automatable parameter.
        .number_of_programs =                                   NUMBER_OF_PRESETS, // presets
        .number_of_params =                                     NUMBER_OF_PARAMETERS, // parameters
        .number_of_inputs =                                     2, // audio inputs
        .number_of_outputs =                                    2, // audio outputs
        .flags = hasReplacing | hasStateChunk | hasEditor,      // Bitflags for things this plugin supports
        .version = VERSION_NUMBER_OF_THIS_SPECIFIC_PLUG,        // plug-in version, not VST version.
        .plugProcessFloatFunc = plugProcessSamplesFloat32,      // Name of the function where the audio processing is done.
        .plugProcessDoubleFunc = NULL,                          // No such function in this plug. Use for funtionpointer to 64bit floating point handeling
    };
    plug_instance *plug =(plug_instance*)calloc(1,sizeof(plug_instance));       // Allocate memory to the plug instance. calloc sets all allocated memory to zero unlike malloc
    memcpy(&(plug->plughead),&myplugin,sizeof(plugHeader));                     // Copy the above header over to the memory reserved with calloc. Destination address is &(plug->plughead)
    plug->plughead.object = plug;                                               // To be able to find all internal parameters.
    plug->hostcall = HostCallback;						// Store callback to host, so the plug can make calls to the host.
    return plug; // Same as... return &(plug->plughead); // Send address to the plugs plug-header to the host. It's the address to all data that is uniqe for the plug instance.
}
