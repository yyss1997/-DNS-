#include "dns.h"


void initSockaddr(struct sockaddr_in * addr, const char * name, int port){
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, name, &addr->sin_addr.s_addr);
}

void initSockaddr_t(struct sockaddr_in * addr, in_addr_t rawIP, int port){
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_addr.s_addr = htonl(rawIP);
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
}


struct sockaddr_in * newSocketaddr(const char * name, int port){
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, name, &addr->sin_addr.s_addr);
    return addr;
}

struct sockaddr_in * newSocketaddr_t(in_addr_t rawIP, int port){
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_addr.s_addr = htonl(rawIP);
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    return addr;
}


int Socket(int isTCP){
    int sock = socket(PF_INET, isTCP ? SOCK_STREAM : SOCK_DGRAM, isTCP ? IPPROTO_TCP : IPPROTO_UDP);
    if (sock < 0) {
        fprintf(stderr, "Socket error"); exit(1);
    }
    return sock;
}

void Connect(int sock, struct sockaddr_in * addr){
    if (connect(sock, (const struct sockaddr *) addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Connect() failed"); exit(1);
    }
}

ssize_t SendTCP(int sock, const char * buf, ssize_t len){
    return send(sock, (void *)buf, len, 0);
}

ssize_t RecvTCP(int sock, char * buf, ssize_t len){
    return recv(sock, (void *) buf, len, 0);
}

ssize_t SendUDP(int sock, const char * buf, ssize_t len, struct sockaddr_in * addr){
    return sendto(sock, (void *)buf, len, 0, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
}

ssize_t RecvUDP(int sock, char * buf, ssize_t len, struct sockaddr_in * addr){
    socklen_t t = sizeof(struct sockaddr_in);
    return recvfrom(sock, (void *)buf, len, 0, (struct sockaddr *)addr, &t);
}


void Bind(int sock, struct sockaddr_in * addr){
    if (bind(sock, (const struct sockaddr *) addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind() 失败"); exit(1);
    }
}

void Listen(int sock){
    if (listen(sock, 100) < 0) {
        perror("Listen() failed"); exit(1);
    }
}

int Accept(int listen_sock, struct sockaddr_in * addr){
    socklen_t t = sizeof(struct sockaddr_in);
    int accpeted_socket = accept(listen_sock, (struct sockaddr *) addr, &t);
    if (accpeted_socket < 0) {
        perror("Accept() failed");
    }
    return accpeted_socket;
}


int startTCPServer(const char * tcpServerIP, int port)
{
    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        fprintf(stderr, "Socket error"); exit(1);
    }
    
    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0){
        perror("setsockopt(SO_REUSEADDR) failed");
    }
#ifdef SO_REUSEPORT
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0){
        perror("setsockopt(SO_REUSEPORT) failed");
    }
#endif
    
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(tcpServerIP);
    
    if (bind(sock, (const struct sockaddr *) addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind() failed"); exit(1);
    }
    
    if (listen(sock, 100) < 0) {
        perror("Listen() failed"); exit(1);
    }
    
    return sock;
}

int startUDPServer(const char * udpServerIP, int port)
{
    int sock = Socket(false);
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(udpServerIP);
    
    if (bind(sock, (const struct sockaddr *) addr, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind() 失败"); exit(1);
    }
    return sock;
}

int startTCPClient(const char * tcpServerIP, int port)
{
    int sock = Socket(true);
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, tcpServerIP, &addr->sin_addr.s_addr);
    Connect(sock, addr);
    printf("TCP连接成功： %s@%d...\n", tcpServerIP, port);
    return sock;
}

int startUDPClient(const char * udpServerIP, int port, struct sockaddr_in ** ret_addr)
{
    int sock = Socket(false);
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, udpServerIP, &addr->sin_addr.s_addr);
    *ret_addr = addr;
    return sock;
}

int startUDPClient_r(in_addr_t rawUDPServerIP, int port, struct sockaddr_in ** ret_addr)
{
    int sock = Socket(false);
    struct sockaddr_in * addr = calloc(1, sizeof(struct sockaddr_in));
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_addr.s_addr = htonl(rawUDPServerIP);
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    *ret_addr = addr;
    return sock;
}

