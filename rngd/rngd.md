# Overview

Rng-Tools (aka rngd) is a utility that can inject entropy from hardware sources, prngs, and http streams into the system. It is capable of adding random numbers to system devices such as `/dev/random` and `/dev/urandom`, as well as user-defined nodes and files.

Qrypt collaborated with the project maintainer to add a Qrypt source to rngd which samples quantum entropy from the Qrypt API at https://portal.qrypt.com .

This guide will provide an overview of the usage and setup of the Qrypt entropy source.

**Please note:**
- An API token from the Qrypt portal is required to use the Qrypt source. The token must be saved to a file and its path must be supplied to rngd.
- Docker VMs generally lack the devices needed to use rngd.
- The system will always stream its own crng output into `/dev/random` and `/dev/urandom`, even when rngd is running. If you need a random device that exclusively uses quantum random, please create a user-defined node created using `mknod`
- Recent changes to the Linux crng allow it to generate random numbers on-demand when depleted, instead of blocking. The new crng coexists gracefully with rngd, but an older crng (such as the one in the Ubuntu 18.04 LTS) provides a better visual representation of rngd's entropy augmentation.

# Installation

1. Clone the latest rng-tools master from GitHub
`git clone https://github.com/nhorman/rng-tools`

1. Install rng-tools dependencies. Additional packages may be required, depending on linux distro. The configure script below will name any missing packages it encounters.
```
sudo apt install \
    make \
    libtool \
    libxml2-dev \
    libssl-dev \
    libcurl3-dev \
    libp11-dev \
    librtlsdr-dev \
    libusb-1.0-0-dev \
    libjansson-dev \
    libcap-dev
```

3. `./autogen.sh`

1. `./configure` (add `--disable-dependency-tracking` if needed)

1. `make`

1. `sudo make install`

1. Verify installation (make note of executable path):
`which rngd`

1. Go to https://portal.qrypt.com to generate an API token with the "Entropy" context and save it to an easily accessible file. Make note of the file path.

# Usage

The resulting `rngd` executable can run directly to start either a daemon or a foreground process. By default, rngd will run as a background daemon and attempt to use the "hwrng", "errand", "pkcs11", and "rtlsdr" random sources.

To run rngd using exclusively the Qrypt source, use:
`sudo rngd -f -x hwrng -x rdrand -x pkcs11 -x rtlsdr -n qrypt -O qrypt:tokenfile:<qrypt token path>`

The above command will run rngd as a foreground process with the Qrypt source enabled and all other entropy sources disabled. rngd will send its random to the `/dev/random` device.

## Arguments
The following arguments are useful for configuring rngd when running the Qrypt source. See the `--help` menu for more details about the complete list of options.
- `-f`: Run rngd as a foreground process. If omitted, rngd will run as a background daemon
- `-o <path>`: Device or file for the random number output. (Default: `/dev/random`)
- `-x <source>`: Disables the specified source if enabled. (Example: `-x hwrng`)
- `-n <source>`: Enables the specified source. (Example: `-n qrypt`)
- `-O <source>:<key>:<value>`: Sets a source specific configuration option. (Example: `-O qrypt:tokenfile:/etc/rngd/qrypt.token`)


# Service Setup
 Rig-Tools also comes packaged with an `rngd.service` file for setting up a systemd service.

To configure rngd so it will automatically start the Qrypt source on boot, follow these steps:

1. Save your Qrypt api token to a system-accessible directory, such as `/etc/rngd/qrypt.token`

1. Edit `rngd.service` to add Qrypt arguments and options:
```
[Unit]
Description=Hardware RNG Entropy Gatherer Daemon
ConditionVirtualization=!container

# The "-f" option is required for the systemd service rngd to work with Type=simple
[Service]
Type=simple
ExecStart=<rngd install path> -f -x hwrng -x rdrand -x pkcs11 -x rtlsdr -n qrypt -O qrypt:tokenfile:<qrypt token path>
SuccessExitStatus=66

[Install]
WantedBy=multi-user.target
```
3. Copy the rngd service to systemd:
`sudo cp rngd.service /etc/systemd/system/rngd.service`
`sudo chmod 644 /etc/systemd/system/rngd.service`

1. Start the rngd service:
`sudo systemctl daemon-reload`
`sudo systemctl start rngd`

1. Verify the rngd service is running properly:
`sudo systemctl status rngd`

```
> Sep 20 19:17:46 rngd-demo systemd[1]: Started Hardware RNG Entropy Gatherer Daemon.
Sep 20 19:17:47 rngd-demo rngd[1148]: Disabling 0: Hardware RNG Device (hwrng)
Sep 20 19:17:47 rngd-demo rngd[1148]: Disabling 2: Intel RDRAND Instruction RNG (rdrand)
Sep 20 19:17:47 rngd-demo rngd[1148]: Disabling 7: PKCS11 Entropy generator (pkcs11)
Sep 20 19:17:47 rngd-demo rngd[1148]: Disabling 8: RTLSDR software defined radio generator (rtlsdr)
Sep 20 19:17:47 rngd-demo rngd[1148]: Enabling 9: Qrypt quantum entropy beacon (qrypt)
Sep 20 19:17:47 rngd-demo rngd[1148]: Initializing available sources
Sep 20 19:17:47 rngd-demo rngd[1148]: [qrypt ]: Initalizing qrypt beacon
Sep 20 19:17:49 rngd-demo rngd[1148]: [qrypt ]: Initialized
```

6. Enable the rngd service for it to start on system boot:
`sudo systemctl enable rngd`