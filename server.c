#define _GNU_SOURCE
#include "common/header.h"
#include "common/const.h"
#include "common/folder_util.c"
#include "common/util.c"
#include "common/getmd5.c"
#include <sqlite3.h>
#include <sched.h>

#define CLIENT_LIMIT 100
#define MAX_READ_BUF 200
#define true 1
#define false 0
#define DB_NAME "account.db"
typedef int bool;
pthread_mutex_t mutx;

void signalHandler(int signo);
void error_handling_exit(char * message);
void msg_send(char *msg, int sock);
size_t msg_read(char *msg, int sock);
void *clnt_connection(void *arg);
char **separateInst(char *msg);
void disconnect(int clnt_sock, char* id);
void ftpserver_sendfile(FILE* file, int clnt_sock);
void ftpserver_recvfile(FILE* file, int clnt_sock);
void LogAppend(char* content);
void DB_Initialization();
int authority_Check(char* id, char* pw);
int clnt_number = 0;
int *clnt_sock_pi, *clnt_sock_dtp;
int serv_sock_pi, serv_sock_dtp;
char working_path[200];

int main(int argc, char ** argv){
    int clnt_sock;
	struct sockaddr_in serv_addr[2]; //serv_addr[0] = PI, serv_addr[1] = DTP
    struct sockaddr_in clnt_addr;
	int clnt_addr_size;
    char inst_buf[MAX_READ_BUF];
    pthread_t thread;

    if(argc != 3){
        argv[1] = (char *)malloc(sizeof(char)*3);
        argv[2] = (char *)malloc(sizeof(char)*3);
		strcpy(argv[1],"21"); //Server PI Port
        strcpy(argv[2],"20"); //Server DTP Port
		printf("Port number is not set.\nStart automatically with Server PI port %s and Server DTP %s.\n",argv[1],argv[2]);
        sleep(2);
	}
    if(exists(DB_NAME) == false){
        DB_Initialization(); //Administrator access account "Anonymous","hs0123"
    }
    srand(time(NULL));
    getcwd(working_path,200);
    if (pthread_mutex_init(&mutx, NULL)) error_handling_exit("mutex init() error");
    signal(SIGINT, (void *)signalHandler);
    signal(SIGSEGV, (void *)signalHandler);
    clnt_sock_pi = (int *)malloc(sizeof(int) * CLIENT_LIMIT);
    clnt_sock_dtp = (int *)malloc(sizeof(int) * CLIENT_LIMIT);

    int option = 1;
    serv_sock_pi = socket(PF_INET, SOCK_STREAM, 0);
    serv_sock_dtp = socket(PF_INET, SOCK_STREAM, 0);
	setsockopt(serv_sock_pi, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); //Prevent TCP from TIME_WAIT state.
    setsockopt(serv_sock_dtp, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	if(serv_sock_pi == -1 || serv_sock_dtp == -1) error_handling_exit("socket() error");

	memset(&serv_addr[0], 0, sizeof(serv_addr[0]));
	memset(&serv_addr[1], 0, sizeof(serv_addr[1]));
    serv_addr[1].sin_family = serv_addr[0].sin_family = AF_INET;
	serv_addr[1].sin_addr.s_addr = serv_addr[0].sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr[0].sin_port = htons(atoi(argv[1]));
	serv_addr[1].sin_port = htons(atoi(argv[2]));
    
    if(bind(serv_sock_pi, (struct sockaddr * ) &serv_addr[0], sizeof(serv_addr[0])) == -1)
		error_handling_exit("pi bind() error");
    bind(serv_sock_dtp, (struct sockaddr * ) &serv_addr[1], sizeof(serv_addr[1]));
    if(listen(serv_sock_pi, CLIENT_LIMIT) == -1) error_handling_exit("pi listen() error");
    listen(serv_sock_dtp, CLIENT_LIMIT);
    printf("The server event log is stored in the root directory.\n");
  	printf("Start the server.\n");
    LogAppend("Start the server.\r\n");

    while(1){
        clnt_addr_size = sizeof(clnt_addr);
  	    clnt_sock = accept(serv_sock_pi, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) error_handling_exit("accept() error");
        pthread_mutex_lock(&mutx);
  	    clnt_sock_pi[clnt_number++] = clnt_sock;
        pthread_mutex_unlock(&mutx);
        printf("Server Protocol Interpreter Accept!\n");
        printf("Client IP=%s, Number=%d\n",inet_ntoa(clnt_addr.sin_addr),clnt_number);
        sprintf(inst_buf,"Client IP=%s\r\n",inet_ntoa(clnt_addr.sin_addr));
        LogAppend(inst_buf);
        LogAppend("Server Protocol Interpreter Accept!\r\n");
        unshare(CLONE_FS); //Unshare file system attributes

        pthread_create(&thread, NULL, clnt_connection, (void *)clnt_sock);
    }
    
    return 0;
}

void *clnt_connection(void *arg){
    int clnt_sock = (int) arg;
	int str_len = 0;
	char inst_buf[MAX_READ_BUF];
    char temp[200];
    char id[30], pw[30];
    char** inst_sep;
	int i, loop=1;
    int port_dtp = get_port(serv_sock_dtp);
    int addr = get_addr(clnt_sock);
    bool enhance_client = false;
    bool Authorized_user = false;
    FILE* pipe, *file;
    sprintf(inst_buf, REP_WELCOME, RC_CMD_SUCC);
    msg_send(inst_buf,clnt_sock);
    chdir("/");

    while(loop){
        int dirfd = open("/", O_RDONLY | O_CLOEXEC);
        int read_len = (long unsigned int)msg_read(inst_buf, clnt_sock);
        if(read_len <= 0){
            disconnect(clnt_sock, id);
            break;
        }

        inst_sep = separateInst(inst_buf);
        CMD_TYPE cmd = get_cmd(inst_sep[0]);
        printf("cmd(%s)(%s) = %d\n",inst_buf,inst_sep[1],(int)cmd);
        sprintf(temp,"cmd(%s)(%s)\r\n",inst_buf,inst_sep[1]);
        LogAppend(temp);
        if(Authorized_user == false && (cmd != USER && cmd != PASS && cmd != OPTS && cmd != QUIT)){ //Commands accessible without authority.
            cmd = NOOP;
        }

        switch(cmd){
            case QUIT:
            sprintf(inst_buf,REP_CLOSE_DATA_CONN,RC_CLOSE_DATA_CONN);
            msg_send(inst_buf, clnt_sock);
            disconnect(clnt_sock, id);
            loop = 0;
            break;

            case USER:{
                strcpy(id, inst_sep[1]);
                sprintf(inst_buf,REP_PASS_REQ,RC_PASS_REQ);
                msg_send(inst_buf, clnt_sock);
                break;
            }
            case PASS:{
                strcpy(pw, inst_sep[1]);
                int res = authority_Check(id, pw);
                if(res == true){
                    sprintf(inst_buf,REP_USER_LOGIN,RC_USER_LOGIN);
                    msg_send(inst_buf, clnt_sock);
                    Authorized_user = true;
                }else{
                    sprintf(inst_buf,REP_LOGIN_FAIL,RC_LOGIN_FAIL);
                    msg_send(inst_buf, clnt_sock);
                }
                break;
            }

            case PWD:
            case XPWD:
            getcwd(temp, 200);
            sprintf(inst_buf,REP_PWD, RC_PATH_CREATED, temp);
            msg_send(inst_buf, clnt_sock);
            break;

            case CWD:
            case XCWD:{
                //!chdir(inst_sep[1])
                if(!chdir(inst_sep[1])){
                    sprintf(inst_buf,REP_CWD_SUCC, RC_FILE_ACT_SUCC);
                    msg_send(inst_buf, clnt_sock);
                }else{
                    sprintf(inst_buf,REP_CWD_FAIL,RC_FILE_ACT_FAIL);
                    msg_send(inst_buf, clnt_sock);
                }
                break;
            }

            case OPTS:
            if(strstr(inst_sep[1], "CLIENT") != NULL){ //Custom client connection.
                printf("Custom client connection.\n");
                sprintf(inst_buf,"%d OK.\r\n", RC_CMD_SUCC);
                msg_send(inst_buf, clnt_sock);
                enhance_client = true;
                LogAppend("Custom client connection.\r\n");
            }else{
                sprintf(inst_buf,"%d Always in UTF8 mode.\r\n", RC_CMD_SUCC);
                msg_send(inst_buf, clnt_sock);
            }    
            break;

            case MODE:
            if(strstr(inst_sep[1],"C") != NULL){
                sprintf(inst_buf, REP_MOD_SET, RC_CMD_SUCC, inst_sep[1]);
                msg_send(inst_buf, clnt_sock);  
            }
            break;

            case TYPE:
            sprintf(inst_buf,REP_TYPE_SUCC, RC_CMD_SUCC);
            msg_send(inst_buf, clnt_sock);
            break;

            case PASV:{
                unsigned int random=0;
                random = (rand() ^ (rand() << 15));
                random = random%(65535-1024)+1024;
                //serv_sock_dtp = socket_listen(0);
                serv_sock_dtp = socket_listen((int)random);
                port_dtp = get_port(serv_sock_dtp);
                serv_sock_dtp = socket_listen(port_dtp);
                //if(serv_sock_dtp < 0) error_handling_exit("DTP socket listen() error");
                if(serv_sock_dtp < 0){
                    sprintf(inst_buf,"%d PASV ERROR\r\n",RC_CMD_NOT_IMPL);
                    msg_send(inst_buf, clnt_sock);
                }
                sprintf(inst_buf,REP_PASV_SUCC, RC_PASV_SUCC, addr&0xff, (addr>>8)&0xff, (addr>>16)&0xff, addr>>24, port_dtp>>8, port_dtp&0xff  );
                msg_send(inst_buf, clnt_sock);
                break;
            }

            case RETR:
            printf("FILE UPLOAD = %s\n", inst_sep[1]);
            file = fopen(inst_sep[1], "rb");
            if (!file) {
                sprintf(inst_buf, REP_FILE_FAIL, RC_FILE_ACT_FAIL, inst_sep[1]);
                msg_send(inst_buf, clnt_sock);
                break;
            } else {
                ftpserver_sendfile(file, clnt_sock);
            }
            if(enhance_client == true){
                sleep(2);
                char* md5 = getMD5(inst_sep[1]);
                msg_send(md5, clnt_sock);
            }
            fclose(file);
            break;

            case CDUP:
            chdir("..");
            sprintf(inst_buf,REP_CWD_SUCC, RC_FILE_ACT_SUCC);
            msg_send(inst_buf, clnt_sock);
            break;

            case DELE:
            remove(inst_sep[1]);
            sprintf(inst_buf, REP_DELE_SUCC, RC_FILE_ACT_SUCC);
            msg_send(inst_buf, clnt_sock);
            break;

            case XMKD:
            case MKD:
            mkdir(inst_sep[1], 0777);
            sprintf(inst_buf, REP_MKD_SUCC, RC_MKD_SUCC, inst_sep[1]);
            msg_send(inst_buf, clnt_sock);
            break;

            case XRMD:
            case RMD:
            rmdir(inst_sep[1]);
            sprintf(inst_buf, REP_RMD_SUCC, RC_RMD_SUCC);
            msg_send(inst_buf, clnt_sock);
            break;

            case STOR:
            pthread_mutex_lock(&mutx);
            file = fopen(inst_sep[1], "wb");
            if (!file) {
                sprintf(inst_buf, REP_FILE_FAIL, RC_FILE_ACT_FAIL, inst_sep[1]);
                msg_send(inst_buf, clnt_sock);
                break;
            } else {
                ftpserver_recvfile(file, clnt_sock);
            }
            fclose(file);
            pthread_mutex_unlock(&mutx);
            if(enhance_client == true){
                msg_read(inst_buf, clnt_sock);
                char* md5 = getMD5(inst_sep[1]);
                if(strstr(inst_buf, md5)== NULL){
                    strcpy(inst_buf,"MD5 Hash do not match.\n");
                    msg_send(inst_buf, clnt_sock);
                }else{
                    strcpy(inst_buf,"File validation passed.\n");
                    msg_send(inst_buf, clnt_sock);
                }
            }
            break;

            case SIZE:{
                struct stat buf;
    	        if( stat(inst_sep[1], &buf) <0 )
	            {
                    sprintf(inst_buf,REP_FILE_FAIL, RC_FILE_ACT_FAIL);
                    msg_send(inst_buf, clnt_sock);
		            break;
	            }
	            if(!S_ISREG(buf.st_mode))
        	    {
                    sprintf(inst_buf,REP_FILE_FAIL, RC_FILE_ACT_FAIL);
                    msg_send(inst_buf, clnt_sock);
		            break;
	            }
	            char text[1024] = {0};
	            sprintf(text,"%ld",buf.st_size);
                sprintf(inst_buf,"213 %s\r\n", text);
                msg_send(inst_buf, clnt_sock);
                break;
            }

            case NLST:{
                list_common(clnt_sock, serv_sock_dtp, 1);
                break;
            }

            case LIST:
            if(strstr(inst_sep[1], "EOF")!=NULL){
                strcpy(inst_sep[1],"");
            }
            sprintf(temp, "ls -l %s", inst_sep[1]);
            pipe = popen(temp, "r");
            ftpserver_sendfile(pipe, clnt_sock);
            pclose(pipe);
            break;

            case SYST:
            sprintf(inst_buf,"%s","215 LINUX Type: L8 Version: 201858142\r\n");
            msg_send(inst_buf, clnt_sock);
            break;

            case NOOP:
            sprintf(inst_buf,"%s","200 OK.\r\n");
            msg_send(inst_buf, clnt_sock);
            break;

            default:
            sprintf(inst_buf,REP_UNKNOWN,RC_CMD_NOT_IMPL);
            msg_send(inst_buf, clnt_sock);
            break;
        }
        free(inst_sep);
    }
    return EXIT_SUCCESS;
}
void disconnect(int clnt_sock, char* id){
    int i=0;
    char tmp[128];
    printf("Disconnect %s\n\n",id);
    sprintf(tmp, "Disconnect %s\r\n",id);
    LogAppend(tmp);
    pthread_mutex_lock(&mutx);
    for(i=0; i<clnt_number; i++){
        if(clnt_sock == clnt_sock_pi[i]){
            for (; i < clnt_number - 1; i++) clnt_sock_pi[i] = clnt_sock_pi[i + 1];
		    break;
        }
    }
    clnt_number--;
    //shutdown(clnt_sock,SHUT_RDWR);
    //shutdown(serv_sock_dtp,SHUT_RDWR);
    pthread_mutex_unlock(&mutx);
}

char **separateInst(char *msg){
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
        RTRIM(ptr);
        strcpy(data[i++], ptr);
        ptr = strtok(NULL, " ");
    }
    strcpy(data[i],"EOF");
    return data;
}

