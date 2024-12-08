// Assignment 3 - CSCN72020 - Client - Liam and Destiny 
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "termcolor.hpp"

#define PORT 27000
#define BUFFER_SIZE 1024 // Temporary storage for data
using namespace termcolor;

int main(void) {
	//displayMenu();
	char buffer [BUFFER_SIZE] = {};
	std::cout << on_yellow << "Starting client..." << reset<<std::endl; // Print statements used for debugging
	
	// Create socket
	int ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == -1) {
		std::cerr<<on_red << "ERROR: Failed to create Client Socket."<<reset << std::endl;
		return 0;
	}
	std::cout<<on_yellow << "Client socket created."<<reset << std::endl;

	// Set server address
	sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_addr.s_addr = inet_addr("172.16.5.12"); // address of server
	SvrAddr.sin_port = htons(PORT);
	
	// Connect to server
	if (connect(ClientSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) == -1) {
		close(ClientSocket);
		std::cerr <<on_red<< "ERROR: Failed to connect to server."<<reset << std::endl;
		return 0;
	}
	while(true){
	// 		char userEmpty[BUFFER_SIZE];
	// int userEmptySignal = recv(ClientSocket, userEmpty, BUFFER_SIZE, 0);
	// std::cout<<on_green << "Client connected to server."<<reset << std::endl;
	// std::cout << userEmptySignal<<std::endl;
	// if(userEmpty[0] == '1'){
	// 	std::cout<<on_cyan<<"There is no user available, please make one."<<reset<<std::endl;
	// }
		std::string option;
		std::cout<<"Press 1 to login."<<std::endl;
		std::cout<<"Press 2 to create user"<<std::endl;
		std::cout<<"Press 3 to disconnect"<<std::endl;
		std::getline(std::cin,option);
		if(option == "1"){
			std::string mode = "l";
			send(ClientSocket, mode.c_str(), mode.size()+1, 0);
			std::string userName;
			std::string password;
			std::cout <<blue<< "Enter username: "<<reset;
			std::getline(std::cin, userName);
			std::cout <<blue<< "Enter password: "<<reset;
			std::getline(std::cin, password);
			std::string login = userName + "\n" + password;
			send(ClientSocket, login.c_str(), login.size(), 0);
			char loginMessage[BUFFER_SIZE];
			int loginRes = recv(ClientSocket, loginMessage, BUFFER_SIZE, 0);
			loginMessage[loginRes] = '\0';
			if(loginRes <= 0){
				std::cerr<<on_red<<"ERROR: Can't receive login response from server, please try to connect again."<<reset<<std::endl;
				close(ClientSocket);
				return -1;
			}
			else{
				std::string loginAuth(loginMessage);
				if(loginAuth == "Authenticate successfully!"){
					std::cout<<on_green<<"Login successfully!"<<reset<<std::endl;
					break;
				}
				else{
					std::cout<<on_red<<"Login failed, username or password is incorrect"<<reset<<std::endl;
				}
			}
		}
		else if(option == "2"){
			std::string mode = "c";
			send(ClientSocket, mode.c_str(), mode.size()+1, 0);
			std::string userName;
			std::string password;
			std::cout <<blue<< "Enter username: "<<reset;
			std::getline(std::cin, userName);
			std::cout <<blue<< "Enter password: "<<reset;
			std::getline(std::cin, password);
			std::string login = userName + "\n" + password;
			send(ClientSocket, login.c_str(), login.size(), 0);
			std::cout <<on_cyan<<"New user created"<<reset<<std::endl;
		}
		else if(option == "3"){
			std::cout <<on_yellow<<"Disconnected with server, bye"<<reset<<std::endl;
			close(ClientSocket);
			return 1;
		}
		else{
			std::cout << on_red <<"Invalid option, please choose again"<<reset<<std::endl;
		}
	}

	// Client menu
	std::string choice; // changed to string
	while (true) {
		std::cout << "Press 1 to send a message." << std::endl;
		std::cout << "Press 2 to exit." << std::endl;
		std::cout << "Enter your choice: ";
		std::getline(std::cin, choice); 
		std::cout<<green << "The choice you entered: " << choice << reset<<std::endl;
		
		// Send a message was chosen
		if (choice == "1") {
			std::string author, topic, message;
			
			// Anonymous option
			std::string anonymous;
			std::cout << "Would you like to stay anonymous?" << std::endl;
			std::cout << "Press 'y' for YES. Press 'n' for NO." << std::endl;
			std::cin >> anonymous;
			std::cin.ignore(); // clear input buffer
			
			// Author
			if (anonymous == "n" || anonymous == "N") {
				std::cout << "Enter your name: " << std::endl;
				std::getline(std::cin, author);	
			} else if (anonymous == "y" || anonymous == "Y") {
			 	author = "Anonymous";	
			}
			std::cout << "Author: " << author << std::endl;
			
			// Topic
			std::cout << "Enter a topic: " << std::endl;
			std::getline(std::cin, topic);
			std::cout << "Topic: " << topic << std::endl;
			
			// Message
			std::cout << "Enter your message (please end your message with a period '.'): " << std::endl;
			std::getline(std::cin, message);
			std::cout << "Message: " << message << std::endl;
			
			if (message.back() != '.') {
				std::cerr<<on_red << "ERROR: Message must end with a period '.'"<<reset<<std::endl;
				continue;
			}
			
			// Send message
			std::string fullMessage = author + "\n" + topic + "\n" + message;
			std::cout << "The message you sent: " << fullMessage << std::endl;
			send(ClientSocket, fullMessage.c_str(), fullMessage.size() + 1, 0);
			
			// Receive server response
			int svrResponse = recv(ClientSocket, buffer, BUFFER_SIZE, 0);
			if(svrResponse==0){
				std::cout<<on_yellow << "Server disconnected, bye"<<reset<<std::endl;
				break;
			}
			std::cout<<on_magenta << "Server response: " << buffer <<reset<< std::endl;
			
		} else if (choice == "2") {
		
			std::cout<<on_yellow << "Disconnected, good bye!"<<reset << std::endl;
            std::string fullMessage = "\0";
            send(ClientSocket, fullMessage.c_str(), fullMessage.size(), 0);
			close(ClientSocket);
			break;
			
		} else {
		
			std::cout<<on_red << "Invalid choice. Please try again." <<reset << std::endl;
			
		}
	}

	//Close socket
	close(ClientSocket);

	return 0;
}
