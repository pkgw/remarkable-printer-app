# reMarkable Printer Application

This is a [“Printer Application”][pappl] that allows “printing” documents
directly to a [reMarkable] device, via the [reMarkable Connect][rmc]
([my.remarkable.com](https://my.remarkable.com/)) service.

[pappl]: https://www.msweet.org/pappl
[reMarkable]: https://remarkable.com/
[rmc]: https://remarkable.com/shop/connect

Under the hood it is powered by the [rmapi] program. The implementation is
derived from a combination of [OpenPrinting/ps-printer-app] and
[remarkable-cups], although in the end it shares very little code with either.

[rmapi]: https://github.com/ddvk/rmapi
[OpenPrinting/ps-printer-app]: https://github.com/OpenPrinting/ps-printer-app
[remarkable-cups]: https://github.com/ofosos/scratch/tree/master/remarkable-cups


## Development/polish status

This package ought to work reliably, but it was built for personal use, so it
hasn’t (yet?) been published in formal channels like [the Snap
Store][snapstore]. Installation instructions and documentation are bare-bones
(this README is it).

[snapstore]: https://snapcraft.io/store


## Basic Installation and Usage

The recommended way to install Printer Applications is, apparently, the [snap]
app packaging system. As mentioned above, this app is not yet formally published
to the Snap Store, so you’ll have to build and install it yourself:

[snap]: https://snapcraft.io/

1. Install [snap] and [snapcraft], the snap build tool
1. Check out this repo
1. Run `snapcraft pack` to build the app’s snap
1. Run `sudo snap install --dangerous remarkable-printer-app_*.snap` to install it

After the snap is installed, set it up as follows:

1. Run `sudo remarkable-printer-app login`
   - This will prompt you to go to the
     [my.remarkable.com](https://my.remarkable.com/) site and get a device
     connection code
1. Run `sudo snap enable remarkable-printer-app`
1. Run `sudo snap restart remarkable-printer-app` just to be safe

[snapcraft]: https://documentation.ubuntu.com/snapcraft/stable/

At this point, if your system can auto-discover printers, it will hopefully
start showing you a new printer called “reMarkable Connect” or
“reMarkable_Connect”. And you can hopefully start “printing” files to it!

You can also run `remarkable-printer-app path/to/a/pdf-file` to print a PDF at
the command line.

Like other Printer Apps, this app also provides a web management UI at
<http://localhost:8000/>. However, since this particular app has very little
flexibility, you should rarely if ever need to use the UI.


## Details

### Customizing the Print Destination

You can provide a `destdir` option to the `remarkable-printer-app login` command
to specify where your printouts will “land” on your reMarkable. The destination
should be specified in the style of a Unix-like absolute path. For instance:

```sh
sudo remarkable-printer-app login -o destdir=/Printouts
```

This will cause printed files to appear in the “Printouts” folder on your
device.

### Snap Networking and Docker

It appears that the default networking setup used by the [snap] system can be
incompatible with that used by [Docker CE][dce] on Linux (and maybe other
OS's?). In some cases you may need to reset your forwarding rules:

[dce]: https://docs.docker.com/engine/

```sh
sudo iptables -F FORWARD
sudo iptables -P FORWARD ACCEPT
```

On Fedora Linux I also needed to run some one-time commands to mark the LXD
bridge interface as trusted:

```sh
sudo firewall-cmd --zone=trusted --change-interface=lxdbr0 --permanent
sudo firewall-cmd --reload
```

### Printing Things That Aren’t PDFs

At the lowest level, this app can only accept PDF file inputs. Hopefully, your
computer will understand this and be able to pre-render other kinds of documents to
PDF before sending them to the reMarkable Printer App … but if there are times when
it can’t, printing will fail.

**TODO:** The “paper size” of the reMarkable printer is configured to be US Letter,
which is not the actual size of a reMarkable's display. A different size should yield
outputs that fit the reMarkable screen better.

### The `remarkable-printer-app` CLI

Installing the snap makes a `remarkable-printer-app` command-line tool
available. It provides a number of subcommands besides the `login` command
mentioned above. But, you will rarely need to use anything besides `login` and
the default `submit` mode.


## Credits

The first version of the reMarkable Printer App was created by Peter K. G.
Williams. The initial code structure was derived from
[OpenPrinting/ps-printer-app], by Till Kamppeter, which in turn was derived from
[hp-printer-app](https://github.com/michaelrsweet/hp-printer-app) by Michael R.
Sweet, who is also the author of the [PAPPL][pappl] library used by this app.


## Legalities

This software is provided under the terms of the [Apache License
2.0](https://www.apache.org/licenses/LICENSE-2.0).