int startUDPClient_o(struct sockaddr_in * addr)
{
    int sock = Socket(false);
    return sock;
}



URLString getURLFromFileString(const char * url){
    if (!url) { return NULL; }
    if (url) {
        if ( url[0] == 0) {
            return strdup("");
        }else if ((url[1] == 0 && url[0] == '.') ){
            return strdup("");
        }
    }
    
    ssize_t final_url_len = strlen(url) + 2;
    if ((int)(strrchr(url, '.') - url) == strlen(url) - 1) {
        final_url_len -= 1;
    }
    
    URLString buf = malloc(256); bzero(buf, 256);
    ssize_t buflen = 0;
    
    URLString turl = strdup(url);
    URLString tok = strtok(turl, ".");
    
    while (tok) {
        uint8_t len = strlen(tok);
        memcpy(&buf[buflen], &len, sizeof(uint8_t)); buflen += 1;
        memcpy(&buf[buflen], tok, len); buflen += len;
        tok = strtok(NULL, ".");
    }
    buf[buflen] = 0;
    
    return strdup(buf);
}

char * getNormalStringFromURL(const char * turl){
    if (!turl) { return NULL; }
    if (strlen(turl) == 0) { return strdup("."); }
    char * ret = strdup(turl);
    char * p = ret;
    for (uint8_t len = 0 ; (len = (uint8_t)p[0]) != 0 && (p[0] = '.') ; (p += len + 1)) { ; }
    p[0] = 0;
    return strdup(ret+1);
    
}

int parseDNSTypeString(const char * typestring){
    if (strcmp(typestring, "A") == 0) {
        return DNS_RECORD_TYPE_A;
    } else if (strcmp(typestring, "CNAME") == 0) {
        return DNS_RECORD_TYPE_CNAME;
    } else if (strcmp(typestring, "MX") == 0) {
        return DNS_RECORD_TYPE_MX;
    } else {
        return -1;
    }
}

int parseClassTypeString(const char * classstring){
    return DNS_CLASS_TYPE_IN;
}

void make_question_query(Question * q, const char * type, const char * name ){
    q->QName = getURLFromFileString(name);
    q->QType = parseDNSTypeString(type);
    q->QClass = DNS_CLASS_TYPE_IN;
}

void add_question_query(Message * mesg, const char * type, const char * name ){
    make_question_query(mesg->questions + mesg->queryNum, type, name);
    mesg->queryNum += 1;
}

void make_question_query_r(Question * q, int type, const char * name_alloced ){
    if (type < 0) { return; }
    q->QType = type;
    q->QClass = DNS_CLASS_TYPE_IN;
    if (name_alloced == NULL) { return; }
    q->QName = (char *) name_alloced;
}

void add_question_query_r(Message * mesg, int type, const char * name_alloced ){
    make_question_query_r(mesg->questions + mesg->queryNum, type, name_alloced);
    mesg->queryNum += 1;
}

int print_question(Question * q){
    int type = q->QType;
    char * typestr = "(null)";
    if (type == DNS_RECORD_TYPE_A) {
        typestr = "A";
    } else if (type == DNS_RECORD_TYPE_CNAME) {
        typestr = "CNAME";
    } else if (type == DNS_RECORD_TYPE_MX) {
        typestr = "MX";
    } else {
        
    }
    printf("  %s\t%s\tIN\n", getNormalStringFromURL(q->QName), typestr);
    
    return 0;
}

void print_resource_record(ResourceRecord * r){
    RecordData * d = &r->rdata;
    printf("  %s\t%s\t%s\t%d\t 数据长度 = %d\t ", getNormalStringFromURL(r->name), DNS_TYPE_SET[r->rtype], DNS_CLASS_SET[r->rclass], r->rttl, r->rdlength);
    
    struct in_addr addr;
    int type = r->rtype;
    
    if (type == DNS_RECORD_TYPE_A) {
        addr.s_addr = d->a_record.addr;
        printf("%s\n", inet_ntoa(addr));
    } else if (type == DNS_RECORD_TYPE_MX) {
        printf("%d\t\t%s\n", d->mx_record.preference, getNormalStringFromURL(d->mx_record.exchange));
    } else if (type == DNS_RECORD_TYPE_CNAME) {
        printf("%s\n", getNormalStringFromURL(d->name_record.name));
    } else {
        return ;
    }
}

