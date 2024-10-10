Step 1: Install Dependencies
1. Update Package Lists
sudo apt-get update

2. Install Essential Build Tools
sudo apt-get install -y build-essential cmake git wget

3. Install Boost Libraries (Including Boost.Asio)
sudo apt-get install -y libboost-all-dev

4. Install libsrtp2
sudo apt-get install -y libsrtp2-dev

5. Install MsQuic SDK
MsQuic is not available via apt-get and needs to be built from source.

Install Prerequisites
sudo apt-get install -y ninja-build clang

Clone and Build MsQuic
# Clone MsQuic repository
git clone --recursive https://github.com/microsoft/msquic.git
cd msquic
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure the build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build MsQuic
ninja

# Install MsQuic
sudo ninja install

# Return to the home directory
cd ../..

#Note: MsQuic headers and libraries are installed in /usr/local/include and /usr/local/lib by default.

6. Install Redis and Redis++
#Install Redis Server

sudo apt-get install -y redis-server
#Install Hiredis

sudo apt-get install -y libhiredis-dev
#Install Redis++ (Redis-Plus-Plus)

# Download Redis++ source code
wget https://github.com/sewenew/redis-plus-plus/archive/refs/tags/1.3.4.tar.gz

# Extract the archive
tar xzf 1.3.4.tar.gz
cd redis-plus-plus-1.3.4

# Create build directory
mkdir build && cd build

# Configure the build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

# Build Redis++
make

# Install Redis++
sudo make install

# Return to the home directory
cd ../..
7. Install spdlog Library

sudo apt-get install -y libspdlog-dev
8. Install OpenSSL Development Libraries
Required for MsQuic and other dependencies.


sudo apt-get install -y libssl-dev
9. Install Boost System and Thread Libraries (If Not Already Installed)

sudo apt-get install -y libboost-system-dev libboost-thread-dev


Step 2: Build the Application



1. Clone Your Project Repository

2. Ensure CMakeLists.txt is Correctly Configured
Your CMakeLists.txt should include the correct paths for the libraries and headers installed in /usr/local.

Example CMakeLists.txt Adjustments:

cmake
# Add these lines to include directories and link directories
include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# Link the necessary libraries
target_link_libraries(QuicRtp
    srtp2
    msquic
    hiredis
    redis++
    pthread
    boost_system
    boost_thread
    spdlog
    ssl
    crypto
)
3. Create a Build Directory and Build the Project

mkdir build && cd build
cmake ..
make
Note: If you encounter any errors during the cmake or make steps, ensure that all dependencies are correctly installed and that the paths in CMakeLists.txt are accurate.

4. Set Library Path Environment Variable
Ensure that the system can find the libraries installed in /usr/local/lib.

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

#Probably best to make it persistent with: 
echo 'export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

Step 3: Run the Application
1. Set the SRTP_KEY Environment Variable
Replace your_srtp_key_here with your actual SRTP key in hexadecimal format (must be 60 characters for a 30-byte key).

export SRTP_KEY=your_srtp_key_here

2. Ensure Redis Server is Running
sudo systemctl start redis-server
sudo systemctl enable redis-server

You can check the status of Redis:
sudo systemctl status redis-server

3. Place config.conf in the Executable Directory
Copy your config.conf file into the build directory where your executable resides.


cp /path/to/config.conf ./build/
4. Run the Executable
Navigate to the build directory if you're not already there:


cd /path/to/QuicRtp/build
Run the application:


./QuicRtp
Complete Command Sequence
Below is a combined sequence of all the commands:


# Update package lists
sudo apt-get update

# Install essential build tools
sudo apt-get install -y build-essential cmake git wget

# Install Boost libraries
sudo apt-get install -y libboost-all-dev

# Install libsrtp2
sudo apt-get install -y libsrtp2-dev

# Install MsQuic prerequisites
sudo apt-get install -y ninja-build clang

# Clone and build MsQuic
git clone https://github.com/microsoft/msquic.git
cd msquic
git submodule update --init --recursive
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
sudo ninja install
cd ../..

# Install Redis server
sudo apt-get install -y redis-server

# Install Hiredis
sudo apt-get install -y libhiredis-dev

# Install Redis++
wget https://github.com/sewenew/redis-plus-plus/archive/refs/tags/1.3.4.tar.gz
tar xzf 1.3.4.tar.gz
cd redis-plus-plus-1.3.4
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
make
sudo make install
cd ../..

# Install spdlog library
sudo apt-get install -y libspdlog-dev

# Install OpenSSL development libraries
sudo apt-get install -y libssl-dev

# Install Boost system and thread libraries (if not already installed)
sudo apt-get install -y libboost-system-dev libboost-thread-dev

# Set library path environment variable
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Clone your project repository
git clone https://your-repo-url.git /path/to/QuicRtp
cd /path/to/QuicRtp

# Create build directory and build the project
mkdir build && cd build
cmake ..
make

# Set SRTP_KEY environment variable
export SRTP_KEY=your_srtp_key_here

# Start Redis server
sudo systemctl start redis-server
sudo systemctl enable redis-server

# Copy config.conf to the build directory
cp /path/to/config.conf ./

# Run the application
./QuicRtp

Additional Notes
Setting up config.conf
Ensure your config.conf file is properly configured. Here is an example:

[RTP]
port_range_start = 5000
port_range_end = 5100

[SRTP]
enable = true

[Cache]
redis_uri = tcp://127.0.0.1:6379

[Logging]
level = info

[QUIC]
server_ip = your_quic_server_ip
server_port = your_quic_server_port

;Replace your_quic_server_ip and your_quic_server_port with the actual IP address and port of your QUIC server.
;Ensure that the port range specified is correct and not used by other applications.

#Verifying MsQuic Installation
If you encounter issues with MsQuic during the build process, ensure that:

The MsQuic headers are located in /usr/local/include/msquic.
The MsQuic library is located in /usr/local/lib.

You can verify the files:

ls /usr/local/include/msquic
ls /usr/local/lib | grep msquic

Firewall Configuration
If you are using a firewall, ensure that the RTP port range and QUIC port are open.

For UFW (Uncomplicated Firewall):
sudo ufw allow 5000:5100/udp
sudo ufw allow your_quic_server_port/tcp

Running the Application with Elevated Privileges
If you encounter permission issues when binding to ports, you may need to run the application with elevated privileges:

sudo ./QuicRtp
However, running applications with sudo is generally discouraged. Instead, consider using ports above 1024 or adjusting permissions.

Environment Variables
To make the environment variables persistent across sessions, add them to your ~/.bashrc or ~/.profile file:

echo 'export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
echo 'export SRTP_KEY=your_srtp_key_here' >> ~/.bashrc

Then, reload your profile:
source ~/.bashrc

Troubleshooting
Common Issues and Solutions

Library Not Found Errors
If you receive errors about libraries not being found when running the application, ensure that LD_LIBRARY_PATH includes /usr/local/lib:
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

Redis Connection Issues
Verify that Redis is running:
sudo systemctl status redis-server

Check if Redis is listening on the correct port (default is 6379).

SRTP Key Errors
Ensure the SRTP_KEY environment variable is set and is the correct length (60 hexadecimal characters for a 30-byte key).
Verify that the key is in hexadecimal format without any spaces or separators.

MsQuic Build Errors
Ensure all MsQuic prerequisites are installed (ninja-build, clang, etc.).
Check the MsQuic GitHub repository for any updates or build instructions specific to your Ubuntu version.

Permission Denied Errors
Avoid running the application as root unless necessary.
Use ports above 1024 to avoid needing elevated privileges.
