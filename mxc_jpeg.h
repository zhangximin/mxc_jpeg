/*
 * =====================================================================================
 *
 *       Filename:  mxc_jpeg.h
 *
 *    Description:  header file of rk3188 jpeg decoder
 *
 *        Version:  1.0
 *        Created:  2015年04月
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Simon Cheung zhangximin@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef  __MXC_JPEG_H__
#define  __MXC_JPEG_H__

#include  <stdio.h>
#include  <string.h>
#include <linux/fb.h>
#include  <stdlib.h>
#include <inttypes.h>

#define err_msg(fmt, arg...) do { if (vpu_test_dbg_level >= 1) \
	printf("[ERR]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); \
	} while (0)
#define warn_msg(fmt, arg...) do { if (vpu_test_dbg_level >= 2)		\
	printf("[WARN]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); \
	} while (0)
#define info_msg(fmt, arg...) do { if (vpu_test_dbg_level >= 3)		\
	printf("[INFO]\t%s:%d " fmt,  __FILE__, __LINE__, ## arg); \
	} while (0)

#define STREAM_BUF_SIZE		0x2000000
#define STREAM_FILL_SIZE		0x40000
#define STREAM_READ_SIZE	(512 * 8)
#define STREAM_END_SIZE		0
#define PS_SAVE_SIZE		0x080000

struct fb_dev {
	int fd_fb;
	void *fb;
	int fb_size;
	int fb_width;
	int fb_height;
	int line_length;
};

struct mxc_g2d {
	void *handle;
	struct fb_dev g2d_fb[2];
};

 int jdisp_jpegdec_init();
 void jdisp_jpegdec_uninit();
 int jdisp_jpegdec_open();
 int jdisp_jpegdec_close();
 int jdisp_jpeg_decode(uint32_t src_bsbuf_addr, uint32_t srcsize, int loff,
 		int toff ,int width ,int height);

 int jdisp_g2d_init();
 void jdisp_g2d_uninit();
 int jdisp_g2d_rgbcopy(uint32_t src_virt_addr, uint32_t srcsize,
			int width, int height, int loff, int toff);
 int jdisp_g2d_cursor(struct fb_cursor *cursor);
 int jdisp_g2d_clear_cursor(struct fb_cursor *cursor);

#endif  /*__MXC_JPEG_H__*/
