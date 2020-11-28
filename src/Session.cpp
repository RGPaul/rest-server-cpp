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

#include <rgpaul/Session.hpp>

#include <chrono>

#include <boost/log/trivial.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/beast/version.hpp>

#include <rgpaul/RestServer.hpp>

using namespace rgpaul;

// ---------------------------------------------------------------------------------------------------------------------
// Constructors / Destructor
// ---------------------------------------------------------------------------------------------------------------------

Session::Session(boost::asio::ip::tcp::socket&& socket, std::shared_ptr<RestServer> server)
    : _stream(std::move(socket)), _restServer(server)
{
}

// ---------------------------------------------------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------------------------------------------------

void Session::run()
{
    boost::asio::dispatch(_stream.get_executor(),
                          boost::beast::bind_front_handler(&Session::doRead, shared_from_this()));
}

void Session::sendResponse(const nlohmann::json& data)
{
    boost::beast::http::response<boost::beast::http::string_body> response {boost::beast::http::status::ok,
                                                                            _req.version()};

    response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(boost::beast::http::field::content_type, "application/json");
    response.keep_alive(_req.keep_alive());
    response.body() = data.dump();
    response.prepare_payload();

    send(std::move(response));
}

void Session::sendBadRequest(boost::beast::string_view why)
{
    nlohmann::json message = {{"error", std::string(why)}};
    boost::beast::http::response<boost::beast::http::string_body> response {boost::beast::http::status::bad_request,
                                                                            _req.version()};
    response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(boost::beast::http::field::content_type, "application/json");
    response.keep_alive(_req.keep_alive());
    response.body() = message.dump();
    response.prepare_payload();

    send(std::move(response));
}

void Session::sendNotFound(boost::beast::string_view target)
{
    nlohmann::json message = {{"error", "The resource '" + std::string(target) + "' was not found."}};
    boost::beast::http::response<boost::beast::http::string_body> response {boost::beast::http::status::not_found,
                                                                            _req.version()};
    response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(boost::beast::http::field::content_type, "application/json");
    response.keep_alive(_req.keep_alive());
    response.body() = message.dump();
    response.prepare_payload();

    send(std::move(response));
}

void Session::sendServerError(boost::beast::string_view what)
{
    nlohmann::json message = {{"error", "An error occurred: '" + std::string(what) + "'"}};
    boost::beast::http::response<boost::beast::http::string_body> response {
        boost::beast::http::status::internal_server_error, _req.version()};
    response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(boost::beast::http::field::content_type, "application/json");
    response.keep_alive(_req.keep_alive());
    response.body() = message.dump();
    response.prepare_payload();

    send(std::move(response));
}

void Session::sendFile(const std::string& path)
{
    // attempt to open the file
    boost::beast::error_code ec;
    boost::beast::http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == boost::beast::errc::no_such_file_or_directory)
        return sendNotFound(_req.target());

    // handle an unknown error
    if (ec)
        return sendServerError(ec.message());

    // Cache the size since we need it after the move
    auto const size = body.size();

    // respond to HEAD request
    if (_req.method() == boost::beast::http::verb::head)
    {
        boost::beast::http::response<boost::beast::http::empty_body> response {boost::beast::http::status::ok,
                                                                               _req.version()};
        response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        response.set(boost::beast::http::field::content_type, mimeType(path));
        response.content_length(size);
        response.keep_alive(_req.keep_alive());
        return send(std::move(response));
    }

    // respond to GET request
    boost::beast::http::response<boost::beast::http::file_body> response {
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(boost::beast::http::status::ok, _req.version())};
    response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
    response.set(boost::beast::http::field::content_type, mimeType(path));
    response.content_length(size);
    response.keep_alive(_req.keep_alive());
    return send(std::move(response));
}

// ---------------------------------------------------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------------------------------------------------

boost::beast::string_view Session::mimeType(boost::beast::string_view path)
{
    using boost::beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == boost::beast::string_view::npos)
            return boost::beast::string_view {};
        return path.substr(pos);
    }();

    if (iequals(ext, ".htm"))
        return "text/html";
    if (iequals(ext, ".html"))
        return "text/html";
    if (iequals(ext, ".php"))
        return "text/html";
    if (iequals(ext, ".css"))
        return "text/css";
    if (iequals(ext, ".txt"))
        return "text/plain";
    if (iequals(ext, ".js"))
        return "application/javascript";
    if (iequals(ext, ".json"))
        return "application/json";
    if (iequals(ext, ".xml"))
        return "application/xml";
    if (iequals(ext, ".swf"))
        return "application/x-shockwave-flash";
    if (iequals(ext, ".flv"))
        return "video/x-flv";
    if (iequals(ext, ".png"))
        return "image/png";
    if (iequals(ext, ".jpe"))
        return "image/jpeg";
    if (iequals(ext, ".jpeg"))
        return "image/jpeg";
    if (iequals(ext, ".jpg"))
        return "image/jpeg";
    if (iequals(ext, ".gif"))
        return "image/gif";
    if (iequals(ext, ".bmp"))
        return "image/bmp";
    if (iequals(ext, ".ico"))
        return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff"))
        return "image/tiff";
    if (iequals(ext, ".tif"))
        return "image/tiff";
    if (iequals(ext, ".svg"))
        return "image/svg+xml";
    if (iequals(ext, ".svgz"))
        return "image/svg+xml";

    return "application/text";
}

void Session::doRead()
{
    // make the request empty before reading
    _req = {};

    // set the timeout
    _stream.expires_after(std::chrono::seconds(30));

    // read a request
    boost::beast::http::async_read(_stream, _buffer, _req,
                                   boost::beast::bind_front_handler(&Session::onRead, shared_from_this()));
}

void Session::onRead(boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // this means they closed the connection
    if (ec == boost::beast::http::error::end_of_stream)
        return doClose();

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "read: " << ec.message();
        return;
    }

    // process request and send response
    handleRequest();

    // send the response
    // handle_request(*doc_root_, std::move(req_), lambda_);
}

void Session::doClose()
{
    // send a tcp shutdown
    boost::beast::error_code ec;
    _stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

    // at this point the connection is closed
    BOOST_LOG_TRIVIAL(info) << "closed connection";
}

void Session::onWrite(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "write: " << ec.message();
        return;
    }

    if (close)
    {
        // this means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        return doClose();
    }

    // we are done with the response
    _res = nullptr;

    // read another request
    doRead();
}

template <bool isRequest, class Body, class Fields>
void Session::send(boost::beast::http::message<isRequest, Body, Fields>&& response)
{
    auto res = std::make_shared<boost::beast::http::message<isRequest, Body, Fields>>(std::move(response));
    _res = res;

    // write the response
    boost::beast::http::async_write(
        _stream, *res, boost::beast::bind_front_handler(&Session::onWrite, shared_from_this(), res->need_eof()));
}

void Session::handleRequest()
{
    std::shared_ptr<RestServer> restServer = _restServer.lock();

    if (restServer)
    {
        restServer->handleRequest(_req, shared_from_this());
    }
}
