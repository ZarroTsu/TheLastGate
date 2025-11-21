// TODO: Modern GCC/MinGW compatibility - Update includes for modern compilers
// - Replace <alloc.h> with <malloc.h> or use stdlib.h functions
// - <io.h> may need to be <unistd.h> or use standard C file functions
// - <windowsx.h> may not be needed with modern Windows SDK
#include <stdio.h>
#include <alloc.h>  // TODO: Replace with <malloc.h> or <stdlib.h>
#include <fcntl.h>
#include <io.h>     // TODO: Replace with <unistd.h> or standard C file I/O
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>  // TODO: Verify if needed with modern Windows SDK
#include <process.h>
#include <time.h>
#include <math.h>
#pragma hdrstop  // TODO: Borland C++ specific - remove for GCC
#define INITGUID
// TODO: DirectDraw is deprecated - consider migrating to:
// - SDL2 for cross-platform compatibility
// - DirectX 9+ (D3D9) for Windows-only
// - OpenGL/Vulkan for modern graphics
#include <ddraw.h>  // TODO: Deprecated API - replace with modern graphics library
#include <dsound.h> // TODO: Deprecated API - replace with modern audio library (SDL_mixer, OpenAL)
#include "dd.h"
#include "common.h"
#include "inter.h"
#include "lpng/png.h"

int tricky_flag=0;

int maxmem=16*1024*1024,maxvid=2*1024*1024;
int usedmem=0,usedvid=0;
char *DDERR;
extern int do_alpha;
extern int do_shadow;
extern int do_darkmode;

extern int screen_width, screen_height, screen_tilexoff, screen_tileyoff;
extern short screen_windowed;
extern short screen_renderdist;

HANDLE heap=NULL;

int blockcnt=0,blocktot=0,blockgc=0;
int alphapix=0,fullpix=0;

unsigned short *dd_load_png(FILE *fp,int *xs,int *ys,unsigned char **alpha_ptr,int *alphacnt_ptr);
void display_alpha(unsigned char *alpha,int alphacnt,int xf,int yf,int effect);
FILE *load_pnglib(int nr);

/**
 * init_xalloc - Initialize custom memory allocator heap
 *
 * Creates a Windows heap for managing memory allocations. This custom allocator
 * tracks memory usage and provides optimized allocation for the game engine.
 * The heap is created with HEAP_GENERATE_EXCEPTIONS to ensure allocation failures
 * raise exceptions rather than returning NULL.
 */
void init_xalloc(void)
{
	// TODO: Modern GCC/MinGW - HeapCreate is Windows-specific
	// Consider using standard malloc/free for cross-platform compatibility
	// or implement platform-specific abstractions
	heap=HeapCreate(HEAP_GENERATE_EXCEPTIONS,maxmem+4096*1024,0);
}

/**
 * xmalloc - Allocate memory from the custom heap
 * @size: Number of bytes to allocate
 *
 * Allocates memory from the custom heap and tracks allocation statistics.
 * Updates global memory usage counters (usedmem) and block count (blockcnt).
 *
 * Returns: Pointer to allocated memory
 */
void *xmalloc(int size)
{
	char *ptr;

	blockcnt++; blocktot++;

	// TODO: Modern GCC/MinGW - Replace HeapAlloc with malloc for portability
	// HeapSize may not be available - track allocations manually
	ptr=HeapAlloc(heap,0,size);
	usedmem+=HeapSize(heap,0,ptr)+16;

	return ptr;
}

/**
 * xfree - Free memory allocated with xmalloc/xcalloc
 * @ptr: Pointer to memory block to free
 *
 * Releases memory back to the custom heap and updates memory tracking counters.
 * Decrements block count and reduces used memory total.
 */
void xfree(void *ptr)
{
	blockcnt--;
	// TODO: Modern GCC/MinGW - Replace HeapFree with free for portability
	usedmem-=HeapSize(heap,0,ptr)+16;

	HeapFree(heap,0,ptr);
}

/**
 * xcalloc - Allocate zero-initialized memory from custom heap
 * @size1: Number of elements
 * @size2: Size of each element in bytes
 *
 * Allocates memory for an array of elements and initializes all bytes to zero.
 * Equivalent to calloc() but uses the custom heap. Updates memory usage counters.
 *
 * Returns: Pointer to zero-initialized memory block
 */
void *xcalloc(int size1,int size2)
{
	int size;
	unsigned char *ptr;

	size=size1*size2;

	blockcnt++; blocktot++;

	// TODO: Modern GCC/MinGW - Replace HeapAlloc with calloc for portability
	ptr=HeapAlloc(heap,HEAP_ZERO_MEMORY,size);
	usedmem+=HeapSize(heap,0,ptr)+16;

	return ptr;
}

extern HWND desk_hwnd;

#define TILE 32
unsigned short background=0;
int invisible=0;

void one(void);
void two(void);
void three(void);
void four(void);
void five(void);

int MAXX,MAXY;
int MAXX1,MAXY1;
int RED,GREEN,BLUE;
int RGBM=-1;

// TODO: Modern GCC/MinGW - Replace DirectDraw interfaces with modern equivalents
// SDL2: SDL_Window*, SDL_Renderer*, SDL_Texture*
// D3D9: IDirect3D9*, IDirect3DDevice9*, IDirect3DSurface9*
LPDIRECTDRAW dd=NULL;  // TODO: Replace with SDL_Renderer* or IDirect3D9*

LPDIRECTDRAWSURFACE sur1=NULL,sur2=NULL,suro=NULL;  // TODO: Replace with SDL_Texture* or IDirect3DSurface9*
LPDIRECTDRAWCLIPPER clip=NULL;  // TODO: Not needed in SDL2 (automatic clipping)

void *load_file(char *file);
void dd_invalidate_cache(void);
void *dd_get_ptr(LPDIRECTDRAWSURFACE sur);
int dd_release_ptr(LPDIRECTDRAWSURFACE sur);
void dd_flip(void);
void dd_flip_windowed(void);

int MAXCACHE;
int MAXXOVER;

int cachex,cachey;

DWORD total,left;

void dd_error(HWND hwnd,char *msg,long err);

/**
 * get_dderr - Convert DirectDraw error code to human-readable string
 * @err: DirectDraw error code (HRESULT)
 *
 * Translates DirectDraw API error codes into descriptive string names
 * for debugging and error reporting purposes.
 *
 * Returns: String representation of the error code
 */
char *get_dderr(long err)
{
	char *ptr;

	switch (err) {
		case DDERR_INVALIDOBJECT:                       ptr="INVALIDOBJECT"; break;
		case DDERR_INVALIDPARAMS:                       ptr="INVALIDPARAMS"; break;
		case DDERR_OUTOFMEMORY:                         ptr="OUTOFMEMORY"; break;
		case DDERR_SURFACEBUSY:                         ptr="SURFACEBUSY"; break;
		case DDERR_SURFACELOST:                         ptr="SURFACELOST"; break;
		case DDERR_WASSTILLDRAWING:                 ptr="WASSTILLDRAWING"; break;
		case DDERR_INCOMPATIBLEPRIMARY:             ptr="INCOMPATIBLEPRIMARY"; break;
		case DDERR_INVALIDCAPS:                   ptr="INVALIDCAPS"; break;
		case DDERR_INVALIDPIXELFORMAT:            ptr="INVALIDPIXELFORMAT"; break;
		case DDERR_NOALPHAHW:                     ptr="NOALPHAHW"; break;
		case DDERR_NOCOOPERATIVELEVELSET:         ptr="NOCOOPERATIVELEVELSET"; break;
		case DDERR_NODIRECTDRAWHW:                ptr="NODIRECTDRAWHW"; break;
		case DDERR_NOEMULATION:                   ptr="NOEMULATION"; break;
		case DDERR_NOEXCLUSIVEMODE:               ptr="NOEXCLUSIVEMODE"; break;
		case DDERR_NOFLIPHW:                      ptr="NOFLIPHW"; break;
		case DDERR_NOMIPMAPHW:                    ptr="NOMIPMAPHW"; break;
		case DDERR_NOOVERLAYHW:                   ptr="NOOVERLAYHW"; break;
		case DDERR_NOZBUFFERHW:                   ptr="NOZBUFFERHW"; break;
		case DDERR_OUTOFVIDEOMEMORY:              ptr="OUTOFVIDEOMEMORY"; break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS:   ptr="PRIMARYSURFACEALREADYEXISTS"; break;
		case DDERR_UNSUPPORTEDMODE:               ptr="UNSUPPORTEDMODE"; break;
		case DDERR_EXCEPTION:                           ptr="EXCEPTION"; break;
		case DDERR_GENERIC:                             ptr="GENERIC"; break;
		case DDERR_INVALIDRECT:                         ptr="INVALIDRECT"; break;
		case DDERR_NOTFLIPPABLE:                        ptr="NOTFLIPPABLE"; break;
		case DDERR_UNSUPPORTED:                         ptr="UNSUPPORTED"; break;
		default:                                                ptr="Unknown Error"; break;
	}
	return ptr;
}

/**
 * dd_clear - Clear all DirectDraw surfaces to black
 *
 * Zeroes out all pixels in the primary surface (sur1), back buffer (sur2),
 * and sprite cache surface (suro). This effectively clears the screen and
 * cache to a black background. Used during initialization and mode changes.
 */
void dd_clear(void)
{
	unsigned short *ptr;
	int n;

	ptr=dd_get_ptr(sur1);
	if (!ptr) return;
	if (ptr) for (n=0; n<MAXX1*MAXY1; n++) ptr[n]=0;
	dd_release_ptr(sur1);

	ptr=dd_get_ptr(sur2);
	if (!ptr) return;
	if (ptr) for (n=0; n<MAXX1*MAXY1; n++) ptr[n]=0;
	dd_release_ptr(sur2);

	ptr=dd_get_ptr(suro);
	if (!ptr) return;
	if (ptr) for (n=0; n<MAXCACHE*1024; n++) ptr[n]=0;
	dd_release_ptr(suro);
}

struct vtab {
	int x,y;
};

struct vtab vtab[]= {
	{   1024,   2048},
	{   800,    2048},
	{   640,    2048},
	{   512,    2048},
	{   1024,   1024},
	{   800,    1024},
	{   640,    1024},
	{   512,    1024},
	{   1024,   600},
	{   1600,    600},
	{   640,    600},
	{   512,    600},
	{   1024,   512},
	{   800,    512},
	{   640,    512},
	{   512,    512},
	{   1024,   480},
	{   800,    480},
	{   640,    480},
	{   512,    480},
	{   1024,   256},
	{   800,    256},
	{   640,    256},
	{   512,    256},
/*	{	1024,	128	},
	{	800,	128	},
	{	640,	128	},
	{	512,	128	},
	{	1024,	64	},
	{	800,	64	},
	{	640,	64	},
	{	512,	64	},
	{	256,	64	},
	{	128,	64	},
	{	64,	64	} */};

int usedvidmem=0;

/**
 * dd_init - Initialize DirectDraw in fullscreen exclusive mode
 * @hwnd: Window handle to attach DirectDraw to
 * @x: Desired screen width in pixels
 * @y: Desired screen height in pixels
 *
 * Initializes DirectDraw for fullscreen rendering with double-buffering.
 * Steps performed:
 * 1. Create DirectDraw object
 * 2. Set exclusive fullscreen cooperative level
 * 3. Set display mode to requested resolution (16-bit color)
 * 4. Detect RGB bit masks for pixel format (565 or 555)
 * 5. Create primary surface with one back buffer for flipping
 * 6. Create offscreen surface (suro) for sprite caching
 * 7. Clear all surfaces and calculate cache dimensions
 *
 * The function tries to allocate sprite cache in video memory first, falling
 * back to system memory if necessary. Cache size is calculated based on the
 * largest available surface dimensions from the vtab array.
 *
 * Returns: 0 on success, negative error code on failure:
 *          -1: DirectDrawCreate failed
 *          -2: SetCooperativeLevel failed
 *          -3: SetDisplayMode failed
 *          -4: GetDisplayMode failed
 *          -5: Not RGB mode
 *          -6: Create primary surface failed
 *          -7: Get back buffer failed
 *          -8: GetSurfaceDesc failed
 *          -9: Create sprite cache surface failed
 *          -10: Get cache surface desc failed
 */
