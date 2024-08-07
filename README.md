# PULP runtime

## About

This module is a simple runtime for the Pulp architecture.

### 設定

You need to first install the Linux dependencies (see [below](#dependencies)).

PremoC1用設定

```sh
source configs/premoc1.sh
```

Then you can get one of the pulp example, compile and run it.

設定スクリプトに以下を追加することで、stdoutをUARTに出力できる

```sh
export io=uart
```

### 実行

プログラムを実行する前に以下のスクリプトが必要
```sh
sudo openocd -f openocd-ftdi2232.cfg
```

プログラムはgdbで送る

### システムクロック

現状クロック 25MHz前提でソースコードを書いている

既知のクロックに依存するソースコード

`drivers/uart.c uart_setup()`

## Linux dependencies

Here are the required system dependencies for building the runtime and its dependencies.

### Ubuntu 16.04

Starting from a fresh Ubuntu 16.04 distribution, here are the commands to be executed to get all required dependencies:

    $ sudo apt install git python3-pip gawk texinfo libgmp-dev libmpfr-dev libmpc-dev
    $ sudo pip3 install pyelftools

## Dependencies

### Build

Have a look at the dependencies documentation to see how to build them.

You can have a look [here](https://github.com/pulp-platform/pulp-riscv-gnu-toolchain.git) for the toolchain.

### Setup

The toolchain must be built separately and the following environment variable should set:

    $ export PATH=<path to the folder containing the bin folder of the toolchain>/bin:$PATH

RTL platforms should also be built separately (see the platform documentation for that) and the following
environment variable must point to the folder where the platform was installed (this example is for pulpissimo):

    $ export VSIM_PATH=<pulpissimo root folder>/sim

### Examples

Some examples can be found here: git@github.com:pulp-platform/pulp-runtime-examples.git

### Useful options

The vsim gui can be opened with this option:

    $ make run gui=1

The uart can be selected for the printf with this option:

    $ make all run io=uart

The baudrate can also be specified with:

    $ make all run io=uart CONFIG_IO_UART_BAUDRATE=9600
