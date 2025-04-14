# Dockerfile using DOWNLOADED AARCH64-HOSTED ARM GCC Toolchain

FROM debian:bookworm-slim AS builder

ARG DEBIAN_FRONTEND=noninteractive
# Toolchain path (update if desired)
ENV ARM_TOOLCHAIN_PATH=/opt/arm-gcc-14-aarch64 
# Changed name for clarity

# Install build essentials and download tools
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    ca-certificates \
    xz-utils \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# --- UPDATE THIS URL ---
# Download and unpack the AARCH64-HOSTED ARM GNU Toolchain targeting arm-none-linux-gnueabihf
# Find the correct URL from https://developer.arm.com/downloads/-/gnu-a
ARG TOOLCHAIN_URL="https://developer.arm.com/-/media/Files/downloads/gnu/12.2.rel1/binrel/arm-gnu-toolchain-12.2.rel1-aarch64-arm-none-linux-gnueabihf.tar.xz"

RUN echo "Downloading AARCH64 Toolchain from ${TOOLCHAIN_URL}..." && \
    mkdir -p ${ARM_TOOLCHAIN_PATH} && \
    curl -SL ${TOOLCHAIN_URL} -o toolchain.tar.xz && \
    echo "Unpacking Toolchain..." && \
    tar -xf toolchain.tar.xz --strip-components=1 -C ${ARM_TOOLCHAIN_PATH} && \
    rm toolchain.tar.xz

WORKDIR /work

# Set environment variables for CC/CXX pointing to the downloaded AARCH64 toolchain
# The triplet might still be arm-none-linux-gnueabihf even if host is aarch64
ENV CC=${ARM_TOOLCHAIN_PATH}/bin/arm-none-linux-gnueabihf-gcc
ENV CXX=${ARM_TOOLCHAIN_PATH}/bin/arm-none-linux-gnueabihf-g++
# Sysroot environment variable might not be needed if toolchain is self-contained
# ENV RPI_SYSROOT=${ARM_TOOLCHAIN_PATH}/arm-none-linux-gnueabihf/libc