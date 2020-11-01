FROM debian:stable
MAINTAINER SirMishaa

# Update packages and install GCC and git
RUN apt update && apt upgrade -y && apt install build-essential git -y

# Nazara and Burgwar dependencies
RUN apt install libopenal-dev libsndfile1-dev libfreetype6-dev libassimp-dev libsdl2-dev libxcb-ewmh-dev libxcb-keysyms1-dev \
                qtbase5-dev libcurl4-gnutls-dev \
				-y

# Configure env for tzdata
ENV TZ=Europe/Brussels
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Build Nazara

WORKDIR /usr/lib

RUN git clone --depth 1 -b master https://github.com/DigitalPulseSoftware/NazaraEngine

WORKDIR /usr/lib/NazaraEngine/build

RUN ./premake5-linux64 gmake

WORKDIR /usr/lib/NazaraEngine/build/gmake

RUN make -j8

WORKDIR /usr/lib/NazaraEngine/build

RUN ./premake5-linux64 package

# Build Burgwar

WORKDIR /root

RUN git clone --depth 1 -b master https://github.com/DigitalPulseSoftware/BurgWar

WORKDIR /root/BurgWar/build

COPY config-docker.lua config.lua

RUN ./premake5-linux64 gmake2

WORKDIR /root/BurgWar/build/gmake2

RUN make -j8
