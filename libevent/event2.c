#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event.h>
#include <time.h>
#include <signal.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

// 这里不是操作io，操作具体数据
//没有read函数，操作缓冲区，不是操作io
void socket_read_callback(struct bufferevent *bev, void *arg)
{
    // 操作读缓冲当中的数据，evbuffer就是chainbuffer链式缓冲区
    struct evbuffer *evbuf = bufferevent_get_input(bev); // 获取封装了的读缓冲区

//read只有两种形式：固定长度读、特殊字符读
    char *msg = evbuffer_readln(evbuf, NULL, EVBUFFER_EOL_LF);//读取以\n结尾
    if (!msg) return;
    
    printf("server read the data: %s\n", msg);

    char reply[4096] = {0};
    sprintf(reply, "recvieced msg: %s\n", msg);
    // -WRN: 需要自己释放资源
    free(msg);
    bufferevent_write(bev, reply, strlen(reply));//write也是操作缓冲区，不再操作io，不需要返回值
    //写入 写缓冲区之后，会自动刷新
}

void stdio_callback(struct bufferevent *bev, void *arg)
{
    struct evbuffer *evbuf = bufferevent_get_input(bev);
    char *msg = evbuffer_readln(evbuf, NULL, EVBUFFER_EOL_LF);
    if (!msg) return;

    if (strcmp(msg, "quit") == 0) {
        printf("safe exit!!!\n");
        exit(0);
        return;
    }
    
    printf("stdio read the data: %s\n", msg);
}

void socket_event_callback(struct bufferevent *bev, short events, void *arg)
{
    if (events & BEV_EVENT_EOF) // read == 0
        printf("connection closed\n");
    else if (events & BEV_EVENT_ERROR) // strerro(errno)
        printf("some other error\n");
    else if (events & BEV_EVENT_TIMEOUT)
        printf("timeout\n");
    bufferevent_free(bev); // close
}

//io操作帮我们隐藏掉 返回值错误码等，不需要关注细节，只需要看回调的返回值就行了
//用不同的回调函数来处理不同的事件，达到解耦的目的
void listener_callback(struct evconnlistener *listener, evutil_socket_t fd,
                       struct sockaddr *sock, int socklen, void *arg)//fd是clientfd
{
    char ip[32] = {0};
    evutil_inet_ntop(AF_INET, sock, ip, sizeof(ip)-1);
    printf("accept a client fd:%d ip:%s\n", fd, ip);
    struct event_base *base = (struct event_base *)arg;

//base是事件管理器，fd是clientfd，BEV_OPT_CLOSE_ON_FREE是free时要close clientfd
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

//回调，read > 0是socket_read_callback， NULL是写缓冲区，read =0 =-1是socket_event_callback错误处理
    bufferevent_setcb(bev, socket_read_callback, NULL, socket_event_callback, NULL);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);//注册，event_add + event_assign
}

static void
do_timer(int fd, short events, void* arg) {
    struct event * timer = (struct event *)arg;
    time_t now = time(NULL);
    printf("do_timer %s", (char*)ctime(&now));
    event_del(timer);
    // struct timeval tv = {1,0};
    // event_add(timer, &tv);
}

static void
do_sig_int(int fd, short event, void *arg) {
    struct event *si = (struct event*) arg;
    event_del(si);
    printf("do_sig_int SIGINT\n");
}

int main()
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8989);

    struct event_base *base = event_base_new();
    struct evconnlistener *listener =
        evconnlistener_new_bind(base, listener_callback, base,
                                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                10, (struct sockaddr *)&sin,
                                sizeof(struct sockaddr_in));

//base是事件管理器，BEV_OPT_CLOSE_ON_FREE会close fd
    struct bufferevent *ioev = bufferevent_socket_new(base, 0, BEV_OPT_CLOSE_ON_FREE);
//
    bufferevent_setcb(ioev, stdio_callback, NULL, NULL, base);
    bufferevent_enable(ioev, EV_READ | EV_PERSIST);

    struct event evtimer;
    struct timeval tv = {1,0};
    event_set(&evtimer, -1, 0, do_timer, &evtimer);
    event_base_set(base, &evtimer);
    event_add(&evtimer, &tv);

    struct event evint;
    event_set(&evint, SIGINT, EV_SIGNAL, do_sig_int, &evint);
    event_base_set(base, &evint);
    event_add(&evint, NULL);

    event_base_dispatch(base);
    evconnlistener_free(listener);
    event_base_free(base);
    return 0;
}

/*
gcc evmain2.c -I./libevent/include -o ev2 -L./libevent/.libs -levent
client:
    telnet 127.0.0.1 8989
*/
