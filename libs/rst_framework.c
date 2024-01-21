// This is an example of an audio effect unit, using the version 2.4 plugin ABI (ABI Application Binary Interface).

#if NUMBER_OF_PARAMETERS != 0
void getProgramName(int32_t index,  char* ptr){	strcpy(ptr,presets[index].preset_name); }; // Copy the preset name and give to the host when asked for.
#endif
void setknob(plug_instance* plug,int knob,float value){
	plug->pth.knob[knob] = value ;
	plug->hostcall(&plug->plughead, 0, knob, 0, 0, plug->pth.knob[knob]); // op-code 0
}
 
// Function is called by the host to make your plug do and answer different things that the DAW asks for.
plugPtr plugInstructionDecoder(plugHeader *plugin, int32_t opCode, int32_t index, plugPtr value, void *ptr, float opt){ // Pointer to this function is used in the myplugin header
	plug_instance *plug = (plug_instance*)plugin->object;
	switch(opCode){
		case plugSupports:
		    if (0 == strcmp(((char*)ptr),"RST-noteID"))		return 0;  //change to 1 if you support it. Future functionallity to support per not modulation.
		    if (0 == strcmp(((char*)ptr),"bypass"))		return 0;  //change to 1 if you support it.	
		    if (0 == strcmp(((char*)ptr),"MPE"))		return 0;  //change to 1 if you support it.
#ifndef NO_MIDI
		    if (0 == strcmp(((char*)ptr+10),"Events"))		return 1;  //It can handle some type of events - Master for turning on/off other events if not returning 1.
		    if (0 == strcmp(((char*)ptr+10),"MidiEvent"))	return 1;  //MIDI-IN
		    if (0 == strcmp(((char*)ptr+10),"TimeInfo"))	return 0;  //change to 1 if you support it.
	#ifdef MIDI_OUT
		    if (0 == strcmp(((char*)ptr+7),"MidiEvent"))	return 1;  //MIDI-OUT
	#endif
#endif
		return -1; // Respond: -1 to everything not supported.
#ifndef NO_MIDI
		case plugProcessMIDI:		return MIDI_in(plug,(struct plugEvents*)ptr);   		break;		// Plug has recived a MIDI event
#endif
#ifndef NO_GUI
		case plugEditRedraw: 
			mouse_handling(plug); // update parameters with mouse.                  
			draw_graphics(plug);  // Draw all graphics,
			ikigui_update_window(&plug->dat.mywin); // Update all graphics in the plugin editor
		break;
		case plugEditorOpen:{
			prepare_graphics(plug,ptr); // Allocate everything needed for the new editor window.
			ikigui_open_plugin_window(&plug->dat.mywin,ptr,PLUG_WIDTH,PLUG_HEIGHT);	// Open the editor window in host.
			return  1;//true;
		}
		break;
		case plugEditorClose:		destroy_graphics(plug,ptr);			return 1;	// Close plug edit window, not the plug instance.
		case plugEditorSize: plug->myrect.bottom = PLUG_HEIGHT; plug->myrect.right = PLUG_WIDTH; *(struct ERect**)ptr = &plug->myrect ; return 1; // Host asks about the editor size.
#endif
#if NUMBER_OF_PARAMETERS != 0
		case plugSetProgram:		if(plug->program_no == value){ return 1;} for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++){ plug->program_no = value; setknob(plug,i,presets[plug->program_no].param[i]); } break;
		case plugGetProgram:		return plug->program_no;			// Return current preset program number
		case plugGetProgramNameIndexed:	getProgramName(index,(char*)ptr);		return 1;	// If the category value is -1 all presets are enumerated linearily. Categorys starts at index 0.
		case plugGetProgramName:	getProgramName(plug->program_no,(char*)ptr);	return 1;	// Alternative to plugGetProgramNameIndexed as some hosts use this instead.	
		case plugGetParamName:          getParameterName(index, (char*)ptr);            return 1;	// Host want the plug to transfer the indexed parameter's name. 
		case plugGetParamText:          getParameterText(plug, index, (char*)ptr);	return 1;	// 
#endif
		case plugGetPlugCategory:	return TYPE_OF_PLUG;						// Return 1 if the plug is an effect, or 2 if it's a synthesizer.
		case plugGetProductString:      strcpy((char*)ptr, product_name);		return 1;	// The name of the plug
		case plugGetVendorString:       strcpy((char*)ptr, brand_name);			return 1;	// request for the vendor string (usually used in the UI for plugin grouping)
		case plugSetSampleRate:         plug->samplerate = opt;	set_samplerate(plug); 	return 1;	// Host tells plug the samplerate

		case plugGetVersion:		return 2400;					// This plugin follows the 2.4 ABI.
		case plugCanBeAutomated:        return 1;			// Return true/1 if if the parameter is automatable. Here is all parameters is automatable. The index variable holds the parameter that the hosts asks about. 
		case plugOpen:	   		for(int i = 0 ; i < NUMBER_OF_PARAMETERS ; i++){
							#if NUMBER_OF_PARAMETERS != 0
							setknob(plug,i,presets[plug->program_no].param[i]); 
							#endif
						} audioplugOpen(plugin); 					return 1; // Load preset 0. OP-Code is sent after the plug starts.
		case plugClose:			audioplugClose(plugin); free(plug);				return 1;	 // This op-code is sent by host before the plug gets deallocated from the system. To free resources and so on.
		case plugGetState:		{ *(void**)ptr = &plug->pth;  return sizeof(struct patch);	break; } 	// Host saves the plug state inside the host.
		case plugSetState:		memcpy(&plug->pth, (unsigned char *)ptr, sizeof(struct patch)); return 1;	// Host loads an old saved state into plug.
		default: break; // ignoring all other opcodes //fprintf(fp, "index:%d value:%ld ptr:%p opt:%f instance:%d \n",index,value,ptr,opt,instances); // For printing op-codes not implemented.
	}
	return 0; // Return that it's not a supported message number by this plug.
}
// These functions is given to the host in the main() function bellow. The host can't call these without the function pointer to them.
void  plugSetParameter(plugHeader *plugin, int32_t index, float parameter){ plug_instance *plug = (plug_instance*)plugin->object; plug->pth.knob[index] = parameter ; }// Host uses this to set parameter in plug
float plugGetParameter(plugHeader *plugin, int32_t index){ plug_instance *plug = (plug_instance*)plugin->object; return (float)(plug->pth.knob[index]); }// Host uses this to get parameter from plug

