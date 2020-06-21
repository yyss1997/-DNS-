#include "dns.h" 

FILE * fs;
int serverPort = PORTS;
int sock;

int main(int argc, const char * argv[]){
    if (argc != 3) {
        printf("USAGE: ./server   SERVER-IP   HOSTFILE\n");
        exit(1);
    }
    const char * serverIP = argv[1];
    const char * hostfile = argv[2];
    
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        return 1;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    addr.sin_addr.s_addr = inet_addr(serverIP);
    
    if (bind(sock, (const struct sockaddr *) & addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind() 错误");
        return 1;
    }
    
    if (!(fs = fopen(hostfile, "r"))) {
        printf("域名文件路径不存在\n");
        return 1;
    }
    
    DataBase * db = calloc(1 , sizeof(DataBase));
    initdb(db, fs);
    
    
    printf("服务器等待请求 %s:%d\n", serverIP, serverPort);
    
    while (1) {
        Buffer * s = newBuffer();
        Buffer * t = newBuffer();

        Message * recvMesg = calloc(1, sizeof(Message));
        struct sockaddr_in * clientAddr = newSocketaddr_t(0, 0);
        DNSPacketRecv_udp(sock, s, clientAddr);
        cin_Message(recvMesg, s);
        print_message(recvMesg);
        int ret_code = fetchAnswerInDBfetchAnswerInDB(recvMesg, db);
        
        if (ret_code == -1) {
            recvMesg->headerrcode = rcode_NXDOMAIN;
        }
        recvMesg->headerQR = 1;
        print_message(recvMesg);
        
        udpMessageSend(sock, recvMesg, clientAddr);
        cout_Message(t, recvMesg);
        DNSPacketSend_udp(sock, t, clientAddr);
        
        freeBuffer(s);
        freeBuffer(t);
    }
    return 0;
}
