FROM debian

WORKDIR /service

COPY src ./src
COPY streams ./streams

RUN apt-get update
RUN apt-get -y install g++ dpkg-dev doxygen dos2unix graphviz curl pcscd libpcsclite-dev libcppunit-dev libcurl3 libcurl3-dev

ENV PATH="/root/usr/local/bin:${PATH}"

WORKDIR /service/src
RUN make install SYSPREFIX=$HOME/usr/local

WORKDIR /service
