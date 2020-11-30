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

#include <rgpaul/UriNode.hpp>

using namespace rgpaul;

// ---------------------------------------------------------------------------------------------------------------------
// Constructors / Destructor
// ---------------------------------------------------------------------------------------------------------------------

UriNode::UriNode(const std::string& id) : _id(id) {}

// ---------------------------------------------------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------------------------------------------------

std::string UriNode::id() const
{
    return _id;
}

std::function<void(std::shared_ptr<Session>, const boost::beast::http::request<boost::beast::http::string_body>&)>
UriNode::callback() const
{
    return _callback;
}

void UriNode::setCallback(
    std::function<void(std::shared_ptr<Session>, const boost::beast::http::request<boost::beast::http::string_body>&)>
        callback)
{
    _callback = std::move(callback);
}

// ---------------------------------------------------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------------------------------------------------

std::shared_ptr<UriNode> UriNode::createRootNode()
{
    return std::make_shared<UriNode>("/");
}

std::shared_ptr<UriNode> UriNode::createNodeForPath(const std::vector<std::string>& uri)
{
    std::shared_ptr<UriNode> node;

    // uri has to have entries
    if (uri.size() <= 1)
        return node;

    // first entry must match the root node
    if (uri.front() != _id)
        return node;

    std::shared_ptr<UriNode> currentNode = findChildNodeWithId(uri.at(1));
    if (!currentNode)
        currentNode = createChildNodeWithId(uri.at(1));

    // check if we have to create more nodes
    if (uri.size() >= 2)
    {
        // iterate over all path entries and search for the items
        for (auto it = uri.begin() + 2; it != uri.end(); it++)
        {
            std::shared_ptr<UriNode> child = currentNode->findChildNodeWithId(*it);
            if (child)
                currentNode = child;
            else
                currentNode = currentNode->createChildNodeWithId(*it);
        }
    }

    // if the currentNode contains the id of the last uri - we created sucessfully the new node for the given path
    if (currentNode->id() == uri.back())
        node = currentNode;

    return node;
}

std::shared_ptr<UriNode> UriNode::findNodeForPath(const std::vector<std::string>& uri)
{
    std::shared_ptr<UriNode> node;

    // uri has to have entries
    if (uri.size() <= 0)
        return node;

    // first entry must match the root node
    if (uri.front() != _id)
        return node;

    // if it is only the first entry, we return ourself
    if (uri.size() == 1)
        return shared_from_this();

    std::shared_ptr<UriNode> currentNode = findChildNodeWithId(uri.at(1));
    if (!currentNode)
        return node;

    // check if we have to search some more
    if (uri.size() >= 2)
    {
        // iterate over all path entries and search for the nodes
        for (auto it = uri.begin() + 2; it != uri.end(); it++)
        {
            std::shared_ptr<UriNode> child = currentNode->findChildNodeWithId(*it);
            if (child)
                currentNode = child;
            else
                return node;  // not found => no node for the path or part of the path
        }
    }

    // if the currentNode contains the id of the last uri - we sucessfully found the node for the given path
    if (currentNode->id() == uri.back())
        node = currentNode;

    return node;
}

// ---------------------------------------------------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------------------------------------------------

std::shared_ptr<UriNode> UriNode::findChildNodeWithId(const std::string& childId) const
{
    std::shared_ptr<UriNode> node;

    auto search = _children.find(childId);
    if (search != _children.end())
        node = search->second;

    return node;
}

std::shared_ptr<UriNode> UriNode::createChildNodeWithId(const std::string& nodeId)
{
    std::shared_ptr<UriNode> node = std::make_shared<UriNode>(nodeId);

    _children.insert({nodeId, node});

    return node;
}
