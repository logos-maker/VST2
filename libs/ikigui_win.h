        //HINSTANCE hInstance; // https://stackoverflow.com/questions/15462064/hinstance-in-createwindow
#define PRINT_ERROR(a, args...) printf("ERROR %s() %s Line %d: " a, __FUNCTION__, __FILE__, __LINE__, ##args);
#include <windows.h>
#include <stdbool.h>

bool quit = false;

typedef struct{
        int x;
        int y;
        int w;
        int h;
} ikigui_rect;

struct mouse{
	int x, y;
	unsigned char buttons;
	POINT pos;
} ;

typedef struct {
	union { int w, width; };
	union { int h, height; };
	unsigned int *pixels;
        unsigned int size;
        unsigned int bg_color; // for filling background
} ikigui_frame;

typedef struct ikigui_screen{
        ikigui_frame frame;
        struct mouse mouse;
        HWND window_handle;
        BITMAPINFO bitmap_info;
        HBITMAP bitmap;
        HDC bitmap_device_context;
        bool keyboard[256];
        bool has_focus;
        MSG message;     
} ikigui_screen; 

int old_x;
int old_y;

enum { MOUSE_LEFT = 0b1, MOUSE_MIDDLE = 0b10, MOUSE_RIGHT = 0b100, MOUSE_X1 = 0b1000, MOUSE_X2 = 0b10000 };

LRESULT CALLBACK WindowProcessMessage(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam) {

	ikigui_screen *mywin = (ikigui_screen*)GetWindowLongPtr(window_handle, GWLP_USERDATA);

	switch(message) {
		case WM_CLOSE: break;// Someone has pressed x on window
		case WM_QUIT:  break;// This function is usually called when the main window receives WM_DESTROY
		case WM_DESTROY: DestroyWindow(window_handle); break;// Window is about to be destroyed, and can not be hindered.
		case WM_PAINT: {
			PAINTSTRUCT paint;
			HDC device_context;
			device_context = BeginPaint(window_handle, &paint);
			BitBlt(device_context, paint.rcPaint.left, paint.rcPaint.top, paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top, mywin->bitmap_device_context, paint.rcPaint.left, paint.rcPaint.top, SRCCOPY);
			EndPaint(window_handle,&paint);
		} break;
		case WM_KILLFOCUS: mywin->has_focus = false; break;
		case WM_SETFOCUS: mywin->has_focus = true; break;

		case WM_KEYDOWN: break;
		case WM_KEYUP:   break;

		case WM_MOUSEMOVE: { POINT pos;
			if(mywin->mouse.buttons == MOUSE_LEFT){ // SetCursorPos();
				GetCursorPos(&pos);
				mywin->mouse.y += pos.y - mywin->mouse.pos.y;
				mywin->mouse.pos.y = pos.y;
			}else{
			mywin->mouse.x = LOWORD(lParam);
		        mywin->mouse.y = HIWORD(lParam);
			}
		} break;
		case WM_MOUSELEAVE: break;
		case WM_LBUTTONDOWN: mywin->mouse.buttons |=  MOUSE_LEFT;   SetCapture(window_handle); GetCursorPos(&mywin->mouse.pos);break;
		case WM_LBUTTONUP:   mywin->mouse.buttons &= ~MOUSE_LEFT;   ReleaseCapture();break;
		case WM_MBUTTONDOWN: mywin->mouse.buttons |=  MOUSE_MIDDLE; break;
		case WM_MBUTTONUP:   mywin->mouse.buttons &= ~MOUSE_MIDDLE; break;
		case WM_RBUTTONDOWN: mywin->mouse.buttons |=  MOUSE_RIGHT;  break;
		case WM_RBUTTONUP:   mywin->mouse.buttons &= ~MOUSE_RIGHT;  break;

		case WM_MOUSEWHEEL: {
			printf("%s\n", wParam & 0b10000000000000000000000000000000 ? "Down" : "Up");
		} break;

		default: return DefWindowProc(window_handle, message, wParam, lParam);
	}
	return 0;
}
int hflip(int hight,int row){ // invert vertical axis
        return (hight - row)-1;
}

unsigned int alpha_channel(unsigned int color,unsigned int temp){ // done with fixed point math
	unsigned char alpha = temp >> 24; // Alpha channel
	unsigned char alpha_inv = ~alpha;
	unsigned char rf =  (temp&0xff0000)>>16;	// Red forground
	unsigned char gf =  (temp&0xff00)>>8;		// Green forground
	unsigned char bf = temp&0xff;			// Blue forground
	unsigned char rb = (color&0xff0000)>>16;	// Red beckground
	unsigned char gb = (color&0xff00)>>8;		// Red background
	unsigned char bb = color&0xff;			// Blue background                
	unsigned char ro = (alpha_inv*rb + alpha*rf)>>8;   // background + forground
	unsigned char go = (alpha_inv*gb + alpha*gf)>>8;   // background + forground
	unsigned char bo = (alpha_inv*bb + alpha*bf)>>8;   // background + forground
	return (unsigned int)((ro << 16) + (go<< 8) + bo); 
}

void ikigui_fill_bg(ikigui_frame *frame,unsigned int color){// A background color for automatic filling of transparent pixels.
	// to precalc graphics for usage with ikigui_blit_part_fast() for faster graphics. Can be convinient in some cases.
        for(int i = 0 ; i < frame->size ; i++){
                frame->pixels[i] = alpha_channel(color,frame->pixels[i]);
        }
}

