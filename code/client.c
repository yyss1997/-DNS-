#include "dns.h"
#define LOCALADDR "127.0.0.20"

Buffer s;
char *    queryType;
char *    queryURL ;

struct timeval  start;
struct timeval  stop;
double    secs           = 0;


int main(int argc, const char * argv[]){
    
    if (argc != 3) {
        printf("USAGE: ./client query-type query-url\n");
        exit(1);
    }
    queryType   =  (char *) argv[1] ;
    queryURL    =  (char *) argv[2] ;
    
    const char *    resolverIP     = LOCALADDR;
    const int       resolverPort   = PORTS;


    gettimeofday(&start, NULL);
    
    Message * sendMesg = calloc(1, sizeof(Message));
    sendMesg->queryNum = 1;
    sendMesg->questions[0].QName = getURLFromFileString(queryURL);
    sendMesg->questions[0].QType = parseDNSTypeString(queryType);
    sendMesg->questions[0].QClass = DNS_CLASS_TYPE_IN;
    
    int sock = socket(PF_INET, SOCK_STREAM,  IPPROTO_TCP);
    if (sock < 0) {
        fprintf(stderr, "Socket打开错误"); exit(1);
    }
    
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(resolverPort);
    addr->sin_addr.s_addr = inet_addr(resolverIP);
    
    if (connect(sock, (const struct sockaddr *) addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Connect() 本地DNS服务器失败"); exit(1);
    }
    
    printf("连接到本地DNS服务器 %s@%d...\n", resolverIP, resolverPort);
    printf("【准备发送报文】\n");
    
    print_message(sendMesg);
    
    Buffer s; bzero(&s, sizeof(Buffer));
    cout_TCP_Header(&s);
    cout_Message(&s, sendMesg);
    DNSPacketSend_tcp(sock, &s);
    
    Message * recvMesg = tcpMessageRecv(sock);
    
    printf("\n\n【接受报文】\n");
    print_message(recvMesg);
    
    gettimeofday(&stop, NULL);
    secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    printf("\n【请求用时】: %f ms\n",secs * 1000);
    
    time_t current_time = time(NULL);
    char * c_time_string = ctime(&current_time);
    printf("%s\n", c_time_string);
    
    return 0;
}

