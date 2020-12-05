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
#define BOOST_TEST_MODULE "RGPRestServer"

#include <rgpaul/RestServer.hpp>

#include <memory>
#include <string>
#include <vector>

// include this last
#include <boost/test/included/unit_test.hpp>

using namespace rgpaul;

BOOST_AUTO_TEST_SUITE(RGPRestServer)

BOOST_AUTO_TEST_CASE(constructor)
{
    BOOST_CHECK_NO_THROW(rgpaul::RestServer("127.0.0.1", 8080));

    std::shared_ptr<RestServer> restServer = std::make_shared<RestServer>("127.0.0.1", 8080);

    BOOST_REQUIRE(restServer);
}

BOOST_AUTO_TEST_CASE(urlencode)
{
    std::string input1 = " @\\%";
    std::string output1 = "%20%40%5C%25";
    BOOST_CHECK_EQUAL(RestServer::urlEncode(input1), output1);

    std::string input2 = "abcdegfhijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    BOOST_CHECK_EQUAL(RestServer::urlEncode(input2), input2);
}

BOOST_AUTO_TEST_CASE(urldecode)
{
    std::string input1 = "%20%40%5C%25";
    std::string output1 = " @\\%";

    BOOST_CHECK_EQUAL(RestServer::urlDecode(input1), output1);

    std::string input2 = "abcdegfhijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    BOOST_CHECK_EQUAL(RestServer::urlDecode(input2), input2);
}

BOOST_AUTO_TEST_SUITE_END()
