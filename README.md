QuicRTP
QuicRTP is a high-performance application designed for asynchronous RTP packet handling with multithreading and SRTP support. It enables dynamic routing of RTP packets based on SSRC information, with the ability to scale horizontally using Redis for session management. The application supports both RTP and QUIC endpoints, providing flexibility in streaming environments.

Features
Asynchronous Operations: Utilizes asynchronous I/O for high performance.
Multithreading: Handles multiple endpoints concurrently using multithreading.
Horizontal Scaling: Integrates with Redis to store session and stream information, allowing for horizontal scaling across multiple instances.
Dynamic Routing: Routes RTP packets based on SSRC using cached information.
Multiple Endpoints: Supports multiple RTP and QUIC endpoints, configurable via /etc/quicrtp/quicrtp.conf.
SRTP Support: Handles SRTP encryption/decryption using libsrtp2.
Setup Instructions
For detailed setup instructions, refer to the INSTALL.md file.

Environment Variables
Before running the application, make sure to set the following environment variable:


export SRTP_KEY=your_srtp_key_here


Docker
To build and run the application using Docker:

docker-compose up --build

The application listens on UDP ports 5000-5100. You can simulate RTP traffic to any of these ports for testing.

Testing RTP Traffic
You can generate RTP traffic using tools like ffmpeg or rtpgen.

Example using ffmpeg:


ffmpeg -re -i input.wav -f rtp rtp://localhost:5000
Logs
The application logs can be viewed in the console, providing information about packet handling, errors, and session management.

Horizontal Scaling
To scale the application horizontally, adjust the number of replicas in the docker-compose.yml file as needed.