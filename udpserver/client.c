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

/*
 * udp�����ҵĿͻ���, �ӽ��̷�����Ϣ,�����̽�����Ϣ
 */
int main(int argc, char* argv[]) {
    int sd, rt;
    struct sockaddr_in addr = { AF_INET };
    socklen_t alen = sizeof addr;
    pid_t pid;
    struct umsg msg = { '1' };    

    // ����򵥼��
    if(argc != 4) {
        fprintf(stderr, "uage : %s [ip] [port] [name]\n", argv[0]);
        exit(-1);
    }    
    // ����Խ�����
    if((rt = atoi(argv[2]))<1024 || rt > 65535)
        CERR("atoi port = %s is error!", argv[2]);
    // �����ж�ip����
    IF_CHECK(inet_aton(argv[1], &addr.sin_addr));
    addr.sin_port = htons(rt);
    // ����ƴ���û�����
    strncpy(msg.name, argv[3], _INT_NAME - 1);
    
    //����socket ����
    IF_CHECK(sd = socket(PF_INET, SOCK_DGRAM, 0));
    // ������Ƿ��͵�¼��Ϣ��udp�����������
    IF_CHECK(sendto(sd, &msg, sizeof msg, 0, (struct sockaddr*)&addr, alen));    
    
    //����һ������, �ӽ��̴�������Ϣ, �����̽�����Ϣ
    IF_CHECK(pid = fork());
    if(pid == 0) { //�ӽ���,�Ⱥ����˳������ֹ��Ϊ��ʬ����
        signal(SIGCHLD, SIG_IGN);                
        while(fgets(msg.text, _INT_TEXT, stdin)){
            if(strcasecmp(msg.text, "quit\n") == 0){ //��ʾ�˳�
                msg.type = '3';
                // �������ݲ����
                IF_CHECK(sendto(sd, &msg, sizeof msg, 0, (struct sockaddr*)&addr, alen));
                break;
            }
            // ϴ�鰴������ͨ��Ϣ
            msg.type = '2';
            IF_CHECK(sendto(sd, &msg, sizeof msg, 0, (struct sockaddr*)&addr, alen));
        }
        // ����������,��ɱ��������
        close(sd);
        kill(getppid(), SIGKILL);
        exit(0);
    }
    // �����Ǹ����̴������ݵĶ�ȡ
    for(;;){
        bzero(&msg, sizeof msg);
        IF_CHECK(recvfrom(sd, &msg, sizeof msg, 0, (struct sockaddr*)&addr, &alen));
        msg.name[_INT_NAME-1] = msg.text[_INT_TEXT-1] = '\0';
        switch(msg.type){
        case '1':printf("%s ��¼��������!\n", msg.name);break;
        case '2':printf("%s ˵��: %s\n", msg.name, msg.text);break;
        case '3':printf("%s �˳���������!\n", msg.name);break;
        default://δʶ����쳣����,����ֱ���˳�
            fprintf(stderr, "msg is error! [%s:%d] => [%c:%s:%s]\n", inet_ntoa(addr.sin_addr),
                    ntohs(addr.sin_port), msg.type, msg.name, msg.text);
            goto __exit;
        }
    }    

__exit:    
    // ɱ�����ȴ��ӽ����˳�
    close(sd);
    kill(pid, SIGKILL);
    waitpid(pid, NULL, -1);    

    return 0;
}