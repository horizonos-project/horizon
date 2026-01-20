FROM debian:stable-slim

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        e2fsprogs \
        dosfstools \
        util-linux \
        coreutils && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /work
