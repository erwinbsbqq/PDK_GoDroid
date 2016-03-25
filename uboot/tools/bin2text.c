#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define APP_NAME	"bin2text"
#define VERSION	"0.0"
#define BUILD_DATE	__DATE__

static void print_usage(void) {
	fprintf(stdout,"[Usage] %s -[b:d:n:o:l:p:h] <value>\n", APP_NAME);
	fprintf(stdout,"\tVersion: %s, Date: %s\n", VERSION, BUILD_DATE);
	fprintf(stdout,"\tAuthor: Youri Zhang\n");
	fprintf(stdout,"\t-b : specify the bin file path/name\n");
	fprintf(stdout,"\t-d : specify the dest file path/name\n");
	fprintf(stdout,"\t-n : specify the name of the text array\n");
	fprintf(stdout,"\t-o : specify the offset(in byte) of the bin file to be transfer\n");
	fprintf(stdout,"\t  if offset not specified, APP will use 0\n");
	fprintf(stdout,"\t-l : specify the length(in byte) be transfer from bin to text array\n");
	fprintf(stdout,"\t  if length not specified, APP will use total bin file size\n");
	fprintf(stdout,"\t-p : specify the bytes per-line of the output text array\n");
	fprintf(stdout,"\t  if -p not specified, APP will outputs 16Bytes per-line\n");
	fprintf(stdout,"\t-h : this help information.\n");
	fprintf(stdout,"Ex:==> (./%s -b input.bin -d test.h -n bin_array).\n", APP_NAME);
}

static void textUpperCase(char *textIn)
{
	int i;
	for(i=0;i<strlen(textIn);i++){
		if(textIn[i] > 0x60 && textIn[i] < 0x7B)
			textIn[i] -= 0x20;
		if(textIn[i] == 0x2E)
			textIn[i] = 0x5F;
	}
}

int main(int argc, char *argv[]) 
{ 
	int opt,ret=1,i;
	FILE *srcFile = NULL, *dstFile = NULL;
	char *SourceFile=NULL;
	char *DestFile=NULL;
	char *ArrayName=NULL;
	unsigned char temDestName[128];
	int length=-1;
	int offset=-1;
	int lineLength = -1;
	struct stat filestat;
	int iReadByte=0;
	int iBufSize = 1024;
	unsigned char pBuf[iBufSize];
	
	if(argc==2 && !strcmp(argv[1],"-h"))
	{
		print_usage();
		exit(0);
	}else if(argc >= 6){
			while ((opt = getopt(argc, argv, "b:d:n:o:l:p:h")) != -1)
			{
				switch (opt)
				{
					case 'b': 
						SourceFile=optarg;
						break;
						
					case 'd': 
						DestFile=optarg;
						break;
						
					case 'n': 
						ArrayName=optarg;
						break;
						
					case 'o': 
						offset = strtoul(optarg, NULL, 0);
						break;

					case 'l':
						length = strtoul(optarg, NULL, 0);
						break;
						
					case 'p':
						lineLength = strtoul(optarg, NULL, 0);
						break;
							
					case 'h': /* help information */
						print_usage();
						exit(0); 
					default :
						fprintf(stderr, "unknown/uncompleted option: %c\n", optopt);
						print_usage();
						exit(ret);
				}
			}
	}
	else 
	{
		print_usage();
		exit(ret);
	}

	if((srcFile = fopen(SourceFile,"r")) == NULL){
		fprintf(stderr, "Error while trying to open %s\n", SourceFile);
		exit(ret);
	}
   	if (stat(SourceFile,&filestat) < 0)
	{
		fprintf(stderr,"While trying to get the file status of %s\n",SourceFile);
		goto failed1;
	}
	length=(unsigned int)length<filestat.st_size?length:filestat.st_size;
	
	fprintf(stdout,"Input bin file length %d Bytes\n",length);

	if(offset == -1)
		offset = 0;
	else{
		if(fseek(srcFile,offset,SEEK_SET) != offset){
			fprintf(stderr,"Error while seeking to 0x%x of %s\n",offset,SourceFile);
			goto failed1;	
		}
	}
	
	if((offset+length)>filestat.st_size)
	{
		fprintf(stderr,"The offset+length > bin file size\n");
		goto failed1;
	}

	if(lineLength == -1 || !lineLength)
		lineLength = 16;
	
	if((dstFile = fopen(DestFile,"w+")) == NULL){
		fprintf(stderr, "Error while trying to open %s\n", DestFile);
		goto failed1;
	}

	memcpy(temDestName,
		DestFile,\
		strlen(DestFile)>sizeof(temDestName)?sizeof(temDestName):strlen(DestFile));

	textUpperCase(temDestName);
	
	memset(pBuf,0x00,sizeof(pBuf));
	sprintf(pBuf,"%s%s%s%s%s%s%s%s%s%d%s","#ifndef _",temDestName, \
		"_\n\0","#define _",temDestName,"_\n\n\0", \
		"unsigned char ",ArrayName,"[",length,"] = {\n\t\0");
	if(fwrite(pBuf,1,strlen(pBuf),dstFile) != strlen(pBuf))
	{
		fprintf(stderr,"Error while writing dest file\n");
		goto failed2;	
	}
	
	
	while((iReadByte=fread(pBuf,1,iBufSize,srcFile))>0 && length>0)
	{
		iReadByte=iReadByte<length?iReadByte:length;
		for(i=0;i<iReadByte;i++){
			fprintf(dstFile, "0x%02x,", pBuf[i]);
			if((i+1)%lineLength == 0){
				if(i+1 != length)
					fprintf(dstFile,"\n\t");
				else
					fprintf(dstFile,"\n");
			}
		}
		length-=iReadByte;
	}
	if((i)%lineLength != 0)
		fprintf(dstFile,"\n};\n\n#endif\n");
	else
		fprintf(dstFile,"};\n\n#endif\n");	
	
	ret=0;

	memset(pBuf,0x00,sizeof(pBuf));
	sprintf(pBuf,"%s%s","chmod 777 ",DestFile);
	system(pBuf);

failed2:
	fclose(dstFile);
failed1:
	fclose(srcFile);
	exit(ret);
} 

