// for oldschool monospace characters and graphic tiles.

#ifdef __linux__ //linux specific code goes here...
	#include "ikigui_lin.h"	// For window and graphics handling in this case for Linux.
#elif _WIN32 // windows specific code goes here...
	#include "windows.h"
	#include "ikigui_win.h" // For window and graphics handling in this case for Windows.
#endif

typedef struct ikigui_map {
   ikigui_image   *renderer;   // The renderer to use for drawing the character display		<- destination
   ikigui_image   *texture;    // The texture with tiles for the monospace font			<- source
   unsigned char  columns;     // Number of columns in the character display
   unsigned char  rows;        // Number of rows in the character display
   unsigned char  char_width;  // The pixel width of the characters in the texture
   unsigned char  char_hight;  // The pixel hight of the characters in the texture
   char  *map;		       // The start address for the character display
   unsigned char  scaling;     // 1 displays graphics as in BMP, 2 doubles pixels in x and Y.
} ikigui_map;

int ikigui_map_init(struct ikigui_map *display, ikigui_image *renderer, ikigui_image *texture, int columns, int rows,int width,int hight ){
   display->map = (char*)calloc(columns*rows,sizeof(char));
   display->char_width = width ;// Set to input given as input by library user.
   display->char_hight = hight ;// Set to input given as input by library user.
   display->columns = columns ; // Set to a default value
   display->rows = rows ;       // Set to a default value
   display->scaling = 1;        // Set to a default value
   display->renderer = renderer ; // Set to renderer given as input by library user.
   display->texture = texture ;   // Set to texture given as input by library user.
   return columns * rows ;
}

void ikigui_map_free(struct ikigui_map *display){
   free(display->map); // Free memory that was alocated with ikigui init.
}

int ikigui_mouse_pos(struct ikigui_map *display, int x, int y){ // returns -1 if outside the character display
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

void ikigui_map_draw(struct ikigui_map *display, char filling, int x, int y){  // x y is pixel coordinate to draw it in the window
   ikigui_rect srcrect;
   ikigui_rect dstrect;

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
          if(srcrect.x <0)srcrect.x = 0; // Program can segment fault without this line.
         
	  switch(filling){ // Draw the character buffer to window.
		  case 0:       ikigui_blit_alpha (display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);	break;
		  case 1:	ikigui_blit_filled(display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);	break;
		  case 2:	ikigui_blit_fast  (display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);	break;
	  }
      }
   }        
}
