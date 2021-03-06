#include "fattester.h"
#include "m_sd.h"
#include "exfuns.h"
#include "malloc.h"
#include "m_Serial.h"


uint8_t SD_file_un;					//存放对应目录下的文件数量
char *SD_item[];            //定义一个指针数组，存放对应目录下的文件名列表
    
//为磁盘注册工作区	 
//path:磁盘路径，比如"0:"、"1:"
//mt:0，不立即注册（稍后注册）；1，立即注册
//返回值:执行结果
uint8_t mf_mount(uint8_t* path,uint8_t mt)
{		   
	return f_mount(fs[0],(const TCHAR*)path,mt); 
}
//打开路径下的文件
//path:路径+文件名
//mode:打开模式
//返回值:执行结果
uint8_t mf_open(uint8_t*path,uint8_t mode)
{
	uint8_t res;	 
	printf("mode=%#x\r\n",mode);
	res=f_open(file,(const TCHAR*)path,mode);//打开文件夹
	return res;
} 
//关闭文件
//返回值:执行结果
uint8_t mf_close(void)
{
	f_close(file);
	return 0;
}
//读出数据
//len:读出的长度
//返回值:执行结果
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
		if(res)	//读数据出错了
		{
			printf("\r\nRead Error:%d\r\n",res);   
		}else
		{
			tlen+=br;
			for(t=0;t<br;t++)printf("%c",fatbuf[t]); 
		}	 
	}
	if(tlen)printf("\r\nReaded data len:%d\r\n",tlen);//读到的数据长度
	printf("Read data over\r\n");	 
	return res;
}
//写入数据
//dat:数据缓存区
//len:写入长度
//返回值:执行结果
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

//打开目录
 //path:路径
//返回值:执行结果
uint8_t mf_opendir(uint8_t* path)
{
	return f_opendir(&dir,(const TCHAR*)path);	
}
//关闭目录 
//返回值:执行结果
uint8_t mf_closedir(void)
{
	return f_closedir(&dir);	
}
//打读取文件夹
//返回值:执行结果
uint8_t mf_readdir(void)
{
	uint8_t res;    
	res=f_readdir(&dir,&fileinfo);	//读取一个文件的信息
	if(res!=FR_OK)return res;		//出错了 
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

 //遍历文件
 //path:路径
 //返回值:执行结果
uint8_t mf_scan_files(uint8_t * path)
{
	FRESULT res;	 
	res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
	if (res == FR_OK) 
	{	
		printf("\r\n"); 
		while(1)
		{
			res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
			//if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
 			printf("%s/", path);//打印路径	
			printf("%s\r\n",fileinfo.fname);//打印文件名	  
		} 
	}	   
    return res;	  
}
//显示剩余容量
//drv:盘符
//返回值:剩余容量(字节)
uint32_t mf_showfree(uint8_t *drv)
{
	FATFS *fs1;
	uint8_t res;
	uint32_t fre_clust=0; 
	long tot_sect=0, fre_sect=0;
	//得到磁盘信息及空闲簇数量
	res = f_getfree((const TCHAR*)drv,(DWORD*)&fre_clust, &fs1);
	if(res==0)
	{											   
	    tot_sect = (fs1->n_fatent - 2) * fs1->csize;//得到总扇区数
	    fre_sect = fre_clust * fs1->csize;			//得到空闲扇区数	   
#if _MAX_SS!=512
		tot_sect*=fs1->ssize/512;
		fre_sect*=fs1->ssize/512;
#endif	  
		if(tot_sect<20480)//总容量小于10M
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n磁盘总容量:%ld KB\r\n"
		           "可用空间:%ld KB\r\n",
		           tot_sect>>1,fre_sect>>1);
		}else
		{
		    /* Print free space in unit of KB (assuming 512 bytes/sector) */
		    printf("\r\n磁盘总容量:%ld MB\r\n"
		           "可用空间:%ld MB\r\n",
		           tot_sect>>11,fre_sect>>11);
		}
	}
	return fre_sect;
}		    
//文件读写指针偏移
//offset:相对首地址的偏移量
//返回值:执行结果.
uint8_t mf_lseek(uint32_t offset)
{
	return f_lseek(file,offset);
}
//读取文件当前读写指针的位置.
//返回值:位置
uint32_t mf_tell(void)
{
	return f_tell(file);
}
//读取文件大小
//返回值:文件大小
uint32_t mf_size(void)
{
	return f_size(file);
} 
//创建目录
//pname:目录路径+名字
//返回值:执行结果
uint8_t mf_mkdir(uint8_t*pname)
{
	return f_mkdir((const TCHAR *)pname);
}
//格式化
//path:磁盘路径，比如"0:"、"1:"
//mode:模式
//au:簇大小
//返回值:执行结果
uint8_t mf_fmkfs(uint8_t* path, uint8_t mode, uint16_t au)
{
	return f_mkfs((const TCHAR*)path,mode,au);//格式化,drv:盘符;mode:模式;au:簇大小
} 
//删除文件/目录
//pname:文件/目录路径+名字
//返回值:执行结果
uint8_t mf_unlink(uint8_t *pname)
{
	return  f_unlink((const TCHAR *)pname);
}

