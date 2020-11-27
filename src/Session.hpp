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

#include <memory>
#include <string>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

namespace rgpaul
{
class RestServer;

class Session : public std::enable_shared_from_this<Session>
{
  public:
    Session() = delete;
    explicit Session(boost::asio::ip::tcp::socket&& socket, std::shared_ptr<RestServer> server);

    void run();

    void sendResponse(const nlohmann::json& data);

    void sendBadRequest(boost::beast::string_view why);
    void sendNotFound(boost::beast::string_view target);
    void sendServerError(boost::beast::string_view what);
    void sendFile(const std::string& path);

  private:
    boost::beast::tcp_stream _stream;
    boost::beast::flat_buffer _buffer;
    boost::beast::http::request<boost::beast::http::string_body> _req;
    std::shared_ptr<void> _res;
    std::weak_ptr<RestServer> _restServer;

    static boost::beast::string_view mimeType(boost::beast::string_view path);

    void doRead();
    void onRead(boost::beast::error_code ec, std::size_t bytes_transferred);
    void doClose();
    void onWrite(bool close, boost::beast::error_code ec, std::size_t bytes_transferred);

    template <bool isRequest, class Body, class Fields>
    void send(boost::beast::http::message<isRequest, Body, Fields>&& response);

    void handleRequest();
};
}  // namespace rgpaul
