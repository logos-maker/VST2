// A simple C library for oldschool characters and tiles.
// See code examples for further understanding of usage.

typedef struct chardis {
   spg_frame   *renderer;   // The renderer to use for drawing the character display
   spg_frame    *texture;    // The texture with tiles for the monospace font
   unsigned char  columns;     // Number of columns in the character display
   unsigned char  rows;        // Number of rows in the character display
   unsigned char  char_width;  // The pixel width of the characters in the texture
   unsigned char  char_hight;  // The pixel hight of the characters in the texture
   char  *map;        // The start address for the character display
   unsigned char  scaling;     // 1 displays graphics as in BMP, 2 doubles pixels in x and Y. 
} chardis;                     // 3 triples, 4 quadruples, 0 halfs the pixels in x and Y (0 is not tested).

int chardis_init(struct chardis *display, spg_frame *renderer, spg_frame *texture, int columns, int rows ){
   display->map = calloc(columns*rows,sizeof(char));
   display->char_width = 8 ;    // Set to a default value
   display->char_hight = 8 ;    // Set to a default value
   display->columns = columns ; // Set to a default value
   display->rows = rows ;       // Set to a default value
   display->scaling = 1;        // Set to a default value
   display->renderer = renderer ; // Set to renderer given as input by library user.
   display->texture = texture ;   // Set to texture given as input by library user.
   return columns * rows ;
}

void chardis_tile_size(struct chardis *display,int width,int hight){ // can be used after chardis_init
   display->char_width = width ;    // Set to a default value
   display->char_hight = hight ;    // Set to a default value
}

void chardis_free(struct chardis *display){
   free(display->map); // Free memory that was alocated with chardis init.
}

int chardis_mouse_pos(struct chardis *display, int x, int y){ // returns -1 if outside the character display
   // x , y is pixel coordinate inside the drawing area
   if( (x < 0) || (y < 0) ) return -1;
   if( ( x < (display->char_width * display->scaling * display->columns))
   &&  ( y < (display->char_hight * display->scaling * display->rows))
   ) return( // Returns the index of the character in your character array. 
      (x / (display->char_width * display->scaling)) + 
      ((y / (display->char_hight * display->scaling)) * display->columns)
   );
   return -1; // Pixel coordinate is outside of the character display. 
}

void chardis_draw(struct chardis *display, char filling, int x, int y){  // x y is pixel coordinate to draw it in the window
   spg_rect srcrect;
   spg_rect dstrect;

   srcrect.y = 0; // Top pixel of characters in picture loaded in to texture.
   srcrect.w = display->char_width; // character width in loaded picture in texture.
   srcrect.h = display->char_hight; // character hight in loaded picture in texture.
   dstrect.w = display->char_width * display->scaling;  // character width on screen.
   dstrect.h = display->char_hight * display->scaling;  // character hight on screen.

   int w = display->columns ;
   int h = display->rows ;

   for(int i = 0 ; i < h ; i++ ){
      dstrect.y = i * dstrect.h + y;
      for(int j = 0 ; j < w ; j++ ){
          dstrect.x = (j * dstrect.w) + x;
          srcrect.x = srcrect.w * (display->map[i*w + j] - 32) ; // -32 becauce the ASCII code should match to where you catch characters from texture.
          if(srcrect.x <0)srcrect.x = 0; // Program can segment fault without this line, this line was not in the original library.

          // Draw the character buffer to window.
          if(filling)spg_blit_part_filled(display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);
          else spg_blit_part(display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);
      }
   }        
}

