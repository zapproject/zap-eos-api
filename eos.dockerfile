FROM ubuntu:16.04

VOLUME ["/app"]

COPY ./ /app

RUN apt-get update
RUN apt-get install apt-utils -y
RUN apt-get install bash -y
RUN apt-get install cmake -y
RUN apt-get install wget -y

RUN wget https://github.com/eosio/eos/releases/download/v1.5.0/eosio_1.5.0-1-ubuntu-18.04_amd64.deb && apt-get install ./eosio_1.5.0-1-ubuntu-18.04_amd64.deb -y
RUN wget https://github.com/EOSIO/eosio.cdt/releases/download/v1.4.1/eosio.cdt-1.4.1.x86_64.deb && apt-get install ./eosio.cdt-1.4.1.x86_64.deb -y

RUN apt-get install nodejs -y
RUN apt-get install npm -y

WORKDIR /app