// TODO: Modern GCC/MinGW - Entire DirectDraw implementation needs replacement
// DirectDraw is deprecated since Windows Vista. Options:
// 1. Use SDL2 (recommended for cross-platform)
// 2. Use Direct3D 9+ (Windows-only, still supported)
// 3. Use OpenGL/GLFW (cross-platform, modern)
// This function would become SDL_CreateWindow + SDL_CreateRenderer
int dd_init(HWND hwnd,int x,int y)
{
	// TODO: Replace DDSURFACEDESC with SDL_Window/SDL_Renderer or D3D9 device
	DDSURFACEDESC surface;
	long ret;
	int ysize,n;

	MAXX=x; MAXY=y;

	// TODO: Replace DirectDrawCreate with modern graphics API initialization
	if ((ret=DirectDrawCreate(NULL,&dd,NULL))!=DD_OK) {
		DDERR=get_dderr(ret);
		return -1;
	}

	ret=dd->lpVtbl->SetCooperativeLevel(dd,hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
	if (ret!=DD_OK) {
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -2;
	}

	ret=dd->lpVtbl->SetDisplayMode(dd,x,y,16);
	if (ret!=DD_OK) {
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -3;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_ALL;
	ret=dd->lpVtbl->GetDisplayMode(dd,&surface);
	if (ret!=DD_OK) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -4;
	}

	if (!(surface.ddpfPixelFormat.dwFlags&DDPF_RGB)) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -5;
	}

	RED=surface.ddpfPixelFormat.u2.dwRBitMask;
	GREEN=surface.ddpfPixelFormat.u3.dwGBitMask;
	BLUE=surface.ddpfPixelFormat.u4.dwBBitMask;

	if (RED==0xF800 && GREEN==0x07E0 && BLUE==0x001F) RGBM=0;
	else if (RED==0x7C00 && GREEN==0x03E0 && BLUE==0x001F) RGBM=1;
	else if (RED==0x001F && GREEN==0x07E0 && BLUE==0xF800) RGBM=2;

	if (tricky_flag) {
		surface.dwSize=sizeof(surface);
		surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
		surface.dwWidth=x;  // 1280
		surface.dwHeight=y; //  720

		dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
	}

	memset(&surface,0,sizeof(surface));
	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
	surface.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX;
	surface.dwBackBufferCount=1;

	ret=dd->lpVtbl->CreateSurface(dd,&surface,&sur1,NULL);
	if (ret!=DD_OK) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -6;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS;
	surface.ddsCaps.dwCaps=DDSCAPS_BACKBUFFER;
	ret=sur1->lpVtbl->GetAttachedSurface(sur1,&surface.ddsCaps,&sur2);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -7;
	}

	if (tricky_flag) {
		suro->lpVtbl->Release(suro);
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_HEIGHT;
	ret=sur1->lpVtbl->GetSurfaceDesc(sur1,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -8;
	}
	MAXX1=surface.u1.lPitch/2;
	MAXY1=surface.dwHeight;
	MAXX=MAXX1;
	MAXY=MAXY1;

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS;
	ret=sur1->lpVtbl->GetSurfaceDesc(sur2,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -108;
	}
	usedvidmem=surface.ddsCaps.dwCaps&DDSCAPS_VIDEOMEMORY;

	// trying some combinations
	if (usedvidmem) {
		for (n=0; n<sizeof(vtab)/sizeof(struct vtab); n++) {
			surface.dwSize=sizeof(surface);
			surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
			surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
			surface.dwWidth=vtab[n].x;
			surface.dwHeight=vtab[n].y;

			ret=dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
			if (ret==DD_OK)	break;
		}
	} else ret=!DD_OK;

	if (ret!=DD_OK) {
		// trying some combinations
		for (n=0; n<sizeof(vtab)/sizeof(struct vtab); n++) {
			surface.dwSize=sizeof(surface);
			surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
			surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
			surface.dwWidth=vtab[n].x;
			surface.dwHeight=vtab[n].y;

			ret=dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
			if (ret==DD_OK)	break;
		}
		if (ret!=DD_OK) {
			DDERR=get_dderr(ret);
			return -9;
		}
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_CAPS|DDSD_WIDTH;
	ret=suro->lpVtbl->GetSurfaceDesc(suro,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -10;
	}
	MAXXOVER=surface.u1.lPitch/2;		// God, I hate DirectDraw!! It sucks!!
	ysize=surface.dwHeight;			// Absolutely everying seems to be variable.

	if (!(surface.ddsCaps.dwCaps&DDSCAPS_VIDEOMEMORY)) {
		xlog(0,"Could not allocate sprite cache (not enough video memory).");
		xlog(0,"Performance will suffer severely.");
	}

	cachex=MAXXOVER/TILE;
	cachey=ysize/TILE;

	MAXCACHE=min(65535,cachex*cachey);

	dd_clear();

	return 0;
}

/**
 * dd_deinit - Clean up DirectDraw resources in fullscreen mode
 *
 * Releases the primary surface (which also releases the back buffer)
 * and restores the original display mode before releasing the DirectDraw
 * object. Should be called before exiting fullscreen mode.
 */
void dd_deinit(void)
{
	if (sur1) {
		sur1->lpVtbl->Release(sur1); sur1=NULL;
	}
	if (dd) {
		dd->lpVtbl->RestoreDisplayMode(dd);
		dd->lpVtbl->Release(dd);
		dd=NULL;
	}
}

/**
 * dd_init_windowed - Initialize DirectDraw in windowed mode
 * @hwnd: Window handle to attach DirectDraw to
 * @x: Window client area width in pixels
 * @y: Window client area height in pixels
 *
 * Initializes DirectDraw for windowed rendering. Creates a primary surface
 * representing the entire screen, an offscreen back buffer (sur2) for rendering,
 * and a clipper to handle window boundaries. The sprite cache surface (suro)
 * is created in video memory if possible.
 *
 * Unlike fullscreen mode, windowed mode:
 * - Uses DDSCL_NORMAL cooperative level
 * - Requires a clipper attached to the primary surface
 * - Uses manual blitting instead of page flipping
 * - Forces 565 RGB format (0xF800, 0x07E0, 0x001F)
 *
 * Returns: 0 on success, negative error code on failure:
 *          -1: DirectDrawCreate failed
 *          -2: SetCooperativeLevel failed
 *          -3: Create primary surface failed
 *          -4: Create back buffer failed
 *          -5: CreateClipper failed
 *          -6: Attach clipper to window failed
 *          -7: Attach clipper to primary failed
 *          -8: GetSurfaceDesc failed
 *          -9: Create sprite cache failed
 *          -10: Get cache surface desc failed
 */
// TODO: Modern GCC/MinGW - Replace windowed DirectDraw with modern API
// SDL2 example: SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_SHOWN)
int dd_init_windowed(HWND hwnd,int x,int y)
{
	// TODO: Replace DirectDraw structures with SDL/D3D equivalents
	DDSURFACEDESC surface;
	long ret;
	int ysize,n;

	printf("DirectDrawCreate\n");
	// TODO: Replace DirectDrawCreate with SDL_CreateWindow or D3D9 CreateDevice
	if ((ret=DirectDrawCreate(NULL,&dd,NULL))!=DD_OK) {
		DDERR=get_dderr(ret); return -1;
	}

	printf("SetCooperativeLevel\n");
	ret=dd->lpVtbl->SetCooperativeLevel(dd,hwnd,DDSCL_NORMAL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -2;
	}

	printf("CreateSurface\n");
	memset(&surface,0,sizeof(surface));
	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS;
	surface.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;

	ret=dd->lpVtbl->CreateSurface(dd,&surface,&sur1,NULL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -3;
	}

	printf("CreateSurface 2\n");
	memset(&surface,0,sizeof(surface));
	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
	surface.dwWidth=x;
	surface.dwHeight=y;

	surface.ddpfPixelFormat.dwSize=sizeof(surface.ddpfPixelFormat);
	surface.ddpfPixelFormat.dwFlags=DDPF_RGB;
	surface.ddpfPixelFormat.u1.dwRGBBitCount=16;
	surface.ddpfPixelFormat.u2.dwRBitMask=0xf800;
	surface.ddpfPixelFormat.u3.dwGBitMask=0x07e0;
	surface.ddpfPixelFormat.u4.dwBBitMask=0x001f;
	surface.ddpfPixelFormat.u5.dwRGBAlphaBitMask=0;

	ret=dd->lpVtbl->CreateSurface(dd,&surface,&sur2,NULL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -4;
	}

	printf("CreateClipper\n");
	ret=dd->lpVtbl->CreateClipper(dd,0,&clip,NULL);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -5;
	}

	printf("Attach Clipper to Window\n");
	ret=clip->lpVtbl->SetHWnd(clip,0,hwnd);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -6;
	}

	printf("Attach Clipper to Primary\n");
	ret=sur1->lpVtbl->SetClipper(sur1,clip);
	if (ret!=DD_OK) {
		DDERR=get_dderr(ret); return -7;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_HEIGHT;
	ret=sur2->lpVtbl->GetSurfaceDesc(sur2,&surface);
	if (ret!=DD_OK) {
		sur2->lpVtbl->Release(sur2);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -8;
	}
	MAXX1=surface.u1.lPitch/2;
	MAXY1=surface.dwHeight;
	MAXX=MAXX1;
	MAXY=MAXY1;

	RED=0xF800;
	GREEN=0x07E0;
	BLUE=0x001F;
	RGBM=0;

	for (n=0; n<sizeof(vtab)/sizeof(struct vtab); n++) {
		surface.dwSize=sizeof(surface);
		surface.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
		surface.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
		surface.dwWidth=vtab[n].x;
		surface.dwHeight=vtab[n].y;
		surface.ddpfPixelFormat.dwSize=sizeof(surface.ddpfPixelFormat);
		surface.ddpfPixelFormat.dwFlags=DDPF_RGB;
		//surface.ddpfPixelFormat.u1.dwRGBBitCount=16;
		//surface.ddpfPixelFormat.u2.dwRBitMask=0x7c00;
		//surface.ddpfPixelFormat.u3.dwGBitMask=0x03e0;
		//surface.ddpfPixelFormat.u4.dwBBitMask=0x001f;
		//surface.ddpfPixelFormat.u5.dwRGBAlphaBitMask=0;

		ret=dd->lpVtbl->CreateSurface(dd,&surface,&suro,NULL);
		if (ret==DD_OK)	break;
	}
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		sur2->lpVtbl->Release(sur2);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -9;
	}

	surface.dwSize=sizeof(surface);
	surface.dwFlags=DDSD_PITCH|DDSD_CAPS|DDSD_WIDTH;
	ret=suro->lpVtbl->GetSurfaceDesc(suro,&surface);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Release(sur1);
		sur2->lpVtbl->Release(sur2);
		dd->lpVtbl->Release(dd);
		DDERR=get_dderr(ret);
		return -10;
	}
	MAXXOVER=surface.u1.lPitch/2;	// God, I hate DirectDraw!! It sucks!!
	ysize=surface.dwHeight;		// Absolutely everying seems to be variable.

	if (!(surface.ddsCaps.dwCaps&DDSCAPS_VIDEOMEMORY)) {
		xlog(0,"Could not allocate sprite cache (not enough video memory).");
		xlog(0,"Performance will suffer severely.");
	}

	cachex=MAXXOVER/TILE;
	cachey=ysize/TILE;

	MAXCACHE=min(65535,cachex*cachey);

	return 0;
}

/**
 * dd_deinit_windowed - Clean up DirectDraw resources in windowed mode
 *
 * Releases the primary surface, back buffer, and DirectDraw object.
 * Unlike fullscreen deinit, no display mode restoration is needed.
 */
