# Networking support in cpp-lib

The NETWORK component (`sys/network.h`) provides very easy to use
classes for TCP connections and UDP packets.  It is roughly based on
the [Networking Proposal for TR2] [1].  This proposal can be used 
as reference for NETWORK, however there there are some differences:

- cpp-lib uses the namespace `cpl::util::network`, not `std::tr2`.
- cpp-lib doesn't implement the `network_error` exceptions, but throws
  `std::runtime_error` instead.
- I/O multiplexing (`iowait()`) is not implemented.
- The UDP (`datagram_sender`/`datagram_receiver`) doc is currently
  out of date, future code will use `datagram_socket`.

The implementation transparently supports IPv4 and IPv6.

IPv6 addresses with port numbers use square brackets as described 
[here] [2]:
  `[2001:db8:85a3:8d3:1319:8a2e:370:7348]:443`


## Examples

Please see `testing/tcp-text.cpp` and `testing/udp-test.cpp` for very
straightforward server and client code.  Execute `make` and `make tests`
first.


### TCP

* To get the current time from NIST daytime servers:
  
  `bin/opt/tcp-test daytime`

* To download a web site:

  `bin/opt/tcp-test wget http://www.boost.org/ > foo.html`
  
  `less foo.html`

* To start a multi-threaded string reversal server listening on port 4711:

  `bin/opt/tcp-test reverse 4711`

  Connect to the server:

  `telnet localhost 4711`, or:
  `bin/opt/tcp-test telnet localhost 4711`

* A simple telnet client:

  `bin/opt/tcp-test telnet localhost 4711`
  `bin/opt/tcp-test telnet your.telnet.server 80`

* Run a hello world server and receive a greeting using netcat (nc):

  `bin/opt/tcp-test hello 12345`
  `nc localhost 12345`

### UDP

To receive data on port 4711 using IPv6:

  `bin/opt/udp-test receive ip6 4711`

To send data to port 4711:

  `bin/opt/udp-test send ip6 ::1 4711`

A simple pong server that listens for incoming packets and sends a reply:
  `bin/opt/udp-test pong ip6 4711`

In another window, you can send a message and receive the reply:
  `bin/opt/udp-test ping ip6 ::1 4711 "Hello world"`

Use ip4 and 127.0.0.1 instead of ::1 and 4711 if your system doesn't 
support IPv6.



### DNS

* To resolve a datagram address:

  `bin/opt/udp-test resolve google.com 4711`

  Notice that multiple node/service pairs may be returned.

## Bugs

See [TODO](TODO.md). 

[1]: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1925.pdf
[2]: http://en.wikipedia.org/wiki/IPv6_address
