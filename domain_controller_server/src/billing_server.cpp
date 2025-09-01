#include "billing_server.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <algorithm>

// Socket includes
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

BillingServer::BillingServer(int port) : server_port(port), running(false), server_socket(-1) {
    std::cout << "[BillingServer] Domain Controller Billing Server initialized on port " << port << std::endl;
    
#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

BillingServer::~BillingServer() {
    stop();
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    std::cout << "[BillingServer] Billing server destroyed" << std::endl;
}

bool BillingServer::initialize_socket() {
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cout << "[BillingServer] Error: Failed to create socket" << std::endl;
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cout << "[BillingServer] Warning: Failed to set socket options" << std::endl;
    }
    
    // Bind socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cout << "[BillingServer] Error: Failed to bind socket to port " << server_port << std::endl;
#ifdef _WIN32
        closesocket(server_socket);
#else
        close(server_socket);
#endif
        return false;
    }
    
    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        std::cout << "[BillingServer] Error: Failed to listen on socket" << std::endl;
#ifdef _WIN32
        closesocket(server_socket);
#else
        close(server_socket);
#endif
        return false;
    }
    
    std::cout << "[BillingServer] Server socket initialized and listening on port " << server_port << std::endl;
    return true;
}

bool BillingServer::start() {
    if (running) {
        std::cout << "[BillingServer] Server is already running" << std::endl;
        return false;
    }
    
    if (!initialize_socket()) {
        return false;
    }
    
    running = true;
    server_thread = std::thread(&BillingServer::server_loop, this);
    
    std::cout << "[BillingServer] ✅ Billing server started successfully!" << std::endl;
    std::cout << "[BillingServer] Listening for billing client connections on port " << server_port << std::endl;
    std::cout << "[BillingServer] Dashboard available at: http://localhost:" << server_port << "/dashboard" << std::endl;
    std::cout << "[BillingServer] API endpoint: http://localhost:" << server_port << "/api/billing/working-hours" << std::endl;
    
    return true;
}

void BillingServer::stop() {
    if (!running) {
        return;
    }
    
    running = false;
    
    // Close server socket to break accept() loop
    if (server_socket >= 0) {
#ifdef _WIN32
        closesocket(server_socket);
#else
        close(server_socket);
#endif
        server_socket = -1;
    }
    
    // Wait for server thread to finish
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    std::cout << "[BillingServer] Server stopped" << std::endl;
}

void BillingServer::server_loop() {
    std::cout << "[BillingServer] Server loop started" << std::endl;
    
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Accept incoming connection
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            if (running) {
                std::cout << "[BillingServer] Error accepting connection" << std::endl;
            }
            continue;
        }
        
        // Get client IP
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        std::cout << "[BillingServer] New connection from: " << client_ip << std::endl;
        
        // Handle request in same thread for simplicity
        handle_request(client_socket, client_ip);
        
        // Close client socket
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
    }
    
    std::cout << "[BillingServer] Server loop ended" << std::endl;
}

void BillingServer::handle_request(int client_socket, const std::string& client_ip) {
    char buffer[4096];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_received <= 0) {
        return;
    }
    
    buffer[bytes_received] = '\0';
    std::string request(buffer);
    
    std::cout << "[BillingServer] Received request from " << client_ip << std::endl;
    
    // Parse request method and path
    std::istringstream request_stream(request);
    std::string method, path, version;
    request_stream >> method >> path >> version;
    
    std::string response;
    
    if (method == "POST" && path == "/api/billing/working-hours") {
        // Handle billing data submission
        std::cout << "[BillingServer] Processing billing data submission" << std::endl;
        
        // Extract JSON payload from request body
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string json_payload = request.substr(body_start + 4);
            bool success = parse_billing_data(json_payload, client_ip);
            response = generate_response(success);
        } else {
            response = generate_response(false);
        }
        
    } else if (method == "GET" && path == "/dashboard") {
        // Handle dashboard request
        std::cout << "[BillingServer] Serving dashboard" << std::endl;
        response = generate_dashboard();
        
    } else if (method == "GET" && path == "/api/data") {
        // Handle API data request
        std::cout << "[BillingServer] Serving API data" << std::endl;
        response = generate_api_response();
        
    } else {
        // Handle unknown requests
        response = "HTTP/1.1 404 Not Found\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 13\r\n";
        response += "\r\n";
        response += "404 Not Found";
    }
    
    // Send response
    send(client_socket, response.c_str(), response.length(), 0);
}

