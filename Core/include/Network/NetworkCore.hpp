#ifndef NETWORKCORE_H
#define NETWORKCORE_H

#include <optional>
#include <queue>
#include <system_error>
#define ASIO_STANDALONE
#include <asio.hpp>

#include <array>
#include <cstddef>
#include <expected>
#include <mutex>
#include <string>
#include <thread>

class NetworkCore {
private:
    asio::io_context ioContext;
    asio::executor_work_guard<asio::io_context::executor_type> workGuard{asio::make_work_guard(ioContext)};
    asio::ip::udp::endpoint endPoint;
    asio::ip::udp::endpoint senderEndpoint;
    asio::ip::udp::socket socket;
    std::array<char, 1024> receiveBuffer;
    std::queue<std::string> receivedMessageQueue;
    std::mutex queueMutex;
    std::thread ioThread;

    void startReceive() {
        socket.async_receive_from(asio::buffer(receiveBuffer), senderEndpoint,
                                  [this](std::error_code ec, std::size_t bytesReceived) {
                                      if (!ec && bytesReceived > 0) {
                                          std::string message(receiveBuffer.data(), bytesReceived);
                                          std::lock_guard<std::mutex> lock(queueMutex);
                                          receivedMessageQueue.push(message);
                                      }
                                      startReceive(); // 继续监听下一条消息
                                  });
    }

public:
    NetworkCore(const std::string &localIp, int localPort)
        : ioContext(), workGuard(asio::make_work_guard(ioContext)), senderEndpoint(), socket(ioContext) {
        try {
            asio::ip::udp::resolver resolver(ioContext);
            auto results = resolver.resolve(localIp, std::to_string(localPort));
            if (results.begin() != results.end()) {
                endPoint = *results.begin();
            } else {
                endPoint = asio::ip::udp::endpoint(asio::ip::make_address(localIp), localPort);
            }
        } catch (const std::system_error &) {
            endPoint = asio::ip::udp::endpoint(asio::ip::make_address(localIp), localPort);
        }

        socket.open(endPoint.protocol());
        socket.bind(endPoint);

        startReceive();
        ioThread = std::thread([this]() { [[maybe_unused]] std::size_t runCount = ioContext.run(); });
    };

    ~NetworkCore() {
        std::error_code ec;
        socket.close(ec);
        workGuard.reset();
        ioContext.stop();
        if (ioThread.joinable())
            ioThread.join();
    };
    NetworkCore(const NetworkCore &) = delete;
    NetworkCore &operator=(const NetworkCore &) = delete;

    std::expected<std::size_t, std::error_code> send(const std::string &message, const std::string &targetIp,
                                                     int targetPort) {
        try {
            asio::ip::udp::resolver resolver(ioContext);
            auto results = resolver.resolve(targetIp, std::to_string(targetPort));
            if (results.begin() != results.end()) {
                asio::ip::udp::endpoint ep = *results.begin();
                return send(message, ep);
            }
            return std::unexpected(std::make_error_code(std::errc::address_not_available));
        } catch (const std::system_error &e) {
            return std::unexpected(e.code());
        }
    }

    std::expected<std::size_t, std::error_code> send(const std::string &message,
                                                     const asio::ip::udp::endpoint &targetEndpoint) {
        std::error_code ec;
        std::size_t bytesSent = socket.send_to(asio::buffer(message), targetEndpoint, 0, ec);
        if (ec) {
            return std::unexpected(ec);
        }
        return bytesSent;
    }

    std::optional<std::string> receive() {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (!receivedMessageQueue.empty()) {
            std::string message = std::move(receivedMessageQueue.front());
            receivedMessageQueue.pop();
            return message;
        }
        return std::nullopt; // 没有消息可用
    }
};

#endif // NETWORKCORE_H