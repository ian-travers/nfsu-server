#include "win_nix.h"
#include "objects.h"

extern void Log(char *log);
extern bool Verbose;

extern char *arr[30];
extern char arr2[30][1024];

extern std::vector<PlayerStat> PS;

std::vector<StarsLap> S1001;
std::vector<StarsLap> S1002;
std::vector<StarsLap> S1003;
std::vector<StarsLap> S1004;
std::vector<StarsLap> S1005;
std::vector<StarsLap> S1006;
std::vector<StarsLap> S1007;
std::vector<StarsLap> S1008;
std::vector<StarsLap> S1102;
std::vector<StarsLap> S1103;
std::vector<StarsLap> S1104;
std::vector<StarsLap> S1105;
std::vector<StarsLap> S1106;
std::vector<StarsLap> S1107;
std::vector<StarsLap> S1108;
std::vector<StarsLap> S1109;
std::vector<StarsLap> S1201;
std::vector<StarsLap> S1202;
std::vector<StarsLap> S1206;
std::vector<StarsLap> S1207;
std::vector<StarsLap> S1210;
std::vector<StarsLap> S1214;
std::vector<StarsDrift> S1301;
std::vector<StarsDrift> S1302;
std::vector<StarsDrift> S1303;
std::vector<StarsDrift> S1304;
std::vector<StarsDrift> S1305;
std::vector<StarsDrift> S1306;
std::vector<StarsDrift> S1307;
std::vector<StarsDrift> S1308;

ServerClass Server; //core ;)
SessionsClass Sessions; // started races players info

UserClass::UserClass() {
	memset (Username, 0, sizeof (Username));
	memset (Personas, 0, sizeof (Personas));
	memset (IP, 0, 20);
	memset (Port, 0, 10);
	SelectedPerson = -1;
	GetMsg = false;
	Idle = 0;
} ;

ServerClass::ServerClass() {
	time (&Startup);
	memset (&ru, 0, sizeof (ru));
	LoadSettings ();
	LoadStat ();
	LoadStars(1001);
	LoadStars(1002);
	LoadStars(1003);
	LoadStars(1004);
	LoadStars(1005);
	LoadStars(1006);
	LoadStars(1007);
	LoadStars(1008);
	LoadStars(1102);
	LoadStars(1103);
	LoadStars(1104);
	LoadStars(1105);
	LoadStars(1106);
	LoadStars(1107);
	LoadStars(1108);
	LoadStars(1109);
	LoadStars(1201);
	LoadStars(1202);
	LoadStars(1206);
	LoadStars(1207);
	LoadStars(1210);
	LoadStars(1214);
	LoadStarsDrift(1301);
	LoadStarsDrift(1302);
	LoadStarsDrift(1303);
	LoadStarsDrift(1304);
	LoadStarsDrift(1305);
	LoadStarsDrift(1306);
	LoadStarsDrift(1307);
	LoadStarsDrift(1308);
} ;

UsersClass::UsersClass() {
	cid = 1;
	Count = 0;
	First = NULL;
} ;

void UserClass::SelectPerson( char *person ) {
	int k;
	k = 0;

	while (Personas[k][0] != 0) {
		if (strcmp (Personas[k], person) == 0) {
			SelectedPerson = k;
			return;
		}
		k++;
	}
} ;

UserClass *UsersClass::UserFromUsername( char *username ) {
	UserClass * tmp;
	tmp = First;

	while (tmp != NULL) {
		if ((strcmp (tmp->Username, username) == 0) || (strcmp (tmp->Personas[tmp->SelectedPerson], username) == 0)) {
			return tmp;
		}
		tmp = tmp->Next;
	}

	return NULL;
} ;

void UsersClass::AddUser( UserClass *user ) {
	memset (user->car, 0, 1024);
	user->Game = NULL;

	if (user->id == -1) user->id = cid++;

	Count++;
	user->Next = NULL;
	user->Idle = 0;

	if (First == NULL) {
		First = user;
		return;
	}

	UserClass * tmp;
	tmp = First;

	while (tmp->Next != NULL)
		tmp = tmp->Next;

	tmp->Next = user;
}

RUsersClass::RUsersClass() {
	First = NULL;
	Count = 0;
}

void RUsersClass::AddUser( RUserClass *user ) {
	Count++;
	user->Next = NULL;

	if (First == NULL) {
		First = user;
		return;
	}

	RUserClass * tmp;
	tmp = First;

	while (tmp->Next != NULL)
		tmp = tmp->Next;

	tmp->Next = user;
}

void RUsersClass::RemoveUser( RUserClass *user ) {
	if (Count == 1) {
		First = NULL;
	} else {
		if (First == user) {
			First = First->Next;
		} else {
			RUserClass * tmp, * prev;
			tmp = First;
			prev = tmp;

			while ((tmp != user) && (tmp != NULL)) {
				prev = tmp;
				tmp = tmp->Next;
			}
			if (tmp != NULL) {
				prev->Next = tmp->Next;
			}
		}
	}
	Count--;
}

void UsersClass::RemoveUser( UserClass *user ) {
	if (Count == 1) {
		First = NULL;
	} else {
		if (First == user) {
			First = First->Next;
		} else {
			UserClass * tmp, * prev;
			tmp = First;
			prev = tmp;

			while ((tmp != user) && (tmp != NULL)) {
				prev = tmp;
				tmp = tmp->Next;
			}
			if (tmp != NULL) {
				prev->Next = tmp->Next;
			}
		}
	}
	Count--;
}

void ServerClass::SendRoomsToUser( UserClass *user, char *buffer ) {
	RoomClass * tmp;
	tmp = Rooms.First;
	while (tmp != NULL) {
		if ((tmp->Count > 0) || (tmp->IsGlobal)) {
			sprintf (arr2[0], "I=%u", tmp->ID);
			sprintf (arr2[1], "N=%s", tmp->Name);
			sprintf (arr2[2], "H=3PriedeZ");
			sprintf (arr2[3], "F=CK");
			sprintf (arr2[4], "T=%u", tmp->Count);
			sprintf (arr2[5], "L=50");
			sprintf (arr2[6], "P=0");
			if (user->Connection != NULL)
				user->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "+rom", arr, 7));
		} else {
			sprintf (buffer, "Error:Room empty\n");
			if (Verbose) Log (buffer);
		}
		tmp = tmp->Next;
	}
} ;

bool ServerClass::RegisterUser( char *username ) {
	RegUser * temp;
	temp = ru.UserFromUsername (username);

	if (temp == NULL) {
		temp = (RegUser *)calloc (1, sizeof (RegUser));
		strcpy (temp->Username, username);
		ru.AddUser (temp);
		return true;
	} else {
		return false;
	} ;
} ;

RoomsClass::RoomsClass() {
	cid = 1;
	Count = 0;
}

void RoomsClass::AddRoom( RoomClass *room ) {
	Count++;
	room->Next = NULL;
	room->Count = 0;
	room->ID = cid++;

	if (First == NULL) {
		First = room;
		return;
	}

	RoomClass * tmp;
	tmp = First;

	while (tmp->Next != NULL)
		tmp = tmp->Next;

	tmp->Next = room;
}

