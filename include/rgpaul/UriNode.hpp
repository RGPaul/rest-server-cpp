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

#include <rgpaul/RestServer.hpp>

namespace rgpaul
{
class UriNode : public std::enable_shared_from_this<UriNode>
{
  public:
    UriNode() = delete;
    UriNode(const std::string& id);

    std::string id() const;

    RestServerCallback callback() const;
    void setCallback(RestServerCallback callback);

    //! creates a root node - a node with "/" as id
    static std::shared_ptr<UriNode> createRootNode();

    //! creates a new entry in the tree - must be called on the root node
    std::shared_ptr<UriNode> createNodeForPath(const std::vector<std::string>& uri);

    //! finds the node for the given uri path - must be called on the root node
    std::shared_ptr<UriNode> findNodeForPath(const std::vector<std::string>& uri);

  private:
    std::string _id;
    RestServerCallback _callback;

    std::weak_ptr<UriNode> _parent;
    std::unordered_map<std::string, std::shared_ptr<UriNode>> _children;

    //! searches for a child with the given id (won't search the childs of a child)
    std::shared_ptr<UriNode> findChildNodeWithId(const std::string& childId) const;

    //! creates a child node with the given id (will be added to children)
    std::shared_ptr<UriNode> createChildNodeWithId(const std::string& nodeId);
};
}  // namespace rgpaul