void msg_send(char *msg, int sock){
    //strcat(msg,"\r\n");
    LogAppend(msg);
    int len = strlen(msg);
    if(send(sock,msg,strlen(msg),0) < 0) disconnect(sock,"ERROR");
}

size_t msg_read(char *msg, int sock){
    memset(msg, 0, MAX_READ_BUF);
    return recv(sock, msg, MAX_READ_BUF, 0);
}

void error_handling_exit(char * message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}
void signalHandler(int signo){
    if(signo == SIGSEGV){

    }else{
	    printf("Shutdown the Server.\n");
        shutdown(serv_sock_dtp,SHUT_RDWR);
	    exit(EXIT_SUCCESS);
    }
}

void ftpserver_sendfile(FILE* file, int clnt_sock) {
    send_msg_verti(clnt_sock, REP_TRANS_BEGIN, RC_OPEN_DATA_CONN);
    clnt_sock_dtp[clnt_number] = socket_accept(serv_sock_dtp);
    printf("Data Transfer Process Accept!\n");
    send_file(clnt_sock_dtp[clnt_number], file);
    send_msg_verti(clnt_sock, REP_TRANS_END, RC_CLOSE_DATA_CONN);
    clear_socket(clnt_sock_dtp[clnt_number]);
    //clear_socket(serv_sock_dtp);
}

