FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6sql6-sqlite \
    libsqlite3-dev \
    cmake \
    pkg-config \
    xauth \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

RUN make clean || true && make

# Передаём путь к базе через переменную
CMD ["./monitor"]

