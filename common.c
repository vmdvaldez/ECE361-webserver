#include "common.h"

void addrinfo_init(char * IP, char * PORT, struct addrinfo * hints, struct addrinfo ** res)
{
	memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_INET;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_flags = AI_PASSIVE;
	printf("test\n");
	assert(!getaddrinfo(IP, PORT, hints, res));
}

char* name_to_IP(char * name)
{
	struct hostent* hostname = gethostbyname(name);
	assert(hostname);

	struct in_addr **addr_list = (struct in_addr **) hostname->h_addr_list;

	// printf("%s\n", inet_ntoa(*addr_list[0]));

	return inet_ntoa(*addr_list[0]);
}

void get_users(struct name_psswd * n_p)
{
	FILE * client_list = fopen("./Users/clients.txt", "r");
	assert(client_list);

	for(int i = 0; feof(client_list) == 0; ++i)
	{
		fscanf(client_list, "%s", n_p[i].user_name);
		fscanf(client_list, "%s", n_p[i].password);
		printf("User:%s, Password:%s\n", n_p[i].user_name, n_p[i].password);

	}
	
	fclose(client_list);
}

bool user_exists(struct name_psswd * n_p, bool * pass_exists, struct message user_pass)
{
	for(int i = 0; n_p[i].user_name[0] != 0; ++i)
		if(strcmp(n_p[i].user_name, user_pass.source) == 0)
		{
			if(pass_exists != NULL && strcmp(n_p[i].password, user_pass.data) == 0)
				*pass_exists = true;

			return true;
		}

	return false;
}