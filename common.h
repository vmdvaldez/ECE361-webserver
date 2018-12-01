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
#include <unordered_map>
#include <utility>
#include <thread>
#include <algorithm>
#include <chrono>
#include <ratio>


#define MAX_NAME 50
#define MAX_DATA 1000
#define BACKLOG 20
#define TIMEOUT 30

#define time std::chrono::high_resolution_clock::time_point 


struct message {
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char destination[MAX_NAME];
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
	c_NS_NACK,
	c_MESSAGE,
	c_QUERY,
	c_QUACK,
	c_QUIT,
	c_QUIT_ACK,
	c_LOGOUT,
	c_LOGOUT_ACK,
	c_PM,
	c_PM_ACK,
	c_PM_NACK
};

struct name_psswd{
	std::string user_name;
	std::string password;
};

struct user_socket{
	std::string user;
	int socket;

	time last_active;
	time  timer;

	std::chrono::duration<double> diff()
	{
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(timer - last_active);
		return time_span;
	}
};

struct hash_elem
{
	std::string session_ID = " ";
	std::vector <struct user_socket> u_s;
	bool deleted = false;
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
void create_msg(struct message& msg, int type, int size, std::string source, std::string dest, std::string data);