//修改文件/目录名字(如果目录不同,还可以移动文件哦!)
//oldname:之前的名字
//newname:新名字
//返回值:执行结果
uint8_t mf_rename(uint8_t *oldname, uint8_t* newname)
{
	return  f_rename((const TCHAR *)oldname,(const TCHAR *)newname);
}
//获取盘符（磁盘名字）
//path:磁盘路径，比如"0:"、"1:"  
void mf_getlabel(uint8_t *path)
{
	uint8_t buf[20];
	uint32_t sn=0;
	uint8_t res;
	res=f_getlabel ((const TCHAR *)path,(TCHAR *)buf,(DWORD*)&sn);
	if(res==FR_OK)
	{
		printf("\r\n磁盘%s 的盘符为:%s\r\n",path,buf);
		printf("磁盘%s 的序列号:%lX\r\n\r\n",path,sn); 
	}else printf("\r\n获取失败，错误码:%X\r\n",res);
}
//设置盘符（磁盘名字），最长11个字符！！，支持数字和大写字母组合以及汉字等
//path:磁盘号+名字，比如"0:ALIENTEK"、"1:OPENEDV"  
void mf_setlabel(uint8_t *path)
{
	uint8_t res;
	res=f_setlabel ((const TCHAR *)path);
	if(res==FR_OK)
	{
		printf("\r\n磁盘盘符设置成功:%s\r\n",path);
	}else printf("\r\n磁盘盘符设置失败，错误码:%X\r\n",res);
} 

//从文件里面读取一段字符串
//size:要读取的长度
void mf_gets(uint16_t size)
{
 	TCHAR* rbuf;
	rbuf=f_gets((TCHAR*)fatbuf,size,file);
	if(*rbuf==0)return  ;//没有数据读到
	else
	{
		printf("\r\nThe String Readed Is:%s\r\n",rbuf);  	  
	}			    	
}
//需要_USE_STRFUNC>=1
//写一个字符到文件
//c:要写入的字符
//返回值:执行结果
uint8_t mf_putc(uint8_t c)
{
	return f_putc((TCHAR)c,file);
}
//写字符串到文件
//c:要写入的字符串
//返回值:写入的字符串长度
uint8_t mf_puts(uint8_t*c)
{
	return f_puts((TCHAR*)c,file);
}

//获取对应目录下的文件数量
void mf_get_file_nu(uint8_t * path)
{
	SD_file_un = 0;
	FRESULT res;	 
	res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
	if (res == FR_OK) 
	{ 
		while(1)
		{
			res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
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
	mf_get_file_nu(path);  //获取目录下的文件数目
	
	FRESULT res;
	res = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
	SD_item[0] = (char*)mymalloc(sizeof(char)*strlen("Back")+1);
	strcpy(SD_item[0], "Back");
	//获取目录下的文件名，存放在SD_item指针数组中
	for(i=1; i<SD_file_un; i++)
	{
		res = f_readdir(&dir, &fileinfo);
		if(res == FR_OK)
		{
			//为对应的文件名申请内存
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