void print_message(Message * message){
    
    printf("<---- DNS 报文开始 ----> \n");
    Header * h = (Header *) message;
    printf("  id:%d\tstatus:%s\t", h->headerID, RCODE_SET[h->headerrcode]);
    printf("  报文头信息 %s%s%s%s%s\n", h->headerQR ? "qr ": "", h->headerAA ? "aa " : "", h->headerTC ? "tc " : "", h->headerRD ? "rd " : "", h->headerRA ? "ra " : "");
    printf("Query Section: %d\n", message->queryNum);
    for (int i = 0; i < message->queryNum; i++) {
        Question * q = message->questions + i;
        int type = q->QType;
        char * typestr = "(null)";
        if (type == DNS_RECORD_TYPE_A) {
            typestr = "A";
            printf("  %s\tA\tIN\n", getNormalStringFromURL(q->QName));
        } else if (type == DNS_RECORD_TYPE_CNAME) {
            typestr = "CNAME";
            printf("  %s\tCNAME\tIN\n", getNormalStringFromURL(q->QName));
        } else if (type == DNS_RECORD_TYPE_MX) {
            typestr = "MX";
            printf("  %s\tMX\tIN\n", getNormalStringFromURL(q->QName));
        }
    }
    
    printf("Answer Section: %d\n",message->answerNum);
    for (int i = 0; i < message->answerNum; i++) {
        ResourceRecord * r = message->answers + i;
        print_resource_record(r);
    }
    
    printf("Authority Section: %d\n", message->authorityNum);
    for (int i = 0; i < message->authorityNum; i++) {
        ResourceRecord * r = message->authorities + i;
        print_resource_record(r);
    }
    
    printf("Additional Section: %d\n", message->additionalNum);
    for (int i = 0; i < message->additionalNum; i++) {
        ResourceRecord * r = message->additionals + i;
        print_resource_record(r);
    }
    
    printf("<---- DNS 报文结束 ----> \n");
}


void trace_message(Message * message){
    
    Header * head = (Header *) message;

    
    if (head->answerNum) {
        printf("Answer Section\n");
    }
    for (int i = 0; i < head->answerNum; i++) {
        ResourceRecord * r = message->answers + i;
        printf("{%d} ", i);
        print_resource_record(r);
    }
    
    
    if (head->authorityNum) {
        printf("Authority Section\n");
    }
    for (int i = 0; i < head->authorityNum; i++) {
        ResourceRecord * r = message->authorities + i;
        print_resource_record(r);
    }
    
    
    if (head->additionalNum) {
        printf("Additional Section\n");
    }
    for (int i = 0; i < head->additionalNum; i++) {
        ResourceRecord * r = message->additionals + i;
        print_resource_record(r);
    }
    printf("\n");
}



void initMessage(Message * message){
    if (!message) { return; }
    bzero(message, sizeof(Message));
}

Message * newMessage(){
    Message * mesg = calloc(1, sizeof(Message));
    initMessage(mesg);
    return mesg;
}













int getTypeIntValue(char * str, const char ** set, int setsize){
    for (int i = 1; i < setsize; i++) {
        if (strcmp(str, set[i]) == 0) {
            return i;
        }
    }
    fprintf(stderr, "cache_parse(): no such value : %s\n", str);
    return -1;
}

void print_cache_record(DataBaseRecord * r){
    char * p = getNormalStringFromURL(r->name);
    printf("Record: %s %d %s %s %s\n", p, r->ttl, DNS_CLASS_SET[r->class], DNS_TYPE_SET[r->type], r->record);
    free(p);
};

void print_db(DataBase * db){
    for (int i = 0; i < db->len; i++) {
        print_cache_record(db->records + i);
    }
};

char * getFormatedURLFromNormalURL(char * normal_url){
    Buffer s; initBuffer(&s);
    char * url = strdup(normal_url);
    char * tok = strtok(url, ".");
    uint8_t len = 0;
    while (tok) {
        len = (uint8_t) strlen(tok);
        put1Byte(&s, len);
        putBytes(&s, tok, len);
        tok = strtok(NULL, ".");
    }
    len = 0;
    put1Byte(&s, len);
    free(url);
    char * ret = strdup(s.buffer);
    return ret;
}

