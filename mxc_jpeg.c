/*
 * =====================================================================================
 *
 *       Filename:  mxc_jpeg.c
 *
 *    Description:  implement of imx6q jpeg decoder
 *
 *        Version:  1.0
 *        Created:  2014年03月13日 17时15分39秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yuan Li (Vital), kylel@wicresoft.com
 *   Organization:  WicreBox Dept.
 *
 * =====================================================================================
 */

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include  "mxc_jpeg.h"

struct decode *dec;
int vpu_test_dbg_level = 2;

int
mxc_vpu_init()
{ 
	RetCode ret;
	vpu_versioninfo ver;
	
	ret = vpu_Init(NULL);
	if (ret) {
		err_msg("VPU init failure, ret:%d\n", ret);
		return -1;
	}

	ret = vpu_GetVersionInfo(&ver);
	if (ret) {
		err_msg("Cannot get version info, ret:%d\n", ret);
		vpu_UnInit();
		return -1;
	}

	info_msg("VPU firmware version: %d.%d.%d_r%d\n", ver.fw_major, ver.fw_minor,
						ver.fw_release, ver.fw_code);
	info_msg("VPU library version: %d.%d.%d\n", ver.lib_major, ver.lib_minor,
						ver.lib_release);

	return 0;
}

void
mxc_vpu_uninit()
{
	vpu_UnInit();
	return;
}

int
mxc_decode_init()
{
	RetCode ret;

	if (dec != NULL) {
		warn_msg("Already allocate decode structure.\n");
	  	return 0;
	} 

	dec = (struct decode *)calloc(1, sizeof(struct decode));
	if (dec == NULL) {
		err_msg("Failed to allocate decode structure.\n");
		return -1;
	}
	memset(dec, 0, sizeof(struct decode));

	dec->mem_desc.size = STREAM_BUF_SIZE;
	ret = IOGetPhyMem(&dec->mem_desc);
	if (ret) {
		err_msg("Unable to obtain physical mem, ret:%d\n", ret);
		free(dec);
		return -1;
	}

	dec->virt_bsbuf_addr = IOGetVirtMem(&dec->mem_desc);
	if (dec->virt_bsbuf_addr <= 0) {
		err_msg("Unable to obtain virtual mem, ret:%d\n", ret);
		IOFreePhyMem(&dec->mem_desc);
		free(dec);
		return -1;
	}

	dec->phy_bsbuf_addr = dec->mem_desc.phy_addr;

	return 0;
}

void 
mxc_decode_uninit()
{
	if (dec == NULL) {
		warn_msg("Decode structure is not initialized.\n");
		return;
	}

	/* free physical and virtual memory  */
	IOFreeVirtMem(&dec->mem_desc);
	IOFreePhyMem(&dec->mem_desc);

	/* free decode structure */
	if (dec)
	  	free(dec);

	return;
}

int 
mxc_jpegdec_init()
{
	int err;
	err = mxc_vpu_init();
	if (err) {
		return -1;
	}

	err = mxc_decode_init();
	if (err) {
		return -1;
	}

	return 0;
}

void
mxc_jpegdec_uninit()
{
	mxc_decode_uninit();
	mxc_vpu_uninit();
	return;
}

int
mxc_jpegdec_open()
{
	RetCode ret;
	DecHandle handle = {0};
	DecOpenParam oparam = {0};
	
	if (dec == 0 || dec->phy_bsbuf_addr == 0 
				|| dec->virt_bsbuf_addr == 0) {
		err_msg("Decode structure is not initialized.\n");
		return -1;
	}

	oparam.bitstreamFormat = STD_MJPG;
	oparam.bitstreamBuffer = dec->phy_bsbuf_addr;
	oparam.bitstreamBufferSize = STREAM_BUF_SIZE;
	oparam.pBitStream = (Uint8 *)dec->virt_bsbuf_addr;
	oparam.reorderEnable = 1;
	oparam.chromaInterleave = 1;
	oparam.mjpg_thumbNailDecEnable = 0;
	oparam.mapType = LINEAR_FRAME_MAP;
	oparam.tiled2LinearEnable = 0;
	oparam.bitstreamMode = 1;
	oparam.jpgLineBufferMode = 0;
	oparam.psSaveBuffer = 0;
	oparam.psSaveBufferSize = PS_SAVE_SIZE;

	ret = vpu_DecOpen(&handle, &oparam);
	if (ret != RETCODE_SUCCESS) {
		err_msg("vpu_DecOpen failed, ret:%d\n", ret);
		return -1;
	}

	dec->handle = handle;
	return 0;
}

