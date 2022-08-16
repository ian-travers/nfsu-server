// nfsuserver2.cpp : Defines the entry point for the console application.
//
#ifdef _WIN32
	#pragma comment(lib, "ws2_32.lib")
	#include <cwchar>
#endif

#include "win_nix.h"
#include "objects.h"
#ifndef _WIN32

int stricmp( const char *b, const char *a ){
    while (*a != '\0' && *b != '\0' && tolower(*a) == tolower(*b)){
        a++;
        b++;
    }

    return *a == *b ? 0 : tolower(*a)<tolower(*b) ? -1 : 1;
}
#endif

#ifdef NT_SERVICE

FILE * g_LogFile;
char g_Msg[255];

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;
#endif

bool EnableLogFile;
bool EnableLogScreen;
bool RewriteLogFile;
bool DisableTimeStamp;
bool Verbose;
bool RegisterGlobal;
bool LogAllTraffic;

bool BanV1;
bool BanV2;
bool BanV3;
bool BanV4;
// 2013-10-16
bool BanCheater;
// 2014-02-02
bool BanRoom;

FILE * logfil = NULL;  //file pointer for logfile
char logtemp[1100];    //temp variable for logging
FILE * tlogfil = NULL; //file pointer for traffic logfile

char * news = NULL;
unsigned int ids = 0;
time_t curtime;

char *arr[30];
char arr2[30][1024];

//sockets
SOCKET RedirectSocket, ListeningSocket, ReportingSocket, ClientReportingSocket, ClientReportingSocketTcp;
bool running = true;

//connection queues
ConnectionsClass RedirectConnections, ClientConnections, ReportingConnections;

std::vector<PlayerStat> PS;

extern std::vector<StarsLap> S1001;
extern std::vector<StarsLap> S1002;
extern std::vector<StarsLap> S1003;
extern std::vector<StarsLap> S1004;
extern std::vector<StarsLap> S1005;
extern std::vector<StarsLap> S1006;
extern std::vector<StarsLap> S1007;
extern std::vector<StarsLap> S1008;
extern std::vector<StarsLap> S1102;
extern std::vector<StarsLap> S1103;
extern std::vector<StarsLap> S1104;
extern std::vector<StarsLap> S1105;
extern std::vector<StarsLap> S1106;
extern std::vector<StarsLap> S1107;
extern std::vector<StarsLap> S1108;
extern std::vector<StarsLap> S1109;
extern std::vector<StarsLap> S1201;
extern std::vector<StarsLap> S1202;
extern std::vector<StarsLap> S1206;
extern std::vector<StarsLap> S1207;
extern std::vector<StarsLap> S1210;
extern std::vector<StarsLap> S1214;
extern std::vector<StarsDrift> S1301;
extern std::vector<StarsDrift> S1302;
extern std::vector<StarsDrift> S1303;
extern std::vector<StarsDrift> S1304;
extern std::vector<StarsDrift> S1305;
extern std::vector<StarsDrift> S1306;
extern std::vector<StarsDrift> S1307;
extern std::vector<StarsDrift> S1308;

#define NFSU_LAN_VERSION "2.5"
#define DEFAULT_NEWS "-=-=-=-\nDefault news\nPlz tell server admin to make news file ;)\n-=-=-=-=-"

extern ServerClass Server; //core ;)
extern SessionsClass Sessions;

