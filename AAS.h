#ifndef __AAS_H__
#define __AAS_H__

/* define---------------滤波相关宏定义----------------------*/
#define DATA_LENGTH 30
#define A           0.98

#define		WIDE		60  // 
#define		NEARBY	10	// 

#define		MAX			80  //
#define		MIN			10	//
typedef unsigned int DataType;
typedef struct 
{
	unsigned int position;     
	DataType data[WIDE];	    
}DATA;


void DataInit(DATA * d, DataType InitValue)	//?á11ì?êy?Y3?ê??ˉ,???á11ì??úμ??a??3?ê??ˉ?a???¨?μ￡?ò?°??a0 
{
	int i = 0;
 	d->position = 0;
 	for(i = 0 ; i < WIDE ; i++)
		d->data[i] = InitValue;
} 	

void UpdateWindowData(DATA * d, DataType NewData)  //?????ˉ′°?úà???μ?êy?Y??DD?üD? 
{
	int i = 0, pos = 0;
	double sum = 0.0 ,residue  = 0.0; 
	DataType Average = 0; 				//???ù?μ 
	
	NewData = NewData*60;//￡?±?á?×?′ó?μ?é?üê?1000￡?3?ò?60??oó￡?×?′ó?μ60000￡??a?ù×?￡??éò????ó???è￡???éùoó?ú??êy????′òà′μ??ó2?
		
	if((NewData<(MAX*60)) && (NewData>(MIN*60)))  	   //êy?Yo?·¨ 
	{
		d->data[d->position] =  NewData ;   
	}
	else								         //êy?Y2?o?·¨￡?ó|???ùìˉμ?Nearby??êy?Yμ±?D 
	{
		sum = NewData;					   //NewData2?ó????ù 
		
		pos = d->position ;			   //??μ±?°????è?3?à′ 
		for(i = 0 ; i < NEARBY-1 ; i++)      //?ó???ù?μ 
		{
			pos	= (pos+1)%WIDE;         //è·±￡êy×é??±ê2?ò?3? 
			
			sum = sum + d->data[pos];  	
			
		}
		Average = (DataType)(sum/(NEARBY*1.0));  //?ó???ù?μ
	 
		d->data[d->position] = Average;                //×?D?ò???êy?Yó????ù?μ′úì? 
		
		residue  = (NewData*1.0 - Average*1.0 )/(NEARBY-1.0);//ê￡óàμ?êy?Y￡?Dèòa??ìˉμ???oónearby??êy?Yμ±?D 
			
		pos = d->position ;			       			   //??μ±?°????è?3?à′ 	
		for(i = 0 ; i < NEARBY-1 ; i++) 	     //?ùìˉêy?Y 
		{
			pos	= (pos + 1)%WIDE;  			         //è·±￡êy×é??±ê2?ò?3? 
			
			if((d->data[pos]*1.0 + residue ) <= 0.0) //è·±￡êy?Y2?ò?3?	
			{
				d->data[pos] = 0 ;  	
			} 
			else
			{
				d->data[pos] = (DataType)(d->data[pos]*1.0 + residue ) ;   //êμ??êy?Y?ùìˉ 
			}
		}
	} 

	d->position = (d->position+1)%WIDE;  //????????oóò? 	
}


DataType GetDUSTReading(DATA * d)            //??è??á11ì??úêy?Yμ????ù?μ  μ￥??￡oug/M3
{
	int i = 0;
	DataType sum = 0;
	for(i = 0; i < WIDE ; i++)
	{
		sum += d->data[i];
	}
	
	return (sum/(WIDE*60.0));  //μ￥??￡oug/m3
}

//RC滤波
float RC_Filter(float curentData,float lastData)
{
  return ((1-A) * curentData + A * lastData);
}
#endif
