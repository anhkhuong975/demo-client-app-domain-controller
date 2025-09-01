# Windows Billing Client Demo System

A comprehensive C++ demonstration system that simulates Windows Domain billing infrastructure with multi-domain support for collecting and processing working hours data.

## System Overview

This project consists of two main components:
- **Billing Client**: Collects working hours data from Windows domains
- **Domain Controller Server**: Receives and processes data from billing clients

## Architecture

```
c-plus-learning/
├── CMakeLists.txt                    # Single build configuration
├── config/                          # Configuration files (shared)
│   └── billing_client.conf         # Client configuration
├── build/                           # Build output directory
│   ├── billing_client              # Client executable
│   └── domain_controller_server    # Server executable
├── billing_client/                 # Client source code
│   └── src/
│       ├── include/
│       │   ├── domain_manager.h    # Windows Domain authentication
│       │   ├── data_collector.h    # Working hours data collection
│       │   ├── config_manager.h    # Configuration management
│       │   └── data_transmitter.h  # Data transmission to server
│       ├── main.cpp               # Client application entry point
│       ├── domain_manager.cpp     # LDAP authentication implementation
│       ├── data_collector.cpp     # Mock data generation
│       ├── config_manager.cpp     # Configuration file handling
│       └── data_transmitter.cpp   # HTTP client for data transmission
└── domain_controller_server/      # Server source code
    ├── include/
    │   └── billing_server.h       # HTTP server interface
    ├── src/
    │   ├── main.cpp              # Server application entry point
    │   └── billing_server.cpp    # HTTP server implementation
    └── data/                     # Server data storage
```

## Features

### Billing Client
- **Multi-Domain Authentication**: LDAP-based Windows Domain authentication
- **Data Collection**: Automated working hours data collection with mock data generation
- **Configuration Management**: File-based configuration for multiple domains
- **Data Transmission**: HTTP client for sending data to domain controller server
- **Interactive Menu**: User-friendly command-line interface

### Domain Controller Server
- **HTTP Server**: RESTful API server listening on port 8080
- **Real-time Dashboard**: Web interface for viewing received data
- **Data Storage**: In-memory storage with CSV export capability
- **Client Monitoring**: Track billing client connections and statistics

## Quick Start

### Prerequisites
- CMake 3.10 or higher
- C++17 compatible compiler
- LDAP libraries (automatically detected on macOS/Linux)

### Build System
The project uses a **single consolidated CMakeLists.txt** for simplified building:

```bash
# Clone and build
git clone <repository>
cd c-plus-learning

# Single build command for all components
mkdir -p build && cd build
cmake ..
make

# Both executables are now available in build/
ls -la
# billing_client
# domain_controller_server
```

### Running the Demo

#### 1. Start Domain Controller Server
```bash
cd build
./domain_controller_server
# Choose option 1 to start HTTP server on port 8080
```

#### 2. Start Billing Client
```bash
# Option 1: Run from project root
./build/billing_client

# Option 2: Run from build directory
cd build
./billing_client

# Interactive menu will guide you through the workflow
```

#### 3. Demo Workflow
1. **Load Configuration** - Client automatically loads domain settings from `config/billing_client.conf`
2. **Connect to Domain** - Authenticate with Windows Domain (demo mode accepts any credentials)
3. **Collect Working Hours Data** - Generate mock employee working hours data
4. **Transmit Data to Server** - Send collected data to domain controller server
5. **View Results** - Check server dashboard at `http://localhost:8080/dashboard`

## Configuration

### Client Configuration (`config/billing_client.conf`)
```ini
[global]
data_collection_interval = 3600
log_level = INFO
max_retry_attempts = 3
output_directory = output

[domain:ds-vm1.local]
domain_controller = 192.168.1.55
service_account = ds-vm2
connection_timeout = 30
use_ssl = false
data_sources = WMI, EventLog, Registry

[ldap]
ldap_server = 192.168.1.55
ldap_port = 389
ldap_use_ssl = false
base_dn = DC=ds-vm1,DC=local
user_search_filter = (sAMAccountName=%s)
bind_dn_format = %s@ds-vm1.local
ldap_timeout = 10
```

## API Endpoints

### Domain Controller Server
- `POST /api/billing/working-hours` - Receive working hours data from clients
- `GET /dashboard` - Web dashboard for viewing collected data
- `GET /api/data` - JSON API for accessing all stored data

## Mock Data Generation

The system generates realistic demo data including:
- **Employee IDs**: EMP001, EMP002, etc.
- **Departments**: IT, Finance, HR, Marketing, Operations
- **Working Hours**: Random hours between 6-10 per day
- **Project Codes**: PROJ001-PROJ007
- **Timestamps**: Current date for all entries

## VirtualBox Demo Environment

For a complete Windows Domain demo:

### Windows Server Setup
1. Install Windows Server 2019/2022
2. Configure Active Directory Domain Services
3. Create domain: `ds-vm1.local`
4. Configure DNS and LDAP services

### Windows Client Setup
1. Install Windows 10 Pro
2. Join the domain `ds-vm1.local`
3. Build and run the billing client

### Network Configuration
- Ensure VM communication on private network
- Configure DNS to point to domain controller
- Open firewall ports: 389 (LDAP), 8080 (HTTP server)

## Build System Changes

**Recent Updates:**
- **Consolidated Build**: Single `CMakeLists.txt` at root level manages all components
- **Simplified Workflow**: One build command creates both executables in `build/` directory
- **Clean Structure**: Removed individual CMakeLists.txt files from subdirectories
- **Shared Configuration**: Configuration files moved to project root level (`config/`) for better organization
- **Flexible Execution**: Applications can run from both project root and build directory

## Output Files

The system generates:
- `config/billing_client.conf` - Configuration file (shared across project)
- `output/billing_report_[domain].txt` - Detailed billing reports
- `output/working_hours_[domain].csv` - Raw data exports
- Server dashboard accessible at `http://localhost:8080/dashboard`

## Production Considerations

For real-world deployment:

### Security Enhancements
- Secure credential storage (Windows Credential Manager)
- Certificate-based authentication
- Encrypted configuration files
- Comprehensive audit logging

### Windows API Integration
- **NetAPI32**: Domain user enumeration
- **ADSI/LDAP**: Active Directory queries
- **WMI**: System information collection
- **Event Log APIs**: Logon/logoff tracking

### Performance Optimizations
- Connection pooling for LDAP connections
- Asynchronous data collection
- Database storage instead of in-memory
- Batch processing for large datasets

## Troubleshooting

### Common Issues
1. **Build Errors**: Ensure CMake 3.10+ and C++17 compiler
2. **LDAP Libraries**: Install `openldap-dev` or `libldap-dev` on Linux
3. **Config Not Found**: Ensure `config/billing_client.conf` exists in project root
4. **Server Connection**: Ensure server is running before client data transmission
5. **Path Issues**: Application automatically detects config file location when run from project root or build directory

### Debug Mode
Set `log_level = DEBUG` in configuration file for detailed logging.

## License
None