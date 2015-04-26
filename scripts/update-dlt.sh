echo -n "# "
grep "Header:" /usr/include/pcap-bpf.h | head -1
grep "#define DLT" /usr/include/pcap/bpf.h | sed 's/#define //' | sed 's/\/\*/#/' | sed 's/\*\///' | sed 's/\t/    /' | sed 's/ / =/'