int
mxc_jpegdec_close()
{
	RetCode ret;

	if (dec == 0 || dec->handle == 0) {
		warn_msg("Jpeg decoder is already closed.\n");
		return 0;
	} 

	ret = vpu_DecClose(dec->handle);
	if (ret == RETCODE_FRAME_NOT_COMPLETE) {
		vpu_SWReset(dec->handle, 0);
		ret = vpu_DecClose(dec->handle);
		if (ret != RETCODE_SUCCESS) {
		  err_msg("vpu_DecClose failed, ret:%d\n", ret);
		  return -1;
		} 
	}

	return 0;
}

int
dec_fill_bsbuffer(DecHandle handle, Uint32 bs_va_startaddr,
			Uint32 bs_va_endaddr, Uint32 bs_pa_startaddr,
			Uint32 src_bs_startaddr, Uint32 srcsize)
{
	RetCode ret;
	PhysicalAddress pa_read_ptr, pa_write_ptr;
	Uint32 target_addr, space;
	int room;

	ret = vpu_DecGetBitstreamBuffer(handle, &pa_read_ptr, &pa_write_ptr,
				&space);
	if (ret != RETCODE_SUCCESS) {
		err_msg("vpu_DecGetBitstreamBuffer failed, err:%d\n", ret);
		return -1;
	}

	/* Decoder bitstream buffer is empty */
	if (space <= 0) {
		err_msg("Decoder bitstream buffer is empty\n");
	  	return -1;
	} 

	/* srcsize is zero or negative */
	if (srcsize <= 0) {
		err_msg("srcsize cannot be zero or negative, srcsize=%ld\n", srcsize);
		return -1;
	}

	/* not enough framebuffer */
	if (space < srcsize) {
		err_msg("not enough framebuffer, fb_size = %ld, src_size = %ld\n", space, srcsize);
		return -1;
	}

	/* Fill the bitstream buffer */
	target_addr = bs_va_startaddr + (pa_write_ptr - bs_pa_startaddr);
	if ((target_addr + srcsize) > bs_va_endaddr) {
		room = bs_va_endaddr - target_addr;
		memcpy((void *)target_addr, (void *)src_bs_startaddr, room);
		memcpy((void *)bs_va_startaddr, (void *)(src_bs_startaddr + room), srcsize - room);
	}
	else {
		memcpy((void *)target_addr, (void *)src_bs_startaddr, srcsize);
	}

	ret = vpu_DecUpdateBitstreamBuffer(handle, srcsize);
	if (ret != RETCODE_SUCCESS) {
		err_msg("vpu_DecUpdateBitstreamBuffer failed, ret:%d\n", ret);
		return -1;
	}

	return 0;
}

int
mxc_jpegdec_parse()
{
	DecInitialInfo initinfo = {0};
	DecHandle handle = dec->handle;
	RetCode ret;

	/* Parse bitstream and get width/height/framerate etc */
	vpu_DecSetEscSeqInit(handle, 1);
	ret = vpu_DecGetInitialInfo(handle, &initinfo);
	vpu_DecSetEscSeqInit(handle, 0);
	if (ret != RETCODE_SUCCESS) {
		err_msg("vpu_DecGetInitialInfo failed, ret:%d, errorcode:%ld\n",
					ret, initinfo.errorcode);
		return -1;
	}

	if (initinfo.streamInfoObtained) {
		dec->mjpg_fmt = initinfo.mjpg_sourceFormat;
		info_msg("MJPG SourceFormat: %d\n", initinfo.mjpg_sourceFormat);
	}

	dec->lastPicWidth = initinfo.picWidth;
	dec->lastPicHeight = initinfo.picHeight;

	info_msg("Decoder: width = %d, height = %d, frameRateRes = %lu, frameRateDiv = %lu, count = %u\n",
				initinfo.picWidth, initinfo.picHeight,
				initinfo.frameRateRes, initinfo.frameRateDiv,
				initinfo.minFrameBufferCount);

	dec->minfbcount = initinfo.minFrameBufferCount;
	dec->regfbcount = dec->minfbcount;
	info_msg("minfb %d\n", dec->minfbcount);

	dec->picwidth = ((initinfo.picWidth + 15) & ~15);
	dec->jpg_pad_w = 0;

	dec->picheight = ((initinfo.picHeight + 15) & ~15);
	dec->jpg_pad_h = 0;

	info_msg("dec->picwidth=%d, dec->picheight=%d, dec->jpg_pad_w=%d, dec->jpg_pad_h=%d\n",
				dec->picwidth, dec->picheight, dec->jpg_pad_w, dec->jpg_pad_h);

	if ((dec->picwidth == 0) || (dec->picheight == 0))
	  	return -1;

	dec->stride = dec->picwidth;
	return 0;
}

