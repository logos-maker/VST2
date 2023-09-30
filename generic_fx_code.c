// This is an example of an audio effect unit, using the VST2.4 plugin ABI (ABI Application Binary Interface).
// Note thas this example is not compatible with the VST2.4 API (Application Programming Interface). javid
// Known problems: Setings doen't seam to be restored by music programs any more.

// Compile to windows...
// gcc generic_fx_code.c -o thelay.so -fPIC -shared // command for compilation.

// Install toolshain to compile to windows...
// sudo apt-get install mingw-w64
// Compile to windows... https://arrayfire.com/blog/cross-compile-to-windows-from-linux/
// x86_64-w64-mingw32-gcc plugin.c -o thelay.dll -fPIC -shared -lgdi32 // Create a 64bit VST2
// i686-w64-mingw32-gcc plugin.c -o thelay.dll -fPIC -shared -lgdi32 // Create a 32bit VST2

// Command for realtime text-log when using Bitwig as the plugin host...
// tail -F ~/.BitwigStudio/log/engine.log // Terminal command for bitwig info about the plug, for debugging

#include <stdint.h> // For new names for variable declarations.
#include <stdbool.h>// For using true and false keywords.
#include <stdio.h>  // For sprintf. Can maybe be replaced with something faster.
#include <stdlib.h> // For malloc function. Can maybe be replaced with something faster or smaller?
#include <string.h>   // för memcopy?

// includes for opening editor on Linux or Windows
#ifdef __linux__ //linux code goes here...
	#include "libs/spg_plug.h"	// For window and graphics handeling in this case for Linux. SPG - Safe Plugin Graphics (plan is to make it multi platform).
#elif _WIN32 // windows code goes here...
	#include "windows.h"
	#include "libs/win_spg.h" // Include my single header lib, SPG - Safe Plugin Graphics.
#endif

#include "libs/spg_chardis.h"	// For tiled graphics and animations.
#include "gfx/knob.h" // Graphics for knobs in 32bit AARRGGBB BMP format.
#include "gfx/bg.h"   // Graphics for background in 32bit AARRGGBB BMP format.

