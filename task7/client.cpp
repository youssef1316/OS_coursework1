#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

//the network port
constexpr int PORT = 8080;
//the buffer size
constexpr size_t BUFFER_SIZE = 1024;

//the receive message logic
//can run in background to allow receiving while typing
void receive_messages(int sock) {
    //incoming array of chars
    char buffer[BUFFER_SIZE];
    //number of bytes received
    int bytes_received;

    //keeps the thread paused until the server sends something
    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        //assign null char
        buffer[bytes_received] = '\0';
        cout << "\r" << buffer;
        //to keep showing the You message
        cout << "You: " << flush;
    }

    cout << "\nServer disconnected.\n";
    exit(0);
}

int main() {
    //new socket
    int sock = 0;
    //network config
    struct sockaddr_in serv_addr;
    string message;

    //assign network config to the socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error\n";
        return -1;
    }

    //expect IPV4
    //server address
    serv_addr.sin_family = AF_INET;
    //convert the port into network byte order
    serv_addr.sin_port = htons(PORT);

    //conver into raw binary format and storing in the server address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address / Address not supported\n";
        return -1;
    }

    //try to connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed\n";
        return -1;
    }

    cout << "Connected to the chat server.\n";

    // Spawn a detached thread to handle incoming messages
    thread(receive_messages, sock).detach();

    //main loop to send the messages
    while (true) {
        cout << "You: ";
        getline(cin, message);

        if (!message.empty()) {
            message += "\n"; // Append newline as a message delimiter
            send(sock, message.c_str(), message.length(), 0);
        }
    }

    close(sock);
    return 0;
}