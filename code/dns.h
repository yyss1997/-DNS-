#ifndef dns_h
#define dns_h

#define PORTS 53

#define MAX_RECORD 32
#define LOCALADDR "127.0.0.20"
#define ROOTIP    "127.0.0.30"


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>

#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <utime.h>
#include <time.h>


static const char * DNS_TYPE_SET [] = { "", "A", "NS", "", "","CNAME", "", "", "", "","", "", "PTR", "", "", "MX" };
static const char * DNS_CLASS_SET [] = { "", "IN" };


#define DNS_RECORD_TYPE_A       1
#define DNS_RECORD_TYPE_NS      2
#define DNS_RECORD_TYPE_CNAME   5
#define DNS_RECORD_TYPE_PTR    12
#define DNS_RECORD_TYPE_MX     15

#define DNS_CLASS_TYPE_IN  1
#define HEADER_FLAG_QR_ISQUERY     0
#define HEADER_FLAG_QR_ISRESPONSE  1

#define HEADER_FLAG_OPCODE_STANDARDQUERY  0
#define HEADER_FLAG_OPCODE_INVQUERY  1
#define HEADER_FLAG_OPCODE_STATUS  2

static const char * RCODE_SET [] = { "NO ERROR", "FORMAT ERROR", "SERVER FAIL", "NXDOMAIN", "NOTIMP", "REFULSED" };

typedef enum {
    rcode_good = 0,
    rcode_format_error = 1,
    rcode_server_fail = 2,
    rcode_NXDOMAIN = 3,
    rcode_query_not_support = 4,
    rcode_policy_refused = 5
} rcode_t;

void initSockaddr(struct sockaddr_in * addr, const char * name, int port);
void initSockaddr_t(struct sockaddr_in * addr, in_addr_t rawIP, int port);
struct sockaddr_in * newSocketaddr(const char * name, int port);
struct sockaddr_in * newSocketaddr_t(in_addr_t rawIP, int port);

int     Socket(int isTCP);
void    Connect(int sock, struct sockaddr_in * addr);
ssize_t SendTCP(int sock, const char * buf, ssize_t len);
ssize_t RecvTCP(int sock, char * buf, ssize_t len);
ssize_t SendUDP(int sock, const char * buf, ssize_t len, struct sockaddr_in * addr);
ssize_t RecvUDP(int sock, char * buf, ssize_t len, struct sockaddr_in * addr);
void    Listen(int sock);
void    Bind(int sock, struct sockaddr_in * addr);
int     Accept(int listen_sock, struct sockaddr_in * addr);

int     startTCPServer(const char * tcpServerIP, int port);
int     startUDPServer(const char * udpServerIP, int port);


int startTCPClient(const char * tcpServerIP, int port);
int startUDPClient(const char * udpServerIP, int port, struct sockaddr_in ** ret_addr);
int startUDPClient_r(in_addr_t rawUDPServerIP, int port, struct sockaddr_in ** ret_addr);
int startUDPClient_o(struct sockaddr_in * addr);



struct Question;
union  RecordData;
struct ResourceRecord;
struct Message;
struct Header;

typedef struct Question Question;
typedef union RecordData RecordData;
typedef struct ResourceRecord ResourceRecord;
typedef struct Message Message;
typedef struct Header Header;


struct Question {
    char *   QName;
    uint16_t QType;
    uint16_t QClass;
};

union RecordData {
    struct { char *   name; }     name_record;
    struct { in_addr_t addr;}     a_record;
    struct { uint16_t preference; char *exchange; } mx_record;
};

struct ResourceRecord {
    char *name;
    uint16_t rtype;
    uint16_t rclass;
    uint32_t rttl;
    uint16_t rdlength;
    union RecordData rdata;
};

struct Header {
    uint16_t headerID;
    bool headerQR;
    uint16_t headerOpcode;
    bool headerAA;
    bool headerTC;
    bool headerRD;
    bool headerRA;
    uint16_t headerz;
    uint16_t headerrcode;
    
    uint16_t queryNum;
    uint16_t answerNum;
    uint16_t authorityNum;
    uint16_t additionalNum;
};

#define RecordItemsMAX 8
struct Message {
    uint16_t headerID;
    bool headerQR;
    uint16_t headerOpcode;
    bool headerAA;
    bool headerTC;
    bool headerRD;
    bool headerRA;
    uint16_t headerz;
    uint16_t headerrcode;
    
    uint16_t queryNum;
    uint16_t answerNum;
    uint16_t authorityNum;
    uint16_t additionalNum;
    
