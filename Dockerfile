# Base image: Ubuntu LTS
FROM ubuntu

# Avoid interactive prompts during package installs
ENV DEBIAN_FRONTEND=noninteractive

# Update and install core build tools
# Core build tools
RUN apt-get update && apt-get install -y \
  build-essential \
  gdb \
  cmake \
  pkg-config \
  curl \
  # C libraries
  libgtk-4-dev \
  libcurl4-openssl-dev \
  libcjson-dev \
  libssl-dev \
  zlib1g-dev \
  libjansson-dev \
  libsqlite3-dev \
  && apt-get clean && rm -rf /var/lib/apt/lists/*

# Set a working directory
WORKDIR /app

# Default command: interactive shell
CMD ["/bin/bash"]