void RoomsClass::RemoveRoom( RoomClass *room ) {
	if (Count == 1) {
		First = NULL;
	} else {
		if (First == room) {
			First = First->Next;
		} else {
			RoomClass * tmp, * prev;
			tmp = First;
			prev = tmp;

			while ((tmp != room) && (tmp != NULL)) {
				prev = tmp;
				tmp = tmp->Next;
			}
			if (tmp != NULL) {
				prev->Next = tmp->Next;
			}
		}
	}
	Count--;
	room->Name[0] = 0;
}

GameClass *GamesClass::GameFromName( char *name ) {
	GameClass * tmp;
	tmp = First;

	while (tmp != NULL) {
		if (strcmp (tmp->Name, name) == 0) {
			return tmp;
		}
		tmp = tmp->Next;
	}
	return NULL;
} ;

void SessionsClass::AddSession(SessionClass *session){
	Count++;
	session->Next=NULL;

	if(First==NULL){
		First=session;		
		return;
	}
	SessionClass *tmp;
	tmp=First;
	while(tmp->Next!=NULL) tmp=tmp->Next;
	tmp->Next=session;
};

void SessionsClass::RemoveSession(SessionClass *session){
	if(Count==1){
		First=NULL;
	}else{
		if (First==session){
			First=First->Next;
		}else{
			SessionClass *tmp, *prev;
			tmp=First;
			prev=tmp;
			while((tmp!=session)&&(tmp!=NULL)){
				prev=tmp;
				tmp=tmp->Next;
			}
			if(tmp!=NULL){
				prev->Next=tmp->Next;
			}
		}
	}
	Count--;
};

SessionsClass::SessionsClass(){
	First=0;
	Count=0;
};

void GamesClass::AddGame( GameClass *game ) {
	Count++;

	if (cid < 1) cid = 1;

	game->Next = NULL;
	game->Count = 0;
	game->ID = cid++;

	if (First == NULL) {
		First = game;
		return;
	}
	GameClass * tmp;
	tmp = First;

	while (tmp->Next != NULL)
		tmp = tmp->Next;

	tmp->Next = game;
}

void GamesClass::RemoveGame( GameClass *game ) {
	if (Count == 1) {
		First = NULL;
	} else {
		if (First == game) {
			First = First->Next;
		} else {
			GameClass * tmp, * prev;
			tmp = First;
			prev = tmp;

			while ((tmp != game) && (tmp != NULL)) {
				prev = tmp;
				tmp = tmp->Next;
			}
			if (tmp != NULL) {
				prev->Next = tmp->Next;
			}
		}
	}
	Count--;
	game->Name[0] = 0;
}

void UsersClass::Broadcast( char *buf, unsigned int len ) {
	UserClass * tmp;
	tmp = First;

	while (tmp != NULL) {
		MessageClass * msg = (MessageClass *)calloc (1, sizeof (MessageClass));
		msg->Message = (char *)calloc (len, sizeof (char));
		msg->Size = len;
		tmp->Connection->OutgoingMessages.AddMessage (msg);
		tmp = tmp->Next;
	}
} ;

RoomClass *RoomsClass::RoomFromName( char *name ) {
	RoomClass * tmp;
	tmp = First;

	while (tmp != NULL) {
		if (strcmp (tmp->Name, name) == 0) return tmp;
		tmp = tmp->Next;
	}
	return NULL;
} ;

void GameClass::SendInfoToUser( UserClass *user, char *buffer ) {
	sprintf (buffer, "GameClass::SendInfoToUser\n");
	if (Verbose) Log (buffer);
	if (Users.Count == 0) {
		sprintf (buffer, "Error: no users in game\n");
		if (Verbose) Log (buffer);
		return;
	}
	int k;
	sprintf (arr2[0], "IDENT=%u", ID);
	sprintf (arr2[1], "WHEN=2003.12.8 15:52:54");
	sprintf (arr2[2], "NAME=%s", Name);
	sprintf (arr2[3], "HOST=%s", Users.First->User->Personas[Users.First->User->SelectedPerson]);
	sprintf (arr2[4], "PARAMS=%s", params);
	sprintf (arr2[5], "ROOM=%s", user->CurrentRoom->Name);
	sprintf (arr2[6], "MAXSIZE=%u", max);
	sprintf (arr2[7], "MINSIZE=%u", min);
	sprintf (arr2[8], "COUNT=%u", Count);
	sprintf (arr2[9], "USERFLAGS=0");
	sprintf (arr2[10], "SYSFLAGS=%u", sysflags);

	k = rand ();
	int l = 0;
	int m = 0;
	RUserClass * ru = Users.First;

	while (ru != NULL) {
		sprintf (arr2[11 + l * 3], "OPID%u=%u", l, ru->User->id);
		sprintf (arr2[12 + l * 3], "OPPO%u=%s", l, ru->User->Personas[ru->User->SelectedPerson]);
		sprintf (arr2[13 + l * 3], "ADDR%u=%s", l, ru->User->IP);
		l++;
		ru = ru->Next;
	}
	if (user->Connection != NULL)
		user->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "+agm", arr, 11 + l * 3));
} ;

