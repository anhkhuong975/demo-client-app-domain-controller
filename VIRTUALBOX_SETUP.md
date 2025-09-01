# VirtualBox Demo Setup Guide

This guide provides step-by-step instructions for setting up a VirtualBox environment to demonstrate the Windows Billing Client application with actual Windows Domain integration.

## Environment Overview

```
┌─────────────────────┐    ┌─────────────────────┐
│   Windows Server    │    │   Windows 10 Pro   │
│   (Domain Controller)│    │   (Client Machine) │
│                     │    │                     │
│  • Active Directory │    │  • Domain Joined   │
│  • DNS Server       │    │  • Billing Client  │
│  • DHCP Server      │◄──►│  • Test Users       │
│                     │    │                     │
└─────────────────────┘    └─────────────────────┘
        192.168.1.10            192.168.1.20
```

## Prerequisites

- VirtualBox 6.0 or later
- Windows Server 2019/2022 ISO
- Windows 10 Pro ISO
- Minimum 8GB RAM (4GB per VM)
- 100GB+ available disk space

## Part 1: Windows Server Setup (Domain Controller)

### 1.1 Create Windows Server VM

```bash
# VM Configuration
Name: WinServer-DC
Type: Microsoft Windows
Version: Windows 2019 (64-bit)
Memory: 4096 MB
Hard Disk: 60 GB (VDI, Dynamically allocated)
```

### 1.2 Network Configuration

1. **VM Settings → Network**
   - Adapter 1: Internal Network
   - Name: `BillingDemo`
   - Advanced → Promiscuous Mode: Allow All

### 1.3 Windows Server Installation

1. Boot from Windows Server ISO
2. Choose **Windows Server 2019 Standard (Desktop Experience)**
3. Complete installation with Administrator password: `P@ssw0rd123`

### 1.4 Network Configuration

```powershell
# Set static IP address
New-NetIPAddress -InterfaceAlias "Ethernet" -IPAddress 192.168.1.10 -PrefixLength 24
Set-DnsClientServerAddress -InterfaceAlias "Ethernet" -ServerAddresses 192.168.1.10
```

### 1.5 Install Active Directory Domain Services

```powershell
# Install AD DS Role
Install-WindowsFeature -Name AD-Domain-Services -IncludeManagementTools

# Promote to Domain Controller
Install-ADDSForest `
    -DomainName "COMPANY.LOCAL" `
    -DomainNetbiosName "COMPANY" `
    -SafeModeAdministratorPassword (ConvertTo-SecureString "P@ssw0rd123" -AsPlainText -Force) `
    -InstallDns `
    -Force
```

### 1.6 Configure DHCP Server

```powershell
# Install DHCP Role
Install-WindowsFeature -Name DHCP -IncludeManagementTools

# Configure DHCP Scope
Add-DhcpServerv4Scope -Name "BillingDemo" -StartRange 192.168.1.100 -EndRange 192.168.1.200 -SubnetMask 255.255.255.0
Set-DhcpServerv4OptionValue -DnsDomain "COMPANY.LOCAL" -DnsServer 192.168.1.10 -Router 192.168.1.10
```

### 1.7 Create Test Users

```powershell
# Create Organizational Units
New-ADOrganizationalUnit -Name "Departments" -Path "DC=COMPANY,DC=LOCAL"
New-ADOrganizationalUnit -Name "IT" -Path "OU=Departments,DC=COMPANY,DC=LOCAL"
New-ADOrganizationalUnit -Name "Finance" -Path "OU=Departments,DC=COMPANY,DC=LOCAL"
New-ADOrganizationalUnit -Name "HR" -Path "OU=Departments,DC=COMPANY,DC=LOCAL"

# Create Test Users
$departments = @("IT", "Finance", "HR")
$users = @("john.doe", "jane.smith", "mike.johnson", "sarah.wilson", "david.brown")