struct frame_buf *
framebuf_alloc(int format, int strideY, int height)
{
	struct frame_buf *fb;
	int err;
	int divX, divY;

	fb = (struct frame_buf *)malloc(sizeof(struct frame_buf));

	divX = (format == MODE420 || format == MODE422) ? 2 : 1;
	divY = (format == MODE420 || format == MODE224) ? 2 : 1;

	memset(&(fb->desc), 0, sizeof(vpu_mem_desc));
	fb->desc.size = (strideY * height  + strideY / divX * height / divY * 2);

	err = IOGetPhyMem(&fb->desc);
	if (err) {
		err_msg("Frame buffer allocation failure\n");
		memset(&(fb->desc), 0, sizeof(vpu_mem_desc));
		return NULL;
	}

	fb->addrY = fb->desc.phy_addr;
	fb->addrCb = fb->addrY + strideY * height;
	fb->addrCr = fb->addrCb + strideY / divX * height / divY;
	fb->strideY = strideY;
	fb->strideC =  strideY / divX;


	fb->desc.virt_uaddr = IOGetVirtMem(&(fb->desc));
	if (fb->desc.virt_uaddr <= 0) {
		IOFreePhyMem(&fb->desc);
		memset(&(fb->desc), 0, sizeof(vpu_mem_desc));
		return NULL;
	}

	return fb;
}

void
framebuf_free(struct frame_buf *fb)
{
	if (fb == NULL)
		return;

	if (fb->desc.virt_uaddr) {
		IOFreeVirtMem(&fb->desc);
	}

	if (fb->desc.phy_addr) {
		IOFreePhyMem(&fb->desc);
	}

	//memset(&(fb->desc), 0, sizeof(vpu_mem_desc));
	free(fb);
}

int
decoder_allocate_framebuffer(struct decode *dec)
{
	DecBufInfo bufinfo;
	int i, totalfb = dec->regfbcount;
	RetCode ret;
	DecHandle handle = dec->handle;
	FrameBuffer *fb;
	struct frame_buf **pfbpool;
	int stride;
	int delay = -1;

	fb = dec->fb = calloc(totalfb, sizeof(FrameBuffer));
	if (fb == NULL) {
		err_msg("Failed to allocate fb\n");
		return -1;
	}

	pfbpool = dec->pfbpool = calloc(totalfb, sizeof(struct frame_buf *));
	if (pfbpool == NULL) {
		err_msg("Failed to allocate pfbpool\n");
		free(dec->fb);
		dec->fb = NULL;
		return -1;
	}

	/* All buffers are linear */
	for (i = 0; i < totalfb; i++) {
		pfbpool[i] = framebuf_alloc(dec->mjpg_fmt, dec->stride, dec->picheight);
		if (pfbpool[i] == NULL)
			goto err;
	}

	for (i = 0; i < totalfb; i++) {
		fb[i].myIndex = i;
		fb[i].bufY = pfbpool[i]->addrY;
		fb[i].bufCb = pfbpool[i]->addrCb;
		fb[i].bufCr = pfbpool[i]->addrCr;
		if (!cpu_is_mx27()) {
			fb[i].bufMvCol = pfbpool[i]->mvColBuf;
		}
	}

	stride = ((dec->stride + 15) & ~15);

	/* User needs to fill max suported macro block value of frame as following */
	bufinfo.maxDecFrmInfo.maxMbX = dec->stride / 16;
	bufinfo.maxDecFrmInfo.maxMbY = dec->picheight / 16;
	bufinfo.maxDecFrmInfo.maxMbNum = dec->stride * dec->picheight / 256;

	/* For H.264, we can overwrite initial delay calculated from syntax.
	 * delay can be 0,1,... (in unit of frames)
	 * Set to -1 or do not call this command if you don't want to overwrite it.
	 * Take care not to set initial delay lower than reorder depth of the clip,
	 * otherwise, display will be out of order. */
	vpu_DecGiveCommand(handle, DEC_SET_FRAME_DELAY, &delay);

	ret = vpu_DecRegisterFrameBuffer(handle, fb, dec->regfbcount, stride, &bufinfo);
	if (ret != RETCODE_SUCCESS) {
		err_msg("Register frame buffer failed, ret=%d\n", ret);
		goto err;
	}

	return 0;

err:
	for (i = 0; i < totalfb; i++) {
		framebuf_free(pfbpool[i]);
	}

	free(dec->fb);
	free(dec->pfbpool);
	dec->fb = NULL;
	dec->pfbpool = NULL;
	return -1;
}

