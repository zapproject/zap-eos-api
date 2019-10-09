FROM ubuntu:18.04

RUN apt update
RUN apt install apt-utils -y
RUN apt install bash -y
RUN apt install cmake -y
RUN apt install wget -y

RUN apt install -y nodejs npm

RUN wget https://github.com/eosio/eos/releases/download/v1.7.0/eosio_1.7.0-1-ubuntu-18.04_amd64.deb && apt install ./eosio_1.7.0-1-ubuntu-18.04_amd64.deb -y
RUN wget https://github.com/EOSIO/eosio.cdt/releases/download/v1.5.0/eosio.cdt_1.5.0-1_amd64.deb && apt install ./eosio.cdt_1.5.0-1_amd64.deb -y