void dd_deinit_windowed(void)
{
	if (sur1) {
		sur1->lpVtbl->Release(sur1); sur1=NULL;
	}

	if (dd) {
		dd->lpVtbl->Release(dd); dd=NULL;
	}
}

// Lock counter for debugging surface access
static int locks=0;

/**
 * dd_get_ptr - Lock a DirectDraw surface and get pointer to video memory
 * @sur: DirectDraw surface to lock
 *
 * Locks the specified surface for direct memory access and returns a pointer
 * to the surface memory. The lock counter is incremented for debugging.
 * Always pair with dd_release_ptr() when done accessing the surface.
 *
 * Uses DDLOCK_WAIT to wait until the surface can be locked.
 *
 * Returns: Pointer to surface memory, or NULL on failure
 */
void *dd_get_ptr(LPDIRECTDRAWSURFACE sur)
{
	// TODO: Modern GCC/MinGW - Replace surface locking with modern API
	// SDL2: SDL_LockSurface() or SDL_LockTexture()
	// D3D9: Lock texture/surface
	DDSURFACEDESC surface;
	long ret;

	surface.dwSize=sizeof(surface);
	// TODO: Replace DirectDraw Lock with SDL_LockSurface/SDL_LockTexture
	ret=sur->lpVtbl->Lock(sur,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_get_ptr",ret);
		return NULL;
	}

	locks++;

	return surface.lpSurface;
}

/**
 * dd_release_ptr - Unlock a DirectDraw surface
 * @sur: DirectDraw surface to unlock
 *
 * Releases a lock obtained with dd_get_ptr(). Must be called after finishing
 * all direct memory access to the surface. Decrements the lock counter.
 *
 * Returns: 0 on success, -1 on failure
 */
int dd_release_ptr(LPDIRECTDRAWSURFACE sur)
{
	long ret;

	locks--;
	// TODO: Modern GCC/MinGW - Replace with SDL_UnlockSurface/SDL_UnlockTexture
	ret=sur->lpVtbl->Unlock(sur,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_release_ptr",ret); return -1;
	}

	return 0;
}

/**
 * _dd_get_ptr - Lock surface without incrementing lock counter
 * @sur: DirectDraw surface to lock
 *
 * Internal version of dd_get_ptr() that doesn't track locks. Used by
 * debugging functions like islocked() to avoid affecting lock count.
 *
 * Returns: Pointer to surface memory, or NULL on failure
 */
void *_dd_get_ptr(LPDIRECTDRAWSURFACE sur)
{
	DDSURFACEDESC surface;
	long ret;

	surface.dwSize=sizeof(surface);

	ret=sur->lpVtbl->Lock(sur,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_get_ptr",ret); return NULL;
	}

	return surface.lpSurface;
}

/**
 * _dd_release_ptr - Unlock surface without affecting lock counter
 * @sur: DirectDraw surface to unlock
 *
 * Internal version of dd_release_ptr() that doesn't decrement the lock counter.
 * Used in conjunction with _dd_get_ptr() for internal operations.
 *
 * Returns: 0 on success, -1 on failure
 */
int _dd_release_ptr(LPDIRECTDRAWSURFACE sur)
{
	long ret;

	ret=sur->lpVtbl->Unlock(sur,NULL);

	if (ret!=DD_OK) {
		dd_error(0,"dd_release_ptr",ret); return -1;
	}

	return 0;
}

/**
 * islocked - Debug indicator for locked surfaces
 *
 * Draws red pixels in the top-left corner if any surfaces are currently locked.
 * This is a debugging aid to detect surfaces that haven't been properly unlocked,
 * which can cause rendering issues. The red indicator appears as a 10x3 pixel bar.
 */
void islocked(void)
{
	unsigned short *ptr;
	int n;

	if (!locks)	return;

	ptr=_dd_get_ptr(sur1);
	if (!ptr) return;

	for (n=0; n<10; n++) {
		ptr[n]=0xF800;
		ptr[n+screen_width]=0xF800;
		ptr[n+screen_width*2]=0xF800;
	}
	_dd_release_ptr(sur1);
}

/**
 * dd_showbar - Draw a filled rectangle with blending
 * @xf: X position (left edge)
 * @yf: Y position (top edge)
 * @xs: Width in pixels
 * @ys: Height in pixels
 * @col: Color in RGB565 or RGB555 format
 *
 * Renders a filled bar to the back buffer (sur2) with color blending.
 * In 565 mode, the color is drawn directly. In 555 mode, colors are averaged
 * with the background for a blending effect. Clipping is performed to keep
 * the bar within screen bounds. Adds 4-pixel offset in windowed mode.
 */
void dd_showbar(int xf,int yf,int xs,int ys,unsigned short col)
{
	unsigned short *ptr;
	int x,y,off;
	int xt,yt,r,g,b;

	ptr=dd_get_ptr(sur2);
	if (ptr==NULL) return;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		yf += 4;
	}

	xt=min(screen_width, xf+xs);
	yt=min(screen_height, yf+ys);
	xf=max(0, xf);
	yf=max(0, yf);

	for (y=yf; y<yt; y++) {
		for (x=xf,off=y*MAXX+xf; x<xt; x++,off++) {
			switch (RGBM) {
				case 0:	  // 565
					//r=((ptr[off]&0xF800)>>11)+((col&0xF800)>>11);
					//g=((ptr[off]&0x07E0)>>5)+((col&0x07E0)>>5);
					//b=((ptr[off]&0x001F))+((col&0x001F));

					//r/=3;
					//g/=3;
					//b/=3;

					//ptr[off]=(unsigned short)((r<<11)+(g<<5)+b);
					ptr[off]=col; // livelier colors?
					break;
				case 1:	 // 555
					r=((ptr[off]&0x7C00)>>10)+((col&0x7C00)>>10);
					g=((ptr[off]&0x03E0)>>5)+((col&0x03E0)>>5);
					b=((ptr[off]&0x001F))+((col&0x001F));

					r/=3;
					g/=3;
					b/=3;

					ptr[off]=(unsigned short)((r<<10)+(g<<5)+b);
					break;
				case 2:	 // 555
					b=((ptr[off]&0x7C00)>>10)+((col&0x7C00)>>10);
					g=((ptr[off]&0x03E0)>>5)+((col&0x03E0)>>5);
					r=((ptr[off]&0x001F))+((col&0x001F));

					r/=3;
					g/=3;
					b/=3;

					ptr[off]=(unsigned short)((b<<10)+(g<<5)+r);
					break;
			}
		}
	}

	dd_release_ptr(sur2);
}

/**
 * dd_showbox - Draw a hollow rectangle outline
 * @xf: X position (left edge)
 * @yf: Y position (top edge)
 * @xs: Width in pixels
 * @ys: Height in pixels
 * @col: Color in RGB format
 *
 * Draws only the border of a rectangle to the back buffer (sur2).
 * Only pixels on the edges are drawn - interior is left unchanged.
 * Adds 4-pixel offset in windowed mode.
 */
void dd_showbox(int xf,int yf,int xs,int ys,unsigned short col)
{
	unsigned short *ptr;
	int x,y,off;
	int xt,yt;

	ptr=dd_get_ptr(sur2);
	if (ptr==NULL) return;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		yf += 4;
	}

	xt=xf+xs;
	yt=yf+ys;

	for (y=yf; y<=yt; y++) {
		for (x=xf,off=y*MAXX+xf; x<=xt; x++,off++) {
			if (x!=xf && x!=xt && y!=yf && y!=yt) continue;
			ptr[off]=col;
		}
	}

	dd_release_ptr(sur2);
}

/**
 * dd_set_background - Set transparent color key for sprite blitting
 * @color: Color value to treat as transparent
 *
 * Configures the color key for the sprite cache surface (suro). Any pixels
 * matching this color will be transparent when blitting sprites. This is
 * essential for sprite rendering with transparency.
 *
 * Returns: 0 on success, -1 on failure
 */
int dd_set_background(int color)
{
	// TODO: Modern GCC/MinGW - Color key transparency
	// SDL2: SDL_SetColorKey(surface, SDL_TRUE, color)
	// For textures, use SDL_SetTextureBlendMode
	DDCOLORKEY key;
	long ret;

	background=(unsigned short)color;
	key.dwColorSpaceLowValue=color;
	key.dwColorSpaceHighValue=color;

	// TODO: Replace SetColorKey with SDL_SetColorKey or alpha channel
	ret=suro->lpVtbl->SetColorKey(suro,DDCKEY_SRCBLT,&key);

	if (ret!=DD_OK) {
		dd_error(0,"dd_set_background",ret); return -1;
	}

	return 0;
}

/**
 * DBBltFast - Manual blit with transparency (BltFast compatible)
 * @sur1: Destination surface
 * @xt: X position on destination
 * @yt: Y position on destination
 * @sur2: Source surface
 * @rect: Source rectangle to copy
 *
 * Software implementation of BltFast with color key transparency.
 * Copies pixels from sur2 to sur1, skipping pixels that match the
 * transparent color (RED+BLUE = magenta). This is a fallback when
 * hardware blitting is unavailable or for special cases.
 *
 * Returns: DD_OK on success, DirectDraw error code on failure
 */
