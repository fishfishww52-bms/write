#include "main.h"
#include "user.h"
#include "stdio.h"
 
#define curadcbase  2076
 
unsigned int  ocv = 0;
unsigned int  r = 200;    //mO
short         I = 0;     //0.01A
uint64_t  c =   (uint64_t)((uint64_t)20*(uint64_t)360000000);    //0.01A*1ms
uint64_t  soc = (uint64_t)((uint64_t)19*(uint64_t)360000000);    //0.01A*1ms

#define	 	VOLTAGE_V(x)							(x*100) 
const unsigned short OCV_25C[13] 	= {VOLTAGE_V(11.40),VOLTAGE_V(11.68),VOLTAGE_V(11.87),VOLTAGE_V(12.06),VOLTAGE_V(12.25),VOLTAGE_V(12.43),VOLTAGE_V(12.62),VOLTAGE_V(12.78),VOLTAGE_V(12.90),VOLTAGE_V(13.06),VOLTAGE_V(13.18),VOLTAGE_V(13.30),VOLTAGE_V(13.40)};
 
unsigned short getocv()
{
	unsigned short   index2 = soc*1000/c; 
	unsigned char    index = soc*10/c; 
	unsigned short   temp;
	temp = OCV_25C[index] + (OCV_25C[index+1] -OCV_25C[index])*(index2-index*100)/100;
	return temp*6;
}

unsigned short asc2dec(unsigned char asc[],unsigned char len)
{
	unsigned short dec=0;
	unsigned short pow1[]={0,1,10,100,1000};
	for(unsigned char i=0;i<len;i++)
	{
		dec += (asc[i]-0x30)*pow1[len-i];
		printf("asc  is  %d  dec is %d \r\n",asc[i],dec);
	}
	return dec;
}

void interface(void)
{  
	  if (str_cmp(rx_buffer, "DAC") == 0) 
    {
			unsigned short dec;
			dec = asc2dec(&rx_buffer[5],4) ;
			
			printf("dec  is  %d \r\n",dec);
			
			if(rx_buffer[3]=='1')
			{
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dec);
			}
			else if (rx_buffer[3]=='2')
			{
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dec);
			}
		}
		else if (str_cmp(rx_buffer, "OCV=") == 0) 
    {
			unsigned short dec;
			dec = asc2dec(&rx_buffer[4],4) ;
			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (unsigned int)(dec*3861)/10000);
			ocv = dec; 			 
		}
    else if (str_cmp(rx_buffer, "CUR=") == 0) 
    {
			unsigned short dec;
			dec = asc2dec(&rx_buffer[5],4) ;
			if (rx_buffer[4]=='-')
			{
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, curadcbase+(unsigned int)(dec*1640)/10000);
				I = -dec; 
			}
			else if (rx_buffer[4]=='+')
			{
				HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, curadcbase-(unsigned int)(dec*1640)/10000);
				I = dec; 
			}
		}
    else if (str_cmp(rx_buffer, "CAP=") == 0) 
     {
			 c=(uint64_t)((uint64_t)asc2dec(&rx_buffer[4],2)*(uint64_t)360000000);
			 soc=(uint64_t)((uint64_t)asc2dec(&rx_buffer[6],2)*(uint64_t)360000000);
			 HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (unsigned int)(getocv()*3861)/10000);
		 }
     else if (str_cmp(rx_buffer, "RES=") == 0) 
     {
			unsigned short dec;
			dec = asc2dec(&rx_buffer[4],3);
			r = dec;
		 }	 
}

void task1ms(void)
{
	unsigned short absi; 
	static int cnt;
	cnt++;
	
	if(I<-100) 
	{ 
		soc -= (-I);
		absi=-I;
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (unsigned int)((getocv()- absi*r/1000)*3861)/10000) ;
		if(cnt>2000)
		{
			cnt=0;
			unsigned short temp = soc*200/c;
			printf("soc is %d\r\n",temp);
		}
		
	}
}  
                     
      ovc1 - r*Ic = v1    
       ovc2 - r*Ic = v2
        ovc3 - r*Ic = v3 

       ovc1 - r*It = v1    
       ovc2 - r*It = v2
        ovc3 - r*It = v3 
           
   
void dacout(void)
{
		//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (unsigned int)(7912*3861)/10000);
	  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (unsigned int)(getocv()*3861)/10000);
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, curadcbase+(unsigned int)(0*1640)/10000);
}
    

  
