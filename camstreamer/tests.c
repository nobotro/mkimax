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

 
//
void* ptr;


int fillImage(uint8_t* buffer, int width, int height, x264_picture_t* pic){
    int ret = x264_picture_alloc(pic, X264_CSP_I420, width, height);
    if (ret < 0) return ret;
    pic->img.i_plane = 3; // Y, U and V
    pic->img.i_stride[0] = width;
    // U and V planes are half the size of Y plane
    pic->img.i_stride[1] = width / 2;
    pic->img.i_stride[2] = width / 2;
    int uvsize = ((width + 1) >> 1) * ((height + 1) >> 1);
    pic->img.plane[0] = buffer; // Y Plane pointer
    pic->img.plane[1] = buffer + (width * height); // U Plane pointer
    pic->img.plane[2] = pic->img.plane[1] + uvsize; // V Plane pointer
    
    return ret;
}
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

// uint8_t* data=((uint8_t *)rgb->data);
// uint8_t* tempdata=data;
// for (int i=0;i<rgb->data_bytes;i+=3)
// {

// float R=*data;
// float G=*(data+1);
// float B=*(data+2);
// (*data)=      (0.257f * R) + (0.504 * G) + (0.098 * B) + 16;

// (*(data+1)) =  (0.439f * R) - (0.368 * G) - (0.071 * B) + 128;

// (*(data+2)) = -(0.148f * R) - (0.291 * G) + (0.439 * B) + 128;
// data+=3;
// }



x264_picture_t pic_in, pic_out;
 
x264_param_t param;
x264_param_default_preset(&param, "medium", "zerolatency");
param.i_threads = 1;
param.i_width =640;
param.i_height = 480;
param.i_fps_num = 30;
param.i_fps_den = 1;
// Intra refres:
param.i_keyint_max = 30;
param.b_intra_refresh = 1;
//Rate control:
param.rc.i_rc_method = X264_RC_CRF;
param.rc.f_rf_constant = 25;
param.rc.f_rf_constant_max = 35;
//For streaming:
param.b_repeat_headers = 1;
param.b_annexb = 1;
x264_param_apply_profile(&param, "high10");
x264_t* encoder = x264_encoder_open(&param);

 x264_picture_alloc(&pic_out, X264_CSP_I420, 640, 480);

// fillImage(rgb,640,480,&pic_in);


int srcstride = 640*3; 
struct SwsContext* convertCtx = sws_getContext(640,480, AV_PIX_FMT_RGB24, 640,480, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

uint8_t* data;

sws_scale(convertCtx, rgb, &srcstride, 0, 480, pic_out.img.plane, pic_out.img.i_stride);




  
x264_nal_t* nals;
int i_nals;
int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_out, &pic_in);
if (frame_size >= 0)
{
  printf("image size %d\n", *(nals->p_payload));

    printf("frame size %d\n", frame_size);

    
((void (*)(uint8_t * ,int,int))ptr)(nals->p_payload,frame_size,devid);



}
  //pass frame data and size to udp sender func
   
 //uvc_free_frame(rgb);
 


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
   
    if((res = uvc_get_stream_ctrl_format_size(devh, &ctrl,UVC_FRAME_FORMAT_ANY,640, 480, 30))<0)
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