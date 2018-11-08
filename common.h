#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>


#define MAX_NAME 50
#define MAX_DATA 1000
#define BACKLOG 20


struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];

	bool signup;
};

enum control{
	c_LOGIN = 0,
	c_LO_ACK,
	c_LO_NACK,
	c_EXIT,
	c_JOIN,
	c_JN_ACK,
	c_JN_NAK,
	c_LEAVE_SESS,
	c_NEW_SESS,
	c_NS_ACK,
	c_MESSAGE,
	c_QUERY,
	c_QU_ACK
};

struct name_psswd{
	char user_name[20];
	char password[10];
};

void addrinfo_init(char * IP, char * PORT, struct addrinfo * hints, struct addrinfo ** res);
char* name_to_IP(char * name);
void get_users(struct name_psswd * n_p);
bool user_exists(struct name_psswd * n_p, bool * pass_exists, struct message user_pass);