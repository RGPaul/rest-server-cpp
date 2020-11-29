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

#include <rgpaul/RestServer.hpp>

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/asio/strand.hpp>
#include <boost/log/trivial.hpp>

#include <rgpaul/Session.hpp>
#include <rgpaul/UriNode.hpp>

using namespace rgpaul;

// ---------------------------------------------------------------------------------------------------------------------
// Constructors / Destructor
// ---------------------------------------------------------------------------------------------------------------------

RestServer::RestServer(const std::string& host, unsigned short port)
{
    _endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host), port);
    boost::system::error_code ec;

    // open the acceptor
    _acceptor.open(_endpoint.protocol(), ec);
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "open: " << ec.message();
        return;
    }

    // allow address reuse
    _acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "set_option: " << ec.message();
        return;
    }

    // bind to the server address
    _acceptor.bind(_endpoint, ec);
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "bind: " << ec.message();
        return;
    }

    // start listening for connections
    _acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "listen: " << ec.message();
        return;
    }

    // init root node for uri
    _registeredEndpoints = std::make_shared<UriNode>();
    _registeredEndpoints->id = "/";
}

// ---------------------------------------------------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------------------------------------------------

void RestServer::registerEndpoint(
    const std::string& target,
    std::function<void(std::shared_ptr<Session>, const boost::beast::http::request<boost::beast::http::string_body>&)>
        callback)
{
    std::vector<std::string> uriPaths = splitUri(target);

    std::shared_ptr<UriNode> node = _registeredEndpoints->createNodeForPath(uriPaths);

    if (node)
        node->callback = std::move(callback);
}

void RestServer::startListening(unsigned short threads)
{
    if (!_acceptor.is_open())
    {
        BOOST_LOG_TRIVIAL(error) << "Error start listening. Acceptor is not open.";
        return;
    }
    doAccept();

    // reserve space for the number of threads
    _threads.reserve(threads);

    // start the amount of given threads and run ioc for each of them
    for (auto i = 0; i < threads; ++i) _threads.emplace_back([this] { _ioc.run(); });
}

std::vector<std::string> RestServer::splitUri(std::string uri)
{
    std::vector<std::string> container;

    // cut get params
    std::size_t pos = uri.find('?');
    if (pos != std::string::npos)
        uri = uri.substr(0, pos);

    boost::split(container, uri, boost::is_any_of("/"));

    // we replace the empty first entry with "/"
    if (container.size() > 0 && container.front().empty())
        container[0] = "/";

    return container;
}

// ---------------------------------------------------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------------------------------------------------

void RestServer::doAccept()
{
    // the new connection gets its own strand
    _acceptor.async_accept(boost::asio::make_strand(_ioc),
                           boost::beast::bind_front_handler(&RestServer::onAccept, shared_from_this()));
}

void RestServer::onAccept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
{
    if (ec)
    {
        BOOST_LOG_TRIVIAL(error) << "accept: " << ec.message();
    }
    else
    {
        // create the session and run it
        BOOST_LOG_TRIVIAL(info) << "server accepted incoming connection.";
        std::make_shared<Session>(std::move(socket), shared_from_this())->run();
    }

    // accept another connection
    doAccept();
}

void RestServer::handleRequest(const boost::beast::http::request<boost::beast::http::string_body>& request,
                               std::shared_ptr<Session> session)
{
    if (!session)
    {
        BOOST_LOG_TRIVIAL(error) << "RestServer handle request got empty session.";
        return;
    }

    // request path must be absolute and not contain "..".
    if (request.target().empty() || request.target()[0] != '/'
        || request.target().find("..") != boost::beast::string_view::npos)
    {
        session->sendBadRequest("Illegal request-target");
        return;
    }

    std::vector<std::string> uriPaths = splitUri(std::string(request.target()));

    std::shared_ptr<UriNode> node = _registeredEndpoints->findNodeForPath(uriPaths);

    if (!node)
    {
        session->sendNotFound(request.target());
        return;
    }

    auto& callback = node->callback;
    if (callback)
    {
        callback(session, request);
    }
}
