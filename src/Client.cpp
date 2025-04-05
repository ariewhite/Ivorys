#include "asio.hpp"
#include <iostream>
using asio::ip::tcp;

int main() {
    try {
        asio::io_context context;
        tcp::resolver resolver(context);
        auto endpoints = resolver.resolve("127.0.0.1", "8070");
        tcp::socket socket(context);
        asio::connect(socket, endpoints);

        std::string msg = "Hello from client!";
        socket.write_some(asio::buffer(msg));

        char reply[1024];
        size_t len = socket.read_some(asio::buffer(reply));
        std::cout << "Reply: " << std::string(reply, len) << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}
