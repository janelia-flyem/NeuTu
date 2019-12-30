//
//  HTTPRequest
//

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

/*
  Copyright (c) 2017, Elviss Strazdiņš
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <system_error>
#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <cctype>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  undef NOMINMAX
#  undef WIN32_LEAN_AND_MEAN
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <unistd.h>
#  include <errno.h>
#endif

namespace http
{
#ifdef _WIN32
    class WinSock final
    {
    public:
        WinSock()
        {
            WSADATA wsaData;
            int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (error != 0)
                throw std::system_error(error, std::system_category(), "WSAStartup failed");

            if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
                throw std::runtime_error("Invalid WinSock version");

            started = true;
        }

        ~WinSock()
        {
            if (started) WSACleanup();
        }

        WinSock(const WinSock&) = delete;
        WinSock& operator=(const WinSock&) = delete;

        WinSock(WinSock&& other):
            started(other.started)
        {
            other.started = false;
        }

        WinSock& operator=(WinSock&& other)
        {
            if (&other != this)
            {
                if (started) WSACleanup();
                started = other.started;
                other.started = false;
            }

            return *this;
        }

    private:
        bool started = false;
    };
#endif

    inline int getLastError()
    {
#ifdef _WIN32
        return WSAGetLastError();
#else
        return errno;
#endif
    }

    enum class InternetProtocol
    {
        V4,
        V6
    };

    inline int getAddressFamily(InternetProtocol internetProtocol)
    {
        switch (internetProtocol)
        {
            case InternetProtocol::V4: return AF_INET;
            case InternetProtocol::V6: return AF_INET6;
        }
    }

    class Socket final
    {
    public:
        Socket(InternetProtocol internetProtocol):
            endpoint(socket(getAddressFamily(internetProtocol), SOCK_STREAM, IPPROTO_TCP))
        {
#ifdef _WIN32
            if (endpoint == INVALID_SOCKET)
                throw std::system_error(WSAGetLastError(), std::system_category(), "Failed to create socket");
#else
            if (endpoint == -1)
                throw std::system_error(errno, std::system_category(), "Failed to create socket");
#endif
        }

#ifdef _WIN32
        Socket(SOCKET s):
            endpoint(s)
        {
        }
#else
        Socket(int s):
            endpoint(s)
        {
        }
#endif
        ~Socket()
        {
#ifdef _WIN32
            if (endpoint != INVALID_SOCKET) closesocket(endpoint);
#else
            if (endpoint != -1) close(endpoint);
#endif
        }

        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        Socket(Socket&& other):
#ifdef _WIN32
            winSock(std::move(other.winSock)),
#endif
            endpoint(other.endpoint)
        {
#ifdef _WIN32
            other.endpoint = INVALID_SOCKET;
#else
            other.endpoint = -1;
#endif
        }

        Socket& operator=(Socket&& other)
        {
            if (&other != this)
            {
#ifdef _WIN32
                winSock = std::move(other.winSock);
                if (endpoint != INVALID_SOCKET) closesocket(endpoint);
#else
                if (endpoint != -1) close(endpoint);
#endif

                endpoint = other.endpoint;

#ifdef _WIN32
                other.endpoint = INVALID_SOCKET;
#else
                other.endpoint = -1;
#endif
            }

            return *this;
        }

#ifdef _WIN32
        operator SOCKET() const { return endpoint; }
#else
        operator int() const { return endpoint; }
#endif

    private:
#ifdef _WIN32
        WinSock winSock;
        SOCKET endpoint = INVALID_SOCKET;
#else
        int endpoint = -1;
#endif
    };

    inline std::string urlEncode(const std::string& str)
    {
        static const char hexChars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

        std::string result;

        for (auto i = str.begin(); i != str.end(); ++i)
        {
            uint8_t cp = *i & 0xFF;

            if ((cp >= 0x30 && cp <= 0x39) || // 0-9
                (cp >= 0x41 && cp <= 0x5A) || // A-Z
                (cp >= 0x61 && cp <= 0x7A) || // a-z
                cp == 0x2D || cp == 0x2E || cp == 0x5F) // - . _
                result += static_cast<char>(cp);
            else if (cp <= 0x7F) // length = 1
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
            else if ((cp >> 5) == 0x6) // length = 2
            {
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
                if (++i == str.end()) break;
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
            }
            else if ((cp >> 4) == 0xe) // length = 3
            {
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
                if (++i == str.end()) break;
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
                if (++i == str.end()) break;
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
            }
            else if ((cp >> 3) == 0x1e) // length = 4
            {
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
                if (++i == str.end()) break;
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
                if (++i == str.end()) break;
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
                if (++i == str.end()) break;
                result += std::string("%") + hexChars[(*i & 0xF0) >> 4] + hexChars[*i & 0x0F];
            }
        }

        return result;
    }

    struct Response
    {
        int code = 0;
        std::vector<std::string> headers;
        std::vector<uint8_t> body;
    };

    class Request final
    {
    public:
        Request(const std::string& url, InternetProtocol protocol = InternetProtocol::V4):
            internetProtocol(protocol)
        {
            size_t schemeEndPosition = url.find("://");

            if (schemeEndPosition != std::string::npos)
            {
                scheme = url.substr(0, schemeEndPosition);
                std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);

                std::string::size_type pathPosition = url.find('/', schemeEndPosition + 3);

                if (pathPosition == std::string::npos)
                {
                    domain = url.substr(schemeEndPosition + 3);
                    path = "/";
                }
                else
                {
                    domain = url.substr(schemeEndPosition + 3, pathPosition - schemeEndPosition - 3);
                    path = url.substr(pathPosition);
                }

                std::string::size_type portPosition = domain.find(':');

                if (portPosition != std::string::npos)
                {
                    port = domain.substr(portPosition + 1);
                    domain.resize(portPosition);
                }
                else
                    port = "80";
            }
        }

        Response send(const std::string& method,
                      const std::map<std::string, std::string>& parameters,
                      const std::vector<std::string>& headers = {})
        {
            std::string body;
            bool first = true;

            for (const auto& parameter : parameters)
            {
                if (!first) body += "&";
                first = false;

                body += urlEncode(parameter.first) + "=" + urlEncode(parameter.second);
            }

            return send(method, body, headers);
        }

        Response send(const std::string& method = "GET",
                      const std::string& body = "",
                      const std::vector<std::string>& headers = {})
        {
            Response response;

            if (scheme != "http")
                throw std::runtime_error("Only HTTP scheme is supported");

            addrinfo hints;
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            hints.ai_flags = AI_DEFAULT;
#else
            hints.ai_flags = (AI_V4MAPPED | AI_ADDRCONFIG);
#endif
            hints.ai_family = getAddressFamily(internetProtocol);
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = 0;
            hints.ai_addrlen = 0;
            hints.ai_addr = nullptr;
            hints.ai_canonname = nullptr;
            hints.ai_next = nullptr;

            addrinfo* info;
            if (getaddrinfo(domain.c_str(), port.c_str(), &hints, &info) != 0)
                throw std::system_error(getLastError(), std::system_category(), "Failed to get address info of " + domain);

            Socket socket(internetProtocol);

            // take the first address from the list
            if (::connect(socket, info->ai_addr, info->ai_addrlen) < 0)
            {
                freeaddrinfo(info);
                throw std::system_error(getLastError(), std::system_category(), "Failed to connect to " + domain + ":" + port);
            }

            freeaddrinfo(info);

            std::string requestData = method + " " + path + " HTTP/1.1\r\n";

            for (const std::string& header : headers)
                requestData += header + "\r\n";

            requestData += "Host: " + domain + "\r\n";
            requestData += "Content-Length: " + std::to_string(body.size()) + "\r\n";

            requestData += "\r\n";
            requestData += body;

#if defined(__APPLE__) || defined(_WIN32)
            int flags = SO_NOSIGPIPE;
#else
            int flags = MSG_NOSIGNAL;
#endif

#ifdef _WIN32
            int remaining = static_cast<int>(requestData.size());
            int sent = 0;
            int size;
#else
            ssize_t remaining = static_cast<ssize_t>(requestData.size());
            ssize_t sent = 0;
            ssize_t size;
#endif

            do
            {
                size = ::send(socket, requestData.data() + sent, static_cast<size_t>(remaining), flags);

                if (size < 0)
                    throw std::system_error(getLastError(), std::system_category(), "Failed to send data to " + domain + ":" + port);

                remaining -= size;
                sent += size;
            }
            while (remaining > 0);

            uint8_t TEMP_BUFFER[65536];
            static const uint8_t clrf[] = {'\r', '\n'};
            std::vector<uint8_t> responseData;
            bool firstLine = true;
            bool parsedHeaders = false;
            int contentSize = -1;
            bool chunkedResponse = false;
            size_t expectedChunkSize = 0;
            bool removeCLRFAfterChunk = false;

            do
            {
                size = recv(socket, reinterpret_cast<char*>(TEMP_BUFFER), sizeof(TEMP_BUFFER), flags);

                if (size < 0)
                    throw std::system_error(getLastError(), std::system_category(), "Failed to read data from " + domain + ":" + port);
                else if (size == 0)
                    break; // disconnected

                responseData.insert(responseData.end(), TEMP_BUFFER, TEMP_BUFFER + size);

                if (!parsedHeaders)
                {
                    for (;;)
                    {
                        auto i = std::search(responseData.begin(), responseData.end(), std::begin(clrf), std::end(clrf));

                        // didn't find a newline
                        if (i == responseData.end()) break;

                        std::string line(responseData.begin(), i);
                        responseData.erase(responseData.begin(), i + 2);

                        // empty line indicates the end of the header section
                        if (line.empty())
                        {
                            parsedHeaders = true;
                            break;
                        }
                        else if (firstLine) // first line
                        {
                            firstLine = false;

                            std::string::size_type pos, lastPos = 0, length = line.length();
                            std::vector<std::string> parts;

                            // tokenize first line
                            while (lastPos < length + 1)
                            {
                                pos = line.find(' ', lastPos);
                                if (pos == std::string::npos) pos = length;

                                if (pos != lastPos)
                                    parts.push_back(std::string(line.data() + lastPos,
                                                                static_cast<std::vector<std::string>::size_type>(pos) - lastPos));

                                lastPos = pos + 1;
                            }

                            if (parts.size() >= 2)
                                response.code = std::stoi(parts[1]);
                        }
                        else // headers
                        {
                            response.headers.push_back(line);

                            std::string::size_type pos = line.find(':');

                            if (pos != std::string::npos)
                            {
                                std::string headerName = line.substr(0, pos);
                                std::string headerValue = line.substr(pos + 1);

                                // ltrim
                                headerValue.erase(headerValue.begin(),
                                                  std::find_if(headerValue.begin(), headerValue.end(),
                                                               std::not1(std::ptr_fun<int, int>(std::isspace))));

                                // rtrim
                                headerValue.erase(std::find_if(headerValue.rbegin(), headerValue.rend(),
                                                               std::not1(std::ptr_fun<int, int>(std::isspace))).base(),
                                                  headerValue.end());

                                if (headerName == "Content-Length")
                                    contentSize = std::stoi(headerValue);
                                else if (headerName == "Transfer-Encoding" && headerValue == "chunked")
                                    chunkedResponse = true;
                            }
                        }
                    }
                }

                if (parsedHeaders)
                {
                    if (chunkedResponse)
                    {
                        bool dataReceived = false;
                        for (;;)
                        {
                            if (expectedChunkSize > 0)
                            {
                                auto toWrite = std::min(expectedChunkSize, responseData.size());
                                response.body.insert(response.body.end(), responseData.begin(), responseData.begin() + static_cast<ptrdiff_t>(toWrite));
                                responseData.erase(responseData.begin(), responseData.begin() + static_cast<ptrdiff_t>(toWrite));
                                expectedChunkSize -= toWrite;

                                if (expectedChunkSize == 0) removeCLRFAfterChunk = true;
                                if (responseData.empty()) break;
                            }
                            else
                            {
                                if (removeCLRFAfterChunk)
                                {
                                    if (responseData.size() >= 2)
                                    {
                                        removeCLRFAfterChunk = false;
                                        responseData.erase(responseData.begin(), responseData.begin() + 2);
                                    }
                                    else break;
                                }

                                auto i = std::search(responseData.begin(), responseData.end(), std::begin(clrf), std::end(clrf));

                                if (i == responseData.end()) break;

                                std::string line(responseData.begin(), i);
                                responseData.erase(responseData.begin(), i + 2);

                                expectedChunkSize = std::stoul(line, 0, 16);

                                if (expectedChunkSize == 0)
                                {
                                    dataReceived = true;
                                    break;
                                }
                            }
                        }

                        if (dataReceived)
                            break;
                    }
                    else
                    {
                        response.body.insert(response.body.end(), responseData.begin(), responseData.end());
                        responseData.clear();

                        // got the whole content
                        if (contentSize == -1 || response.body.size() >= static_cast<size_t>(contentSize))
                            break;
                    }
                }
            }
            while (size > 0);

            return response;
        }

    private:
        InternetProtocol internetProtocol;
        std::string scheme;
        std::string domain;
        std::string port;
        std::string path;
    };
}

#endif
