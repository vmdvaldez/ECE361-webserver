#include "common.h"


int main(int argc, char** argv)
{

	char command[10], client_ID[20], psswd[10], server_IP[20], server_port[10];
	struct message msg;
	int signup;

	printf("1: SIGNUP\n2: LOGIN\n");
	scanf("%d\n", &signup);

	if(signup == 1)
		msg.signup = true;
	else
		msg.signup = false;

	scanf("%s\n",command);
	// printf("%s\n",command);

	if(strcmp(command, "/login") != 0)
	{
		printf("Login first\n");
		scanf("%s\n", command);		
	}


	scanf("%s%s%s%s", client_ID, psswd, server_IP, server_port);
	// printf("%s,%s,%s,%s\n", client_ID, psswd, server_IP, server_port);

	// printf("%d, %d ,%d\n", c_LOGIN, c_LO_ACK, c_LO_NACK);

	struct addrinfo hints, *res;

	addrinfo_init(name_to_IP(server_IP), server_port, &hints, &res);

	int sckt, ret;
	
	if((sckt = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		perror("Socket Error");
	assert(sckt >= 0);

	if((ret = connect(sckt, res->ai_addr, res->ai_addrlen)) == -1)
		perror("Connect error");
	assert(!ret);

	msg.type = c_LOGIN;
	msg.size = strlen(psswd);
	strcpy(msg.source, client_ID);
	strcpy(msg.data, psswd);

	send(sckt, &msg, sizeof(msg), 0);

}