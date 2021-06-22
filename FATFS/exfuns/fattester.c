#include "fattester.h"
#include "m_sd.h"
#include "exfuns.h"
#include "malloc.h"
#include "m_Serial.h"


uint8_t SD_file_un;					//��Ŷ�ӦĿ¼�µ��ļ�����
char *SD_item[];            //����һ��ָ�����飬��Ŷ�ӦĿ¼�µ��ļ����б�
    
//Ϊ����ע�Ṥ����	 
//path:����·��������"0:"��"1:"
//mt:0��������ע�ᣨ�Ժ�ע�ᣩ��1������ע��
//����ֵ:ִ�н��
uint8_t mf_mount(uint8_t* path,uint8_t mt)
{		   
	return f_mount(fs[0],(const TCHAR*)path,mt); 
}
//��·���µ��ļ�
//path:·��+�ļ���
//mode:��ģʽ
//����ֵ:ִ�н��
uint8_t mf_open(uint8_t*path,uint8_t mode)
{
	uint8_t res;	 
	printf("mode=%#x\r\n",mode);
	res=f_open(file,(const TCHAR*)path,mode);//���ļ���
	return res;
} 
//�ر��ļ�
//����ֵ:ִ�н��
uint8_t mf_close(void)
{
	f_close(file);
	return 0;
}
//��������
//len:�����ĳ���
//����ֵ:ִ�н��
uint8_t mf_read(uint16_t len)
{
	uint16_t i,t;
	uint8_t res=0;
	uint16_t tlen=0;
	printf("\r\nRead file data is:\r\n");
	for(i=0;i<len/512;i++)
	{
		res=f_read(file,fatbuf,512,&br);
		if(res)
		{
			printf("Read Error:%d\r\n",res);
			break;
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}
	}
	if(len%512)
	{
		res=f_read(file,fatbuf,len%512,&br);
		if(res)	//�����ݳ�����
		{
			printf("\r\nRead Error:%d\r\n",res);   
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}	 
	}
	if(tlen)printf("\r\nReaded data len:%d\r\n",tlen);//���������ݳ���
	printf("Read data over\r\n");	 
	return res;
}
//д������
//dat:���ݻ�����
//len:д�볤��
//����ֵ:ִ�н��
uint8_t mf_write(uint8_t*dat, uint16_t len)
{			    
	uint8_t res;	   					   

	printf("\r\nBegin Write file...\r\n");
	printf("Write data len:%d\r\n",len);	 
	res=f_write(file,dat,len,&bw);
	if(res)
	{
		printf("Write Error:%d\r\n",res);   
	}else printf("Writed data len:%d\r\n",bw);
	printf("Write data over.\r\n");
	return res;
}

//��Ŀ¼
 //path:·��
//����ֵ:ִ�н��
uint8_t mf_opendir(uint8_t* path)
{
	return f_opendir(&dir,(const TCHAR*)path);	
}
//�ر�Ŀ¼ 
//����ֵ:ִ�н��
uint8_t mf_closedir(void)
{
	return f_closedir(&dir);	
}
//���ȡ�ļ���
//����ֵ:ִ�н��
uint8_t mf_readdir(void)
{
	uint8_t res;    
	res=f_readdir(&dir,&fileinfo);	//��ȡһ���ļ�����Ϣ
	if(res!=FR_OK)return res;		//������ 
	printf("\r\n DIR info:\r\n");

	printf("dir.dptr:%ld\r\n",dir.dptr);
	printf("dir.obj.id:%d\r\n",dir.obj.id);
	printf("dir.obj.sclust:%ld\r\n",dir.obj.sclust);
	printf("dir.obj.objsize:%lld\r\n",dir.obj.objsize);
	printf("dir.obj.c_ofs:%ld\r\n",dir.obj.c_ofs);
	printf("dir.clust:%ld\r\n",dir.clust);
	printf("dir.sect:%ld\r\n",dir.sect);	  
	printf("dir.blk_ofs:%ld\r\n",dir.blk_ofs);	  

	printf("\r\n");
	printf("File Name is:%s\r\n",fileinfo.fname);
	printf("File Size is:%lld\r\n",fileinfo.fsize);
	printf("File data is:%d\r\n",fileinfo.fdate);
	printf("File time is:%d\r\n",fileinfo.ftime);
	printf("File Attr is:%d\r\n",fileinfo.fattrib);
	printf("\r\n"); 
	return 0;
}			 

 //�����ļ�
 //path:·��
 //����ֵ:ִ�н��
