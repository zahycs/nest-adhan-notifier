FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

# Install system dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    git \
    curl \
    udev \
    # Serial / USB tools
    usbutils \
    minicom \
    screen \
    # Build essentials (needed by PlatformIO native toolchain)
    build-essential \
    pkg-config \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Add vscode user to dialout group so it can access serial ports (/dev/ttyUSB*, /dev/ttyACM*)
RUN usermod -aG dialout vscode

# Install PlatformIO Core (CLI) for the vscode user
USER vscode
RUN curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o /tmp/get-platformio.py \
    && python3 /tmp/get-platformio.py \
    && rm /tmp/get-platformio.py

# Add PlatformIO to PATH for non-interactive shells
ENV PATH="/home/vscode/.platformio/penv/bin:${PATH}"

# Pre-install the Espressif32 platform and common ESP32 toolchain so the
# first build inside the container doesn't need a full download.
RUN /home/vscode/.platformio/penv/bin/pio platform install espressif32

USER root
