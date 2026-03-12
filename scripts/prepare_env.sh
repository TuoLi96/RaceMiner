#!/bin/bash

sudo apt update
sudo apt install cmake build-essential flex bison libssl-dev libelf-dev bc -y
sudo apt install sqlite3 libsqlite3-dev zlib1g-dev nlohmann-json3-dev -y
sudo apt install clang libclang-dev lld llvm llvm-dev -y
sudo apt install python3-venv -y
sudo apt install bear -y
sudo apt install python3-sphinx -y
sudo apt install libspdlog-dev -y

mkdir $RACEMINER_CONFIG_ROOT
cp $RACEMINER_ROOT/scripts/compile.config $RACEMINER_CONFIG_ROOT
