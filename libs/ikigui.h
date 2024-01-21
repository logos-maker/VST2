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
   char		  *map;	       // The start address for the character display
   unsigned char  direction;   // The direction of the immages in the source image. Is autodetected by ikigui_map_init().
   uint16_t       max_index;   // The number of tiles in the map - 1.
   int		  x_spacing;   // The horizontal spacing between the left sides of the tiles in pixels.
   int		  y_spacing;   // The vertical spacing between the top sides of the tiles in pixels.
   signed char	  offset;      // The number offset for the numbers in the map. Can be used to fill it with ASCII text for example.
} ikigui_map;

int ikigui_map_init(struct ikigui_map *display, ikigui_image *renderer, ikigui_image *texture,  int8_t offset, int x_spacing, int y_spacing, int width, int hight, int columns, int rows ){
   display->map = (char*)calloc(columns*rows,sizeof(char));
   display->offset = offset ; // value offset to all values in the map array.
   display->char_width = width ;// Set to input given as input by library user.
   display->char_hight = hight ;// Set to input given as input by library user.
   display->x_spacing = x_spacing ;// Set to input given as input by library user.
   display->y_spacing = y_spacing ;// Set to input given as input by library user.
   if(y_spacing == 0) display->y_spacing = display->char_hight ; // if y_spacing is zero, set it to the size of the tile so they are placed side by side.
   if(x_spacing == 0) display->x_spacing = display->char_width;  // if x_spacing is zero, set it to the size of the tile so they are placed side by side.
   display->columns = columns ; // Set to a default value
   display->rows = rows ;       // Set to a default value
   display->max_index = (texture->w / width) * (texture->h / hight) - 1 ; //
   display->renderer = renderer ; // Set to renderer given as input by library user.
   display->texture = texture ;   // Set to texture given as input by library user.
   if(texture->w == width) display->direction = 0; else display->direction = 1; // Automatic detection for tile-atlas direction (horizontal or vertical).
   return columns * rows ;
}

void ikigui_map_free(struct ikigui_map *display){
   free(display->map); // Free memory that was allocated with ikigui init.
}
int ikigui_mouse_hit(ikigui_rect *box, int x, int y){ // is the x y coordinate inside the ikigui_rect
	if(x<box->x) return 0;
	if(x>(box->x+box->w)) return 0;
	if(y<box->y) return 0;
	if(y>(box->y+box->h)) return 0;
	return -1; // Return a hit/true value.
}
void ikigui_blit_area(int x, int y, ikigui_rect *source_rect,ikigui_rect *destin_rect){ // Fill in the parameters of the destin_rect. Convinience to automatically get the rect area for a hypthetical blit operation parameters.
	destin_rect->x = x ;
	destin_rect->y = y ;
	destin_rect->w = source_rect->w ;
	destin_rect->h = source_rect->h ; 
}
int ikigui_mouse_pos(struct ikigui_map *display, int x, int y){ // returns -1 if outside the character display
   if( (x < 0) || (y < 0) ) return -1;			// To the left or over the map
   if( ( x < (( display->x_spacing) * display->columns))// To the right of the map 
   &&  ( y < (( display->y_spacing) * display->rows))	// below the map 
   ){  
	int col = x / display->x_spacing ;
	int row = y / display->y_spacing ;
	if( (x < ( col * display->x_spacing  +display->char_width)) && (y < ( row * display->y_spacing  +display->char_hight)) ) // inside the tile
	return (col + (row * display->columns)); // Returns the index of the character in your character array.
   };
   return -1; // Pixel coordinate is outside of the character display. 
}

enum offset {ASCII = -32 }; // For ikigui_map_draw function.
enum blit_type { APLHA = 0, FILLED = 1, SOLID = 2 }; // Types of 'filling' for ikigui_map_draw()
void ikigui_map_draw(struct ikigui_map *display, char filling, int x, int y){  // x y is pixel coordinate to draw it in the window
   
   ikigui_rect srcrect = { .w = display->char_width, .h = display->char_hight }; // , .x = 0, .y = 0,  } ;
   ikigui_rect dstrect = { .w = display->char_width, .h = display->char_hight };

   int set_w;
   if(display->direction) set_w = srcrect.w ;
   else                   set_w = srcrect.h ;

   for(int i = 0 ; i < display->rows ; i++ ){	// draw all rows
      dstrect.y = i * display->y_spacing + y;
      for(int j = 0 ; j < display->columns ; j++ ){	// draw all columns
	  dstrect.x = (j * display->x_spacing) + x;
          int val = set_w * (display->map[i*display->columns + j] + display->offset) ;
	  if(val<0) continue ; // Don't draw tile if given lower value than 0. // val=0; 
	  if(display->direction)srcrect.x = val ; else srcrect.y = val ;
	  switch(filling){ // Draw the character buffer to window.
		  case 0:       ikigui_blit_alpha (display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);	break;
		  case 1:	ikigui_blit_filled(display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);	break;
		  case 2:	ikigui_blit_fast  (display->renderer,display->texture, dstrect.x, dstrect.y, &srcrect);	break;
	  }
      }
   }
}