char ascii[256] =
{
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
  64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
  64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

void base64_out (char *buf, char *obuf, int len) {
	int nprbytes;
	char *p = buf;
	while (ascii[(int) *(p++)] <= 63);

	nprbytes = len - 1;

	while (nprbytes > 4 && *buf != '\0') {
		*(obuf++) = (ascii[(int) *buf] << 2 | ascii[(int) buf[1]] >> 4);
		*(obuf++) = (ascii[(int) buf[1]] << 4 | ascii[(int) buf[2]] >> 2);
		*(obuf++) = (ascii[(int) buf[2]] << 6 | ascii[(int) buf[3]]);
		buf += 4;
		nprbytes -= 4;
    }
	if (nprbytes > 1)
		*(obuf++) = (ascii[(int) *buf] << 2 | ascii[(int) buf[1]] >> 4);
	if (nprbytes > 2)
		*(obuf++) = (ascii[(int) buf[1]] << 4 | ascii[(int) buf[2]] >> 2);
	if (nprbytes > 3)
		*(obuf++) = (ascii[(int) buf[2]] << 6 | ascii[(int) buf[3]]);
	*(obuf)++ = '\0';
} ;

void ShiftPersona(UserClass *user, RegUser *tr, int l){
	strcpy(user->Personas[l], user->Personas[l+1]);
	memset(user->Personas[l+1], 0, 16);
	strcpy(tr->Personas[l], tr->Personas[l+1]);
	memset(tr->Personas[l+1], 0, 16);
};

//2013-10-16
// Cheating check
bool IsCheater(const unsigned char wei, const unsigned char sus, const unsigned char eng, const unsigned char tur,
			   const unsigned char nos, const unsigned char ecu, const unsigned char tra, const unsigned char tir,
			   const unsigned char bra){
   unsigned char wei_tj, sus_tj, eng_tj, tur_tj, nos_tj, ecu_tj, tra_tj, tir_tj, bra_tj;

   wei_tj = wei & 0x01;
   sus_tj = sus & 0x01;
   eng_tj = eng & 0x01;
   tur_tj = tur & 0x01;
   nos_tj = nos & 0x01;
   ecu_tj = ecu & 0x01;
   tra_tj = tra & 0x01;
   tir_tj = tir & 0x01;
   bra_tj = bra & 0x01;

   // more than 3 TJ's
   if ((wei_tj + sus_tj + eng_tj + tur_tj + nos_tj + ecu_tj + tra_tj + tir_tj + bra_tj) > 3){
	   return true;
   }
   // if carier car
   if ((wei_tj + sus_tj + eng_tj + tur_tj + nos_tj + ecu_tj + tra_tj + tir_tj + bra_tj) == 3){
	   // at least one from first set of TJ's (eng trans tires)
	   if ((eng_tj + tra_tj + tir_tj) < 1) {
		   return true;
	   }
	   // no more than one part of the third set of TJ (weight reduction, suspension, NOS)
	   if ((wei_tj + sus_tj + nos_tj) > 1) {
		   return true;
	   }
   }

   return false;
}

// 2013-12-05
// Car performance level validation
bool IsValidCar(const unsigned char wei, const unsigned char sus, const unsigned char eng, const unsigned char tur,
			   const unsigned char nos, const unsigned char ecu, const unsigned char tra, const unsigned char tir,
			   const unsigned char bra, const unsigned char level){
   
	unsigned char wei_l, sus_l, eng_l, tur_l, nos_l, ecu_l, tra_l, tir_l, bra_l;
   
	wei_l = (wei >> 1) & 0x03;
	sus_l = (sus >> 1) & 0x03;
	eng_l = (eng >> 1) & 0x03;
	tur_l = (tur >> 1) & 0x03;
	nos_l = (nos >> 1) & 0x03;
	ecu_l = (ecu >> 1) & 0x03;
	wei_l = (tra >> 1) & 0x03;
	tir_l = (tir >> 1) & 0x03;
	bra_l = (bra >> 1) & 0x03;
	
	if ((wei_l > level) || (sus_l > level) || (eng_l > level) || (tur_l > level) || (nos_l > level) || (ecu_l > level) || (wei_l > level) || (tir_l > level) || (bra_l > level)){
		return false;
	}
			
	return true;
}

void Log( char *log ){
    if (!DisableTimeStamp){
        time_t t;
        time(& t);
        sprintf(logtemp, "[ %s  %s", ctime(& t), log);
        logtemp[26] = 32;
        logtemp[27] = 93;
    }else{
        strcpy(logtemp, log);
    }

    if (EnableLogScreen)
        printf(logtemp);

    if ((logfil != NULL) && (EnableLogFile)){
        fwrite(logtemp, strlen(logtemp), 1, logfil);
        fflush(logfil);
    }
};

void LogTraffic( char *log, int len){
    if (tlogfil != NULL){
		fwrite(log, len, 1, tlogfil);
        fwrite("\x0D\x0A", 2, 1, tlogfil);
        fflush(tlogfil);
    }
};

void LoadNews(){
    FILE * fil;
    fil = fopen("news", "r");

    if (fil == NULL){
        news = _strdup(DEFAULT_NEWS);
        return;
    }

    fseek(fil, 0, SEEK_END);
    int siz = ftell(fil);
    fseek(fil, 0, SEEK_SET);

    if(siz>10238) siz=10238;

    if (news != NULL)
        free(news);

    news=(char*)calloc(siz+1, sizeof(char));
    fread(news, siz, 1, fil);
    fclose(fil);
};

threadfunc IOThread(void * Dummy){
	char log[1024];	
	MessageClass *msg;
	ConnectionClass *temp;
	ConnectionsClass *con=(ConnectionsClass*)Dummy;
	sprintf(log, "Starting %s IO thread.\n", con->Name);
	Log(log);
	int k;
	fd_set check;
	timeval tim;
	tim.tv_sec=0;
	tim.tv_usec=100;
	GameClass *Game;
	RoomClass *Room;

	while(running){
		temp=con->First;
		while(temp!=NULL){
			if(temp->Abort){
				sprintf(log, "Aborting connection @ %s. IP : %s, SessionID : %u\n", con->Name, inet_ntoa(temp->remote_ip.sin_addr), temp->id);
				Log(log);
				con->RemoveConnection(temp);				
				if(temp->user!=NULL){					
					temp->user->Connection=NULL;
					if(temp->user->CurrentRoom!=NULL){
						if(temp->user->Game!=NULL){
							Game=temp->user->Game;
							Game->RemoveUser(temp->user, temp->Buffer);
							if(Game->Count==0){
								temp->user->CurrentRoom->Games.RemoveGame(Game);
								free(Game);
							}
							temp->user->CurrentRoom->RefreshUser(temp->user, temp->Buffer);
						}
						Room=temp->user->CurrentRoom;
						Room->RemoveUser(temp->user, temp->Buffer);
						if((Room->Count==0)&&(!Room->IsGlobal)){
							Server.Rooms.RemoveRoom(Room);
							free(Room);
						}
					}					
				}				
				temp->IncomingMessages.Clear();
				temp->IncomingMessages.mut.DeInit();
				temp->OutgoingMessages.Clear();
				temp->OutgoingMessages.mut.DeInit();
				closesocket(temp->sock);
				free(temp);
				temp=NULL;
				break;
			}

			msg=temp->OutgoingMessages.RemoveFirstMessage();
			while(msg!=NULL){				
				if(Verbose){
					sprintf(log, "Sending outgoing message @ %s. Command: %s, IP : %s, SessionID : %u\n", con->Name, msg->Message, inet_ntoa(temp->remote_ip.sin_addr), temp->id);
					Log(log);
				}
				k=send(temp->sock, msg->Message, msg->Size, 0);
				if(k!=msg->Size){
					closesocket(temp->sock);
					temp->Abort=true;					
					break;
				}
				free(msg->Message);
				free(msg);
				msg=temp->OutgoingMessages.RemoveFirstMessage();
			}
			FD_ZERO(&check);
			FD_SET(temp->sock, &check);
			tim.tv_sec=0;
			tim.tv_usec=0;
			k=select(temp->sock+1, &check, NULL, NULL, &tim);
			if(k<0){
#ifndef _WIN32
				if(errno != EAGAIN){
					sprintf(log, "Error on select() errno : %u\n", errno);
#else
				if(WSAGetLastError()!=WSAENOTSOCK){
					sprintf(log, "Error on select() WSAError : %u\n", WSAGetLastError());
#endif				
					Log(log);
#ifdef _WIN32
				}else{
					temp->Abort=true;
#endif
				}
			}
			if(k==1){
				if(temp->Received<12){
					k=recv(temp->sock, temp->Buffer+temp->Received, 12-temp->Received, 0);
					if(k<1){						
						if(Verbose){
							sprintf(log, "Receiving command failed - connection closed @ %s. IP : %s, SessionID : %u\n", con->Name, inet_ntoa(temp->remote_ip.sin_addr), temp->id);
							Log(log);						
						}
						temp->Abort=true;
					}else{
						//sprintf(log, "Receiving command succeed @ %s\n", con->Name);
						//Log(log);
						temp->Received+=k;
					}
				}
				if(temp->Received>11){
					//sprintf(log, "Receiving command data.\n");
					//Log(log);
					DWORD req=(temp->Buffer[11]&0xFF)|((temp->Buffer[10]<<8)&0xFF00)|((temp->Buffer[9]<<16)&0xFF0000)|((temp->Buffer[8]<<24)&0xFF000000);
//					WSASetLastError(0);
					if((req>12)&&(req<1000)){
						k=recv(temp->sock, temp->Buffer+temp->Received, req-temp->Received, 0);
						if(k<1){							
							if(Verbose){
								sprintf(log, "Receiving command failed - connection closed @ %s. IP : %s, SessionID : %u\n", con->Name, inet_ntoa(temp->remote_ip.sin_addr), temp->id);
								Log(log);
							}
							closesocket(temp->sock);
							temp->Abort=true;
						}else{
						//sprintf(log, "Receiving data succeed @ %s : %s\n", con->Name, inet_ntoa(temp->remote_ip.sin_addr));
						//Log(log);
							temp->Received+=k;
						}
					}
					if(req>999){
						temp->Abort=true;
					}
					if(temp->Received==req){						
						if(Verbose){
							sprintf(log, "Receiving data complete @ %s. Command : %s, IP : %s, SessionID  :%u\n", con->Name, temp->Buffer, inet_ntoa(temp->remote_ip.sin_addr), temp->id);
							Log(log);
						}
						msg=(MessageClass*)calloc(1, sizeof(MessageClass));
						msg->Message=(char*)calloc(temp->Received, sizeof(char));
						msg->Size=temp->Received;
						memcpy(msg->Message, temp->Buffer, temp->Received);
						temp->IncomingMessages.AddMessage(msg);
						temp->Received=0;
					}
				}
			}
			if((int)(difftime(curtime, temp->Idle))==50){
				sprintf(log, "Aborting connection due to timeout @ %s, IP: %s\n", con->Name, inet_ntoa(temp->remote_ip.sin_addr));
				Log(log);
				temp->Abort=true;
			}			
			temp=temp->Next;
		}
		Sleep(10);
	}
};

threadfunc AcceptThread(void *Dummy){
	SOCKET cl;
	char log[1024];

	ConAccParam *acc=(ConAccParam*)Dummy;

	sprintf(log, "Starting %s thread\n", acc->Name);
	Log(log);

	unsigned long remote_sockaddr_length = sizeof(SOCKADDR_IN);	

	ConnectionClass *temp;

#ifdef _WIN32
	int timeo=10;
#else
	struct timeval timeo;
	timeo.tv_sec  = 0;
	timeo.tv_usec = 10000;
#endif
	while(running){
		cl=accept(acc->sock, NULL, NULL);
		if(cl!=INVALID_SOCKET){
			temp=(ConnectionClass*)calloc(1, sizeof(ConnectionClass));
			temp->IncomingMessages.mut.Init();
			temp->OutgoingMessages.mut.Init();
			temp->sock=cl;
#ifndef _WIN32
			getsockname(temp->sock, (SOCKADDR *)&temp->local_ip,(socklen_t*) &remote_sockaddr_length);
			getpeername(temp->sock, (SOCKADDR *)&temp->remote_ip,(socklen_t*) &remote_sockaddr_length);
#else
			getsockname(temp->sock, (SOCKADDR *)&temp->local_ip ,(int*) &remote_sockaddr_length);
			getpeername(temp->sock, (SOCKADDR *)&temp->remote_ip,(int*) &remote_sockaddr_length);
#endif
			time(&temp->Idle);
			acc->Connections->AddConnection(temp);
			temp->id=ids++;				
			sprintf(log, "Accepting client @ %s. IP : %s, ID : %u\n", acc->Name, inet_ntoa(temp->remote_ip.sin_addr), temp->id);
			Log(log);
		}
	}
};

threadfunc RedirectorWorker(void *Dummy){
	Log("Starting RedirectorWorker thread.\n");
	char log[1024];
	char buffer[1024];

	MessageClass *msg;
	MessageClass *tmsg;

	ConnectionClass *temp;
	while(running){
		RedirectConnections.mut.Lock();
		temp=RedirectConnections.First;
		while(temp!=NULL){
			msg=temp->IncomingMessages.RemoveFirstMessage();
			while(msg!=NULL){
				free(msg->Message);
				free(msg);				
				if(Verbose){
					sprintf(log, "Adding outgoing message : server_info_to_nfsu : %s\n", inet_ntoa(temp->remote_ip.sin_addr));
					Log(log);
				}
				sprintf(arr2[0], "ADDR=%s", inet_ntoa(temp->local_ip.sin_addr));
				sprintf(arr2[1], "PORT=10901");
				sprintf(arr2[2], "SESS=1072010288");
				sprintf(arr2[3], "MASK=0295f3f70ecb1757cd7001b9a7a5eac8");
				int k=MakeCommand(buffer, "@dir", arr, 4);
				tmsg=(MessageClass*)calloc(1, sizeof(MessageClass));
				tmsg->Message=(char*)calloc(k, sizeof(char));
				memcpy(tmsg->Message, buffer, k);
				tmsg->Size=k;
				temp->OutgoingMessages.AddMessage(tmsg);

				msg=temp->IncomingMessages.RemoveFirstMessage();
			}
			temp=temp->Next;
		}
		RedirectConnections.mut.Unlock();
		Sleep(10);
	}
};

void Subscribe(){
	struct hostent *hostInfo = gethostbyname("3priedez.net");
	SOCKADDR_IN remote_sockaddr_in;
	unsigned long remote_sockaddr_length = sizeof(SOCKADDR_IN);	
	char buf[1024];

	remote_sockaddr_in.sin_family = AF_INET;
	remote_sockaddr_in.sin_port = htons(80);

	if(hostInfo!=NULL){		
#ifdef _WIN32
		memcpy(&remote_sockaddr_in.sin_addr.S_un.S_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
#else
		memcpy(&remote_sockaddr_in.sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
#endif
	}else{
		remote_sockaddr_in.sin_addr.s_addr = inet_addr("195.2.101.48");
		if (remote_sockaddr_in.sin_addr.s_addr == INADDR_NONE) return;
	}

	SOCKET sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock==INVALID_SOCKET) return;
	if(connect(sock, (const sockaddr*)&remote_sockaddr_in, remote_sockaddr_length)==0){
		sprintf(buf, "GET /nfsug/submit.py HTTP/1.1\x0d\x0aHost: 3priedez.net\x0d\x0a\x0d\x0a");
		send(sock, buf, strlen(buf), 0);
		recv(sock, buf, 1024, 0);
		closesocket(sock);
	}
}

threadfunc WebReport(void *dummy){
	while(running){
		Subscribe();
		Sleep(1000*60*5);
	}
};

threadfunc ListenerWorker(void *Dummy){
	char log[1024];
	sprintf(log, "Starting ListenerWorker thread.\n");
	Log(log);
	char buffer[1024];
	char *buf;
	char *tmp, *tmp2;

	char resu[1024];
	char dec_resu[1024];

	char car_b64[9]; // car info in base64 from AUXI replica
	char car_dec[6]; // decoded car info 



	MessageClass *msg;
	UserClass *user;
	
	int p=22;
	char ttm[5];


	bool IsNew;

	int k;

	ConnectionClass *temp;
	while(running){
		ClientConnections.mut.Lock();
		temp=ClientConnections.First;
		while(temp!=NULL){
			if((int)(difftime(curtime, temp->Idle))==30){
				//sprintf(log, "Sending ping IP: %s\n", inet_ntoa(temp->remote_ip.sin_addr));
				//Log(log);
				ttm[3]=(char)p++;
				ttm[2]=(char)p>>8;
				ttm[1]=(char)p>>16;
				ttm[0]=(char)p>>24;
				memcpy(log, "~png", 4);
				memcpy(log+4, ttm, 4);
				memcpy(log+8, "\x00\x00\x00\x0c", 4);

				msg=(MessageClass*)calloc(1, sizeof(MessageClass));
				msg->Message=(char*)calloc(12, sizeof(char));
				memcpy(msg->Message, log, 12);
				msg->Size=12;
				temp->OutgoingMessages.AddMessage(msg); 

				temp->Idle-=1;
			}
			msg=temp->IncomingMessages.RemoveFirstMessage();
			while(msg!=NULL){
				buf=msg->Message;
				user=temp->user;

				if(LogAllTraffic) LogTraffic(msg->Message, msg->Size);
					
				switch(buf[0]){
					case 'g':
						switch(buf[1]){
							case 'l':
								//glea
								if(strncmp(buf+2, "ea", 2)==0){
							tmp=strchr(buf+17, 10);
							if(tmp==NULL) tmp=strchr(buf+17, 9);
							if(tmp!=NULL) tmp[0]=0;
							GameClass *game=user->CurrentRoom->Games.GameFromName(buf+17);
							if(game!=NULL){
								game->RemoveUser(user, buffer);
							}
						}
								break;
							case 'g':
								//gget
								if(strncmp(buf+2, "et", 2)==0){
							tmp=buf+17;
							GameClass *game=user->CurrentRoom->Games.GameFromName(tmp);
							if(game!=NULL){
								sprintf(arr2[0], "IDENT=%u", game->ID);
								sprintf(arr2[1], "WHEN=2003.12.8 15:52:54");
								sprintf(arr2[2], "NAME=%s", game->Name);
								sprintf(arr2[3], "HOST=%s", game->Users.First->User->Personas[game->Users.First->User->SelectedPerson]);
								sprintf(arr2[4], "PARAMS=%s", game->params);
								sprintf(arr2[5], "ROOM=%s", user->CurrentRoom->Name);
								sprintf(arr2[6], "MAXSIZE=%u", game->max);
								sprintf(arr2[7], "MINSIZE=%u", game->min);
								sprintf(arr2[8], "COUNT=%u", game->Count);
								sprintf(arr2[9], "USERFLAGS=0");
								sprintf(arr2[10], "SYSFLAGS=%u", game->sysflags);
								sprintf(arr2[11], "OPID0=%u", game->Users.First->User->id);
								sprintf(arr2[12], "OPPO0=%s", game->Users.First->User->Personas[game->Users.First->User->SelectedPerson]);
								sprintf(arr2[13], "ADDR0=%s", game->Users.First->User->IP);

								temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "gget", arr, 14));
							}
						}
								break;
							case 'j':
								//gjoi
								if(strncmp(buf+2, "oi", 2)==0){
									GameClass *game=user->CurrentRoom->Games.GameFromName(buf+17);
										if(game!=NULL){
											game->AddUser(user, buffer);
										}
								}
								break;
							case 'd':
								//gdel
								if(strncmp(buf+2, "el", 2)==0){
									tmp=strchr(buf+17, 10);
									tmp[0]=0;
									GameClass *game=user->CurrentRoom->Games.GameFromName(buf+17);
									if(game!=NULL){
										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "gdel", NULL, 0));

										while(game->Count>0){
											game->Users.First->User->Game=NULL;
											user->CurrentRoom->RefreshUser(game->Users.First->User, buffer);
											game->RemoveUser(game->Users.First->User, buffer);
										}
										user->CurrentRoom->Games.RemoveGame(game);

										sprintf(arr2[0], "IDENT=%u", game->ID);
										BroadCastCommand(user->CurrentRoom->Users, "+agm", arr, 1, buffer);
										free(game);
									}
								}
								break;
							case 's':
								//gsta
								if(strncmp(buf+2, "ta", 2)==0){
							GameClass *game=user->CurrentRoom->Games.GameFromName(buf+17);
							if(game->Users.Count<game->min){
								temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "gstanepl", NULL, 0));
							}else{
								arr2[0][0]=0;
								temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "gsta", arr, 1));
								if(game!=NULL){
									game->StartGame(buffer);
								}
							}
						}
								break;
							case 'c':
								//gcre
								if(strncmp(buf+2, "re", 2)==0){							
									if(Verbose){
										sprintf(log, "Creating challenge\n");
										Log(log);
									}
									GameClass *game=(GameClass*)calloc(1, sizeof(GameClass));
									tmp=strchr(buf+17, 10);
									if(tmp==NULL) tmp=strchr(buf+17, 9);									
									if(Verbose){
										sprintf(log, "After getting challenge name\n");
										Log(log);
									}
									tmp[0]=0;
									tmp+=6;
									strcpy(game->Name, buf+17);									
									if(Verbose){
										sprintf(log, "After name copy\n");
										Log(log);
									}
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									RoomClass *room=Server.Rooms.RoomFromName(tmp);
                                    
									if(Verbose){
										sprintf(log, "After getting room\n");
										Log(log);
									}

									tmp=tmp2+9;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
																		
									if(Verbose){
										sprintf(log, "Before atoi : %u - %s\n", game->max, tmp);
										Log(log);
									}

									game->max=atoi(tmp);
									
									if(Verbose){
										sprintf(log, "After getting max\n");
										Log(log);
									}

									tmp=tmp2+9;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									game->min=atoi(tmp);
									
									if(Verbose){
										sprintf(log, "After getting min\n");
										Log(log);
									}

									tmp=tmp2+10;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									game->sysflags=atoi(tmp);
									
									if(Verbose){
										sprintf(log, "After getting sysflags\n");
										Log(log);
									}

									tmp=tmp2+8;
									strcpy(game->params, tmp);
									
									if(Verbose){
										sprintf(log, "After setting all params\n");
										Log(log);
									}

									room->Games.AddGame(game);
									
									if(Verbose){
										sprintf(log, "After adding game to room.games\n");
										Log(log);
									}

									game->AddUser(user, buffer);
									
									if(Verbose){
										sprintf(log, "After adding user\n");
										Log(log);
									}

									sprintf(arr2[0], "IDENT=%u", game->ID);
									sprintf(arr2[1], "WHEN=2003.12.8 15:52:54");
									sprintf(arr2[2], "NAME=%s", game->Name);
									sprintf(arr2[3], "HOST=%s", user->Personas[user->SelectedPerson]);
									sprintf(arr2[4], "PARAMS=%s", game->params);
									sprintf(arr2[5], "ROOM=%s", room->Name);
									sprintf(arr2[6], "MAXSIZE=%u", game->max);
									sprintf(arr2[7], "MINSIZE=%u", game->min);
									sprintf(arr2[8], "COUNT=%u", game->Count);
									sprintf(arr2[9], "USERFLAGS=0");
									sprintf(arr2[10], "SYSFLAGS=%u", game->sysflags);
									sprintf(arr2[11], "OPID0=%u", user->id);
									sprintf(arr2[12], "OPPO0=%s", user->Personas[user->SelectedPerson]);
									sprintf(arr2[13], "ADDR0=%s", user->IP);

									temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "gcre", arr, 14));

									room->RefreshUser(user, buffer);

									BroadCastCommand(room->Users, "+agm", arr, 14, buffer);
								}
								break;
						}
						break;
					case 'n':
						//news
						if(strncmp(buf+1, "ews", 3)==0){
							strcpy(arr2[0], news);
							temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "newsnew0", arr, 1));
						}
						break;
					case 'm':
						switch(buf[1]){
							case 'o':
								//move
								if(strncmp(buf+2, "ve", 2)==0){
									tmp=strchr(buf+17, 10);
									tmp[0]=0;

									RoomClass *rom=Server.Rooms.RoomFromName(buf+17);

									if(rom!=NULL){

										//2013-12-05
										// Check for car performance level restriction in some rooms
										
										// Check closed 2014-06-01 
										/*
										unsigned char Car_brand, Wei, Sus, Eng, Tur, Nos, Ecu, Tra, Tir, Bra;

										strncpy(car_b64, user->car, 9);

										base64_out(car_b64, car_dec, 9);
										// get car brand
										Car_brand = (((car_dec[1] << 4) >> 2) & 0x3F) | ((car_dec[2] >> 6) & 0x03);
										// get Weight Reduction
										Wei = ((car_dec[2] << 2) >> 5) & 0x07;
										// get Suspension
										Sus = car_dec[2] & 0x07;
										// get Engine
										Eng = (car_dec[3] >> 5) & 0x07;
										// get Turbo
										Tur = ((car_dec[3] << 3) >> 5) & 0x07;
										// get NOS
										Nos = (((car_dec[3] << 6) >> 5) & 0x07) | ((car_dec[4] >> 7) & 0x01);
										// get ECU
										Ecu = ((car_dec[4] << 1) >> 5) & 0x07;
										// get Transmission
										Tra = ((car_dec[4] << 4) >> 5) & 0x07;
										// get Tires
										Tir = (((car_dec[4] << 7) >> 5) & 0x07) | ((car_dec[5] >> 6) & 0x03);
										// get Brake Kits
										Bra = ((car_dec[5] << 2) >> 5) & 0x07;

										// buf+19 get room name w/o type char and dot

										if (strncmp(buf+19, "STOCK_ONLY", 10)==0) {
											if (!IsValidCar(Wei, Sus, Eng, Tur, Nos, Ecu, Tra, Tir, Bra, 0)) {
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "movefull", NULL, 0));
												break;
											}
										}

										if (strncmp(buf+19, "STREET_ONLY", 11)==0) {
											if (!IsValidCar(Wei, Sus, Eng, Tur, Nos, Ecu, Tra, Tir, Bra, 1)) {
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "movefull", NULL, 0));
												break;
											}
										}

										if (strncmp(buf+19, "PRO_ONLY", 8)==0) {
											if (!IsValidCar(Wei, Sus, Eng, Tur, Nos, Ecu, Tra, Tir, Bra, 2)) {
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "movefull", NULL, 0));
												break;
											}
										}
										*/

										rom->AddUser(user, buffer);

										sprintf(arr2[0], "Z=%u/%u", rom->ID, rom->Count);
										arr[0]=(char*)&arr2[0];
										BroadCastCommand(&Server.Users, "+pop", arr, 1, buffer);



										// сообщение при входе в комнату
										UserClass *us;
										us=Server.Users.UserFromUsername(user->Personas[user->SelectedPerson]);
										if (us!=NULL){
											if (us->Connection!=NULL){
												if (!us->GetMsg) {
													// attempt 1
													// need pause for queue a message
													Sleep(25);
													us->GetMsg = true;
													//sprintf(arr2[0], "F=P");
													//sprintf(arr2[1], "T=www.nfsu-2013.ru");
													//sprintf(arr2[2], "N=newbie");
													//us->Connection->OutgoingMessages.AddMessage(MakeMessage(buffer, "+msg", arr, 3));
													
													sprintf(arr2[0], "F=A"); // username + (admin)
													//sprintf(arr2[0], "F=M"); // username + (mod)
													//sprintf(arr2[0], "F=H"); // username + (host)
													sprintf(arr2[1], "T=www.nfsu-cup.com");
													sprintf(arr2[2], "N=newbie");
													us->Connection->OutgoingMessages.AddMessage(MakeMessage(buffer, "+msg", arr, 3));
												}
											}
										}



									}else{				
										sprintf(arr2[0], "IDENT=0");
										sprintf(arr2[1], "NAME=");
										sprintf(arr2[2], "COUNT=0");
										if(user->CurrentRoom==NULL){
											sprintf(arr2[3], "LIDENT=0");
										}else{
											sprintf(arr2[3], "LIDENT=%u", user->CurrentRoom->ID);
										}
										sprintf(arr2[4], "LCOUNT=0");

										RoomClass *rc=NULL;

										if(user->CurrentRoom!=NULL){
											rc=user->CurrentRoom;
											user->CurrentRoom->RemoveUser(user, buffer);
											if((rc->Users.Count==0)&&(!rc->IsGlobal)){
												Server.Rooms.RemoveRoom(rc);
												sprintf(arr2[0], "I=%u", rc->ID);
												arr[0]=(char*)&arr2[0];
												BroadCastCommand(&Server.Users, "+rom", arr, 1, buffer);
												free(rc);
											}
										}

										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "move", arr, 5));

										if(rc!=NULL){
											sprintf(arr2[0], "Z=%u/%u", rc->ID, rc->Count);
											arr[0]=(char*)&arr2[0];
											BroadCastCommand(&Server.Users, "+pop", arr, 1, buffer);
										}
									}
								}
								break;
							case 'e':
								//mesg
								if(strncmp(buf+2, "sg", 2)==0){
									if(Verbose) {
										sprintf(log, "Message received from %s\n", temp->user->IP);
										Log(log);
									}

									temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "mesg", NULL, 0));//reply that msg is recv

									tmp=buf+17;
									tmp2=strchr(tmp, 10);
									if (tmp2==NULL) tmp2=strchr(tmp, 9);
									if (tmp2!=NULL){
										tmp2[0]=0;
										if (strncmp(tmp2+1, "PRIV", 4)==0){
											if (Verbose) {
												sprintf(log, "Private message\n");
												Log(log);
											}
											tmp=tmp2+6;
											tmp2=strchr(tmp, 10);
											if (tmp2==NULL) tmp2=strchr(tmp, 9);
											tmp2[0]=0;
											UserClass *us;
											us=Server.Users.UserFromUsername(tmp);
											if (us!=NULL){
												if (us->Connection!=NULL){
													sprintf(arr2[0], "F=EP5");
													sprintf(arr2[1], "T=%s", buf+17);
													sprintf(arr2[2], "N=%s", user->Personas[user->SelectedPerson]);
													us->Connection->OutgoingMessages.AddMessage(MakeMessage(buffer, "+msg", arr, 3));
													// reply to author
													sprintf(arr2[1], "T=%s: %s", us->Personas[us->SelectedPerson], buf+17);
													user->Connection->OutgoingMessages.AddMessage(MakeMessage(buffer, "+msg", arr, 3));
												}
											}else{
												if(Verbose) {
													sprintf(log, "Didn't find user\n");
													Log(log);
												}
											}
										}else{
											if(Verbose) {
												sprintf(log, "Global message\n");
												Log(log);
											}
											if(user->CurrentRoom!=NULL){
												//sprintf(arr2[0], "F=U");
												//sprintf(arr2[1], "T=%s", buf+17);
												//sprintf(arr2[2], "N=%s", user->Personas[user->SelectedPerson]);
												//BroadCastCommand(user->CurrentRoom->Users, "+msg", arr, 3, buffer);

												if (strncmp(user->Personas[user->SelectedPerson], "newbie", 6)==0) {
													sprintf(arr2[0], "F=A");
												} else {
													sprintf(arr2[0], "F=U");
												}
												
												sprintf(arr2[1], "T=%s", buf+17);
												sprintf(arr2[2], "N=%s", user->Personas[user->SelectedPerson]);
												BroadCastCommand(user->CurrentRoom->Users, "+msg", arr, 3, buffer);
												
											}
										}

									}else{
										sprintf(log, "Smth wrong with mesg message.\n");
										Log(log);
									}
								}
								break;
						}
						break;
					case 'r':
						switch(buf[1]){
							case 'a':
								//rank
								if(strncmp(buf+2, "nk", 2)==0){
									char *rept, *name0, *name1, *name2, *name3;

									// get REPT  
									tmp = strstr(buf+12, "REPT=");
									tmp2 = strchr(tmp, 10);
									if (tmp2 == NULL) tmp2 = strchr(tmp, 9);
									tmp2[0] = 0;
									rept = tmp + 5;
		
									// get NAME0
									tmp = tmp2 + 1;
									tmp = strstr(tmp, "NAME");
									tmp2 = strchr(tmp, 10);
									if (tmp2 == NULL) tmp2 = strchr(tmp, 9);
									tmp2[0] = 0;
									name0 = tmp + 6;
	
									// get NAME1
									tmp = tmp2 + 1;
									tmp = strstr(tmp, "NAME");
									tmp2 = strchr(tmp, 10);
									if (tmp2 == NULL) tmp2 = strchr(tmp, 9);
									tmp2[0] = 0;
									name1 = tmp + 6;
	
									// get NAME2
									tmp = tmp2 + 1;
									tmp = strstr(tmp, "NAME");

									if (tmp != NULL){
										tmp2 = strchr(tmp, 10);
										if (tmp2 == NULL) tmp2 = strchr(tmp, 9);
										tmp2[0] = 0;
										name2 = tmp + 6;
	
										// get NAME3
										tmp = tmp2 + 1;
										tmp = strstr(tmp, "NAME");
		
										if (tmp != NULL) {
											tmp2 = strchr(tmp, 10);
											if (tmp2 == NULL) tmp2 = strchr(tmp, 9);
											tmp2[0] = 0;
											name3 = tmp + 6;
										}else{
											name3 = NULL;
										}
									}else{
										name2 = NULL;
										name3 = NULL; 
									}

									// get RESU		
									tmp = tmp2 + 1;
									tmp = strstr(tmp, "RESU");
									if (tmp != NULL) {
										tmp2 = strchr(tmp, 10);
										if (tmp2 == NULL) tmp2 = strchr(tmp, 9);
										tmp2[0] = 0;
										sprintf(resu, tmp + 5);
									}
									// decode RESU (get dec_resu)
									base64_out(resu, dec_resu, strlen(resu));
//*
									char PlayersCount, RaceType, ReptNo, Dir, Laps;
									unsigned short Track;
									PlayersCount = dec_resu[0];
									RaceType = dec_resu[1];
									ReptNo = dec_resu[2];
									Track = (dec_resu[4] & 0xFF) | ((dec_resu[3] << 8) & 0xFF00);
									Dir = dec_resu[5];
									Laps = dec_resu[6];

									// fix 1099 secret track report and hang up reporter
									if (Track == 1099) {
										break;
									}
									
									char Place, FinishMark, SeedNo, Car;
									int Disc;
									float FF;
									float *pFF;
									int BestLap;
									int BestDrift;
									int BlockSize = 58 + 8 * Laps;

									// цикл для нахождения информации о результатах автора реплики (он же REPT)
									int l=0;
									while (l < PlayersCount) {
										SeedNo = dec_resu[7+BlockSize*l];
										if (SeedNo == ReptNo) {
											Place = dec_resu[7+BlockSize*l+2];
											FinishMark = dec_resu[7+BlockSize*l+3];
											Car = dec_resu[7+12+Laps*4+32+Laps*4+10+BlockSize*l];
											BestLap = (dec_resu[7+15+BlockSize*l]&0xFF)|((dec_resu[7+14+BlockSize*l]<<8)&0xFF00)|((dec_resu[7+13+BlockSize*l]<<16)&0xFF0000)|((dec_resu[7+12+BlockSize*l]<<24)&0xFF000000);
											pFF = (float*)&BestLap;
											FF = *pFF;
											BestLap = FF * 1000;
											Disc = (dec_resu[7+15+Laps*4+24+BlockSize*l]&0xFF)|((dec_resu[7+14+Laps*4+24+BlockSize*l]<<8)&0xFF00)|((dec_resu[7+13+Laps*4+24+BlockSize*l]<<16)&0xFF0000)|((dec_resu[7+12+Laps*4+24+BlockSize*l]<<24)&0xFF000000);
											pFF = (float*)&Disc;
											FF = *pFF;
											Disc = FF * 1;
											BestDrift = (dec_resu[7+15+Laps*4+32+BlockSize*l]&0xFF)|((dec_resu[7+14+Laps*4+32+BlockSize*l]<<8)&0xFF00)|((dec_resu[7+13+Laps*4+32+BlockSize*l]<<16)&0xFF0000)|((dec_resu[7+12+Laps*4+32+BlockSize*l]<<24)&0xFF000000);
											// не очень участок ниже, может отказаться от него
											/*
											if (FinishMark == 9) Place = PlayersCount;
											if (FinishMark == 10) Place = PlayersCount - 1;
											if (FinishMark == 11) Place = PlayersCount - 2;
											if (FinishMark == 12) Place = PlayersCount - 3;
											//*/
										}
										l++;
									}
                                    /*
									 тут из Sessions находим игрока и определяем комнату
									 в которой он проводил последнюю гонку
									 если комната рейтинговая, то пересчитываем статистику
                                    //*/
                                    SessionClass *session=Sessions.First;
                                    
									char RoomType='A';

									while (session != NULL){
 									  if (stricmp(rept, session->Persona)==0){
										  sprintf(log, "Race finished REPT=%s IP=%s ROOM=%s PLACE=%d\n", rept, session->IP, session->FromRoom, Place);
										  Log(log);
										  RoomType = *(session->FromRoom);
										  Sessions.RemoveSession(session);
										  free(session);
										  break;
									  }
									  session = session->Next;
									}
									/* 
									   Первая буква имени комнаты для рейтинговых A, B, C, D 
									   или 65, 66, 67, 68 DEC соответственно.
									   Для нерейтинговых комнат первая буква E, F, G, H
									   или 69, 70, 71, 72 DEC соответственно
									*/
									if (RoomType<69){ // если комната рейтинговая пересчет статистики
										// calculate reporter statistic
										switch (ReptNo) {
											case 0:
												CalcStat(name0, name1, name2, name3, PlayersCount, RaceType, Track, Laps, Place, Disc);
												break;
											case 1:
												CalcStat(name1, name0, name2, name3, PlayersCount, RaceType, Track, Laps, Place, Disc);
												break;
											case 2:
												CalcStat(name2, name0, name1, name3, PlayersCount, RaceType, Track, Laps, Place, Disc);
												break;
											case 3:
												CalcStat(name3, name0, name1, name2, PlayersCount, RaceType, Track, Laps, Place, Disc);
												break;
										}
										// check/update stars of week Ranked rooms only
										//if (FinishMark == 1)  // надо поработать над этим условием
										//	UpdateBestTimes(Track, Dir, rept, Car, BestLap, BestDrift);
									}
									// check/update stars of week (best lap times)
									// ranked & unranked room 
									if (FinishMark == 1)  // надо поработать над этим условием
										UpdateBestTimes(Track, Dir, rept, Car, BestLap, BestDrift);

									sprintf(arr2[0], "RANK=Unranked");
									sprintf(arr2[1], "TIME=866");
									temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "rank", arr, 2));
								}
								break;
							case 'o':
								//room
								if(strncmp(buf+2, "om", 2)==0){							
									if(Verbose){
										sprintf(log, "Create room\n");
										Log(log);
									}


									//2014-02-02
									// Room creation ban check
									if (BanRoom) {
										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "roomroom", NULL, 0));
									}

									else {
										RoomClass *rom;
										rom=(RoomClass*)calloc(1, sizeof(RoomClass));
										rom->Count=0;

										rom->Games.cid=1;
										rom->Games.Count=0;
										rom->Games.First=NULL;

										strcpy(rom->Name, buf+17);
										Server.Rooms.AddRoom(rom);

										sprintf(arr2[0], "IDENT=%u", rom->ID);
										sprintf(arr2[1], "NAME=%s", rom->Name);
										sprintf(arr2[2], "HOST=%s", user->Personas[user->SelectedPerson]);
										sprintf(arr2[3], "DESC=");
										sprintf(arr2[4], "COUNT=1");
										sprintf(arr2[5], "LIMIT=50");
										sprintf(arr2[6], "FLAGS=C");

										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "room", arr, 7));

										sprintf(arr2[0], "I=%u", user->id);
										sprintf(arr2[1], "N=%s", user->Personas[user->SelectedPerson]);
										sprintf(arr2[2], "M=%s", user->Username);
										sprintf(arr2[3], "F=");
										sprintf(arr2[4], "A=%s", user->IP);
										sprintf(arr2[5], "S=%s", GetPlayerStat(user->Personas[user->SelectedPerson]));
										sprintf(arr2[6], "X=%s", user->car);
										sprintf(arr2[7], "R=%s", rom->Name);
										sprintf(arr2[8], "RI=%u", rom->ID);
										sprintf(arr2[9], "RF=C");
										sprintf(arr2[10], "RT=1");

										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+who", arr, 11));

										rom->AddUser(user, buffer);

										sprintf(arr2[0], "F=CU");
										sprintf(arr2[1], "T=\"has created the room\"");
										sprintf(arr2[2], "N=%s", user->Personas[user->SelectedPerson]);

										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+msg", arr, 3));


										sprintf(arr2[0], "I=%u", rom->ID);
										sprintf(arr2[1], "N=%s", rom->Name);
										sprintf(arr2[2], "H=%s", user->Personas[user->SelectedPerson]);
										sprintf(arr2[3], "F=CH");
										sprintf(arr2[4], "T=1");
										sprintf(arr2[5], "L=25");
										sprintf(arr2[6], "P=10");
										sprintf(arr2[7], "A=%s", user->IP);

										BroadCastCommand(&Server.Users, "+rom", arr, 8, buffer);

										sprintf(arr2[0], "PI=%u", user->id);
										sprintf(arr2[1], "N=%s", user->Personas[user->SelectedPerson]);
										sprintf(arr2[2], "M=%s", user->Username);
										sprintf(arr2[3], "F=HU");
										sprintf(arr2[4], "A=%s", user->IP);
										sprintf(arr2[5], "P=223");
										sprintf(arr2[6], "S=%s", GetPlayerStat(user->Personas[user->SelectedPerson]));
										sprintf(arr2[7], "X=%s", user->car);
										sprintf(arr2[8], "G=0");
										sprintf(arr2[9], "T=1");
										
										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+usr", arr, 10));			
									}
								}
								break;
						}												
						break;
					case 'a':
						switch(buf[1]){
							case 'u':
								switch(buf[2]){
									case 'x':
										//auxi
										if(buf[3]=='i'){											
											if(Verbose){
												sprintf(log, "Car received\n");
												Log(log);
											}
											strcpy(user->car, buf+17);
											strcpy(arr2[0], buf+12);
											temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "auxi", arr, 1)); 
//*
											//2013-10-16
											//check car for cheating
											unsigned char Car_brand, Wei, Sus, Eng, Tur, Nos, Ecu, Tra, Tir, Bra;
											
											if (BanCheater) {

												strncpy(car_b64, buf+17, 9);
												base64_out(car_b64, car_dec, 9);
												// get car brand
												Car_brand = (((car_dec[1] << 4) >> 2) & 0x3F) | ((car_dec[2] >> 6) & 0x03);
												// get Weight Reduction
												Wei = ((car_dec[2] << 2) >> 5) & 0x07;
												// get Suspension
												Sus = car_dec[2] & 0x07;
												// get Engine
												Eng = (car_dec[3] >> 5) & 0x07;
												// get Turbo
												Tur = ((car_dec[3] << 3) >> 5) & 0x07;
												// get NOS
												Nos = (((car_dec[3] << 6) >> 5) & 0x07) | ((car_dec[4] >> 7) & 0x01);
												// get ECU
												Ecu = ((car_dec[4] << 1) >> 5) & 0x07;
												// get Transmission
												Tra = ((car_dec[4] << 4) >> 5) & 0x07;
												// get Tires
												Tir = (((car_dec[4] << 7) >> 5) & 0x07) | ((car_dec[5] >> 6) & 0x03);
												// get Brake Kits
												Bra = ((car_dec[5] << 2) >> 5) & 0x07;
												if (IsCheater(Wei, Sus, Eng, Tur, Nos, Ecu, Tra, Tir, Bra)) {
													sprintf(log, "Cheater detected: %s\n", temp->user->Personas[temp->user->SelectedPerson]);
													Log(log);
													temp->Abort = true;
												}
											}
//*/
										}
										break;
									case 't':
										//auth
										if(buf[3]=='h'){
											IsNew=true;											
											if(Verbose){
												sprintf(log, "auth\n");
												Log(log);
											}
											/*
+	buf+18	0x00468e8a "dlgnome
PASS=~pn$P29&$Fyt6Zy`tGsfBx{hTkEJ#Ll
TOS=1
MID=$00e01851bf21
FROM=US
LANG=EN
PROD=nfs-pc-2003
VERS="pc/1.2-Nov 12 2003"
SLUS=SLUS
REGN=NA
CLST=51733
NETV=5
"	char *
											
											*/
/*/+	buf+18	0x00468e8a "dlgnome
PASS=~pn$P29&.`l`['%22%22K6?'V_I8(SjD%3dNR
TOS=1
MID=$00e01851bf21
FROM=US
LANG=EN
PROD=nfs-pc-2003
VERS="pc/1.1001-Oct 30 2003"
SLUS=SLUS
REGN=NA
CLST=0
NETV=5
"	char *
*/
											tmp=strstr(buf+17, "VERS=\"");
											if(tmp!=NULL){
												k=tmp[11]-'0';
											}else{
												k=1;
											}

											tmp=buf+17;
											tmp2=strchr(tmp, 10);
											if(tmp2==NULL) tmp2=strchr(tmp, 9);
											tmp2[0]=0;

											// 2013-08-13 
											// fixed account name length bug (empty account name)
											if ((strlen(buf+17)<4)||(strlen(buf+17)>15)) {
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "authmiss", NULL, 0));
												break;
											}

											RegUser *tr=Server.ru.UserFromUsername(tmp);
											if(tr==NULL){												
												if(Verbose){
													sprintf(log, "No such user\n");
													Log(log);
												}
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "authimst", NULL, 0));
											}else{
												user=Server.Users.UserFromUsername(tmp);
												if(user==NULL){																	
													if(Verbose){
														sprintf(log, "New user\n");
														Log(log);
													}
													user=(UserClass*)calloc(1, sizeof(UserClass));
													strcpy(user->IP, temp->IP);
													strcpy(user->Port, temp->Port);
													user->id=-1;
													strcpy(user->Username, tmp);
													int l=0;
													RegUser *tr=Server.ru.UserFromUsername(user->Username);
													while(tr->Personas[l][0]!=0){
														strcpy(user->Personas[l], tr->Personas[l]);						
														l++;
														if(l==4) break;
													}
													user->Idle=0;
													user->Connection=temp;
													temp->user=user;
													Server.Users.AddUser(user);
												}else
												{
													if(user->Connection!=NULL){
														//in case user is already connected
														temp->Abort=true;
													}else{
														IsNew=false;													
														if(Verbose){
															sprintf(log, "Found user\n");
															Log(log);
														}
														strcpy(user->IP, temp->IP);
														strcpy(user->Port, temp->Port);
														temp->user=user;
														temp->user->Idle=0;
														user->Connection=temp;
													}
												}
												if(!temp->Abort){
													sprintf(arr2[0], "TOS=%u", user->id);
													sprintf(arr2[1], "NAME=%s", user->Username);
													sprintf(arr2[2], "MAIL=vdl@3priedez.net");
													sprintf(arr2[3], "BORN=19800325");
													sprintf(arr2[4], "GEND=M");
													sprintf(arr2[5], "FROM=US");
													sprintf(arr2[6], "LANG=en");
													sprintf(arr2[7], "SPAM=NN");
													int l=0;
													char str[1024];
													memset(str, 0, 1024);
													while(user->Personas[l][0]!=0){
														if(l>0) strcat(str, ",");
														strcat(str, user->Personas[l]);
														l++;
														if(l==4) break;
													}
													sprintf(arr2[8], "PERSONAS=%s", str);
													sprintf(arr2[9], "LAST=2003.12.8 15:51:38");
													temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "auth", arr, 10));
												}
											}
											if(user!=NULL){
												user->Version=k;
												switch(user->Version){
													case 4:
														if(BanV4) temp->Abort=true;
														break;
													case 3:
														if(BanV3) temp->Abort=true;
														break;
													case 2:
														if(BanV2) temp->Abort=true;
														break;
													default:
														if(BanV1) temp->Abort=true;
														break;
												}
											}
											if((user!=NULL)&&!IsNew)
												Server.SendRoomsToUser(user, buffer);
										}
										break;
								}
								break;
							case 'c':
								//acct
								if(strncmp(buf+2, "ct", 2)==0){
									tmp=strchr(buf+17, 10);
									tmp[0]=0;									
									if(Verbose){
										sprintf(log, "Try to register username\n");
										Log(log);
									}

									// check user name length
									if ((strlen(buf+17)<4)||(strlen(buf+17)>15)) {
										// name length 4-15
										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "acctinam", NULL, 0));
										break;
									}
									if(Server.ru.UserFromUsername(buf+17)!=NULL){
										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "acctdupl", NULL, 0));
									}else{
										Server.RegisterUser(buf+17);
										sprintf(arr2[0], "NAME=%s", buf+17);
										sprintf(arr2[1], "PERSONAS=");
										sprintf(arr2[2], "AGE=24");

										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "acct", arr, 3));
									}
								}
								break;
							case 'd':
								//addr
								if(strncmp(buf+2, "dr", 2)==0){									
									if(Verbose){
										sprintf(log, "addr\n");			
										Log(log);									
									}
									tmp=strchr(buf+17, 10);
									if(tmp==NULL) tmp=strchr(buf+17, 9);
									tmp[0]=0;
									//using only addr reported from socket
									strcpy(temp->IP, inet_ntoa(temp->remote_ip.sin_addr));									
									//strcpy(temp->IP, buf+17);
									//if(strcmp(temp->IP, "0.0.0.0")==0){
									//	sprintf(log, "Client sent 0.0.0.0\n");
									//	if(Verbose)Log(log);
									//	sockaddr_in so;
									//	k=sizeof(so);			
									//	strcpy(temp->IP, inet_ntoa(temp->remote_ip.sin_addr));
									//}									
									if(Verbose){
										sprintf(log, "IP: %s\n", temp->IP);
										Log(log);
									}
									tmp+=6;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									strcpy(temp->Port, tmp);
									sprintf(arr2[0], "SKEY=$37940faf2a8d1381a3b7d0d2f570e6a7");
									temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "skey", arr, 1));
								}
								break;
						}
						break;
					case '~':
						//~png
						if(strncmp(buf+1, "png", 3)==0){
							//sprintf(log, "Receiving ping from IP: %s\n", inet_ntoa(temp->remote_ip.sin_addr));
							//Log(log);
							time(&temp->Idle);
							if(temp->user!=NULL){
								user->Idle=0;
							}
						}
						break;
					case 'c':
						//cper
						if(strncmp(buf+1, "per", 3)==0){
							tmp=buf+17;
							tmp2=strchr(buf+17, 10);
							if(tmp2==NULL) tmp2=strchr(buf+17, 9);
							tmp2[0]=0;
							 

							//looking for user name dupl
							int pers_exist=0;
							RegUser *tr=Server.ru.First;

							while ((tr!=NULL)&&(!pers_exist)){
								for (int i=0; i<4; i++){
									if (stricmp(tr->Personas[i], tmp)==0){
										pers_exist=1;
										break;
									}
								}
								tr = tr->Next;
							}
							if (pers_exist){
								temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "cperdupl", NULL, 0));
								break;
							}

							// check user name length
							if ((strlen(tmp)<3)||(strlen(tmp)>12)) {
								temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "cperinam", NULL, 0));
								break;
							}

							// check characters in the user name
							/*
							char ch=0;
							while (tmp[ch]!=0) {
								if ((tmp[ch]<48)||(tmp[ch]>122)) {
									pers_exist = 1;
									break;
								}
								if ((tmp[ch]>57)&&(tmp[ch]<65)) {
									pers_exist = 1;
									break;
								}
								if ((tmp[ch]>90)&&(tmp[ch]<95)) {
									pers_exist = 1;
									break;
								}
								ch++;
							}
							if (pers_exist){
								// invalid characters in the name
								temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "cperiper", NULL, 0));
								break;
							}
							//*/	
								// invalid name or characters
								//temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "cperfilt", NULL, 0));
							// reach maximum user names
							//temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "cpermaxp", NULL, 0));
								

							for(int l=0;l<4;l++){
								if(user->Personas[l][0]==0){
									strcpy(user->Personas[l], tmp);
									RegUser *tr=Server.ru.UserFromUsername(user->Username);
									if(tr!=NULL){
										strcpy(tr->Personas[l], tmp);

										// add to vector
										PlayerStat *tps;
										
										PS.push_back(PlayerStat(tmp, 9999, 0,0,0,100,101,101,
														9999, 0,0,0,100,101,101,
														9999, 0,0,0,100,101,101,
														9999, 0,0,0,100,101,101,
														9999, 0,0,0,100,101,101));
										
				
										// REP bonus 
										// CIR = 50000 SPR = 5000 DRA = 5000 DRI = 50000
										/*
										PS.push_back(PlayerStat(tmp, 9999, 0,0,0,27500,101,101,
														9999, 0,0,0,50000,101,101,
														9999, 0,0,0,5000,101,101,
														9999, 0,0,0,5000,101,101,
														9999, 0,0,0,50000,101,101));
										*/

										Server.SaveSettings();
										Server.SaveStat();
										sprintf(arr2[0], "PERS=%s", tmp);
										sprintf(arr2[1], "NAME=%s", user->Username);
										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "cper", arr, 2));
									}else{										
										if(Verbose){
											sprintf(buffer, "Panic - user tries to create subaccount name without having username.\n");
											Log(buffer);
										}
									}
									break;
								}
							}
						}
						break;
					case 'p':
						//pers
						if(strncmp(buf+1, "ers", 3)==0){
							tmp=strstr(buf+12, "PERS=");
							if(tmp!=NULL){
								tmp+=5;
								tmp2=strchr(tmp, 10);
								if(tmp2==0) tmp2=strchr(tmp, 9);
								if(tmp2!=NULL){
									tmp2[0]=0;
									user->SelectedPerson=-1;
									user->SelectPerson(tmp);
									if(user->SelectedPerson!=-1){
										sprintf(arr2[0], "NAME=%s", user->Username);
										sprintf(arr2[1], "PERS=%s", user->Personas[user->SelectedPerson]);
										sprintf(arr2[2], "LAST=2003.12.8 15:51:58");
										sprintf(arr2[3], "PLAST=2003.12.8 16:51:40");
										sprintf(arr2[4], "LKEY=3fcf27540c92935b0a66fd3b0000283c");

										temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "pers", arr, 5));
									}else{
										sprintf(log, "Could not select person.\n");
										Log(log);
									}
								}else{
									sprintf(log, "Wrong params in pers message\n");
									Log(log);
								}
							}else{
								sprintf(log, "Wrong params in pers message\n");
								Log(log);
							}
						} 
						break;
					case 's':
						switch(buf[1]){
							case 'n':
								//snap
/*
+snp
N=username
R=item index
P=rep points
S=1,wins_in_hex,loses_in_hex,
0x0000   2B 73 6E 70 00 00 00 00-00 00 00 A4 4E 3D 4C 75   +snp.......¤N=Lu
0x0010   67 6E 65 72 63 68 72 69-73 09 52 3D 31 09 50 3D   gnerchris.R=1.P=
0x0020   32 35 30 30 30 30 37 34-09 53 3D 31 2C 66 36 61   25000074.S=1,f6a
0x0030   2C 39 33 38 2C 31 39 36-2C 31 37 64 37 38 38 61   ,938,196,17d788a
0x0040   2C 36 39 37 66 31 2C 61-32 63 2C 32 37 30 66 2C   ,697f1,a2c,270f,
0x0050   2C 2C 2C 36 34 2C 36 35-2C 36 35 2C 32 37 30 66   ,,,64,65,65,270f
0x0060   2C 2C 2C 2C 36 34 2C 36-35 2C 36 35 2C 32 37 30   ,,,,64,65,65,270
0x0070   66 2C 2C 31 2C 2C 36 34-2C 36 61 34 38 2C 32 31   f,,1,,64,6a48,21
0x0080   35 39 2C 31 2C 66 36 61-2C 39 33 37 2C 31 39 36   59,1,f6a,937,196
0x0090   2C 35 66 35 65 30 66 66-2C 31 39 66 34 62 32 2C   ,5f5e0ff,19f4b2,
0x00A0   36 38 64 00 2B 73 6E 70-00 00 00 00 00 00 00 A2   68d.+snp.......ў
*/
								if(strncmp(buf+2, "ap", 2)==0){
									if(Verbose){
										sprintf(log, "Rank list\n");
										Log(log);
									}
									int index, chan, start, range;
									tmp=buf+18;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									index=atoi(tmp);

									tmp=tmp2+6;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									chan=atoi(tmp);

									tmp=tmp2+7;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									start=atoi(tmp);

									tmp=tmp2+7;
									tmp2=strchr(tmp, 10);
									if(tmp2==NULL) tmp2=strchr(tmp, 9);
									tmp2[0]=0;
									range=atoi(tmp);

									sprintf(arr2[0], "INDEX=%u", index);
									sprintf(arr2[1], "CHAN=%u", chan);
									sprintf(arr2[2], "START=%u", start);
									sprintf(arr2[3], "RANGE=%u", range);
									sprintf(arr2[4], "SEQN=0");

									temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "snap", arr, 5));

									// HEX значения, разделенные запятыми 
									// 0 - рейтинг
									// 1 - победы
									// 2 - поражения
									// 3 - отключения
									// 4 - очки репутации
									// 5 - очки репутации соперников (среднее значение)
									// 6 - рейтинг соперников (среднее значение)
									
									int ind = 1;
									std::vector<PlayerStat>::iterator it;
									std::vector<StarsLap>::iterator it2;
									std::vector<StarsDrift>::iterator it3;

									switch (index) {
										case 1:
											// top 100 all
											std::sort(PS.begin(), PS.end(), sort_REP_All);
											for (it = PS.begin(); it != PS.end(); it++) {
												sprintf(arr2[0], "N=%s", it->Name);
												sprintf(arr2[1], "S=%x,%x,%x,%x,%x,%x,%x", ind, it->Wins_All, it->Loses_All,
													it->Disc_All, it->REP_All, it->OppsREP_All, it->OppsRating_All);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind>100) break;
											}
											break;
										case 2:
											// top 100 circuit
											std::sort(PS.begin(), PS.end(), sort_REP_Circ);
											for (it = PS.begin(); it != PS.end(); it++) {
												sprintf(arr2[0], "N=%s", it->Name);
												sprintf(arr2[1], "S=,,,,,,,%x,%x,%x,%x,%x,%x,%x", ind, it->Wins_Circ, it->Loses_Circ,
													it->Disc_Circ, it->REP_Circ, it->OppsREP_Circ, it->OppsRating_Circ);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind>100) break;
											}
											break;
										case 3:
											// top 100 sprint
											std::sort(PS.begin(), PS.end(), sort_REP_Sprint);
											for (it = PS.begin(); it != PS.end(); it++) {
												sprintf(arr2[0], "N=%s", it->Name);
												sprintf(arr2[1], "S=,,,,,,,,,,,,,,%x,%x,%x,%x,%x,%x,%x", ind, it->Wins_Sprint, it->Loses_Sprint,
													it->Disc_Sprint, it->REP_Sprint, it->OppsREP_Sprint, it->OppsRating_Sprint);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind>100) break;
											}
											break;
										case 4:
											// top 100 Drag
											std::sort(PS.begin(), PS.end(), sort_REP_Drag);
											for (it = PS.begin(); it != PS.end(); it++) {
												sprintf(arr2[0], "N=%s", it->Name);
												sprintf(arr2[1], "S=,,,,,,,,,,,,,,,,,,,,,%x,%x,%x,%x,%x,%x,%x", ind, it->Wins_Drag, it->Loses_Drag,
													it->Disc_Drag, it->REP_Drag, it->OppsREP_Drag, it->OppsRating_Drag);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind>100) break;
											}
											break;
										case 5:
											// top 100 drift
											std::sort(PS.begin(), PS.end(), sort_REP_Drift);
											for (it = PS.begin(); it != PS.end(); it++) {
												sprintf(arr2[0], "N=%s", it->Name);
												sprintf(arr2[1], "S=,,,,,,,,,,,,,,,,,,,,,,,,,,,,%x,%x,%x,%x,%x,%x,%x", ind, it->Wins_Drift, it->Loses_Drift,
													it->Disc_Drift, it->REP_Drift, it->OppsREP_Drift, it->OppsRating_Drift);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind>100) break;
											}
											break;
										case 6:
											// circuit Market Street
											std::sort(S1001.begin(), S1001.end(), sort_Time);
											for (it2 = S1001.begin(); it2 != S1001.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind>10) break;
											}
											break;
										case 7:
											// circuit Stadium
											std::sort(S1002.begin(), S1002.end(), sort_Time);
											for (it2 = S1002.begin(); it2 != S1002.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 8:
											// circuit Olympic Square
											std::sort(S1003.begin(), S1003.end(), sort_Time);
											for (it2 = S1003.begin(); it2 != S1003.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 9:
											// circuit Terminal
											std::sort(S1004.begin(), S1004.end(), sort_Time);
											for (it2 = S1004.begin(); it2 != S1004.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 10:
											// circuit Atlantica
											std::sort(S1005.begin(), S1005.end(), sort_Time);
											for (it2 = S1005.begin(); it2 != S1005.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 11:
											// circuit Inner City
											std::sort(S1006.begin(), S1006.end(), sort_Time);
											for (it2 = S1006.begin(); it2 != S1006.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 12:
											// circuit Port Royal
											std::sort(S1007.begin(), S1007.end(), sort_Time);
											for (it2 = S1007.begin(); it2 != S1007.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 13:
											// circuit National Rail
											std::sort(S1008.begin(), S1008.end(), sort_Time);
											for (it2 = S1008.begin(); it2 != S1008.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 14:
											// sprint Liberty Gardens
											std::sort(S1102.begin(), S1102.end(), sort_Time);
											for (it2 = S1102.begin(); it2 != S1102.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 15:
											// sprint Broadway
											std::sort(S1103.begin(), S1103.end(), sort_Time);
											for (it2 = S1103.begin(); it2 != S1103.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 16:
											// sprint Lock Up
											std::sort(S1104.begin(), S1104.end(), sort_Time);
											for (it2 = S1104.begin(); it2 != S1104.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 17:
											// sprint 9th & Frey
											std::sort(S1105.begin(), S1105.end(), sort_Time);
											for (it2 = S1105.begin(); it2 != S1105.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 18:
											// sprint Bedard Bridge
											std::sort(S1106.begin(), S1106.end(), sort_Time);
											for (it2 = S1106.begin(); it2 != S1106.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 19:
											// sprint Sprillway
											std::sort(S1107.begin(), S1107.end(), sort_Time);
											for (it2 = S1107.begin(); it2 != S1107.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 20:
											// sprint 1st Ave. Truck Stop
											std::sort(S1108.begin(), S1108.end(), sort_Time);
											for (it2 = S1108.begin(); it2 != S1108.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 21:
											// sprint 7th & Sparling
											std::sort(S1109.begin(), S1109.end(), sort_Time);
											for (it2 = S1109.begin(); it2 != S1109.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 22:
											// drag 14th and Vine Construction
											std::sort(S1201.begin(), S1201.end(), sort_Time);
											for (it2 = S1201.begin(); it2 != S1201.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 23:
											// drag Highway 1
											std::sort(S1202.begin(), S1202.end(), sort_Time);
											for (it2 = S1202.begin(); it2 != S1202.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 24:
											// drag Main Street
											std::sort(S1206.begin(), S1206.end(), sort_Time);
											for (it2 = S1206.begin(); it2 != S1206.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 25:
											// drag Commercial 
											std::sort(S1207.begin(), S1207.end(), sort_Time);
											for (it2 = S1207.begin(); it2 != S1207.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 26:
											// drag 14th and Vine
											std::sort(S1210.begin(), S1210.end(), sort_Time);
											for (it2 = S1210.begin(); it2 != S1210.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 27:
											// drag Main Street Construction
											std::sort(S1214.begin(), S1214.end(), sort_Time);
											for (it2 = S1214.begin(); it2 != S1214.end(); it2++) {
												sprintf(arr2[0], "N=%s", it2->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it2->Time, it2->Dir, it2->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 28:
											// drift Track 1
											/*
											sprintf(arr2[0], "N=Will soon be");
											sprintf(arr2[1], "S=0,0,0");
											temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
											
											sprintf(arr2[0], "N=added to the");
											sprintf(arr2[1], "S=0,0,0");
											temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
											
											sprintf(arr2[0], "N=new version!");
											sprintf(arr2[1], "S=0,0,0");
											temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
											
											sprintf(arr2[0], "N=---");
											sprintf(arr2[1], "S=0,0,0");
											temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));

											sprintf(arr2[0], "N=iTPEMOP");
											sprintf(arr2[1], "S=0,0,0");
											temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
											break;
											//*/
											//*
											std::sort(S1301.begin(), S1301.end(), sort_Points);
											for (it3 = S1301.begin(); it3 != S1301.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											
											break;
										case 29:
											// drift Track 2
											std::sort(S1302.begin(), S1302.end(), sort_Points);
											for (it3 = S1302.begin(); it3 != S1302.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 30:
											// drift Track 3
											std::sort(S1303.begin(), S1303.end(), sort_Points);
											for (it3 = S1303.begin(); it3 != S1303.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 31:
											// drift Track 4
											std::sort(S1304.begin(), S1304.end(), sort_Points);
											for (it3 = S1304.begin(); it3 != S1304.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 32:
											// drift Track 5
											std::sort(S1305.begin(), S1305.end(), sort_Points);
											for (it3 = S1305.begin(); it3 != S1305.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 33:
											// drift Track 6
											std::sort(S1306.begin(), S1306.end(), sort_Points);
											for (it3 = S1306.begin(); it3 != S1306.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 34:
											// drift Track 7
											std::sort(S1307.begin(), S1307.end(), sort_Points);
											for (it3 = S1307.begin(); it3 != S1307.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
										case 35:
											// drift Track 8
											std::sort(S1308.begin(), S1308.end(), sort_Points);
											for (it3 = S1308.begin(); it3 != S1308.end(); it3++) {
												sprintf(arr2[0], "N=%s", it3->Name);
												sprintf(arr2[1], "S=%x,%x,%x", it3->Points, it3->Dir, it3->Car);
												temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "+snp", arr, 2));
												ind++;
												if (ind > 10) break;
											}
											break;
									
									//*/
									}
									
								}
								break;
							case 'k':
								//skey
								if(strncmp(buf+2, "ey", 2)==0){							
									if(Verbose){
										sprintf(log, "skey\n");
										Log(log);
									}
									sprintf(arr2[0], "SKEY=$37940faf2a8d1381a3b7d0d2f570e6a7");
									temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "skey", arr, 1));
								}
								break;
							case 'e':
								//sele
								if(strncmp(buf+2, "le", 2)==0){							
									if(Verbose){
										sprintf(log, "sele\n");
										Log(log);
									}
									sprintf(arr2[0], "GAMES=1");
									sprintf(arr2[1], "ROOMS=1");
									sprintf(arr2[2], "USERS=1");
									sprintf(arr2[3], "MESGS=1");
									sprintf(arr2[4], "RANKS=0");
									sprintf(arr2[5], "MORE=1");
									sprintf(arr2[6], "SLOTS=36");

									temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "sele", arr, 7));
								}
								break;
						}
						break;
					case 'u':
						// user
						if(strncmp(buf+1, "ser", 3)==0){
							if(Verbose){
								sprintf(log, "user\n");
								Log(log);
							}
							sprintf(arr2[0], "PERS=%s", temp->user->Personas[temp->user->SelectedPerson]);
							sprintf(arr2[1], "LAST=2004.6.1 15:57:52");
							sprintf(arr2[2], "EXPR=1072566000");
							sprintf(arr2[3], "STAT=");
							sprintf(arr2[4], "CHEAT=3");
							sprintf(arr2[5], "ACK_REP=186");
							sprintf(arr2[6], "REP=186");
							sprintf(arr2[7], "PLAST=2004.6.1 15:57:46");
							sprintf(arr2[8], "PSINCE=2003.11.25 07:56:09");
							sprintf(arr2[9], "DCNT=0");
							sprintf(arr2[10], "ADDR=%s", temp->user->IP);
							sprintf(arr2[11], "SERV=159.153.229.239");
							sprintf(arr2[12], "RANK=99999");
							sprintf(arr2[13], "MESG=vdlgnome");

							temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "user", arr, 14));
						}
						break;
					case 'd':
						// dper
						if (strncmp(buf+1, "per", 3)==0){
							
							tmp=strchr(buf+17, 10);
							if (tmp==NULL) tmp=strchr(buf+17, 9);
							tmp[0]=0;

							// erase from vertor
							std::vector<PlayerStat>::iterator it;
							it = std::find(PS.begin(), PS.end(), buf+17);
							if (it!=PS.end() ) {
								PS.erase(it);
							}

							int l;
							for(l=0;l<4;l++){
								if (strcmp(user->Personas[l], buf+17)==0) break;
							}
							RegUser *tr=Server.ru.UserFromUsername(user->Username);
							switch(l){
								case 3:
									memset(user->Personas[l], 0, 16);
									memset(tr->Personas[l], 0, 16);
									break;
								case 2:
									if(user->Personas[l+1]==0){
										memset(user->Personas[l], 0, 16);
										memset(tr->Personas[l], 0, 16);
									}else{
										ShiftPersona(user, tr, l);
									}
									break;
								case 1:
									if(user->Personas[l+1]==0){
										memset(user->Personas[l], 0, 16);
										memset(tr->Personas[l], 0, 16);
									}else{
										ShiftPersona(user, tr, l);
									}
									if (user->Personas[l+2]==0){
										memset(user->Personas[l+1], 0, 16);
										memset(tr->Personas[l+1], 0, 16);
									}else{
										ShiftPersona(user, tr, l+1);
									}
									break;
								case 0:
									if(user->Personas[l+1]==0){
										memset(user->Personas[l], 0, 16);
										memset(tr->Personas[l], 0, 16);
									}else{
										ShiftPersona(user, tr, l);
									}
									if (user->Personas[l+2]==0){
										memset(user->Personas[l+1], 0, 16);
										memset(tr->Personas[l+1], 0, 16);
									}else{
										ShiftPersona(user, tr, l+1);
									}
									if(user->Personas[l+3]==0){
										memset(user->Personas[l+2], 0, 16);
										memset(tr->Personas[l+2], 0, 16);
									}else{
										ShiftPersona(user, tr, l+2);
									}
									break;
							}
							Server.SaveSettings();
							Server.SaveStat();
							temp->OutgoingMessages.AddMessage(MakeMessage(buffer, "dper", NULL, 0));
						}
						break;
					default:						
						if(Verbose){
							sprintf(log, "Other recv: %s\n", buf);
							Log(log);
						}
						break;
				}	
				free(msg->Message);
				free(msg);
				msg=temp->IncomingMessages.RemoveFirstMessage();
			}
			temp=temp->Next;
		}
		ClientConnections.mut.Unlock();
		Sleep(10);
	}
};

bool SendData(char *text, unsigned long ip) {
	SOCKADDR_IN remote_sockaddr_in;

	remote_sockaddr_in.sin_family = AF_INET;
	remote_sockaddr_in.sin_port = htons(10801);
	remote_sockaddr_in.sin_addr.s_addr = ip;

	return (sendto(ClientReportingSocket,text,strlen(text),0, (SOCKADDR *)&remote_sockaddr_in, sizeof(SOCKADDR_IN)) != SOCKET_ERROR);
};

threadfunc ClientReporter(void *Dummy){
	int status;
	SOCKADDR_IN remote_sockaddr_in;
	unsigned long remote_sockaddr_length = sizeof(SOCKADDR_IN);
	char tempBuff[1024];

	while(running){
#ifndef _WIN32
		status = recvfrom(ClientReportingSocket, tempBuff, 1024, 0, (SOCKADDR *)&remote_sockaddr_in,(socklen_t*) &remote_sockaddr_length);
#else
		status = recvfrom(ClientReportingSocket, tempBuff, 1024, 0, (SOCKADDR *)&remote_sockaddr_in,(int *) &remote_sockaddr_length);
#endif
		if (status == SOCKET_ERROR) {
			sprintf(tempBuff, "Recvfrom failed @ ClientReporter.\nExiting ClientReporter thread - AutoFind feature won't be available till server is restarted!!!.\n");
			Log(tempBuff);
			closesocket(ClientReportingSocket);
			RETURNFROMTHREAD;
		}		
		if(Verbose){
			sprintf(tempBuff, "Server finder connected from %s\n", inet_ntoa(remote_sockaddr_in.sin_addr));
			Log(tempBuff);
		}
		sprintf(tempBuff, "%s", NFSU_LAN_VERSION);
#ifdef _WIN32
		if(!SendData(tempBuff, remote_sockaddr_in.sin_addr.S_un.S_addr)){
#else
		if(!SendData(tempBuff, remote_sockaddr_in.sin_addr.s_addr)){
#endif
			sprintf(tempBuff, "Could not send data to server finder: %s\n", inet_ntoa(remote_sockaddr_in.sin_addr));
			Log(tempBuff);
		}
	}
	closesocket(ClientReportingSocket);
};

threadfunc ClientReporterTcp(void *Dummy){
	SOCKET cl;
	char tempBuff[1024];

	sprintf(tempBuff, "Starting ClientReporterTcp thread\n");
	Log(tempBuff);

	SOCKADDR_IN remote_sockaddr_in;
	unsigned long remote_sockaddr_length = sizeof(SOCKADDR_IN);	
	int status;

	while(running){
		cl=accept(ClientReportingSocketTcp, NULL, NULL);
		if(cl!=INVALID_SOCKET){
#ifndef _WIN32
			getpeername(cl, (SOCKADDR *)&remote_sockaddr_in,(socklen_t*) &remote_sockaddr_length);
#else
			getpeername(cl, (SOCKADDR *)&remote_sockaddr_in,(int*) &remote_sockaddr_length);
#endif			
			if(Verbose){
				sprintf(tempBuff, "Server finder connected from %s\n", inet_ntoa(remote_sockaddr_in.sin_addr));
				Log(tempBuff);
			}
			status=(int)difftime(curtime, Server.Startup);
			sprintf(tempBuff, "%u|%u|%u|%s|%s|%s", ClientConnections.Count, Server.Rooms.Count, status, SERVER_PLATFORM, NFSU_LAN_VERSION, Server.Name);
			send(cl, tempBuff, strlen(tempBuff), 0);
			closesocket(cl);
		}
	}
	closesocket(ClientReportingSocketTcp);
};

threadfunc StatThread(void *Dummy){
	SOCKET cl;
	char tempBuff[1024];

	sprintf(tempBuff, "Starting StatThread\n");
	Log(tempBuff);

	SOCKADDR_IN remote_sockaddr_in;
	unsigned long remote_sockaddr_length = sizeof(SOCKADDR_IN);	
	int status, ban_tj_cheat, players_in_race, ban_room_creation;
	RoomClass *tr;
	RUserClass *tu;

	while(running){
		cl=accept(ReportingSocket, NULL, NULL);
		if(cl!=INVALID_SOCKET){
#ifndef _WIN32
			getpeername(cl, (SOCKADDR *)&remote_sockaddr_in,(socklen_t*) &remote_sockaddr_length);
#else
			getpeername(cl, (SOCKADDR *)&remote_sockaddr_in,(int*) &remote_sockaddr_length);
#endif			
			if(Verbose){
				sprintf(tempBuff, "Stat client connected from %s\n", inet_ntoa(remote_sockaddr_in.sin_addr));
				Log(tempBuff);
			}
			status=(int)difftime(curtime, Server.Startup);

			// 2013-10-29
			// add ban TJ cheat status (0 - disabled / 1 - enabled)
			// add players in race info
			if (BanCheater) {
				ban_tj_cheat = 1;
			}else{
				ban_tj_cheat = 0;
			}
			players_in_race = Sessions.Count;
			// 2014-06-01
			// add ban room creation status (0 - disabled / 1 - enabled)
			if (BanRoom) {
				ban_room_creation = 1;
			} else {
				ban_room_creation = 0;
			}

			sprintf(tempBuff, "%u|%u|%u|%s|%s|%s|%u|%u|%u~~~", ClientConnections.Count, Server.Rooms.Count, status, SERVER_PLATFORM, NFSU_LAN_VERSION, Server.Name, ban_tj_cheat, players_in_race, ban_room_creation);
			send(cl, tempBuff, strlen(tempBuff), 0);

			tr=Server.Rooms.First;
			while(tr!=NULL){
				sprintf(tempBuff, "%s|%u|[", tr->Name, tr->Users.Count);
				send(cl, tempBuff, strlen(tempBuff), 0);
				tu=tr->Users.First;
				while(tu!=NULL){
					sprintf(tempBuff, "%s|", tu->User->Personas[tu->User->SelectedPerson]);
					send(cl, tempBuff, strlen(tempBuff), 0);
					tu=tu->Next;
				}
				send(cl, "]", 1, 0);
				tr=tr->Next;
			}
			closesocket(cl);
		}
	}
	closesocket(ReportingSocket);
};

//watches for timeouted clients
threadfunc Maintenance(void *Dummy){
	Log("Maintenance thread started\n");
	UserClass *user, *t;

	// 2013-11-04 fix sessions
	// remove all session longer than 20 min
	SessionClass *session, *ts;

	char tempBuff[1024];

	while(running){
	//	sprintf(tempBuff, "vector size = %d\n", PS.size());
	//	Log(tempBuff);
		Sleep(1000);
		time(&curtime);
		user=Server.Users.First;
		while(user!=NULL){
			user->Idle++;
			if((user->Idle>60*45)&&(user->Connection==NULL)){
				Log("Removing user due to 45 min timeout\n");
				t=user->Next;
				Server.Users.RemoveUser(user);
				free(user);
				user=t;
			}else{
				user=user->Next;
			}
		}

		// 2013-11-04 fix sessions
		// remove all sessions that lives more then 20 min
		session = Sessions.First;
		while (session != NULL){
			session->Idle++;
			if(session->Idle > 60*15){
				sprintf(tempBuff, "Clear race info due to 15 min timeout. Username=%s, ROOM=%s\n", session->Persona, session->FromRoom); 
				Log(tempBuff);
				ts = session->Next;
				Sessions.RemoveSession(session);
				free(session);
				session = ts;
			}else{
				session = session->Next;
			}
		}
	}
	RETURNFROMTHREAD;
};

#ifdef NT_SERVICE
VOID WriteInLogFile( char *text )
{
    g_LogFile = fopen( "Service.log", "a+t" );
    
	if ( g_LogFile )
    {
        fprintf( g_LogFile, text );
        fclose( g_LogFile );
    }
    
	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{ 
    sprintf(g_Msg, "In to service control...\n");
    WriteInLogFile(g_Msg);

    switch(dwControl) 
    { 
        case SERVICE_CONTROL_PAUSE: 
            ServiceStatus.dwCurrentState = SERVICE_PAUSED; 
            break; 
 
        case SERVICE_CONTROL_CONTINUE: 
            ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
            break; 
 
        case SERVICE_CONTROL_STOP: 
            sprintf(g_Msg, "Stopping...\n");
            WriteInLogFile(g_Msg);
            
			ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING; 
            ServiceStatus.dwCheckPoint    = 0; 
            ServiceStatus.dwWaitHint      = 0; 
 
            running = false;

            if ( !SetServiceStatus( ServiceStatusHandle, &ServiceStatus) )
            { 
				sprintf(g_Msg,"NFSU:LAN SetServiceStatus() error %ld\n",GetLastError() ); 
                WriteInLogFile(g_Msg);
            } 

            sprintf(g_Msg, "NFSU:LAN leaving handler \n", 0 ); 
            WriteInLogFile(g_Msg);
            
			return; 
 
        case SERVICE_CONTROL_INTERROGATE: 
            break; 
 
        default: 
            sprintf(g_Msg, "NFSU:LAN unrecognized control code %ld\n", dwControl ); 
            WriteInLogFile(g_Msg);
    } 
 
    // Send current status. 
    if ( !SetServiceStatus(ServiceStatusHandle, &ServiceStatus) ) 
    { 
        sprintf(g_Msg,"NFSU:LAN SetServiceStatus() error %ld\n", GetLastError() ); 
        WriteInLogFile( g_Msg );
    } 

    return; 
} 
#endif

bool InitServer(){
#ifdef WIN32
	EnableLogFile = GetPrivateProfileInt("NFSU:LAN", "EnableLogFile", 1, ".\\nfsu.ini");
	EnableLogScreen = false;
	RewriteLogFile = GetPrivateProfileInt("NFSU:LAN", "RewriteLogFile", 1, ".\\nfsu.ini");
	DisableTimeStamp = GetPrivateProfileInt("NFSU:LAN", "DisableTimeStamp", 0, ".\\nfsu.ini");
	Verbose = GetPrivateProfileInt("NFSU:LAN", "Verbose", 0, ".\\nfsu.ini");
	RegisterGlobal = GetPrivateProfileInt("NFSU:LAN", "RegisterGlobal", 0, ".\\nfsu.ini");
	LogAllTraffic = GetPrivateProfileInt("NFSU:LAN", "LogAllTraffic", 0, ".\\nfsu.ini");
	BanV1 = GetPrivateProfileInt("NFSU:LAN", "BanV1", 0, ".\\nfsu.ini");
	BanV2 = GetPrivateProfileInt("NFSU:LAN", "BanV2", 0, ".\\nfsu.ini");
	BanV3 = GetPrivateProfileInt("NFSU:LAN", "BanV3", 0, ".\\nfsu.ini");
	BanV4 = GetPrivateProfileInt("NFSU:LAN", "BanV4", 0, ".\\nfsu.ini");
	GetPrivateProfileString("NFSU:LAN", "ServerName", "LAN Service server", Server.Name, 100, ".\\nfsu.ini");
#endif

	time(&curtime);
	RoomClass *room;

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "A.GLOBAL");
	Server.Rooms.AddRoom(room);

	/*
	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "A.STOCK_ONLY");
	Server.Rooms.AddRoom(room);

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "A.STREET_ONLY");
	Server.Rooms.AddRoom(room);

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "A.PRO_ONLY");
	Server.Rooms.AddRoom(room);
	*/

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "B.GLOBAL");
	Server.Rooms.AddRoom(room);
	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "C.GLOBAL");
	Server.Rooms.AddRoom(room);
	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "D.GLOBAL");
	Server.Rooms.AddRoom(room);

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "E.GLOBAL");
	Server.Rooms.AddRoom(room);
	// tourney's room
	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "E.TOURNEY");
	Server.Rooms.AddRoom(room);

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "F.GLOBAL");
	Server.Rooms.AddRoom(room);
	// tourney's room
	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "F.TOURNEY");
	Server.Rooms.AddRoom(room);

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "G.GLOBAL");
	Server.Rooms.AddRoom(room);
	// tourney's room
	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "G.TOURNEY");
	Server.Rooms.AddRoom(room);

	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "H.GLOBAL");
	Server.Rooms.AddRoom(room);	
	// tourney's room
	room=(RoomClass*)calloc(1, sizeof(RoomClass));
	room->IsGlobal=true;
	strcpy(room->Name, "H.TOURNEY");
	Server.Rooms.AddRoom(room);

	int k;
	char log[1024];

	if(Server.Name[0]==0){
		strcpy(Server.Name, "Default server name");
	}

	//opening logfile
	if(EnableLogFile){
		if(RewriteLogFile){
			logfil=fopen("server.log", "w");
		}else{
			logfil=fopen("server.log", "a");
		}
		if(logfil==NULL){
			EnableLogFile=false;
			sprintf(log, "Could not open logfile - logging to file will be disabled.\n");
		}
	}
	
	//opening logfile
	if(LogAllTraffic){
		tlogfil=fopen("traffic.log", "w");
		if(tlogfil==NULL){
			LogAllTraffic=false;
			sprintf(log, "Could not open traffic logfile - logging to file will be disabled.\n");
		}
	}

	sprintf(log, "%s NFSU:LAN server [%s] v %s starting\n", Server.Name, SERVER_PLATFORM, NFSU_LAN_VERSION);
	Log(log);
	
#ifdef _WIN32
	WSAData wda;
	if(WSAStartup(MAKEWORD( 2, 2 ), &wda)!=0){
		sprintf(log, "Could not init winsocks.\n");
		Log(log);
		return false;
	}
#endif

	//reading news;
	LoadNews();
	
	//making sockets
	RedirectSocket = socket(AF_INET,SOCK_STREAM,0);
	ListeningSocket = socket(AF_INET,SOCK_STREAM,0);
	ReportingSocket = socket(AF_INET,SOCK_STREAM,0);
	ClientReportingSocket = socket(AF_INET, SOCK_DGRAM, 0);
	ClientReportingSocketTcp =  socket(AF_INET, SOCK_STREAM, 0);

	//binding them to specific ports
	SOCKADDR_IN localsin;
	localsin.sin_family = AF_INET;
	localsin.sin_addr.s_addr = INADDR_ANY;

	localsin.sin_port = htons(10900);
	k=bind(RedirectSocket,(SOCKADDR *)&localsin, sizeof(SOCKADDR_IN));
	if(k==INVALID_SOCKET){
		sprintf(log, "Could not bind socket to 10900.\n");
		Log(log);
		return false;
	}
	localsin.sin_port = htons(10901);
	k=bind(ListeningSocket,(SOCKADDR *)&localsin, sizeof(SOCKADDR_IN));
	if(k==INVALID_SOCKET){
		sprintf(log, "Could not bind socket to 10901.\n");
		Log(log);
		closesocket(RedirectSocket);
		return false;
	}
	localsin.sin_port = htons(10980);
	k=bind(ReportingSocket,(SOCKADDR *)&localsin, sizeof(SOCKADDR_IN));
	if(k==INVALID_SOCKET){
		sprintf(log, "Could not bind socket to 10980.\n");
		Log(log);
		closesocket(RedirectSocket);
		closesocket(ListeningSocket);
		return false;
	}
	localsin.sin_port = htons(10800);
	k=bind(ClientReportingSocket,(SOCKADDR *)&localsin, sizeof(SOCKADDR_IN));
	if(k==INVALID_SOCKET){
		sprintf(log, "Could not bind socket to 10800.\n");
		Log(log);
		closesocket(RedirectSocket);
		closesocket(ListeningSocket);
		closesocket(ReportingSocket);
		return false;
	}

	localsin.sin_port = htons(10800);
	k=bind(ClientReportingSocketTcp,(SOCKADDR *)&localsin, sizeof(SOCKADDR_IN));
	if(k==INVALID_SOCKET){
		sprintf(log, "Could not bind socket to 10800.\n");
		Log(log);
		closesocket(RedirectSocket);
		closesocket(ListeningSocket);
		closesocket(ReportingSocket);
		closesocket(ClientReportingSocket);
		return false;
	}

	//listen to ports
	listen(RedirectSocket, 100);
	listen(ListeningSocket, 100);
	listen(ReportingSocket, 100);
	listen(ClientReportingSocket, 100);
	listen(ClientReportingSocketTcp, 100);

	strcpy(RedirectConnections.Name, "RedirectConnections");
	strcpy(ClientConnections.Name, "ClientConnections");
	strcpy(ReportingConnections.Name, "ReportingConnections");

	//starting threads
	ConAccParam Redirector, Listener, Reporter;

	Redirector.Connections=&RedirectConnections;
	Redirector.sock=RedirectSocket;
	strcpy(Redirector.Name, "Redirector");
	Listener.Connections=&ClientConnections;
	Listener.sock=ListeningSocket;
	strcpy(Listener.Name, "Listener");
	Reporter.Connections=&ReportingConnections;
	Reporter.sock=ReportingSocket;
	strcpy(Reporter.Name, "Reporter");
	
	_beginthread(AcceptThread, 0, (void*)&Listener);
	_beginthread(AcceptThread, 0, (void*)&Redirector);
	//_beginthread(AcceptThread, 0, (void*)&Reporter);

	_beginthread(IOThread, 0, (void*)&RedirectConnections);
	_beginthread(IOThread, 0, (void*)&ClientConnections);
	//_beginthread(IOThread, 0, (void*)&ReportingConnections);

	_beginthread(RedirectorWorker, 0, NULL);
	_beginthread(ListenerWorker, 0, NULL);

	_beginthread(Maintenance, 0, NULL);

	//responses to client find_server requests
	_beginthread(ClientReporter, 0, NULL);
	_beginthread(ClientReporterTcp, 0, NULL);
	_beginthread(StatThread, 0, NULL);
	if(RegisterGlobal)
		_beginthread(WebReport, 0, NULL);

	return true;
}

void DeInitServer(){
	char log[1024];
	//closing sockets
	closesocket(RedirectSocket);
	closesocket(ListeningSocket);
	closesocket(ReportingSocket);
	closesocket(ClientReportingSocket);

	free(news);
	UserClass *tuser;
	GameClass *tgame;
	RoomClass *troom;
	RegUser *treguser;
	ConnectionClass *tcon;


	while(RedirectConnections.Count>0){
		tcon=RedirectConnections.First;
		RedirectConnections.RemoveConnection(tcon);
		if(tcon->user!=NULL) tcon->user->Connection=NULL;
		tcon->IncomingMessages.Clear();
		tcon->OutgoingMessages.Clear();
		tcon->IncomingMessages.mut.DeInit();
		tcon->OutgoingMessages.mut.DeInit();
		free(tcon);
	}

	while(Server.Users.Count>0) {
		tuser=Server.Users.First;
		Server.Users.RemoveUser(tuser);
		if(tuser->Game!=NULL){
			tgame=tuser->Game;
			tgame->RemoveUser(tuser, log);
			if(tgame->Count==0) {
				tuser->CurrentRoom->Games.RemoveGame(tgame);
				free(tgame);
			}
		}
		if(tuser->CurrentRoom!=NULL){
			troom=tuser->CurrentRoom;
			troom->RemoveUser(tuser, log);
			if(troom->Count==0){
				Server.Rooms.RemoveRoom(troom);
				free(troom);
			}
		}
		free(tuser);
	}
	while(Server.Rooms.Count>0){
		troom=Server.Rooms.First;
		Server.Rooms.RemoveRoom(troom);
		free(troom);
	}
	while(Server.ru.Count>0){
		treguser=Server.ru.First;
		Server.ru.RemoveUser(treguser);
		free(treguser);
	}

	//closing logfile
	if(logfil!=NULL) fclose(logfil);

#ifdef _WIN32
	//unloading winsocks
	WSACleanup();
#endif
};

#ifdef NT_SERVICE
// Second function to implement
VOID WINAPI ServiceMain( DWORD argc, LPTSTR *argv ) 
{ 
//    DWORD status; 
//    DWORD specificError; 

    ServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS; 
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP; 
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    ServiceStatusHandle = RegisterServiceCtrlHandler( "NFSU:LAN", ServiceCtrlHandler ); 
 
    if ( ServiceStatusHandle == (SERVICE_STATUS_HANDLE) 0 ) 
    { 
        sprintf(g_Msg, "NFSU:LAN RegisterServiceCtrlHandler() failed %d\n", GetLastError() ); 
        WriteInLogFile( g_Msg );
        return; 
    } 
 
//    status = ServiceInitialization( argc, argv, &specificError ); 
//    if ( status != NO_ERROR ) 
//    { 
//        ServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
//        ServiceStatus.dwCheckPoint         = 0; 
//        ServiceStatus.dwWaitHint           = 0; 
//        ServiceStatus.dwWin32ExitCode      = status; 
//        ServiceStatus.dwServiceSpecificExitCode = specificError; 
// 
//        SetServiceStatus( ServiceStatusHandle, &ServiceStatus ); 
//        return; 
//    } 

    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    if ( !SetServiceStatus( ServiceStatusHandle, &ServiceStatus ) ) 
    { 
        sprintf( g_Msg,"NFSU:LAN SetServiceStatus() error %ld\n", GetLastError() ); 
        WriteInLogFile(g_Msg);
    } 

    running = InitServer();

	if(running){
		sprintf( g_Msg, "Just before processing Loop...\n" );
		WriteInLogFile(g_Msg);

		ServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
		ServiceStatus.dwCheckPoint         = 0; 
		ServiceStatus.dwWaitHint           = 0; 
	 
		if ( !SetServiceStatus( ServiceStatusHandle, &ServiceStatus ) ) 
		{ 
			sprintf( g_Msg,"NFSU:LAN SetServiceStatus() error %ld\n", GetLastError() ); 
			WriteInLogFile(g_Msg);
		} 


		while (running)
		{
			Sleep(10);
		}
		DeInitServer();
	}
 
    ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
    
	// Send current status. 
    if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)) 
    { 
        sprintf(g_Msg,"HelloService! SetServiceStatus error %ld\n", GetLastError() ); 
        WriteInLogFile( g_Msg );
    } 
 
    return; 
} 
#endif


int main(int argc, char* argv[]){
	for(int k=0;k<30;k++) arr[k]=(char*)&arr2[k];
#ifdef NT_SERVICE	
//	The SERVICE_TABLE_ENTRY structure is used by the StartServiceCtrlDispatcher function
//	to specify the ServiceMain function for a service that can run in the calling process.

	SERVICE_TABLE_ENTRY DispatchTable[] = { { "NFSU:LAN", ServiceMain }, { 0, 0 } };

    sprintf( g_Msg, "Log opened\n" );
    WriteInLogFile( g_Msg );

	// Install a Service if -i switch used
    if ( argc > 1 && !( stricmp(argv[1], "-i") ) )
	{
        char szBuffer[255];
        char szPath[MAX_PATH];

        sprintf( g_Msg, "First Calling OpenSCManager() \n");
        WriteInLogFile( g_Msg );

//		The OpenSCManager function establishes a connection to the Service Control Manager(SCM) 
//		on the specified computer and opens the specified service control manager database. 

        SC_HANDLE scmHandle = OpenSCManager ( NULL, NULL, SC_MANAGER_ALL_ACCESS );

        if (scmHandle == NULL) // Perform error handling.
        {
            sprintf( g_Msg, "NFSU:LAN OpenSCManager error = %d\n", GetLastError() ); 
            WriteInLogFile(g_Msg);
        }

        GetModuleFileName( GetModuleHandle(NULL), szPath, MAX_PATH );
        
        strcpy( szBuffer, "\"" );
        strcat( szBuffer, szPath );
        strcat( szBuffer, "\"" );
 
		//printf( "\n CreateService()! Installing Service %s\n", szPath );

        SC_HANDLE scHandle;
		scHandle = CreateService (
			scmHandle, 
			"NFSU:LAN", 
            "NFSU:LAN", 
			SERVICE_ALL_ACCESS, 
            SERVICE_WIN32_OWN_PROCESS, 
            SERVICE_AUTO_START, 
            SERVICE_ERROR_NORMAL, 
            szBuffer, NULL, NULL, NULL, NULL, NULL );

        if ( scHandle == NULL ) // Process error
        {
            sprintf(g_Msg, " NFSU:LAN CreateService error = %d\n", GetLastError() ); 
            WriteInLogFile(g_Msg);
		}else{
			printf("NFSU:LAN service installed...");
		}

        CloseServiceHandle(scHandle);
        CloseServiceHandle(scmHandle);

    }
    else if ( argc > 1 && !( stricmp(argv[1], "-u" ) ) ) // Uninstall the Service
    {
        SC_HANDLE scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);

        if (scmHandle == NULL) // Perform error handling.
        {
            sprintf(g_Msg, "NFSU:LAN OpenSCManager error = %d\n", GetLastError() ); 
            WriteInLogFile(g_Msg);
        }
   
		SC_HANDLE scHandle;
        scHandle = OpenService( scmHandle, "NFSU:LAN", SERVICE_ALL_ACCESS );
		if(DeleteService( scHandle )!=0){
	        printf("NFSU:LAN service uninstalled...");
		}else{
			switch(GetLastError()){
				case ERROR_ACCESS_DENIED:
					printf("Access denied\n");
					break;
				case ERROR_INVALID_HANDLE:
					printf("Invalid handle\n");
					break;
				case ERROR_SERVICE_MARKED_FOR_DELETE:
					printf("The specified service has already been marked for deletion.");
					break;
				default:
					printf("Error\n");
					break;
			}
		}
    }
    else
    {
		SC_HANDLE scHandle;

		SC_HANDLE scmHandle = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
        scHandle = OpenService( scmHandle, "NFSU:LAN", SERVICE_ALL_ACCESS );

		if(scHandle){
			CloseHandle(scHandle);
			if ( !StartServiceCtrlDispatcher(DispatchTable) ) 
			{ 
				//ghmm smth wrong
				printf("To install service use param -i\nTo uninstall service use param -u\n");
			}
		}else{
			//service is not installed but is started without params
			printf("To install service use param -i\nTo uninstall service use param -u\n");
		}
    } 

    return 0;
#endif

#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	EnableLogFile=true;
	EnableLogScreen=true;
	RewriteLogFile=true;
	DisableTimeStamp=false;
	Verbose=false;
	RegisterGlobal=false;
	Server.Name[0]=0;
	LogAllTraffic=false;
	BanV1=false;
	BanV2=false;
	BanV3=false;
	BanV4=false;
	//2013-10-16
	BanCheater=false;
	BanRoom=false; 

#ifndef _WIN32
	bool daemon=false;
	bool writepid=false;
#endif

	//parse commandline
	for(int k=1;k<argc;k++){
#ifndef _WIN32
		if(stricmp(argv[k], "-d")==0){
			daemon=true;
		}
		if(stricmp(argv[k], "-p")==0){
			writepid=true;
		}
#endif
		if(stricmp(argv[k], "enablelogfile")==0){
			EnableLogFile=true;
		}
		if(stricmp(argv[k], "disablelogfile")==0){
			EnableLogFile=false;
		}
		if(stricmp(argv[k], "registerglobal")==0){
			RegisterGlobal=true;
		}
		if(stricmp(argv[k], "verbose")==0){
			Verbose=true;
		}
		if(stricmp(argv[k], "disablelogscreen")==0){
			EnableLogScreen=false;
		}
		if(stricmp(argv[k], "rewritelogfile")==0){
			RewriteLogFile=true;
		}
		if(stricmp(argv[k], "disabletimestamp")==0){
			DisableTimeStamp=true;
		}
		if(strstr(argv[k], "servername=")!=NULL){
			strncpy(Server.Name, strstr(argv[k], "servername=")+11, 99);
		}
		if(stricmp(argv[k], "logalltraffic")==0){
			LogAllTraffic=true;
		}
		if(stricmp(argv[k], "banv1")==0){
			BanV1=true;
		}
		if(stricmp(argv[k], "banv2")==0){
			BanV2=true;
		}
		if(stricmp(argv[k], "banv3")==0){
			BanV3=true;
		}
		if(stricmp(argv[k], "banv4")==0){
			BanV4=true;
		}
		// 2013-10-16
		if(stricmp(argv[k], "bancheater")==0){
			BanCheater=true;
		}
		// 2014-02-02
		if(stricmp(argv[k], "banroom")==0){
			BanRoom=true;
		}
	}

#ifndef _WIN32
	if(daemon){
		//diable output to console in deamon mode
		EnableLogScreen=false;
		/* Ignore terminal I/O signals */
		signal(SIGTTOU, SIG_IGN);
	    	signal(SIGTTIN, SIG_IGN);
	    	signal(SIGTSTP, SIG_IGN);
		int pid;
		pid = fork();
		if (pid != 0)
			exit(0); /* this is the parent process here, so let's exit it for our daemon work */
		setsid(); /* child process, so let's make us a session leader, and lose control tty */
		signal(SIGHUP, SIG_IGN); /* SIGHUP will be sent to child process so ignore it */
		if (pid != 0)
			exit(0);

		if(writepid){
			FILE *pidfile=fopen("nfsuserver.pid", "w");
			if(pidfile!=NULL){
				char pi[100];
				sprintf(pi, "%u", getpid());
				fwrite(pi, strlen(pi), sizeof(char), pidfile);
				fclose(pidfile);
			}
		}
	}	
#endif
	if(InitServer()){
		while(running){
			Sleep(10);
		}
	}
	DeInitServer();
	return 0;
}

