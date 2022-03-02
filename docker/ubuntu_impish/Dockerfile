FROM ubuntu:impish
COPY build.sh /root
RUN chmod +x /root/build.sh

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC
RUN apt-get update
RUN apt-get install -y git cmake g++ qtbase5-dev libmuparser-dev file dpkg-dev libshp-dev
