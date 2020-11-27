/*
 -----------------------------------------------------------------------------------------------------------------------
 The MIT License (MIT)

 Copyright (c) 2020 Ralph-Gordon Paul. All rights reserved.

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 -----------------------------------------------------------------------------------------------------------------------
*/

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

#include <boost/log/core/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include "RestServer.hpp"

const std::string kAppVersion = "1.0";

// hostname that should be used
std::string serverHost {"0.0.0.0"};

// port that should be used
unsigned short serverPort {8080};

// this will hold all possible program options that can be specified
std::unique_ptr<boost::program_options::options_description> optionsDescription;

// reading in program parameters
void processArgs(int argc, const char** argv);

int main(int argc, const char** argv)
{
    using namespace rgpaul;

    // output some info if the program was started
    std::cout << "Rest Server v" << kAppVersion << std::endl
              << "Copyright (c) 2020 Ralph-Gordon Paul. All rights reserved." << std::endl
              << std::endl;

#if defined(DEBUG)
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
#else
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

    // using boost program_options to parse the program args
    processArgs(argc, argv);

    BOOST_LOG_TRIVIAL(info) << "using hostname: " << serverHost << " and port: " << serverPort;

    auto restServer = std::make_shared<RestServer>(serverHost);

    restServer->startListening();

    return EXIT_SUCCESS;
}

// reading in program parameters
void processArgs(int argc, const char** argv)
{
    optionsDescription =
        std::make_unique<boost::program_options::options_description>("The following parameters are available");

    optionsDescription->add_options()("host,h", boost::program_options::value<std::string>(),
                                      "Specify the hostname that should be used. default: 0.0.0.0")(
        "port,p", boost::program_options::value<std::string>(), "Specify the port that should be used. default: 8080")(
        "help", "Show all available options.");

    boost::program_options::variables_map map;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, *optionsDescription), map);
    boost::program_options::notify(map);

    // if the help option was selected, we just print the available options and exit the program
    if (map.count("help"))
    {
        std::cout << *optionsDescription << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    // if the host parameter was specified, we store that information in the host variable
    if (map.count("host"))
    {
        serverHost = map["host"].as<std::string>();
    }

    // if the port parameter was specified, we store that information in the port variable
    if (map.count("port"))
    {
        serverPort = map["port"].as<unsigned short>();
    }
}