int DBBltFast(LPDIRECTDRAWSURFACE sur1,int xt,int yt,LPDIRECTDRAWSURFACE sur2,RECT *rect)
{
	unsigned short *from,*to;
	DDSURFACEDESC surface;
	int ret,ys,xs,x;

	surface.dwSize=sizeof(surface);
	ret=sur1->lpVtbl->Lock(sur1,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
		dd_error(0,"dd_get_ptr",ret); return ret;
	}
	to=surface.lpSurface;

	surface.dwSize=sizeof(surface);
	ret=sur2->lpVtbl->Lock(sur2,NULL,&surface,DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if (ret!=DD_OK) {
        sur1->lpVtbl->Unlock(sur1,NULL);
		dd_error(0,"dd_get_ptr",ret); return ret;
	}
	from=surface.lpSurface;

	from+=rect->top*MAXXOVER+rect->left;
	to+=yt*MAXX+xt;
	xs=rect->right-rect->left;
	
	for (ys=rect->bottom-rect->top; ys; ys--) {
		for (x=0; x<xs; x++,from++,to++) {
			if (*from!=RED+BLUE) *to=*from;
		}
		from+=MAXXOVER-xs;
		to+=MAXX-xs;
	}

	sur1->lpVtbl->Unlock(sur1,NULL);
	sur2->lpVtbl->Unlock(sur2,NULL);

	return DD_OK;
}

/**
 * dd_copytile - Copy a cached tile sprite to screen
 * @nr: Tile number in sprite cache
 * @x: X position on destination surface
 * @y: Y position on destination surface
 * @sur: Destination surface (usually sur2)
 * @mapcheck: If non-zero, performs additional bounds checking
 *
 * Blits a 32x32 tile from the sprite cache (suro) to the destination surface
 * using hardware acceleration (BltFast). Handles clipping for tiles partially
 * off-screen by adjusting the source rectangle. Uses color key transparency
 * so background pixels are not drawn.
 *
 * The tile cache is organized in a grid, with tile number converted to
 * X,Y coordinates: x = (nr % cachex) * TILE, y = (nr / cachex) * TILE
 *
 * Returns: 0 on success, -1 on failure, 0 if tile is completely off-screen
 */
// TODO: Modern GCC/MinGW - Replace BltFast with modern blitting
// SDL2: SDL_RenderCopy with texture
// D3D9: StretchRect or texture rendering
int dd_copytile(int nr,int x,int y,LPDIRECTDRAWSURFACE sur,int mapcheck)
{
	RECT rect;
	long ret;
	int xs=0,ys=0,xe=0,ye=0;

	if (!mapcheck) {
		if (x<-31 || y<-31 || x>=screen_width || y>=screen_height)	return 0;

		if (x<0) {
			xs=-x;    x=0;
		}
		if (y<0) {
			ys=-y; y=0;
		}

		if (x+32>=screen_width) xe=x-screen_width+32;
		if (y+32>=screen_height) ye=y-screen_height+32;
	} else {
		if (x<-31 || y<-31 || x>=screen_width || y>=screen_height)	return 0;

		if (x<0) {
			xs=-x;    x=0;
		}
		if (y<0) {
			ys=-y; y=0;
		}

		if (x+32>=screen_width) xe=x-screen_width+32;
		if (y+32>=screen_height) ye=y-screen_height+32;
	}

	rect.left=(nr%cachex)*TILE+xs;  rect.right=(nr%cachex)*TILE+TILE-xe;
	rect.top=(nr/cachex)*TILE+ys;   rect.bottom=(nr/cachex)*TILE+TILE-ye;


	//ret=DBBltFast(sur,x,y,suro,&rect);
	// TODO: Modern GCC/MinGW - Replace BltFast with SDL_RenderCopy or D3D DrawPrimitive
	ret=sur->lpVtbl->BltFast(sur,x,y,suro,&rect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);

	if (ret!=DD_OK) {
		dd_error(0,"dd_copytile",ret); return -1;
	}
	return 0;
}

/**
 * dd_alphaeffect_magic_1 - Apply magic effect with RGB555 format
 * @nr: Effect color flags (bit 0=red, bit 1=green, bit 2=blue)
 * @str: Effect strength (higher = weaker effect)
 * @xpos: World X position
 * @ypos: World Y position
 * @xoff: Screen X offset
 * @yoff: Screen Y offset
 *
 * Renders a colored glow effect in RGB555 mode. Creates a 64x64 pixel elliptical
 * glow that's brighter in the center and fades toward edges. The glow color is
 * determined by nr flags. Used for spell effects and magic animations.
 */
void dd_alphaeffect_magic_1(int nr,int str,int xpos,int ypos,int xoff,int yoff)
{
	int rx,ry;
	unsigned short *dst,d;
	int x,y,dstx,dsty,e;
	int r,g,b,r1,g1,b1,e2;

	rx=(xpos/2)+(ypos/2)-(2*16)+32+screen_tilexoff-((screen_renderdist-34)/2*32);
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+screen_tileyoff-2*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	dst=dd_get_ptr(sur2);

	dstx=rx;
	dsty=ry;

	for (y=0; y<64; y++) {
		if (dsty+y<0 || dsty+y>=screen_height) continue;
		for (x=0; x<64; x++) {
			if (dstx+x<0 || dstx+x>=screen_width) continue;

			d=dst[(dstx+x)+(dsty+y)*screen_width];

			r1=(d&0x7C00)>>10;
			g1=(d&0x03E0)>>5;
			b1=(d&0x001F);

			e=32;
			if (x<32) e-=32-x;
			if (x>31) e-=x-31;
			if (y<16) e-=16-y;
			if (y>55) e-=(y-55)*2;
			if (e<0) e=0;
			e/=max(1,str);

			e2=0;
			if (nr&1) e2+=e;
			if (nr&2) e2+=e;
			if (nr&4) e2+=e;

			r=r1-e2/2; if (r<0)	r=0;
			g=g1-e2/2; if (g<0)	g=0;
			b=b1-e2/2; if (b<0)	b=0;

			if (nr&1) {
				r=r+e;
				if (r>31) r=31;
			}
			if (nr&2) {
				g=g+e;
				if (g>31) g=31;
			}
			if (nr&4) {
				b=b+e;
				if (b>31) b=31;
			}
			dst[(dstx+x)+(dsty+y)*screen_width]=(unsigned short)((r<<10)+(g<<5)+b);
		}
	}

	dd_release_ptr(sur2);

}

/**
 * dd_alphaeffect_magic_0 - Apply magic effect with RGB565 format
 * @nr: Effect color flags (bit 0=red, bit 1=green, bit 2=blue)
 * @str: Effect strength (higher = weaker effect)
 * @xpos: World X position
 * @ypos: World Y position
 * @xoff: Screen X offset
 * @yoff: Screen Y offset
 *
 * Renders a colored glow effect in RGB565 mode. Same as magic_1 but uses
 * 565 color format with 6 bits for green channel. Creates elliptical glow
 * for magic spell visual effects.
 */
void dd_alphaeffect_magic_0(int nr,int str,int xpos,int ypos,int xoff,int yoff)
{
	int rx,ry;
	unsigned short *dst,d;
	int x,y,dstx,dsty,e;
	int r,g,b,r1,g1,b1,e2;

	rx=(xpos/2)+(ypos/2)-(2*16)+32+screen_tilexoff-((screen_renderdist-34)/2*32);
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+screen_tileyoff-2*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	dst=dd_get_ptr(sur2);

	dstx=rx;
	dsty=ry;

	for (y=0; y<64; y++) {
		if (dsty+y<0 || dsty+y>=screen_height) continue;
		for (x=0; x<64; x++) {
			if (dstx+x<0 || dstx+x>=screen_width) continue;

			d=dst[(dstx+x)+(dsty+y)*MAXX];

			r1=(d&0xF800)>>11;
			g1=(d&0x07E0)>>5;
			b1=(d&0x001F);

			e=32;
			if (x<32) e-=32-x;
			if (x>31) e-=x-31;
			if (y<16) e-=16-y;
			if (y>55) e-=(y-55)*2;
			if (e<0) e=0;
			e/=max(1,str);

			e2=0;
			if (nr&1) e2+=e;
			if (nr&2) e2+=e;
			if (nr&4) e2+=e;

			r=r1-e2/2; if (r<0)	r=0;
			g=g1-e2; if (g<0) g=0;
			b=b1-e2/2; if (b<0)	b=0;

			if (nr&1) {
				r=r+e;
				if (r>31) r=31;
			}
			if (nr&2) {
				g=g+e*2;
				if (g>63) g=63;
			}
			if (nr&4) {
				b=b+e;
				if (b>31) b=31;
			}
			dst[(dstx+x)+(dsty+y)*MAXX]=(unsigned short)((r<<11)+(g<<5)+b);
		}
	}

	dd_release_ptr(sur2);
}

/**
 * dd_alphaeffect_magic - Apply magic glow effect (dispatch function)
 * @nr: Effect color flags (bit 0=red, bit 1=green, bit 2=blue)
 * @str: Effect strength (higher = weaker effect)
 * @xpos: World X position
 * @ypos: World Y position
 * @xoff: Screen X offset
 * @yoff: Screen Y offset
 *
 * Renders a colored glow effect for magic spells. Automatically selects
 * the appropriate implementation based on the current RGB mode (555 or 565).
 * This is the public interface for magic effect rendering.
 */
void dd_alphaeffect_magic(int nr,int str,int xpos,int ypos,int xoff,int yoff)
{
	if (RGBM==1) dd_alphaeffect_magic_1(nr,str,xpos,ypos,xoff,yoff);
	else dd_alphaeffect_magic_0(nr,str,xpos,ypos,xoff,yoff);
}

/**
 * dd_getDC - Get device context for a surface
 * @sur: DirectDraw surface
 *
 * Obtains a GDI device context for drawing on a DirectDraw surface.
 * This allows using GDI drawing functions on the surface. Must be
 * released with dd_releaseDC() when done.
 *
 * Returns: Device context handle
 */
// TODO: Modern GCC/MinGW - GDI device contexts not available in SDL/modern APIs
// If GDI drawing is needed, consider rendering to SDL_Surface/SDL_Texture
// or using GPU-accelerated rendering instead
HDC dd_getDC(LPDIRECTDRAWSURFACE sur)
{
	HDC hdc;

	// TODO: Replace GetDC with SDL surface access or remove GDI dependency
	suro->lpVtbl->GetDC(sur,&hdc);

	return hdc;
}

/**
 * dd_releaseDC - Release a device context obtained from surface
 * @hdc: Device context to release
 * @sur: DirectDraw surface the DC belongs to
 *
 * Releases a device context obtained via dd_getDC(). Must be called
 * after finishing GDI operations on the surface.
 */
void dd_releaseDC(HDC hdc,LPDIRECTDRAWSURFACE sur)
{
	// TODO: Replace ReleaseDC with SDL equivalent or remove
	suro->lpVtbl->ReleaseDC(sur,hdc);
}


/**
 * dd_flip - Flip primary surface and back buffer (fullscreen)
 *
 * Swaps the front and back buffers to display the newly rendered frame.
 * Uses hardware page flipping for smooth, tear-free rendering. This is
 * only used in fullscreen mode where the back buffer is attached to the
 * primary surface. Waits for vertical sync if supported.
 */
void dd_flip(void)
{
	long ret;

	// TODO: Modern GCC/MinGW - Replace Flip with SDL_RenderPresent or D3D Present
	// SDL2: SDL_RenderPresent(renderer)
	// D3D9: device->Present(...)
	ret=sur1->lpVtbl->Flip(sur1,NULL,DDFLIP_WAIT);
	if (ret!=DD_OK) {
		dd_error(0,"dd_flip",ret);
	}
}

/**
 * dd_flip_windowed - Update screen in windowed mode
 *
 * Copies the back buffer (sur2) to the primary surface (sur1) in windowed mode.
 * Since page flipping isn't available in windowed mode, this uses blitting.
 * Adjusts for window borders and title bar (+20 pixels top, +1 pixel edges).
 * This is slower than flipping but allows the game to run in a window.
 */
void dd_flip_windowed(void)
{
	RECT rect;

	// TODO: Modern GCC/MinGW - GetWindowRect is Win32 API
	// SDL2: SDL_GetWindowSize() or SDL_GetWindowPosition()
	GetWindowRect(desk_hwnd,&rect);

	// determine bounds of window in a more sensible way !!!
	rect.bottom-=1;
	rect.left+=1;
	rect.right-=1;
	rect.top+=20;

	// TODO: Modern GCC/MinGW - Replace Blt with SDL_RenderPresent or D3D Present
	// In SDL2, windowed mode uses same SDL_RenderPresent as fullscreen
	sur1->lpVtbl->Blt(sur1,&rect,sur2,NULL,DDBLT_WAIT,NULL);
}

/**
 * dd_error - Display DirectDraw error message
 * @hwnd: Window handle (may be NULL)
 * @msg: Context message describing where error occurred
 * @err: DirectDraw error code
 *
 * Shows an error message on screen by drawing directly to the window's DC.
 * Displays both the context message and the translated error code string.
 * Suppresses SURFACELOST errors as these are expected during mode changes.
 */
void dd_error(HWND hwnd,char *msg,long err)
{
	char *ptr;
	HDC hdc;

	ptr=get_dderr(err);

	if (err!=DDERR_SURFACELOST) {
		// TODO: Modern GCC/MinGW - GetWindowDC/TextOut are GDI functions
		// Replace with SDL_ShowSimpleMessageBox or fprintf to stderr
		hdc=GetWindowDC(hwnd);
		TextOut(hdc,10,10,msg,strlen(msg));
		TextOut(hdc,10,30,ptr,strlen(ptr));
		ReleaseDC(hwnd,hdc);
	}
}


/* -------------
   Helper routines for dd_load_bitmap
   ------------- */

// Static buffer for file loading to avoid repeated allocations
static void*__mem=NULL;
static int __msize=0;

/**
 * load_file - Load entire file into memory
 * @file: Path to file to load
 *
 * Reads a complete file into a static memory buffer. The buffer grows
 * as needed and is reused across calls to minimize allocations. This is
 * used primarily for loading bitmap and sprite files.
 *
 * Returns: Pointer to file data, or NULL on failure
 */
void *load_file(char *file)
{
	int handle;
	int size;
	unsigned char *ptr;

	if (locks) xlog(0,"trying to read while locked");

	// TODO: Modern GCC/MinGW - Replace low-level file I/O with standard C
	// Use fopen/fread/fclose instead of open/read/close for portability
	// O_BINARY flag may not be needed on Unix systems
	handle=open(file,O_RDONLY|O_BINARY);  // TODO: Replace with fopen(file, "rb")
	if (handle==-1) {
		return NULL;
	} // xlog(0,"File not found: %s",strerror(errno));
	size=lseek(handle,0L,SEEK_END);  // TODO: Replace with fseek/ftell
	lseek(handle,0L,SEEK_SET);       // TODO: Replace with fseek
	if (size>__msize) {
		if (__mem) xfree(__mem);
		__msize=max(2*1024*2024,size);
		__mem=xmalloc(__msize);
	}
	ptr=__mem;
	if (!ptr) {
		xlog(0,"memory low"); return NULL;
	}
	read(handle,ptr,size);  // TODO: Replace with fread
	close(handle);          // TODO: Replace with fclose

	return ptr;
}

/**
 * unload_file - Free file data loaded by load_file
 * @ptr: Pointer returned by load_file (currently ignored)
 *
 * Placeholder for file cleanup. Currently does nothing since load_file
 * uses a static buffer that persists across calls.
 */
// TODO: Modern GCC/MinGW - #pragma argsused is Borland C++ specific
// Use GCC attribute instead: __attribute__((unused))
// Or simply remove if parameter is truly unused
#pragma argsused  // TODO: Replace with __attribute__((unused)) or remove
static void unload_file(void *ptr)
{
	//xfree(ptr);
	;
}

/**
 * dd_load_bitmap - Load a Windows bitmap file into memory
 * @name: Path to .BMP file
 * @xs: Output parameter for image width
 * @ys: Output parameter for image height
 * @sur: DirectDraw surface (for getting DC)
 *
 * Loads a bitmap file and converts it to 16-bit RGB format in memory.
 * Uses GDI functions to decode the bitmap, then extracts the raw pixel
 * data. The returned buffer must be freed with xfree().
 *
 * Returns: Pointer to 16-bit pixel data, or NULL on failure
 */
// TODO: Modern GCC/MinGW - Replace GDI bitmap loading with modern image library
// Consider using SDL_image, stb_image.h, or libpng directly
// This function uses Windows GDI which is not portable
void *dd_load_bitmap(char *name,int *xs,int *ys,LPDIRECTDRAWSURFACE sur)
{
	// TODO: Replace Windows bitmap structures with SDL_Surface or generic format
	BITMAPINFO *info;
	BITMAPFILEHEADER *file_hdr;
	HDC hdc,hdcMem;
	char *ptr,*target;
	void *bits;
	HBITMAP hbm;

	ptr=load_file(name);
	if (ptr==NULL) return NULL;

	// TODO: Modern GCC/MinGW - Replace GDI DC access with SDL_Surface loading
	// SDL_image: IMG_Load(name) returns SDL_Surface directly
	if (sur->lpVtbl->GetDC(sur,&hdc)!=DD_OK) {
		unload_file(ptr); sur->lpVtbl->ReleaseDC(sur,hdc); xlog(0,"cannot get dc"); return NULL;
	}

	file_hdr=(void*)ptr;
	bits=ptr+file_hdr->bfOffBits;
	info=(void*)&ptr[sizeof(BITMAPFILEHEADER)];

	*xs=info->bmiHeader.biWidth;
	*ys=info->bmiHeader.biHeight;

	// TODO: Replace GDI bitmap functions with SDL_LoadBMP or SDL_image
	hdcMem=CreateCompatibleDC(hdc);
	if (hdcMem==0) {
		unload_file(ptr); sur->lpVtbl->ReleaseDC(sur,hdc); xlog(0,"cannot create hdcmem"); return NULL;
	}

	hbm=CreateDIBitmap(hdc,&info->bmiHeader,CBM_INIT,bits,info,DIB_RGB_COLORS);
	if (hbm==0) {
		unload_file(ptr); sur->lpVtbl->ReleaseDC(sur,hdc); DeleteDC(hdcMem); xlog(0,"cannot create hbm (%d)",GetLastError()); return NULL;
	}

	target=xmalloc(*xs**ys*2);

	GetBitmapBits(hbm,*xs**ys*2,target);

	DeleteObject(hbm);

	DeleteDC(hdcMem);

	sur->lpVtbl->ReleaseDC(sur,hdc);

	unload_file(ptr);

	return target;
}

/* ------------------
     sprite manager
   ------------------*/

// note:
// tiles in the upper left corner of the cache area will be destroyed by loading a new
// sprite

#define MAXEFFECT	1024

// TODO: Modern GCC/MinGW - GetTickCount() is Windows-specific
// Cross-platform alternatives:
// - SDL_GetTicks() in SDL2
// - clock_gettime() on POSIX systems
// - std::chrono in C++11+
#define current_tick GetTickCount()  // TODO: Replace with SDL_GetTicks() or cross-platform timer

// TODO: Modern GCC/MinGW - MEMORYSTATUS is deprecated
// Use MEMORYSTATUSEX instead for 64-bit support
// Or use cross-platform alternatives (sysconf, sysctl, /proc/meminfo)
static MEMORYSTATUS memstat;  // TODO: Replace with MEMORYSTATUSEX

struct sprtab {
	unsigned short *image;			// null means not loaded	
	unsigned char *alpha;			// null means no alpha information present
	unsigned short *cache;
	unsigned char xs;			// in tiles
	unsigned char ys;			// in tiles
	unsigned short alphacnt;
	unsigned int ticker;
	unsigned short avgcol;
};

struct cachetab {
	unsigned int sprite;
	unsigned int ticker;
	unsigned int effect;
	unsigned int visible;
};

static struct sprtab *sprtab=NULL;
static struct cachetab *cachetab=NULL;

int dd_cache_hit=0,dd_cache_miss=0;

void free_2nd_cache(void)
{
	int n,m,old=0,tmp,t;

	t=GetTickCount();

	while (usedmem+256*1024>maxmem) {
		for (n=0; n<MAXSPRITE; n++) {
			if (!sprtab[n].image) continue;
			if (!sprtab[n].ticker) continue;
			tmp=t-sprtab[n].ticker;
			if (tmp>old) old=tmp;
		}
		old-=old/3;
		for (n=0; n<MAXSPRITE; n++) {
			if (!sprtab[n].image) continue;
			if (!sprtab[n].ticker) continue;
			tmp=t-sprtab[n].ticker;

			if (tmp>old) {
				for (m=0; m<MAXEFFECT*sprtab[n].xs*sprtab[n].ys; m++) {
					tmp=sprtab[n].cache[m];
					if (cachetab[tmp].sprite) {
						usedvid--;
						cachetab[tmp].sprite=0;
						cachetab[tmp].ticker=0;
						cachetab[tmp].effect=0;
						cachetab[tmp].visible=0;
					}
				}
				xfree(sprtab[n].cache); blockgc++;
				xfree(sprtab[n].image); blockgc++;

				if (sprtab[n].alpha) { free(sprtab[n].alpha); }

				sprtab[n].image=NULL;
				sprtab[n].alpha=NULL;
				sprtab[n].alphacnt=0;
				sprtab[n].ticker=0;
				sprtab[n].avgcol=0;
			}

		}
	}
}

unsigned short avgcolor(unsigned short *ptr,int xs,int ys)
{
	int size=xs*ys,n;
	int r=0,g=0,b=0,c=0;

	for (n=0; n<size; n++) {
		switch (RGBM) {
			case 0:
				if (ptr[n]!=0xf81f) {
					r+=((ptr[n]&0xF800)>>11);
					g+=((ptr[n]&0x07E0)>>5);
					b+=((ptr[n]&0x001F));
					c++;
				}
				break;
			case 1:
				if (ptr[n]!=0x7c1f) {
					r+=((ptr[n]&0x7C00)>>10);
					g+=((ptr[n]&0x03E0)>>5);
					b+=((ptr[n]&0x001F));
					c++;
				}
				break;
			case 2:
				if (ptr[n]!=0xf81f) {
					b+=((ptr[n]&0x7C00)>>10);
					g+=((ptr[n]&0x03E0)>>5);
					r+=((ptr[n]&0x001F));
					c++;
				}
				break;
		}
	}
	if (c) {
		r/=c;
		g/=c;
		b/=c;
	}

	switch (RGBM) {
		case 0: return(unsigned short)((r<<11)+(g<<5)+b);
		case 1: return(unsigned short)((r<<10)+(g<<5)+b);
		case 2: return(unsigned short)((b<<11)+(g<<5)+r);
		default: return 42;
	}
}

int get_avgcol(int nr)
{
	return sprtab[nr].avgcol;
}

void *conv_load(int nr,int *xs,int *ys);

extern char path[];

void dd_load_sprite(int nr)
{
	int xs,ys,alphacnt=0;
	unsigned short *optr=NULL;
	unsigned char *alpha=NULL;
	char buf[80];
	FILE *fp;

	if (usedmem>maxmem) free_2nd_cache();

	if (!optr) {	// try to load from single png file
		sprintf(buf,"%sgfx\\%05d.png",path,nr);
		fp=fopen(buf,"rb");
		if (fp) {
			optr=dd_load_png(fp,&xs,&ys,&alpha,&alphacnt);
			fclose(fp);
		}
	}	
        if (!optr) {	// try to load from pnglib
		fp=load_pnglib(nr);
		if (fp) optr=dd_load_png(fp,&xs,&ys,&alpha,&alphacnt);
	} 
        if (!optr) {	// try to load from single bmp file
	    sprintf(buf,"%sgfx\\%05d.bmp",path,nr);
	    optr=dd_load_bitmap(buf,&xs,&ys,suro);
	}
	if (!optr) {	// try to load from bmplib
		optr=conv_load(nr,&xs,&ys);
	}
	if (!optr) {	// not found!
		//sprintf(buf,"Sprite %d is missing",nr);
		//xlog(0,buf);
		fp=load_pnglib(35);
		if (fp) optr=dd_load_png(fp,&xs,&ys,&alpha,&alphacnt);
		if (!optr) optr=conv_load(35,&xs,&ys);
		//return;
	}

	sprtab[nr].image=optr;
	sprtab[nr].alpha=alpha;
	sprtab[nr].alphacnt=(unsigned short)alphacnt;
	sprtab[nr].xs=(unsigned char)(xs/TILE);
	sprtab[nr].ys=(unsigned char)(ys/TILE);
	sprtab[nr].avgcol=avgcolor(optr,xs,ys);

	sprtab[nr].cache=xcalloc(MAXEFFECT*sizeof(short),sprtab[nr].xs*sprtab[nr].ys);
}

/**
 * dd_invalidate_cache - Clear the entire sprite tile cache
 *
 * Resets all cached sprite tiles, forcing them to be reloaded from disk
 * on next use. Clears sprite numbers, effects, visibility, and usage tracking.
 * This is called when switching display modes or when memory needs to be freed.
 */
void dd_invalidate_cache(void)
{
	int n;

	for (n=0; n<MAXCACHE; n++) {
		cachetab[n].ticker=0;
		cachetab[n].sprite=0;
		cachetab[n].effect=0;
		cachetab[n].visible=0;
	}
	usedvid=0;
}

/**
 * dd_init_sprites - Initialize the sprite management system
 *
 * Sets up the sprite and cache tables for managing game sprites. Allocates
 * memory for sprite metadata (sprtab) and tile cache tracking (cachetab).
 * Determines max memory usage based on available physical RAM. Sets the
 * transparent color key to magenta (RED+BLUE).
 */
void dd_init_sprites(void)
{
	// TODO: Modern GCC/MinGW - GlobalMemoryStatus is deprecated
	// Use GlobalMemoryStatusEx for 64-bit memory sizes
	// Or use cross-platform alternatives (sysconf on Linux)
	GlobalMemoryStatus(&memstat);
	maxmem=max(8192*1024,memstat.dwTotalPhys-8192*1024);
	maxvid=MAXCACHE;

	if (!sprtab) sprtab=xcalloc(MAXSPRITE,sizeof(struct sprtab));
	if (!cachetab) cachetab=xcalloc(MAXCACHE,sizeof(struct cachetab));

	dd_set_background(RED+BLUE);
}

#define LEFFECT	(gamma-4880)   //120

int gamma=5000;

/**
 * do_effect - Apply visual effects to a pixel color
 * @val: Original 16-bit RGB pixel value
 * @effect: Bit flags controlling effects and brightness
 * @seed1: Random seed 1 (unused)
 * @seed2: Random seed 2 (unused)
 * @sprite: Sprite number (unused)
 *
 * Applies various visual effects to a single pixel:
 * - Bits 0-15: Brightness adjustment (gamma correction)
 * - Bit 16 (16): Red border highlight
 * - Bit 32 (32): Green border highlight
 * - Bit 64 (64): Invisibility (darken heavily)
 * - Bit 128 (128): Greyscale conversion
 * - Bit 256 (256): Infrared vision (red tint)
 * - Bit 512 (512): Underwater tint (blue)
 *
 * Handles both RGB565 and RGB555 formats based on RGBM setting.
 *
 * Returns: Modified pixel color value
 */
// TODO: Modern GCC/MinGW - Replace #pragma argsused with GCC __attribute__
#pragma argsused  // TODO: Use __attribute__((unused)) for seed1, seed2, sprite
unsigned short do_effect(unsigned short val,int effect,int seed1,int seed2,int sprite)
{
	int r,g,b,invis=0,tmp,grey=0,infra=0,water=0,bloody=0,red=0,green=0;

	if (effect&16)   { effect-=  16; red   =1; } //red border
	if (effect&32)   { effect-=  32; green =1; } //green border
	if (effect&64)   { effect-=  64; invis =1; } //blackened out
	if (effect&128)  { effect-= 128; grey  =1; } //grey scale
	if (effect&256)  { effect-= 256; infra =1; } //grey scale
	if (effect&512)  { effect-= 512; water =1; } //grey scale
//	if (effect&1024) { effect-=1024; bloody=1; } //grey scale

	switch (RGBM) {
		case 0:
			if (val!=0xf81f) {
				r=(val&0xf800)>>11;
				g=(val&0x07e0)>>5;
				b=(val&0x001f);

				/* !!!!!!!!  color channel stuff
				if (sprite>2000+30*1024) {
					int r2=0,g2=0,b2=0;

					r2+=r*300/256;
					g2+=r*300/256;
					b2+=r*300/256; 

					r2+=g*120/512;	// 224
					g2+=g*50/512;	// 167
					b2+=g*20/512;	// 144

					r2+=b*256/256;
					g2+=b*  0/256;
					b2+=b*  0/256;

                                        r=r2;
					g=g2*2;
					b=b2;

					if (r>31) r=31;
					if (g>63) g=63;
					if (b>31) b=31;

				} !!!!!! */

				if (effect) {
					r=(r*LEFFECT)/(effect*effect+LEFFECT);
					g=(g*LEFFECT)/(effect*effect+LEFFECT);
					b=(b*LEFFECT)/(effect*effect+LEFFECT);
				}

				if (grey) {
					tmp=(r+(g/2)+b)/6;
					r=tmp;
					b=tmp;
					g=tmp*2;
				}

				if (infra) {
					tmp=(r+(g/2)+b)/3;
					r=tmp;
					b=0;
					g=0;
				}

				if (water) {
					tmp=(r+(g/2)+b)/2;
					if (tmp>31)	tmp=31;
					r=(r+tmp)/3;
					b=(b+tmp); if (b>31) b=31;
					g=((g+tmp)/3)*2;
				}

				if (gamma!=5000) {
					r=r*gamma/5000; if (r>31) r=31;
					g=g*gamma/5000; if (g>63) g=63;
					b=b*gamma/5000; if (b>31) b=31;
				}

				//if (red) { r+=15; if (r>31) r=31; }

				if (red) { 
					r*=2; if (r>31) r=31;
					g*=2; if (g>63) g=63;
					b*=2; if (b>31) b=31;					
				}

				if (green) { g+=31; if (g>63) g=63; }

				if (invis) {
					r=g=b=0;
				}

                                return(unsigned short)((r<<11)+(g<<5)+b);
			} else return val;

		case 1:
			if (val!=0x7C1F) {
				r=(val&0x7C00)>>10;
				g=(val&0x03E0)>>5;
				b=(val&0x001F);

				if (effect) {
					r=(r*LEFFECT)/(effect*effect+LEFFECT);
					g=(g*LEFFECT)/(effect*effect+LEFFECT);
					b=(b*LEFFECT)/(effect*effect+LEFFECT);
				}

				if (grey) {
					tmp=(r+g+b)/6;
					r=tmp;
					b=tmp;
					g=tmp;
				}

				if (infra) {
					tmp=(r+g+b)/3;
					r=tmp;
					b=0;
					g=0;
				}

				if (water) {
					tmp=(r+g+b)/2;
					if (tmp>31)	tmp=31;
					r=(r+tmp)/3;
					b=(b+tmp); if (b>31) b=31;
					g=(g+tmp)/3;
				}

				if (gamma!=5000) {
					r=r*gamma/5000; if (r>31) r=31;
					g=g*gamma/5000; if (g>31) g=31;
					b=b*gamma/5000; if (b>31) b=31;
				}

				if (red) { r+=15; if (r>31) r=31; }
				if (green) { g+=15; if (g>31) g=31; }

				if (invis) {
					r=g=b=0;
				}

				return(unsigned short)((r<<10)+(g<<5)+b);
			} else return val;

		case 2:
			if (val!=0xf81f) {
				b=(val&0xf800)>>11;
				g=(val&0x07e0)>>5;
				r=(val&0x001f);

				if (effect) {
					r=(r*LEFFECT)/(effect*effect+LEFFECT);
					g=(g*LEFFECT)/(effect*effect+LEFFECT);
					b=(b*LEFFECT)/(effect*effect+LEFFECT);
				}

				if (invis) {
					r=g=b=0;
				}

				if (grey) {
					tmp=(r+(g/2)+b)/6;
					r=tmp;
					b=tmp;
					g=tmp*2;
				}

				if (infra) {
					tmp=(r+(g/2)+b)/3;
					r=tmp;
					b=0;
					g=0;
				}

				if (water) {
					tmp=(r+g+b)/2;
					if (tmp>31)	tmp=31;
					r=(r+tmp)/3;
					b=(b+tmp); if (b>31) b=31;
					g=(g+tmp)/3;
				}

				if (gamma!=5000) {
					r=r*gamma/5000; if (r>31) r=31;
					g=g*gamma/5000; if (g>63) g=63;
					b=b*gamma/5000; if (b>31) b=31;
				}

				return(unsigned short)((b<<11)+(g<<5)+r);
			} else return val;

		default:    return val;
	}
}

int tile2cache(int tile,int sprite,int xpos,int ypos,int xs,int effect)
{
	int cx,cy,sx,sy,x,y,visible=0;
	unsigned short *screen,*image;
	extern int ticker;

	screen=dd_get_ptr(suro);
	if (!screen) return 0;

	image=sprtab[sprite].image;

	cx=(tile%cachex)*TILE;
	cy=(tile/cachex)*TILE;

	sx=xpos*32;
	sy=ypos*32;

	screen+=cx+cy*MAXXOVER;
	image+=sx+sy*xs*32;

	for (y=0; y<32; y++) {
		for (x=0; x<32; x++) {
			if ((*screen=do_effect(*image,effect,x,y,sprite))!=background)	visible=1;
			image++; screen++;
		}
		screen+=MAXXOVER-32;
		image+=(xs-1)*32;
	}


	dd_release_ptr(suro);

	return visible;
}

int gettile(unsigned int sprite,unsigned int effect,int x,int y,int xs)
{
	int n,old=0;
	unsigned int nr,oldrec=0;

	nr=(sprite<<16)+x+y*xs;

	n=sprtab[sprite].cache[(x+y*xs)*MAXEFFECT+effect];
	if (cachetab[n].sprite==nr && cachetab[n].effect==effect) {
		cachetab[n].ticker=0;
		dd_cache_hit++;
		if (!cachetab[n].visible) {
			invisible++; return -1;
		}
		return n;
	}

	for (n=0; n<MAXCACHE; n++) {
		if (cachetab[n].sprite==0) {
			oldrec=0x7fffffff; old=n;
		}
		if (cachetab[n].ticker>oldrec) {
			oldrec=cachetab[n].ticker; old=n;
		}
		cachetab[n].ticker++;
	}
	dd_cache_miss++;

	if (oldrec==0x7fffffff)	usedvid++;

	cachetab[old].visible=tile2cache(old,sprite,x,y,xs,effect);
	cachetab[old].sprite=nr;
	cachetab[old].ticker=random(1024)+24;
	cachetab[old].effect=effect;
	sprtab[sprite].cache[(x+y*xs)*MAXEFFECT+effect]=(short)old;

	if (!cachetab[old].visible) {
		invisible++; return -1;
	}

	return old;
}

void copysprite(int nr,int effect,int xpos,int ypos,int xoff,int yoff)
{
	unsigned int x,y,xs,ys,n,rx,ry;

	if (nr==0) return;

        // image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		ypos += 4;
	}

	xs=sprtab[nr].xs;
	ys=sprtab[nr].ys;
	sprtab[nr].ticker=current_tick;

	rx=(xpos/2)+(ypos/2)-(xs*16)+32+screen_tilexoff-((screen_renderdist-34)/2*32);
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+screen_tileyoff-ys*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	for (y=0; y<ys; y++) {
		for (x=0; x<xs; x++) {
			n=gettile(nr,effect,x,y,xs);
			if (n==-1) continue;
			dd_copytile(n,rx+x*32,ry+y*32,sur2,1);
		}
	}
	if (sprtab[nr].alpha) display_alpha(sprtab[nr].alpha,sprtab[nr].alphacnt,rx,ry,effect);
}

