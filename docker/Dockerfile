FROM debian:latest

LABEL org.opencontainers.image.authors="Jerome Leclercq;Axel "Elanis" Soupe"
EXPOSE 14768/udp
HEALTHCHECK --interval=1m --timeout=3s CMD netstat -nltpu | grep -c 14768

# We need gcc 9+
RUN echo 'deb http://deb.debian.org/debian testing main' >> /etc/apt/sources.list 

# Update system
RUN apt update
RUN apt upgrade -y

# Install all we need ...
RUN apt install -y build-essential curl git net-tools unzip

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

USER burgwar
WORKDIR /home/burgwar

# Install xmake for use 
RUN curl -fsSL https://xmake.io/shget.text | /bin/bash

# Build server
RUN git clone https://github.com/DigitalPulseSoftware/BurgWar.git
WORKDIR /home/burgwar/BurgWar

COPY --chown=burgwar:burgwar xmake.lua xmake.lua

RUN /home/burgwar/.local/bin/xmake config --mode=releasedbg -y
RUN /home/burgwar/.local/bin/xmake -r BurgWarServer
RUN /home/burgwar/.local/bin/xmake -r BurgWarMapTool

# TMP: prepare blank server
RUN /home/burgwar/.local/bin/xmake run BurgWarMapTool ../../maps/beta_map -c beta_map.bmap
#RUN cp serverconfig.lua bin/linux_x86_64_releasedbg/
COPY serverconfig.lua bin/linux_x86_64_releasedbg/serverconfig.lua
COPY assets/ assets/

ENTRYPOINT /home/burgwar/.local/bin/xmake run BurgWarServer