void GameClass::RemoveUser( UserClass *user, char *buffer ) {
	RUserClass * tmp;
	tmp = Users.First;

	while (tmp != NULL) {
		if (tmp->User == user) {
			Users.RemoveUser (tmp);
			Count--;
			sprintf (buffer, "User removed - %u users in game\n", Count);
			if (Verbose) Log (buffer);
			user->Game = NULL;
			int k;

			if (Count > 0) {
				sprintf (arr2[0], "IDENT=%u", ID);
				sprintf (arr2[1], "WHEN=2003.12.8 15:52:54");
				sprintf (arr2[2], "NAME=%s", Name);
				sprintf (arr2[3], "HOST=%s", Users.First->User->Personas[Users.First->User->SelectedPerson]);
				sprintf (arr2[4], "PARAMS=%s", params);
				sprintf (arr2[5], "ROOM=%s", user->CurrentRoom->Name);
				sprintf (arr2[6], "MAXSIZE=%u", max);
				sprintf (arr2[7], "MINSIZE=%u", min);
				sprintf (arr2[8], "COUNT=%u", Count);
				sprintf (arr2[9], "USERFLAGS=0");
				sprintf (arr2[10], "SYSFLAGS=%u", sysflags);

				k = rand ();
				int l = 0;
				int m = 0;
				RUserClass * ru = Users.First;

				while (ru != NULL) {
					sprintf (arr2[11 + l * 3], "OPID%u=%u", l, ru->User->id);
					sprintf (arr2[12 + l * 3], "OPPO%u=%s", l, ru->User->Personas[ru->User->SelectedPerson]);
					sprintf (arr2[13 + l * 3], "ADDR%u=%s", l, ru->User->IP);
					l++;
					ru = ru->Next;
				}
				if (user->Connection != NULL) {
					user->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "glea", arr, 11 + l * 3));
					user->CurrentRoom->RefreshUser (user, buffer);
				}
				if (max == Users.Count) {
					StartGame (buffer);
				}
				sprintf (arr2[0], "IDENT=%u", ID);
				sprintf (arr2[1], "WHEN=2003.12.8 15:52:54");
				sprintf (arr2[2], "NAME=%s", Name);
				sprintf (arr2[3], "HOST=%s", Users.First->User->Personas[Users.First->User->SelectedPerson]);
				sprintf (arr2[4], "PARAMS=%s", params);
				sprintf (arr2[5], "ROOM=%s", user->CurrentRoom->Name);
				sprintf (arr2[6], "MAXSIZE=%u", max);
				sprintf (arr2[7], "MINSIZE=%u", min);
				sprintf (arr2[8], "COUNT=%u", Count);
				sprintf (arr2[9], "USERFLAGS=0");
				sprintf (arr2[10], "SYSFLAGS=%u", sysflags);

				l = 0;
				m = 0;
				ru = Users.First;

				while (ru != NULL) {
					sprintf (arr2[11 + l * 3], "OPID%u=%u", l, ru->User->id);
					sprintf (arr2[12 + l * 3], "OPPO%u=%s", l, ru->User->Personas[ru->User->SelectedPerson]);
					sprintf (arr2[13 + l * 3], "ADDR%u=%s", l, ru->User->IP);
					l++;
					ru = ru->Next;
				}
				BroadCastCommand (user->CurrentRoom->Users, "+agm", arr, 11 + l * 3, buffer);
			} else {
				sprintf (arr2[0], "IDENT=%u", ID);
				BroadCastCommand (user->CurrentRoom->Users, "+agm", arr, 1, buffer);
			}
			free (tmp);
			tmp = NULL;
			return;
		} else {
			tmp = tmp->Next;
		}
	}
} ;

void GameClass::AddUser( UserClass *user, char *buffer ) {
	sprintf (buffer, "Add user\n");
	if (Verbose) Log (buffer);
	user->Game = this;
	Count++;
	RUserClass * tm = (RUserClass *)calloc (1, sizeof (RUserClass));
	tm->User = user;
	Users.AddUser (tm);
	int k;

	sprintf (arr2[0], "IDENT=%u", ID);
	sprintf (arr2[1], "WHEN=2003.12.8 15:52:54");
	sprintf (arr2[2], "NAME=%s", Name);
	sprintf (arr2[3], "HOST=%s", Users.First->User->Personas[Users.First->User->SelectedPerson]);
	sprintf (arr2[4], "PARAMS=%s", params);
	sprintf (arr2[5], "ROOM=%s", user->CurrentRoom->Name);
	sprintf (arr2[6], "MAXSIZE=%u", max);
	sprintf (arr2[7], "MINSIZE=%u", min);
	sprintf (arr2[8], "COUNT=%u", Count);
	sprintf (arr2[9], "USERFLAGS=0");
	sprintf (arr2[10], "SYSFLAGS=%u", sysflags);

	k = rand ();
	int l = 0;
	int m = 0;
	RUserClass * ru = Users.First;

	while (ru != NULL) {
		sprintf (arr2[11 + l * 3], "OPID%u=%u", l, ru->User->id);
		sprintf (arr2[12 + l * 3], "OPPO%u=%s", l, ru->User->Personas[ru->User->SelectedPerson]);
		sprintf (arr2[13 + l * 3], "ADDR%u=%s", l, ru->User->IP);
		l++;
		ru = ru->Next;
	}
	if (user->Connection) user->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "gjoi", arr, 11 + l * 3));

	user->CurrentRoom->RefreshUser (user, buffer);

	if (max == Users.Count) {
		StartGame (buffer);
	}
	sprintf (arr2[0], "IDENT=%u", ID);
	sprintf (arr2[1], "WHEN=2003.12..8 15:52:54");
	sprintf (arr2[2], "NAME=%s", Name);
	sprintf (arr2[3], "HOST=%s", Users.First->User->Personas[Users.First->User->SelectedPerson]);
	sprintf (arr2[4], "PARAMS=%s", params);
	sprintf (arr2[5], "ROOM=%s", user->CurrentRoom->Name);
	sprintf (arr2[6], "MAXSIZE=%u", max);
	sprintf (arr2[7], "MINSIZE=%u", min);
	sprintf (arr2[8], "COUNT=%u", Count);
	sprintf (arr2[9], "USERFLAGS=0");
	sprintf (arr2[10], "SYSFLAGS=%u", sysflags);

	l = 0;
	m = 0;
	ru = Users.First;

	while (ru != NULL) {
		sprintf (arr2[11 + l * 3], "OPID%u=%u", l, ru->User->id);
		sprintf (arr2[12 + l * 3], "OPPO%u=%s", l, ru->User->Personas[ru->User->SelectedPerson]);
		sprintf (arr2[13 + l * 3], "ADDR%u=%s", l, ru->User->IP);
		l++;
		ru = ru->Next;
	}
	BroadCastCommand (user->CurrentRoom->Users, "+agm", arr, 11 + l * 3, buffer);
}

void GameClass::StartGame( char *buffer ) {
	sprintf (buffer, "start game\n");
	if (Verbose) Log (buffer);
	int k;

	sprintf (arr2[0], "IDENT=%u", ID);
	sprintf (arr2[1], "WHEN=2003.12.8 15:52:54");
	sprintf (arr2[2], "NAME=%s", Name);
	sprintf (arr2[3], "HOST=%s", Users.First->User->Personas[Users.First->User->SelectedPerson]);
	sprintf (arr2[4], "PARAMS=%s", params);
	sprintf (arr2[5], "ROOM=%s", Users.First->User->CurrentRoom->Name);
	sprintf (arr2[6], "MAXSIZE=%u", max);
	sprintf (arr2[7], "MINSIZE=%u", min);
	sprintf (arr2[8], "COUNT=%u", Count);
	sprintf (arr2[9], "USERFLAGS=0");
	sprintf (arr2[10], "SYSFLAGS=%u", sysflags);

	k = rand ();
	int l = 0;
	int m = 0;
	RUserClass * ru = Users.First;

	while (ru != NULL) {
		sprintf (arr2[11 + l * 3], "OPID%u=%u", l, ru->User->id);
		sprintf (arr2[12 + l * 3], "OPPO%u=%s", l, ru->User->Personas[ru->User->SelectedPerson]);
		sprintf (arr2[13 + l * 3], "ADDR%u=%s", l, ru->User->IP);
		l++;
		ru = ru->Next;
	}
	ru = Users.First;
	SessionClass *session;

	while (ru != NULL) {
		sprintf (arr2[11 + l * 3], "SEED=%u", k);
		sprintf (arr2[12 + l * 3], "SELF=%s", ru->User->Personas[ru->User->SelectedPerson]);

		if (ru->User->Connection != NULL){
			ru->User->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "+ses", arr, 13 + l * 3));
			/*
			  добаляем инфу о начавшихся сессиях (в понимании сервера сессия - это заезд!)
			  нужно сохранить инфу об имени игрока и типе комнаты, чтоб определить
			  в дальнейшем рейтинговый заезд был или нет.
			//*/
			session=(SessionClass*)calloc(1, sizeof(SessionClass));
			strcpy(session->Persona, ru->User->Personas[ru->User->SelectedPerson]);
			strcpy(session->FromRoom, Users.First->User->CurrentRoom->Name);
			strcpy(session->IP, ru->User->IP);
			session->Idle = 0;
			Sessions.AddSession(session);
		}
		ru = ru->Next;
	}
	BroadCastCommand (Users.First->User->CurrentRoom->Users, "+agm", arr, 11 + l * 3, buffer);
}