void copyspritex(int nr,int xpos,int ypos,int effect)
{
	unsigned int x,y,xs,ys,n;

	if (nr==0) return;

	// image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		ypos += 4;
	}

	xs=sprtab[nr].xs;
	ys=sprtab[nr].ys;
	sprtab[nr].ticker=current_tick;

	for (y=0; y<ys; y++) {
		for (x=0; x<xs; x++) {
			n=gettile(nr,effect,x,y,xs);
			if (n==-1) continue;
			dd_copytile(n,xpos+x*32,ypos+y*32,sur2,0);
		}
	}
	if (sprtab[nr].alpha) display_alpha(sprtab[nr].alpha,sprtab[nr].alphacnt,xpos,ypos,effect);
}

void dd_savescreen(void)
{
	unsigned char *ptr;
	int n;
	HBITMAP hbm;
	HDC hdc;
	unsigned char *pix;
	static int lasttime=0;

	if (time(NULL)-lasttime<10) {
		xlog(0,"One moment please, I'm counting my bits!");
		return;
	}

	lasttime=time(NULL);

	if (!OpenClipboard(desk_hwnd)) return;
	if (!EmptyClipboard()) return;

	sur1->lpVtbl->GetDC(sur1,&hdc);

	hbm=CreateCompatibleBitmap(hdc,screen_width,screen_height);
	if (hbm==NULL) return;

	sur1->lpVtbl->ReleaseDC(sur1,hdc);

	pix=xmalloc(screen_width*screen_height*2);
	ptr=dd_get_ptr(sur1);
	if (!ptr) return;

	for (n=0; n<screen_width*screen_height*2; n++)	pix[n]=ptr[n];

	dd_release_ptr(sur1);

	if (!SetBitmapBits(hbm,screen_width*screen_height*2,pix)) return;
	if (!SetClipboardData(CF_BITMAP,hbm)) return;
	if (!CloseClipboard()) return;

	xfree(pix);

	xlog(2,"Placed screenshot in clipboard.");
}

