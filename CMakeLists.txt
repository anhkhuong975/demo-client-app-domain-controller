cmake_minimum_required(VERSION 3.10)
project(C_PLUS_LEARNING)

set(CMAKE_CXX_STANDARD 17)

# =============================================================================
# BILLING CLIENT TARGET
# =============================================================================

# Add include directories for billing client
include_directories(billing_client/src/include)

# Create the billing client executable
add_executable(billing_client 
    billing_client/src/main.cpp
    billing_client/src/domain_manager.cpp
    billing_client/src/data_collector.cpp
    billing_client/src/config_manager.cpp
    billing_client/src/data_transmitter.cpp
)

# Find and link LDAP libraries for billing client
find_library(LDAP_LIBRARY ldap)
find_library(LBER_LIBRARY lber)

if(LDAP_LIBRARY AND LBER_LIBRARY)
    target_link_libraries(billing_client ${LDAP_LIBRARY} ${LBER_LIBRARY})
    message(STATUS "Found LDAP libraries: ${LDAP_LIBRARY}, ${LBER_LIBRARY}")
else()
    message(WARNING "LDAP libraries not found. Install openldap-dev or libldap-dev package.")
endif()

# Link Windows libraries for domain operations
if(WIN32)
    target_link_libraries(billing_client netapi32 advapi32 user32)
endif()

# Set output directory for billing client
set_target_properties(billing_client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# =============================================================================
# DOMAIN CONTROLLER SERVER TARGET
# =============================================================================

# Add include directories for domain controller server
include_directories(domain_controller_server/include)

# Create the domain controller server executable
add_executable(domain_controller_server 
    domain_controller_server/src/main.cpp
    domain_controller_server/src/billing_server.cpp
)

# Link system libraries for socket operations
if(WIN32)
    target_link_libraries(domain_controller_server ws2_32)
else()
    # Unix/Linux systems - no additional libraries needed for sockets
    target_link_libraries(domain_controller_server pthread)
endif()

# Set output directory for domain controller server
set_target_properties(domain_controller_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# =============================================================================
# PROJECT SETTINGS
# =============================================================================

# Set default target
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT billing_client)
