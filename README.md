Instructions
Install Dependencies:

Install libsrtp2: sudo apt-get install libsrtp2-dev
Install MsQuic: Follow the MsQuic build instructions.
Install Redis++: Follow the Redis++ installation guide.
Install Boost.Asio: sudo apt-get install libboost-system-dev libboost-thread-dev
Build the Project:

Create a build directory: mkdir build && cd build
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

Notes
Security: Proper key handling for SRTP is essential. Ensure keys are securely managed.
Error Handling: Additional error handling and logging should be implemented for production use.
Performance Tuning: Optimize thread pool sizes, buffer sizes, and other parameters based on workload.
Testing: Thorough testing with various RTP/QUIC endpoints is recommended.


