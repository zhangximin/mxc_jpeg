//============================================================================
// Name        : rk_jpeg.cpp
// Author      : Simon Cheung
// Version     :
// Copyright   : zhangximin@gmail.com 2014~2016
// Description :
//============================================================================

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/fb.h>

#include "mxc_jpeg.h"

#include <rk_jpeg.h>

//using namespace std;

#define OVERLAY_BPP      32

int vpu_test_dbg_level = 2;

static char g_fb0_device[100] = "/dev/fb0";
static char g_fb1_device[100] = "/dev/fb1";

struct mxc_g2d *g2d = NULL;

 int jdisp_jpegdec_init() {
	return 0;
}

 void jdisp_jpegdec_uninit() {
	return;
}

 int jdisp_jpegdec_open() {
	return 0;
}

 int jdisp_jpegdec_close() {
	return 0;
}

 int jdisp_jpeg_decode(uint32_t src_bsbuf_addr, uint32_t srcsize, int loff,
		int toff ,int width ,int height) {
	 char * out_data;
	 //info_msg("jdisp_jpeg_decode:%d %d %d %d",src_bsbuf_addr,srcsize,loff,toff);
	 printf("jdisp_jpeg_decode:%d %d %d %d %d %d",src_bsbuf_addr,srcsize,loff,toff,width ,height);
	 out_data = (char *)malloc(width*height*4);
	 if(NULL == out_data){
		 printf("malloc out_data error\n");
	 }else{
	 memset(out_data,0,width*height*4);
	 hwjpeg_decoder((char*) src_bsbuf_addr,out_data,srcsize,  loff, toff,width,height);
	 jdisp_g2d_rgbcopy((uint32_t) out_data, g2d->g2d_fb[0].fb_size, width,
	 		 height, loff, toff);
	 free(out_data);
	 }
		return 0;
}

 int
 jdisp_g2d_init()
 {
	 return 0;

 	int fd_fb0 = 0, fd_fb1 = 0, fb0_size = 0, fb1_size = 0;
 	int fb_w, fb_h;
 	void *fb0, *fb1;
 	struct fb_var_screeninfo fb_var;
 	//struct mxcfb_gbl_alpha gbl_alpha;
 	//struct mxcfb_pos pos;

 	/* get screen size */
 	fd_fb0 = open(g_fb0_device, O_RDWR, 0);
 	ioctl(fd_fb0, FBIOGET_VSCREENINFO, &fb_var);

 	/* set display window */
 	fb_w = fb_var.xres;
 	fb_h = fb_var.yres;
 	//pos.x = 0;
 	//pos.y = 0;

 	/* set overlay fb */
 	fd_fb1 = open(g_fb1_device, O_RDWR, 0);
 	ioctl(fd_fb1, FBIOBLANK, FB_BLANK_UNBLANK);
 	ioctl(fd_fb1, FBIOGET_VSCREENINFO, &fb_var);
 	fb_var.xres = fb_w;
 	fb_var.yres = fb_h;
 	fb_var.xres_virtual = fb_var.xres + 32;
 	fb_var.yres_virtual = fb_var.yres;
 	fb_var.bits_per_pixel = (OVERLAY_BPP == 16 ? 16 : 32);
 	ioctl(fd_fb1, FBIOPUT_VSCREENINFO, &fb_var);

 	fb0_size = fb_var.xres * fb_var.yres * fb_var.bits_per_pixel / 8;
 	fb1_size = fb_var.xres_virtual * fb_var.yres_virtual * fb_var.bits_per_pixel / 8;
 	fb0 = (unsigned short *)mmap(0, fb0_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb0, 0);
 	memset(fb0, 0, fb0_size);
 	fb1 = (unsigned short *)mmap(0, fb1_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb1, 0);
 	memset(fb1, 0, fb1_size);

 	//ioctl(fd_fb1, MXCFB_SET_OVERLAY_POS, &pos);

 	/* set alpha */
 	//gbl_alpha.enable = 0;
 	//gbl_alpha.alpha = 0;
 	//ioctl(fd_fb1, MXCFB_SET_GBL_ALPHA, &gbl_alpha);

 	fb_var.xoffset = 0;
 	fb_var.yoffset = 0;
 	ioctl(fd_fb1, FBIOPAN_DISPLAY, fb_var);

 	g2d = (struct mxc_g2d *) malloc(sizeof(struct mxc_g2d));
 	memset(g2d, 0, sizeof(struct mxc_g2d));

 	g2d->handle = NULL;
 	g2d->g2d_fb[0].fd_fb = fd_fb0;
 	g2d->g2d_fb[0].fb = fb0;
 	g2d->g2d_fb[0].fb_size = fb0_size;
 	g2d->g2d_fb[1].fd_fb = fd_fb1;
 	g2d->g2d_fb[1].fb = fb1;
 	g2d->g2d_fb[1].fb_size = fb1_size;

 	return 0;
 }

 void jdisp_g2d_uninit() {
	 return;
	//struct mxcfb_gbl_alpha gbl_alpha;
	int i;

	/* set alpha */
	//gbl_alpha.enable = 1;
	//gbl_alpha.alpha = 0;
	//ioctl(g2d->g2d_fb[1].fd_fb, MXCFB_SET_GBL_ALPHA, &gbl_alpha);

	//ioctl(g2d->fd_fb1, FBIOBLANK, FB_BLANK_UNBLANK);
	//memset(g2d->fb1, 0, g2d->fb_size);
	for (i = 0; i < 2; i++) {
		munmap(g2d->g2d_fb[i].fb, g2d->g2d_fb[i].fb_size);
		if (g2d->g2d_fb[i].fd_fb != -1)
			close(g2d->g2d_fb[i].fd_fb);
	}
	free(g2d);

	return;
}

 int jdisp_g2d_rgbcopy(uint32_t src_virt_addr, uint32_t srcsize, int width,
		int height, int loff, int toff) {
return 0;
	// clip
			int i,x0, y0, x1, y1,x,y;
			int Width,Height;
			int BPP,Size,LineLen;
			void *Addr;

			x = loff;
			y = toff;

			Width = g2d->g2d_fb[0].fb_width;
			Height = g2d->g2d_fb[0].fb_height;

			BPP = 32;
			LineLen = g2d->g2d_fb[0].line_length;
			Size = LineLen * Height;
			Addr = g2d->g2d_fb[0].fb;

			x0 = loff;
			y0 = toff;
			x1 = x0 + width - 1;
			y1 = y0 + height - 1;
			if (x0 < 0) {
				x0 = 0;
			}
			if (x0 > Width - 1) {
				return 0;
			}
			if( x1 < 0) {
				return 0;
			}
			if (x1 > Width - 1) {
				x1 = Width - 1;
			}
			if (y0 < 0) {
				y0 = 0;
			}
			if (y0 > Height - 1) {
				return 0;
			}
			if (y1 < 0) {
				return 0;
			}
			if (y1 > Height - 1) {
				y1 = Height -1;
			}

			//copy
			int copyLineLen = (x1 + 1 - x0) * BPP / 8;
			void *DstPtr = Addr + LineLen * y0 + x0 * BPP / 8;
			void *SrcPtr = (void *)src_virt_addr + width *(y0 - y) + (x0 - x) * BPP / 8;

			for (i = y0; i <= y1; i++) {
				memcpy(DstPtr, SrcPtr, copyLineLen);
				DstPtr += LineLen;
				SrcPtr += width;
			}

	return 0;
}

 int jdisp_g2d_cursor(struct fb_cursor *cursor) {

	return 0;
}

 int jdisp_g2d_clear_cursor(struct fb_cursor *cursor) {

	return 0;
}

