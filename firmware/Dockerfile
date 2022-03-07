FROM ubuntu:latest

WORKDIR /opt

RUN apt-get update && apt-get install -y \
  git \
  curl \
  python2 \
  gcc-msp430 \
  make

RUN curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py \
    python2 get-pip.py

RUN pip2 install \
    pyserial

ENV board goodfet41

RUN git clone https://github.com/travisgoodspeed/goodfet.git && \
    cd goodfet/firmware && \
    ./configure && \
    make clean && make
    

