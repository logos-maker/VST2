// include the X library headers
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include  <unistd.h>
#include  <signal.h>

typedef struct { 
        int x;
        int y;
        int w;
        int h;
} spg_rect ;

typedef struct { 
	union { int w, width; };
	union { int h, height; };
	unsigned int *pixels;
        unsigned int size;
        unsigned int bg_color; // for filling background
} spg_frame; // frame = {0};

enum { MOUSE_LEFT = 0b1, MOUSE_MIDDLE = 0b10, MOUSE_RIGHT = 0b100, MOUSE_X1 = 0b1000, MOUSE_X2 = 0b10000 };

struct mouse{
	int x, y, x_rel, y_rel;
	unsigned char buttons;
} ;

typedef struct {
        // here are our X variables
        Display *dis;           // X Server connection
        int screen;             // X11 Screen
        Window win;             // the X11 window       
        GC gc;                  // X11 Graphic Context
	XEvent event;		// the XEvent declaration !!! */
        Atom wm_delete_window;  // the window close event

        // to handle keypress events
	KeySym key;		/* a dealie-bob to handle KeyPress Events */	
	char text[255];		/* a char buffer for KeyPress Events */

        // to handle mouse events
        struct mouse mouse;

        // To have pixels
	XImage* image;
	Pixmap bitmap;
        spg_frame frame;
} spg;                     

#define MAX_INSTANCES 10 

spg mywin; // innan jag gjorde det till en array
int old_x;
int old_y;

void spg_blit_part(spg_frame *mywin,spg_frame *frame, int x, int y, spg_rect *part){ // Draw area
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w <= (x+part->w))return; // shelding crash
        if(mywin->h <= (y+part->h))return; // shelding crash
        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        mywin->pixels[x+i+(j+y)*mywin->w] = frame->pixels[i+part->x+frame->w*(j+part->y)];
                }
        }
}

void spg_blit_part_filled(spg_frame *mywin,spg_frame *frame, int x, int y, spg_rect *part){ // Draw area
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w <= (x+part->w))return; // shielding crash
        if(mywin->h <= (y+part->h))return; // shielding crash

        unsigned int color = mywin->bg_color;

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        unsigned int temp = frame->pixels[i+part->x+frame->w*(j+part->y)];

                        // Fixed point math, probably faster than floats
                        unsigned char alpha = temp >> 24; // Alpha channel
                        unsigned char alpha_inv = ~alpha;
                        unsigned char rf =  (temp&0xff0000)>>16; // Red forground
                        unsigned char gf =  (temp&0xff00)>>8;    // Green forground
                        unsigned char bf = temp&0xff;         // Blue forground
                        unsigned char rb = (color&0xff0000)>>16; // Red beckground
                        unsigned char gb = (color&0xff00)>>8;    // Red background
                        unsigned char bb = color&0xff;           // Blue background                
                        unsigned char ro = (alpha_inv*rb + alpha*rf)>>8;   // background + forground
                        unsigned char go = (alpha_inv*gb + alpha*gf)>>8;   // background + forground
                        unsigned char bo = (alpha_inv*bb + alpha*bf)>>8;   // background + forground

                        mywin->pixels[x+i+(j+y)*mywin->w] = (unsigned int)((ro << 16) + (go<< 8) + bo);
                }
        }
}

void spg_blit(spg_frame *mywin,spg_frame *frame, int x, int y){ // Draw area. Flexible to Blit in windows and pixel buffers. Can be optimized greatly.
        for(int j = 0 ; j < frame->h ; j++){ // vertical
                for(int i = 0 ; i < frame->w ; i++){   // horizontal         
                        mywin->pixels[x+i+(j+y)*mywin->w] = frame->pixels[i+frame->w*j];
                }
        }
}

void spg_fill_bg(spg_frame *frame,unsigned int color){// A background color for automatic filling of transparent pixels.
        for(int i = 0 ; i < frame->size ; i++){
                unsigned int temp = frame->pixels[i];

                float alpha = ((float)(temp >> 24)) / 255; // Alpha channel
                float rf =  (float)((unsigned int)(temp&0xff0000)>>16); // Red forground
                float gf =  (float)((unsigned int)(temp&0xff00)>>8);    // Green forground
                float bf =  (float)((unsigned int)(temp&0xff));         // Blue forground
                float rb = (float)((unsigned int)(color&0xff0000)>>16); // Red beckground
                float gb = (float)((unsigned int)(color&0xff00)>>8);    // Red background
                float bb = (float)((unsigned int)color&0xff);           // Blue background                
                unsigned char ro = (char)((1.0-alpha)*rb + alpha*rf);   // background + forground
                unsigned char go = (char)((1.0-alpha)*gb + alpha*gf);   // background + forground
                unsigned char bo = (char)((1.0-alpha)*bb + alpha*bf);   // background + forground

                frame->pixels[i] = (unsigned int)((ro << 16) + (go<< 8) + bo);
        }
}

