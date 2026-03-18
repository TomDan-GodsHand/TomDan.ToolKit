#include "ExampleController.h"
#include <string>
#include <ctime>

std::string ExampleController::getLastPathSegment(const std::string& path) {
    size_t lastSlash = path.find_last_of('/');
    return (lastSlash == std::string::npos ? path : path.substr(lastSlash + 1));
}

std::string ExampleController::getRoot(const std::string& /*path*/, const std::string& /*body*/) {
    return "HTTP Server is running! Available endpoints:\n"
           "GET  /api/hello - Get welcome message\n"
           "GET  /api/users - Get all users\n"
           "GET  /api/users/{id} - Get user by ID\n"
           "POST /api/users - Create new user\n"
           "PUT  /api/users/{id} - Update user\n"
           "DELETE /api/users/{id} - Delete user\n"
           "PATCH /api/users/{id}/status - Update user status";
}

std::string ExampleController::getHello(const std::string& /*path*/, const std::string& /*body*/) {
    return "Hello from HTTP Server! Current time: " + std::to_string(time(nullptr));
}

std::string ExampleController::getUsers(const std::string& /*path*/, const std::string& /*body*/) {
    return "[\n"
           "  {\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\"},\n"
           "  {\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\"},\n"
           "  {\"id\": 3, \"name\": \"Charlie\", \"email\": \"charlie@example.com\"}\n"
           "]";
}

std::string ExampleController::getUserById(const std::string& path, const std::string& /*body*/) {
    std::string idStr = getLastPathSegment(path);
    int id = std::stoi(idStr);

    if (id == 1) {
        return "{\"id\": 1, \"name\": \"Alice\", \"email\": \"alice@example.com\", \"status\": \"active\"}";
    } else if (id == 2) {
        return "{\"id\": 2, \"name\": \"Bob\", \"email\": \"bob@example.com\", \"status\": \"active\"}";
    } else {
        return "{\"error\": \"User not found\", \"id\": " + idStr + "}";
    }
}

std::string ExampleController::createUser(const std::string& /*path*/, const std::string& body) {
    if (body.empty()) {
        return "{\"error\": \"Request body is required\"}";
    }
    return "{\"message\": \"User created successfully\", \"data\": " + body + ", \"id\": 100}";
}

std::string ExampleController::updateUser(const std::string& path, const std::string& body) {
    if (body.empty()) {
        return "{\"error\": \"Request body is required for update\"}";
    }
    std::string idStr = getLastPathSegment(path);
    return "{\"message\": \"User updated successfully\", \"id\": " + idStr + ", \"data\": " + body + "}";
}

std::string ExampleController::deleteUser(const std::string& path, const std::string& /*body*/) {
    std::string idStr = getLastPathSegment(path);
    return "{\"message\": \"User deleted successfully\", \"id\": " + idStr + "}";
}

std::string ExampleController::patchUserStatus(const std::string& path, const std::string& body) {
    if (body.empty()) {
        return "{\"error\": \"Status data is required\"}";
    }
    std::string idStr = getLastPathSegment(path);
    return "{\"message\": \"User status updated\", \"id\": " + idStr + ", \"status\": " + body + "}";
}

std::string ExampleController::getHealth(const std::string& /*path*/, const std::string& /*body*/) {
    return "{\"status\": \"healthy\", \"timestamp\": " + std::to_string(time(nullptr)) + "}";
}

std::string ExampleController::getServerInfo(const std::string& /*path*/, const std::string& /*body*/) {
    return "{\"name\": \"TomDan HTTP Server\", \"version\": \"1.0.0\", \"framework\": \"cpp-httplib\"}";
}

void ExampleController::registerRoutes(HttpServer* server) {
    REGISTER_ROUTE(GET, getRoot);
    REGISTER_ROUTE(GET, getHello);
    REGISTER_ROUTE(GET, getUsers);
    REGISTER_ROUTE(GET, getUserById);
    REGISTER_ROUTE(POST, createUser);
    REGISTER_ROUTE(PUT, updateUser);
    REGISTER_ROUTE(DELETE, deleteUser);
    REGISTER_ROUTE(PATCH, patchUserStatus);
    REGISTER_ROUTE(GET, getHealth);
    REGISTER_ROUTE(GET, getServerInfo);
}

std::string ExampleController::getName() const {
    return "ExampleController";
}