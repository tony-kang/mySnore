/************************************************
 *   Qstruct:usage.
 *   This program is use for the queue operate. 
 *   author: rong bin su <rongbinsu@21cn.com>  
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Qstruct.h"
int CheckDataCntInQ(QDef *Queue)
{   unsigned char Q;
  //  QDef *Queue;
   // Queue=(QDef *)Qaddress;
	   if(Queue->in==Queue->out)
			 return 0;
		if(Queue->in>Queue->out) 
			return Queue->in-Queue->out;
		
		Q=QBUFF_MAX_CHANNEL-Queue->out;
		
	return 	Q+= Queue->in;



}

int inputDataToQDef(unsigned char *data,int size,QDef *Queue)
{
	//QDef *Queue;
	if(size>QBUFF_DATA_MAX_SIZE)
	{
		#ifdef PRINT_ERROR
		printf("snore error ,input data size more than the buf's max size!!");
		#endif
		return -1;
	}
	//pthread_mutex_lock(&mymutex);
	//Queue=(QDef *)Qaddress;



	if(Queue->in>=Queue->out)
	{
		

		if((Queue->in!=QBUFF_MAX_CHANNEL-1))
		{

                  // Queue->buf[Queue->in]=*data;

			memcpy(Queue->buf[Queue->in],data,size);
			Queue->bufDataSize[Queue->in]=size;
			Queue->in++;
		//	pthread_mutex_unlock(&mymutex);
			return 1;
		}
		else if(Queue->out!=0)
		{

			  //Queue->buf[Queue->in]=*data;
			memcpy(Queue->buf[Queue->in],data,size);
			Queue->bufDataSize[Queue->in]=size;
			Queue->in=0;
			//pthread_mutex_unlock(&mymutex);
			return 1;
		}
		else 
		{
			#ifdef PRINT_ERROR
			PRINT_ERROR("snore add data to Q error , the Q channel is full! \n");
			#endif
			//pthread_mutex_unlock(&mymutex);
			return -2;
		}	
	}

	if((Queue->out-Queue->in)>1)
	{
		  //Queue->buf[Queue->in]=*data;
		memcpy(Queue->buf[Queue->in],data,size);
		Queue->bufDataSize[Queue->in]=size;
		Queue->in++;
		//pthread_mutex_unlock(&mymutex);
		return 1;

	}
	#ifdef PRINT_ERROR
	PRINT_ERROR("snore add data to Q error , the Q channel is full! \n");
	#endif
	//pthread_mutex_unlock(&mymutex);
	return -2;	
}

int outputDataFromQDef(unsigned char *data,QDef *Queue)
{
	//QDef *Queue;
	int returnCount;
	//pthread_mutex_lock(&mymutex);
	//Queue=(QDef *)Qaddress;

	if(Queue->in==Queue->out)
	{
		#ifdef PRINT_ERROR
		PRINT_ERROR("snore read data from Q error , the Q channel is empty! \n");
		#endif
	//	pthread_mutex_unlock(&mymutex);
		return -1;
	}
	// *data= Queue->buf[Queue->out];
	returnCount=Queue->bufDataSize[Queue->out];
	memcpy(data,Queue->buf[Queue->out],returnCount);

	Queue->bufDataSize[Queue->out]=0;
	if(Queue->out==(QBUFF_MAX_CHANNEL-1))
	   Queue->out=0;
	else Queue->out++;
	//pthread_mutex_unlock(&mymutex);
	return returnCount;
}










