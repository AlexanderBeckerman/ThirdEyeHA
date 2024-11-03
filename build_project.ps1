# Create the build directory if it doesn't exist
if (!(Test-Path -Path "./build")) {
    New-Item -ItemType Directory -Path "./build"
}

# Navigate to the build directory
Set-Location -Path "./build"

# Run CMake to configure and build the project
cmake ..
cmake --build .

# Return to the original directory
Set-Location -Path ".."
