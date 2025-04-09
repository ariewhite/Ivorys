#include "Ivorys.h"
//#include "Echo.h"
#include "FileServer.h"

using namespace std;

int main(int argc, char** argv)
{
	try {
		/*if (argc != 2)
		{
			std::cerr << "echo <port>" << std::endl;
			return 1;
		}*/
		asio::io_context context;
		server ser(context, /*std::atoi(argv[1])*/ 8080);

		context.run();
	}
	catch (std::exception& e) {
		std::cerr << "e: " << e.what() << std::endl;
	}

	return 0;
}