void ikigui_blit_part(ikigui_frame *mywin,ikigui_frame *frame, int x, int y, ikigui_rect *part){ // Draw area
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w <= (x+part->w))return; // shielding crash
        if(mywin->h <= (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			mywin->pixels[(x+i+(hflip(mywin->h,j+y))*mywin->w)] 
			= alpha_cannel(mywin->pixels[(x+i+(hflip(mywin->h,j+y))*mywin->w)], frame->pixels[i+part->x+frame->w*(j+part->y)]);
                }
        }
}

void ikigui_blit_part_filled(ikigui_frame *mywin,ikigui_frame *frame, int x, int y, ikigui_rect *part){ // Draw area
        if((x<0) || (y<0))return; // sheilding crash
        if(mywin->w <= (x+part->w))return; // shielding crash
        if(mywin->h <= (y+part->h))return; // shielding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
			mywin->pixels[(x+i+(hflip(mywin->h,j+y))*mywin->w)] 
			= alpha_cannel(mywin->bg_color, frame->pixels[i+part->x+frame->w*(j+part->y)]);
                }
        }
}

void ikigui_blit_part_fast(ikigui_frame *mywin,ikigui_frame *frame, int x, int y, ikigui_rect *part){ // Draw area
        if((x<0) || (y<0))return; // shelding crash
        if(mywin->w <= (x+part->w))return; // shelding crash
        if(mywin->h <= (y+part->h))return; // shelding crash

        for(int j = 0 ; j < part->h ; j++){ // vertical
                for(int i = 0 ; i < part->w ; i++){   // horizontal
                        mywin->pixels[(x+i+(hflip(mywin->h,j+y))*mywin->w)] = frame->pixels[i+part->x+frame->w*(j+part->y)];
                }
        }
}

void ikigui_blit(ikigui_frame *mywin,ikigui_frame *frame, int x, int y){
        for(int j = 0 ; j < frame->h ; j++){ // vertical
                for(int i = 0 ; i < frame->w ; i++){   // horizontal
                        mywin->pixels[(x+i+(hflip(mywin->h,j+y))*mywin->w)] = frame->pixels[i+frame->w*(j)];
                }
        }
}

void ikigui_bmp_include(ikigui_frame *frame,const unsigned char* bmp_incl){
        unsigned int start;
        frame->w = bmp_incl[0x12] + (bmp_incl[0x12+1]<<8) + (bmp_incl[0x12+2]<<16) + (bmp_incl[0x12+3]<<24);
        frame->h = bmp_incl[0x16] + (bmp_incl[0x16+1]<<8) + (bmp_incl[0x16+2]<<16) + (bmp_incl[0x16+3]<<24);
        start =    bmp_incl[0x0a] + (bmp_incl[0x0a+1]<<8) + (bmp_incl[0x0a+2]<<16) + (bmp_incl[0x0a+3]<<24);

        frame->pixels = (unsigned int*)malloc(frame->w*frame->h*4);

        int counter = 0 ; 
        for(int j = frame->h -1 ; j >= 0 ; j--){ 
                for(int i = 0 ; i < frame->w ; i++){
                        int pixl_addr = (i+(j*frame->w))*4+start;
                        frame->pixels[counter++]= bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24);
                        unsigned int temp = bmp_incl[pixl_addr]+ (bmp_incl[pixl_addr+1]<<8)+ (bmp_incl[pixl_addr+2]<<16)+ (bmp_incl[pixl_addr+3]<<24);
                }
        }
        frame->size = frame->w * frame->h ;
}

void ikigui_open_plugin_window(ikigui_screen *mywin,void *ptr,int w, int h){

	const wchar_t window_class_name[] = L"Window Class";
	static WNDCLASS window_class = { 0 };

	window_class.lpfnWndProc = WindowProcessMessage;
        window_class.hInstance = GetModuleHandle(NULL);
        window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.lpszClassName = (PCSTR)window_class_name;
        window_class.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
        window_class.lpszMenuName = NULL;
	RegisterClass(&window_class);

        // To do before opening window!
	mywin->bitmap_info.bmiHeader.biSize = sizeof(mywin->bitmap_info.bmiHeader);
	mywin->bitmap_info.bmiHeader.biPlanes = 1;
	mywin->bitmap_info.bmiHeader.biBitCount = 32;
	mywin->bitmap_info.bmiHeader.biCompression = BI_RGB;
	mywin->bitmap_device_context = CreateCompatibleDC(0);

        mywin->window_handle = CreateWindow((PCSTR)window_class_name, "BitBlt Test Program ", WS_CHILD | WS_VISIBLE , CW_USEDEFAULT, CW_USEDEFAULT, w, h, (HWND)ptr, NULL, NULL, NULL);
	if(mywin->window_handle == NULL) {
		PRINT_ERROR("CreateWindow() failed. Returned NULL.\n");
	}

	mywin->frame.w = mywin->bitmap_info.bmiHeader.biWidth = w;
	mywin->frame.h = mywin->bitmap_info.bmiHeader.biHeight = h;
	if(mywin->bitmap) DeleteObject(mywin->bitmap);
	mywin->bitmap = CreateDIBSection(NULL, &mywin->bitmap_info, DIB_RGB_COLORS, (void**)&mywin->frame.pixels, 0, 0);
	SelectObject(mywin->bitmap_device_context, mywin->bitmap);
	SetWindowLongPtr(mywin->window_handle, GWLP_USERDATA, (LONG_PTR)mywin);
}

void ikigui_get_events(struct ikigui_screen *mywin){
        while(PeekMessage(&mywin->message, NULL, 0, 0, PM_REMOVE)) { DispatchMessage(&mywin->message); }
}

void ikigui_update_window(struct ikigui_screen *mywin){
        InvalidateRect(mywin->window_handle, NULL, FALSE);
        UpdateWindow(mywin->window_handle);
}
