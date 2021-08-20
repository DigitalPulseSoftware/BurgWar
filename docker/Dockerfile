FROM debian:latest as build-env

# We need gcc 9+
RUN echo 'deb http://deb.debian.org/debian testing main' >> /etc/apt/sources.list 

# Update system
RUN apt update
RUN apt upgrade -y

# Install all we need ...
RUN apt install -y build-essential curl git unzip

# TEMP: Install nazara legacy dependencies
RUN apt-get install -y libopenal-dev libsndfile1-dev libfreetype6-dev libsdl2-dev libxcb-cursor-dev libxcb-ewmh-dev libxcb-keysyms1-dev libx11-dev mesa-common-dev libgl1-mesa-dev libassimp-dev

# Install xmake with root (so it will install dependencies)
RUN curl -fsSL https://xmake.io/shget.text | /bin/bash

# Add user
RUN useradd burgwar
RUN mkdir -p /home/burgwar
RUN chown -R burgwar:burgwar /home/burgwar

# That's ugly ... but we need it to install xmake :/
RUN mkdir -p /tmp/
RUN chmod -R 777 /tmp/ 

# Switch to burgwar user
USER burgwar
WORKDIR /home/burgwar

# Install xmake for burgwar user 
RUN curl -fsSL https://xmake.io/shget.text | /bin/bash
RUN /home/burgwar/.local/bin/xmake update -s dev

# Build server
RUN git clone https://github.com/DigitalPulseSoftware/BurgWar.git
WORKDIR /home/burgwar/BurgWar

COPY --chown=burgwar:burgwar xmake.lua xmake.lua

RUN /home/burgwar/.local/bin/xmake config --mode=releasedbg -y
RUN /home/burgwar/.local/bin/xmake -r BurgWarServer

# Compile every default map
RUN /home/burgwar/.local/bin/xmake -r BurgWarMapTool
RUN /home/burgwar/.local/bin/xmake run BurgWarMapTool -c /home/burgwar/BurgWar/maps/*

RUN /home/burgwar/.local/bin/xmake install -v -o build/ BurgWarServer

##############################
# Runtime image
##############################
FROM debian:latest

LABEL org.opencontainers.image.authors="Jerome Leclercq;Axel \"Elanis\" Soupe"
EXPOSE 14768/udp
HEALTHCHECK --interval=1m --timeout=3s CMD netstat -nltpu | grep -c 14768

ENV LD_LIBRARY_PATH=/home/burgwar/lib/

# We need gcc 9+
RUN echo 'deb http://deb.debian.org/debian testing main' >> /etc/apt/sources.list
RUN apt update
RUN apt install -y libgcc-s1 net-tools

# TEMP: Install nazara legacy dependencies
RUN apt-get install -y libopenal-dev libsndfile1-dev libfreetype6-dev libsdl2-dev libxcb-cursor-dev libxcb-ewmh-dev libxcb-keysyms1-dev libx11-dev mesa-common-dev libgl1-mesa-dev libassimp-dev

# Add user
RUN useradd burgwar
RUN mkdir -p /home/burgwar
RUN chown -R burgwar:burgwar /home/burgwar

USER burgwar
WORKDIR /home/burgwar 

COPY --from=build-env /home/burgwar/BurgWar/build/ .

# TEMP: prepare blank server
COPY serverconfig.lua serverconfig.lua
COPY assets/ assets/

# Copy mods and scripts from bw repo
COPY --from=build-env /home/burgwar/BurgWar/mods/ mods/
COPY --from=build-env /home/burgwar/BurgWar/scripts/ scripts/

# Copy every default map
COPY --from=build-env /home/burgwar/BurgWar/bin/linux_x86_64_releasedbg/*.bmap /home/burgwar/

# Set entrypoint
ENTRYPOINT /home/burgwar/bin/BurgWarServer
