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

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include <rgpaul/Session.hpp>

namespace rgpaul
{
class RestServer : public std::enable_shared_from_this<RestServer>
{
  public:
    RestServer() = delete;
    explicit RestServer(const std::string& host, unsigned short port = 8080);

    void registerEndpoint(const std::string& target,
                          std::function<void(std::shared_ptr<Session>,
                                             const boost::beast::http::request<boost::beast::http::string_body>&)>);

    //! starts listening with given number of threads - this call won't block
    void startListening(unsigned short threads = 1);

  private:
    // the io_context is required for all i/o
    boost::asio::io_context _ioc;

    boost::asio::ip::tcp::endpoint _endpoint;
    boost::asio::ip::tcp::acceptor _acceptor {_ioc};

    // holds all threads that are listening for incoming connections
    std::vector<std::thread> _threads;

    std::unordered_map<std::string,
                       std::function<void(std::shared_ptr<Session>,
                                          const boost::beast::http::request<boost::beast::http::string_body>&)>>
        _callbacks;

    void doAccept();
    void onAccept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

    friend Session;
    void handleRequest(const boost::beast::http::request<boost::beast::http::string_body>& req,
                       std::shared_ptr<Session> session);
};
}  // namespace rgpaul
