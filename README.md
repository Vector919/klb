### KLB

klb is a simple load balancer written in C.

Currently, klb balances traffic betweeen any hosts passed to it via the
command line

Example:
```bash
./klb 9000 localhost www.google.com
# balances traffic between localhost and google

> Started server on port 9000!
```