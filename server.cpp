#include "common.h"

//Hashing
unsigned long djb2_hash(char* sess_name);
unsigned long hash_lookup(std::string session_ID);
void hash_insert(unsigned long h_index, std::string user, int socket, std::string session_ID);

//Request Handling
void f_login(struct message msg, bool signup, int socket);
void f_newsess(struct message msg, int socket);
void f_joinsess(struct message msg, int socket);
void f_leavesess(struct message msg, int socket);
void f_message(struct message msg, int socket);
void f_query(struct message msg, int socket);
void f_quit(struct message msg, int socket);
void f_logout(struct message msg, int socket);
void f_pm(struct message msg, int socket);
//Debugging
void print_sess_users();
void print_us();

// hashtable; collection of users and their socket associated with the hashed session ID.
std::unordered_map<unsigned long, struct hash_elem> hash_sesh;
// reverse hashing
std::unordered_map<std::string, unsigned long> rev_hash_sesh;

// Sockets of online users; (change to struct user_socket)?? + session they're in
std::vector <struct user_socket> client_sockets;

//Create Array of Users based on registration list (from file).
std::vector <struct name_psswd> name_pass;


int main(int argc, char** argv)
{

	std::chrono::duration<double> timeout(TIMEOUT);

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

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	// setsockopt(sckt, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);


	while(1)
	{
		read_fds = master_fds;
		select(max_sd + 1, &read_fds, NULL, NULL, &tv);



		if(FD_ISSET(main_sckt, &read_fds))
		{
			int ret;
			struct sockaddr_storage client_addr;
			socklen_t addr_size;

			if((ret = accept(main_sckt, (struct sockaddr*) &client_addr, &addr_size)) == -1)
				perror("Accept Error");
			assert(ret >= 0);

			auto join_time = std::chrono::high_resolution_clock::now();

			client_sockets.emplace_back(user_socket{" ", ret, join_time, join_time});
			FD_SET(ret, &master_fds);
			if(max_sd < ret)
				max_sd = ret;
		}

		for(int i = 0; i < client_sockets.size(); ++i)
		{

			if(!FD_ISSET(client_sockets[i].socket, &read_fds)){
				if(client_sockets[i].diff() >= timeout)
				{
					struct message quit;
					create_msg(quit, c_QUIT, client_sockets[i].user.length() + 1, client_sockets[i].user, "", "");
					f_quit(quit, client_sockets[i].socket);
				}
				else
					client_sockets[i].timer = std::chrono::high_resolution_clock::now();

				continue;
			}

			client_sockets[i].last_active = std::chrono::high_resolution_clock::now();

			struct message msg;
			recv(client_sockets[i].socket, &msg, sizeof(msg), 0);


			if(msg.type == c_LOGIN)
				f_login(msg ,msg.signup, client_sockets[i].socket);			
			else if(msg.type == c_NEW_SESS)
				f_newsess(msg, client_sockets[i].socket);
			else if(msg.type == c_JOIN)
				f_joinsess(msg, client_sockets[i].socket);
			else if(msg.type == c_LEAVE_SESS)
				f_leavesess(msg, client_sockets[i].socket);
			else if(msg.type == c_MESSAGE)
				f_message(msg, client_sockets[i].socket);
			else if(msg.type == c_QUERY)
				f_query(msg, client_sockets[i].socket);
			else if(msg.type == c_LOGOUT)
				f_logout(msg, client_sockets[i].socket);
			else if(msg.type == c_QUIT)
				f_quit(msg, client_sockets[i].socket);
			else if(msg.type == c_PM)
				f_pm(msg, client_sockets[i].socket);
		} 
	}

	close(main_sckt);

}




unsigned long djb2_hash(char* sess_name){
	unsigned long hash = 5381;

	int c;

	while(c = *sess_name++)
		hash = hash * 33 + c;

	return hash;
}

unsigned long hash_lookup(std::string session_ID){ return rev_hash_sesh[session_ID];}

void hash_insert(unsigned long h_index, std::string user, int socket, std::string session_ID)
{
	while(hash_sesh[h_index].session_ID != " ")
		h_index++;

	std::vector<struct user_socket> v_us;
	struct user_socket u_s{user, socket};
	struct hash_elem h_e;

	v_us.emplace_back(u_s);

	h_e.session_ID = session_ID;
	h_e.u_s = v_us;
	h_e.deleted = false;

	hash_sesh[h_index] = h_e;
	rev_hash_sesh[session_ID] = h_index;

}


