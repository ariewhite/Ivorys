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
                        send_file("C:/Users/murzabaev.c/project/Ivorys/sample.txt");
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
    fstream_.open(filename, std::ios::binary | std::ios::ate);

    bool is_open = fstream_.is_open();
    if (!is_open) {
        std::cerr << "❌ Failed to open file: " << filename << "\n";
    }
    // get file size
    fstream_.seekg(0, std::ios::end);
    auto filesize = fstream_.tellg();

    if (filesize == -1) {
        std::cerr << "❌ Failed to determine file size: " << filename << "\n";   
    }
    
    // get hash sum (sha-256)
    auto hash = calculate_sha256(filename);
    
    // get file name
    std::string filenamewithext = "empty.txt";
    size_t pos = filename.find_last_of("/");
    if (pos < filename.length())
        filenamewithext = filename.substr(pos + 1);

    // prepare to first response w/ file info
    http_request hello;
    hello.headers_.emplace("Content-Length", std::to_string(filesize));
    hello.headers_.emplace("Content-Disposition", "attachment");
    hello.headers_.emplace("File-Name", filenamewithext);
    hello.headers_.emplace("SHA-256", hash);

    std::string x = hello.get_string();

    auto self = shared_from_this();
    asio::async_write(socket_,
        asio::buffer(hello.get_string()),
        [this, self ](asio::error_code ec, std::size_t leng) {
            if (ec) {
                std::cerr << "failure to send start request\n";
            }
            
        });

    if (is_open) {
        fstream_.close();
        std::cout << "📂 File opened successfully\n";
            
        try {
            from_file = std::make_unique<asio::stream_file>(socket_.get_executor(), filename, asio::stream_file::read_only);
          /*  from_file.reset(new asio::stream_file(socket_.get_executor()));
            from_file->open(filename, asio::stream_file::read_only);*/
        }
        catch (const std::exception& ptr) {
            std::cerr << "error with from_file: " << ptr.what() << std::endl;
        }
        
        std::cout << "start---\n";
        do_read_file();
    }
    else {
        send_error();
        std::cerr << "sended error to client\n";
    }
}


void session::send_error()
{
    std::string err =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 0\r\n\r\n";

    auto self = shared_from_this();
    asio::async_write(socket_, 
        asio::buffer(err),
        [this, self](asio::error_code ec, std::size_t /*n*/) {
            if (ec) {
                std::cerr << "error to send erros response\n";
            }
        });
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

void session::write_file(const asio::error_code& ec)
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

// ---------------------------------------------------------------------------------------------

void session::do_read_file()
{
    std::cout << "read block\n";
    auto self = shared_from_this();
    from_file->async_read_some(asio::buffer(data_), 
        [this, self](asio::error_code ec, std::size_t length) {
            if (!ec) {
                do_write_file(length);
            }
            else if (ec != asio::error::eof) {
                std::cerr << "error copy file: " << ec.message() << std::endl;
            }
        });
}


void session::do_write_file(size_t length)
{
    //std::cout.write(data_, length);

    /*std::string err =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 0\r\n\r\n";

    asio::async_write(socket_, asio::buffer(err),
        [self = shared_from_this()](asio::error_code, std::size_t) {});*/
    
    auto self = shared_from_this();
    std::cout << "try write block: " << length << "\n";
    asio::async_write(socket_, 
        asio::buffer(data_, length),
        [this, self](std::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                do_read_file();
            }
            else {
                std::cerr << "error copy file - w: " << ec.message() << std::endl;
            }
        });
}


// ---------------------------------------------------------------------------------------------

http_request::http_request() {}

std::string http_request::get_string() const
{
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/octet-stream\r\n";
        
    for (const auto& x : headers_)
    {
        response.append(x.first + ": " + x.second + "\r\n");
        //std::cout << x.first << " ... " << x.second;
    }
    response.append("\r\n");

    response.append(body_ + "\n\n");
    
    return response;
    
}
