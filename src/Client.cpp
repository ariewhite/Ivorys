#include <asio.hpp>
#include <iostream>
#include <vector>

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io;

        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");
        tcp::socket socket(io);
        asio::connect(socket, endpoints);

        // Отправка запроса
        std::string request = "GET /sample HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
        asio::write(socket, asio::buffer(request));

        std::vector<char> buffer(4096);
        std::string response;

        bool done = false;
        while (!done) {
            asio::error_code ec;
            std::size_t length = socket.read_some(asio::buffer(buffer), ec);

            if (!ec) {
                response.append(buffer.begin(), buffer.begin() + length);

                size_t pos_end = response.find_first_of("\r\n\r\n");

                if (pos_end) {
                    std::string header = response.substr(0, pos_end + 1);
                    std::cout << header << std::endl
                        << response << std::endl 
                        << "size: " << pos_end << std::endl;
                    
                    
                    done = true;
                    buffer.clear();
                }
            }

            if (ec == asio::error::eof) {
                // Соединение закрыто сервером — выходим
                break;
            }
            else if (ec) {
                // Любая другая ошибка
                throw asio::system_error(ec);
            }
        }
        buffer.clear();

        //std::cout << "норм строка\n" << response << std::endl;

        std::string limiter = "\r\n";
        
        size_t file_size;
        std::string file_name;

        auto start = 0U;
        auto pos = response.find(limiter);
        std::string token;
        while (pos != std::string::npos)
        {
            token = response.substr(start, pos - start);
            start = pos + limiter.length();
            pos = response.find(limiter, start);
            

            if (token.contains("Content-Length")) {
                //token.substr(0, token.find(":"));
                token = token.erase(0, token.find(":" + 1));
                std::cout << "file size: " << token;
            }
        }

       
    }
    catch (std::exception& e) {
        std::cerr << "⚠ Exception: " << e.what() << "\n";
    }

    return 0;
}
