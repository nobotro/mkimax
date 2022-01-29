#include <stdio.h>
#include "cam.h"
#include "libuvc/libuvc.h"
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdlib.h> 

#define SERVER_IP "127.0.0.1"

#define SERVER_PORT 1327
#define MAX_CHUNK_SIZE 50000

int sockfd;


//serveraddr structs  is used for video streaming
struct sockaddr_in servaddr[4];  

//udp ports which is used for video streamin
int video_stream_ports[]={1327,1328,1329,1330};


void sg(uint8_t* data,int size,int device_id)
{

 printf("dev id : %d size: %d framee:%d\n",device_id,size, *data);
    

  while(size)
    {

    int packet_size=(size / MAX_CHUNK_SIZE >0) ?  MAX_CHUNK_SIZE : size;
    size-=packet_size;
    sendto(sockfd, data, packet_size,NULL,&servaddr[device_id],sizeof(servaddr[device_id])); 
    data+=packet_size;

    
   

  	 }

  	   //exit(EXIT_FAILURE); 	  
                      
}

void main()
{
	 //initilize udp client
     if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  


  for(uint8_t i=0;i<4;i++)
  {

    memset(&servaddr[i], 0, sizeof(servaddr[i])); 
      
    // Filling server information 
    servaddr[i].sin_family = AF_INET; 
    servaddr[i].sin_port = htons(video_stream_ports[i]); 
    servaddr[i].sin_addr.s_addr = inet_addr(SERVER_IP);
      


  }
  
    
    

	handle_cams(&sg);


 	while(1)sleep(1000);
   


}
