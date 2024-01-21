//*********************************************************************************
//  Some generic stuff not specific to this plug, no need to change anything here
//*********************************************************************************

struct patch{ // You normaly don't need to change anything here, it's the patch that the DAW will save and restore when saving and loading audio projects.
	float knob[NUMBER_OF_PARAMETERS];
};

typedef struct{ // You normaly don't need to change anything here, It's a some generic declarations that gets allocated when the DAW cratest a new plug instance
	struct plugHeader plughead;	// It must be first in the struct. Note that each instance has this header.
	plugPtr (*hostcall) (plugHeader* effect, int32_t opcode, int32_t index, plugPtr value, void* ptr, float opt);
	struct patch pth;		// all data to save and restore by host be the op-codes plugGetChunk and plugSetChunk functions.
	struct data dat; 		// buffers and variables for the audio algorithm.
	struct ERect myrect;		// Window size and placement.
	int program_no; 		// the current preset number (not used in this plug).
	int knob_selected; 		// index value for knob changed by user in the plug GUI/Editor.
	float samplerate;		// Hold the current sample rate.
} plug_instance;

struct preset{ // You normaly don't need to change anything here, it's general struct used for internal presets in the plug
	char preset_name[24];
	float param[NUMBER_OF_PARAMETERS];
};