void initdb(DataBase * db, FILE * fs){
    
    if (fs == NULL) { return ; }
    
    db->len = 0;
    
    bzero(db, sizeof(DataBase));
    
    char * b = malloc(1024);
    
    int i = 0;
    for (i = 0; !feof(fs); i++) {
        
        for (int j = 0; j < 1024; j++) { b[j] = 0; }
        fscanf(fs, "%s", b);
        if (strlen(b) == 0) { break; }
        printf("添加数据库资料   %s\n", b);
        
        DataBaseRecord * r = & db->records[i];
        
        
        char * last = b;
        char * token = strtok_r(b, ",", &last);
        r->original_name = strdup(token);
        r->name = getFormatedURLFromNormalURL(token);
        
        
        token = strtok_r(NULL, ",", &last);
        r->ttl = atoi(token);
        
        
        token = strtok_r(NULL, ",", &last);
        r->class = getTypeIntValue(token, DNS_CLASS_SET, sizeof(DNS_CLASS_SET)/sizeof(char*));
        
        
        token = strtok_r(NULL, ",", &last);
        r->type = getTypeIntValue(token, DNS_TYPE_SET, sizeof(DNS_TYPE_SET)/sizeof(char *));
        
        
        token = strtok_r(NULL, ",", &last);
        
        r->record = strdup(token);
        
        char * mx_pref = NULL;
        char * mx_addr = NULL;
        if (r->type == DNS_RECORD_TYPE_A) {
            r->data.a_record.addr = inet_addr(r->record);
        }else if (r->type == DNS_RECORD_TYPE_MX){
            mx_pref = strtok(token, ":");
            mx_addr = strtok(NULL, ":");
            mx_addr = getURLFromFileString(mx_addr);
            uint16_t pref = atoi(mx_pref);
            r->data.mx_record.exchange = mx_addr;
            r->data.mx_record.preference = pref;
        }else if (r->type == DNS_RECORD_TYPE_CNAME){
            r->data.name_record.name = getURLFromFileString(r->record);
        }
        db->len += 1;
        
    }
}

DataBase * newdb(const char * fname){
    FILE * fs = fopen(fname, "r");
    DataBase * db = calloc(1 , sizeof(DataBase));
    
    initdb(db, fs);
    return db;
}

void savedb(DataBase * db, FILE * fs){
    for (int i = 0; i < db->len; i++) {
        DataBaseRecord * r = & db->records[i];
        fprintf(fs, "%s,", r->name);
        fprintf(fs, "%d,", r->ttl);
        fprintf(fs, "%s,", DNS_CLASS_SET[r->class]);
        fprintf(fs, "%s,", DNS_TYPE_SET[r->type]);
        fprintf(fs, "%s", r->record);
        if (i != db->len - 1) { fprintf(fs, "\n"); }
    }
}

void addrecord_db(DataBase * db, const char * name, uint32_t ttl, int class, int type, const char * record){
    DataBaseRecord * r = &db->records[db->len];
    r->name = strdup(name);
    r->ttl = ttl;
    r->class = class;
    r->type = type;
    r->record = strdup(record);
    db->len++;
}

DataBaseRecord * addrecord_db_t(DataBase * db, ResourceRecord * record){
    DataBaseRecord * r = &db->records[db->len];
    r->name = strdup(record->name);
    r->ttl = record->rttl;
    r->class = record->rclass;
    r->type = record->rtype;
    
    
    if (r->type == DNS_RECORD_TYPE_A) {
        r->data.a_record.addr = record->rdata.a_record.addr;
    }else if (r->type == DNS_RECORD_TYPE_MX){
        r->data.mx_record.exchange   = strdup(record->rdata.mx_record.exchange);
        r->data.mx_record.preference = record->rdata.mx_record.preference;
    }else if (r->type == DNS_RECORD_TYPE_CNAME || r->type == DNS_RECORD_TYPE_NS || r->type == DNS_RECORD_TYPE_PTR){
        r->data.name_record.name = strdup(record->rdata.name_record.name);
    }
    
    
    Buffer * s = newBuffer();
    if (r->type == DNS_RECORD_TYPE_A) {
        struct sockaddr_in addr;
        addr.sin_addr.s_addr = record->rdata.a_record.addr;
        r->record = inet_ntoa(addr.sin_addr);
    }else if (r->type == DNS_RECORD_TYPE_MX){
        char preference = record->rdata.mx_record.preference + '0';
        putBytes(s, (char *) &preference, sizeof(char));
        put1Byte(s, (uint8_t) ':');
        char * urlstring = getNormalStringFromURL(record->rdata.mx_record.exchange);
        putBytes(s, (char *) urlstring, strlen(urlstring));
        r->record = strdup(s->buffer);
    }else if (r->type == DNS_RECORD_TYPE_CNAME){
        r->record = getNormalStringFromURL(record->rdata.name_record.name);
    }
    
    freeBuffer(s);
    
    db->len++;
    return r;
}

