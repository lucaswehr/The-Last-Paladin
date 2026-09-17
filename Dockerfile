FROM ubuntu:24.04

# 1. Install dependencies
RUN apt-get update && \
    apt-get install -y \
    g++ \
    cmake \
    git \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev \
    libogg-dev \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 2. Download SFML 3.0.0
RUN git clone --branch 3.0.0 --depth 1 \
    https://github.com/SFML/SFML.git /tmp/SFML

# 3. Configure SFML
RUN cmake -S /tmp/SFML -B /tmp/SFML/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DSFML_BUILD_EXAMPLES=OFF \
    -DSFML_BUILD_TEST_SUITE=OFF

# 4. Build SFML
RUN cmake --build /tmp/SFML/build -j$(nproc)

# 5. Show what SFML actually built
RUN find /tmp/SFML/build -type f \
    \( -name 'libsfml*.so*' -o -name 'libsfml*.a' \) -print

# 6. Install SFML
RUN cmake --install /tmp/SFML/build

RUN ldconfig

# 7. Show what was installed
RUN find /usr/local -type f \
    \( -name 'libsfml*.so*' -o -name 'libsfml*.a' \) -print

# 8. Copy your project
COPY ["2D Sidescroller/", "/app/"]

# 9. Compile your server
# 9. Compile your server
RUN g++ -std=c++17 \
    Arrow.cpp \
    AssetManager.cpp \
    Client.cpp \
    DeflectionDemo.cpp \
    Dragon.cpp \
    Knight.cpp \
    Level.cpp \
    LobbyClient.cpp \
    LobbyServer.cpp \
    main.cpp \
    ParryDemo.cpp \
    Player.cpp \
    Samurai.cpp \
    Skeleton.cpp \
    text.cpp \
    Tile.cpp \
    wolf.cpp \
    -o game_server \
    -I/usr/local/include \
    -L/usr/local/lib \
    -lsfml-graphics \
    -lsfml-window \
    -lsfml-audio \
    -lsfml-network \
    -lsfml-system

# 10. Run the server
CMD ["./game_server", "server"]