#include "common.h"

//Hashing
unsigned long djb2_hash(char* sess_name);
struct hash_elem * hash_lookup(unsigned long h_index , std::string session_ID);
void hash_insert(unsigned long h_index, std::string user, int socket, std::string session_ID);

//Debugging
void print_sess_users();

// hashtable; collection of users and their socket associated with the hashed session ID.
std::unordered_map<unsigned long, struct hash_elem> hash_sesh;

// Sockets of online users; (change to struct user_socket)?? + session they're in
std::vector <int> client_sockets;

//Create Array of Users based on registration list (from file).
std::vector <struct name_psswd> name_pass;


int main(int argc, char** argv)
{

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
						gen_ACK(msg, c_LO_NACK, "User Already Exists");
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

				unsigned long h_index = djb2_hash((char *)session_ID.c_str());

				struct hash_elem * h_elem = hash_lookup(h_index, session_ID);

				if(h_elem != NULL)
				{
					std::cout << "hit" << std::endl;

					gen_ACK(msg, c_NS_NACK, session_ID);
					send(client_sockets[i], &msg, sizeof(msg), 0);
				}
				else
				{

					std::cout << "miss" << std::endl;
					hash_insert(h_index, source , client_sockets[i],session_ID);

					gen_ACK(msg, c_NS_ACK, session_ID);
					send(client_sockets[i], &msg, sizeof(msg), 0);

				}
			}
				

			else if(msg.type == c_JOIN)
			{
				std::string session_ID((char*) msg.data);
				std::string source((char *)msg.source);

				unsigned long h_index = djb2_hash((char *)session_ID.c_str());
				struct hash_elem * h_elem = hash_lookup(h_index, session_ID);

				if(h_elem == NULL)
				{
					std::string nack = session_ID + "\nReason: Session Does Not Exist.";
					gen_ACK(msg, c_JN_NACK, nack);
					send(client_sockets[i], &msg, sizeof(msg), 0);
				}
				else
				{
					bool not_found = true;

					for (auto x : h_elem->u_s)
						if(x.socket == client_sockets[i])
						{
							std::string nack = session_ID + "\nReason: Already A Part of The Session";
							gen_ACK(msg, c_JN_NACK, nack);
							send(client_sockets[i], &msg, sizeof(msg), 0);
							not_found = false;
							break;
						}

					if(not_found)
					{
						struct user_socket temp = {source, client_sockets[i]};

						h_elem->u_s.push_back(temp);

						gen_ACK(msg, c_JN_ACK, session_ID);
						send(client_sockets[i], &msg, sizeof(msg), 0);
					}

				}
			}

			else if(msg.type == c_LEAVE_SESS)
			{

				std::string source((char*)msg.source);

				int client_sckt;
				std::string session_ID;
				struct user_socket u_s;
				std::unordered_map<unsigned long, struct hash_elem> hashptr;


				for(auto x : hash_sesh)
					for(auto y : x.second.u_s)
						if(y.user == source){
							client_sckt = y.socket;
							session_ID = x.second.session_ID;
							u_s = y;
						}
						
				unsigned long h_index = djb2_hash((char *)session_ID.c_str());
				struct hash_elem * h_elem = hash_lookup(h_index, session_ID);

				if(h_elem == NULL)
				{
					//SEND ERROR

				}
				else
				{
					std::vector<user_socket>::iterator it;
					for(it = h_elem->u_s.begin(); it != h_elem->u_s.end(); ++it)
						if(it->socket == client_sckt)
							break;

					// std::cout << h_elem->u_s.size() << std::endl;

					h_elem->u_s.erase(it);

					
					if(h_elem->u_s.size() == 0)
						hash_sesh.erase(h_index);


					// std::cout << h_elem->u_s.size() << std::endl;	


				}



			}

			else if(msg.type == c_MESSAGE)
			{

				std::string source((char*)msg.source);
				std::string data((char*) msg.data);
				// std::vector<user_socket> u_s;
				
				int client_sckt;
				std::string session_ID;

				for(auto x : hash_sesh)
					for(auto y : x.second.u_s)
						if(y.user == source)
						{
							client_sckt = y.socket;
							session_ID = x.second.session_ID;
						}

				unsigned long h_index = djb2_hash((char *)session_ID.c_str());
				struct hash_elem * h_elem = hash_lookup(h_index, session_ID);


				for(auto k : h_elem->u_s)
					if(k.socket != client_sckt)
						send(k.socket, &msg, sizeof(msg), 0);
			}

			else if(msg.type == c_QUERY)
			{
				std::string source((char *)msg.source);
				std::string data((char*) msg.data);

				for(auto x : hash_sesh)
				{
					std:: cout << std::endl <<"Session: " <<x.second.session_ID << std::endl;
					for(auto y: x.second.u_s)
					std::cout << std::endl << "User:" <<y.user <<std::endl;
				}

				std::string list;
				int size = 0;

				for(auto x : hash_sesh)
				{
					size += 9 + x.second.session_ID.length() + 1;
					list += "Session: " + x.second.session_ID + "\n";

					for(auto y : x.second.u_s)
					{
						if(size + y.user.length() + 1 >= MAX_DATA)
						{
							list += "\0";
							create_msg(msg, c_QUERY, list.length() + 1, source, list);
							send(client_sockets[i], &msg, sizeof(msg), 0);
							size = 0;
							list.clear();
						}


						size += y.user.length() + 1;
						list += y.user + "\n";

					}
					create_msg(msg, c_QUERY, list.length() + 1, source, list);
					send(client_sockets[i], &msg, sizeof(msg), 0);

					list.clear();
				}

				gen_ACK(msg, c_QUACK, "");
				send(client_sockets[i], &msg, sizeof(msg), 0);

			}

			else if(msg.type == c_QUIT)
			{
				std::cout << "teste"<< client_sockets[i] <<std::endl;

				// std::cout << "bBEFORE" <<std::endl;
				// for (auto x : client_sockets)
				// 	std::cout << x <<std::endl;

				std::vector<int>::iterator it;
				for(it = client_sockets.begin(); it != client_sockets.end(); ++it)
					if(*it == client_sockets[i])
						break;

				int client_sckt = *it;

				client_sockets.erase(it);


				gen_ACK(msg, c_QUIT, " ");
				send(client_sckt, &msg, sizeof(msg), 0);

				// std::cout << "AFTER" <<std::endl;
				// for (auto x : client_sockets)
				// 	std::cout << x <<std::endl;

			}

		} 




	}

	close(main_sckt);

}




