ARG UBUNTU_VERSION=focal
FROM ubuntu:${UBUNTU_VERSION}

ARG DEBIAN_FRONTEND="noninteractive"
ENV TZ=America/New_York

RUN apt-get update && apt-get upgrade -y && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    build-essential devscripts debhelper fakeroot cdbs software-properties-common cmake \
    && rm -rf /var/lib/apt/lists/*

RUN add-apt-repository ppa:mutlaqja/ppa

RUN apt-get update && apt-get install -y \
    libindi-dev libnova-dev libz-dev libgsl-dev \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /src
RUN mkdir -p /build
WORKDIR /build
