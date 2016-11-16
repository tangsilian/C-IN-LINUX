#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// 名字长度包含'\0'
#define _INT_NAME (64)
// 报文最大长度,包含'\0'
#define _INT_TEXT (512)

//4.0 控制台打印错误信息, fmt必须是双引号括起来的宏
#define CERR(fmt, ...) \
    fprintf(stderr,"[%s:%s:%d][error %d:%s]" fmt "\r\n",\
         __FILE__, __func__, __LINE__, errno, strerror(errno),##__VA_ARGS__)

//4.1 控制台打印错误信息并退出, t同样fmt必须是 ""括起来的字符串常量
#define CERR_EXIT(fmt,...) \
    CERR(fmt,##__VA_ARGS__),exit(EXIT_FAILURE)
/*
 * 简单的Linux上API错误判断检测宏, 好用值得使用
 */
#define IF_CHECK(code) \
    if((code) < 0) \
        CERR_EXIT(#code)

// 发送和接收的信息体
struct umsg{
    char type;                //协议 '1' => 向服务器发送名字, '2' => 向服务器发送信息, '3' => 向服务器发送退出信息
    char name[_INT_NAME];    //保存用户名字
    char text[_INT_TEXT];    //得到文本信息,空间换时间
};

// 维护一个客户端链表信息,记录登录信息
typedef struct ucnode {
    struct sockaddr_in addr;
    struct ucnode* next;
} *ucnode_t ;


// 新建一个结点对象
static inline ucnode_t _new_ucnode(struct sockaddr_in* pa){
    ucnode_t node = calloc(sizeof(struct ucnode), 1);    
    if(NULL == node)
        CERR_EXIT("calloc sizeof struct ucnode is error. ");
    node->addr = *pa;
    return node;
}

// 插入数据,这里head默认头结点是当前服务器结点
static inline void _insert_ucnode(ucnode_t head, struct sockaddr_in* pa) {
    ucnode_t node = _new_ucnode(pa);
    node->next = head->next;
    head->next = node;    
}

// 这里是有用户登录处理
static void _login_ucnode(ucnode_t head, int sd, struct sockaddr_in* pa, struct umsg* msg) {
    _insert_ucnode(head, pa);
    head = head->next;
    // 从此之后才为以前的链表
    while(head->next){
        head = head->next;
        IF_CHECK(sendto(sd, msg, sizeof(*msg), 0, (struct sockaddr*)&head->addr, sizeof(struct sockaddr_in)));
    }
}

// 信息广播
static void _broadcast_ucnode(ucnode_t head, int sd, struct sockaddr_in* pa, struct umsg* msg) {
    int flag = 0; //1表示已经找到了
    while(head->next) {
        head = head->next;
        if((flag) || !(flag=memcmp(pa, &head->addr, sizeof(struct sockaddr_in))==0)){
            IF_CHECK(sendto(sd, msg, sizeof(*msg), 0, (struct sockaddr*)&head->addr, sizeof(struct sockaddr_in)));
        }
    }
}

// 有人退出群聊
static void _quit_ucnode(ucnode_t head, int sd, struct sockaddr_in* pa, struct umsg* msg) {
    int flag = 0;//1表示已经找到
    while(head->next) {
        if((flag) || !(flag = memcmp(pa, &head->next->addr, sizeof(struct sockaddr_in))==0)){
            IF_CHECK(sendto(sd, msg, sizeof(*msg), 0, (struct sockaddr*)&head->next->addr, sizeof(struct sockaddr_in)));
            head = head->next;
        }        
        else { //删除这个退出的用户
            ucnode_t tmp = head->next;
            head->next = tmp->next;
            free(tmp);
        }
    }        
}

// 销毁维护的对象池,没有往复杂的考虑了简单处理退出了
static void _destroy_ucnode(ucnode_t* phead) {
    ucnode_t head;
    if((!phead) || !(head=*phead)) return;    
    while(head){
        ucnode_t tmp = head->next;
        free(head);
        head = tmp;
    }    

    *phead = NULL;
}

/*
 * udp聊天室的服务器, 子进程广播信息,父进程接受信息
 */
int main(int argc, char* argv[]) {
    int sd, rt;
    struct sockaddr_in addr = { AF_INET };
    socklen_t alen = sizeof addr;
    struct umsg msg;    
    ucnode_t head;

    // 这里简单检测
    if(argc != 3) {
        fprintf(stderr, "uage : %s [ip] [port]\n", argv[0]);
        exit(-1);
    }    
    // 下面对接数据
    if((rt = atoi(argv[2]))<1024 || rt > 65535)
        CERR("atoi port = %s is error!", argv[2]);
    // 接着判断ip数据
    IF_CHECK(inet_aton(argv[1], &addr.sin_addr));
    addr.sin_port = htons(rt); //端口要采用网络字节序
    // 创建socket
    IF_CHECK(sd = socket(PF_INET, SOCK_DGRAM, 0));
    // 这里bind绑定设置的地址
    IF_CHECK(bind(sd, (struct sockaddr*)&addr, alen));
    
    //开始监听了
    head = _new_ucnode(&addr);    
    for(;;){
        bzero(&msg, sizeof msg);
        IF_CHECK(recvfrom(sd, &msg, sizeof msg, 0, (struct sockaddr*)&addr, &alen));
        msg.name[_INT_NAME-1] = msg.text[_INT_TEXT-1] = '\0';
        fprintf(stdout, "msg is [%s:%d] => [%c:%s:%s]\n", inet_ntoa(addr.sin_addr),
                    ntohs(addr.sin_port), msg.type, msg.name, msg.text);
        // 开始判断处理
        switch(msg.type) {
        case '1':_login_ucnode(head, sd, &addr, &msg);break;
        case '2':_broadcast_ucnode(head, sd, &addr, &msg);break;
        case '3':_quit_ucnode(head, sd, &addr, &msg);break;
        default://未识别的异常报文,程序把其踢走
            fprintf(stderr, "msg is error! [%s:%d] => [%c:%s:%s]\n", inet_ntoa(addr.sin_addr),
                    ntohs(addr.sin_port), msg.type, msg.name, msg.text);
            _quit_ucnode(head, sd, &addr, &msg);
            break;
        }        
    }
        
    // 这段代码是不会执行到这的, 可以加一些控制让其走到这. 看人
    close(sd);
    _destroy_ucnode(&head);    
    return 0;
}