bool BillingServer::parse_billing_data(const std::string& json_payload, const std::string& client_ip) {
    std::cout << "[BillingServer] Parsing billing data from client: " << client_ip << std::endl;
    std::cout << "[BillingServer] Payload size: " << json_payload.length() << " bytes" << std::endl;
    
    // Simple JSON parsing (in production, use proper JSON library)
    // For demo purposes, we'll extract key information manually
    
    // Get current timestamp
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream timestamp_stream;
    timestamp_stream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string current_timestamp = timestamp_stream.str();
    
    // Store client info
    ClientInfo client_info;
    client_info.client_ip = client_ip;
    client_info.timestamp = current_timestamp;
    client_info.client_version = "1.0.0"; // Extract from JSON in production
    client_info.data_source = "Windows_Client_Demo";
    client_connections.push_back(client_info);
    
    // Parse working hours data (simplified parsing)
    // Look for employee records in JSON
    size_t pos = 0;
    int records_parsed = 0;
    
    while ((pos = json_payload.find("\"employee_id\":", pos)) != std::string::npos) {
        ReceivedWorkingHoursData data;
        data.received_timestamp = current_timestamp;
        data.client_source = client_ip;
        
        // Extract employee_id
        size_t id_start = json_payload.find("\"", pos + 14) + 1;
        size_t id_end = json_payload.find("\"", id_start);
        if (id_end != std::string::npos) {
            data.employee_id = json_payload.substr(id_start, id_end - id_start);
        }
        
        // Extract employee_name
        size_t name_pos = json_payload.find("\"employee_name\":", pos);
        if (name_pos != std::string::npos) {
            size_t name_start = json_payload.find("\"", name_pos + 16) + 1;
            size_t name_end = json_payload.find("\"", name_start);
            if (name_end != std::string::npos) {
                data.employee_name = json_payload.substr(name_start, name_end - name_start);
            }
        }
        
        // Extract department
        size_t dept_pos = json_payload.find("\"department\":", pos);
        if (dept_pos != std::string::npos) {
            size_t dept_start = json_payload.find("\"", dept_pos + 13) + 1;
            size_t dept_end = json_payload.find("\"", dept_start);
            if (dept_end != std::string::npos) {
                data.department = json_payload.substr(dept_start, dept_end - dept_start);
            }
        }
        
        // Extract hours_worked
        size_t hours_pos = json_payload.find("\"hours_worked\":", pos);
        if (hours_pos != std::string::npos) {
            size_t hours_start = hours_pos + 15;
            size_t hours_end = json_payload.find(",", hours_start);
            if (hours_end == std::string::npos) {
                hours_end = json_payload.find("}", hours_start);
            }
            if (hours_end != std::string::npos) {
                std::string hours_str = json_payload.substr(hours_start, hours_end - hours_start);
                // Remove whitespace
                hours_str.erase(std::remove_if(hours_str.begin(), hours_str.end(), ::isspace), hours_str.end());
                try {
                    data.hours_worked = std::stod(hours_str);
                } catch (...) {
                    data.hours_worked = 0.0;
                }
            }
        }
        
        // Extract date
        size_t date_pos = json_payload.find("\"date\":", pos);
        if (date_pos != std::string::npos) {
            size_t date_start = json_payload.find("\"", date_pos + 7) + 1;
            size_t date_end = json_payload.find("\"", date_start);
            if (date_end != std::string::npos) {
                data.date = json_payload.substr(date_start, date_end - date_start);
            }
        }
        
        // Extract project_code
        size_t proj_pos = json_payload.find("\"project_code\":", pos);
        if (proj_pos != std::string::npos) {
            size_t proj_start = json_payload.find("\"", proj_pos + 15) + 1;
            size_t proj_end = json_payload.find("\"", proj_start);
            if (proj_end != std::string::npos) {
                data.project_code = json_payload.substr(proj_start, proj_end - proj_start);
            }
        }
        
        received_data.push_back(data);
        records_parsed++;
        pos = id_end + 1;
    }
    
    std::cout << "[BillingServer] ✅ Successfully parsed " << records_parsed << " working hours records" << std::endl;
    std::cout << "[BillingServer] Total records in database: " << received_data.size() << std::endl;
    
    return records_parsed > 0;
}