void print_us()
{
	for(auto x : client_sockets)
		std::cout << x.user << " " << x.socket << std::endl;
}

void print_sess_users()
{
	for(auto x : hash_sesh){
		std:: cout << std::endl <<"Session: " <<x.second.session_ID << std::endl;
		for(auto y: x.second.u_s)
			std::cout << std::endl << "User:" <<y.user <<std::endl << "Socket: "<<y.socket <<std::endl;
	}
}

void f_login(struct message msg, bool signup, int socket)
{
	if(msg.signup)
	{
		if(user_exists(name_pass, NULL, msg))
		{
			gen_ACK(msg, c_LO_NACK, "User Already Exists");
			send(socket, &msg, sizeof(msg), 0);
			return;
		}

		add_user((char*)msg.source, (char*)msg.data);
		struct name_psswd n_p{(char*)msg.source, (char*)msg.data};
		name_pass.push_back(n_p);
		gen_ACK(msg, c_LO_ACK, "Signup Successful");
		send(socket, &msg, sizeof(msg), 0);
	
		return;	
	}

	bool pass_exists = false;
	if(!user_exists(name_pass, &pass_exists, msg))
	{
		gen_ACK(msg, c_LO_NACK, "Need To Sign UP");
		send(socket, &msg, sizeof(msg), 0);
		return;
	}

	if(!pass_exists)
	{
		gen_ACK(msg, c_LO_NACK, "Wrong Password");
		send(socket, &msg, sizeof(msg), 0);
		return;
	}

	std::string user((char *)msg.source);

	for(auto x : client_sockets)
		if(x.user == user)
		{
			gen_ACK(msg, c_LO_NACK, "User is Already Online");
			send(socket, &msg, sizeof(msg), 0);
			return;
		}	


	for(auto &x : client_sockets)
		if(x.socket == socket)
			x.user = user;

	gen_ACK(msg, c_LO_ACK, "Login Successful");
	send(socket, &msg, sizeof(msg), 0);
}

void f_newsess(struct message msg, int socket)
{
	std::string session_ID((char*) msg.data);
	std::string source((char *)msg.source);
	unsigned long found_index = hash_lookup(session_ID);

	if(found_index != 0)
	{
		gen_ACK(msg, c_NS_NACK, session_ID);
		send(socket, &msg, sizeof(msg), 0);
		return;
	}
	
	unsigned long h_index = djb2_hash((char *)session_ID.c_str());	
	hash_insert(h_index, source , socket, session_ID);

	gen_ACK(msg, c_NS_ACK, session_ID);
	send(socket, &msg, sizeof(msg), 0);

	
}

void f_joinsess(struct message msg, int socket)
{
	std::string session_ID((char*) msg.data);
	std::string source((char *)msg.source);

	unsigned long found_index = hash_lookup(session_ID);

	if(found_index == 0)
	{
		std::string nack = session_ID + "\nReason: Session Does Not Exist.";
		gen_ACK(msg, c_JN_NACK, nack);
		send(socket, &msg, sizeof(msg), 0);
		return;
	}

	bool not_found = true;

	struct hash_elem * h_elem = &hash_sesh[found_index];

	for (auto x : h_elem->u_s)
		if(x.socket == socket)
		{
			std::string nack = session_ID + "\nReason: Already A Part of The Session";
			gen_ACK(msg, c_JN_NACK, nack);
			send(socket, &msg, sizeof(msg), 0);
			not_found = false;
			break;
		}

	if(not_found)
	{
		struct user_socket temp = {source, socket};

		h_elem->u_s.push_back(temp);

		gen_ACK(msg, c_JN_ACK, session_ID);
		send(socket, &msg, sizeof(msg), 0);
	}

	
}

void f_leavesess(struct message msg, int socket)
{

	std::string source((char*)msg.source);

	// int client_sckt;
	std::string session_ID;
	struct user_socket u_s;
	std::unordered_map<unsigned long, struct hash_elem> hashptr;


	for(auto x : hash_sesh)
		for(auto y : x.second.u_s)
			if(y.user == source){
				session_ID = x.second.session_ID;
				u_s = y;
			}
			
	unsigned long found_index = hash_lookup(session_ID);

	if(found_index == 0)
	{
		//SEND ERROR

		return;

	}

	struct hash_elem * h_elem = &hash_sesh[found_index];

	std::vector<struct user_socket>::iterator it;
	for(it = h_elem->u_s.begin(); it != h_elem->u_s.end(); ++it)
		if(it->socket == socket)
			break;

	h_elem->u_s.erase(it);

	
	if(h_elem->u_s.size() == 0)
	{
		hash_sesh.erase(found_index);
		rev_hash_sesh.erase(session_ID);
	}


}

