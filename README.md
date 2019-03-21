# RCI

Internet communication using UDP and TCP with peer to peer and client server

Missing:
  - stream_api : most of peer to peer protocol implementation & children connecting
  - iamroot : missing some POP queries
  - general udp coms: select socket with a timer before recv so it can timout if needed
  - TEST with nc on localhost (nc -l 127.0.0.1 59000) and use default root server (./iamroot streamname:127.0.0.1:59000 -d)