int findQuestionInDB_i(DataBase * db, int start, Question * q){
    for (int i = start; i < db->len; i++) {
        int a = (strcmp(db->records[i].name, q->QName) == 0);
        int b = (db->records[i].type == q->QType);
        int c = (db->records[i].class == q->QClass);
        if (a && b && c) {
            return i;
        }
    }
    return -1;
}

int search_db_identical_record_fromindex(DataBase *db, int start, ResourceRecord * r){
    for (int i = start; i < db->len; i++) {
        int a = (strcmp(db->records[i].name, r->name) == 0);
        int b = (db->records[i].type == r->rtype);
        int c = (db->records[i].class == r->rclass);
        int d = (b && r->rtype == DNS_RECORD_TYPE_MX) ? strcmp(r->rdata.mx_record.exchange, db->records[i].data.mx_record.exchange) == 0 : 1;
        if (a && b && c && d) {
            return i;
        }
    }
    return -1;
}

int findQuestionAnswerinDB(DataBase * db, Question * q){
    return findQuestionInDB_i(db, 0, q);
}

int findResourceRecordMatchInDB(DataBase * db, ResourceRecord * r){
    Question q;
    q.QClass = r->rclass;
    q.QType = r->rtype;
    q.QName = r->name;
    return findQuestionInDB_i(db, 0, &q);
}

int findNameServerInDB(DataBase * db, int start, Question * q){
    for (int i = start; i < db->len; i++) {
        char * substring = strstr(q->QName, db->records[i].name);
        int a = substring ? 1 : 0;
        int b = db->records[i].type == DNS_RECORD_TYPE_A;
        int c = substring && strlen(substring) == strlen(db->records[i].name);
        
        if (a && b && c) {
            return i;
        }
    }
    return -1;
}


void cacheRR(DataBase * db, ResourceRecord * r){
    if (search_db_identical_record_fromindex(db, 0, r) == -1) {
        DataBaseRecord * dbr = addrecord_db_t(db, r);
        printf("数据库缓存记录: "); print_cache_record(dbr);
    }
}


void add_resource(ResourceRecord * mr, DataBaseRecord * r){
    mr->rclass = r->class;
    mr->name = strdup(r->name);
    mr->rttl = r->ttl;
    mr->rtype = r->type;
    
    switch (r->type) {
        case DNS_RECORD_TYPE_A:
            mr->rdata.a_record.addr = inet_addr(r->record);
            mr->rdlength = sizeof(in_addr_t);
            break;
        case DNS_RECORD_TYPE_CNAME:
            if (strcmp(r->data.name_record.name, "") == 0) {
                mr->rdata.name_record.name = strdup("");
                mr->rdlength = 1;
                break;
            }
            mr->rdata.name_record.name = strdup(r->data.name_record.name);
            mr->rdlength = strlen(r->data.name_record.name) + 1;
            break;
        case DNS_RECORD_TYPE_MX:
            mr->rdata.mx_record.exchange = strdup(r->data.mx_record.exchange);
            mr->rdata.mx_record.preference = r->data.mx_record.preference;
            mr->rdlength = strlen(mr->rdata.mx_record.exchange) + 1 + sizeof(uint16_t);
            break;
        default:
            break;
    }
}

void add_answer(Message * msg, DataBaseRecord * r){
    ResourceRecord * mr = &msg->answers[msg->answerNum++];
    add_resource(mr, r);
}

