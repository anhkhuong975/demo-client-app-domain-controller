cmake_minimum_required(VERSION 3.10)
project(WINDOWS_BILLING_CLIENT)

set(CMAKE_CXX_STANDARD 17)

# Add include directories
include_directories(src/include)

# Create the billing client executable
add_executable(billing_client 
    src/main.cpp
    src/domain_manager.cpp
    src/data_collector.cpp
    src/config_manager.cpp
)

# Find and link LDAP libraries
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
