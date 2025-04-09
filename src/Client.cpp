#include <asio.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io;

        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");
        tcp::socket socket(io);
        asio::connect(socket, endpoints);

        // Отправляем GET-запрос
        std::string request = "GET /sample HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
        asio::write(socket, asio::buffer(request));

        // Читаем заголовок: имя файла и размер
        asio::streambuf header_buf;
        asio::read_until(socket, header_buf, "\n\n");
        std::istream header_stream(&header_buf);

        std::string filename;
        size_t filesize;
        header_stream >> filename >> filesize;
        header_stream.ignore(2); // Пропускаем лишний \n\n

        std::cout << "📦 Receiving file: " << filename << ", size: " << filesize << " bytes\n";

        // Создаем файл
        std::ofstream output_file(filename, std::ios::binary);
        if (!output_file.is_open()) {
            std::cerr << "❌ Failed to open file for writing\n";
            return 1;
        }

        // Сначала — всё, что уже в буфере
        size_t bytes_written = 0;
        if (header_buf.size() > 0) {
            std::vector<char> buf(header_buf.size());
            header_buf.sgetn(buf.data(), buf.size());
            output_file.write(buf.data(), buf.size());
            bytes_written += buf.size();
        }

        // Читаем оставшееся
        std::vector<char> buffer(4096);
        while (bytes_written < filesize) {
            size_t to_read = std::min(buffer.size(), filesize - bytes_written);
            size_t bytes_read = socket.read_some(asio::buffer(buffer.data(), to_read));
            output_file.write(buffer.data(), bytes_read);
            bytes_written += bytes_read;
        }

        std::cout << "✅ File received successfully.\n";
        output_file.close();
    }
    catch (std::exception& e) {
        std::cerr << "⚠ Exception: " << e.what() << "\n";
    }

    return 0;
}
