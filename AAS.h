#ifndef __AAS_H__
#define __AAS_H__

/* define---------------�˲���غ궨��----------------------*/
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


void DataInit(DATA * d, DataType InitValue)	//?��11��?��y?Y3?��??��,???��11��??����??a??3?��??��?a???��?�̡�?��?��??a0 
{
	int i = 0;
 	d->position = 0;
 	for(i = 0 ; i < WIDE ; i++)
		d->data[i] = InitValue;
} 	

void UpdateWindowData(DATA * d, DataType NewData)  //?????�����?����???��?��y?Y??DD?��D? 
{
	int i = 0, pos = 0;
	double sum = 0.0 ,residue  = 0.0; 
	DataType Average = 0; 				//???��?�� 
	
	NewData = NewData*60;//��?��?��?��?�䨮?��?��?����?1000��?3?��?60??o����?��?�䨮?��60000��??a?����?��??����????��???����???����o��?��??��y????�䨰�����??��2?
		
	if((NewData<(MAX*60)) && (NewData>(MIN*60)))  	   //��y?Yo?���� 
	{
		d->data[d->position] =  NewData ;   
	}
	else								         //��y?Y2?o?������?��|???��������?Nearby??��y?Y�̡�?D 
	{
		sum = NewData;					   //NewData2?��????�� 
		
		pos = d->position ;			   //??�̡�?��????��?3?���� 
		for(i = 0 ; i < NEARBY-1 ; i++)      //?��???��?�� 
		{
			pos	= (pos+1)%WIDE;         //�������꨺y����??����2?��?3? 
			
			sum = sum + d->data[pos];  	
			
		}
		Average = (DataType)(sum/(NEARBY*1.0));  //?��???��?��
	 
		d->data[d->position] = Average;                //��?D?��???��y?Y��????��?�̡䨲��? 
		
		residue  = (NewData*1.0 - Average*1.0 )/(NEARBY-1.0);//���ꨮ����?��y?Y��?D����a??������???o��nearby??��y?Y�̡�?D 
			
		pos = d->position ;			       			   //??�̡�?��????��?3?���� 	
		for(i = 0 ; i < NEARBY-1 ; i++) 	     //?��������y?Y 
		{
			pos	= (pos + 1)%WIDE;  			         //�������꨺y����??����2?��?3? 
			
			if((d->data[pos]*1.0 + residue ) <= 0.0) //�������꨺y?Y2?��?3?	
			{
				d->data[pos] = 0 ;  	
			} 
			else
			{
				d->data[pos] = (DataType)(d->data[pos]*1.0 + residue ) ;   //����??��y?Y?������ 
			}
		}
	} 

	d->position = (d->position+1)%WIDE;  //????????o����? 	
}


DataType GetDUSTReading(DATA * d)            //??��??��11��??����y?Y��????��?��  �̣�??��oug/M3
{
	int i = 0;
	DataType sum = 0;
	for(i = 0; i < WIDE ; i++)
	{
		sum += d->data[i];
	}
	
	return (sum/(WIDE*60.0));  //�̣�??��oug/m3
}

//RC�˲�
float RC_Filter(float curentData,float lastData)
{
  return ((1-A) * curentData + A * lastData);
}
#endif
