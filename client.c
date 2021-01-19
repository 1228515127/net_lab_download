#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>


int sockfd = 0;



// 上传
void upload(void);
// 下载
void download(void);


// 主函数
int main()
{
	sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockfd<0){
		printf("socket error\n");
		return -1;
	}
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(60000);
	//  要走本地回环请同时修改server.c 和 client.c 的IP
	addr.sin_addr.s_addr = inet_addr("192.168.127.128");
	socklen_t len = sizeof(addr);
	if(connect(sockfd,(struct sockaddr *)&addr,len)){
		printf("connect error\n");
		return -1;
	}
	printf("输入：1 下载   输入2：上传  输入3：退出\n");
	char cmd[5];
	while (1)
	{
		fgets(cmd,sizeof(cmd),stdin);
		write(sockfd,cmd,sizeof(cmd));
		if(cmd[0] == '1'){
			download(); 
		}else if(cmd[0] == '2'){
			upload();
		}else if(cmd[0] =='3'){
			break;
		}
		memset(cmd,0,sizeof(cmd));
	}
	close(sockfd);
	return 0;
}


void download(void){
	char filename[50];
	char buf[1024];
	int r_size=1;
	int w_size=1;
	memset(filename,0,sizeof(filename));
	printf("输入要下载的文件名：\n");
	//文件名
	fgets(filename,sizeof(filename),stdin);
	write(sockfd,filename,sizeof(filename));
	//解决换行符
	filename[strlen(filename)-1]=0;
	int fd = open(filename,O_CREAT | O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	while(1){
		memset(buf,0,sizeof(buf));
		r_size = read(sockfd,buf,sizeof(buf));
		//printf("收到的数据为%s\n",buf);
		//printf("收到%d字节\n",r_size);
		if(buf[r_size-1]==0){
			//   收到 /0  文件结束  把buf写入文件，但/0并不会写入
			w_size = write(fd,buf,r_size-1);
			//printf("写入%d字节\n",w_size);
			break;
		}
		w_size = write(fd,buf,r_size);
		//printf("写入%d字节\n",w_size);
	}
	printf("下载完成\n");
	close(fd);
	return;
}
// 上传
void upload(void){
	char filename[50];
	char buf[1024];
	int r_size=1;
	int w_size=0;
	printf("输入要上传的文件名：\n");
	fgets(filename,sizeof(filename),stdin);
	write(sockfd,filename,sizeof(filename));
	filename[strlen(filename)-1]=0;
	int fd = open(filename,O_RDONLY);
	if(fd<0){
		printf("open error\n");
		return;	
	}
	while(1){
		memset(buf,0,sizeof(buf));
		r_size = read(fd,buf,sizeof(buf));
		//printf("从文件中读取%d字节\n",r_size);
		w_size =  write(sockfd,buf,r_size);
		//printf("向服务器写%d字节\n",w_size);
		if(r_size==0){
			//读到文件结尾，发送一个  \0 给client告诉client文件结束
			memset(buf,0,sizeof(buf));
			write(sockfd,buf,1);
			break;
		}
	}	
	close(fd);
	return;
}