/*
=============================================================================================
Chardis library documentation:
=============================================================================================


Example for fast usage
======================
chardis statusbar; // struct for feild
chardis_init(&statusbar, renderer, texture,40,1 ); // Initialization for usage. 40 columns and 1 row
sprintf(statusbar.map,"text to display %d",var); // Write text to field an a variable to keep track of
chardis_draw(&statusbar,0,0,0); // Draw feild to screen buffer to show with SDL_RenderPresent(renderer);


Font specification
==================
As a "font" or tiles, you need to have a SDL_Texture with graphics.
The tiles needs to be equal in size, where the first is in the
left upper corner, with the tiles arranged horizontally in the texture.
To load a BMP file into a texture see code bellow... 


How to make bmp font to a header file and how to use it input in chardis_init()
===============================================================================
Load the bmp file into a hexeditor like for example Okteta, and export it to a c array header file.
#include "font.h" // the array definition in the header file is... const unsigned char thefont[131194];
SDL_Texture *texture;
SDL_RWops *rw = SDL_RWFromConstMem(fonten, sizeof(thefont)); // thefont is in this case the name of the array.
texture = SDL_CreateTextureFromSurface(plug->r, SDL_LoadBMP_RW(rw, 1));


Example code for getting started fast
=====================================

// Gives a 640 x 480 Window and displays mouse coordinates in window // what is program does
// gcc test.c -o test -lSDL2           // command to compile program
// ./test                              // command to run program

#include <SDL2/SDL.h>
#include "chardis_v7.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Event event;

int x,y;

void main(){
   SDL_Init(SDL_INIT_VIDEO);

   if (SDL_CreateWindowAndRenderer(600,200 , SDL_WINDOW_RESIZABLE, &window, &renderer)) { // 640 x 480 pixels
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
       return;
   }
   if(!(texture = SDL_CreateTextureFromSurface(renderer,SDL_LoadBMP("./font.bmp")))){
     printf("%s\n",SDL_GetError());
     SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from BMP file: %s\n",SDL_GetError()); return;
   }

   chardis statusbar; chardis_init(&statusbar, renderer, texture, 80, 60 ); // init... status statusbar
   while (1) {
        SDL_WaitEvent(&event); // SDL_PollEvent(&event);

        if (event.type == SDL_QUIT) { // Is here as you cant exit the while loop with break inside the switch.
            break;
        }
        switch(event.type){
            case SDL_MOUSEBUTTONDOWN:{
               statusbar.map[ chardis_mouse_pos( &statusbar, event.motion.x, event.motion.y) ] = 'X';
               // SDL_Point p;
               // SDL_GetMouseState(&p.x, &p.y);
               // SDL_PointInRect(&p,(SDL_Rect*));
            }
            break;
            case SDL_KEYUP: 
               switch(event.key.keysym.sym){ // Fill in for keyboard usage. Names for keys is found here https://wiki.libsdl.org/SDL_Keycode
                  case SDLK_LEFT:  break; // only an example, remove if not needed.
                  case SDLK_RIGHT: break; // only an example, remove if not needed.
                  case SDLK_UP:    break; // only an example, remove if not needed.
                  case SDLK_DOWN:  break; // only an example, remove if not needed.
              }
            break;
            case SDL_MOUSEMOTION: {
               x = event.motion.x ;
               y = event.motion.y ;
            }
            break;
        }

        SDL_SetRenderDrawColor(renderer, 0x70, 0x70, 0x70, 0x00); // Set background color
        SDL_RenderClear(renderer);                                // Clean background

        // Statusbar
        sprintf(statusbar.map,"STATUS: %4d %4d",x,y);           // Statusbar apperance
        chardis_draw(&statusbar,0,0,0); // Draw statubar on top

        SDL_RenderPresent(renderer);         // Show everything in window.
   }
}


Chardis - A library for a classic character display with SDL2
=============================================================
Old computers had a memory area that coresponded to the characters and 
tiles drawn on the monitor screen. That gave a monospace font that could
easily be used for making GUI's or displaying game graphics with custom tiles.
Your tiles can use as many colors you want and has an alpha channel for transparancy.



Character display functions
===========================
To draw the character display you use the following function...
void chardis_draw(struct chardis *display, char filling, int x, int y);  // X,Y Position to draw in SDL2 window. filling 0 for unfilled and 1 for filled.



Availible helper functions
--------------------------
int  chardis_mouse_pos(struct chardis *display, int x, int y); // returns address of clicked char, or -1 if outside the character display
int chardis_init(struct chardis *display, SDL_Renderer *renderer, SDL_Texture *texture,80,60 ); // Setting the struct to some default values. 80 columns, 60 rows.



Why chardis was made
====================
I found myself writing new code for character displays when using SDL2 over and over.
And recogniced that I didn't reuse code becuse the code got nested with uniqe things
that i did. So made this library for for fast and easy reuse in my future programs.
And kept it intentionally simple, to be able to make programs even faster and simpler.

When you in the middle of making a program and you recogices that it would be handy
with a character display, then it's often a burdon to write your own and retrofit 
into your project. But this library makes this easier.



Uses for chardis
================
* For debugging purposes, when terminal messages isn't the right choice.
* An alternative to using the terminal window for user input and output.
* For nostalgia, making programming more like how it was in the 80's.
* Layering graphics in different colors using many character displays.
* For making dropdown menu's and status bars.
* Easy colaboration, as it uses BMP pictures for loading tiles and characters.


*/

