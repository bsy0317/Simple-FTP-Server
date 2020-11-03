#define BUFFER_SIZE 200
#define  RTRIM(_str) do  {                            \
        long pos =  strlen(_str)-1;                   \
        for (; pos>=0; pos-- ) {                      \
            if(_str[pos] == 0x0a){_str[pos] = 0x00;}  \
            else  exit;                               \
        }                                             \
    } while(0)
    
char inst_list[][10] = {"USER", "PASS", "CWD", "XCWD", "CDUP", "XCUP", "QUIT", "ACCT", "SMNT", "REIN", "PORT", "PASV", "TYPE", "STRU", "MODE", "RETR", "STOR", "APPE", "LIST", "NLST", "REST"
,"ABOR", "PWD", "XPWD", "MKD", "XMKD", "RMD", "XRMD", "DELE", "RNFR", "RNTO", "SITE", "SYST", "FEAT", "SIZE", "STAT", "NOOP", "HELP", "STOU", "ALLO", "OPTS", "ERROR" };

char **separateInst_(char *msg){
    char **data;
    char *ptr;
    int i=0;
    data = (char **)malloc(sizeof(char *) * 50);
    for(i=0; i<50; i++){
        data[i] = (char *)malloc(sizeof(char) * 50);
    }
    i=0;
    ptr = strtok(msg, " ");
    while(ptr != NULL){
        strcpy(data[i++], ptr);
        ptr = strtok(NULL, " ");
    }
    strcpy(data[i],"EOF");

    CMD_TYPE ret = ERROR;
    for (i = 0; i < (int)ERROR; i++) {
        if (!strcasecmp(data[0], inst_list[i])) {
            ret = (CMD_TYPE)i;
            break;
        }
    }
    data[0] = (char)ret;
    return data;
}
char* getDateChar(){
    char *ptr = (char *)malloc(sizeof(char)*7);
    struct tm *date;
    const time_t t = time(NULL);
    date = localtime(&t);
    sprintf(ptr,"(%d%d%d)",date->tm_hour , date->tm_min , date->tm_sec);
    return ptr;
}
char *replace(char *st, char *orig, char *repl) {
  static char buffer[4096];
  char *ch;
  if (!(ch = strstr(st, orig)))
     return st;
    strncpy(buffer, st, ch-st);  
    buffer[ch-st] = 0;
    sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
    return buffer;
}
int socket_close(int sock) {
    if (sock < 0) return -1;
    return close(sock);
}
void print_r(char* path) {
    FILE* file = fopen(path, "r");
    char c;
    while (fscanf(file, "%c", &c) != EOF)
        printf("%c", c);
    fclose(file);
}
void send_file(int sock, FILE* file) {
    char data[BUFFER_SIZE];
    int size;
    while ((size = fread(data, 1, BUFFER_SIZE, file)) > 0) {
        send(sock, data, size, 0);
    }
}

int recv_file(int sock, FILE* file) {
    char data[200];
    int size;
    while ((size = recv(sock, data, 200, 0)) > 0) {
        fwrite(data, 1, size, file);
    }
    if(size < 0){
        printf("recv_file() error!\n");
        return -1;
    }
    return 0;
}
int recv_msg(int sock, char* buf, int size) {
    memset(buf, 0, size);
    return recv(sock, buf, size, 0);
}
void msg_send(char *msg, int sock){
    //strcat(msg,"\r\n");
    int len = strlen(msg);
    if(send(sock,msg,strlen(msg),0) < 0) close(sock);
}

int pasv(int *dpt_sock, char* inst_buf) {
    int a[6];
    sscanf(inst_buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).",
            &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
    char host[BUFFER_SIZE];
    int port;
    sprintf(host, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
    port = (a[4]<<8) + a[5];

    *dpt_sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);
    if (connect(*dpt_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) return -1;
    return 0;
}

int list(int *dpt_sock) {
    char fname[BUFFER_SIZE];
    sprintf(fname, "ftp_list_%d.tmp", *dpt_sock);
    FILE* tmp_file = fopen(fname, "w");
    if(recv_file(*dpt_sock, tmp_file) < 0){
        return -1;
    }
    fclose(tmp_file);

    print_r(fname);

    char rmfunc[BUFFER_SIZE];
    strcpy(rmfunc, "rm -f ");
    strcat(rmfunc, fname);
    system(rmfunc);
    socket_close(*dpt_sock);
    return 0;
}
char* list_char(int *dpt_sock){
    char fname[BUFFER_SIZE];
    char* buffer;
    buffer = (char *)malloc(sizeof(char) * 1024);
    sprintf(fname, "ftp_list_%d.tmp", *dpt_sock);
    FILE* tmp_file = fopen(fname, "w");
    if(recv_file(*dpt_sock, tmp_file) < 0){
        return NULL;
    }
    fclose(tmp_file);

    strcpy(buffer, "");
    tmp_file = fopen(fname, "r");
    while (feof(tmp_file) == 0) {
        char str[256];
        fgets(str, 256, tmp_file);
        strcat(buffer, str);
    }

    char rmfunc[BUFFER_SIZE];
    strcpy(rmfunc, "rm -f ");
    strcat(rmfunc, fname);
    system(rmfunc);
    socket_close(*dpt_sock);
    return buffer;
}

int retr(int *dpt_sock, char** arg) {
    char data[BUFFER_SIZE];
    char temp[100] = "";
    char* ptr, *chr;
    if(exists(arg[0])){ //if the destination file already exists
        strcpy(temp, getDateChar());
        strcat(temp, arg[0]);
        strcpy(arg[0],temp);
    }
    FILE* file = fopen(arg[0], "wb");
    if(recv_file(*dpt_sock, file) < 0){
        printf("recv_file() error!\n");
        return -1;
    }
    recv_msg(*dpt_sock, data, BUFFER_SIZE);
    printf("%s",data);
    fclose(file);
    socket_close(*dpt_sock);
    return 0;
}

int stor(int *dpt_sock, char** arg) {
    FILE* file = fopen(arg[0], "rb");
    if (!file) {
        printf("ERROR / File Error\n");
        return -1;
    }

    send_file(*dpt_sock, file);
    fclose(file);
    socket_close(*dpt_sock);
    return 0;
}
int doPWD(int *dpt_sock){
    char data[BUFFER_SIZE];
    recv_msg(*dpt_sock, data, BUFFER_SIZE);
    printf("%s",data);
}
int serverExists(char* list_data, char* file_name){
    if(strstr(list_data, file_name) != NULL){
        return 1;
    }else{
        return 0;
    }
}


void get_arg(const char* command, char* arg) {
    while (*command != ' ' && *command != '\r') command++;
    if (*command != '\r') {
        command++;
        while (*command != '\r') *arg++ = *command++;
    }
    *arg = '\0';
}

int exists(const char *fname){
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}