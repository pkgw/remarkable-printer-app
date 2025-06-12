# reMarkable Printer Application

This is a [“Printer Application”][paapl] that allows printing of files directly
to the [reMarkable Connect][rmc] (formerly “reMarkable Cloud”) service.

[paapl]: https://www.msweet.org/pappl
[rmc]: https://remarkable.com/shop/connect

Under the hood it is powered by the [rmapi] program.

[rmapi]: https://github.com/ddvk/rmapi

The implementation is derived from a combination of [OpenPrinting/ps-printer-app]
and [remarkable-cups].

[OpenPrinting/ps-printer-app]: https://github.com/OpenPrinting/ps-printer-app
[remarkable-cups]: https://github.com/ofosos/scratch/tree/master/remarkable-cups


## Snap Build on Fedora

Sigh: Docker interferes with snapd networking by default ...

```sh
# one-time prep:
sudo firewall-cmd --zone=trusted --change-interface=lxdbr0 --permanent
sudo firewall-cmd --reload
# once-per-boot prep (will presumably break Docker):
sudo iptables -F FORWARD
sudo ip6tables -F FORWARD
sudo iptables -P FORWARD ACCEPT
sudo ip6tables -P FORWARD ACCEPT
# actual build
snapcraft pack
```

Then to run:

```sh
sudo snap install --dangerous remarkable-printer-app_*.snap
```

Then visit: <http://localhost:8000>.


## Local build

```sh
gcc $(PKG_CONFIG_PATH=$(pwd)/pfx/lib/pkgconfig pkg-config --cflags --libs pappl) -O0 -g remarkable-printer-app.c -o remarkable-printer-app
LD_LIBRARY_PATH=$(pwd)/pfx/lib ./remarkable-printer-app server
```

https://www.msweet.org/pappl/pappl.html
