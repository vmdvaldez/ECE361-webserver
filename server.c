#include "common.h"

int main(int argc, char** argv)
{

	struct addrinfo hints, *res;

	addrinfo_init(NULL, argv[1], &hints, &res);

	int sckt, ret;
	if((sckt = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		perror("Socket Error");
	assert(sckt >= 0);

	if((ret = bind(sckt, res->ai_addr, res->ai_addrlen)) == -1)
		perror("Binding Error");
	assert(ret >= 0);

	if((ret = listen(sckt, BACKLOG)) == -1)
		perror("Listen Error");
	assert(ret >= 0);

	struct sockaddr_storage client_addr;
	socklen_t addr_size;

	if((ret = accept(sckt, (struct sockaddr*) &client_addr, &addr_size)) == -1)
		perror("Accept Error");
	assert(ret >= 0);

	int new_fd = ret;

	struct message msg;
	recv(new_fd, &msg, sizeof(msg), 0);

	struct name_psswd name_pass[100] = {0};

	get_users(name_pass);
	
	
	if(msg.signup)
	{
		if(user_exists(name_pass, NULL, msg)){
			printf("User already exists\n");
			// SEND NACK
			return 0;
		}

		FILE * client_list = fopen("./Users/clients.txt", "ab+");
		fprintf(client_list, msg.source, 0);
		fprintf(client_list, " ", 0);
		fprintf(client_list, msg.data, 0);
		fprintf(client_list, "\n", 0);

		printf("Signup Successful\n");
	}
	else
	{
		bool pass_exists = false;
		if(!user_exists(name_pass, &pass_exists, msg)){
			printf("Need To Sign Up\n");
			// SEND NACK
			return 0;
		}

		if(!pass_exists)
		{
			printf("Wrong Password\n");
			// SEND NACK
			return 0;
		}

		printf("Login Successful\n");


	}

}