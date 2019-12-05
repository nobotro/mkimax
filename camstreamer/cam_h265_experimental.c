#include "libuvc/libuvc.h"
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <x265.h>
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

x265_picture *pic_in, *pic_out;
 
x265_param *param;
//x265_param_default_preset(&param, "medium", "zerolatency");
 

 
 
  printf("aiiiiiii\n");
    param=x265_param_alloc();
  x265_param_default(param);
x265_param_default_preset(param, "medium", "zerolatency");
 param->frameNumThreads=1;
    param->fpsNum = 30;
  param->fpsDenom = 4;
  param->internalBitDepth=8;
  param->bIntraRefresh=1;
  param->bRepeatHeaders = 1;
   param->rc.rateControlMode= X265_RC_ABR;
   param->rc.rfConstant = 30; 
 param->rc.qpMin = 25; 
 param->rc.qp = 32; 
 param->rc.qpMax = 35;
  param->rc.rfConstant = 28;
    param->rc.bitrate = 0;
    param->rc.qCompress = 0.9;
  param->internalCsp=X265_CSP_I420;
  param->sourceWidth=frame->width;
  param->sourceHeight=frame->height;
x265_param_apply_profile(param, "main");
//x265_param_apply_profile(&param, "baseline");
x265_encoder* encoder = x265_encoder_open(param);
printf("mmmm\n");

 pic_out=x265_picture_alloc();
 x265_picture_init(param,pic_out);
int y_size= param->sourceWidth * param->sourceHeight;
char *buff=NULL;
buff=(char *)malloc(y_size*3/2);
    pic_out->planes[0]=buff;
    pic_out->planes[1]=buff+y_size;
    pic_out->planes[2]=buff+y_size*5/4;
    pic_out->stride[0]=param->sourceWidth;
    pic_out->stride[1]=param->sourceWidth/2;
    pic_out->stride[2]=param->sourceWidth/2;

int srcstride = frame->width*3; 
struct SwsContext* convertCtx = sws_getContext(frame->width,frame->height, AV_PIX_FMT_RGB24, frame->width,frame->height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

uint8_t* data[]={(uint8_t *)rgb->data};
 printf("vvvvvvvv%d\n", *pic_out->stride);
sws_scale(convertCtx, data, &srcstride, 0, frame->height,pic_out->planes ,pic_out->stride);
 
 
x265_nal  *nals;
int i_nals;
int frame_size = x265_encoder_encode(encoder, &nals, &i_nals, pic_out, NULL);
 
 uint8_t *pl=nals->payload;
printf("zzzzz%d",nals->sizeBytes);
for(int j=0;j<i_nals;j++){
  ((void (*)(uint8_t * ,int,int))ptr)(nals[j].payload,nals[j].sizeBytes,devid);

       
    }
  


    x265_picture_free(pic_out);
 x265_encoder_close(encoder);
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