#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>




int clifd = 0;


// 开始运行
void *start_run(void *arg);
// 客户端上传
void upload();
// 客户端下载
void download();




// 主函数
int main()
{
	printf("服务器创建socket...\n");
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd<0){
		perror("socket");
		return -1;
	}

	printf("准备地址...\n");
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	// 端口 IP 自行修改
	addr.sin_port = htons(60000);
	//  要走本地回环请同时修改server.c 和 client.c 的IP
	addr.sin_addr.s_addr = inet_addr("192.168.127.128");
	socklen_t len = sizeof(addr);

	printf("绑定socket与地址...\n");
	if (bind(sockfd, (struct sockaddr *)&addr, len))
	{
		perror("bind");
		return -1;
	}

	printf("设置监听...\n");
	if (listen(sockfd, 5))
	{
		perror("listen");
		return -1;
	}

	printf("等待客户端连接...\n");
	for (;;)
	{
		struct sockaddr_in addrcli = {};
		// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		clifd = accept(sockfd, (struct sockaddr *)&addrcli, &len);
		if (0 > clifd)
		{
			perror("accept");
			continue;
		}

		pthread_t pid;
		// 创建线程函数 int pthread_create(pthread_t *restrict tidp,const pthread_attr_t *restrict_attr,void*（*start_rtn)(void*),void *restrict arg);
		//第一个参数为指向线程标识符的指针。
		//第二个参数用来设置线程属性。
		//第三个参数是线程运行函数的地址。
		//最后一个参数是运行函数的参数。
		pthread_create(&pid, NULL, start_run, &clifd);
	}

	return 0;
}

// 开始运行
void *start_run(void *arg)
{
	printf("用户连接\n");
	char buf[5];
	//接收客户端发来的命令
	while (1){
		memset(buf,0,sizeof(buf));
		read(clifd,buf,sizeof(buf));
		//下载
		if(buf[0]=='1')
			download();
		//上传
		else if(buf[0]=='2')
			upload();
		//结束
		else if(buf[0]=='3')
			break;
	}
	
	close(clifd);
	return NULL;
}

void download(){
	char filename[50];
	char buf[1024];
	int r_size=1;
	int w_size=0;
	//文件名
	read(clifd,filename,sizeof(filename));
	//printf("收到的文件名为：%s\n",filename);
	//解决换行符
	filename[strlen(filename)-1]=0;
	printf("收到的文件名为：%s\n",filename);
	int fd = open(filename,O_RDONLY);
	if(fd<0){
		printf("open error\n");
		return;	
	}
	while(1){
		memset(buf,0,sizeof(buf));
		r_size = read(fd,buf,sizeof(buf));
		//printf("从文件中读取%d字节\n",r_size);
		w_size =  write(clifd,buf,r_size);
		//printf("向客户端写%d字节\n",w_size);
		if(r_size==0){
			//读到文件结尾，发送一个  \0 给client告诉client文件结束
			memset(buf,0,sizeof(buf));
			write(clifd,buf,1);
			break;
		}
	}	
	close(fd);
	return;
}

void upload(){
	char filename[50];
	char buf[1024];
	int r_size=1;
	int w_size=1;
	memset(filename,0,sizeof(filename));
	//文件名
	read(clifd,filename,sizeof(filename));
	//解决换行符
	filename[strlen(filename)-1]=0;
	int fd = open(filename,O_CREAT | O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	if(fd<0){
		printf("open error\n");
		return;	
	}
	while(1){
		memset(buf,0,sizeof(buf));
		r_size = read(clifd,buf,sizeof(buf));
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
	printf("上传完成\n");
	close(fd);
	return;
}