void add_authority(Message * msg, DataBaseRecord * r){
    ResourceRecord * mr = &msg->authorities[msg->authorityNum++];
    add_resource(mr, r);
}

void add_additional(Message * msg, DataBaseRecord * r){
    ResourceRecord * mr = &msg->additionals[msg->additionalNum];
    msg->additionalNum++;
    add_resource(mr, r);
}


int fetchAnswerInDB(Message * message, DataBase * db){
    int searchedItems = 0;
    int return_code = -1;
    int item = 0;
    
    
    Question * q = message->questions;
    int type = q->QType;
    while ((item = findQuestionInDB_i(db, item, q)) != -1) {
        message->headerAA = 1;
        return_code = 1;
        if (type == DNS_RECORD_TYPE_A || type == DNS_RECORD_TYPE_CNAME) {
            add_answer(message, & db->records[item]);
        } else if (type == DNS_RECORD_TYPE_MX){
            DataBaseRecord * r = & db->records[item];
            add_answer(message, r);
            
            Question p; p.QClass =  DNS_CLASS_TYPE_IN;
            p.QType  =  DNS_RECORD_TYPE_A;
            p.QName  =  strdup(r->data.mx_record.exchange);
            int ptem = findQuestionAnswerinDB(db, &p); assert(ptem >= 0);
            add_additional(message, &db->records[ptem]);
        }
        
        searchedItems ++;
        item ++;
    }
    
    
    if (searchedItems <= 0) {
        return_code = 0;
        if ((item = findNameServerInDB(db, 0, q)) != -1) {
            add_authority(message, & db->records[item]);
            searchedItems ++;
            return return_code;
        }
        
    }
    
    
    if (searchedItems <= 0) {
        return return_code = -1;
    }
    
    return return_code;
}



void initBuffer(Buffer * s){
    bzero( (void *) s, sizeof(Buffer));
}

Buffer * newBuffer(){
    Buffer * s = calloc(1, sizeof(Buffer));
    return s;
}

void freeBuffer(Buffer * s){
    free(s);
}

int putBytes(Buffer * des, char * src, size_t size){
    memcpy(&des->buffer[des->content_len], src, size);
    des->content_len += size;
    return (int) size;
}

int getBytes(char * des, Buffer * src, size_t size){
    if (src->cursor_pos > src->content_len) {
        printf("Parse Error: %d (> %d)\n", src->cursor_pos, src->content_len);
    }
    memcpy(des, &src->buffer[src->cursor_pos], size);
    src->cursor_pos += size;
    return (int) size;
}


int put1Byte(Buffer * des, uint8_t src){
    return putBytes(des, (char *) & src, sizeof(uint8_t));
}

int put2Bytes(Buffer * des, uint16_t src){
    src = htons(src);
    return putBytes(des, (char *) & src, sizeof(uint16_t));
}

int put4Bytes(Buffer * des, uint32_t src){
    src = htonl(src);
    return putBytes(des, (char *) & src, sizeof(uint32_t));
}


uint8_t get1Byte(Buffer * src){
    uint8_t t; getBytes((char *) &t, src, sizeof(uint8_t)); return t;
}

uint16_t get2Bytes(Buffer * src){
    uint16_t t; getBytes((char *) &t, src, sizeof(uint16_t));
    return ntohs(t);
}

uint32_t get4Bytes(Buffer * src){
    uint32_t t; getBytes((char *) &t, src, sizeof(uint32_t));
    return ntohl(t);
}


int getURLString(char * des, Buffer * src){
    uint8_t len;
    int original_pos = src->cursor_pos;
    while ((len = get1Byte(src)) != 0) {
        des[0] = len; des++;
        getBytes(des, src, len); des += len;
    }
    des[0] = 0;
    
    return src->cursor_pos - original_pos;;
}

int putURLString(Buffer * des, char * src){
    uint8_t len;
    const char * src_p = src;
    
    len = src[0];
    while (len != 0) {
        put1Byte(des, len); src++;
        putBytes(des, src, len); src += len;
        len = src[0];
    }
    
    put1Byte(des, len);
    return (int)(src - src_p);
}

char * getURLString_allocated(Buffer * src){
    char * url = calloc(256, sizeof(char));
    getURLString(url, src);
    char * ret = strdup(url);
    free(url);
    return ret;
}