typedef int64_t plugPtr;
typedef struct plugHeader plugHeader;
struct plugHeader{ // A basic audio effect plugin using the ABI for VST2.4
	int32_t magicNumber;
	plugPtr (*plugInstuctionDecoderFunc)(plugHeader* effect, int32_t opcode, int32_t index, plugPtr value, void* ptr, float opt);
	void(*deprecatedProcess)(void); // not used, is deprecated
        void  (*plugSetParameterFunc)(plugHeader* effect, int32_t index, float parameter);
        float (*plugGetParameterFunc)(plugHeader* effect, int32_t index);
	int32_t number_of_programs;            // number of programs in plug
	int32_t number_of_params;		// all programs are assumed to have numParams parameters
	int32_t number_of_inputs;		// number of audio inputs in plug
	int32_t number_of_outputs;	        // number of audio outputs in plug
	int32_t flags;			// see plugPropertiesFlags
	plugPtr reserved_for_host[2];
	int32_t initialDelay;	        // plug latency in samples
	int32_t deprecated1;
	int32_t deprecated2;
	float   deprecated3;
	void* object;
	void* user;
	int32_t uniqueID; // Depricated by plugs not following it.
	int32_t version;  // Version of this plug, to force updates about plug in host.
        void (*plugProcessFloatFunc) (plugHeader* effect, float** inputs, float** outputs, int32_t sampleFrames); // Used by host to make plug process 32bit audio buffers (float).
        void (*plugProcessDoubleFunc)(plugHeader* effect, double** inputs, double** outputs, int32_t sampleFrames);// Used by host to make plug process 64bit audio buffers(double).
	char reserved3_for_future[56];// reserved
};
typedef plugPtr (*hostCallback) (plugHeader* effect, int32_t opcode, int32_t index, plugPtr value, void* ptr, float opt); // VstIntPtr (*audioMasterCallback) i dokumentation
enum { LEFT, RIGHT }; // 0 for left audio channel, 1 for right audio channel.
enum { UNKNOWN, EFFECT_UNIT, SYNTHESIZER}; // 0 for unknown, 1 for effect unit, 2 for synthesizer.
enum plugPropertiesFlags{
        hasEditor          = 1,   // Plug has an editor window
	hasReplacing       = 16,  // Plug has function for writing audio out over the audio in buffer in 32bit floating point (float).
	hasStateChunk      = 32,  // internal state in plug can be copied to host, for saving and loading audio projects in host.
	hasDoubleReplacing = 4096,// Plug has function for writing audio out over the audio in buffer in 64bit floating point (double).
	hasSynth	   = 256, // Plug is a VSTi.
};
enum opcodes{ // With notes if vestige or the FST header uses this op-codes
        plugEditOpen=14,        //Attention: Is called when the DAW whants the plug to open window.
        plugEditClose=15,       //Attention: After you answered this call, then the window is closed by DAW.
        plugEditGetRect=13,
        plugEditRedraw=19,
        plugOpen = 0,           //Host warns it will start to use this plug instance before it does.
        plugClose = 1,          //Host warns that it will end the usage of this plug instance.
        plugGetParamText=7,     //Host asks for a text to display for the value of a parameter.
        plugGetParamName=8,     //Host asks for the name of a parameter in plug.
        plugSetSampleRate=10,   //The host tells the plug what sample rate that is currently used.
        plugGetChunk=23,        //Host asks for an adress and the length of a data chunk that the host shall save for the plug.
        plugSetChunk=24,        //Host asks for an adress and length of the data segment that the host shall fill to restore an old state.
        plugCanBeAutomated =26, //For the host to check if it can automate a certain parameter.
        plugGetVendorString=47, //For the host to ask for the name of the author of the plug.
        plugGetProductString=48,//For the host to ast for the vestige//FST
        plugCanDo=51,           //For the host to ask if the plug supports a sertain functionallity, the name of the functionallity is supplied in a text string by the host.
        plugGetVstVersion=58,   //Asks the plug what version of the VST ABI that was designed for.
	plugGetProgramNameIndexed=29,//Gets the preset name by index, Bitwig uses this.
	plugGetProgramName=5,	//Alternative to plugGetProgramNameIndexed.
	plugSetProgram=2,	//Change plug preset program
	plugGetProgram=3,	//Ask plug for program number
	plugGetPlugCategory=35  //Ask if it's a effect unit or a synthesizer. Other categories exists but is futile.
};
struct ERect{
	int16_t top;    // Set to zero
	int16_t left;   // Set to zero
	int16_t bottom; // Set to window width
	int16_t right;  // Set to window higth
};
struct ERect myrect = {
    .top = 0,	 // Set to 0, anything else is futile.
    .left = 0,	 // Set to 0, anything else is futile.
    .bottom = 90,// Editor width
    .right = 345,// Editor hight
};
#define MAX_NUMBER_OF_PARAMETERS 128 // Increase this value if your plug needs more parameters than the maximum 128
struct patch{ // all data to save and restore by host when saving and loading audio projects.
    float knob[MAX_NUMBER_OF_PARAMETERS];
};
typedef struct{
    struct plugHeader plughead; // It must be first in the struct. Note that each instance has this header.
    plugPtr (*hostcall) (plugHeader* effect, int32_t opcode, int32_t index, plugPtr value, void* ptr, float opt); // VstIntPtr (*audioMasterCallback) i dokumentation
    struct patch pth; // all data to save and restore by host be the op-codes plugGetChunk and plugSetChunk functions.
    int instance_no; // the number of this particular instance.
    int instance_destroyed; // is this instance is destroyed by the host.
    int window_open;
    int program_no; // the current preset number (not used in this plug).
    spg mywin ; // datat som relaterar till fönstret. NY!
    chardis knober;
    int nedtryck ;
    int ned_x ;
    int ned_y ;
    int old;
    int knob_selected;
    float delaybuffer[2][100000];
    int delaytap;
    float filt_buff[2][16];
} plug_instance;

struct preset{ // struct used for internal presets in the plug
	char preset_name[24];
	float param[MAX_NUMBER_OF_PARAMETERS]; // Maybe not optimal but keeps the specific simple and understandable.
};

float samplerate; // not used, just here for the sake of the example.
plug_instance *instance[1024]; // This is only an array of pointers, and no reserverad data for the instances.
static int instances = 0 ; // It's incremented by one, each time main is called by host to create a new instance.

spg_frame knob_anim;	// Raw global source graphics for knobs
spg_frame bg;		// Raw global source graphics for background

#include "plug_specific_code.c"

void getProgramName(int32_t index,  char* ptr){	strcpy(ptr,presets[index].preset_name); }; // Copy the preset name, and give to the host when asked for.

