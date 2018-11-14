#include "common.h"

void sending_func(int sckt, std::string client_ID);

volatile int exited = 0;

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
			std::cout << std::endl << msg.data << std::endl;
			std::cout << "1: SIGNUP" <<std::endl << "2: LOGIN" << std::endl;

			std::getline(std::cin, input);
			std::stringstream ss1(input);
			ss1 >> signup;

			msg.signup = signup == 1 ? true : false;

			std::string check;
			do 
			{
				std::getline(std::cin, input);
				std::stringstream ss(input);

				ss >> check >> client_ID >> psswd >> server_IP >> server_port;

				if(check != "/login")
				 	std::cout << "Login first" << std::endl;

						
			}while(check != "/login");
		}
	}


	// struct timeval tv;
	// tv.tv_sec = 0;
	// tv.tv_usec = 1000;
	// setsockopt(sckt, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);


	std::thread sender(&sending_func, sckt,client_ID);

	while(exited == 0)
	{
		// struct message new_msg;

		recv(sckt, &msg, sizeof(msg), 0);

		switch(msg.type)
		{

			//MESSAGING
			case c_MESSAGE:
				std::cout << msg.data << std::endl;
				break;

			// Join session ACKS
			case c_JN_ACK:
				std::cout << "Successfully Joined!" << std::endl << "Session: " << msg.data << std::endl;
				break;
			case c_JN_NACK:
				std::cout << "Failed to Join" <<std::endl << "Session: " << msg.data << std::endl;
				break;

			// New Session ACKS
			case c_NS_ACK:
				std::cout << "Successfully Created!" << std::endl << "Session: " << msg.data << std::endl;
				break;
			case c_NS_NACK:
				std::cout << "Failed to Create" <<std::endl << "Session: " << msg.data << std::endl;
				break;
			case c_QUERY:
				std::cout << msg.data << std::endl;
				break;
			case c_QUACK:
				std::cout << "End of List" <<std::endl;
				break;
			case c_QUIT:
				std::cout << "EXIT" << std::endl;
				exited = 1; // lock?
				break;

		}
	}

	sender.join();

	close(sckt);
}

void sending_func(int sckt, std::string client_ID)
{
	while(exited == 0)
	{
		std::string command, input;
		struct message msg;
		std::getline(std::cin, input);
		std::stringstream ss(input);

		ss >> command;



		if(command == "/createsession")
		{
			std::string sess_ID;
			ss >> sess_ID;
		
			create_msg(msg, c_NEW_SESS, sess_ID.length() + 1, client_ID, sess_ID);
			send(sckt, &msg, sizeof(msg), 0);

			std::cout << std::endl;
		}
		else if(command == "/joinsession")
		{
			std::string sess_ID;
			ss >> sess_ID;

			create_msg(msg, c_JOIN, sess_ID.length() + 1, client_ID, sess_ID);
			send(sckt, &msg, sizeof(msg), 0);

			std::cout << std::endl;
		}
		else if(command == "/leavesession")
		{

			create_msg(msg, c_LEAVE_SESS, 0 , client_ID , " ");
			send(sckt, &msg, sizeof(msg), 0);
			std::cout << std::endl;
		}
		else if(command == "/list")
		{
			create_msg(msg, c_QUERY, 0 , client_ID , " ");
			send(sckt, &msg, sizeof(msg), 0);
			std::cout << std::endl;			
		}
		else if(command == "/quit")
		{
			create_msg(msg, c_LEAVE_SESS, 0 , client_ID , " ");
			send(sckt, &msg, sizeof(msg), 0);

			create_msg(msg, c_QUIT, 0, client_ID, "");
			send(sckt, &msg, sizeof(msg), 0);

			while(exited != 1);
			
		}
		else
		{
			// std::string message;
			// ss >> message;
			create_msg(msg, c_MESSAGE, command.length() + 1, client_ID, command);
			send(sckt, &msg, sizeof(msg), 0);
		}
	}
}

//TODO: MAKE CONDITIONS FOR THREAD EXITING