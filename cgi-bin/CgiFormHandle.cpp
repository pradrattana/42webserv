/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiFormHandle.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npiya-is <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/29 18:43:52 by npiya-is          #+#    #+#             */
/*   Updated: 2023/11/09 22:05:26 by npiya-is         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fstream> // Add this line to include the <fstream> header file
#include <cstring>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>


//download file form method

// Simple HTTP response for successful file upload
const char *successResponse =
	"HTTP/1.1 200 OK\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<html><body>File received successfully</body></html>";

// Simple HTTP response for invalid request
const char *invalidResponse =
	"HTTP/1.1 405 Method Not Allowed\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<html><body>Method Not Allowed</body></html>";

const int BUFFER_SIZE = 100000;

#include "../cgi/CgiPostHandle.hpp"

int main(int argc, char *argv[]){
	char buffer[BUFFER_SIZE + 1] = {0};
	std::cout << "-------------Execute FormCgi-------------\n";
	std::cout << "argc: " << argc << std::endl;
	if (argc != 3){
		std::cout << "Usage: ./cgiFormHandle socketfd filebuffer boundary contentlength\n";
		send(std::atoi(argv[1]), invalidResponse, strlen(invalidResponse), 0);
		return EXIT_FAILURE;
	}
	int sockfd = std::atoi(argv[1]);
	int writefd = std::atoi(argv[2]);
	std::cout << "sockfd: " << sockfd << std::endl;
	std::cout << "writefd: " << writefd << std::endl;
	int bytes_received = 0;
	bytes_received = recv(sockfd, buffer, BUFFER_SIZE, MSG_PEEK);
	if (errno)
		perror("read error : ");
	CgiPostHandle cgiPost;
	cgiPost.setSockfd(sockfd, writefd);
	cgiPost.findContentlength(buffer);
	if ((bytes_received = recv(sockfd, buffer, bytes_received, 0)) < 0){
		perror("read error : ");
		return EXIT_FAILURE;
	}
	// while (cgiPost.getBytesRead() <= cgiPost.getContentLength()){
	while (bytes_received > 0) {
		cgiPost.setFilebuffer(buffer);
		bytes_received = cgiPost.createFile(buffer, sockfd, bytes_received);
		// std::cout << "bytes_received: " << bytes_received << std::endl;
	}
	if (bytes_received == 0){
		send(sockfd, successResponse, strlen(successResponse), 0);
	}
	else {
		send(sockfd, invalidResponse, strlen(invalidResponse), 0);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
//c++ -Wall -Werror -Wextra -std=c++98 CgiFormHandle.cpp -o testcgi