void cout_TCP_Header(Buffer * s){
    put2Bytes(s, 0);
}

void cin_recordData(ResourceRecord * r, Buffer * s){
    RecordData * d = & r->rdata;
    int type = r->rtype;
    switch (type) {
        case DNS_RECORD_TYPE_A:
            d->a_record.addr = ntohl(get4Bytes(s));
            break;
        case DNS_RECORD_TYPE_MX:
            d->mx_record.preference = get2Bytes(s);
            d->mx_record.exchange = getURLString_allocated(s);
            break;
        case DNS_RECORD_TYPE_CNAME:
            d->name_record.name = getURLString_allocated(s);
            break;
            
        default:
            break;
    }
}

void cout_recordData(Buffer * s, ResourceRecord * r){
    RecordData * d = & r->rdata;
    int type = r->rtype;
    
    switch (type) {
        case DNS_RECORD_TYPE_A:
            put4Bytes(s, htonl(d->a_record.addr));
            break;
        case DNS_RECORD_TYPE_MX:
            put2Bytes(s, d->mx_record.preference);
            putURLString(s, d->mx_record.exchange);
            break;
        case DNS_RECORD_TYPE_CNAME:
            putURLString(s, d->name_record.name);
            break;
        default:
            break;
    }
    
}


uint16_t packHeaderFlags(Header * head){
    uint16_t flags = 0;
    flags |= head->headerQR ? 0x8000 : 0;
    flags |= head->headerOpcode << 11;
    flags |= head->headerAA ? 0x0400 : 0;
    flags |= head->headerTC ? 0x0200 : 0;
    flags |= head->headerRD ? 0x0100 : 0;
    flags |= head->headerRA ? 0x0080 : 0;
    flags |= head->headerrcode;
    return flags;
}

void unpackHeaderFlags(Header * head, uint16_t flags){
    head->headerQR     = flags & 0x8000 ? 1 : 0;
    head->headerOpcode = (flags & 0x7800) >> 11;
    head->headerAA     = flags & 0x0400 ? 1 : 0;
    head->headerTC     = flags & 0x0200 ? 1 : 0;
    head->headerRD     = flags & 0x0100 ? 1 : 0;
    head->headerRA     = flags & 0x0080 ? 1 : 0;
    head->headerrcode  = flags & 0x000f;
}


void cin_Message(Message * mesg, Buffer * s){
    
    
    Header * head = (Header *) mesg;
    cin_Header(head, s);
    
    
    for (int i = 0; i < mesg->queryNum; i++) {
        Question * q = & mesg->questions[i];
        cin_Question(q, s);
    }
    
    
    for (int i = 0; i < mesg->answerNum; i++) {
        ResourceRecord * r = & mesg->answers[i];
        cin_ResourceRecord(r, s);
    }
    
    for (int i = 0; i < mesg->authorityNum; i++) {
        ResourceRecord * r = & mesg->authorities[i];
        cin_ResourceRecord(r, s);
        
    }
    
    for (int i = 0; i < mesg->additionalNum; i++) {
        ResourceRecord * r = & mesg->additionals[i];
        cin_ResourceRecord(r, s);
    }
    
    
}

void cin_Header(Header * head, Buffer * s){
    head->headerID = get2Bytes(s);
    uint16_t flags = get2Bytes(s);
    unpackHeaderFlags(head, flags);
    head->queryNum = get2Bytes(s);
    head->answerNum = get2Bytes(s);
    head->authorityNum = get2Bytes(s);
    head->additionalNum = get2Bytes(s);
}

void cin_Question(Question * q, Buffer *s){
    q->QName = getURLString_allocated(s);
    q->QType = get2Bytes(s);
    q->QClass = get2Bytes(s);
}

void cin_ResourceRecord(ResourceRecord * r, Buffer * s){
    r->name = getURLString_allocated(s);
    r->rtype = get2Bytes(s);
    r->rclass = get2Bytes(s);
    r->rttl = get4Bytes(s);
    r->rdlength = get2Bytes(s);
    cin_recordData(r, s);
    
}

/* Reader Functions */

