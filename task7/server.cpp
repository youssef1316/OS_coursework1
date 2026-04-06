#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

//the chosen port
constexpr int PORT = 8080;
//max number of clients
constexpr size_t MAX_CLIENTS = 10;
//the decided size of a block to be sent
constexpr size_t BUFFER_SIZE = 1024;

// a list of all the clients connected
vector<int> client_sockets;
//a mutex to prevent race conditions
mutex clients_mutex;


//loop over all connected clients to send the message
void broadcast_message(const string& message, int sender_fd) {
    //locks the mutex to prevent race conditions
    lock_guard<mutex> lock(clients_mutex);
    for (int client_fd : client_sockets) {
        //insure that the sender doesnt receive the message they sent
        if (client_fd != sender_fd) {
            //send the message to the user
            //posix system call to send message using TCP to the connected socket
            //to - message - total size - flag showing standard behavior
            send(client_fd, message.c_str(), message.length(), 0);
        }
    }
    //a mutex lock is needed to prevent the crash if a client disconnects
    //while another user is sending a message at the same time
}

//listener for a single connected client
//run in threads
void handle_client(int client_fd) {
    //splitting the message into chunks of the selected buffer size
    char buffer[BUFFER_SIZE];
    int bytes_received;

    //receive the message sent by a user
    //if there are no messages being received, the thread is on hold until a message is received
    while ((bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        //assign null char to the end of the received messages
        buffer[bytes_received] = '\0';
        string message = "Client " + to_string(client_fd) + ": " + buffer;
        cout << message;
        //send the message to the other users
        broadcast_message(message, client_fd);
    }

    // Handle client disconnect
    close(client_fd);
    {
        //using a mutex lock, the user leaving is being deleted from the client vector
        lock_guard<mutex> lock(clients_mutex);
        client_sockets.erase(remove(client_sockets.begin(), client_sockets.end(), client_fd), client_sockets.end());
    }
    cout << "Client " << client_fd << " disconnected.\n";
}

int main() {
    //server and new client file descriptor
    int server_fd, new_socket;
    //network config
    struct sockaddr_in address;
    //the size of the address the accept function will need
    socklen_t addrlen = sizeof(address);

    //creates the main server check
    //AF_INET --> IPV4
    //SOCK_STREAM --> TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket failed\n";
        //error if the server couldnt be initialized
        return EXIT_FAILURE;
    }

    // Allow port reuse to prevent "Address already in use" errors upon restart
    int opt = 1;
    //forcing the OS to release the port immediately in case of restart
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //assign the address to IPV4
    address.sin_family = AF_INET;
    //listen from any network location
    address.sin_addr.s_addr = INADDR_ANY;
    //assign the port in the form of a network byte
    address.sin_port = htons(PORT);

    //links the server_fd to the configured network settings
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    //make the server listening
    //the limit of accepting new users simultaneously
    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed\n";
        return EXIT_FAILURE;
    }

    cout << "Server listening on port " << PORT << "...\n";

    //the main loop keeping the server up
    while (true) {
        //accepting a new user connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            cerr << "Accept failed\n";
            continue;
        }

        cout << "New client connected: " << new_socket << "\n";
        //check if there is room for more connections
        {
            //lock to prevent race conditions
            lock_guard<mutex> lock(clients_mutex);
            if (client_sockets.size() >= MAX_CLIENTS) {
                cout << "Max clients reached. Rejecting connection.\n";
                close(new_socket);
                continue;
            }
            client_sockets.push_back(new_socket);
        }

        // Spawn a new detached thread for the client
        thread(handle_client, new_socket).detach();
    }

    close(server_fd);
    return 0;
}