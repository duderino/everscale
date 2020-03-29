# setup
sudo apt-get update && apt-get install linux-tools-5.3.0-42-generic

cd ~/src
git clone https://github.com/brendangregg/FlameGraph

set build profile in profile.cmake to RELWITHDEBINFO

# measure

# cpu
sudo rm /tmp/test.*; sudo perf record -o /tmp/test.data -F 99 -g -- ./http/proxy/http-client-server-test -c 500 -i 5000 -r 1
# malloc - not useful
sudo rm /tmp/test.*; sudo perf record -o /tmp/test.data -e syscalls:sys_enter_brk -a -g -- ./http/proxy/http-client-server-test -c 500 -i 5000 -r 1
# page faults - not useful
sudo rm /tmp/test.*; sudo perf record -o /tmp/test.data -e page-faults -a -g -- ./http/proxy/http-client-server-test -c 500 -i 5000 -r 1

# summarize

sudo chmod 444 /tmp/test.data; perf script -i /tmp/test.data --demangle >> /tmp/test.perf; ~/src/FlameGraph/stackcollapse-perf.pl /tmp/test.perf | c++filt -n | ~/src/FlameGraph/flamegraph.pl --cp > /tmp/test.svg

in browser open file:///tmp/test.svg
