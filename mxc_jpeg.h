/*
 * =====================================================================================
 *
 *       Filename:  mxc_jpeg.h
 *
 *    Description:  header file of imx6q jpeg decoder
 *
 *        Version:  1.0
 *        Created:  2014年03月13日 17时11分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yuan Li (Vital), kylel@wicresoft.com
 *   Organization:  WicreBox Dept.
 *
 * =====================================================================================
 */

#ifndef  __MXC_JPEG_H__
#define  __MXC_JPEG_H__

#include  <stdio.h>
#include  <string.h>
#include <linux/fb.h>
#include "vpu_lib.h"
#include "vpu_io.h" 
#include  <stdlib.h>

extern int vpu_test_dbg_level;

/*#define dprintf(level, fmt, arg...)     if (vpu_test_dbg_level >= level) \*/
/*printf("[DEBUG]\t%s:%d " fmt, __FILE__, __LINE__, ## arg)*/

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
#define STREAM_FILL_SIZE	0x40000
#define STREAM_READ_SIZE	(512 * 8)
#define STREAM_END_SIZE		0
#define PS_SAVE_SIZE		0x080000


enum {
    MODE420 = 0,
    MODE422 = 1,
    MODE224 = 2,
    MODE444 = 3,
    MODE400 = 4
};

struct frame_buf {
	int addrY;
	int addrCb;
	int addrCr;
	int strideY;
	int strideC;
	int mvColBuf;
	vpu_mem_desc desc;
};

struct fb_dev {
	int fd_fb;
	void *fb;
	int fb_size;
};

struct mxc_g2d {
	void *handle;
	struct fb_dev g2d_fb[2];
};

struct decode
{
	DecHandle handle;
	PhysicalAddress phy_bsbuf_addr;
	Uint32 virt_bsbuf_addr;
	Uint32 src_bsbuf_addr;
	Uint32 srcsize;

	int mjpg_fmt;
	int picwidth;
	int picheight;
	int loff;
	int toff;
	int stride;
	int lastPicWidth;
	int lastPicHeight;
	int jpg_pad_w;
	int jpg_pad_h;
	int minfbcount;
	int regfbcount;

	FrameBuffer *fb;
	struct frame_buf **pfbpool;
	struct vpu_mem_desc mem_desc;
};

int jdisp_jpegdec_init();
void jdisp_jpegdec_uninit();
int jdisp_jpegdec_open();
int jdisp_jpegdec_close();
int jdisp_jpeg_decode(Uint32 src_bsbuf_addr, Uint32 srcsize, int loff, int toff);

struct frame_buf *framebuf_alloc(int format, int strideY, int height);
void framebuf_free(struct frame_buf *fb);

int jdisp_g2d_init();
void jdisp_g2d_uninit();
int jdisp_g2d_convert(struct decode *dec);
int jdisp_g2d_rgbcopy(Uint32 src_virt_addr, Uint32 srcsize,
			int width, int height, int loff, int toff);
int jdisp_g2d_cursor(struct fb_cursor *cursor);
int jdisp_g2d_clear_cursor(struct fb_cursor *cursor);

#endif  /*__MXC_JPEG_H__*/