void setknob(plug_instance* plug,int knob,float value){
	plug->pth.knob[knob] = value ;
	plug->hostcall(&plug->plughead, 0,   knob, 0, 0, plug->pth.knob[knob]); //audioMasterAutomate has op-code 0
}
void debug(char* text){ FILE *fp; fp=fopen("/home/seven/vst_utv/VST2_with_editor_in_C_clean/plugdebug.txt", "a"); fprintf(fp, "\n%s", text); fclose(fp); }
// Function is called by the host to make your plug do and answer different things.
plugPtr plugInstructionDecoder(plugHeader *vstPlugin, int32_t opCode, int32_t index, plugPtr value, void *ptr, float opt){ // Pointer to this function is used in the myplugin header
    plug_instance *plug = (plug_instance*)vstPlugin->object;
    switch(opCode){
	case plugEditGetRect:     *(struct ERect**)ptr = &myrect ;   return true; break; // If host asks about the size of the editor size. Needed if you going to have an editor.
        case plugEditRedraw:                    
                spg_get_events(&plug->mywin);
                if((plug->old == 0) & (plug->mywin.mouse.buttons & MOUSE_LEFT)){ // Mouse down event
                        plug->knob_selected = chardis_mouse_pos(&plug->knober, plug->mywin.mouse.x -16, plug->mywin.mouse.y-16);
                        if(-1 != plug->knob_selected){
                                plug->nedtryck = 1; // Is in kob[0] area.
                                plug->hostcall(&plug->plughead, 43, plug->knob_selected, 0, 0, 0); // Tell host we grabed the knob // audioMasterBeginEdit has op-code 43
                        }
                }
                if(plug->nedtryck){ // Change pressed knob according to relative mouse movement.
                        float temp = plug->pth.knob[plug->knob_selected] + (float)(plug->ned_y - plug->mywin.mouse.y) * 0.01; 
                        if(0 > temp)            plug->pth.knob[plug->knob_selected] = 0; // knob can't go bellow 0.
                        else if(1 < temp)       plug->pth.knob[plug->knob_selected] = 1; // knob can't go above 1.
                        else                    plug->pth.knob[plug->knob_selected] = temp ; // New knob value.
                        
                        plug->hostcall(&plug->plughead, 0,   plug->knob_selected, 0, 0, plug->pth.knob[plug->knob_selected]); //audioMasterAutomate has op-code 0
                }
                // values for recognicing changes in mousemovements and mouse buttons.
                plug->old = plug->mywin.mouse.buttons;  // old value for buttons.
                plug->ned_x = plug->mywin.mouse.x ;     // old value for x coordinate.
                plug->ned_y = plug->mywin.mouse.y ;     // old value for y coodrinate.
                if(plug->nedtryck && (plug->mywin.mouse.buttons == 0)){ // Release of mouse button
                        plug->nedtryck = 0;
                        plug->hostcall(&plug->plughead, 44,   plug->knob_selected, 0, 0, 0); // Tell host we ungrabed the knob // audioMasterEndEdit has op-code 44
                }
                for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++ ){
                        plug->knober.map[i] = (char)(plug->pth.knob[i] * 64) +31; // Select animation frame for knob value.
                }
                plug->knober.renderer->bg_color = 0x004466FF;
                spg_blit(&plug->mywin.frame,&bg, 0, 0); // Paint background.
                chardis_draw(&plug->knober,0,10,10);
                spg_update_window(&plug->mywin);
        break;
        case plugEditOpen:{
            spg_bmp_include(&knob_anim,knob_array);				// Load knob graphics.
	    spg_bmp_include(&bg,bg_array);					// Load background graphics. 
            spg_fill_bg(&knob_anim,(unsigned int)0x4466FF);			// set background color (rbg color) for knob array.

            spg_open_plugin_window(&plug->mywin,ptr,350,90);			// Open a editor window in host
            chardis_init(&plug->knober, &plug->mywin.frame,&knob_anim,5,1);	// Set number of knobs in the tile array
            chardis_tile_size(&plug->knober,64,56); 				// Set tile size of knob animation

            plug->window_open = 1 ; // We do it last, so we don't try to draw anything before it's ready for drawing.
            return  1;//true;
        }
        break;
	case plugGetPlugCategory:	return TYPE_OF_PLUG; // Return 1 if the plug is an effect, or 2 if it's a synthesizer.
        case plugEditClose:             plug->window_open = 0;				return true;   // Close plug edit window, not the plug instance.
        case plugGetProductString:      strcpy((char*)ptr, product_name);		return true;   // The name of the plug
        case plugGetVendorString:       strcpy((char*)ptr, brand_name);			return true;   // request for the vendor string (usually used in the UI for plugin grouping)
        case plugSetSampleRate:         samplerate = opt;				return true;   // Host tells plug the samplerate
	case plugSetProgram:		if(plug->program_no == value){ return true;} for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++){ plug->program_no = value; setknob(plug,i,presets[plug->program_no].param[i]); } break;
	case plugGetProgram:		return plug->program_no;			// Return current preset program number 
	case plugGetProgramNameIndexed:	getProgramName(index,(char*)ptr);		return true;	// If the category value is -1 all presets are enumerated linearily. Categorys starts at index 0.
	case plugGetProgramName:	getProgramName(plug->program_no,(char*)ptr);	return true;	// Alternative to plugGetProgramNameIndexed as some hosts use this instead.	
        case plugGetParamName:          getParameterName(index, (char*)ptr);            return true;	// Host whant the plug to transfer the indexed parameter's name. 
        case plugGetParamText:          getParameterText(plug, index, (char*)ptr);	return true;	// 
        case plugGetVstVersion:         return 2400; // kVstVersion;					// This plugin follows the VST2.4 ABI.
        case plugCanBeAutomated:        return true; // Return true if if the parameter is automatable. The index variable holds the parameter that the hosts asks about. In this example all parameters is automatable.
        case plugOpen:	   		for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++){ setknob(plug,i,presets[plug->program_no].param[i]); } return true; // Load preset 0. OP-Code is sent after the plug starts.
        case plugClose:			plug->instance_destroyed = 1; free(plug);	return true; // This op-code is sent by host before the plug gets deallocated from the system. To free resources and so on.
        case plugCanDo:			return -1; // Respond: -1 to everything not supported. // example of checking text... if (0 == strcmp(((char*)ptr),"receiveVstEvents"))   return 1;  //receiveVstEvents
        case plugGetChunk:		{ *(void**)ptr = &plug->pth;  return sizeof(struct patch); break; } // Host saves the plug state inside the host.
        case plugSetChunk:		memcpy(&plug->pth, (unsigned char *)ptr, sizeof(struct patch)); return true; // Host loads an old saved state into plug.
        default: break; // ignoring all other opcodes //fprintf(fp, "index:%d value:%ld ptr:%p opt:%f instance:%d \n",index,value,ptr,opt,instances); // For printing op-codes not implemented.
    }
    return 0; // Return not supported message number.
}
// These functions is given to the host in the main() function bellow. The host can't call these without the function pointer to them.
void  plugSetParameter(plugHeader *vstPlugin, int32_t index, float parameter){ plug_instance *plug = (plug_instance*)vstPlugin->object; plug->pth.knob[index] = parameter ; }// Host uses this to set parameter in plug
float plugGetParameter(plugHeader *vstPlugin, int32_t index){ plug_instance *plug = (plug_instance*)vstPlugin->object; return (float)(plug->pth.knob[index]); }// Host uses this to get parameter from plug