/**
 * dd_gputc - Draw a single character in game world coordinates
 * @xpos: World X position
 * @ypos: World Y position
 * @font: Font index (0-9)
 * @c: Character to draw (ASCII)
 *
 * Renders a single 6x9 pixel character from a font sprite to the screen.
 * Converts world coordinates to screen coordinates using isometric projection.
 * Performs clipping against screen bounds. Uses transparency for anti-aliased
 * text. Font sprites are at sprite indices 18100-18109.
 */
void dd_gputc(int xpos,int ypos,int font,int c)
{
	unsigned short *fptr,*tptr;
	int x,y,off,nr;

	if (c>127) return;
	c-=32;

	if (font<0 || font>9) return;
	nr=18100+font;

	// image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		ypos += 4;
	}

	off=c*6;
	tptr=dd_get_ptr(sur2);
	if (!tptr) return;
	tptr+=xpos+ypos*MAXX;
	fptr=sprtab[nr].image+off+576;
	sprtab[nr].ticker=current_tick;

	for (y=0; y<9; y++) {
        if (ypos + y < 0 || ypos + y >= screen_height) continue;

        for (x=0; x<6; x++,tptr++,fptr++) {
            if (xpos + x < 0 || xpos + x >= screen_width) continue;

            if (*fptr!=background)
                *tptr=*fptr;
        }
        tptr+=MAXX-6;
        fptr+=576-6;
    }

	dd_release_ptr(sur2);

	return;
}

