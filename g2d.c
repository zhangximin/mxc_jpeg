///*
// * Copyright 2004-2014 Freescale Semiconductor, Inc.
// *
// * Copyright (c) 2006, Chips & Media.  All rights reserved.
// */
//
///*
// * The code contained herein is licensed under the GNU General Public
// * License. You may obtain a copy of the GNU General Public License
// * Version 2 or later at the following locations:
// *
// * http://www.opensource.org/licenses/gpl-license.html
// * http://www.gnu.org/copyleft/gpl.html
// */
//
//#include "g2d_driver.h"
//#include <fcntl.h>
//#include <errno.h>
//#include <sys/ioctl.h>
//#include <string.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <unistd.h>
////#include <linux/mxcfb.h>
//#include "mxc_jpeg.h"
////#include <linux/ipu.h>
//#include <sys/mman.h>
//#include <linux/fb.h>
//#include <linux/rk_fb.h>
//
//#define OVERLAY_BPP      32
//
//static char g_fb0_device[100] = "/dev/fb0";
//static char g_fb1_device[100] = "/dev/fb1";
//
//struct mxc_g2d *g2d = NULL;
//
//static void yuv444sp_to_yuv422sp(unsigned char* yuv444sp, unsigned char* yuv422sp, int width, int height)
//{
//	int i, j;
//	int y_size;
//	unsigned char* p_y1;
//	unsigned char* p_uv1;
//	unsigned char* p_y2;
//	unsigned char* p_uv2;
//
//	y_size = width * height;
//
//	p_y1 = yuv444sp;
//	p_uv1 = p_y1 + y_size;
//
//	p_y2 = yuv422sp;
//	p_uv2 = p_y2 + y_size;
//
//	memcpy(p_y2, p_y1, y_size);
//
//	for (j = 0, i = 0; j < y_size * 2; j += 4, i += 2)
//	{
//		p_uv2[i] = p_uv1[j];
//		p_uv2[i+1] = p_uv1[j+1];
//	}
//}
//
//int jdisp_g2d_clear_cursor(struct fb_cursor *cursor)
//{
//	int fb_w, fb_h, loff, toff;
//	struct fb_var_screeninfo fb_var;
//	struct fb_fix_screeninfo fb_fix;
//	struct g2d_surface dst;
//	void *handle = NULL;
//
//	if(g2d_open(&handle)) {
//		err_msg("g2d_open fail.\n");
//		return -1;
//	}
//
//	fb_w = cursor->image.width * 2;
//	fb_h = cursor->image.height * 2;
//	loff = cursor->hot.x - cursor->image.dx - cursor->image.width;
//	toff = cursor->hot.y - cursor->image.dy - cursor->image.height;
//	if (loff < 0) loff = 0;
//	if (toff < 0) toff = 0;
//
//	ioctl(g2d->g2d_fb[1].fd_fb, FBIOGET_VSCREENINFO, &fb_var);
//	ioctl(g2d->g2d_fb[1].fd_fb, FBIOGET_FSCREENINFO, &fb_fix);
//
//	dst.planes[0] = fb_fix.smem_start;
//	dst.format = (OVERLAY_BPP == 16 ? G2D_RGB565 : G2D_BGRA8888);
//	dst.left = loff;
//	dst.top = toff;
//	dst.right = fb_w + loff;
//	dst.bottom = fb_h + toff;
//	dst.stride = fb_var.xres_virtual;
//	dst.width  = fb_var.xres_virtual;
//	dst.height = fb_var.yres_virtual;
//	dst.rot    = G2D_ROTATION_0;
//	dst.clrcolor = 0;
//
//	g2d_clear(handle, &dst);
//	g2d_finish(handle);
//
//	g2d_close(handle);
//
//	return 0;
//}
//
//int jdisp_g2d_cursor(struct fb_cursor *cursor)
//{
//	int in_left, in_top, in_right, in_bottom;
//	int fb_w, fb_h, loff, toff;
//	struct fb_var_screeninfo fb_var;
//	struct fb_fix_screeninfo fb_fix;
//	struct g2d_surface src, dst;
//	int size, ret;
//	void *handle = NULL;
//	struct g2d_buf *cursor_buf = NULL;
//
//	if(g2d_open(&handle)) {
//		err_msg("g2d_open fail.\n");
//		return -1;
//	}
//
//	if (cursor->enable == 0)
//		goto err;
//
//	size = cursor->image.width * cursor->image.height * cursor->image.depth / 8;
//	cursor_buf = g2d_alloc(size, 0);
//	if (cursor_buf == 0) {
//		err_msg("g2d_alloc failed.\n");
//		goto err;
//	}
//
//	if (cursor->image.data != NULL)
//		memcpy((void *)cursor_buf->buf_vaddr, cursor->image.data, size);
//	else
//	  	goto err1;
//
//	in_right = cursor->image.width;
//	in_bottom = cursor->image.height;
//	in_left = 0;
//	in_top = 0;
//
//	fb_w = cursor->image.width;
//	fb_h = cursor->image.height;
//	loff = cursor->hot.x - cursor->image.dx;
//	toff = cursor->hot.y - cursor->image.dy;
//	if (loff < 0) loff = 0;
//	if (toff < 0) toff = 0;
//
//	ioctl(g2d->g2d_fb[1].fd_fb, FBIOGET_VSCREENINFO, &fb_var);
//	ioctl(g2d->g2d_fb[1].fd_fb, FBIOGET_FSCREENINFO, &fb_fix);
//
//	src.planes[0] = cursor_buf->buf_paddr;
//	src.format = G2D_BGRA8888;
//	src.left = in_left;
//	src.top = in_top;
//	src.right = in_right;
//	src.bottom =  in_bottom;
//	src.stride = fb_w;
//	src.width  = fb_w;
//	src.height = fb_h;
//	src.rot    = G2D_ROTATION_0;
//
//	dst.planes[0] = fb_fix.smem_start;
//	dst.format = (OVERLAY_BPP == 16 ? G2D_RGB565 : G2D_BGRA8888);
//	dst.left = loff;
//	dst.top = toff;
//	dst.right = fb_w + loff;
//	dst.bottom = fb_h + toff;
//	dst.stride = fb_var.xres_virtual;
//	dst.width  = fb_var.xres_virtual;
//	dst.height = fb_var.yres_virtual;
//	dst.rot    = G2D_ROTATION_0;
//
//	g2d_blit(handle, &src, &dst);
//	g2d_finish(handle);
//
//err1:
//	ret = g2d_free(cursor_buf);
//	if (ret) {
//		err_msg("g2d_free failed, ret = %d\n", ret);
//	}
//err:
//	g2d_close(handle);
//
//	return 0;
//}
//
//int jdisp_g2d_rgbcopy(Uint32 src_virt_addr, Uint32 srcsize,
//			int width, int height, int loff, int toff)
//{
//	int in_left, in_top, in_right, in_bottom;
//	int fb_w, fb_h;
//	struct fb_var_screeninfo fb_var;
//	struct fb_fix_screeninfo fb_fix;
//	struct g2d_surface src, dst;
//	struct g2d_buf *src_buf;
//	void *handle = NULL;
//	int ret;
//
//	if(g2d_open(&handle))
//	{
//		err_msg("g2d_open fail.\n");
//		return -1;
//	}
//
//	src_buf = g2d_alloc(srcsize, 0);
//	if (src_buf == 0) {
//		err_msg("g2d_alloc failed.\n");
//		return -1;
//	}
//
//	memcpy((void *)src_buf->buf_vaddr, (void *)src_virt_addr, srcsize);
//
//	in_right = width;
//	in_bottom = height;
//	in_left = 0;
//	in_top = 0;
//
//	ioctl(g2d->g2d_fb[0].fd_fb, FBIOGET_VSCREENINFO, &fb_var);
//	fb_w = width;
//	fb_h = height;
//
//	ioctl(g2d->g2d_fb[0].fd_fb, FBIOGET_FSCREENINFO, &fb_fix);
//
//	src.planes[0] = src_buf->buf_paddr;
//	src.format = G2D_BGRA8888;
//	src.left = in_left;
//	src.top = in_top;
//	src.right = in_right;
//	src.bottom =  in_bottom;
//	src.stride = fb_w;
//	src.width  = fb_w;
//	src.height = fb_h;
//	src.rot    = G2D_ROTATION_0;
//
//	dst.planes[0] = fb_fix.smem_start;
//	dst.format = (OVERLAY_BPP == 16 ? G2D_RGB565 : G2D_BGRA8888);
//	dst.left = loff;
//	dst.top = toff;
//	dst.right = fb_w + loff;
//	dst.bottom = fb_h + toff;
//	dst.stride = fb_var.xres;
//	dst.width  = fb_var.xres;
//	dst.height = fb_var.yres;
//	dst.rot    = G2D_ROTATION_0;
//
//	g2d_blit(handle, &src, &dst);
//	g2d_finish(handle);
//
//	ret = g2d_free(src_buf);
//	if (ret) {
//		err_msg("g2d_free failed, ret = %d\n", ret);
//	}
//
//	g2d_close(handle);
//
//	return 0;
//}
//
//int jdisp_g2d_convert(struct decode *dec)
//{
//	int fb_w, fb_h;
//	int in_right, in_bottom, in_left, in_top;
//	int sec, usec, times = 0;
//	struct timeval tdec_begin, tdec_end;
//	struct g2d_surface src,dst;
//	struct fb_var_screeninfo fb_var;
//	struct fb_fix_screeninfo fb_fix;
//	struct frame_buf *pfb_dec;
//	void *handle = NULL;
//
//	if(g2d_open(&handle))
//	{
//		err_msg("g2d_open fail.\n");
//		return -1;
//	}
//
//	switch (dec->mjpg_fmt) {
//	case MODE420:
//		src.format = G2D_NV12;
//		pfb_dec = dec->pfbpool[0];
//		break;
//	case MODE422:
//		src.format = G2D_NV16;
//		pfb_dec = dec->pfbpool[0];
//		break;
//	case MODE444:
//		pfb_dec = framebuf_alloc(MODE422, dec->stride, dec->picheight);
//		yuv444sp_to_yuv422sp((unsigned char*)dec->pfbpool[0]->desc.virt_uaddr,
//					(unsigned char*)pfb_dec->desc.virt_uaddr, dec->stride, dec->picheight);
//		src.format = G2D_NV16;
//		break;
//	default:
//	    return -1;
//	}
//
//	ioctl(g2d->g2d_fb[0].fd_fb, FBIOGET_FSCREENINFO, &fb_fix);
//
//	/*
//	 * src is yuv format, dst is rgb format.
//	 * g2d require src.planes[0] and src.planes[1] align with 64 bytes, src.stride align with 8 bytes,
//	 * dst.planes[0] align with 16 bytes, dst.stride align with 16 bytes.
//	 * since both dec->picwidth and dec->picheight have aligned with 16 bytes, dec output can meet the alignment requirement
//	 */
//
//	in_right = dec->picwidth - dec->jpg_pad_w;
//	in_bottom = dec->picheight - dec->jpg_pad_h;
//	in_left = 0;
//	in_top = 0;
//
//	ioctl(g2d->g2d_fb[0].fd_fb, FBIOGET_VSCREENINFO, &fb_var);
//	fb_w = dec->picwidth;
//	fb_h = dec->picheight;
//
//	gettimeofday(&tdec_begin, NULL);
//
//	src.planes[0] = pfb_dec->addrY;
//	src.planes[1] = pfb_dec->addrY + dec->picwidth * dec->picheight;
//	src.left = in_left;
//	src.top = in_top;
//	src.right = in_right;
//	src.bottom =  in_bottom;
//	src.stride = dec->picwidth;
//	src.width  = dec->picwidth;
//	src.height = dec->picheight;
//	src.rot    = G2D_ROTATION_0;
//
//	dst.planes[0] = fb_fix.smem_start;
//	dst.format = (OVERLAY_BPP == 16 ? G2D_RGB565 : G2D_BGRA8888);
//	dst.left = dec->loff;
//	dst.top = dec->toff;
//	dst.right = fb_w + dec->loff;
//	dst.bottom = fb_h + dec->toff;
//	dst.stride = fb_var.xres;
//	dst.width  = fb_var.xres;
//	dst.height = fb_var.yres;
//	dst.rot    = G2D_ROTATION_0;
//
//	g2d_blit(handle, &src, &dst);
//	g2d_finish(handle);
//
//	times++;
//	info_msg("x = %d, y = %d, w = %d, h = %d\n", dst.left, dst.top, dst.width, dst.height);
//
//	gettimeofday(&tdec_end, NULL);
//	sec = tdec_end.tv_sec - tdec_begin.tv_sec;
//	usec = tdec_end.tv_usec - tdec_begin.tv_usec;
//	if (usec < 0) {
//		sec--;
//		usec = usec + 1000000;
//	}
//	info_msg("g2c convert %d times take %ds %dus, %dus/time\n", times, sec, usec, (sec*1000000+usec)/times);
//
//	if (dec->mjpg_fmt == MODE444)
//		framebuf_free(pfb_dec);
//
//	g2d_close(handle);
//
//	return -1;
//}
//
//int
//jdisp_g2d_init()
//{
//	int fd_fb0 = 0, fd_fb1 = 0, fb0_size = 0, fb1_size = 0;
//	int fb_w, fb_h;
//	void *fb0, *fb1;
//	struct fb_var_screeninfo fb_var;
//	struct mxcfb_gbl_alpha gbl_alpha;
//	struct mxcfb_pos pos;
//
//	/* get screen size */
//	fd_fb0 = open(g_fb0_device, O_RDWR, 0);
//	ioctl(fd_fb0, FBIOGET_VSCREENINFO, &fb_var);
//
//	/* set display window */
//	fb_w = fb_var.xres;
//	fb_h = fb_var.yres;
//	pos.x = 0;
//	pos.y = 0;
//
//	/* set overlay fb */
//	fd_fb1 = open(g_fb1_device, O_RDWR, 0);
//	ioctl(fd_fb1, FBIOBLANK, FB_BLANK_UNBLANK);
//	ioctl(fd_fb1, FBIOGET_VSCREENINFO, &fb_var);
//	fb_var.xres = fb_w;
//	fb_var.yres = fb_h;
//	fb_var.xres_virtual = fb_var.xres + 32;
//	fb_var.yres_virtual = fb_var.yres;
//	fb_var.bits_per_pixel = (OVERLAY_BPP == 16 ? 16 : 32);
//	ioctl(fd_fb1, FBIOPUT_VSCREENINFO, &fb_var);
//
//	fb0_size = fb_var.xres * fb_var.yres * fb_var.bits_per_pixel / 8;
//	fb1_size = fb_var.xres_virtual * fb_var.yres_virtual * fb_var.bits_per_pixel / 8;
//	fb0 = (unsigned short *)mmap(0, fb0_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb0, 0);
//	memset(fb0, 0, fb0_size);
//	fb1 = (unsigned short *)mmap(0, fb1_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb1, 0);
//	memset(fb1, 0, fb1_size);
//
//	ioctl(fd_fb1, MXCFB_SET_OVERLAY_POS, &pos);
//
//	/* set alpha */
//	gbl_alpha.enable = 0;
//	gbl_alpha.alpha = 0;
//	ioctl(fd_fb1, MXCFB_SET_GBL_ALPHA, &gbl_alpha);
//
//	fb_var.xoffset = 0;
//	fb_var.yoffset = 0;
//	ioctl(fd_fb1, FBIOPAN_DISPLAY, fb_var);
//
//	g2d = (struct mxc_g2d *) malloc(sizeof(struct mxc_g2d));
//	memset(g2d, 0, sizeof(struct mxc_g2d));
//
//	g2d->handle = NULL;
//	g2d->g2d_fb[0].fd_fb = fd_fb0;
//	g2d->g2d_fb[0].fb = fb0;
//	g2d->g2d_fb[0].fb_size = fb0_size;
//	g2d->g2d_fb[1].fd_fb = fd_fb1;
//	g2d->g2d_fb[1].fb = fb1;
//	g2d->g2d_fb[1].fb_size = fb1_size;
//
//	return 0;
//}
//
//void
//jdisp_g2d_uninit()
//{
//	struct mxcfb_gbl_alpha gbl_alpha;
//	int i;
//
//	/* set alpha */
//	gbl_alpha.enable = 1;
//	gbl_alpha.alpha = 0;
//	ioctl(g2d->g2d_fb[1].fd_fb, MXCFB_SET_GBL_ALPHA, &gbl_alpha);
//
//	//ioctl(g2d->fd_fb1, FBIOBLANK, FB_BLANK_UNBLANK);
//	//memset(g2d->fb1, 0, g2d->fb_size);
//	for (i = 0; i < 2; i++) {
//		munmap(g2d->g2d_fb[i].fb, g2d->g2d_fb[i].fb_size);
//		if (g2d->g2d_fb[i].fd_fb != -1)
//			close(g2d->g2d_fb[i].fd_fb);
//	}
//	free(g2d);
//
//	return;
//}