std::string BillingServer::generate_response(bool success) {
    std::string response = "HTTP/1.1 ";
    std::string body;
    
    if (success) {
        response += "200 OK\r\n";
        body = "{\"status\":\"success\",\"message\":\"Billing data received and processed\"}";
    } else {
        response += "400 Bad Request\r\n";
        body = "{\"status\":\"error\",\"message\":\"Failed to process billing data\"}";
    }
    
    response += "Content-Type: application/json\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    response += body;
    
    return response;
}

std::string BillingServer::generate_dashboard() {
    std::ostringstream html;
    
    html << "HTTP/1.1 200 OK\r\n";
    html << "Content-Type: text/html\r\n";
    html << "\r\n";
    
    html << "<!DOCTYPE html>\n";
    html << "<html><head><title>Domain Controller - Billing Data Dashboard</title>\n";
    html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }\n";
    html << ".header { background-color: #2c3e50; color: white; padding: 20px; border-radius: 5px; }\n";
    html << ".stats { display: flex; gap: 20px; margin: 20px 0; }\n";
    html << ".stat-box { background: white; padding: 20px; border-radius: 5px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); flex: 1; }\n";
    html << ".data-table { background: white; padding: 20px; border-radius: 5px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }\n";
    html << "table { width: 100%; border-collapse: collapse; }\n";
    html << "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }\n";
    html << "th { background-color: #34495e; color: white; }\n";
    html << ".refresh-btn { background-color: #3498db; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; }\n";
    html << "</style>\n";
    html << "<script>function refreshPage() { location.reload(); }</script>\n";
    html << "</head><body>\n";
    
    html << "<div class='header'>\n";
    html << "<h1>🖥️ Domain Controller - Billing Data Dashboard</h1>\n";
    html << "<p>Real-time monitoring of working hours data from Windows billing clients</p>\n";
    html << "</div>\n";
    
    // Statistics
    html << "<div class='stats'>\n";
    html << "<div class='stat-box'>\n";
    html << "<h3>📊 Total Records</h3>\n";
    html << "<h2>" << received_data.size() << "</h2>\n";
    html << "</div>\n";
    html << "<div class='stat-box'>\n";
    html << "<h3>💻 Connected Clients</h3>\n";
    html << "<h2>" << client_connections.size() << "</h2>\n";
    html << "</div>\n";
    html << "<div class='stat-box'>\n";
    html << "<h3>⏰ Last Update</h3>\n";
    html << "<h2>" << (received_data.empty() ? "No data" : received_data.back().received_timestamp) << "</h2>\n";
    html << "</div>\n";
    html << "</div>\n";
    
    // Refresh button
    html << "<button class='refresh-btn' onclick='refreshPage()'>🔄 Refresh Data</button>\n";
    
    // Working Hours Data Table
    html << "<div class='data-table'>\n";
    html << "<h2>📋 Working Hours Data</h2>\n";
    html << "<table>\n";
    html << "<tr><th>Employee ID</th><th>Name</th><th>Department</th><th>Hours</th><th>Date</th><th>Project</th><th>Received</th><th>Source</th></tr>\n";
    
    for (const auto& data : received_data) {
        html << "<tr>";
        html << "<td>" << data.employee_id << "</td>";
        html << "<td>" << data.employee_name << "</td>";
        html << "<td>" << data.department << "</td>";
        html << "<td>" << std::fixed << std::setprecision(2) << data.hours_worked << "</td>";
        html << "<td>" << data.date << "</td>";
        html << "<td>" << data.project_code << "</td>";
        html << "<td>" << data.received_timestamp << "</td>";
        html << "<td>" << data.client_source << "</td>";
        html << "</tr>\n";
    }
    
    html << "</table>\n";
    html << "</div>\n";
    
    // Client Connections
    html << "<div class='data-table'>\n";
    html << "<h2>🔗 Client Connections</h2>\n";
    html << "<table>\n";
    html << "<tr><th>Client IP</th><th>Connection Time</th><th>Version</th><th>Data Source</th></tr>\n";
    
    for (const auto& client : client_connections) {
        html << "<tr>";
        html << "<td>" << client.client_ip << "</td>";
        html << "<td>" << client.timestamp << "</td>";
        html << "<td>" << client.client_version << "</td>";
        html << "<td>" << client.data_source << "</td>";
        html << "</tr>\n";
    }
    
    html << "</table>\n";
    html << "</div>\n";
    
    html << "</body></html>";
    
    return html.str();
}

