FROM ubuntu
MAINTAINER rmspeers

RUN apt-get update
RUN apt-get install -y python-gtk2 python-cairo python-usb python-crypto python-serial python-dev libgcrypt-dev mercurial

RUN hg clone https://bitbucket.org/secdev/scapy-com
RUN cd scapy-com && python setup.py install

RUN mkdir /opt/killerbee
ADD . /opt/killerbee
RUN cd /opt/killerbee && python setup.py install

# NOTE: When you run this container, you likely need to pass --device=/dev/ttyUSB0 or the proper USB path.

WORKDIR /root
ENTRYPOINT bash