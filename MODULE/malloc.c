#include "malloc.h"

//�ڴ��(32�ֽڶ���)
__align(32) uint8_t membase[MEM1_MAX_SIZE];		   //�ڲ�SRAM�ڴ��

//�ڴ������
uint16_t memmapbase[MEM1_ALLOC_TABLE_SIZE];				 //�ڲ�SRAM�ڴ��MAP

//�ڴ��������	   
const uint32_t memtblsize = MEM1_ALLOC_TABLE_SIZE; //�ڴ����С
const uint32_t memblksize = MEM1_BLOCK_SIZE ;		   //�ڴ�ֿ��С
const uint32_t memsize = MEM1_MAX_SIZE;				     //�ڴ��ܴ�С

//�ڴ����������
struct _m_mallco_dev mallco_dev = {
	my_mem_init,				//�ڴ��ʼ��
	my_mem_perused,				//�ڴ�ʹ����
	membase,			        //�ڴ��
	memmapbase,                //�ڴ����״̬��
	0,		 				    //�ڴ����δ����
};

//�����ڴ�
//*des:Ŀ�ĵ�ַ
//*src:Դ��ַ
//n:��Ҫ���Ƶ��ڴ泤��(�ֽ�Ϊ��λ)
void mymemcpy(void *des,void *src,uint32_t n)  
{  
	uint8_t *xdes=des;
	uint8_t *xsrc=src; 
	while(n--)*xdes++=*xsrc++;  
}

//�����ڴ�
//*s:�ڴ��׵�ַ
//c :Ҫ���õ�ֵ
//count:��Ҫ���õ��ڴ��С(�ֽ�Ϊ��λ)
void mymemset(void *s,uint8_t c,uint32_t count)  
{  
    uint8_t *xs = s;  
    while(count--)*xs++=c;  
}	

//�ڴ������ʼ��  
//memx:�����ڴ��
void my_mem_init(void)  
{  
  mymemset(mallco_dev.memmap, 0,memtblsize*2);//�ڴ�״̬����������  
	mymemset(mallco_dev.membase, 0,memsize);	//�ڴ��������������  
	mallco_dev.memrdy=1;								//�ڴ������ʼ��OK  
} 

//��ȡ�ڴ�ʹ����
//memx:�����ڴ��
//����ֵ:ʹ����(0~100)
uint8_t my_mem_perused(void)  
{  
    uint32_t used=0;  
    uint32_t i;  
    for(i=0;i<memtblsize;i++)  
    {  
        if(mallco_dev.memmap[i])used++; 
    } 
    return (used*100)/(memtblsize);  
}

//�ڴ����(�ڲ�����)
//memx:�����ڴ��
//size:Ҫ������ڴ��С(�ֽ�)
//����ֵ:0XFFFFFFFF,��������;����,�ڴ�ƫ�Ƶ�ַ 
uint32_t my_mem_malloc(uint32_t size)  
{  
	signed long offset=0;  
	uint32_t nmemb;	//��Ҫ���ڴ����  
	uint32_t cmemb=0;//�������ڴ����
	uint32_t i;  
	if(!mallco_dev.memrdy)mallco_dev.init();//δ��ʼ��,��ִ�г�ʼ�� 
	if(size==0)return 0XFFFFFFFF;//����Ҫ����
	nmemb=size/memblksize;  	//��ȡ��Ҫ����������ڴ����
	if(size%memblksize)nmemb++;  
	for(offset=memtblsize-1;offset>=0;offset--)//���������ڴ������  
	{     
	if(!mallco_dev.memmap[offset])cmemb++;//�������ڴ��������
	else cmemb=0;								//�����ڴ������
	if(cmemb==nmemb)							//�ҵ�������nmemb�����ڴ��
	{
		for(i=0;i<nmemb;i++)  					//��ע�ڴ��ǿ� 
		{  
			mallco_dev.memmap[offset+i]=nmemb;  
		}  
		return (offset*memblksize);//����ƫ�Ƶ�ַ  
	}
	}  
	return 0XFFFFFFFF;//δ�ҵ����Ϸ����������ڴ��  
}  

//�ͷ��ڴ�(�ڲ�����) 
//offset:�ڴ��ַƫ��
//����ֵ:0,�ͷųɹ�;1,�ͷ�ʧ��;  
uint8_t my_mem_free(uint32_t offset)  
{  
    int i;  
    if(!mallco_dev.memrdy)//δ��ʼ��,��ִ�г�ʼ��
	{
		mallco_dev.init();    
        return 1;//δ��ʼ��  
    }  
    if(offset<memsize)//ƫ�����ڴ����. 
    {  
        int index=offset/memblksize;			//ƫ�������ڴ�����  
        int nmemb=mallco_dev.memmap[index];	//�ڴ������
        for(i=0;i<nmemb;i++)  						//�ڴ������
        {  
            mallco_dev.memmap[index+i]=0;  
        }  
        return 0;  
    }else return 2;//ƫ�Ƴ�����.  
}  

//�ͷ��ڴ�(�ⲿ����) 
//memx:�����ڴ��
//ptr:�ڴ��׵�ַ 
void myfree(void *ptr)  
{  
	uint32_t offset;   
	if(ptr==NULL)return;//��ַΪ0.  
 	offset=(uint32_t)ptr-(uint32_t)mallco_dev.membase;     
    my_mem_free(offset);	//�ͷ��ڴ�      
} 

//�����ڴ�(�ⲿ����)
//memx:�����ڴ��
//size:�ڴ��С(�ֽ�)
//����ֵ:���䵽���ڴ��׵�ַ.
void *mymalloc(uint32_t size)  
{  
	uint32_t offset;   
	offset=my_mem_malloc(size);  	   	 	   
	if(offset==0XFFFFFFFF)return NULL;  
	else return (void*)((uint32_t)mallco_dev.membase+offset);  
}

//���·����ڴ�(�ⲿ����)
//memx:�����ڴ��
//*ptr:���ڴ��׵�ַ
//size:Ҫ������ڴ��С(�ֽ�)
//����ֵ:�·��䵽���ڴ��׵�ַ.
void *myrealloc(void *ptr,uint32_t size)  
{  
    uint32_t offset;    
    offset=my_mem_malloc(size);   	
    if(offset==0XFFFFFFFF)return NULL;     
    else  
    {  									   
	    mymemcpy((void*)((uint32_t)mallco_dev.membase+offset),ptr,size);	//�������ڴ����ݵ����ڴ�   
        myfree(ptr);  											  		//�ͷž��ڴ�
        return (void*)((uint32_t)mallco_dev.membase+offset);  				//�������ڴ��׵�ַ
    }  
}