void
decoder_free_framebuffer(struct decode *dec)
{
	int i, totalfb;

	totalfb = dec->regfbcount;

	for (i = 0; i < totalfb; i++) {
		if (dec->pfbpool)
			framebuf_free(dec->pfbpool[i]);
	}

	if (dec->fb) {
		free(dec->fb);
		dec->fb = NULL;
	}
	if (dec->pfbpool) {
		free(dec->pfbpool);
		dec->pfbpool = NULL;
	}
	
	return;
}

int
mxc_jpegdec_start(struct decode *dec)
{
	DecHandle handle = dec->handle;
	int rot_angle;
	DecParam decparam = {0};
	DecOutputInfo outinfo = {0};
	RetCode ret;
	int loop_id = 0, rot_stride = 0;
	FrameBuffer *fb = dec->fb;
	int mirror;
	int err = 0;

	decparam.mjpegScaleDownRatioWidth = 0;
	decparam.mjpegScaleDownRatioHeight = 0;

	rot_angle = 0;
	vpu_DecGiveCommand(handle, SET_ROTATION_ANGLE, &rot_angle);
	mirror = 0;
	vpu_DecGiveCommand(handle, SET_MIRROR_DIRECTION, &mirror);

	rot_stride = (dec->picwidth + 15) & ~15;
	vpu_DecGiveCommand(handle, SET_ROTATOR_STRIDE, &rot_stride);
	vpu_DecGiveCommand(handle, SET_ROTATOR_OUTPUT, (void *)&fb[0]);

	err = dec_fill_bsbuffer(handle, dec->virt_bsbuf_addr,
			(dec->virt_bsbuf_addr + STREAM_BUF_SIZE),
			dec->phy_bsbuf_addr, dec->src_bsbuf_addr, dec->srcsize);
	if (err < 0) {
		err_msg("dec_fill_bsbuffer failed\n");
		return -1;
	}

	ret = vpu_DecStartOneFrame(handle, &decparam);
	if (ret != RETCODE_SUCCESS) {
		err_msg("DecStartOneFrame failed, ret=%d\n", ret);
		return -1;
	}

	loop_id = 0;
	while (vpu_IsBusy()) {
		vpu_WaitForInt(200);
		if (loop_id == 10) {
			ret = vpu_SWReset(handle, 0);
			warn_msg("vpu_SWReset in dec\n");
			return -1;
		}
		loop_id ++;
	}

	ret = vpu_DecGetOutputInfo(handle, &outinfo);
	if (ret != RETCODE_SUCCESS) {
		err_msg("vpu_DecGetOutputInfo failed %d\n", ret);
		return -1;
	}

	if (outinfo.decodingSuccess & 0x10) {
		err_msg("vpu needs more bitstream in rollback mode\n");
		return -1;
	}

	return 0;
}

int
mxc_jpeg_decode(Uint32 src_bsbuf_addr, Uint32 srcsize, int loff, int toff)
{
	int err, ret = 0;

	err = dec_fill_bsbuffer(dec->handle, dec->virt_bsbuf_addr,
				(dec->virt_bsbuf_addr + STREAM_BUF_SIZE),
				dec->phy_bsbuf_addr, src_bsbuf_addr, srcsize);

	if (err < 0)
		return -1;

	dec->src_bsbuf_addr = src_bsbuf_addr;
	dec->srcsize = srcsize;
	dec->loff = loff;
	dec->toff = toff;

	/* parse the bitstream */
	err = mxc_jpegdec_parse();
	if (err < 0)
		return -1;

	/* allocate frame buffer */
	err = decoder_allocate_framebuffer(dec);
	if (err < 0) {
		ret = -1;
	  	goto err;
	} 

	/* start decoding */
	err = mxc_jpegdec_start(dec);
	if (err < 0) {
		ret = -1;
	  	goto err;
	} 

	g2d_convert(dec);

err:
	decoder_free_framebuffer(dec);
	return ret;
}