foreach ($dept in $departments) {
    foreach ($user in $users) {
        $username = "$user.$dept"
        New-ADUser -Name $username -SamAccountName $username -UserPrincipalName "$username@COMPANY.LOCAL" `
                   -Path "OU=$dept,OU=Departments,DC=COMPANY,DC=LOCAL" `
                   -AccountPassword (ConvertTo-SecureString "User123!" -AsPlainText -Force) `
                   -Enabled $true
    }
}

# Create Service Account for Billing Client
New-ADUser -Name "billing_service" -SamAccountName "billing_service" `
           -UserPrincipalName "billing_service@COMPANY.LOCAL" `
           -AccountPassword (ConvertTo-SecureString "Service123!" -AsPlainText -Force) `
           -Enabled $true
```

## Part 2: Windows 10 Pro Setup (Client Machine)

### 2.1 Create Windows 10 Pro VM

```bash
# VM Configuration
Name: Win10-Client
Type: Microsoft Windows
Version: Windows 10 (64-bit)
Memory: 4096 MB
Hard Disk: 40 GB (VDI, Dynamically allocated)
```

### 2.2 Network Configuration

1. **VM Settings → Network**
   - Adapter 1: Internal Network
   - Name: `BillingDemo` (same as server)

### 2.3 Windows 10 Installation

1. Boot from Windows 10 Pro ISO
2. Complete installation
3. Create local user: `localuser` / `P@ssw0rd123`

### 2.4 Join Domain

```powershell
# Configure network (should get IP from DHCP)
# Verify connectivity to domain controller
Test-NetConnection -ComputerName 192.168.1.10 -Port 389

# Join domain
Add-Computer -DomainName "COMPANY.LOCAL" -Credential (Get-Credential) -Restart
# Use: COMPANY\Administrator / P@ssw0rd123
```

### 2.5 Install Development Tools

1. **Install Visual Studio Community 2022**
   - Workloads: Desktop development with C++
   - Components: CMake tools, Windows 10/11 SDK

2. **Install Git**
   - Download from git-scm.com
   - Use default settings

## Part 3: Deploy Billing Client Application

### 3.1 Transfer Source Code

1. **Option A: USB/Shared Folder**
   ```bash
   # Copy source code to client machine
   # Place in C:\BillingClient\
   ```

2. **Option B: Git Repository**
   ```bash
   git clone <repository-url> C:\BillingClient
   ```

### 3.2 Build Application

```cmd
cd C:\BillingClient
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### 3.3 Configure Application

Create `config\billing_client.conf`:
```ini
[global]
log_level = INFO
data_collection_interval = 3600
max_retry_attempts = 3
output_directory = output

[domain:COMPANY.LOCAL]
domain_controller = DC01.COMPANY.LOCAL
service_account = billing_service
connection_timeout = 30
use_ssl = false
data_sources = WMI, EventLog, Registry
```

## Part 4: Demo Execution

### 4.1 Start Demo

```cmd
cd C:\BillingClient\build\Release
billing_client.exe
```

### 4.2 Demo Workflow

1. **Load Configuration** (Option 1)
   - Creates default config if not exists
   - Displays configured domains

2. **Connect to Domain** (Option 2)
   - Select: COMPANY.LOCAL
   - Username: `billing_service`
   - Password: `Service123!`

3. **Collect Working Hours Data** (Option 3)
   - Queries domain users
   - Generates mock working hours data

4. **Generate Billing Report** (Option 4)
   - Calculates costs based on hourly rate
   - Creates detailed report

5. **Export Data to CSV** (Option 5)
   - Exports raw data for analysis

6. **View Configuration** (Option 6)
   - Shows current settings

### 4.3 Verify Output Files

```cmd
dir output\
# Should contain:
# - billing_report_COMPANY.LOCAL.txt
# - working_hours_COMPANY.LOCAL.csv
```

## Part 5: Advanced Testing

### 5.1 Multi-Domain Setup

To test multi-domain functionality:

1. **Create Second Domain**
   - Setup another Windows Server VM
   - Create domain: `BRANCH.LOCAL`
   - Establish trust relationship

2. **Update Configuration**
   ```ini
   [domain:BRANCH.LOCAL]
   domain_controller = DC02.BRANCH.LOCAL
   service_account = billing_service
   connection_timeout = 30
   use_ssl = false
   data_sources = WMI, EventLog, Registry
   ```

### 5.2 Real Windows API Integration

For production testing, replace mock implementations with:

```cpp
// In domain_manager.cpp
#include <windows.h>
#include <lm.h>
#include <dsgetdc.h>

// Real domain authentication
bool DomainManager::authenticate(const std::string& domain_name, 
                                const std::string& user_name, 
                                const std::string& password) {
    HANDLE hToken;
    BOOL result = LogonUser(
        user_name.c_str(),
        domain_name.c_str(),
        password.c_str(),
        LOGON32_LOGON_NETWORK,
        LOGON32_PROVIDER_DEFAULT,
        &hToken
    );
    
    if (result) {
        CloseHandle(hToken);
        return true;
    }
    return false;
}

// Real user enumeration
std::vector<std::string> DomainManager::get_domain_users() {
    std::vector<std::string> users;
    LPUSER_INFO_0 pBuf = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    
    NET_API_STATUS nStatus = NetUserEnum(
        NULL,                    // servername
        0,                       // level
        FILTER_NORMAL_ACCOUNT,   // filter
        (LPBYTE*)&pBuf,         // bufptr
        MAX_PREFERRED_LENGTH,    // prefmaxlen
        &dwEntriesRead,         // entriesread
        &dwTotalEntries,        // totalentries
        NULL                    // resume_handle
    );
    
    if (nStatus == NERR_Success) {
        for (DWORD i = 0; i < dwEntriesRead; i++) {
            std::wstring wstr(pBuf[i].usri0_name);
            std::string str(wstr.begin(), wstr.end());
            users.push_back(str);
        }
        NetApiBufferFree(pBuf);
    }
    
    return users;
}
```

## Troubleshooting

### Common Issues

1. **Domain Join Failed**
   - Check DNS settings on client
   - Verify domain controller is reachable
   - Ensure time synchronization

2. **Authentication Failed**
   - Verify user credentials
   - Check domain trust relationships
   - Review firewall settings

3. **Build Errors**
   - Install Visual Studio C++ tools
   - Update Windows SDK
   - Check CMake version

### Network Diagnostics

```powershell
# Test DNS resolution
nslookup COMPANY.LOCAL

# Test domain controller connectivity
Test-NetConnection -ComputerName DC01.COMPANY.LOCAL -Port 389

# Verify domain membership
Get-ComputerInfo | Select-Object CsDomain, CsDomainRole
```

## Security Considerations

1. **Production Environment**
   - Use secure passwords
   - Enable SSL/TLS for LDAP
   - Implement certificate-based authentication
   - Regular security updates

2. **Service Account**
   - Minimal required permissions
   - Regular password rotation
   - Monitor account usage

3. **Network Security**
   - Firewall configuration
   - Network segmentation
   - VPN for remote access
