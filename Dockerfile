# Multi-stage Docker build for RecipeForADisaster
# Build stage: Install dependencies and build application
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    git \
    cmake \
    build-essential \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
RUN git clone https://github.com/Microsoft/vcpkg.git /vcpkg && \
    cd /vcpkg && \
    ./bootstrap-vcpkg.sh

# Install C++ dependencies via vcpkg
RUN cd /vcpkg && \
    ./vcpkg install \
    asio \
    boost-system \
    boost-date-time \
    curl \
    openssl \
    azure-core-cpp \
    nlohmann-json \
    crow \
    utf8proc \
    zlib \
    --triplet x64-linux

# Copy source code
WORKDIR /src
COPY . .

# Build application
RUN cmake -S . -B /build \
    -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release && \
    cmake --build /build --config Release

# Runtime stage: Minimal image with application and libraries
FROM ubuntu:22.04 AS runtime

# Install minimal runtime dependencies
RUN apt-get update && apt-get install -y \
    libcurl4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Copy vcpkg-installed libraries
COPY --from=builder /vcpkg/installed/x64-linux/lib/ /usr/local/lib/

# Copy built application
COPY --from=builder /build/web_server /app/web_server
COPY --from=builder /build/RecipeForADisaster /app/RecipeForADisaster

# Set library path
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Create app directory
WORKDIR /app

# Expose web server port
EXPOSE 8080

# Default command
CMD ["/app/web_server"]