void spg_bmp_include(spg_frame *frame,const unsigned char* bmp_incl){
        unsigned int start;
        frame->w = bmp_incl[0x12] + (bmp_incl[0x12+1]<<8) + (bmp_incl[0x12+2]<<16) + (bmp_incl[0x12+3]<<24);
        frame->h = bmp_incl[0x16] + (bmp_incl[0x16+1]<<8) + (bmp_incl[0x16+2]<<16) + (bmp_incl[0x16+3]<<24);
        start =    bmp_incl[0x0a] + (bmp_incl[0x0a+1]<<8) + (bmp_incl[0x0a+2]<<16) + (bmp_incl[0x0a+3]<<24);

        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4); // allokera minne för pixlarna i bilden

        int counter = 0 ; 
        for(int j = frame->h -1 ; j >= 0 ; j--){ // läs ut alla rader i bilden 
                for(int i = 0 ; i < frame->w ; i++){// Läs ut en rad av pixlar och visa i fönster
                        int pixl_addr = (i+(j*frame->w))*4+start;
                        frame->pixels[counter++]= bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24);
                        //frame->pixels[i+(j*frame->w)]= bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24); // Grafiken blir upp och nedvänd
                }
        }
        frame->size = frame->w * frame->h ;
}

void spg_open_plugin_window(spg *mywin,void *ptr,int w, int h){
        mywin->frame.w = w;
        mywin->frame.h = h;
	mywin->dis=XOpenDisplay((char *)0);     // Get the display
   	mywin->screen=DefaultScreen(mywin->dis); // Get the screen
        int x = 0;//(XDisplayWidth(mywin->dis, mywin->screen) >>2) -(w >>1); 
        int y = 0; //(XDisplayHeight(mywin->dis, mywin->screen)>>2) -(h >>1);
	unsigned long black;
	black=BlackPixel(mywin->dis,mywin->screen),// get the color black

        mywin->win=XCreateSimpleWindow(mywin->dis,DefaultRootWindow(mywin->dis),x,y,w, h, 5,black, black); // Create window

        // För att få event av att stänga fönster
        mywin->wm_delete_window = XInternAtom(mywin->dis, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(mywin->dis, mywin->win, &mywin->wm_delete_window, 1);

        // Event typer att ta emot
	XSelectInput(mywin->dis, mywin->win, ExposureMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|FocusChangeMask|EnterWindowMask|LeaveWindowMask); // Select event types to get

        mywin->gc=DefaultGC(mywin->dis,mywin->screen); 

	XMapRaised(mywin->dis, mywin->win);

        XWindowAttributes wa = {0};
        XGetWindowAttributes(mywin->dis, mywin->win, &wa);

	mywin->frame.pixels = (unsigned int *)malloc(w * h * 4);
	mywin->bitmap = XCreatePixmap(mywin->dis, mywin->win, w, h, 1);
	mywin->image = XCreateImage(mywin->dis, wa.visual, wa.depth, ZPixmap, 0, (char*)mywin->frame.pixels, w, h, 32, w * 4);

        XReparentWindow(mywin->dis, mywin->win,(Window)ptr, 0, 0);
        XFlush(mywin->dis);
}

void spg_close_window(spg *mywin){ // Denna kallas på av spg_get_events()
	XDestroyWindow(mywin->dis,mywin->win);
	XCloseDisplay(mywin->dis);				
};

void spg_update_window(spg *mywin){ // Denna kallas på av spg_get_events()
        XPutImage(mywin->dis, mywin->win, mywin->gc, mywin->image, 0, 0, 0, 0, mywin->frame.w, mywin->frame.h);
};

void spg_get_events(spg *mywin){ // ingen spg function kallar på denna funktionen
        while( XPending(mywin->dis) > 0 ){ // no of events in que
                XNextEvent(mywin->dis, &mywin->event); // Get next event

                if (mywin->event.type== ClientMessage){ // User Closes window
                        if ((Atom) mywin->event.xclient.data.l[0] == mywin->wm_delete_window) {
                            		spg_close_window(mywin);
	                                exit(1);
                        }
                }
	        if (mywin->event.type==Expose && mywin->event.xexpose.count==0) spg_update_window(mywin); // Window graphics needs a redraw
                if (mywin->event.type==KeyPress&& XLookupString(&mywin->event.xkey,mywin->text,255,&mywin->key,0)==1) { // the XLookupString routine to converts the KeyPress event data into regular text.
		        
	        }

                switch(mywin->event.type){
                        case MotionNotify:{
                                mywin->mouse.x_rel =  mywin->event.xmotion.x -old_x ;
                                mywin->mouse.y_rel =  mywin->event.xmotion.y -old_y ;
                            
                                old_x = mywin->mouse.x = mywin->event.xmotion.x ;
                                old_y = mywin->mouse.y = mywin->event.xmotion.y ;
                        }
                        break;
                        case ButtonPress:  
                                switch(mywin->event.xbutton.button){
                                        case 1: mywin->mouse.buttons |=  MOUSE_LEFT;   break; // printf("Left\n"); break;
                                        case 2: mywin->mouse.buttons |=  MOUSE_MIDDLE; break; // printf("Middle\n");break;
                                        case 3: mywin->mouse.buttons |=  MOUSE_RIGHT;  break; // printf("Right\n");break;
                                        //default: printf("???%d",mywin.event.xbutton.button);break;
                                }
                        break;
                        case ButtonRelease: 
                                switch(mywin->event.xbutton.button){
                                        case 1: mywin->mouse.buttons &= ~MOUSE_LEFT;   break; // printf("Left\n"); break;
                                        case 2: mywin->mouse.buttons &= ~MOUSE_MIDDLE; break; // printf("Middle\n");break;
                                        case 3: mywin->mouse.buttons &= ~MOUSE_RIGHT;  break; // printf("Right\n");break;
                                        //default: printf("???%d",mywin.event.xbutton.button);break;
                                }
                        break;
                }
        } // while loop end
};