void f_message(struct message msg, int socket)
{

	std::string source((char*)msg.source);
	std::string data((char*) msg.data);
	std::string session_ID;

	for(auto x : hash_sesh)
		for(auto y : x.second.u_s)
			if(y.user == source)
				session_ID = x.second.session_ID;


	unsigned long found_index = hash_lookup(session_ID);

	if(found_index == 0)
		return;

	struct hash_elem * h_elem = &hash_sesh[found_index];

	for(auto k : h_elem->u_s)
		if(k.socket != socket)
			send(k.socket, &msg, sizeof(msg), 0);
}

void f_query(struct message msg, int socket)
{
	std::string source((char *)msg.source);
	std::string data((char*) msg.data);

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
				create_msg(msg, c_QUERY, list.length() + 1, source, "", list);
				send(socket, &msg, sizeof(msg), 0);
				size = 0;
				list.clear();
			}


			size += y.user.length() + 1;
			list += y.user + "\n";

		}
		create_msg(msg, c_QUERY, list.length() + 1, source, "", list);
		send(socket, &msg, sizeof(msg), 0);
		size = 0;
		list.clear();
	}

	gen_ACK(msg, c_QUACK, "");
	send(socket, &msg, sizeof(msg), 0);

}

void f_quit(struct message msg, int socket)
{
	std::string source((char*)msg.source);

	int client_sckt;
	std::vector<std::string> session_ID;


	for(auto x : hash_sesh)
		for(auto y : x.second.u_s)
			if(y.user == source){
				client_sckt = y.socket;
				session_ID.push_back(x.second.session_ID);
			}
			
	for(int i = 0; i < session_ID.size(); ++i){		
		unsigned long found_index = hash_lookup(session_ID[i]);


		struct hash_elem * h_elem = &hash_sesh[found_index];

		std::vector<user_socket>::iterator it;
		for(it = h_elem->u_s.begin(); it != h_elem->u_s.end(); ++it)
			if(it->socket == client_sckt)
				break;

		h_elem->u_s.erase(it);

		
		if(h_elem->u_s.size() == 0)
		{
			hash_sesh.erase(found_index);
			rev_hash_sesh.erase(session_ID[i]);
		}

	}

	std::vector<struct user_socket>::iterator it;
	for(it = client_sockets.begin(); it != client_sockets.end(); ++it)
		if(it->socket == socket)
			break;

	client_sockets.erase(it);

	gen_ACK(msg, c_QUIT, " ");
	send(socket, &msg, sizeof(msg), 0);

}

void f_logout(struct message msg, int socket)
{
	std::string source((char*)msg.source);

	int client_sckt;
	std::vector<std::string> session_ID;


	for(auto x : hash_sesh)
		for(auto y : x.second.u_s)
			if(y.user == source){
				client_sckt = y.socket;
				session_ID.push_back(x.second.session_ID);
			}
			
	for(int i = 0; i < session_ID.size(); ++i){		
		unsigned long found_index = hash_lookup(session_ID[i]);


		struct hash_elem * h_elem = &hash_sesh[found_index];

		std::vector<user_socket>::iterator it;
		for(it = h_elem->u_s.begin(); it != h_elem->u_s.end(); ++it)
			if(it->socket == client_sckt)
				break;

		h_elem->u_s.erase(it);

		
		if(h_elem->u_s.size() == 0)
		{
			hash_sesh.erase(found_index);
			rev_hash_sesh.erase(session_ID[i]);
		}

	}

	std::vector<struct user_socket>::iterator it;
	for(it = client_sockets.begin(); it != client_sockets.end(); ++it)
		if(it->socket == socket)
			break;

	it->user = "21312321";
	// client_sockets.erase(it);


	gen_ACK(msg, c_LOGOUT_ACK, " ");
	send(socket, &msg, sizeof(msg), 0);

}

void f_pm(struct message msg, int socket)
{
	std::string source((char*)msg.source);
	std::string data((char*) msg.data);
	std::string dest((char *)msg.destination);
	bool found = false;

	for(auto x : client_sockets)
		if(x.user == dest){
			found = true;
			msg.type = c_PM_ACK;
			send(x.socket, &msg, sizeof(msg), 0);
			break;
		}

	if(!found){
		gen_ACK(msg, c_PM_NACK, "");
		send(socket, &msg, sizeof(msg), 0);
	}
		
}
//CLOSE SOCKET PROPERLY