std::string BillingServer::generate_api_response() {
    std::ostringstream json;
    
    json << "HTTP/1.1 200 OK\r\n";
    json << "Content-Type: application/json\r\n";
    json << "Access-Control-Allow-Origin: *\r\n";
    json << "\r\n";
    
    json << "{\n";
    json << "  \"server_info\": {\n";
    json << "    \"name\": \"Domain Controller Billing Server\",\n";
    json << "    \"port\": " << server_port << ",\n";
    json << "    \"total_records\": " << received_data.size() << ",\n";
    json << "    \"total_clients\": " << client_connections.size() << "\n";
    json << "  },\n";
    json << "  \"working_hours_data\": [\n";
    
    for (size_t i = 0; i < received_data.size(); ++i) {
        const auto& data = received_data[i];
        json << "    {\n";
        json << "      \"employee_id\": \"" << data.employee_id << "\",\n";
        json << "      \"employee_name\": \"" << data.employee_name << "\",\n";
        json << "      \"department\": \"" << data.department << "\",\n";
        json << "      \"hours_worked\": " << data.hours_worked << ",\n";
        json << "      \"date\": \"" << data.date << "\",\n";
        json << "      \"project_code\": \"" << data.project_code << "\",\n";
        json << "      \"received_timestamp\": \"" << data.received_timestamp << "\",\n";
        json << "      \"client_source\": \"" << data.client_source << "\"\n";
        json << "    }";
        
        if (i < received_data.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    
    json << "  ]\n";
    json << "}";
    
    return json.str();
}

bool BillingServer::is_running() const {
    return running;
}

std::vector<ReceivedWorkingHoursData> BillingServer::get_received_data() const {
    return received_data;
}

std::vector<ClientInfo> BillingServer::get_client_info() const {
    return client_connections;
}

void BillingServer::display_statistics() const {
    std::cout << "\n=== Domain Controller Billing Server Statistics ===" << std::endl;
    std::cout << "Server Port: " << server_port << std::endl;
    std::cout << "Server Status: " << (running ? "RUNNING" : "STOPPED") << std::endl;
    std::cout << "Total Records Received: " << received_data.size() << std::endl;
    std::cout << "Total Client Connections: " << client_connections.size() << std::endl;
    
    if (!received_data.empty()) {
        std::cout << "Last Data Received: " << received_data.back().received_timestamp << std::endl;
        std::cout << "Last Client: " << received_data.back().client_source << std::endl;
    }
    
    std::cout << "Dashboard URL: http://localhost:" << server_port << "/dashboard" << std::endl;
    std::cout << "API URL: http://localhost:" << server_port << "/api/data" << std::endl;
}

bool BillingServer::export_to_csv(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "[BillingServer] Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    
    // Write CSV header
    file << "Employee_ID,Employee_Name,Department,Hours_Worked,Date,Project_Code,Received_Timestamp,Client_Source\n";
    
    // Write data rows
    for (const auto& data : received_data) {
        file << data.employee_id << ","
             << data.employee_name << ","
             << data.department << ","
             << std::fixed << std::setprecision(2) << data.hours_worked << ","
             << data.date << ","
             << data.project_code << ","
             << data.received_timestamp << ","
             << data.client_source << "\n";
    }
    
    file.close();
    std::cout << "[BillingServer] Data exported to: " << filename << std::endl;
    return true;
}
