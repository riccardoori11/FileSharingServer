#include <netinet/in.h>
#include <cstdint>
#include <filesystem>
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>


class FileServer{

		
private :

		int server_port;
		int serversocket;
		std::string dir;

public:
		
		//Constructor
		FileServer(int port,const std::string dir):server_port(port),serversocket(-1),dir(dir) {}
		
		~FileServer(){
				
				if (serversocket == -1){
						
						close(serversocket);
				}

		}
		
			void start(){

				serversocket = socket(AF_INET, SOCK_STREAM,0);

				if (serversocket == -1){
						
						throw std::runtime_error("Creation of socket failed");

				}

				sockaddr_in server_addr;
				server_addr.sin_addr.s_addr = INADDR_ANY;
				server_addr.sin_port = htons(server_port);
				server_addr.sin_family = AF_INET;

				if (bind(serversocket, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){
						
						throw std::runtime_error("Failed to bind");


				}

				if(listen(serversocket,5) == -1){
						
						throw std::runtime_error("Failed to listen");

				}
				
				std::cout << "Server listening on port: "<< server_port << std::endl;
				
				//accepting clients indefinitiely
				while (true){
						
						sockaddr_in clientAddr;
						socklen_t clientAddr_size = sizeof(clientAddr);
						int client_socket = accept(serversocket,(struct sockaddr*)&clientAddr,&clientAddr_size);

						if (client_socket == -1){
								std::cerr<< "Failed to accept connection" << std::endl;
								// continue the server
								continue;
						}

						char clientIP[INET_ADDRSTRLEN];
						inet_ntop(AF_INET,&clientAddr.sin_addr,clientIP,INET_ADDRSTRLEN);
						int clientPort = ntohs(clientAddr.sin_port);
						std::cout << "Accepted connection from " << clientIP << ":" << clientPort
						<< std::endl;

						handleClient(client_socket);
						close(client_socket);






		}
	}	

private:

	void handleClient(int client_socket){

				char buffer[1024];
				ssize_t command_bytes = recv(client_socket,buffer,sizeof(buffer) -1,0);

				
				if (command_bytes <= 0){
						
						std::cout << "did not pass a command"<< std::endl;
						return;

				}
				buffer[command_bytes] = '\0';

				std::string command(buffer);

				std::cout << "Command demanded:" << command << std::endl;

				if (command.substr(0,4) == "List"){
						
						listfile(client_socket);
				}

				




	}

	void listfile(int client_socket){

				std::string filelist = "Files: \n";
				
				std::string filename;

				//catch error when iterating
				std::error_code ec;
				
				if (!std::filesystem::exists(dir)){
						
						filelist = "Directory does not exist\n";
						send(client_socket,filelist.c_str(),filelist.length(),0);
						std::cerr << "Directory does not exist" << std::endl;
						return;



				}

				if (!std::filesystem::is_directory(dir)){
						
						filelist = "Not a directory \n";
						send(client_socket,filelist.c_str(),filelist.length(),0);
						std::cerr << "Not a directory" << std::endl;
						return;

				}
				
				auto dir_itr = std::filesystem::recursive_directory_iterator(dir,ec);
				
				if (ec){
						
						filelist = "Error cannot access directory: "+ ec.message() + "\n";
						send(client_socket,filelist.c_str(),filelist.length(),0);
						std::cerr <<  ec.message() << std::endl;
						return;

				}

				for (auto const& dir_entry: dir_itr){

						if (!dir_entry.is_regular_file(ec)){

								std::cerr << "Could not verify file content\n" << std::endl;
								ec.clear();
								continue;
						}

						else{
						
							filename = dir_entry.path().string();	
							uintmax_t size = dir_entry.file_size(ec);

							if (ec){
								
								filelist += filename += "(size unknown) \n";
								ec.clear();
							}
							else{
								
								filelist += filename + "(" + std::to_string(size) + ")";

							}

						}
				}

				if (filelist == "Files: \n"){
						
						filelist += "No files found";
				}

				send(client_socket,filelist.c_str(),filelist.length(),0);
				std:: cout << "Sent the file list\n" << std::endl;
	}






};


int main(){

		





		return 0;
}
