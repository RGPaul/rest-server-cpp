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

#include "Session.hpp"

#include <chrono>

#include <boost/log/trivial.hpp>

using namespace rgpaul;

// ---------------------------------------------------------------------------------------------------------------------
// Constructors / Destructor
// ---------------------------------------------------------------------------------------------------------------------

Session::Session(boost::asio::ip::tcp::socket&& socket) : _stream(std::move(socket)) {}

// ---------------------------------------------------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------------------------------------------------

void Session::run()
{
    boost::asio::dispatch(_stream.get_executor(),
                          boost::beast::bind_front_handler(&Session::doRead, shared_from_this()));
}

// ---------------------------------------------------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------------------------------------------------

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

void Session::handleRequest()
{
    // Returns a bad request response
    auto const bad_request = [this](boost::beast::string_view why) {
        boost::beast::http::response<boost::beast::http::string_body> res {boost::beast::http::status::bad_request,
                                                                           _req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(_req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const not_found = [this](boost::beast::string_view target) {
        boost::beast::http::response<boost::beast::http::string_body> res {boost::beast::http::status::not_found,
                                                                           _req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(_req.keep_alive());
        res.body() = "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error = [this](boost::beast::string_view what) {
        boost::beast::http::response<boost::beast::http::string_body> res {
            boost::beast::http::status::internal_server_error, _req.version()};
        res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(boost::beast::http::field::content_type, "text/html");
        res.keep_alive(_req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if (_req.method() != boost::beast::http::verb::get && _req.method() != boost::beast::http::verb::head)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (_req.target().empty() || _req.target()[0] != '/' || _req.target().find("..") != boost::beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    return send(not_found("blubb"));
}

void Session::send(boost::beast::http::response<boost::beast::http::empty_body> res)
{
    // auto sp = std::make_shared<boost::beast::http::message<false, Body, Fields>>(std::move(msg));

    auto r = std::make_shared<boost::beast::http::response<boost::beast::http::empty_body>>(std::move(res));

    // // write the response
    boost::beast::http::async_write(
        _stream, *r, boost::beast::bind_front_handler(&Session::onWrite, shared_from_this(), r->need_eof()));
}

void Session::send(boost::beast::http::response<boost::beast::http::file_body> res) {}

void Session::send(boost::beast::http::response<boost::beast::http::string_body> res)
{
    auto r = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>(std::move(res));
    _res = r;

    // // write the response
    boost::beast::http::async_write(
        _stream, *r, boost::beast::bind_front_handler(&Session::onWrite, shared_from_this(), r->need_eof()));
}
