// FileServer.cpp
#include "FileServer.h"
#include <iostream>


session::session(tcp::socket socket)
    : socket_(std::move(socket))
{
    
}

void session::start()
{
    do_read();
}

void session::do_read()
{
    auto self = shared_from_this();
    socket_.async_read_some(buffer(buffer_.prepare(1024)),
        [this, self](asio::error_code ec, std::size_t length)
        {
            if (!ec) {
                buffer_.commit(length);
                std::istream istream(&buffer_);

                std::string method, path, version;
                istream >> method >> path >> version;

                std::cout << "=== Accepted data ===\n";
                std::cout << method << " " << path << " " << version << "\n";
                std::cout << "========================\n";

                if (method == "GET") {
                    if (path == "/sample") {
                        send_file("C:/Users/murzabaev.c/project/Ivorys/rockyou.txt");
                        return;
                    }
                }

                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 13\r\n"
                    "\r\n"
                    "Hello, world!";

                asio::async_write(socket_, asio::buffer(response),
                    [self](asio::error_code, std::size_t) {});
            }
            else if (ec != asio::error::eof) {
                std::cerr << "Read error: " << ec.message() << "\n";
            }
        });
}

void session::send_file(const std::string& filename)
{
    // open file
    auto fileres = open_file(filename);
    // get hash sum (sha-256)
    auto hash = calculate_sha256(filename);

    // prepare to first response w/ file info
    http_request hello;
    hello.headers_.emplace("Filename", filename);
    hello.headers_.emplace("SHA-256", hash);
    hello.body_ = "Robot, wait for file:)";

    asio::async_write(socket_, asio::buffer(hello.get_string()),
        [self = shared_from_this()](asio::error_code, std::size_t) {});



    if (fileres) {
        std::cout << "📂 File opened successfully\n";
        std::cout << "DEBUG buffer_ size: " << buffer_.size() << "\n";

        auto buffer = asio::buffer(buffer_.data(), buffer_.size());

    }
    else {
        std::string err =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n\r\n";

        asio::async_write(socket_, asio::buffer(err),
            [self = shared_from_this()](asio::error_code, std::size_t) {});
    }
}

bool session::open_file(const std::string& path)
{
    fstream_.open(path, std::ios::binary | std::ios::ate);
    if (!fstream_.is_open()) {
        std::cerr << "❌ Failed to open file: " << path << "\n";
        return false;
    }



    fstream_.seekg(0, std::ios::end);
    auto filesize = fstream_.tellg();
    fstream_.seekg(0, std::ios::beg);

    if (filesize == -1) {
        std::cerr << "❌ Failed to determine file size: " << path << "\n";
        return false;
    }

    std::ostream requestStream(&buffer_);
    std::filesystem::path p(path);
    requestStream << p.filename().string() << "\n" << filesize << "\n\n";

    std::cout << "📦 File open OK: " << p.filename().string() << ", size: " << filesize << "\n";
    return true;
}

void session::do_write_file(const asio::error_code& ec)
{
    if (fstream_)
    {
        fstream_.read(buf_.data(), buf_.size());
        if (fstream_.fail() && !fstream_.eof()) {
            std::cout << "fail\n";
        }

        std::stringstream ss;
        ss << fstream_.gcount() << " bytes, total: " << fstream_.tellg() << " bytes;";
        std::cout << ss.str() << std::endl;

        auto buf = asio::buffer(buf_.data(), static_cast<size_t>(fstream_.gcount()));
        write_buf(buf);
    }
        
}

server::server(asio::io_context& context, unsigned short port)
    : acceptor_(context, tcp::endpoint(tcp::v4(), port)), socket_(context)
{
    do_accept();
}

void server::do_accept()
{
    acceptor_.async_accept(socket_, [this](asio::error_code ec) {
        if (!ec) {
            std::make_shared<session>(std::move(socket_))->start();
        }
        do_accept();
        });
}


void session::do_read_file()
{
    return;
}


void session::do_write_file()
{
    return;
}



http_request::http_request() {}

std::string http_request::get_string() const
{
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: " + std::to_string(body_.size()) + "\n";
        
    for (const auto& x : headers_)
    {
        response.append(x.first + ": " + x.second + "\r\n");
    }
    response.append("\r\n");

    response.append(body_);
    
    return response;
    
}
