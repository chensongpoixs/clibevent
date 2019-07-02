#include <event2/event.h>
#include <string.h>
#include <unistd.h>

struct event *readEv = NULL;

void readcb(evutil_socket_t fd, short events, void *arg)
{
    char buf[256] = {0};
    int ret = recv(fd,buf,sizeof(buf),0);
    if(ret > 0)
    {
        send(fd,buf,ret,0);
    }
    else if(ret == 0)
    {
        //客户端关闭
        printf("client closed\n");
        close(fd);
        event_del(readEv);
        event_free(readEv);
        readEv = NULL;
    }
    else {
        perror("recv err");
        close(fd);
        event_del(readEv);
        event_free(readEv);
        readEv = NULL;
    }
}

void conncb(evutil_socket_t fd, short events, void *arg)
{
    struct event_base *base = arg;
   struct sockaddr_in client;
   socklen_t len = sizeof(client);
   int cfd = accept(fd,(struct sockaddr*)&client,&len);
   if(cfd > 0)
   {
       //得到新连接,上树
        readEv = event_new(base,cfd,EV_READ|EV_PERSIST,readcb,NULL);
        event_add(readEv,NULL);
   }
}

int main(int argc, char *argv[])
{
    //1. 创建socket
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    //2. 绑定bind
    struct sockaddr_in serv;
    bzero(&serv,sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(9111);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));//设置端口复用

    if(bind(lfd,(struct sockaddr*)&serv,sizeof(serv)) < 0)
    {
        perror("bind err");
        return -1;
    }
    //3. 监听listen
    listen(lfd,128);

    //libevent 创建根节点
    struct event_base *base = event_base_new();
    //创建event事件,设置回调函数,事件类型,文件描述符
    //struct event *event_new(struct event_base *, evutil_socket_t, short, event_callback_fn, void *); 
    struct event *connEv = event_new(base, lfd, EV_READ|EV_PERSIST, conncb, base);
    //设置监听event_add
    // event_add(struct event *ev, const struct timeval *timeout);
    event_add(connEv,NULL);//开始监听新连接事件
    //事件循环 event_base_dispatch 
    event_base_dispatch(base);
    //各种释放
    event_base_free(base);//释放根
    event_free(connEv);
    if(readEv )
    {
        event_free(readEv);
    }
    close(lfd);
    return 0;
}
