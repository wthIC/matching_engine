#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <fstream>
#include <sstream>

using namespace std;

//To inform the threads whether to keep running or not 
std::atomic<bool> keepRunning(true);

// Struct to hold information regarding orders
struct Order {
    int orderID;
    int type;
    int price;
    int quantity;
};

bool isRandom;
int testIdx, testSize = 0;
vector<Order> testOrders;


// Function to send orders in a thread
void sendOrders(int sock) {
    int global_id = 1; //To determine the Order ID of the each sent order
    while(true) { // Change this into a different condition if you want to send a set amount of orders 
        if (!isRandom && testIdx == testSize) {
            //std::cout << "Test case done! " << std::endl;
            //keepRunning = false;
            break;
        }
        char order[1024];
        int type = isRandom ? rand() % 2 : testOrders[testIdx].type; // Randomly choose between 0 (Buy) and 1 (Sell)
        int price = isRandom ? rand() % 100 + 1 : testOrders[testIdx].price; // Random price
        int quantity = isRandom ? rand() % 50 + 1 : testOrders[testIdx].quantity; // Random quantity

        snprintf(order, sizeof(order), "OrderID:%d, Type:%s, Price:%d, Quantity:%d\n",
                isRandom ? global_id++ : testOrders[testIdx++].orderID, (type == 0 ? "Sell" : "Buy"), price, quantity); 

        if (send(sock, order, strlen(order), 0) < 0) {
            std::cerr << "Failed to send order." << std::endl;
            break;
        }
    }
}

//Function to listen for responses from server in a thread
void receiveResponses(int sock) {
    char buffer[1024];
    while (keepRunning) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(sock, buffer, sizeof(buffer), 0);
        std::string bufferStr = "";
        for (int i = 0;i<1024;i++) {
            bufferStr += buffer[i];
        }
        if (bytes_read > 0) {
            std::cout <<  bufferStr ;
            sleep(2);
        } else if (bytes_read <= 0) {
            break; // Break the loop if the server closes connection or error occurs
        }
    }
}

int main(int argc, char *argv[]) {

    // Check if the command line argument corresponds with random or preset testing
    if (argc > 1 && std::string(argv[1]) == "false") {
        isRandom = false;
        testIdx = 0;
        std::string line;
        ifstream MyReadFile("test4.txt");
        int cnt = 1; //Keep track of order id
        while (getline(MyReadFile, line)) {
            testSize++;
            std::istringstream lineStream(line);
            std::string token;
            Order order;

            order.orderID = cnt++;

            // Extracting Type
            std::getline(lineStream, token, ',');
            order.type = std::stoi(token);

            // Extracting Price
            std::getline(lineStream, token, ',');
            order.price = std::stoi(token);

            // Extracting Quantity
            std::getline(lineStream, token);
            order.quantity = std::stoi(token);

            testOrders.push_back(order);
        }
    }
    else {
        isRandom = true;
    }
    

    // Server details
    const char* server_ip = "127.0.0.1";
    int server_port = 8080;

    // Initialize random seed
    std::srand(std::time(nullptr));

    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    // Define server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection to server failed." << std::endl;
        return 1;
    }

    // Create threads and join them for running two functions concurrently 
    std::thread sendThread(sendOrders, sock);
    std::thread receiveThread(receiveResponses, sock);

    sendThread.join();
    receiveThread.join();

    // Close the socket
    close(sock);

    return 0;
}
