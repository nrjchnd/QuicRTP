FROM ubuntu:24.04

# Install dependencies
RUN apt-get update && \
    apt-get install -y build-essential cmake libsrtp2-dev libboost-all-dev libspdlog-dev libhiredis-dev libssl-dev

# Install Redis++
RUN apt-get install -y wget
RUN wget https://github.com/sewenew/redis-plus-plus/archive/refs/tags/1.3.4.tar.gz
RUN tar xzf 1.3.4.tar.gz && \
    cd redis-plus-plus-1.3.4 && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make && make install

# Copy source code
COPY . /app
WORKDIR /app

# Build the application
RUN mkdir build && cd build && \
    cmake .. && make

# Expose ports
EXPOSE 5000-5100/udp

# Run the application
CMD ["./build/QuicRtpTranslator"]
