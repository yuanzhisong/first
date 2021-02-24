#include<sys/types.h>
#include<sys/socket.h>
#include<s_stdio.h>
#include<unistd.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include"web.h"
//服务器端存放网页文件的路径
char *w_dir="/home/xuniji1/html";
void get_request_info(int fd,request_t *req){
	char buf[1024],path[256];
	//将浏览器的请求信息填充到buf中
	int ret=read(fd,buf,1024);
	//提取第一行的信息
	sscanf(buf,"%s %s %s\r\n",req->meth,req->path,req->proto);
	if(strcmp(req->path,"/")==0)strcpy(req->path,"/index.html");
	//将请求路径转换为服务器的本地路径
	strcpy(path,w_dir);
	strcat(path,req->path);
	strcpy(req->path,path);
	return;
}
char *get_file_type(const char *file){
  char *p=strrchr(file,',');
  if(p==NULL)return "text/html";
  if(strcmp(p,".jpg")==0) return "image/jpg";
  if(strcmp(p,".png")==0) return "image/png";
  return "text/html";
}
void get_respon_info(request_t *req,response_t *res){
  //这个req->path空间里存放的是客户端请求的文件的路径
  res->code=access(req->path,R_OK)?404:200;
  if(res->code==200)
  res->f_typ=get_file_type(req->path);
  else
  res->f_typ="text/html";

  return ;
  }
  void response_b(int fd,response_t *res,request_t *req){
   //向浏览器写响应头
   char head[256];
   sprintf(head,"%s %d\r\nContent-Type:%s\r\n\r\n",\
   req->proto,res->code,res->f_typ);
   write(fd,head,strlen(head));
   if(res->code==404){
     char *html="<html><body><h2>file not found.!</body></html>";
     write(fd,html,strlen(html));
     }else{
     dup2(fd,1);
     execlp("cat","cat",req->path,NULL);
     }
    return ;
    }
int main(void){
	char IP[32];
	char buf[1024];
	int sfd,b,r,cfd,i;
	struct sockaddr_in serv,cli;//具体的ipv4地址结构
	//创建socket设备，返回该设备的文件描述符
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd==-1)E_MSG("socket",-1);
	//初始化serv的成员
	serv.sin_family=AF_INET;
	serv.sin_port=htons(4466);
	serv.sin_addr.s_addr=htonl(INADDR_ANY);
	//将sfd绑定到本地地址（ip地址和端口号）
	b=bind(sfd,(struct sockaddr *)&serv,sizeof(serv));
	if(b==-1)E_MSG("bind",-1);
	//将sfd设置为被动连接状态，监听客户端连接的请求，将客户端到来的连接请求放入到未决连接队列中
	listen(sfd,5);
	printf("accept..\n");
	while(1){
		socklen_t len=sizeof(cli);
		//从sfd指定的设备的未决连接队列中取出一个连接请求，进行连接处理，返回和客户端的连接描述符   
		cfd=accept(sfd,(struct sockaddr *)&cli,&len);
		if(cfd==-1)E_MSG("accept",-1);
		request_t req;response_t res;
		//到这里连接已经建立完毕
		//将客户端的ip地址和端口号保存到cli的空间里
		//cli里客户端的地址是binary格式的  需要转换为text格式
		//const char *ip=inet_ntop(AF_INET,&cli.sin_addr,IP,32);
		//		printf("ip:%s\n",ip);
		//从cfd中获取客户端的请求信息
		//	r=read(cfd,buf,sizeof(buf));
		//   处理客户端请求信息
		//for(i=0;i<=r;i++)
		//	buf[i]=toupper(buf[i]);//将客户端的字符转换为大写
		//  将处理结果返回给客户端
		//将请求信息的第一行中的内容摘取出来
		get_request_info(cfd,&req);
		printf("%s %s %s\r\n",req.meth,req.path,req.proto);
		get_respon_info(&req,&res);
		printf("code:%d\tf_type:%s\n",res.code,res.f_typ);
		//响应客户端
		response_b(cfd,&res,&req);

		close(cfd);
	}

	return 0;
}
