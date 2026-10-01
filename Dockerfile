# --- Stage 1: Build & Test ---
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files
COPY . .

# Build executable and run GoogleTest suite
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build
RUN ctest --test-dir build --output-on-failure

# --- Stage 2: Minimal Runtime Image ---
FROM ubuntu:22.04 AS runner

WORKDIR /app

# Copy only the compiled binary from builder stage
COPY --from=builder /app/build/lightwave .

# Run daemon
CMD ["./lightwave"]
