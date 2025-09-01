cmake_minimum_required(VERSION 3.10)
project(DOMAIN_CONTROLLER_SERVER)

set(CMAKE_CXX_STANDARD 17)

# Add include directories
include_directories(include)

# Create the domain controller server executable
add_executable(domain_controller_server 
    src/main.cpp
    src/billing_server.cpp
)

# Link system libraries for socket operations
if(WIN32)
    target_link_libraries(domain_controller_server ws2_32)
else()
    # Unix/Linux systems - no additional libraries needed for sockets
    target_link_libraries(domain_controller_server pthread)
endif()

# Set output directory
set_target_properties(domain_controller_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/domain_controller_server
)