    struct Question questions[RecordItemsMAX];
    struct ResourceRecord answers[RecordItemsMAX];
    struct ResourceRecord authorities[RecordItemsMAX];
    struct ResourceRecord additionals[RecordItemsMAX];
    
};

typedef char * URLString;

URLString getURLFromFileString(const char * str);
char * getNormalStringFromURL(const char * url);

int parseDNSTypeString(const char * typestring);

void make_question_query_r(Question * q, int type, const char * name_alloced );
void add_question_query(Message * mesg, const char * type, const char * name);
void add_question_query_r(Message * mesg, int type, const char * name_alloced );

void print_message(Message * message);
void trace_message(Message * message);
void initMessage(Message * message);

Message * newMessage(void);













struct DataBaseRecord {
    char * name;
    char * original_name;
    uint32_t ttl;
    int class;
    int  type;
    char * record;
    RecordData data;
};

struct DataBase {
    struct DataBaseRecord records[30];
    int len;
};

typedef struct DataBaseRecord DataBaseRecord;

typedef struct DataBase DataBase;


void initdb(DataBase * db, FILE * fs);

DataBase * newdb(const char * fname);

void savedb(DataBase * db, FILE * fs);

void addrecord_db(DataBase * db, const char * name, uint32_t ttl, int class, int type, const char * record);

DataBaseRecord * addrecord_db_t(DataBase * db, ResourceRecord * record);

int findQuestionAnswerinDB(DataBase * db, Question * q);

int findResourceRecordMatchInDB(DataBase * db, ResourceRecord * r);

int findQuestionInDB_i(DataBase * db, int start, Question * q);

int findNameServerInDB(DataBase * db, int start, Question * q);

void cacheRR(DataBase * db, ResourceRecord * r);

int search_db_identical_record_fromindex(DataBase *db, int start, ResourceRecord * r);

void add_resource(ResourceRecord * mr, DataBaseRecord * r);

void add_answer(Message * msg, DataBaseRecord * r);

void add_authority(Message * msg, DataBaseRecord * r);

void add_additional(Message * msg, DataBaseRecord * r);




int getTypeIntValue(char * str, const char ** set, int setsize);

void print_cache_record(DataBaseRecord * r);

void print_db(DataBase * db);

int fetchAnswerInDB(Message * mesg, DataBase * cacheDB);




struct Buffer {
    char buffer[4096];
    int content_len;
    int cursor_pos;
    
};

typedef struct Buffer Buffer;

void initBuffer(Buffer * s);

Buffer * newBuffer(void);
void freeBuffer(Buffer * s);

int getBytes(char * des, Buffer * src, size_t size);
uint8_t get1Byte(Buffer * src);
uint16_t get2Bytes(Buffer * src);
uint32_t get4Bytes(Buffer * src);


int putBytes(Buffer * des, char * src, size_t size);
int put1Byte(Buffer * des, uint8_t src);
int put2Bytes(Buffer * des, uint16_t src);
int put4Bytes(Buffer * des, uint32_t src);



/* Writer Functions */

void cin_Message(Message * mesg, Buffer * s);
void cin_Header(Header * head, Buffer * s);
void cin_Question(Question * q, Buffer *s);
void cin_ResourceRecord(ResourceRecord * r, Buffer * s);
void cin_recordData(ResourceRecord * r, Buffer * s);

int  getURLString(char * des, Buffer * src);

char * getURLString_allocated(Buffer * src);

/* Reader Functions */

void cout_Message(Buffer * s, Message * mesg);
void cout_Header(Buffer * s, Header * head);
void cout_Question(Buffer * s, Question * q);
void cout_ResourceRecord(Buffer * s, ResourceRecord * r);
void cout_recordData(Buffer * s, ResourceRecord * r);
void cout_TCP_Header(Buffer * s);

int  putURLString(Buffer * des, char * src);


ssize_t DNSPacketSend_tcp (int sock, Buffer *);
ssize_t DNSPacketRecv_tcp (int sock, Buffer *);
ssize_t DNSPacketSend_udp (int sock, Buffer *, struct sockaddr_in * addr);
ssize_t DNSPacketRecv_udp (int sock, Buffer *, struct sockaddr_in * addr);

void       tcpMessageSend (int sock, Message * mesg);
Message *  tcpMessageRecv (int sock);
void       udpMessageSend (int sock, Message * mesg, struct sockaddr_in * addr);
Message *  udpMessageRecv (int sock, struct sockaddr_in ** ret_clientAddr);




#endif /* dns_h */

