#include "common.h"

int main(int argc, char** argv)
{


	// Create Array of Users based on registration list.
	std::vector <struct name_psswd> name_pass;
	std::vector <int> client_sockets;

	std::map<std::string, int> hash_sesh;
	std::vector <std::vector <struct user_socket>> sessions;
	get_users(name_pass);
	
	// Connection establishment
	struct addrinfo hints, *res;
	addrinfo_init(NULL, argv[1], &hints, &res);

	int main_sckt, ret;
	if((main_sckt = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		perror("Socket Error");
	assert(main_sckt >= 0);

	if((ret = bind(main_sckt, res->ai_addr, res->ai_addrlen)) == -1)
		perror("Binding Error");
	assert(ret >= 0);

	if((ret = listen(main_sckt, BACKLOG)) == -1)
		perror("Listen Error");
	assert(ret >= 0);


	int max_sd = main_sckt;
	fd_set master_fds, read_fds, write_fds;
	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);
	FD_SET(main_sckt, &master_fds);

	while(1)
	{
		read_fds = master_fds;
		write_fds = master_fds;
		select(max_sd + 1, &read_fds, &write_fds, NULL, NULL);

		// printf("estsesetse\n");

		if(FD_ISSET(main_sckt, &read_fds))
		{
			struct sockaddr_storage client_addr;
			socklen_t addr_size;

			if((ret = accept(main_sckt, (struct sockaddr*) &client_addr, &addr_size)) == -1)
				perror("Accept Error");
			assert(ret >= 0);


			client_sockets.push_back(ret);
			FD_SET(ret, &master_fds);
			if(max_sd < ret)
				max_sd = ret;
		}

		for(int i = 0; i < client_sockets.size(); ++i)
		{
			// printf("%d\n", FD_ISSET(client_sockets[i], &read_fds));
			if(!FD_ISSET(client_sockets[i], &read_fds))
				continue;

			struct message msg;

			recv(client_sockets[i], &msg, sizeof(msg), 0);

			std::string data;

			if(msg.type == c_LOGIN)
				if(msg.signup)
				{
					if(user_exists(name_pass, NULL, msg))
					{
					// std::cout << "User already exists" << std::endl;
				
					data = "User already exists";
					msg.type = c_LO_NACK;
					msg.size = data.length() + 1;
					memcpy(&msg.data, data.c_str(), data.length() + 1);
					send(client_sockets[i], &msg, sizeof(msg), 0);
					}
					else
					{
						std::ofstream client_list("./Users/clients.txt", std::ofstream::app);
						client_list << msg.source << " " << msg.data << std::endl;

						client_list.close();
						// std::cout << "Signup Successful" << std::endl;
						msg.type = c_LO_ACK;
						data = "Signup Successful";
						msg.size = data.length() + 1;
						memcpy(&msg.data, data.c_str(), data.length() + 1);
						send(client_sockets[i], &msg, sizeof(msg), 0);
					}
				}
				else
				{
					bool pass_exists = false;
					if(!user_exists(name_pass, &pass_exists, msg)){
					// 	// std::cout << "Need To Sign Up" << std::endl;

						msg.type = c_LO_NACK;
						printf("test\n");
						data = "Need To Sign Up";
						msg.size = data.length() + 1;
						// std::cout << data << std::endl;
						memcpy(&msg.data, data.c_str(), data.length() + 1);
						send(client_sockets[i], &msg, sizeof(msg), 0);
					}
					else
					{
						if(!pass_exists)
						{
							// std::cout << "Wrong Password" << std::endl;
							msg.type = c_LO_NACK;
							data = "Wrong Password";
							msg.size = data.length() + 1;
							memcpy(&msg.data, data.c_str(), data.length() + 1);
							send(client_sockets[i], &msg, sizeof(msg), 0);
						}
						else
						{
							msg.type = c_LO_ACK;
							// std::cout << "Login Successful" << std::endl;
							data = "Login Successful";
							msg.size = data.length() + 1;
							memcpy(&msg.data, data.c_str(), data.length() + 1);
							send(client_sockets[i], &msg, sizeof(msg), 0);
						}	
					}
				}
			

			if(msg.type == c_NEW_SESS)
			{
				std::string session_ID((char*) msg.data);
				 std::string source((char *)msg.source);

				auto search = hash_sesh.find(session_ID);
				if(search != hash_sesh.end())
				{
					//optional
					//SEND NACK
				}
				else
				{
					hash_sesh.insert(hash_sesh.begin(), std::pair<std::string, int>(session_ID, sessions.size()));

					// std::cout << hash_sesh[session_ID] << std::endl;

					std::vector<user_socket> u_s;
					struct user_socket temp = {source, client_sockets[i]};

					// std::cout << source << std::endl  << session_ID << std::endl << client_sockets[i] << std::endl;
					u_s.push_back(temp);
					sessions.push_back(u_s);
							
					msg.type = c_NS_ACK;
							// std::cout << "Login Successful" << std::endl;
					data = "Created New Session";
					msg.size = data.length() + 1;
					memcpy(&msg.data, data.c_str(), data.length() + 1);
					send(client_sockets[i], &msg, sizeof(msg), 0);
					// std::cout << (sessions[0])[0].user << (sessions[0])[0].socket <<std::endl;

					// CHECK IF IT WORKS

				}
			}
				
		}
	}

	close(main_sckt);

}


//CLOSE SOCKET PROPERLY