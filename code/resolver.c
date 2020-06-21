#include "dns.h"
#define CACHE_PATH "resolver-cache.txt"

int listensock;
void resolver(const char * resolverIP, const int resolverPort, const char * rootServerIP, const int rootServerPort, const char * resolverHostFilePath){
    DataBase * db = newdb(resolverHostFilePath);
    
    listensock = startTCPServer(resolverIP, resolverPort);
    struct sockaddr_in * cliaddr = newSocketaddr_t(0, 0);
    printf("本地DNS开始运行 %s:%d\n", resolverIP, resolverPort);
    while (1) {
        
        int clisock = Accept(listensock, cliaddr);
        struct timeval start, stop;
        double secs = 0;
        gettimeofday(&start, NULL);
        printf("开始DNS解析 %s:%d\n", inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));

        
        Message * clientQueryMessage = newMessage();
        Buffer * s = newBuffer();
        s->content_len = (int) recv(clisock, (void *) s->buffer, 4096, 0);
        get2Bytes(s);
        cin_Message(clientQueryMessage, s);
        
        Question * q = clientQueryMessage->questions;
        print_message(clientQueryMessage);
        
        
        Message * possibleAnswerMessage = calloc(1, sizeof(struct Message));
        Question * qp = possibleAnswerMessage->questions;
        possibleAnswerMessage->queryNum = 1;
        qp->QClass = DNS_CLASS_TYPE_IN;
        qp->QType = q->QClass;
        qp->QName = strdup(q->QName);
        
        
        int ret_code = fetchAnswerInDB(possibleAnswerMessage, db);
        if (ret_code > 0) {
            printf("本地查找成功\n");
            Buffer * s = newBuffer();
            put2Bytes(s, 0); 
            cout_Message(s, possibleAnswerMessage);
            printf("直接返回\n");
            DNSPacketSend_tcp(clisock, s);
            freeBuffer(s);
            print_message(possibleAnswerMessage);
            close(clisock);
            continue;
        }
        
        
        struct sockaddr_in * nextServerAddr = newSocketaddr(rootServerIP, rootServerPort);
        printf("准备访问 %s@%d\n", inet_ntoa(nextServerAddr->sin_addr), ntohs(nextServerAddr->sin_port));
        
        do {
            
            int serverSock = startUDPClient_o(nextServerAddr);
            Buffer * s = newBuffer();
            cout_Message(s, clientQueryMessage);
            DNSPacketSend_udp(serverSock, s, nextServerAddr);
            freeBuffer(s);
            possibleAnswerMessage = udpMessageRecv(serverSock, &nextServerAddr);
            close(serverSock);
            
            
            if (possibleAnswerMessage->answerNum != 0) {
                trace_message(possibleAnswerMessage);
                for (int i = 0 ; i < possibleAnswerMessage->answerNum; i++) {
                    ResourceRecord * r = possibleAnswerMessage->answers + i;
                    if (search_db_identical_record_fromindex(db, 0, r) == -1) {
                        DataBaseRecord * dbr = addrecord_db_t(db, r);
                        printf("数据库记录: "); print_cache_record(dbr);
                    }
                }
                
                
                for (int i = 0 ; i < possibleAnswerMessage->additionalNum; i++) {
                    ResourceRecord * r = possibleAnswerMessage->additionals + i;
                    if (search_db_identical_record_fromindex(db, 0, r) == -1) {
                        DataBaseRecord * dbr = addrecord_db_t(db, r);
                        printf("数据库记录: "); print_cache_record(dbr);
                    }
                }

                printf("数据库缓存更新: %s\n", CACHE_PATH);
                print_db(db);
                FILE * resolverCache = fopen(CACHE_PATH, "w+");
                if (resolverCache == NULL) {
                    printf("数据库缓存更新失败: %s\n", CACHE_PATH);
                    break;
                }
                savedb(db, resolverCache);
                fclose(resolverCache);
                break;
            }
            
            
            if (possibleAnswerMessage->headerrcode == rcode_NXDOMAIN) {
                printf("报文错误: NXDOMAIN\n");
                break;
            }
            
            
            if (possibleAnswerMessage->authorityNum != 0) {
                trace_message(possibleAnswerMessage);
                ResourceRecord * r = possibleAnswerMessage->authorities;
                nextServerAddr = newSocketaddr_t(ntohl(r->rdata.a_record.addr), PORTS);
                struct sockaddr_in addr; addr.sin_addr.s_addr = r->rdata.a_record.addr;
                continue;
            }
            
            
            printf("Encountered Error in resolver!\n");
            possibleAnswerMessage->headerrcode = rcode_server_fail;
            break;
            
        } while (possibleAnswerMessage->answerNum == 0);
        
        printf("<< Ready to send back \n");
        print_message(possibleAnswerMessage);
        
        Buffer * t = newBuffer();
        put2Bytes(t, 0);
        cout_Message(t, possibleAnswerMessage);
        DNSPacketSend_tcp(clisock, t);
        freeBuffer(t);
        
        
        gettimeofday(&stop, NULL);
        secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
        printf("\n< ** Client Service End **\nRequest finished in %f ms\n", secs * 1000);
        
        time_t current_time = time(NULL);
        char * c_time_string = ctime(&current_time);
        (void) printf("Current Time: %s\n", c_time_string);
        
        close(clisock);
    }
}


int main(int argc, const char * argv[]){
    
    if (argc != 3) {
        printf("USAGE:  ./resolver  LocalDNS-bind-IP  HostFile\n");
        exit(1);
    }
    resolver(argv[1], PORTS, ROOTIP, PORTS, argv[2]);
    return 0;
}