void* main(hostCallback HostCallback){ // New plug instances is created here. After plug load-in, it's the only known function for the host to run, that will give addresses for more functions for the host to run.
	struct plugHeader myplugin = { // Declaration of the struct that a plugin must return to the host. And then some data to fill in important information.
		.magicNumber =                                          1450406992, // Identifier that it's an audio plug
		.plugInstructionDecoderFunc =                           plugInstructionDecoder,  // A callback address for Host to send opcodes to plug
		.plugSetParameterFunc =                                 plugSetParameter, // Set new value of automatable parameter.
		.plugGetParameterFunc =                                 plugGetParameter, // Returns current value of automatable parameter.
		.number_of_programs =                                   NUMBER_OF_PRESETS, // presets
		.number_of_params =                                     NUMBER_OF_PARAMETERS, // parameters
		.number_of_inputs =                                     0, // audio inputs
		.number_of_outputs =                                    2, // audio outputs
		.flags = hasFloatAudio | hasSaveState,      		// Bitflags for things this plugin supports
		.version = VERSION_NUMBER_OF_THIS_SPECIFIC_PLUG,        // plug-in version, not ABI version.
		.plugProcessFloatFunc = audio_in_out_float,      	// Name of the function where the audio processing is done.
		.plugProcessDoubleFunc = NULL,                          // No such function in this plug. Useed for double 64bit floating point audio handeling.
	};
	#if TYPE_OF_PLUG == EFFECT_UNIT
		myplugin.number_of_inputs = 2; // audio inputs
	#endif
	#ifndef NO_GUI
		myplugin.flags |= hasEditor;
	#endif
	plug_instance *plug =(plug_instance*)calloc(1,sizeof(plug_instance));	// Allocate memory to the plug instance. calloc sets all allocated memory to zero unlike malloc
	memcpy(&(plug->plughead),&myplugin,sizeof(plugHeader));			// Copy/clone/'constructor for' the above header over to the memory reserved with calloc. Destination address is &(plug->plughead)
	plug->plughead.object = plug;						// To be able to find all internal parameters.
	plug->hostcall = HostCallback;						// Store callback to host, so the plug can make calls to the host.
	return plug; // Same as... return &(plug->plughead); // Send address to the plugs plug-header to the host. It's the address to all data that is uniqe for the plug instance.
}
