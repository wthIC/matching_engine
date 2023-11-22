#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <sys/time.h>


// Struct to hold information regarding orders
struct Order {
    int orderID;
    int type;
    int price;
    int quantity;
};

//To keep track of the incomplete orders at the end of each message from data_generator
std::string incompleteOrderStr = "";

//Parse the message sent from data_genarator and keep track of incomplete orders at the end if any
std::vector<Order> parseOrderString(const std::string& ordersStr) {
    std::vector<Order> orders;
    std::string line = "";

    for (int i = 0;i<ordersStr.length();i++) {
        //Look for endline symbols (indicates end of an order)
        if (ordersStr[i] == '\n') {
            std::istringstream lineStream(line);
            std::string token;
            Order order;

            // Extracting OrderID
            std::getline(lineStream, token, ':'); // Skip "OrderID"
            std::getline(lineStream, token, ',');
            order.orderID = std::stoi(token);

            // Extracting Type
            std::getline(lineStream, token, ':'); // Skip "Type"
            std::getline(lineStream, token, ',');
            order.type = token == "Sell" ? 0 : 1;

            // Extracting Price
            std::getline(lineStream, token, ':'); // Skip "Price"
            std::getline(lineStream, token, ',');
            order.price = std::stoi(token);

            // Extracting Quantity
            std::getline(lineStream, token, ':'); // Skip "Quantity"
            std::getline(lineStream, token);
            order.quantity = std::stoi(token);


            orders.push_back(order); // Add the parsed order to the vector
            line = "";
        }
        else {
            line += ordersStr[i];
        }
    }

    //If any characters are remaining, this is an incomplete order as we did not reach to an endine symbol
    incompleteOrderStr = line;

    return orders;
}

//To keep track of remaining buy (0) and sell (1) orders in first-in-first-out fashion
std::unordered_map<int, std::queue< int > >orders[2];

//To keep track of how much of the order is remaning in order to accomplish partial filling
std::unordered_map<int,int> remaining;

//Generate a string to return to the client
std::string printMatching(int buyerID, int sellerID, int price, int quantity) {
    return "Match found - Buyer: " + std::to_string(buyerID) + ", Seller: " + std::to_string(sellerID) + ", Price: " + 
        std::to_string(price) + ", Quantity: " + std::to_string(quantity) + '\n';
}

int main(int argc, char *argv[]) {

    //Check if the user wants to see information regarding received orders
    bool printOrderInformation = argc <= 1 ? false : (std::string(argv[1]) == "true");

    // Create a socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    // Define server information to listen without any specific IP
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);  // Port number to listen on

    //Bind to the server
    if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        return 1;
    }

    //Start listening for any messages from clients
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed." << std::endl;
        return 1;
    }

    std::cout << "Server started. Listening for connections..." << std::endl;

    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    while (true) {

        if (new_socket < 0) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        char buffer[1024] = {0};
        int bytes_read = recv(new_socket, buffer, 1024, 0);
        if (bytes_read < 0) {
            std::cerr << "Failed to read from socket." << std::endl;
        } 
        else if (bytes_read == 0) {
            std::cout << "Client disconnected." << std::endl;
            break;
        } 
        else {

            //Transfer the received order into a new string in order to eliminate any unintended symbols created due to overflowing length
            std::string bufferStr = "";
            for (int i = 0;i<1024;i++) {
                bufferStr += buffer[i];
            }

            if (printOrderInformation) {
                std::cout << "Received from client: " << std::endl;
                std::cout << bufferStr << std::endl << std::endl;
            }
            
            //Parse the order string into a vector of orders
            std::vector<Order> newOrders = parseOrderString(incompleteOrderStr + bufferStr);

            //Iterate through each order
            for (auto newOrder : newOrders) {
                int id = newOrder.orderID;
                int type = newOrder.type;
                int price = newOrder.price;
                int quantity = newOrder.quantity;

                //Check if there are any orders with same price in the opposite order type and process them and send a message to client
                while (!orders[1 - type][price].empty() && quantity > 0) {
                    int x = orders[1-type][price].front();
                    if (remaining[x] == 0) {
                        orders[1-type][price].pop();
                    }
                    else if (remaining[x] <= quantity) {
                        quantity -= remaining[x];
                        orders[1-type][price].pop();
                        std::string message = printMatching(type == 0 ? x : id, type == 0 ? id : x, price, remaining[x]);
                        send(new_socket, message.c_str(), message.size(), 0);
                    }
                    else {
                        remaining[x] -= quantity;
                        std::string message = printMatching(type == 0 ? x : id, type == 0 ? id : x, price, quantity);
                        send(new_socket, message.c_str(), message.size(), 0);
                        quantity = 0;
                        break;
                    }
                }

                //If still remeaning, add it to the map of withstanding orders
                if (quantity > 0) {
                    remaining[id] = quantity;
                    orders[type][price].push(id);
                }
            }
            
        }

    }

    //Don't forget the close the socket and the server
    close(new_socket);
    close(server_fd);

    return 0;
}