void RoomClass::AddUser( UserClass *user, char *buffer ) {
	if (Verbose) Log ("Add user\n");

	sprintf (arr2[0], "IDENT=%u", ID);
	sprintf (arr2[1], "NAME=%s", Name);
	sprintf (arr2[2], "COUNT=%u", Users.Count);
	sprintf (arr2[3], "FLAGS=C");

	if (user->Connection != NULL) user->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "move", arr, 4));

	user->CurrentRoom = this;
	Count++;
	RUserClass * tm = (RUserClass *)calloc (1, sizeof (RUserClass));
	tm->User = user;

	Users.AddUser (tm);

	sprintf (buffer, "User added - %u users in room\n", Count);
	if (Verbose) Log (buffer);

	sprintf (arr2[0], "I=%u", user->id);
	sprintf (arr2[1], "N=%s", user->Personas[user->SelectedPerson]);
	sprintf (arr2[2], "M=%s", user->Username);
	sprintf (arr2[3], "F=");
	sprintf (arr2[4], "A=%s", user->IP);
	sprintf (arr2[5], "S=%s", GetPlayerStat(user->Personas[user->SelectedPerson]));
	sprintf (arr2[6], "X=%s", user->car);
	sprintf (arr2[7], "R=%s", Name);
	sprintf (arr2[8], "RI=%u", ID);
	sprintf (arr2[9], "RF=C");
	sprintf (arr2[10], "RT=1");

	if (user->Connection != NULL) user->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "+who", arr, 11));

	RefreshUser (user, buffer);

	GameClass * game = Games.First;

	while (game != NULL) {
		game->SendInfoToUser (user, buffer);
		game = game->Next;
	}
	ListToUser (user, buffer);
} ;

void RoomClass::RefreshUser( UserClass *user, char *buffer ) {
	sprintf (buffer, "RoomClass::RefreshUser\n");
	if (Verbose) Log (buffer);

	sprintf (arr2[0], "I=%u", user->id);
	sprintf (arr2[1], "N=%s", user->Personas[user->SelectedPerson]);
	sprintf (arr2[2], "M=%s", user->Username);
	sprintf (arr2[3], "F=H");
	sprintf (arr2[4], "A=%s", user->IP);
	sprintf (arr2[5], "P=211");
	sprintf (arr2[6], "S=%s", GetPlayerStat(user->Personas[user->SelectedPerson]));
	sprintf (arr2[7], "X=%s", user->car);

	if (user->Game != NULL) {
		sprintf (arr2[8], "G=%u", user->Game->ID);
	} else {
		sprintf (arr2[8], "G=0");
	}
	sprintf (arr2[9], "T=2");
	BroadCastCommand (Users, "+usr", arr, 10, buffer);
} ;

void RoomClass::RemoveUser( UserClass *user, char *buffer ) {
	RUserClass * tmp;
	tmp = Users.First;

	while (tmp != NULL) {
		if (tmp->User == user) {
			Users.RemoveUser (tmp);
			user->CurrentRoom = NULL;
			Count--;
			sprintf (buffer, "User removed - %u users in room\n", Count);
			if (Verbose) Log (buffer);

			sprintf (arr2[0], "I=%u", user->id);
			sprintf (arr2[1], "T=1");

			BroadCastCommand (Users, "+usr", arr, 2, buffer);

			sprintf (arr2[0], "F=C");
			sprintf (arr2[1], "T=\"has left the room\"");
			sprintf (arr2[2], "N=%s", user->Personas[user->SelectedPerson]);

			BroadCastCommand (Users, "+msg", arr, 3, buffer);

			free (tmp);
			tmp = NULL;
			return;
		} else {
			tmp = tmp->Next;
		}
	}
} ;

void RoomClass::ListToUser( UserClass *user, char *buffer ) {
	RUserClass * tmp;
	tmp = Users.First;
	sprintf (buffer, "Sending list\n");
	if (Verbose) Log (buffer);

	while (tmp != NULL) {
		sprintf (arr2[0], "I=%u", tmp->User->id);
		sprintf (arr2[1], "N=%s", tmp->User->Personas[tmp->User->SelectedPerson]);
		sprintf (arr2[2], "M=%s", tmp->User->Username);
		sprintf (arr2[3], "F=H");
		sprintf (arr2[4], "A=%s", tmp->User->IP);
		sprintf (arr2[5], "P=211");
		sprintf (arr2[6], "S=%s", GetPlayerStat(tmp->User->Personas[tmp->User->SelectedPerson]));
		sprintf (arr2[7], "X=%s", tmp->User->car);

		if (tmp->User->Game != NULL) {
			sprintf (arr2[8], "G=%u", tmp->User->Game->ID);
		} else {
			sprintf (arr2[8], "G=0");
		}
		sprintf (arr2[9], "T=2");

		if (user->Connection != NULL)
			user->Connection->OutgoingMessages.AddMessage (MakeMessage (buffer, "+usr", arr, 10));
		tmp = tmp->Next;
	}
} ;

RegUser *RegUsers::UserFromUsername( char *username ) {
	RegUser * temp = First;

	while (temp != NULL) {
		if (stricmp (temp->Username, username) == 0) {
			return temp;
		}
		temp = temp->Next;
	}
	return NULL;
} ;

void RegUsers::AddUser( RegUser *user ) {
	user->Next = NULL;

	if (Count == 0) {
		First = user;
		Last = user;
	} else {
		Last->Next = user;
		Last = user;
	}
	Count++;
} ;

void RegUsers::RemoveUser( RegUser *user ) {
	if (user == NULL) return;

	if (Count == 0) return;

	if (Count == 1) {
		First = NULL;
		Last = NULL;
		Count = 0;
	} else {
		if (First == user) {
			First = First->Next;
			Count--;
			return;
		}
		RegUser * prev, * temp;
		temp = First;
		prev = NULL;

		while ((temp != user) && (temp != NULL)) {
			prev = temp;
			temp = temp->Next;
		}
		if (prev != NULL) {
			if (temp == Last) {
				Last = prev;
				prev->Next = NULL;
			} else {
				prev->Next = temp->Next;
			}
			Count--;
		}
	}
} ;