unsigned long djb2_hash(char* sess_name){
	unsigned long hash = 5381;

	int c;

	while(c = *sess_name++)
		hash = hash * 33 + c;

	// std::cout << hash << std::endl;
	return hash;
}

struct hash_elem * hash_lookup(unsigned long h_index , std::string session_ID)
{
	while(hash_sesh[h_index].session_ID != " " || hash_sesh[h_index].deleted)
	{
		if(hash_sesh[h_index].deleted)
			h_index ++;
		else
		{
			if(hash_sesh[h_index].session_ID == session_ID)
				return &hash_sesh[h_index];

			h_index++;
		}
	}
	return NULL;
}

void hash_insert(unsigned long h_index, std::string user, int socket, std::string session_ID)
{
	while(hash_sesh[h_index].session_ID != " ")
		h_index ++;

	std::vector<struct user_socket> v_us;
	struct user_socket u_s{user, socket};
	struct hash_elem h_e;

	v_us.emplace_back(u_s);

	h_e.session_ID = session_ID;
	h_e.u_s = v_us;
	h_e.deleted = false;

	hash_sesh[h_index] = h_e;

}



void print_sess_users()
{
	for(auto x : hash_sesh){
		std:: cout << std::endl <<"Session: " <<x.second.session_ID << std::endl;
		for(auto y: x.second.u_s)
			std::cout << std::endl << "User:" <<y.user <<std::endl << "Socket: "<<y.socket <<std::endl;
	}
}

//CLOSE SOCKET PROPERLY
//FIX HASHTABLE