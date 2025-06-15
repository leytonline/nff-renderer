# Determine platform
OS=$(uname)

# Choose Makefile based on OS
if [[ "$OS" == "Linux" ]]; then
    cp Makefiles/Makefile_wsl Makefile
elif [[ "$OS" == "Darwin" ]]; then
    cp Makefiles/Makefile_mac Makefile
else
    echo "Unsupported OS: $OS"
    exit 1
fi

echo "Makefile configured for $OS"