void ServerClass::SaveSettings() {
	FILE * fil;
	fil = fopen ("rusers.dat", "wb");

	if (fil == NULL) return;

	RegUser * temp = ru.First;

	while (temp != NULL) {
		if (temp->Username[0] != 0) {
			fwrite (temp, 1, sizeof (RegUser), fil);
		}
		temp = temp->Next;
	}
	fclose (fil);
} ;

void ServerClass::LoadSettings() {
	while (ru.Count > 0)
		ru.RemoveUser (ru.Last);

	FILE * fil;
	fil = fopen ("rusers.dat", "rb");

	if (fil == NULL) return;

	RegUser * temp;

	while (!feof (fil)) {
		temp = (RegUser *)calloc (1, sizeof (RegUser));
		fread (temp, 1, sizeof (RegUser), fil);
		if (temp->Username[0]!=0){
			temp->Next = NULL;
			ru.AddUser (temp);
		}
		temp->Next = NULL;
		ru.AddUser (temp);
	}
	fclose (fil);
} ;

void ServerClass::SaveStat (){
	FILE * fil;

	fil = fopen ("stat.dat", "wb");
	if (fil == NULL) return;
	
	std::vector<PlayerStat>::iterator it;
	PlayerStat * tps;
	tps = (PlayerStat *)calloc(1, sizeof(PlayerStat));

	for (it = PS.begin(); it !=PS.end(); it++) {
		if (it->Name[0]!=0) {
			strcpy(tps->Name, it->Name);
			tps->Rating_All = it->Rating_All;
			tps->Wins_All = it->Wins_All;
			tps->Loses_All = it->Loses_All;
			tps->Disc_All = it->Disc_All;
			tps->REP_All = it->REP_All;
			tps->OppsREP_All = it->OppsREP_All;
			tps->OppsRating_All = it->OppsRating_All;
			tps->Rating_Circ = it->Rating_Circ;
			tps->Wins_Circ = it->Wins_Circ;
			tps->Loses_Circ = it->Loses_Circ;
			tps->Disc_Circ = it->Disc_Circ;
			tps->REP_Circ = it->REP_Circ;
			tps->OppsREP_Circ = it->OppsREP_Circ;
			tps->OppsRating_Circ = it->OppsRating_Circ;
			tps->Rating_Sprint = it->Rating_Sprint;
			tps->Wins_Sprint = it->Wins_Sprint;
			tps->Loses_Sprint = it->Loses_Sprint;
			tps->Disc_Sprint = it->Disc_Sprint;
			tps->REP_Sprint = it->REP_Sprint;
			tps->OppsREP_Sprint = it->OppsREP_Sprint;
			tps->OppsRating_Sprint = it->OppsRating_Sprint;
			tps->Rating_Drag = it->Rating_Drag;
			tps->Wins_Drag = it->Wins_Drag;
			tps->Loses_Drag = it->Loses_Drag;
			tps->Disc_Drag = it->Disc_Drag;
			tps->REP_Drag = it->REP_Drag;
			tps->OppsREP_Drag = it->OppsREP_Drag;
			tps->OppsRating_Drag = it->OppsRating_Drag;
			tps->Rating_Drift = it->Rating_Drift;
			tps->Wins_Drift = it->Wins_Drift;
			tps->Loses_Drift = it->Loses_Drift;
			tps->Disc_Drift = it->Disc_Drift;
			tps->REP_Drift = it->REP_Drift;
			tps->OppsREP_Drift = it->OppsREP_Drift;
			tps->OppsRating_Drift = it->OppsRating_Drift;
			
			fwrite (tps, 1, sizeof (PlayerStat), fil);
		}
	}
	fclose (fil);
} ;

void ServerClass::LoadStat (){
	FILE * fil;

	fil = fopen ("stat.dat", "rb");

	if (fil == NULL) return;

	PlayerStat *tps;

	while (!feof (fil)){
		tps = (PlayerStat *)calloc (1, sizeof(PlayerStat));
		fread (tps, 1, sizeof(PlayerStat), fil);
		if (tps->Name[0]!=0) {
			PS.push_back(PlayerStat(tps->Name,
				tps->Rating_All, tps->Wins_All, tps->Loses_All, tps->Disc_All, tps->REP_All, tps->OppsREP_All, tps->OppsRating_All,
				tps->Rating_Circ, tps->Wins_Circ, tps->Loses_Circ, tps->Disc_Circ, tps->REP_Circ, tps->OppsREP_Circ, tps->OppsRating_Circ,
				tps->Rating_Sprint, tps->Wins_Sprint, tps->Loses_Sprint, tps->Disc_Sprint, tps->REP_Sprint, tps->OppsREP_Sprint, tps->OppsRating_Sprint,
				tps->Rating_Drag, tps->Wins_Drag, tps->Loses_Drag, tps->Disc_Drag, tps->REP_Drag, tps->OppsREP_Drag, tps->OppsRating_Drag,
				tps->Rating_Drift, tps->Wins_Drift, tps->Loses_Drift, tps->Disc_Drift, tps->REP_Drift, tps->OppsREP_Drift, tps->OppsRating_Drift));
		}
	}
	fclose (fil);
};

void ServerClass::LoadStars(int track){
	FILE * fil;

	switch (track){
		case 1001:
			fil = fopen("s1001.dat", "rb");
			break;
		case 1002:
			fil = fopen("s1002.dat", "rb");
			break;
		case 1003:
			fil = fopen("s1003.dat", "rb");
			break;
		case 1004:
			fil = fopen("s1004.dat", "rb");
			break;
		case 1005:
			fil = fopen("s1005.dat", "rb");
			break;
		case 1006:
			fil = fopen("s1006.dat", "rb");
			break;
		case 1007:
			fil = fopen("s1007.dat", "rb");
			break;
		case 1008:
			fil = fopen("s1008.dat", "rb");
			break;
		case 1102:
			fil = fopen("s1102.dat", "rb");
			break;
		case 1103:
			fil = fopen("s1103.dat", "rb");
			break;
		case 1104:
			fil = fopen("s1104.dat", "rb");
			break;
		case 1105:
			fil = fopen("s1105.dat", "rb");
			break;
		case 1106:
			fil = fopen("s1106.dat", "rb");
			break;
		case 1107:
			fil = fopen("s1107.dat", "rb");
			break;
		case 1108:
			fil = fopen("s1108.dat", "rb");
			break;
		case 1109:
			fil = fopen("s1109.dat", "rb");
			break;
		case 1201:
			fil = fopen("s1201.dat", "rb");
			break;
		case 1202:
			fil = fopen("s1202.dat", "rb");
			break;
		case 1206:
			fil = fopen("s1206.dat", "rb");
			break;
		case 1207:
			fil = fopen("s1207.dat", "rb");
			break;
		case 1210:
			fil = fopen("s1210.dat", "rb");
			break;
		case 1214:
			fil = fopen("s1214.dat", "rb");
			break;
	}
	if (fil == NULL) return;

	StarsLap *tsl;

	while (!feof (fil)){
		tsl = (StarsLap *)calloc (1, sizeof(StarsLap));
		fread (tsl, 1, sizeof(StarsLap), fil);

		if (tsl->Name[0] !=0) {
			switch (track) {
				case 1001:
					S1001.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1002:
					S1002.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1003:
					S1003.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1004:
					S1004.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1005:
					S1005.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1006:
					S1006.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1007:
					S1007.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1008:
					S1008.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1102:
					S1102.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1103:
					S1103.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1104:
					S1104.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1105:
					S1105.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1106:
					S1106.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1107:
					S1107.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1108:
					S1108.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1109:
					S1109.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1201:
					S1201.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1202:
					S1202.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1206:
					S1206.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1207:
					S1207.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1210:
					S1210.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
				case 1214:
					S1214.push_back(StarsLap(tsl->Name, tsl->Time, tsl->Car, tsl->Dir));
					break;
			}
		}
	}
	fclose (fil);
};

