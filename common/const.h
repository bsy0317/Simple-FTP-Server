/***
 *    ██████╗  ██████╗  ██╗ █████╗ ███████╗ █████╗  ██╗██╗  ██╗██████╗     ███████╗████████╗██████╗     ███████╗███████╗██████╗ ██╗   ██╗███████╗██████╗ 
 *    ╚════██╗██╔═████╗███║██╔══██╗██╔════╝██╔══██╗███║██║  ██║╚════██╗    ██╔════╝╚══██╔══╝██╔══██╗    ██╔════╝██╔════╝██╔══██╗██║   ██║██╔════╝██╔══██╗
 *     █████╔╝██║██╔██║╚██║╚█████╔╝███████╗╚█████╔╝╚██║███████║ █████╔╝    █████╗     ██║   ██████╔╝    ███████╗█████╗  ██████╔╝██║   ██║█████╗  ██████╔╝
 *    ██╔═══╝ ████╔╝██║ ██║██╔══██╗╚════██║██╔══██╗ ██║╚════██║██╔═══╝     ██╔══╝     ██║   ██╔═══╝     ╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██╔══╝  ██╔══██╗
 *    ███████╗╚██████╔╝ ██║╚█████╔╝███████║╚█████╔╝ ██║     ██║███████╗    ██║        ██║   ██║         ███████║███████╗██║  ██║ ╚████╔╝ ███████╗██║  ██║
 *    ╚══════╝ ╚═════╝  ╚═╝ ╚════╝ ╚══════╝ ╚════╝  ╚═╝     ╚═╝╚══════╝    ╚═╝        ╚═╝   ╚═╝         ╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚══════╝╚═╝  ╚═╝                                                                                                                                                      
 */
/* Reply message */
#define REP_WELCOME "%d Welcome to FTP Server.\r\n"                                                                    
#define REP_HELP "%d quit help user pwd cwd type pasv retr stor list\r\n"
#define REP_USER_LOGIN "%d Login successful.\r\n" 
#define REP_PWD "%d \"%s\" is the current dir.\r\n"
#define REP_CWD_SUCC "%d Change to %s.\r\n"
#define REP_CWD_FAIL "%d Failed changed to %s.\r\n" 
#define REP_TYPE_SUCC "%d Type remains unchanged.\r\n"
#define REP_PASV_SUCC "%d Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n"
#define REP_TRANS_BEGIN "%d Data transfer begins.\r\n"
#define REP_TRANS_END "%d Data transfer ends.\r\n" 
#define REP_FILE_FAIL "%d File %s unavailable.\r\n" 
#define REP_UNKNOWN "%d Command not implemented.\r\n"
#define REP_LIST_SUCC "List command success.\r\n" 
#define REP_RETR_SUCC "Retr command success.\r\n" 
#define REP_STOR_SUCC "Stor command success.\r\n"
#define REP_DELE_SUCC "%d DELE command success.\r\n"
#define REP_MKD_SUCC "%d \"%s\" - Directory successfully created.\r\n"
#define REP_RMD_SUCC "%d - Directory successfully deleted.\r\n"
#define REP_MOD_SET "%d Mode set to %s.\n"
#define REP_CLOSE_DATA_CONN "%d Bye Bye~\r\n"
#define REP_PASS_REQ "%d Please specify the password.\r\n"
#define REP_LOGIN_FAIL "%d Login Failed.\r\n"
/* Reply code */
#define RC_OPEN_DATA_CONN 150 
#define RC_CMD_SUCC 200 
#define RC_HELP 214 
#define RC_SERVICE_READY 220 
#define RC_CLOSE_DATA_CONN 226 
#define RC_PASV_SUCC 227 
#define RC_USER_LOGIN 230 
#define RC_FILE_ACT_SUCC 250 
#define RC_RMD_SUCC 250 
#define RC_PATH_CREATED 257
#define RC_MKD_SUCC 257  
#define RC_CMD_NOT_IMPL 520 
#define RC_FILE_ACT_FAIL 550
#define RC_PASS_REQ 331
#define RC_LOGIN_REQ 332
#define RC_LOGIN_FAIL 530
/* Commands */ 
typedef enum CMD_TYPE { 
   USER, PASS, CWD, XCWD, CDUP, XCUP, QUIT, ACCT, SMNT, REIN, PORT, PASV, TYPE, STRU, MODE, RETR, STOR, APPE, LIST, NLST, REST
,ABOR, PWD, XPWD, MKD, XMKD, RMD, XRMD, DELE, RNFR, RNTO, SITE, SYST, FEAT, SIZE, STAT, NOOP, HELP, STOU, ALLO, OPTS, ERROR
} CMD_TYPE;
