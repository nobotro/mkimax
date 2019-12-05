#include "libuvc/libuvc.h"
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <x264.h>
#include <libswscale/swscale.h>

 
#define FRAME_WIDTH 320
#define FRAME_HEIGHT 240
//
void* ptr;



void cb(uvc_frame_t *frame, int devid) {


   
  uvc_frame_t *rgb;
 uvc_error_t ret;
   /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
  rgb = uvc_allocate_frame(frame->width * frame->height * 3);

  // /* Do the BGR conversion */
   switch(frame->frame_format)
   {
       case UVC_FRAME_FORMAT_YUYV:
         uvc_yuyv2rgb(frame, rgb);
         break;
       case UVC_FRAME_FORMAT_MJPEG:
         uvc_mjpeg2rgb(frame, rgb);
         break;
      
      default:
         return UVC_ERROR_NOT_SUPPORTED;
   }

x264_picture_t pic_in, pic_out;
 
x264_param_t param;
x264_param_default_preset(&param, "medium", "zerolatency");
 
param.i_threads = 1;
  param.i_width = frame->width;
  param.i_height = frame->height;
  param.i_fps_num = 30;
  param.i_fps_den = 1;
  param.i_keyint_max = 30;
  param.b_intra_refresh = 1;
   param.rc.i_qp_constant = 30; 
 param.rc.i_qp_min = 25; 
 param.rc.i_qp_max = 35;
  param.b_repeat_headers = 1;
  param.b_annexb = 1;
  param.i_bitdepth =8;
x264_param_apply_profile(&param, "baseline");
x264_t* encoder = x264_encoder_open(&param);

 x264_picture_alloc(&pic_out, X264_CSP_I420, frame->width, frame->height);


int srcstride = frame->width*3; 
struct SwsContext* convertCtx = sws_getContext(frame->width,frame->height, AV_PIX_FMT_RGB24, frame->width,frame->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

uint8_t* data[]={(uint8_t *)rgb->data};

sws_scale(convertCtx, data, &srcstride, 0, frame->height, pic_out.img.plane, pic_out.img.i_stride);


x264_nal_t* nals;
int i_nals;
int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_out, &pic_in);
if(frame_size>0)
 ((void (*)(uint8_t * ,int,int))ptr)(nals->p_payload,frame_size,devid);


    x264_picture_clean(&pic_out);
 x264_encoder_close(encoder);
 sws_freeContext(convertCtx);
     
 uvc_free_frame(rgb);


 
 

}




void  handle_cams(void* frame_send_func)
{ 
  ptr=frame_send_func;

  uvc_context_t *ctx;
  uvc_device_t ***devs;
  uvc_device_handle_t *devh;
  uvc_stream_ctrl_t ctrl;
  uvc_error_t res;

  int devindex=0;

  //0x18ec, 0x3399,
  if((res = uvc_init(&ctx, NULL))<0)
  {
    
      perror("uvc init failed"); 
      exit(EXIT_FAILURE); 
  }

  if((res = uvc_get_device_list(ctx, &devs))<0)
  {
        perror("uvc find device failed"); 
        exit(EXIT_FAILURE); 
  }


while(devs[devindex]!=NULL){
   
    if((res = uvc_open(devs[devindex], &devh))<0)
    {

            perror("uvc open device failed"); 
            exit(EXIT_FAILURE); 

    }
uvc_print_diag(devh, stderr);
   
    if((res = uvc_get_stream_ctrl_format_size(devh, &ctrl,UVC_FRAME_FORMAT_ANY,FRAME_WIDTH,FRAME_HEIGHT, 30))<0)
    {  
        perror("uvc configure stream failed"); 
        exit(EXIT_FAILURE); 
    }

    if((res = uvc_start_streaming(devh, &ctrl, &cb, devindex, 0))<0)
    {
        perror("uvc cstart stream failed"); 
        exit(EXIT_FAILURE); 

    }
devindex++;
  }  


 

   


}