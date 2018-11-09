#include "common.h"

int main(int argc, char** argv)
{


	// Create Array of Users based on registration list.
	std::vector <struct name_psswd> name_pass;
	get_users(name_pass);
	
	// Connection establishment
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


	// Client msg
	int new_fd = ret;
	struct message msg;
	recv(new_fd, &msg, sizeof(msg), 0);


	// Checks if user and pass status
	if(msg.signup)
	{
		if(user_exists(name_pass, NULL, msg)){
			printf("User already exists\n");
			// SEND NACK
			return 0;
		}

		std::ofstream client_list("./Users/clients.txt", std::ofstream::app);
		client_list << msg.source << " " << msg.data << std::endl;

		client_list.close();
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

	close(sckt);

}