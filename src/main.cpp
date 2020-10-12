#include "http_server.h"
#include <iostream>
#include <boost/program_options.hpp>

// use getopt instead ?
namespace opt = boost::program_options;

int main(int argc, char *argv[])
{  
	opt::options_description opt_desc("Available options");

	opt_desc.add_options()
		("ip,h", opt::value<std::string>(), "ip to listen on")
		("port,p", opt::value<std::string>(), "port number")
    	("directory,d", opt::value<std::string>(), "server root directory");

	opt::variables_map vmap;
    opt::store(opt::parse_command_line(argc, argv, opt_desc), vmap);
    opt::notify(vmap);

	const auto directory = vmap["directory"].as<std::string>();
    const auto ip = vmap["ip"].as<std::string>();
    const auto port = vmap["port"].as<std::string>();

	const unsigned n_threads = 5; 
    HttpServer server{ directory, ip, port, n_threads };
    server.run();
}