void* main(hostCallback vstHostCallback){ // New plug instances is created here. After plug load-in, it's the only known function for the host to run, that will give addresses for more functions for the host to run.
    struct plugHeader myplugin = {                              // Declaration of the struct that a plugin must return to the host.
        .magicNumber =                                          1450406992, // Identifier that it's an audio plug
        .plugInstuctionDecoderFunc =                            plugInstructionDecoder,  // A callback adress for Host to send opcodes to plug
        .plugSetParameterFunc =                                 plugSetParameter, // Set new value of automatable parameter.
        .plugGetParameterFunc =                                 plugGetParameter, // Returns current value of automatable parameter.
        .number_of_programs =                                   NUMBER_OF_PRESETS, // presets
        .number_of_params =                                     NUMBER_OF_PARAMETERS, // parameters
        .number_of_inputs =                                     2, // audio inputs
        .number_of_outputs =                                    2, // audio outputs
        .flags = hasReplacing | hasStateChunk | hasEditor,      // Bitflags for things this plugin supports
        .version = VERSION_NUMBER_OF_THIS_SPECICIC_PLUG,        // plug-in version, not VST version.
        .plugProcessFloatFunc = plugProcessSamplesFloat32,      // Name of the function where the audio processing is done.
        .plugProcessDoubleFunc = NULL,                          // No such function in this plug. Use for funtionpointer to 64bit floating point handeling
    };
    plug_instance *plug =(plug_instance*)calloc(1,sizeof(plug_instance));       // Allocate memory to the plug instance. calloc sets all allocated memory to zero unlike malloc
    memcpy(&(plug->plughead),&myplugin,sizeof(plugHeader));                     // Copy the above header over to the memory reserved with calloc. Destimation address is &(plug->plughead)
    instance[instances] = plug;                                                 // Copy the pointer for the instance to the instance[] array, so we can find data in all instances.
    plug->plughead.object = plug;                                               // To be able to find all internal parameters.
    plug->instance_no = instances;                                              // We put the number here, what no?. Not strictly necessary.
    instances++;                                                                // How many instances of this plug, that has been started totally by the host.
    plug->hostcall = vstHostCallback;						// Store callback to host, that was/is given by the host when this function is called.
    return plug; // Same as... return &(plug->plughead); // Send address to the plugs plug-header to the host. It's the address to all data that is uniqe for the plug instance.
}
