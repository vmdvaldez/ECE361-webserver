#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>



#define MAX_NAME 50
#define MAX_DATA 1000
#define BACKLOG 20


struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];

	bool signup = false;
};

enum control{
	c_LOGIN = 0,
	c_LO_ACK,
	c_LO_NACK,
	c_EXIT,
	c_JOIN,
	c_JN_ACK,
	c_JN_NACK,
	c_LEAVE_SESS,
	c_NEW_SESS,
	c_NS_ACK,
	c_MESSAGE,
	c_QUERY,
	c_QU_ACK
};

struct name_psswd{
	std::string user_name;
	std::string password;
};

struct user_socket{
	std::string user;
	int socket;
};

//Connection Establishment Functions
int connection_establishment(struct addrinfo& hints, struct addrinfo * &res, int type);
void addrinfo_init(const char * IP, const char * PORT, struct addrinfo * hints, struct addrinfo ** res);
char* name_to_IP(const char * name);

//Registration Lists
void get_users(std::vector<struct name_psswd>& n_p);
bool user_exists(std::vector<struct name_psswd> n_p, bool * pass_exists, struct message user_pass);
void add_user(std::string name, std::string password);

//Create Message
void gen_ACK(struct message &msg, int type, std::string data);
void create_msg(struct message& msg, int type, int size, std::string source, std::string data);

;