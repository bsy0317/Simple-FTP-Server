#define BUFFER_SIZE 200
#define  RTRIM(_str) do  {                        \
        long pos =  strlen(_str)-1;                  \
        for (; pos>=0; pos-- ) {                     \
            if(_str[pos] == 0x0d) _str[pos] = 0x00;  \
            else  exit;                              \
        }                                            \
    } while(0)

CMD_TYPE get_cmd(char* msg){
    char list[][10] = {"USER", "PASS", "CWD", "XCWD", "CDUP", "XCUP", "QUIT", "ACCT", "SMNT", "REIN", "PORT", "PASV", "TYPE", "STRU", "MODE", "RETR", "STOR", "APPE", "LIST", "NLST", "REST"
,"ABOR", "PWD", "XPWD", "MKD", "XMKD", "RMD", "XRMD", "DELE", "RNFR", "RNTO", "SITE", "SYST", "FEAT", "SIZE", "STAT", "NOOP", "HELP", "STOU", "ALLO", "OPTS", "ERROR" };
    int i = 0;
    for(i = 0; i<sizeof(list)/10; i++){
        if(strcasecmp(msg, list[i]) == NULL){
            return (CMD_TYPE)i;
        }
    }
}
int get_addr(int sock){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getsockname(sock, (struct sockaddr *)&addr, &len);
    return addr.sin_addr.s_addr;
}
int get_port(int sock){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getsockname(sock, (struct sockaddr *)&addr, &len);
    return addr.sin_port;    
}

void send_msg_verti(int sock, const char* fmt, ...) {
    char msg[BUFFER_SIZE];
    va_list vl;
    va_start(vl, fmt);
    vsnprintf(msg, sizeof(msg), fmt, vl);
    va_end(vl);

    send(sock, msg, strlen(msg), 0);
}
void send_file(int sock, FILE* file) {
    char data[BUFFER_SIZE];
    int size;
    while ((size = fread(data, 1, BUFFER_SIZE, file)) > 0) {
        send(sock, data, size, 0);
    }
}

void recv_file(int sock, FILE* file) {
    char data[BUFFER_SIZE];
    int size;
    while ((size = recv(sock, data, BUFFER_SIZE, 0)) > 0) {
        fwrite(data, 1, size, file);
    }
}
int socket_accept(int sock) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    return accept(sock, (struct sockaddr *)&addr, &addr_len);
}
int socket_listen(int port) {
    int sock = -1;
    int option = 1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&addr, sizeof addr) < 0) return -1;
    if (listen(sock, 10) < 0) return -1;
    return sock;
}
int clear_socket(int sock){
    return close(sock);
}
int list_common(int clnt_sock,int serv_sock_dtp,int detail)
{
	DIR* dir = opendir(".");
	if(dir ==NULL)
	{
		return 0;
	}
	struct dirent* dt;
	struct stat sbuf;

    send_msg_verti(clnt_sock, REP_TRANS_BEGIN, RC_OPEN_DATA_CONN);
    int dtp_port = socket_accept(serv_sock_dtp);
	
    while( (dt = readdir(dir))!=NULL )
	{
		if(lstat(dt->d_name,&sbuf) < 0)
		{
			continue;
		}
		if(dt->d_name[0] == '.')
			continue;

		char buf[1024] = {0};

		if(detail)
		{
			const char* perms = statbuf_get_perms(&sbuf);

			int off =0;
			off += sprintf(buf,"%s ",perms);
			off +=sprintf(off + buf,"%3lu %-8d %-8d",sbuf.st_nlink,sbuf.st_uid,sbuf.st_gid);
			off +=sprintf(off + buf,"%8lu ",sbuf.st_size);

			const char* datebuf = statbuf_get_date(&sbuf);
			off +=sprintf(off + buf,"%s ",datebuf);

			if(S_ISLNK(sbuf.st_mode))
			{
				char tmp[1024] = {0};
				readlink(dt->d_name,tmp,sizeof(tmp));
				off +=sprintf(off + buf,"%s -> %s\r\n",dt->d_name,tmp);
			}
			else
			{
				off +=sprintf(off + buf,"%s\r\n",dt->d_name);
			}
			}
			else
			{
				sprintf(buf,"%s\r\n",dt->d_name);
			}
            write(dtp_port, buf, sizeof(buf));

	}
    send_msg_verti(clnt_sock, REP_TRANS_END, RC_CLOSE_DATA_CONN);
    clear_socket(dtp_port);
    //clear_socket(serv_sock_dtp);
	closedir(dir);
	return 0;
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