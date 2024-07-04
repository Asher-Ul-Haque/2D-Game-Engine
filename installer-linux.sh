#!/bin/bash
# Written by chatGPT 4-o

# Function to determine the appropriate shell configuration file
get_shell_config_file() {
    local shell_name
    shell_name=$(basename "$SHELL")
    if [[ "$shell_name" == "zsh" ]]; then
        echo "$HOME/.zshrc"
    elif [[ "$shell_name" == "bash" ]]; then
        echo "$HOME/.bashrc"
    elif [[ "$shell_name" == "fish" ]]; then
        echo "$HOME/.config/fish/config.fish"
    else
        echo "$HOME/.profile"
    fi
}

# Function to install Vulkan SDK from the tarball
install_vulkan_sdk() {
    echo "Installing Vulkan SDK from tarball..."

    # Extract the tarball into the user's home directory
    tar -xvf VulkanLinux.tar.gz -C "$HOME"

    # Assume the tarball extracts to a directory named VulkanSDK in the user's home directory
    VULKAN_SDK_DIR="$HOME/Vulkan"

    # Navigate into the extracted Vulkan SDK directory and run setup-env.sh
    cd "$VULKAN_SDK_DIR" || { echo "Failed to enter the VulkanSDK directory"; exit 1; }
    source setup-env.sh
}

# Function to install required packages for Debian-based systems
install_packages_debian() {
    echo "Updating package list and installing required packages for Debian-based systems..."
    sudo apt-get update
    sudo apt-get install -y git clang make cmake libx11-xcb-dev libxkbcommon-x11-dev wget libvulkan-dev vulkan-tools vulkan-validationlayers
    sudo apt-get autoremove
}

# Function to install required packages for Arch-based systems
install_packages_arch() {
    echo "Updating package list and installing required packages for Arch-based systems..."
    sudo pacman -Syu --noconfirm
    sudo pacman -S --noconfirm git clang make cmake libx11-xcb libxkbcommon-x11 wget libvulkan vulkan-tools vulkan-validationlayers
}

# Determine the package manager and install required packages
if command -v apt-get &> /dev/null; then
    install_packages_debian
elif command -v pacman &> /dev/null; then
    install_packages_arch
else
    echo "Unsupported package manager. Please install the required packages manually."
    exit 1
fi

# Install Vulkan SDK from tarball
install_vulkan_sdk

# Clone the specified GitHub repository
echo "Cloning the Just-Forge-Engine repository..."
git clone -b Just-Forge-3D https://github.com/Asher-Ul-Haque/Just-Forge-Engine.git

# Change directory into the cloned repository
cd Just-Forge-Engine || { echo "Failed to enter the Just-Forge-Engine directory"; exit 1; }

# Make the build script executable
echo "Making build-all.sh executable..."
chmod +x build-all.sh

# Run the build script
echo "Running build-all.sh..."
./build-all.sh

# Change directory into the build directory
cd build || { echo "Failed to enter the build directory"; exit 1; }

# Run the Just_Forge_Tester program
echo "Running Just_Forge_Tester..."
./Just_Forge_Tester
