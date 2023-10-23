#ifdef __x86_64
	typedef int64_t plugPtr;
#else
	typedef int32_t plugPtr;
#endif

//typedef int64_t plugPtr;
typedef struct plugHeader plugHeader;
struct plugHeader{ // The ABI for audio effect plugins
	int32_t magicNumber;
	plugPtr (*plugInstuctionDecoderFunc)(plugHeader* effect, int32_t opcode, int32_t index, plugPtr value, void* ptr, float opt);
	void(*deprecatedProcess)(void); // not used, is deprecated
        void  (*plugSetParameterFunc)(plugHeader* effect, int32_t index, float parameter);
        float (*plugGetParameterFunc)(plugHeader* effect, int32_t index);
	int32_t number_of_programs;     // number of programs in plug
	int32_t number_of_params;	// all programs are assumed to have numParams parameters
	int32_t number_of_inputs;	// number of audio inputs in plug
	int32_t number_of_outputs;	// number of audio outputs in plug
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
typedef plugPtr (*hostCallback) (plugHeader* effect, int32_t opcode, int32_t index, plugPtr value, void* ptr, float opt); 
enum { LEFT, RIGHT }; // 0 for left audio channel, 1 for right audio channel.
enum { UNKNOWN, EFFECT_UNIT, SYNTHESIZER}; // 0 for unknown, 1 for effect unit, 2 for synthesizer.
enum plugPropertiesFlags{
        hasEditor          = 1,   // Plug has an editor window
	hasReplacing       = 16,  // Plug has function for writing audio out over the audio in buffer in 32bit floating point (float).
	hasStateChunk      = 32,  // internal state in plug can be copied to host, for saving and loading audio projects in host.
	hasDoubleReplacing = 4096,// Plug has function for writing audio out over the audio in buffer in 64bit floating point (double).
	hasSynth	   = 256, // Plug is a synthesizer.
};
enum opcodes{
        plugEditOpen=14,        //Attention: Is called when the DAW whants the plug to open window.
        plugEditClose=15,       //Attention: After you answered this call, then the window is closed by DAW.
        plugEditGetRect=13,
        plugEditRedraw=19,
        plugOpen = 0,           //Host warns it will start to use this plug instance before it does.
        plugClose = 1,          //Host warns that it will end the usage of this plug instance.
        plugGetParamText=7,     //Host asks for a text to display for the value of a parameter.
        plugGetParamName=8,     //Host asks for the name of a parameter in plug.
        plugSetSampleRate=10,   //The host tells the plug what sample rate that is currently used.
        plugGetChunk=23,		//Host asks for an adress and the length of a data chunk that the host shall save for the plug.
        plugSetChunk=24,		//Host asks for an adress and length of the data segment that the host shall fill to restore an old state.
        plugCanBeAutomated =26,		//For the host to check if it can automate a certain parameter.
        plugGetVendorString=47,		//For the host to ask for the name of the author of the plug.
        plugGetProductString=48,	
        plugCanDo=51,			//For the host to ask if the plug supports a sertain functionallity, the name of the functionallity is supplied in a text string by the host.
        plugGetVersion=58,		//Asks the plug what version of the ABI that was designed for.
	plugGetProgramNameIndexed=29,	//Gets the preset name by index, Bitwig uses this.
	plugGetProgramName=5,		//Alternative to plugGetProgramNameIndexed.
	plugSetProgram=2,		//Change plug preset program
	plugGetProgram=3,		//Ask plug for program number
	plugGetPlugCategory=35		//Ask if it's a effect unit or a synthesizer. Other categories exists but is futile.
};
struct ERect{
	int16_t top;    // Set to zero
	int16_t left;   // Set to zero
	int16_t bottom; // Set to window width
	int16_t right;  // Set to window higth
};