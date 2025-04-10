#include "iostream"
#include "asio.hpp"
#include "fstream"
#include "filesystem"
#include "string"
#include "map"
#include "memory"
#include "asio/ts/buffer.hpp"
#include "asio/read_until.hpp"
#include "array"
#include "cstdint"
#include "sstream"
#include "iomanip"
#include "openssl/sha.h"


#ifdef __UNIX
//#include "sys/sendfile.h"
#endif // __UNIX

static std::string calculate_sha256(const std::string& filepath);

using asio::ip::tcp;

class http_request;

class session : public std::enable_shared_from_this<session>
{
private:
	asio::ip::tcp::socket socket_;
	asio::streambuf       buffer_;
	std::ifstream	      fstream_;
	std::unique_ptr<asio::stream_file> from_file;


private:
	//const std::string shared_dir = "s";
	enum { MessageSize = 1024 };
	std::array<char, MessageSize> buf_;
	char data_[4096];

public:
	explicit session(asio::ip::tcp::socket socket);

	~session() = default;

	void start();

private:
	void do_read();
	void send_file(const std::string& value);

	void send_error();
	template<typename Buffer>
	void write_buf(Buffer& buffer);
	bool open_file(std::string const& path);
	void write_file(const asio::error_code& ec);

	void do_read_file();
	void do_write_file(std::size_t length);
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

class http_request 
{
public: 
	explicit http_request();
	
	// headers <header name, content>
	std::map<std::string, std::string> headers_;
	// body
	std::string body_;

	std::string get_string() const;
};


template<typename Buffer>
void session::write_buf(Buffer &buf)
{
	std::cout << "write buf" << "\n";
	auto self = shared_from_this();

	asio::async_write(socket_, 
		buf,
		[this](std::error_code ec, size_t /*length*/) {
			write_file(ec);
		});
}


static std::string calculate_sha256(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		throw std::runtime_error("Could not open file: " + filepath);
	}

	SHA256_CTX sha256;
	SHA256_Init(&sha256);

	char buffer[4096];
	while (file.read(buffer, sizeof(buffer))) {
		SHA256_Update(&sha256, buffer, file.gcount());
	}
	SHA256_Update(&sha256, buffer, file.gcount());

	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_Final(hash, &sha256);

	std::stringstream ss;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
	}

	return ss.str();
}