uint8_t mf_scan_files(uint8_t * path)
{
	FRESULT res;	 
	res = f_opendir(&dir,(const TCHAR*)path); //��һ��Ŀ¼
	if (res == FR_OK) 
	{	
		printf("\r\n"); 
		while(1)
		{
			res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
			//if (fileinfo.fname[0] == '.') continue;             //�����ϼ�Ŀ¼
 			printf("%s/", path);//��ӡ·��	
			printf("%s\r\n",fileinfo.fname);//��ӡ�ļ���	  
		} 
	}	   
    return res;	  
}
//��ʾʣ������
//drv:�̷�
//����ֵ:ʣ������(�ֽ�)
uint32_t mf_showfree(uint8_t *drv)
{
	FATFS *fs1;
	uint8_t res;
	uint32_t fre_clust=0; 
	long tot_sect=0, fre_sect=0;
	//�õ�������Ϣ�����д�����
	res = f_getfree((const TCHAR*)drv,(DWORD*)&fre_clust, &fs1);
	if(res==0)
	{											   
	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;//�õ���������
	    fre_sect = fre_clust * fs1->csize;			//�õ�����������	   
#if _MAX_SS!=512
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		if(tot_sect<20480)//������С��10M
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n����������:%ld KB\r\n"
		           "���ÿռ�:%ld KB\r\n",
		           tot_sect>>1,fre_sect>>1);
		}else
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n����������:%ld MB\r\n"
		           "���ÿռ�:%ld MB\r\n",
		           tot_sect>>11,fre_sect>>11);
		}
	}
	return fre_sect;
}		    
//�ļ���дָ��ƫ��
//offset:����׵�ַ��ƫ����
//����ֵ:ִ�н��.
uint8_t mf_lseek(uint32_t offset)
{
	return f_lseek(file,offset);
}
//��ȡ�ļ���ǰ��дָ���λ��.
//����ֵ:λ��
uint32_t mf_tell(void)
{
	return f_tell(file);
}
//��ȡ�ļ���С
//����ֵ:�ļ���С
uint32_t mf_size(void)
{
	return f_size(file);
} 
//����Ŀ¼
//pname:Ŀ¼·��+����
//����ֵ:ִ�н��
uint8_t mf_mkdir(uint8_t*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
//��ʽ��
//path:����·��������"0:"��"1:"
//mode:ģʽ
//au:�ش�С
//����ֵ:ִ�н��
uint8_t mf_fmkfs(uint8_t* path, uint8_t mode, uint16_t au)
{
	return f_mkfs((const TCHAR*)path,mode,au);//��ʽ��,drv:�̷�;mode:ģʽ;au:�ش�С
} 
//ɾ���ļ�/Ŀ¼
//pname:�ļ�/Ŀ¼·��+����
//����ֵ:ִ�н��
uint8_t mf_unlink(uint8_t *pname)
{
	return  f_unlink((const TCHAR *)pname);
}

//�޸��ļ�/Ŀ¼����(���Ŀ¼��ͬ,�������ƶ��ļ�Ŷ!)
//oldname:֮ǰ������
//newname:������
//����ֵ:ִ�н��
uint8_t mf_rename(uint8_t *oldname, uint8_t* newname)
{
	return  f_rename((const TCHAR *)oldname,(const TCHAR *)newname);
}
//��ȡ�̷����������֣�
//path:����·��������"0:"��"1:"  
void mf_getlabel(uint8_t *path)
{
	uint8_t buf[20];
	uint32_t sn=0;
	uint8_t res;
	res=f_getlabel ((const TCHAR *)path,(TCHAR *)buf,(DWORD*)&sn);
	if(res==FR_OK)
	{
		printf("\r\n����%s ���̷�Ϊ:%s\r\n",path,buf);
		printf("����%s �����к�:%lX\r\n\r\n",path,sn); 
	}else printf("\r\n��ȡʧ�ܣ�������:%X\r\n",res);
}
//�����̷����������֣����11���ַ�������֧�����ֺʹ�д��ĸ����Լ����ֵ�
//path:���̺�+���֣�����"0:ALIENTEK"��"1:OPENEDV"  
void mf_setlabel(uint8_t *path)
{
	uint8_t res;
	res=f_setlabel ((const TCHAR *)path);
	if(res==FR_OK)
	{
		printf("\r\n�����̷����óɹ�:%s\r\n",path);
	}else printf("\r\n�����̷�����ʧ�ܣ�������:%X\r\n",res);
} 

//���ļ������ȡһ���ַ���
//size:Ҫ��ȡ�ĳ���
void mf_gets(uint16_t size)
{
 	TCHAR* rbuf;
	rbuf=f_gets((TCHAR*)fatbuf,size,file);
	if(*rbuf==0)return  ;//û�����ݶ���
	else
	{
		printf("\r\nThe String Readed Is:%s\r\n",rbuf);  	  
	}			    	
}
//��Ҫ_USE_STRFUNC>=1
//дһ���ַ����ļ�
//c:Ҫд����ַ�
//����ֵ:ִ�н��
uint8_t mf_putc(uint8_t c)
{
	return f_putc((TCHAR)c,file);
}
//д�ַ������ļ�
//c:Ҫд����ַ���
//����ֵ:д����ַ�������
uint8_t mf_puts(uint8_t*c)
{
	return f_puts((TCHAR*)c,file);
}

//��ȡ��ӦĿ¼�µ��ļ�����
void mf_get_file_nu(uint8_t * path)
{
	SD_file_un = 0;
	FRESULT res;	 
	res = f_opendir(&dir,(const TCHAR*)path); //��һ��Ŀ¼
	if (res == FR_OK) 
	{ 
		while(1)
		{
			res = f_readdir(&dir, &fileinfo);                   //��ȡĿ¼�µ�һ���ļ�
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //������/��ĩβ��,�˳�
 			SD_file_un++;	  
		}
		SD_file_un++;		
	}
	f_closedir(&dir);
	//printf("file_number:%d\r\n",SD_file_un);
}


void mf_get_file_name(uint8_t * path)
{
	uint8_t i;
	mf_get_file_nu(path);  //��ȡĿ¼�µ��ļ���Ŀ
	
	FRESULT res;
	res = f_opendir(&dir,(const TCHAR*)path); //��һ��Ŀ¼
	SD_item[0] = (char*)mymalloc(sizeof(char)*strlen("Back")+1);
	strcpy(SD_item[0], "Back");
	//��ȡĿ¼�µ��ļ����������SD_itemָ��������
	for(i=1; i<SD_file_un; i++)
	{
		res = f_readdir(&dir, &fileinfo);
		if(res == FR_OK)
		{
			//Ϊ��Ӧ���ļ��������ڴ�
			SD_item[i] = (char*)mymalloc(sizeof(char)*(strlen(fileinfo.altname)+1));
			strcpy(SD_item[i], fileinfo.fname);
		}
	}
	f_closedir(&dir);
//	for(i=0; i<SD_file_un; i++)
//	{
//		printf("SD_item[%d]:%s\r\n",i,SD_item[i]);
//	}
}













