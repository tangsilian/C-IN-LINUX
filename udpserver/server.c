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

// ���ֳ��Ȱ���'\0'
#define _INT_NAME (64)
// ������󳤶�,����'\0'
#define _INT_TEXT (512)

//4.0 ����̨��ӡ������Ϣ, fmt������˫�����������ĺ�
#define CERR(fmt, ...) \
    fprintf(stderr,"[%s:%s:%d][error %d:%s]" fmt "\r\n",\
         __FILE__, __func__, __LINE__, errno, strerror(errno),##__VA_ARGS__)

//4.1 ����̨��ӡ������Ϣ���˳�, tͬ��fmt������ ""���������ַ�������
#define CERR_EXIT(fmt,...) \
    CERR(fmt,##__VA_ARGS__),exit(EXIT_FAILURE)
/*
 * �򵥵�Linux��API�����жϼ���, ����ֵ��ʹ��
 */
#define IF_CHECK(code) \
    if((code) < 0) \
        CERR_EXIT(#code)

// ���ͺͽ��յ���Ϣ��
struct umsg{
    char type;                //Э�� '1' => ���������������, '2' => �������������Ϣ, '3' => ������������˳���Ϣ
    char name[_INT_NAME];    //�����û�����
    char text[_INT_TEXT];    //�õ��ı���Ϣ,�ռ任ʱ��
};

// ά��һ���ͻ���������Ϣ,��¼��¼��Ϣ
typedef struct ucnode {
    struct sockaddr_in addr;
    struct ucnode* next;
} *ucnode_t ;


// �½�һ��������
static inline ucnode_t _new_ucnode(struct sockaddr_in* pa){
    ucnode_t node = calloc(sizeof(struct ucnode), 1);    
    if(NULL == node)
        CERR_EXIT("calloc sizeof struct ucnode is error. ");
    node->addr = *pa;
    return node;
}

// ��������,����headĬ��ͷ����ǵ�ǰ���������
static inline void _insert_ucnode(ucnode_t head, struct sockaddr_in* pa) {
    ucnode_t node = _new_ucnode(pa);
    node->next = head->next;
    head->next = node;    
}

// ���������û���¼����
static void _login_ucnode(ucnode_t head, int sd, struct sockaddr_in* pa, struct umsg* msg) {
    _insert_ucnode(head, pa);
    head = head->next;
    // �Ӵ�֮���Ϊ��ǰ������
    while(head->next){
        head = head->next;
        IF_CHECK(sendto(sd, msg, sizeof(*msg), 0, (struct sockaddr*)&head->addr, sizeof(struct sockaddr_in)));
    }
}

// ��Ϣ�㲥
static void _broadcast_ucnode(ucnode_t head, int sd, struct sockaddr_in* pa, struct umsg* msg) {
    int flag = 0; //1��ʾ�Ѿ��ҵ���
    while(head->next) {
        head = head->next;
        if((flag) || !(flag=memcmp(pa, &head->addr, sizeof(struct sockaddr_in))==0)){
            IF_CHECK(sendto(sd, msg, sizeof(*msg), 0, (struct sockaddr*)&head->addr, sizeof(struct sockaddr_in)));
        }
    }
}

// �����˳�Ⱥ��
static void _quit_ucnode(ucnode_t head, int sd, struct sockaddr_in* pa, struct umsg* msg) {
    int flag = 0;//1��ʾ�Ѿ��ҵ�
    while(head->next) {
        if((flag) || !(flag = memcmp(pa, &head->next->addr, sizeof(struct sockaddr_in))==0)){
            IF_CHECK(sendto(sd, msg, sizeof(*msg), 0, (struct sockaddr*)&head->next->addr, sizeof(struct sockaddr_in)));
            head = head->next;
        }        
        else { //ɾ������˳����û�
            ucnode_t tmp = head->next;
            head->next = tmp->next;
            free(tmp);
        }
    }        
}

// ����ά���Ķ����,û�������ӵĿ����˼򵥴����˳���
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
 * udp�����ҵķ�����, �ӽ��̹㲥��Ϣ,�����̽�����Ϣ
 */
int main(int argc, char* argv[]) {
    int sd, rt;
    struct sockaddr_in addr = { AF_INET };
    socklen_t alen = sizeof addr;
    struct umsg msg;    
    ucnode_t head;

    // ����򵥼��
    if(argc != 3) {
        fprintf(stderr, "uage : %s [ip] [port]\n", argv[0]);
        exit(-1);
    }    
    // ����Խ�����
    if((rt = atoi(argv[2]))<1024 || rt > 65535)
        CERR("atoi port = %s is error!", argv[2]);
    // �����ж�ip����
    IF_CHECK(inet_aton(argv[1], &addr.sin_addr));
    addr.sin_port = htons(rt); //�˿�Ҫ���������ֽ���
    // ����socket
    IF_CHECK(sd = socket(PF_INET, SOCK_DGRAM, 0));
    // ����bind�����õĵ�ַ
    IF_CHECK(bind(sd, (struct sockaddr*)&addr, alen));
    
    //��ʼ������
    head = _new_ucnode(&addr);    
    for(;;){
        bzero(&msg, sizeof msg);
        IF_CHECK(recvfrom(sd, &msg, sizeof msg, 0, (struct sockaddr*)&addr, &alen));
        msg.name[_INT_NAME-1] = msg.text[_INT_TEXT-1] = '\0';
        fprintf(stdout, "msg is [%s:%d] => [%c:%s:%s]\n", inet_ntoa(addr.sin_addr),
                    ntohs(addr.sin_port), msg.type, msg.name, msg.text);
        // ��ʼ�жϴ���
        switch(msg.type) {
        case '1':_login_ucnode(head, sd, &addr, &msg);break;
        case '2':_broadcast_ucnode(head, sd, &addr, &msg);break;
        case '3':_quit_ucnode(head, sd, &addr, &msg);break;
        default://δʶ����쳣����,�����������
            fprintf(stderr, "msg is error! [%s:%d] => [%c:%s:%s]\n", inet_ntoa(addr.sin_addr),
                    ntohs(addr.sin_port), msg.type, msg.name, msg.text);
            _quit_ucnode(head, sd, &addr, &msg);
            break;
        }        
    }
        
    // ��δ����ǲ���ִ�е����, ���Լ�һЩ���������ߵ���. ����
    close(sd);
    _destroy_ucnode(&head);    
    return 0;
}