/**
 * dd_gputtext - Draw text string in game world coordinates
 * @xpos: World X position
 * @ypos: World Y position
 * @font: Font index (0-9)
 * @text: Null-terminated string to draw
 * @xoff: Additional X screen offset
 * @yoff: Additional Y screen offset
 *
 * Renders a text string by converting world coordinates to screen coordinates
 * and drawing each character. Centers the text horizontally around the world
 * position. Stops drawing if text goes off-screen.
 */
void dd_gputtext(int xpos,int ypos,int font,char *text,int xoff,int yoff)
{
	int rx,ry,len;

	len=strlen(text);

	rx=(xpos/2)+(ypos/2)+32-((len*5)/2)+screen_tilexoff-((screen_renderdist-34)/2*32);
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+screen_tileyoff-64;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	while (*text) {
		if (rx<0 || rx>screen_width-7 || ry<0 || ry>screen_height-10) return;
		dd_gputc(rx,ry,font,*text);
		text++; rx+=6;
	}
}

/**
 * dd_putc - Draw a single character at screen coordinates
 * @xpos: Screen X position
 * @ypos: Screen Y position
 * @font: Font index (0-9)
 * @c: Character to draw (ASCII)
 *
 * Draws a 6x9 pixel character directly at screen coordinates without
 * coordinate conversion. No clipping is performed. Used for UI elements
 * and HUD text.
 */
void dd_putc(int xpos,int ypos,char font,int c)
{
	unsigned short *fptr,*tptr;
	int x,y,off,nr;

	if (c>127) return;
	c-=32;

	if (font<0 || font>9) return;

	nr=18100+font;

	// image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		ypos += 4;
	}

	off=c*6;
	tptr=dd_get_ptr(sur2);
	if (!tptr) return;
	tptr+=xpos+ypos*MAXX;
	fptr=sprtab[nr].image+off+576;
	sprtab[nr].ticker=current_tick;

	for (y=0; y<9; y++) {
		for (x=0; x<6; x++,tptr++,fptr++) {
			if (*fptr!=background)
				*tptr=*fptr;
		}
		tptr+=MAXX-6;
		fptr+=576-6;
	}

	dd_release_ptr(sur2);

	return;
}

/**
 * dd_puttext - Draw text string at screen coordinates
 * @x: Screen X position
 * @y: Screen Y position
 * @font: Font index (0-9)
 * @text: Null-terminated string to draw
 *
 * Renders text horizontally at the specified screen position. Each character
 * is 6 pixels wide. No clipping or coordinate conversion.
 */
void dd_puttext(int x,int y,int font,char *text)
{
	while (*text) {
		dd_putc(x,y,(char)font,*text);
		text++; x+=6;
	}
}

/**
 * dd_xputtext - Draw formatted text at screen coordinates
 * @x: Screen X position
 * @y: Screen Y position
 * @font: Font index (0-9)
 * @format: Printf-style format string
 * @...: Variable arguments for format string
 *
 * Renders formatted text using printf-style formatting. Useful for
 * displaying numbers and dynamic text in the UI.
 */
void dd_xputtext(int x,int y,int font,char *format,...)
{
	va_list args;
	char buf[1024];

	va_start(args,format);
	// TODO: Modern GCC/MinGW - vsprintf is unsafe, replace with vsnprintf
	// vsnprintf(buf, sizeof(buf), format, args) to prevent buffer overflow
	vsprintf(buf,format,args);
	dd_puttext(x,y,font,buf);
	va_end(args);
}

int dd_isvisible(void)
{
	long ret;

	ret=sur1->lpVtbl->IsLost(sur1);
	if (ret!=DD_OK) {
		sur1->lpVtbl->Restore(sur1);
	}
	ret=sur2->lpVtbl->IsLost(sur2);
	if (ret!=DD_OK) {
		sur2->lpVtbl->Restore(sur2);
	}
	ret=suro->lpVtbl->IsLost(suro);
	if (ret!=DD_OK) {
		suro->lpVtbl->Restore(suro); dd_invalidate_cache();
	}

	ret=sur1->lpVtbl->IsLost(sur1);
	if (ret!=DD_OK)	return 0;
	ret=sur2->lpVtbl->IsLost(sur2);
	if (ret!=DD_OK)	return 0;
	ret=suro->lpVtbl->IsLost(suro);
	if (ret!=DD_OK)	return 0;

	return 1;
}

void dd_show_map(unsigned short *src,int xo,int yo,int magnify)
{
	unsigned short *dst;
	int x,y,d,s,wnd_xtra,p;

	wnd_xtra=0;
	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		wnd_xtra=4;
	}

	dst=dd_get_ptr(sur2);
	if (!dst) return;
	
	// Draw the mini-map (magnify x magnify)
	for (x=0; x<128; x++) 
	{
		d=(x+wnd_xtra+582)*MAXX+6;
		s=((x/magnify)+xo)*MAPX_MAX+yo;
		p=s*magnify;
		for (y=0; y<128; y++) 
		{
			dst[d++]=src[s];
			p++; s=p/magnify;
		}
	}
	
	dd_release_ptr(sur2);
}

void do_rgb8_effect(int *r1,int *g1,int *b1,int effect)
{
    int r,g,b,invis=0,tmp,grey=0,infra=0,water=0,bloody=0,red=0,green=0;

    if (effect&16) { effect-=16; red=1; } //red border
    if (effect&32) { effect-=32; green=1; } //green border
    if (effect&64) { effect-=64; invis=1; } //blackened out
    if (effect&128) { effect-=128; grey=1; } //grey scale
    if (effect&256) { effect-=256; infra=1; } //infrared
    if (effect&512) { effect-=512; water=1; } //under water
//    if (effect&1024) { effect-=1024; bloody=1; } //bloody
		
    r=*r1;
    g=*g1;
    b=*b1;

    if (effect) {
	    r=(r*LEFFECT)/(effect*effect+LEFFECT);
	    g=(g*LEFFECT)/(effect*effect+LEFFECT);
	    b=(b*LEFFECT)/(effect*effect+LEFFECT);
    }

    if (grey) {
	    tmp=(r+g+b)/6;
	    r=tmp;
	    b=tmp;
	    g=tmp;
    }
	
    if (infra) {
	    tmp=(r+g+b)/3;
	    r=tmp;
	    b=0;
	    g=0;
    }

    if (water) {
	    tmp=(r+g+b)/2;
	    if (tmp>31)	tmp=31;
	    r=(r+tmp)/3;
	    b=(b+tmp); if (b>31) b=31;
	    g=(g+tmp)/3;
    }

    if (gamma!=5000) {
	    r=r*gamma/5000; if (r>255) r=255;
	    g=g*gamma/5000; if (g>255) g=255;
	    b=b*gamma/5000; if (b>255) b=255;
    }

    //if (red) { r+=128; if (r>255) r=255; }
    if (red) { 
	    r*=2; if (r>255) r=255; 
	    g*=2; if (g>255) g=255; 
	    b*=2; if (b>255) b=255; 
	}
    if (green) { g+=128; if (g>255) g=255; }

    if (invis) {
	    r=g=b=0;
    }

    *r1=r;
    *g1=g;
    *b1=b;
}


void display_alpha(unsigned char *alpha,int alphacnt,int xf,int yf,int effect)
{
    unsigned short *dst,val;
    int x,y,r1,g1,b1,r2,b2,g2,a1,a2,r,g,b,n,pos;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		yf += 4;
	}

    dst=dd_get_ptr(sur2);
    if (!dst) return;

    for (n=0; n<alphacnt; n+=6) {	
	x=alpha[n+0];
	y=alpha[n+1];
        r1=alpha[n+2];
	g1=alpha[n+3];
	b1=alpha[n+4];
	a1=alpha[n+5];
	a2=256-a1;

	do_rgb8_effect(&r1,&g1,&b1,effect);

	pos=x+xf+y*MAXX+yf*MAXX;
	val=dst[pos];

	if (RGBM==0) {
	    r2=(val&RED)>>8; 
	    g2=(val&GREEN)>>3;
	    b2=(val&BLUE)<<3;
	} else {
	    r2=(val&RED)>>7;
	    g2=(val&GREEN)>>2;
	    b2=(val&BLUE)<<3;
	}	

	r=(r1*a1+r2*a2)/256;
	g=(g1*a1+g2*a2)/256;
	b=(b1*a1+b2*a2)/256;

        if (RGBM==0) dst[pos]=(unsigned short)(((r<<8)&RED)+((g<<3)&GREEN)+((b>>3)&BLUE));
	else dst[pos]=(unsigned short)(((r<<7)&RED)+((g<<2)&GREEN)+((b>>3)&BLUE));
	
    }

    dd_release_ptr(sur2);
}

// displays a png in RGBA 8 bit format (memory blocks, not the silly rows-concept)
void display_png(unsigned char *png,int xf,int yf,int xs,int ys)
{
    unsigned short *dst;
    int x,y,r1,g1,b1,r2,b2,g2,a1,a2,r,g,b;

	if (screen_windowed == 1) {
		// Adjust windowed mode drawing height
		yf += 4;
	}

    dst=dd_get_ptr(sur2);
    if (!dst) return;

    for (y=0; y<ys; y++) {
	for (x=0; x<xs; x++) {
	    r1=png[x*4+y*xs*4+0];
	    g1=png[x*4+y*xs*4+1];
	    b1=png[x*4+y*xs*4+2];
	    a1=png[x*4+y*xs*4+3];
	    a2=256-a1;

            if (RGBM==0) {
		r2=(dst[x+xf+y*MAXX+yf*MAXX]&RED)>>8;
		g2=(dst[x+xf+y*MAXX+yf*MAXX]&GREEN)>>3;
		b2=(dst[x+xf+y*MAXX+yf*MAXX]&BLUE)<<3;
	    } else {
		r2=(dst[x+xf+y*MAXX+yf*MAXX]&RED)>>7;
		g2=(dst[x+xf+y*MAXX+yf*MAXX]&GREEN)>>2;
		b2=(dst[x+xf+y*MAXX+yf*MAXX]&BLUE)<<3;
	    }	

	    r=(r1*a1+r2*a2)/256;
	    g=(g1*a1+g2*a2)/256;
	    b=(b1*a1+b2*a2)/256;

            if (RGBM==0) dst[x+xf+y*MAXX+yf*MAXX]=(unsigned short)(((r<<8)&RED)+((g<<3)&GREEN)+((b>>3)&BLUE));
	    else dst[x+xf+y*MAXX+yf*MAXX]=(unsigned short)(((r<<7)&RED)+((g<<2)&GREEN)+((b>>3)&BLUE));
	}
    }

    dd_release_ptr(sur2);
}