void ServerClass::SaveStars(int track){
	FILE * fil;

	switch (track){
		case 1001:
			fil = fopen("s1001.dat", "wb");
			break;
		case 1002:
			fil = fopen("s1002.dat", "wb");
			break;
		case 1003:
			fil = fopen("s1003.dat", "wb");
			break;
		case 1004:
			fil = fopen("s1004.dat", "wb");
			break;
		case 1005:
			fil = fopen("s1005.dat", "wb");
			break;
		case 1006:
			fil = fopen("s1006.dat", "wb");
			break;
		case 1007:
			fil = fopen("s1007.dat", "wb");
			break;
		case 1008:
			fil = fopen("s1008.dat", "wb");
			break;
		case 1102:
			fil = fopen("s1102.dat", "wb");
			break;
		case 1103:
			fil = fopen("s1103.dat", "wb");
			break;
		case 1104:
			fil = fopen("s1104.dat", "wb");
			break;
		case 1105:
			fil = fopen("s1105.dat", "wb");
			break;
		case 1106:
			fil = fopen("s1106.dat", "wb");
			break;
		case 1107:
			fil = fopen("s1107.dat", "wb");
			break;
		case 1108:
			fil = fopen("s1108.dat", "wb");
			break;
		case 1109:
			fil = fopen("s1109.dat", "wb");
			break;
		case 1201:
			fil = fopen("s1201.dat", "wb");
			break;
		case 1202:
			fil = fopen("s1202.dat", "wb");
			break;
		case 1206:
			fil = fopen("s1206.dat", "wb");
			break;
		case 1207:
			fil = fopen("s1207.dat", "wb");
			break;
		case 1210:
			fil = fopen("s1210.dat", "wb");
			break;
		case 1214:
			fil = fopen("s1214.dat", "wb");
			break;
		case 1301:
			fil = fopen("s1301.dat", "wb");
			break;
		case 1302:
			fil = fopen("s1302.dat", "wb");
			break;
		case 1303:
			fil = fopen("s1303.dat", "wb");
			break;
		case 1304:
			fil = fopen("s1304.dat", "wb");
			break;
		case 1305:
			fil = fopen("s1305.dat", "wb");
			break;
		case 1306:
			fil = fopen("s1306.dat", "wb");
			break;
		case 1307:
			fil = fopen("s1307.dat", "wb");
			break;
		case 1308:
			fil = fopen("s1308.dat", "wb");
			break;
	}
	if (fil == NULL) return;

	std::vector<StarsLap>::iterator it;
	std::vector<StarsDrift>::iterator it2;
	StarsLap * tsl;
	StarsDrift * tsd;
	tsl = (StarsLap *)calloc(1, sizeof(StarsLap));
	tsd = (StarsDrift *)calloc(1, sizeof(StarsDrift));

	switch (track){
		case 1001:
			for (it = S1001.begin(); it !=S1001.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1002:
			for (it = S1002.begin(); it !=S1002.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1003:
			for (it = S1003.begin(); it !=S1003.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1004:
			for (it = S1004.begin(); it !=S1004.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1005:
			for (it = S1005.begin(); it !=S1005.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1006:
			for (it = S1006.begin(); it !=S1006.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1007:
			for (it = S1007.begin(); it !=S1007.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1008:
			for (it = S1008.begin(); it !=S1008.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1102:
			for (it = S1102.begin(); it !=S1102.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1103:
			for (it = S1103.begin(); it !=S1103.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1104:
			for (it = S1104.begin(); it != S1104.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1105:
			for (it = S1105.begin(); it != S1105.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1106:
			for (it = S1106.begin(); it != S1106.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1107:
			for (it = S1107.begin(); it != S1107.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1108:
			for (it = S1108.begin(); it != S1108.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1109:
			for (it = S1109.begin(); it != S1109.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1201:
			for (it = S1201.begin(); it != S1201.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1202:
			for (it = S1202.begin(); it != S1202.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1206:
			for (it = S1206.begin(); it != S1206.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1207:
			for (it = S1207.begin(); it != S1207.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1210:
			for (it = S1210.begin(); it != S1210.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
		case 1214:
			for (it = S1214.begin(); it != S1214.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Time = it->Time;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsLap), fil);
				}
			}
			break;
	}
	fclose(fil);
};

void ServerClass::LoadStarsDrift(int track){
	FILE * fil;

	switch (track){
		case 1301:
			fil = fopen("s1301.dat", "rb");
			break;
		case 1302:
			fil = fopen("s1302.dat", "rb");
			break;
		case 1303:
			fil = fopen("s1303.dat", "rb");
			break;
		case 1304:
			fil = fopen("s1304.dat", "rb");
			break;
		case 1305:
			fil = fopen("s1305.dat", "rb");
			break;
		case 1306:
			fil = fopen("s1306.dat", "rb");
			break;
		case 1307:
			fil = fopen("s1307.dat", "rb");
			break;
		case 1308:
			fil = fopen("s1308.dat", "rb");
			break;
	}
	if (fil == NULL) return;

	StarsDrift *tsl;

	while (!feof (fil)){
		tsl = (StarsDrift *)calloc (1, sizeof(StarsDrift));
		fread (tsl, 1, sizeof(StarsDrift), fil);

		if (tsl->Name[0] !=0) {
			switch (track) {
				case 1301:
					S1301.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
				case 1302:
					S1302.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
				case 1303:
					S1303.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
				case 1304:
					S1304.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
				case 1305:
					S1305.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
				case 1306:
					S1306.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
				case 1307:
					S1307.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
				case 1308:
					S1308.push_back(StarsDrift(tsl->Name, tsl->Points, tsl->Car, tsl->Dir));
					break;
			}
		}
	}
	fclose (fil);
};

void ServerClass::SaveStarsDrift(int track){
	FILE * fil;

	switch (track){
		case 1301:
			fil = fopen("s1301.dat", "wb");
			break;
		case 1302:
			fil = fopen("s1302.dat", "wb");
			break;
		case 1303:
			fil = fopen("s1303.dat", "wb");
			break;
		case 1304:
			fil = fopen("s1304.dat", "wb");
			break;
		case 1305:
			fil = fopen("s1305.dat", "wb");
			break;
		case 1306:
			fil = fopen("s1306.dat", "wb");
			break;
		case 1307:
			fil = fopen("s1307.dat", "wb");
			break;
		case 1308:
			fil = fopen("s1308.dat", "wb");
			break;
	}
	if (fil == NULL) return;

	std::vector<StarsDrift>::iterator it;
	StarsDrift * tsl;
	tsl = (StarsDrift *)calloc(1, sizeof(StarsDrift));

	switch (track){
		case 1301:
			for (it = S1301.begin(); it !=S1301.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
		case 1302:
			for (it = S1302.begin(); it !=S1302.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
		case 1303:
			for (it = S1303.begin(); it !=S1303.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
		case 1304:
			for (it = S1304.begin(); it !=S1304.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
		case 1305:
			for (it = S1305.begin(); it !=S1305.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
		case 1306:
			for (it = S1306.begin(); it !=S1306.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
		case 1307:
			for (it = S1307.begin(); it !=S1307.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
		case 1308:
			for (it = S1308.begin(); it != S1308.end(); it++) {
				if (it->Name[0] != 0) {
					strcpy(tsl->Name, it->Name);
					tsl->Points = it->Points;
					tsl->Car = it->Car;
					tsl->Dir = it->Dir;
					fwrite (tsl, 1, sizeof (StarsDrift), fil);
				}
			}
			break;
	}
	fclose(fil);
};

void BroadCastCommand(RUsersClass& users, char* command, char* params[], int paramcount, char* buffer) {
	RUserClass* tmp = users.First;

	while (tmp != NULL) {
		if (tmp->User->Connection != NULL) {
			tmp->User->Connection->OutgoingMessages.AddMessage(MakeMessage(buffer, command, params, paramcount));
		}
		tmp = tmp->Next;
	}
};

void BroadCastCommand(UsersClass* users, char* command, char* params[], int paramcount, char* buffer) {
	UserClass* tmp = users->First;

	while (tmp != NULL) {
		if (tmp->Connection != NULL) {
			tmp->Connection->OutgoingMessages.AddMessage(MakeMessage(buffer, command, params, paramcount));
		}
		tmp = tmp->Next;
	}
};

int atoi2(char* str) {
	int ret = 0;
	char* tmp;
	tmp = str;

	while ((tmp != NULL) && (tmp[0] >= '0') && (tmp[0] <= '9')) {
		ret = ret * 10 + tmp[0] - '0';
		tmp++;
	}
	return ret;
};

char* GetPlayerStat(const char* Name) {
	char tmp[1024];
	std::vector<PlayerStat>::iterator it;
	it = std::find(PS.begin(), PS.end(), Name);
	if (it != PS.end()) {
		sprintf(tmp, "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
			it->Rating_All, it->Wins_All, it->Loses_All, it->Disc_All, it->REP_All, it->OppsREP_All, it->OppsRating_All,
			it->Rating_Circ, it->Wins_Circ, it->Loses_Circ, it->Disc_Circ, it->REP_Circ, it->OppsREP_Circ, it->OppsRating_Circ,
			it->Rating_Sprint, it->Wins_Sprint, it->Loses_Sprint, it->Disc_Sprint, it->REP_Sprint, it->OppsREP_Sprint, it->OppsRating_Sprint,
			it->Rating_Drag, it->Wins_Drag, it->Loses_Drag, it->Disc_Drag, it->REP_Drag, it->OppsREP_Drag, it->OppsRating_Drag,
			it->Rating_Drift, it->Wins_Drift, it->Loses_Drift, it->Disc_Drift, it->REP_Drift, it->OppsREP_Drift, it->OppsRating_Drift);
		return tmp;
	}
	return NULL;
};

void GetOppREP_Rating(const char* name, const char race_type, int& rep, int& rating) {
	FILE* fil;

	fil = fopen("stat.dat", "rb");

	if (fil == NULL) return;

	PlayerStat* tps;

	while (!feof(fil)) {
		tps = (PlayerStat*)calloc(1, sizeof(PlayerStat));
		fread(tps, 1, sizeof(PlayerStat), fil);
		if (tps->Name[0] != 0) {
			if (strcmp(tps->Name, name) == 0) {
				switch (race_type) {
				case 0:
					rep = tps->REP_Circ;
					rating = tps->Rating_Circ;
					break;
				case 1:
					rep = tps->REP_Sprint;
					rating = tps->Rating_Sprint;
					break;
				case 2:
					rep = tps->REP_Drag;
					rating = tps->Rating_Drag;
					break;
				case 3:
					rep = tps->REP_Drift;
					rating = tps->Rating_Drift;
					break;
				}
				fclose(fil);
				return;
			}
		}
	}
	fclose(fil);
	return;
};

bool sort_Time(StarsLap a, StarsLap b) {
	return a.Time < b.Time;
};

bool sort_Points(StarsDrift a, StarsDrift b) {
	return a.Points > b.Points;
};

void UpdateBestTimes(const int track, const int dir, const char* name0, const int car, const int best_lap, const int best_drift) {

	std::vector<StarsLap>::iterator it;
	std::vector<StarsDrift>::iterator it2;
	int l = 1;

	switch (track) {
	case 1001:
		if (S1001.empty()) {
			S1001.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1001.begin(), S1001.end(), sort_Time);

			it = std::find(S1001.begin(), S1001.end(), name0);
			if (it != S1001.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1001.begin(); it != S1001.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1001.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}

		break;
	case 1002:
		if (S1002.empty()) {
			S1002.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1002.begin(), S1002.end(), sort_Time);

			it = std::find(S1002.begin(), S1002.end(), name0);
			if (it != S1002.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1002.begin(); it != S1002.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1002.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1003:
		if (S1003.empty()) {
			S1003.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			it = std::find(S1003.begin(), S1003.end(), name0);
			if (it != S1003.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				std::sort(S1003.begin(), S1003.end(), sort_Time);
				for (it = S1003.begin(); it != S1003.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1003.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1004:
		if (S1004.empty()) {
			S1004.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1004.begin(), S1004.end(), sort_Time);

			it = std::find(S1004.begin(), S1004.end(), name0);
			if (it != S1004.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1004.begin(); it != S1004.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1004.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1005:
		if (S1005.empty()) {
			S1005.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1005.begin(), S1005.end(), sort_Time);

			it = std::find(S1005.begin(), S1005.end(), name0);
			if (it != S1005.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1005.begin(); it != S1005.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1005.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1006:
		if (S1006.empty()) {
			S1006.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1006.begin(), S1006.end(), sort_Time);

			it = std::find(S1006.begin(), S1006.end(), name0);
			if (it != S1006.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1006.begin(); it != S1006.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1006.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1007:
		if (S1007.empty()) {
			S1007.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1007.begin(), S1007.end(), sort_Time);

			it = std::find(S1007.begin(), S1007.end(), name0);
			if (it != S1007.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1007.begin(); it != S1007.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1007.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1008:
		if (S1008.empty()) {
			S1008.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1008.begin(), S1008.end(), sort_Time);

			it = std::find(S1008.begin(), S1008.end(), name0);
			if (it != S1008.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1008.begin(); it != S1008.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1008.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1102:
		if (S1102.empty()) {
			S1102.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1102.begin(), S1102.end(), sort_Time);

			it = std::find(S1102.begin(), S1102.end(), name0);
			if (it != S1102.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1102.begin(); it != S1102.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1102.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1103:
		if (S1103.empty()) {
			S1103.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1103.begin(), S1103.end(), sort_Time);

			it = std::find(S1103.begin(), S1103.end(), name0);
			if (it != S1103.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1103.begin(); it != S1103.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1103.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1104:
		if (S1104.empty()) {
			S1104.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1104.begin(), S1104.end(), sort_Time);

			it = std::find(S1104.begin(), S1104.end(), name0);
			if (it != S1104.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1104.begin(); it != S1104.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1104.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1105:
		if (S1105.empty()) {
			S1105.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1105.begin(), S1105.end(), sort_Time);

			it = std::find(S1105.begin(), S1105.end(), name0);
			if (it != S1105.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1105.begin(); it != S1105.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1105.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1106:
		if (S1106.empty()) {
			S1106.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1106.begin(), S1106.end(), sort_Time);

			it = std::find(S1106.begin(), S1106.end(), name0);
			if (it != S1106.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1106.begin(); it != S1106.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1106.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1107:
		if (S1107.empty()) {
			S1107.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1107.begin(), S1107.end(), sort_Time);

			it = std::find(S1107.begin(), S1107.end(), name0);
			if (it != S1107.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1107.begin(); it != S1107.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1107.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1108:
		if (S1108.empty()) {
			S1108.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1108.begin(), S1108.end(), sort_Time);

			it = std::find(S1108.begin(), S1108.end(), name0);
			if (it != S1108.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1108.begin(); it != S1108.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1108.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1109:
		if (S1109.empty()) {
			S1109.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1109.begin(), S1109.end(), sort_Time);

			it = std::find(S1109.begin(), S1109.end(), name0);
			if (it != S1109.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1109.begin(); it != S1109.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1109.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1201:
		if (S1201.empty()) {
			S1201.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1201.begin(), S1201.end(), sort_Time);

			it = std::find(S1201.begin(), S1201.end(), name0);
			if (it != S1201.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1201.begin(); it != S1201.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1201.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1202:
		if (S1202.empty()) {
			S1202.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1202.begin(), S1202.end(), sort_Time);

			it = std::find(S1202.begin(), S1202.end(), name0);
			if (it != S1202.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1202.begin(); it != S1202.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1202.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1206:
		if (S1206.empty()) {
			S1206.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1206.begin(), S1206.end(), sort_Time);

			it = std::find(S1206.begin(), S1206.end(), name0);
			if (it != S1206.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1206.begin(); it != S1206.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1206.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1207:
		if (S1207.empty()) {
			S1207.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1207.begin(), S1207.end(), sort_Time);

			it = std::find(S1207.begin(), S1207.end(), name0);
			if (it != S1207.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1207.begin(); it != S1207.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1207.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1210:
		if (S1210.empty()) {
			S1210.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1210.begin(), S1210.end(), sort_Time);

			it = std::find(S1210.begin(), S1210.end(), name0);
			if (it != S1210.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1210.begin(); it != S1210.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1210.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1214:
		if (S1214.empty()) {
			S1214.push_back(StarsLap(name0, best_lap, car, dir));
		}
		else {
			std::sort(S1214.begin(), S1214.end(), sort_Time);

			it = std::find(S1214.begin(), S1214.end(), name0);
			if (it != S1214.end()) {
				// if name0 is already in the vector
				if (it->Time > best_lap) {
					it->Time = best_lap;
					it->Car = car;
					it->Dir = dir;
				}
			}
			else {
				for (it = S1214.begin(); it != S1214.end(); it++) {
					if ((it->Time > best_lap) || (l < 11)) {
						S1214.push_back(StarsLap(name0, best_lap, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1301:
		if (S1301.empty()) {
			S1301.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1301.begin(), S1301.end(), sort_Points);

			it2 = std::find(S1301.begin(), S1301.end(), name0);
			if (it2 != S1301.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1301.begin(); it2 != S1301.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1301.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1302:
		if (S1302.empty()) {
			S1302.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1302.begin(), S1302.end(), sort_Points);

			it2 = std::find(S1302.begin(), S1302.end(), name0);
			if (it2 != S1302.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1302.begin(); it2 != S1302.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1302.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1303:
		if (S1303.empty()) {
			S1303.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1303.begin(), S1303.end(), sort_Points);

			it2 = std::find(S1303.begin(), S1303.end(), name0);
			if (it2 != S1303.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1303.begin(); it2 != S1303.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1303.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1304:
		if (S1304.empty()) {
			S1304.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1304.begin(), S1304.end(), sort_Points);

			it2 = std::find(S1304.begin(), S1304.end(), name0);
			if (it2 != S1304.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1304.begin(); it2 != S1304.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1304.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1305:
		if (S1305.empty()) {
			S1305.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1305.begin(), S1305.end(), sort_Points);

			it2 = std::find(S1305.begin(), S1305.end(), name0);
			if (it2 != S1305.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1305.begin(); it2 != S1305.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1305.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1306:
		if (S1306.empty()) {
			S1306.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1306.begin(), S1306.end(), sort_Points);

			it2 = std::find(S1306.begin(), S1306.end(), name0);
			if (it2 != S1306.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1306.begin(); it2 != S1306.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1306.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1307:
		if (S1307.empty()) {
			S1307.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1307.begin(), S1307.end(), sort_Points);

			it2 = std::find(S1307.begin(), S1307.end(), name0);
			if (it2 != S1307.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1307.begin(); it2 != S1307.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1307.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;
	case 1308:
		if (S1308.empty()) {
			S1308.push_back(StarsDrift(name0, best_drift, car, dir));
		}
		else {
			std::sort(S1308.begin(), S1308.end(), sort_Points);

			it2 = std::find(S1308.begin(), S1308.end(), name0);
			if (it2 != S1308.end()) {
				// if name0 is already in the vector
				if (it2->Points < best_drift) {
					it2->Points = best_drift;
					it2->Car = car;
					it2->Dir = dir;
				}
			}
			else {
				for (it2 = S1308.begin(); it2 != S1308.end(); it++) {
					if ((it2->Points < best_drift) || (l < 11)) {
						S1308.push_back(StarsDrift(name0, best_drift, car, dir));
						break;
					}
					l++;
					if (l > 10) break;
				}
			}
		}
		break;

	}
	if (track < 1301) {
		Server.SaveStars(track);
	}
	else {
		Server.SaveStarsDrift(track);
	}
};
