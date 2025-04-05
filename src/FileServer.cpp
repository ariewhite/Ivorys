#include "FileServer.h"


session::session(asio::ip::tcp::socket socket) 
	: socket_(std::move(socket))
{

}


void session::start()
{
	do_read();
}


void session::do_read()
{
	auto self(shared_from_this());
	socket_.async_read_some(buffer(buffer_.prepare(1024)),
		[this, self](asio::error_code ec, std::size_t length)
		{
			if (!ec) {
				buffer_.commit(length);
				std::istream istream(&buffer_);
				std::string method, path, version;
				istream >> method >> path >> version;

				std::cout << "accept new data: method: " << method << std::endl;
				std::string filename = path.substr(1);
				send_file(filename);
			}

			else if (ec != asio::error::eof)
			{
				std::cerr << "Error copying file: " << ec.message() << "\n";
			}
			 
		});
}

void session::send_file(const std::string &value)
{
	//std::istream file(shared_dir + "/" + value, std::ios::binary);

	auto self(shared_from_this());

	asio::async_write(socket_, asio::buffer(value),
		[this, self] (asio::error_code ec, std::size_t length) mutable {
			if (!ec)
			{
				std::cout << "file sended\n";
			}
		});
}



server::server(asio::io_context &context, unsigned short port)
	: 
	acceptor_(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
	socket_(context)
{
	do_accept();
}

void server::do_accept()
{
	acceptor_.async_accept(socket_, [this](asio::error_code ec) {
		if (!ec)
		{
			std::make_shared<session>(std::move(socket_))->start();
		}
		do_accept();
	});

}