#include "common/header.h"
#include "common/const.h"
#include "client/client_util.c"
#include "common/getmd5.c"
#define true 1
#define false 0

void signalHandler(int signo);
void error_handling(char * message);
void getPasv(int clnt_sock, int *dpt_sock, char* inst_buf);
void sendClient(int sock);
typedef int bool;
bool pasv_check = false;

int main(int argc, char ** argv){
	int sock = -1,dpt_sock=-1, loop=1, i=0;
	char** inst_sep_1;
	char inst_buf[BUFFER_SIZE];
	char type_inst[BUFFER_SIZE];
	char temp_buf[BUFFER_SIZE];
	char** arg;
	struct sockaddr_in addr;

	if (argc != 3) {
		printf("Usage: %s <IP> <port> \n", argv[0]);
		exit(1);
	}

	arg = (char **)malloc(sizeof(char *) * 50);
    for(i=0; i<50; i++){
        arg[i] = (char *)malloc(sizeof(char) * 50);
    }
    i=0;

	pasv_check = false;
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1) error_handling("socket() error");
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1){
		error_handling("conn() error");
	}

	recv_msg(sock, inst_buf, BUFFER_SIZE);
	printf("%s",inst_buf);
	sendClient(sock);
	while(loop){
		printf("ftp> ");
    	fgets(type_inst,sizeof(type_inst),stdin);
    	RTRIM(type_inst);
		strcpy(temp_buf, type_inst); //Data 처리용

    	inst_sep_1 = separateInst_(temp_buf);
		CMD_TYPE cmd = (CMD_TYPE)inst_sep_1[0];
		for(i=1; i<50; i++){
			if(strstr(inst_sep_1[i], "EOF") != NULL){
				i=0;
				break;
			}
			strcpy(arg[i-1],inst_sep_1[i]);
		}

		if(!pasv_check){
			if(cmd == LIST || cmd == RETR || cmd == STOR)
				getPasv(sock, &dpt_sock, inst_buf);
			if(cmd == STOR){
				char* list_temp;
				strcpy(temp_buf, "LIST\r\n");
				msg_send(temp_buf, sock);
				recv_msg(sock, inst_buf, BUFFER_SIZE);
				list_temp = list_char(&dpt_sock);
				if(list_temp  == NULL){
					printf("list() execute error!\n");
				}
				if(serverExists(list_temp,arg[0]) != 0){
					printf("==== The file exists on the server. ====\n");
					cmd = NOOP;
					strcpy(type_inst,"NOOP\r\n");
				}
				recv_msg(sock, inst_buf, BUFFER_SIZE);
				getPasv(sock, &dpt_sock, inst_buf);
			}
		}

		strcat(type_inst, "\r\n");
		msg_send(type_inst, sock);
		recv_msg(sock, inst_buf, BUFFER_SIZE);
		printf("%s",inst_buf);

		switch (cmd) {
    	    case PASV:
    	        if(pasv(&dpt_sock, inst_buf) < 0){
					printf("PASV() Error!\n");
				}else{
					pasv_check = true;
				}
            break;

    	    case LIST:
				if(!pasv_check) getPasv(sock, &dpt_sock, inst_buf);
    	        if(list(&dpt_sock)< 0){
					printf("list() execute error!\n");
					pasv_check = false;
				}else{
					recv_msg(sock, inst_buf, BUFFER_SIZE);
					printf("%s",inst_buf);
					pasv_check = false;
				}
            break;

        	case RETR:
				if(!pasv_check) getPasv(sock, &dpt_sock, inst_buf);
            	if(retr(&dpt_sock, arg) < 0){
					printf("retr() execute error!\n");
					pasv_check = false;
				}
				else{
					recv_msg(sock, inst_buf, BUFFER_SIZE);
					printf("%s",inst_buf); //226 Data transfer ends.
					pasv_check = false;
					//MD5//
					printf("MD5 Checksum Caculating....");
                	recv_msg(sock, inst_buf, BUFFER_SIZE); //MD5 receive
                	char* md5 = getMD5(arg[0]);
					printf("OK\n");
                	if(strstr(inst_buf, md5)== NULL){
						printf("MD5 Hash do not match.\n");
                	}else{
						printf("File validation passed.\n");
             		}
				}
            break;

        	case STOR:
				if(!pasv_check) getPasv(sock, &dpt_sock, inst_buf);
            	if(stor(&dpt_sock, arg) < 0){
					printf("stor() execute error!\n");
					pasv_check = false;
				}
				else{
					recv_msg(sock, inst_buf, BUFFER_SIZE);
					printf("%s",inst_buf);
					pasv_check = false;
					//MD5//
					printf("MD5 Checksum Caculating....");
					char* hash = getMD5(arg[0]);
					sprintf(type_inst,"%s",hash);
					printf("Sending....");
					msg_send(type_inst, sock);
					printf("OK\n");
					recv_msg(sock, inst_buf, BUFFER_SIZE);
					printf("%s",inst_buf);
				}
            break;

			case QUIT:
			loop = 0;
			break;
    	}
		//fseek(stdin,NULL,SEEK_END); //fflush stdin buffer
		strcpy(type_inst,"");
	}

    return 0;
}

void error_handling(char * message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(EXIT_FAILURE);
}
void signalHandler(int signo){
	//shutdown(serv_sock, SHUT_RDWR);
	//shutdown(clnt_sock, SHUT_RDWR);
	exit(EXIT_SUCCESS);
}
void getPasv(int clnt_sock, int *dpt_sock, char* inst_buf){
	char buf[50];
	strcpy(buf,"PASV\r\n");
	msg_send(buf, clnt_sock);
	recv_msg(clnt_sock, inst_buf, BUFFER_SIZE);
	printf("%s",inst_buf);
	if(pasv(dpt_sock, inst_buf) < 0) printf("getPASV() Error!\n");
	else pasv_check = true;
}
void sendClient(int sock){
	char temp_buf[BUFFER_SIZE], inst_buf[BUFFER_SIZE];
	strcpy(temp_buf, "OPTS CLIENT\r\n");
	msg_send(temp_buf, sock);
	recv_msg(sock, inst_buf, BUFFER_SIZE);
}