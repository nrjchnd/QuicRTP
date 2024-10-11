# Use the official Ubuntu 24.04 LTS as the base image
FROM ubuntu:24.04

# Set environment variables to avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Update package lists and install essential build tools and libraries
RUN apt-get update && \
    apt-get install -y \
        build-essential \
        cmake \
        git \
        wget \
        ninja-build \
        clang \
        libboost-all-dev \
        libsrtp2-dev \
        libspdlog-dev \
        libssl-dev \
        libboost-system-dev \
        libboost-thread-dev && \
    rm -rf /var/lib/apt/lists/*

# Clone and build MsQuic
RUN git clone https://github.com/microsoft/msquic.git && \
    cd msquic && \
    git submodule update --init --recursive && \
    mkdir build && cd build && \
    cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release && \
    ninja && \
    ninja install && \
    cd ../.. && \
    rm -rf msquic

# Install Hiredis from source
RUN cd /usr/src && \
    git clone https://github.com/redis/hiredis.git && \
    cd hiredis && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make && \
    make install && \
    cd ../.. && \
    rm -rf /usr/src/hiredis

# Install Redis++ (redis-plus-plus)
RUN wget https://github.com/sewenew/redis-plus-plus/archive/refs/tags/1.3.4.tar.gz && \
    tar xzf 1.3.4.tar.gz && \
    cd redis-plus-plus-1.3.4 && \
    # Apply necessary patch for Ubuntu 24.04
    sed -i '/#include "cxx_utils.h"/a #include <cstdint>' src/sw/redis++/utils.h && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make && \
    make install && \
    cd ../.. && \
    rm -rf redis-plus-plus-1.3.4 1.3.4.tar.gz

# Update the dynamic linker run-time bindings
RUN echo "/usr/local/lib" > /etc/ld.so.conf.d/local.conf && \
    ldconfig

# Create symbolic links for Hiredis libraries
RUN ln -s /usr/local/lib/libhiredis.so /usr/lib/x86_64-linux-gnu/libhiredis.so && \
    ln -s /usr/local/lib/libhiredis.so.1.2.1-dev /usr/local/lib/libhiredis.so.1.1.0 && \
    ln -s /usr/local/lib/libhiredis.so.1.2.1-dev /usr/lib/x86_64-linux-gnu/libhiredis.so.1.1.0

# Copy source code
COPY . /app
WORKDIR /app

# Build the application
RUN mkdir build && cd build && \
    cmake .. && make && make install

# Expose ports
EXPOSE 5000-5100/udp 5000-5100/tcp 4433/udp 4433/tcp

# Run the application
CMD ["QuicRTP"]
