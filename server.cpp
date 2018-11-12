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

	int main_sckt = connection_establishment(hints, res, 0);

	int max_sd = main_sckt;
	fd_set master_fds, read_fds;
	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);
	FD_SET(main_sckt, &master_fds);

	while(1)
	{
		read_fds = master_fds;
		select(max_sd + 1, &read_fds, NULL, NULL, NULL);

		// printf("estsesetse\n");

		if(FD_ISSET(main_sckt, &read_fds))
		{
			int ret;
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
						gen_ACK(msg, c_LO_ACK, "User Already Exists");
					else
					{
						add_user((char*)msg.source, (char*)msg.data);
						struct name_psswd n_p{(char*)msg.source, (char*)msg.data};
						name_pass.push_back(n_p);
						gen_ACK(msg, c_LO_ACK, "Signup Successful");
					}
					send(client_sockets[i], &msg, sizeof(msg), 0);
				}
				else
				{
					bool pass_exists = false;
					if(!user_exists(name_pass, &pass_exists, msg))
						gen_ACK(msg, c_LO_NACK, "Need To Sign UP");
					else
					{
						if(!pass_exists)
							gen_ACK(msg, c_LO_NACK, "Wrong Password");
						else
							gen_ACK(msg, c_LO_ACK, "Login Successful");

					}

					send(client_sockets[i], &msg, sizeof(msg), 0);
				}
			
			else if(msg.type == c_NEW_SESS)
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
					

					gen_ACK(msg, c_NS_ACK, session_ID);
					send(client_sockets[i], &msg, sizeof(msg), 0);
					// std::cout << (sessions[0])[0].user << (sessions[0])[0].socket <<std::endl;

					// CHECK IF IT WORKS
				}
			}
				

			else if(msg.type == c_JOIN)
			{
				std::string session_ID((char*) msg.data);
				std::string source((char *)msg.source);

				auto search = hash_sesh.find(session_ID);
				if(search == hash_sesh.end())
				{
					std::string temp = session_ID + "\nReason: Session Does Not Exist.";
					gen_ACK(msg, c_JN_NACK, temp);
					send(client_sockets[i], &msg, sizeof(msg), 0);
				}
				else
				{
					int index = hash_sesh[session_ID];
					struct user_socket temp = {source, client_sockets[i]};

					std::vector<user_socket>& u_s = sessions[index];
					u_s.push_back(temp);

					gen_ACK(msg, c_JN_ACK, session_ID);
					send(client_sockets[i], &msg, sizeof(msg), 0);
				}
			}

			else if(msg.type == c_MESSAGE)
			{
				std::string source((char*)msg.source);
				std::vector<user_socket> u_s;
				// FIX 
				for(int j = 0 ; i < sessions.size(); ++i)
					for(auto x : sessions[i])
						if(x.user == source)
							u_s = sessions[i];


				for(auto x : u_s)
					if(x.user != source){
						std::cout << x.socket << x.user;
						send(x.socket, &msg, sizeof(msg), 0);
					}
			}
			// for(i = 0 ; i < sessions.size(); ++i)
			// 	for(auto j : sessions[i])
			// 		std::cout << j.user << std::endl;

		} 




	}

	close(main_sckt);

}


//CLOSE SOCKET PROPERLY