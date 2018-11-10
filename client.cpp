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


	while(1)
	{
		struct message new_msg;
		// std::cout << "Enter:" << std::endl;
		std::getline(std::cin, input);
		std::stringstream ss(input);

		ss >> command;

		if(command == "/createsession")
		{
			std::string sess_ID;
			ss >> sess_ID;
	
			create_msg(msg, c_NEW_SESS, sess_ID.length() + 1,client_ID, sess_ID);
			send(sckt, &msg, sizeof(msg), 0);
			ss.clear();
		}
		else if(command == "test")
			break;

		std::cout << "TEST";
		// std::cout << command;
		// command = " ";
	}


	close(sckt);
}

// BUGGY connection establish function??