#include "common.h"

void addrinfo_init(const char * IP, const char * PORT, struct addrinfo * hints, struct addrinfo ** res)
{
	memset(hints, 0, sizeof(struct addrinfo));
	hints->ai_family = AF_INET;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_flags = AI_PASSIVE;
	assert(!getaddrinfo(IP, PORT, hints, res));
}

char* name_to_IP(const char * name)
{
	struct hostent* hostname = gethostbyname(name);
	assert(hostname);

	struct in_addr **addr_list = (struct in_addr **) hostname->h_addr_list;

	// printf("%s\n", inet_ntoa(*addr_list[0]));

	return inet_ntoa(*addr_list[0]);
}

void get_users(std::vector<struct name_psswd>& n_p)
{
	std::ifstream client_list("./Users/clients.txt", std::ifstream::in);
	assert(client_list);

	for(int i = 0; !client_list.eof(); ++i)
	{
		std::string user, pas;

		client_list >> user;
		client_list >> pas;

		struct name_psswd u_p{user, pas};
		n_p.push_back(u_p);
		// std::cout << "Username: " << n_p[i].user_name << " Password: "<< n_p[i].password << std::endl;
	}
	n_p.erase(n_p.end());
	client_list.close();
	
}

bool user_exists(std::vector<struct name_psswd> n_p, bool * pass_exists, struct message user_pass)
{
	for(int i = 0; i < n_p.size(); ++i)
		if(strcmp(n_p[i].user_name.c_str(), (const char*)user_pass.source) == 0)
		{
			if(pass_exists != NULL && strcmp(n_p[i].password.c_str() , (const char*)user_pass.data) == 0)
				*pass_exists = true;

			return true;
		}

	return false;
}

