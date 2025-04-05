#include "iostream"
#include "asio.hpp"
#include "fstream"
#include "filesystem"
#include "string"
#include "asio/ts/buffer.hpp"
#include "asio/read_until.hpp"

using asio::ip::tcp;


class session : public std::enable_shared_from_this<session>
{
private:
	asio::ip::tcp::socket socket_;
	asio::streambuf buffer_;

	const std::string shared_dir = "s";
	

public:
	explicit session(asio::ip::tcp::socket socket);
	//~session();

	void start();

private:
	void do_read();
	void send_file(const std::string& value);

};


class server {
private:
	asio::ip::tcp::socket socket_;
	asio::ip::tcp::acceptor acceptor_;
	
public:
	server(asio::io_context &context, unsigned short port);
	//~server();

	void do_accept();

};