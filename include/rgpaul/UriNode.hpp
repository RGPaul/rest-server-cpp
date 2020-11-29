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
#include <unordered_map>

#include <boost/beast/http.hpp>

#include <rgpaul/Session.hpp>

namespace rgpaul
{
struct UriNode
{
    std::string id;
    std::function<void(std::shared_ptr<Session>, const boost::beast::http::request<boost::beast::http::string_body>&)>
        callback;

    std::weak_ptr<UriNode> parent;
    std::unordered_map<std::string, std::shared_ptr<UriNode>> children;

    //! creates a new entry in the tree - must be called on the root node
    std::shared_ptr<UriNode> createNodeForPath(const std::vector<std::string>& uri);

    //! finds the node for the given uri path - must be called on the root node
    std::shared_ptr<UriNode> findNodeForPath(const std::vector<std::string>& uri);
};
}  // namespace rgpaul
