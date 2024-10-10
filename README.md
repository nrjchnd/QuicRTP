Instructions

Note: REFER TO INSTALL.MD for thorough setup instructions. 

For testing: 
Set Environment Variables:
export SRTP_KEY=your_srtp_key_here
docker-compose up --build
Testing:
The application listens on UDP ports 5000-5100.
You can simulate RTP traffic to any of these port

Generate RTP Traffic:

Use tools like ffmpeg or rtpgen to send RTP packets to the dynamic ports.

For example:

ffmpeg -re -i input.wav -f rtp rtp://localhost:5000

The application logs can be viewed in the console.
Logs include information about packet handling, errors, and session management.

The application can be scaled horizontally by increasing the number of replicas.
Update docker-compose.yml accordingly




Manual Install: 
Install Dependencies:

Install libsrtp2: sudo apt-get install libsrtp2-dev
Install MsQuic: Follow the MsQuic build instructions.
Install Redis++: Follow the Redis++ installation guide.
Install Boost.Asio: sudo apt-get install libboost-system-dev libboost-thread-dev
Build the Project:

Create a build directory: 
mkdir build && cd build
Run CMake: cmake ..
Build: make
Run the Application:

Ensure Redis server is running.
Place config.conf in the executable directory.
Run the executable: ./QuicRtpTranslator

Features
Asynchronous Operations: Utilizes asynchronous I/O for high performance.
Multithreading: Employs multithreading to handle multiple endpoints concurrently.
Horizontal Scaling: Integrates with Redis to store session and stream information, allowing for horizontal scaling across multiple instances.
Dynamic Routing: Routes RTP packets based on SSRC using cached information.
Multiple Endpoints: Supports multiple RTP and QUIC endpoints defined in the configuration file.
SRTP Support: Handles SRTP encryption/decryption using libsrtp2.


