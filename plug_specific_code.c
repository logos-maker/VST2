// This is not open source, and can't be used in open source software <- Is the way to go? 
// In this C file, you make the changes specific to your plug (that is not related to the plugin GUI).
char brand_name[]   = "DSC";       // Place your brand name inside ""
char product_name[] = "THELAY"; // Place your plug name inside ""
#define VERSION_NUMBER_OF_THIS_SPECICIC_PLUG 1  // Version number for this plug is set to 1. Increase number for your plug when you make improvements.
#define NUMBER_OF_PARAMETERS 5                  // Number of parameters in plug. Change that to the number of parameters you need, but don't excede 128.
#define NUMBER_OF_PRESETS 3			// Number of presets inside the plug.
#define TYPE_OF_PLUG EFFECT_UNIT // Set this to EFFECT_UNIT or SYNTHESIZER
struct preset presets[NUMBER_OF_PRESETS] = {	// the preset presets inside the plug. Change NUMBER_OF_PRESETS if changing the number of presets.
	{"SHORT",  0.1,0.9,0.1,0.2,0.3},// First preset
	{"LONG",   0.7,0.7,0.7,0.1,0.2},// Second preset
	{"BYPASS", 0.7,0.7,0.7,0.1,0.0},// Third preset
};
void getParameterName(int32_t index,  char* ptr){ // Names of all user parameters in the plug. Max 7 characters for each name(8 if the implicit \n is included), but the spec is futile.
        switch(index){ // Copy the name of the the right paramter to be displayed in the host.
                case  0: strcpy(ptr, "DELAY");          break; // Name of the first  parameter is between ""
                case  1: strcpy(ptr, "CUTOFF");         break; // Name of the second parameter is between ""
                case  2: strcpy(ptr, "OVERDRIVE");	break; // Is "to long" but works. Hosts allows longer names for compatibiliy reasons.
                case  3: strcpy(ptr, "FEEDBACK");       break; // ...as almost no plugins follows this limit.
                case  4: strcpy(ptr, "DRY/WET");        break;
                default: strcpy(ptr, "???"); break; // A default name, reminding to add create any missing case for some parameter.
        }
};
void getParameterText(plug_instance *plug,int32_t index,char* ptr){ if(NULL!=gcvt(plug->pth.knob[index], 6, ptr)) return; }; // Host whant the plug indexed parameter value in text. if(NULL is for not giving compilation warnings.

void filter(plug_instance *plug,float filter_out[2],float filter_in[2],float cutoff,int poles){ // Filter function used by audio algorithm in the audio buffer.
                #define POLES 4
                for(int i = 0 ; i < poles ; i++){	// out              =     filter-in      -    out-of-filterstage  * cutoff      +    out-of-filterstage
						plug->filt_buff[LEFT ][i+1] = ((filter_in[LEFT ] - plug->filt_buff[LEFT ][i]) * cutoff) + plug->filt_buff[LEFT ][i];
						plug->filt_buff[RIGHT][i+1] = ((filter_in[RIGHT] - plug->filt_buff[RIGHT][i]) * cutoff) + plug->filt_buff[RIGHT][i];
                }
                plug->filt_buff[LEFT ][0] = filter_out[LEFT ] = plug->filt_buff[LEFT ][poles];
                plug->filt_buff[RIGHT][0] = filter_out[RIGHT] = plug->filt_buff[RIGHT][poles]; // Uträknad sampling ut ifrån sista filtersteget.
}
// Function is called by the host to make your plug process the audio buffer. Here the 'audio in' gets processed and sent to audio out. Audio levels is between -1 to +1
void plugProcessSamplesFloat32(plugHeader *vstPlugin, float **inputs, float **outputs, int32_t sampleFrames){ plug_instance *plug = (plug_instance*)vstPlugin->object;
	enum { PAR_DELAY,PAR_OVERDR,PAR_CUTOFF,PAR_FEEDBK,PAR_DRY}; // Create enumeration on words from 0 to 4 to get the right parameters.
        float cutoff = (plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]*plug->pth.knob[PAR_CUTOFF]); // log4 knob value
        for(int j = 0; j < sampleFrames; j++){ // Loop trough all the samples in buffer. Put your audio algorithm inside here....
		float filt_in[2], filt_out[2];

                filt_in[LEFT ] = inputs[LEFT ] [j] * (plug->pth.knob[PAR_OVERDR]*4);//if(filt_in[LEFT ]>1)filt_in[LEFT ]=1;else if(filt_in[LEFT ]<-1)filt_in[LEFT ]=-1;
                filt_in[RIGHT] = inputs[RIGHT] [j] * (plug->pth.knob[PAR_OVERDR]*4);//if(filt_in[RIGHT]>1)filt_in[RIGHT]=1;else if(filt_in[RIGHT]<-1)filt_in[RIGHT]=-1;
		if(filt_in[LEFT ]>1)filt_in[LEFT ]=1;else if(filt_in[LEFT ]<-1)filt_in[LEFT ]=-1;
		if(filt_in[RIGHT]>1)filt_in[RIGHT]=1;else if(filt_in[RIGHT]<-1)filt_in[RIGHT]=-1;

                filter(plug,filt_out,filt_in,cutoff,4);

                plug->delaybuffer[LEFT] [plug->delaytap] = ( plug->delaybuffer[LEFT] [plug->delaytap] * (plug->pth.knob[PAR_FEEDBK] ) ) +  ( filt_out[LEFT ] * (1-plug->pth.knob[PAR_FEEDBK]) ) ;
                plug->delaybuffer[RIGHT][plug->delaytap] = ( plug->delaybuffer[RIGHT][plug->delaytap] * (plug->pth.knob[PAR_FEEDBK] ) ) +  ( filt_out[RIGHT] * (1-plug->pth.knob[PAR_FEEDBK]) ) ;

                plug->delaytap++; if(plug->delaytap > (int)(plug->pth.knob[PAR_DELAY] *50000)) plug->delaytap = 0;

                outputs[LEFT][j]   = inputs[LEFT ][j]* (1-plug->pth.knob[PAR_DRY]) + plug->delaybuffer[LEFT ][plug->delaytap] * plug->pth.knob[PAR_DRY];
                outputs[RIGHT][j]  = inputs[RIGHT][j]* (1-plug->pth.knob[PAR_DRY]) + plug->delaybuffer[RIGHT][plug->delaytap] * plug->pth.knob[PAR_DRY];
        }
}
