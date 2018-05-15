FROM ubuntu
MAINTAINER rmspeers

RUN apt-get update
# Needed by KillerBee directly:
RUN apt-get install -y python-gtk2 python-cairo python-usb python-crypto python-serial python-dev libgcrypt-dev
# Needed for other dependencies being installed:
RUN apt-get install -y mercurial wget unzip

# Not strictly needed, but helpful to have on the system:
RUN apt-get install -y python-pip vim
#RUN pip install --upgrade pip

# Depdenencies not from package management:
WORKDIR /opt
RUN hg clone https://bitbucket.org/secdev/scapy-com
RUN cd scapy-com && python setup.py install

RUN mkdir /opt/sewio
WORKDIR /opt/sewio
RUN wget https://www.sewio.net/wp-content/uploads/productlist/OpenSniffer/generation2/PythonLib_OpenSniffer_beta_v0.1.zip
RUN unzip PythonLib_OpenSniffer_beta_v0.1.zip
RUN unzip OpenSniffer-0.1.zip
RUN cd OpenSniffer-0.1 && python setup.py install

# Install KillerBee
RUN mkdir /opt/killerbee
#ADD . /opt/killerbee
#RUN cd /opt/killerbee && python setup.py install

# NOTE: When you run this container, you likely need to pass --device=/dev/ttyUSB0 or the proper USB path.

WORKDIR /root
ENTRYPOINT bash