void cout_Message(Buffer * s, Message * mesg){
    
    
    Header * head = (Header *) mesg;
    cout_Header(s, head);
    
    
    for (int i = 0; i < mesg->queryNum; i++) {
        Question * q = & mesg->questions[i];
        cout_Question(s, q);
    }
    
    
    for (int i = 0; i < mesg->answerNum; i++) {
        ResourceRecord * r = & mesg->answers[i];
        cout_ResourceRecord(s, r);
    }
    
    for (int i = 0; i < mesg->authorityNum; i++) {
        ResourceRecord * r = & mesg->authorities[i];
        cout_ResourceRecord(s, r);
    }
    
    for (int i = 0; i < mesg->additionalNum; i++) {
        ResourceRecord * r = & mesg->additionals[i];
        cout_ResourceRecord(s, r);
    }
    
}

void cout_Header(Buffer * s, Header * head){
    put2Bytes(s, head->headerID);
    uint16_t flags = packHeaderFlags(head);
    put2Bytes(s, flags);
    put2Bytes(s, head->queryNum);
    put2Bytes(s, head->answerNum);
    put2Bytes(s, head->authorityNum);
    put2Bytes(s, head->additionalNum);
}

void cout_Question(Buffer * s, Question * q){
    putURLString(s, q->QName);
    put2Bytes(s, q->QType);
    put2Bytes(s, q->QClass);
}

void cout_ResourceRecord(Buffer * s, ResourceRecord * r){
    putURLString(s, r->name);
    put2Bytes(s, r->rtype);
    put2Bytes(s, r->rclass);
    put4Bytes(s, r->rttl);
    put2Bytes(s, r->rdlength);
    cout_recordData(s, r);
}

ssize_t DNSPacketSend_tcp(int sock, Buffer * buf){
    *((uint16_t *) buf->buffer) =  (uint16_t) buf->content_len;
    return SendTCP(sock, buf->buffer, buf->content_len);
}


ssize_t DNSPacketRecv_tcp(int sock, Buffer * buf){
    buf->content_len = (int) RecvTCP(sock, buf->buffer, 4096);
    get2Bytes(buf);
    return buf->content_len;
}

ssize_t DNSPacketSend_udp(int sock, Buffer * buf, struct sockaddr_in * addr){
    ssize_t sc = sendto(sock, (void *)(buf->buffer), buf->content_len, 0, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
    return sc;
}

ssize_t DNSPacketRecv_udp(int sock, Buffer * buf, struct sockaddr_in * addr){
    socklen_t t = sizeof(struct sockaddr_in);
    ssize_t rc = (buf->content_len = (int) recvfrom(sock, (void *)(buf->buffer), 4096, 0, (struct sockaddr *)addr, &t));
    return rc;
}


void tcpMessageSend(int sock, Message * mesg){
    Buffer * buf = newBuffer();
    cout_TCP_Header(buf);
    cout_Message(buf, mesg);
    *((uint16_t *) buf->buffer) =  (uint16_t) buf->content_len;
    send(sock, (void *)buf->buffer, buf->content_len, 0);
    freeBuffer(buf);
}

Message * tcpMessageRecv(int sock){
    Message * mesg = newMessage();
    Buffer * buf = newBuffer();
    if ((buf->content_len = (int) recv(sock, (void *) buf->buffer, 4096, 0)) < 0) {
        buf->content_len = -1;
        return NULL;
    } else{
        get2Bytes(buf);
    }
    cin_Message(mesg, buf);
    return mesg;
}

void udpMessageSend(int sock, Message * mesg, struct sockaddr_in * toAddr){
    Buffer * buf = newBuffer();
    cout_Message(buf, mesg);
    if (sendto(sock, (void *)(buf->buffer), buf->content_len, 0, (const struct sockaddr *)toAddr, sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "sendto() 信息空\n");
        return ;
    }else{
        free(buf);
    }
    
    return ;
}


Message * udpMessageRecv(int sock, struct sockaddr_in ** fromAddr){
    Message * mesg = newMessage();
    Buffer * s = newBuffer();
    if (!fromAddr) {
        fromAddr = calloc(1, sizeof(struct sockaddr_in *));
    }
    *fromAddr = newSocketaddr_t(0, 0);
    if (DNSPacketRecv_udp(sock, s, *fromAddr) < 0) {  return NULL; }
    cin_Message(mesg, s);
    return mesg;
}

