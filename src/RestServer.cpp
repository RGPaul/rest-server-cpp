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
    _registeredEndpoints = UriNode::createRootNode();
}

// ---------------------------------------------------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------------------------------------------------

void RestServer::registerEndpoint(const std::string& target, RestServerCallback callback)
{
    // check if it is the root element - we can assign the callback directly
    if (target == "/")
    {
        _registeredEndpoints->setCallback(std::move(callback));
        return;
    }

    // split the uri path
    std::vector<std::string> uriPaths = splitUri(target);

    // create a node for the path
    std::shared_ptr<UriNode> node = _registeredEndpoints->createNodeForPath(uriPaths);

    // if the node could be created, we assign the callback to it
    if (node)
        node->setCallback(std::move(callback));
}

void RestServer::startListening(unsigned short threads)
{
    if (!_acceptor.is_open())
    {
        BOOST_LOG_TRIVIAL(error) << "Error start listening. Acceptor is not open.";
        return;
    }

    // accept incoming connections
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
    {
        container[0] = "/";

        // remove trailing empty entry
        if (container.back().empty())
            container.pop_back();
    }

    return container;
}

std::string RestServer::urlEncode(const std::string& url)
{
    // only alphanum are safe characters
    const char kSafe[256] = {/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
                             /* 0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* 1 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* 2 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* 3 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,

                             /* 4 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                             /* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
                             /* 6 */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                             /* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,

                             /* 8 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* 9 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* A */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* B */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

                             /* C */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* D */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* E */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* F */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const char kDec2hex[16 + 1] = "0123456789ABCDEF";
    const unsigned char* source = reinterpret_cast<const unsigned char*>(url.c_str());
    const int kSourceLength = url.length();
    // we allocate a new char* that has space for tripple the amount of characters
    unsigned char* const pStart = new unsigned char[kSourceLength * 3];
    unsigned char* pEnd = pStart;
    const unsigned char* const kSourceEnd = source + kSourceLength;

    for (; source < kSourceEnd; ++source)
    {
        // check if this character is a safe one (that don't needs to be escaped)
        if (kSafe[*source])
            *pEnd++ = *source;
        else
        {
            *pEnd++ = '%';
            *pEnd++ = kDec2hex[*source >> 4];
            *pEnd++ = kDec2hex[*source & 0x0F];
        }
    }

    std::string result(reinterpret_cast<char*>(pStart), reinterpret_cast<char*>(pEnd));
    delete[] pStart;
    return result;
}

std::string RestServer::urlDecode(const std::string& url)
{
    // Note from RFC1630: "Sequences which start with a percent
    // sign but are not followed by two hexadecimal characters
    // (0-9, A-F) are reserved for future extension"

    const char kHex2Dec[256] = {/*       0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
                                /* 0 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* 1 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* 2 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* 3 */ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1,

                                /* 4 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* 5 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* 6 */ -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* 7 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

                                /* 8 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* 9 */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* A */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* B */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

                                /* C */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* D */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* E */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                /* F */ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    const unsigned char* source = reinterpret_cast<const unsigned char*>(url.c_str());
    const int kSourceLength = url.length();
    const unsigned char* const kSourceEnd = source + kSourceLength;

    // last decodable '%' if there is one
    const unsigned char* const kSourceLastDec = kSourceEnd - 2;

    // we allocate a new char* for the decoded characters
    char* const pStart = new char[kSourceLength];
    char* pEnd = pStart;

    while (source < kSourceLastDec)
    {
        if (*source == '%')
        {
            char dec1, dec2;
            if (-1 != (dec1 = kHex2Dec[*(source + 1)]) && -1 != (dec2 = kHex2Dec[*(source + 2)]))
            {
                *pEnd++ = (dec1 << 4) + dec2;
                source += 3;
                continue;
            }
        }

        *pEnd++ = *source++;
    }

    // the last 2 characters
    while (source < kSourceEnd) *pEnd++ = *source++;

    std::string result(pStart, pEnd);
    delete[] pStart;
    return result;
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

    std::string target = std::string(request.target());

    // request path must be absolute and not contain "..".
    if (target.empty() || target[0] != '/' || target.find("..") != boost::beast::string_view::npos)
    {
        session->sendBadRequest("Illegal request-target");
        return;
    }

    // split the target
    std::vector<std::string> uriPaths = splitUri(target);

    // find the node for the given target
    std::shared_ptr<UriNode> node = _registeredEndpoints->findNodeForPath(uriPaths);

    // if there is no node or no callback for the node, we send a not found
    if (!node || !node->callback())
    {
        session->sendNotFound(target);
        return;
    }

    // call the callback for the found node
    const auto& callback = node->callback();
    callback(session, request);
}
