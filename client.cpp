#include "common.h"


int main(int argc, char** argv)
{


	struct message msg;
	int signup;
	std::string input;
	//Asks User to signup or Login
	std::cout << "1: SIGNUP" <<std::endl << "2: LOGIN" << std::endl;
	std::getline(std::cin, input);
	std::stringstream ss(input);
	ss >> signup;

	if(signup == 1)
		msg.signup = true;

	std::string command, client_ID, psswd, server_IP, server_port;

	// makes sure client logs in first
	do 
	{
		std::getline(std::cin, input);
		std::stringstream ss(input);

		ss >> command >> client_ID >> psswd >> server_IP >> server_port;

		if(command != "/login")
		 	std::cout << "Login first" << std::endl;

				
	}while(command != "/login");


	//Connection establishment
	struct addrinfo hints, *res;
	addrinfo_init(name_to_IP(server_IP.c_str()), server_port.c_str(), &hints, &res);
	int sckt = connection_establishment(hints, res, 1);
	

	// Creates login message

	while(1)
	{
		create_msg(msg, c_LOGIN, psswd.length(), client_ID, psswd);

		send(sckt, &msg, sizeof(msg), 0);
		recv(sckt, &msg, sizeof(msg), 0);

		if(msg.type == c_LO_ACK){
			std::cout << msg.data << std::endl;
			break;
		}
		else if(msg.type == c_LO_NACK)
		{
			std::string check;
			do 
			{
				std::getline(std::cin, input);
				std::stringstream ss(input);

				ss >> check >> client_ID >> psswd >> server_IP >> server_port;
				// std::cout << command << std::endl;

				if(check != "/login")
				 	std::cout << "Login first" << std::endl;

						
			}while(check != "/login");
		}
	}


	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	setsockopt(sckt, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

	while(1)
	{
		struct message new_msg;
		std::getline(std::cin, input);
		std::stringstream ss(input);

		recv(sckt, &msg, sizeof(msg), 0);

		ss >> command;

		if(command == "/createsession")
		{

			std::string sess_ID;
			ss >> sess_ID;
	
			create_msg(msg, c_NEW_SESS, sess_ID.length() + 1,client_ID, sess_ID);
			send(sckt, &msg, sizeof(msg), 0);
			recv(sckt, &msg, sizeof(msg), 0);

			if(msg.type == c_NS_ACK)
				std::cout << "Successfully Created " << msg.data << std::endl;
			else
				std::cout << "Failed to Create " << msg.data << std::endl;				
		}
		else if(command == "/joinsession")
		{
			std::string sess_ID;
			ss >> sess_ID;

			create_msg(msg, c_JOIN, sess_ID.length() + 1, client_ID, sess_ID);
			send(sckt, &msg, sizeof(msg), 0);
			recv(sckt, &msg, sizeof(msg), 0);

			if(msg.type == c_JN_ACK)
				std::cout << "Successfully Joined " << msg.data << std::endl;
			else
				std::cout << "Failed to Join " << msg.data << std::endl;	
	
		}
		else if(msg.type == c_MESSAGE)
		{
			std::cout << msg.data;
		}
		else
		{
			std::string message;
			ss >> message;

			create_msg(msg, c_MESSAGE, message.length() + 1, client_ID, message);
			send(sckt, &msg, sizeof(msg), 0);
		}
		// std::cout << "TEST";
		// std::cout << command << std::endl;
		// command = " ";
	}


	close(sckt);
}

// BUGGY connection establish function??
//MAKE USER PRINT OUT SESSION ID