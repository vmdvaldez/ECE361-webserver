#include "common.h"


int main(int argc, char** argv)
{


	struct message msg;
	int signup;

	//Asks User to signup or Login
	std::cout << "1: SIGNUP" <<std::endl << "2: LOGIN" << std::endl;
	std::cin >> signup;
	std::cin.ignore();

	if(signup == 1)
		msg.signup = true;

	std::string input, command, client_ID, psswd, server_IP, server_port;

	// makes sure client logs in first
	do 
	{
		std::getline(std::cin, input);
		std::stringstream ss(input);

		ss >> command >> client_ID >> psswd >> server_IP >> server_port;
		std::cout << command << std::endl;

		if(command != "/login")
		 	std::cout << "Login first" << std::endl;

				
	}while(command != "/login");


	//Connection establishment
	struct addrinfo hints, *res;

	addrinfo_init(name_to_IP(server_IP.c_str()), server_port.c_str(), &hints, &res);

	int sckt, ret;
	
	if((sckt = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		perror("Socket Error");
	assert(sckt >= 0);

	if((ret = connect(sckt, res->ai_addr, res->ai_addrlen)) == -1)
		perror("Connect error");
	assert(!ret);

	// Creates login message

	while(1)
	{
		msg.type = c_LOGIN;
		msg.size = psswd.length();
		memcpy(&msg.source, client_ID.c_str(), client_ID.length() + 1);
		memcpy(&msg.data, psswd.c_str(), psswd.length() + 1);

		send(sckt, &msg, sizeof(msg), 0);

		recv(sckt, &msg, sizeof(msg), 0);

		if(msg.type == c_LO_ACK){
			std::cout << msg.data << std::endl;
			break;
		}
		else if(msg.type == c_LO_NACK)
		{
			do 
			{
				std::getline(std::cin, input);
				std::stringstream ss(input);

				ss >> command >> client_ID >> psswd >> server_IP >> server_port;
				std::cout << command << std::endl;

				if(command != "/login")
				 	std::cout << "Login first" << std::endl;

						
			}while(command != "/login");
		}
	}


	while(1)
	{
		struct message new_msg;
		std::getline(std::cin, input);
		std::stringstream ss(input);

		ss >> command;

		if(command == "/createsession")
		{
			std::string sess_ID;
			ss >> sess_ID;
	
			msg.type = c_NEW_SESS;
			msg.size = sess_ID.length() + 1;
			memcpy(&msg.source, client_ID.c_str(), client_ID.length() + 1);
			memcpy(&msg.data, sess_ID.c_str(), sess_ID.length() + 1);
					std::cout << msg.source;
			send(sckt, &msg, sizeof(msg), 0);
		}

		command = "2412";
	}



	// while(1);
	close(sckt);
}