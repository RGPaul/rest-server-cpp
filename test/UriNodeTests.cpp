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

// define the module name (prints at testing)
#define BOOST_TEST_MODULE "RGPUriNode"

#include <rgpaul/UriNode.hpp>

#include <memory>
#include <string>
#include <vector>

// include this last
#include <boost/test/included/unit_test.hpp>

using namespace rgpaul;

BOOST_AUTO_TEST_SUITE(RGPUriNode)

BOOST_AUTO_TEST_CASE(constructor)
{
    BOOST_CHECK_NO_THROW(rgpaul::UriNode {});

    std::shared_ptr<UriNode> rootNode = UriNode::createRootNode();

    BOOST_REQUIRE(rootNode);
    BOOST_REQUIRE_EQUAL(rootNode->id, "/");
}

BOOST_AUTO_TEST_CASE(create)
{
    std::shared_ptr<UriNode> rootNode = UriNode::createRootNode();
    std::shared_ptr<UriNode> node;

    std::vector<std::string> path1 {"fail"};
    std::vector<std::string> path2 {"fail2", "fail2"};
    std::vector<std::string> path3 {"/"};
    std::vector<std::string> path4 {"/", "test1"};
    std::vector<std::string> path5 {"/", "test1", "test2", "test3"};

    node = rootNode->createNodeForPath(path1);
    BOOST_CHECK(!node);

    node = rootNode->createNodeForPath(path2);
    BOOST_CHECK(!node);

    node = rootNode->createNodeForPath(path3);
    BOOST_CHECK(!node);

    node = rootNode->createNodeForPath(path4);
    BOOST_REQUIRE(node);
    BOOST_CHECK_EQUAL(node->id, "test1");

    node = rootNode->createNodeForPath(path5);
    BOOST_REQUIRE(node);
    BOOST_CHECK_EQUAL(node->id, "test3");
}

BOOST_AUTO_TEST_CASE(find)
{
    std::shared_ptr<UriNode> rootNode = UriNode::createRootNode();
    std::shared_ptr<UriNode> node1;
    std::shared_ptr<UriNode> node2;

    std::vector<std::string> path1 {"/", "test1"};
    std::vector<std::string> path2 {"/", "test1", "test2", "test3"};

    node1 = rootNode->createNodeForPath(path1);
    BOOST_REQUIRE(node1);
    BOOST_CHECK_EQUAL(node1->id, "test1");

    node2 = rootNode->findNodeForPath(path1);
    BOOST_REQUIRE(node2);
    BOOST_CHECK_EQUAL(node1, node2);

    node2.reset();
    
    node2 = rootNode->createNodeForPath(path2);
    BOOST_REQUIRE(node2);
    BOOST_CHECK_EQUAL(node2->id, "test3");
}

BOOST_AUTO_TEST_SUITE_END()