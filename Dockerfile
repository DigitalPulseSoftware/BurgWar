FROM ubuntu:20.04
MAINTAINER SirMishaa

RUN apt update && apt upgrade -y && apt install curl -y
RUN apt install git -y

WORKDIR /usr/lib

RUN git clone https://github.com/DigitalPulseSoftware/NazaraEngine NazaraEngine

# Configure env for tzdata
ENV TZ=Europe/Brussels
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt install libopenal-dev libsndfile1-dev libfreetype6-dev libxcb-cursor-dev libxcb-ewmh-dev libxcb-randr0-dev \
	libxcb-icccm4-dev libxcb-keysyms1-dev libx11-dev mesa-common-dev libgl1-mesa-dev libassimp-dev libsdl2-dev -y

WORKDIR /usr/lib/NazaraEngine/build

RUN ./premake5-linux64 gmake

WORKDIR /usr/lib/NazaraEngine/build/gmake

# Install GCC
RUN apt install build-essential -y
RUN make -j4