// reads a PNG file and transforms the data into a 16 bit RGB data (returned as ptr to memory block)
// the alpha information is stored in alpha_ptr & alphacnt_ptr in the format x,y,r,g,b,a for all
// pixels actually having alpha information.
// pixel with alpha!=255 (i.e. partially or fully transparent pixels) are marked as the transparent
// color in the 16 bit RGB data to allow blitting them with FastBLT.
// the alpha information may be used, but doesnt need to - it just looks better...
// TODO: Modern GCC/MinGW - Ensure libpng is properly linked
// MinGW: link with -lpng -lz
// May need to install libpng-dev or libpng16-dev package
// Consider using SDL_image (IMG_LoadPNG_RW) for easier cross-platform integration
// Verify libpng version compatibility
unsigned short *dd_load_png(FILE *fp,int *xs,int *ys,unsigned char **alpha_ptr,int *alphacnt_ptr)
{
    int x,y,n,m,alphacnt=0,alphamax=0,noalpha=0,mul=4,tmp,r,g,b,a;
    unsigned char **row,*alpha=NULL;
    unsigned short *bmp;
    // TODO: Modern GCC/MinGW - Check libpng API compatibility with version being used
    png_structp png_ptr;
    png_infop info_ptr;
    png_infop end_info;

    png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if (!png_ptr) {
	xlog(0,"create read"); return 0;
    }

    info_ptr=png_create_info_struct(png_ptr);
    if (!info_ptr) {
	png_destroy_read_struct(&png_ptr,(png_infopp)NULL,(png_infopp)NULL); xlog(0,"create info1"); return 0;
    }

    end_info=png_create_info_struct(png_ptr);
    if (!end_info) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"create info2"); return 0;
    }

    png_init_io(png_ptr,fp);

    png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_PACKING,NULL);

    row=png_get_rows(png_ptr,info_ptr);
    if (!row) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"read row"); return 0;
    }

    x=png_get_image_width(png_ptr,info_ptr);
    y=png_get_image_height(png_ptr,info_ptr);

    tmp=png_get_rowbytes(png_ptr,info_ptr);
    if (tmp==x*3) { noalpha=1; mul=3; }
    else if (tmp!=x*4) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"rowbytes!=x*4 (%d != %d)",tmp,x*4); return 0;
    }
    if (png_get_bit_depth(png_ptr,info_ptr)!=8) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"bit depth!=8"); return 0;
    }
    if (png_get_channels(png_ptr, info_ptr)!=mul) {
	png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL); xlog(0,"channels!=mul"); return 0;
    }

    bmp=xmalloc(x*y*2);

    for (n=0; n<y; n++) {
	for (m=0; m<x; m++) {
	    if (noalpha) {
		if (row[n][m*mul]>250 && row[n][m*mul+1]<5 && row[n][m*mul+2]>250) a=0;
		else a=255;
	    } else a=row[n][m*mul+3];

	    // this gets the rgb info from the png and scales the colors to their original value. 3dsmax will mix the image
	    // color and the background color AND supply alpha information, which is just plain wrong...
	    if (a) {
		    r=min(255,row[n][m*mul+0]*255/a);
		    g=min(255,row[n][m*mul+1]*255/a);
		    b=min(255,row[n][m*mul+2]*255/a);
	    } else {
		    r=row[n][m*mul+0];
		    g=row[n][m*mul+1];
		    b=row[n][m*mul+2];
	    }

	    if ((do_alpha==0 && a<64) ||
		(do_alpha==1 && a<192) ||
		(do_alpha==2 && a<255))	bmp[m+n*x]=(unsigned short)(RED|BLUE);
	    else if (RGBM==0) bmp[m+n*x]=(unsigned short)(((r<<8)&RED)+((g<<3)&GREEN)+((b>>3)&BLUE));
	    else bmp[m+n*x]=(unsigned short)(((r<<7)&RED)+((g<<2)&GREEN)+((b>>3)&BLUE));

	    if (!noalpha && ((do_alpha==1 && a>63 && a<192) || (do_alpha==2 && a!=0 && a!=255))) {
		if (alphacnt>=alphamax-8) {
		    alphamax+=32;
		    alpha=realloc(alpha,alphamax);
		}
		*(unsigned char*)(alpha+alphacnt)=(unsigned char)m; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=(unsigned char)n; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=r; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=g; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=b; alphacnt+=1;
		*(unsigned char*)(alpha+alphacnt)=a; alphacnt+=1;

		alphapix++;
	    } else fullpix++;
	}
    }

    if (alpha) alpha=realloc(alpha,alphacnt);

    png_destroy_read_struct(&png_ptr,&info_ptr,(png_infopp)NULL);

    *alpha_ptr=alpha;
    *alphacnt_ptr=alphacnt;
    *xs=x;
    *ys=y;

    return bmp;
}

void dd_invalidate_alpha(void)
{
    int n,tmp,m;

    for (n=0; n<MAXSPRITE; n++) {
	if (!sprtab[n].image) continue;

	for (m=0; m<MAXEFFECT*sprtab[n].xs*sprtab[n].ys; m++) {
            tmp=sprtab[n].cache[m];
		if (cachetab[tmp].sprite) {
			usedvid--;
			cachetab[tmp].sprite=0;
			cachetab[tmp].ticker=0;
			cachetab[tmp].effect=0;
			cachetab[tmp].visible=0;
		}
	}
	xfree(sprtab[n].cache); blockgc++;
	xfree(sprtab[n].image); blockgc++;

	if (sprtab[n].alpha) { free(sprtab[n].alpha); }

	sprtab[n].image=NULL;
	sprtab[n].alpha=NULL;
	sprtab[n].alphacnt=0;
	sprtab[n].ticker=0;
	sprtab[n].avgcol=0;
    }
}

static unsigned char *shadowmap;

/**
 * dd_shadow_clear - Reset the shadow rendering map
 *
 * Clears the shadow bitmap used to track which pixels already have shadows.
 * This prevents overlapping shadows from being rendered twice on the same
 * pixel. Should be called once per frame before rendering shadows.
 */
void dd_shadow_clear(void)
{
	// TODO: Modern GCC/MinGW - calloc parameters are (count, size)
	// Current usage: calloc(MAXX, MAXY) should be calloc(MAXX*MAXY, 1)
	// or calloc(MAXX, MAXY) if MAXY is the element size
	if (!shadowmap) shadowmap=calloc(MAXX,MAXY);  // TODO: Verify parameter order
	else memset(shadowmap,0,MAXX*MAXY);
}

/**
 * dd_shadow - Render a sprite's shadow
 * @nr: Sprite number to cast shadow from
 * @xpos: World X position
 * @ypos: World Y position
 * @xoff: Screen X offset
 * @yoff: Screen Y offset
 *
 * Draws a darkened version of a sprite to create a shadow effect. The shadow
 * is offset downward from the sprite position. Uses the shadowmap to avoid
 * drawing the same pixel twice. Only works for certain sprite ranges (game
 * characters and objects). Shadows are rendered by halving pixel brightness.
 */
void dd_shadow(int nr,int xpos,int ypos,int xoff,int yoff)
{
	unsigned int x,y,xs,ys,rx,ry,m,p,v,ok=0,disp,swap=0,n;
	unsigned short *dst,*src;

	if (nr==0) return;

	if (nr>=2000 && nr<16336) { ok=1; disp=14; }
	if (nr>17360) { ok=1; disp=14; }

	if (!ok) return;

        // image loaded?
	if (!sprtab[nr].image) dd_load_sprite(nr);
	if (!sprtab[nr].image) return;

	xs=sprtab[nr].xs;
	ys=sprtab[nr].ys;
	sprtab[nr].ticker=current_tick;

	rx=(xpos/2)+(ypos/2)-(xs*16)+32+screen_tilexoff-((screen_renderdist-34)/2*32);
	if (xpos<0 && (xpos&1))	rx--;
	if (ypos<0 && (ypos&1))	rx--;
	ry=(xpos/4)-(ypos/4)+screen_tileyoff-ys*32;
	if (xpos<0 && (xpos&3))	ry--;
	if (ypos<0 && (ypos&3))	ry++;

	rx+=xoff;
	ry+=yoff;

	ry+=ys*32-disp;
	
	if (rx>screen_width || ry>screen_height || rx<0 || ry<0) return;

	dst=dd_get_ptr(sur2);
	if (!dst) return;

	src=sprtab[nr].image;

	for (y=ry+ys*8,m=0; y>ry; y--,m+=xs*96) {
                for (x=rx; x<rx+xs*32; x++,m++) {
                        if (src[m]!=(RED|BLUE)) {
                                
				if (swap==1) p=x+y*MAXX+(int)((x-rx)*0.885)*MAXX;
				else if (swap==2) p=x+y*MAXX-(int)((x-rx)*0.885)*MAXX;
                                else p=x+y*MAXX;

				if (p>=(unsigned)(MAXX*MAXY)) continue;
				if (shadowmap[p]) continue;

				v=dst[p];
				v>>=1;
				if (RGBM==0) v&=(0x7800|0x03E0|0x000F); else v&=(0x3C00|0x01E0|0x000F);
				dst[p]=v;

				shadowmap[p]=1;
			}
		}
	}

	if (sprtab[nr].alpha) {	
		for (n=0; n<(unsigned)sprtab[nr].alphacnt; n+=6) {	
			
			x=sprtab[nr].alpha[n+0]+rx;
			y=ry+ys*8-(sprtab[nr].alpha[n+1]/4);

			if (swap==1) p=x+y*MAXX+(int)((x-rx)*0.885)*MAXX;
			else if (swap==2) p=x+y*MAXX-(int)((x-rx)*0.885)*MAXX;
			else p=x+y*MAXX;
	
			if (p>=(unsigned)(MAXX*MAXY)) continue;
			if (shadowmap[p]) continue;
	
			v=dst[p];
			v>>=1;
			if (RGBM==0) v&=(0x7800|0x03E0|0x000F); else v&=(0x3C00|0x01E0|0x000F);
                        dst[p]=v;
	
			shadowmap[p]=1;
		}
	}
        
	dd_release_ptr(sur2);
}

/*
	switch(nr) {
		case 106:	disp=12; ok=1; break;		// big pillar

		case 460:
		case 461:
		case 462:
		case 463:
		case 464:       disp=12; ok=1; break;		// big candle on

		case 594:
		case 595:	disp=12; ok=1; break;		// big street lamp

		case 792:	disp=12; ok=1; break;		// shrine

		case 946:	disp=12; ok=1; break;		// thick pillar

		case 968:	disp=8; ok=1; swap=2; break;	// iron bars
		case 976:	disp=22; ok=1; swap=1; break;	// iron bars
		case 978:	disp=22; ok=1; swap=1; break;	// iron bars with door

		case 997:	disp=30; ok=1; swap=1; break;	// iron door open
		case 998:	disp=8; ok=1; swap=2; break;	// iron door closed

		case 1102:	
		case 1104:
		case 1106:
		case 1108:	disp=11; ok=1; break;		// golden candle on

		case 1026:	
		case 1028:
		case 1030:
		case 1032:	disp=11; ok=1; break;		// blue candle on

		case 1112:	disp=10; ok=1; break;		// tree
		case 1114:	disp=10; ok=1; break;		// tree

		case 1154:      disp=8; ok=1; break;		// red flower
		case 1155:	disp=8; ok=1; break;		// small green plant
		case 1184:	disp=8; ok=1; break;		// red flower

		case 1160:	disp=12; ok=1; break;		// marble pillar

		case 1162:      disp=11; ok=1; break;		// golden candle off

		case 1166:      disp=11; ok=1; break;		// blue candle off

		case 1186:	disp=24; ok=1; swap=1; break;	// street sign
		case 1190:	disp=24; ok=1; swap=1; break;	// street sign
		case 1194:	disp=0; ok=1; swap=2; break;	// street sign
		case 1196:	disp=0; ok=1; swap=2; break;	// street sign

		case 1234:
		case 1235:	disp=8; ok=1; break;		// purple flower
		case 1236:	disp=8; ok=1; break;		// small green plant
		case 1238:	disp=12; ok=1; break;		// tree
		case 1240:	disp=12; ok=1; break;		// tree

		case 1625:	disp=12; ok=1; break;		// titan helmet

		case 16510:					// fire in pot
		case 16511:					// fire in pot
		case 16512:					// fire in pot
		case 16513:					// fire in pot
		case 16514:	disp=8; ok=1; break;		// fire in pot
				
		case 16969:	disp=8; ok=1; break;		// trashcan
		

				
	}
*/