void ftpserver_recvfile(FILE* file, int clnt_sock) {
    send_msg_verti(clnt_sock, REP_TRANS_BEGIN, RC_OPEN_DATA_CONN);
    clnt_sock_dtp[clnt_number] = socket_accept(serv_sock_dtp);
    printf("Data Transfer Process Accept!\n");
    recv_file(clnt_sock_dtp[clnt_number], file);
    send_msg_verti(clnt_sock, REP_TRANS_END, RC_CLOSE_DATA_CONN);
    clear_socket(clnt_sock_dtp[clnt_number]);
}
void LogAppend(char* content){
    if(content == NULL) return;
    pthread_mutex_lock(&mutx);
    FILE* fp = fopen("/server.log","a+");
    if(fp == NULL){
        pthread_mutex_unlock(&mutx);
        return;
    }
    time_t t = time(NULL);
    localtime(&t);
    char data[200]="";
    sprintf(data,"[%.19s] %s",ctime(&t),content);
    fputs(data, fp);
    fclose(fp);
    pthread_mutex_unlock(&mutx);
}
int authority_Check(char* id, char* pw){
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;
    char wkd[200];
    strcpy(wkd, "");
    sprintf(wkd, "%s/%s",working_path,DB_NAME);
    pthread_mutex_lock(&mutx);
    int rc = sqlite3_open(wkd, &db);
    if (rc != SQLITE_OK){
        fprintf(stdout, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }
    char *sql = "SELECT Count(*) FROM `Account` WHERE ID = ? AND PW = ?"; 
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc == SQLITE_OK){
        sqlite3_bind_text(res, 1, id ,-1, SQLITE_STATIC);
        sqlite3_bind_text(res, 2, pw ,-1, SQLITE_STATIC);
    }
    else{
        fprintf(stdout, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    int step = sqlite3_step(res);
    if(sqlite3_column_text(res, 0) == NULL) return false;
    char *ptr = strdup((char*)sqlite3_column_text(res, 0));
    sqlite3_finalize(res);
    sqlite3_close(db);
    pthread_mutex_unlock(&mutx);
    if(ptr == NULL) return false;
    return atoi(ptr);
}
void DB_Initialization(){
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stdout, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char *sql = "CREATE TABLE Account(Num INTEGER PRIMARY KEY, ID TEXT, PW TEXT); \
    INSERT INTO Account VALUES(1, 'Anonymous', 'hs0123');\
    INSERT INTO Account VALUES(2, 'bmg0001', 'bmg1004');";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK )
    {
        fprintf(stderr, "Failed to create table\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    sqlite3_close(db);
}