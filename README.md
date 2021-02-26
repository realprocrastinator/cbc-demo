# Simple CBC(Cipher Bock Chaining) Demostration Program

## Introduction
The current version only supports encryption and decryption which uses default configuration.

## How to use?
To run the demo, you can run script `run_demo` or use `Makefile` to compile from source then run 
`./demo E|D -P <plaintext file> -K <key file> -L <key len> -B <block size>`. for encryption or 
decryption depedning on the flag `E` or `D`. If not flag provided this program will simply just run 
`./demo` to demo the encryption first then demo the decryption.

## Future TODO work
Make the encryption and decryption configurable by user.
Seperate the lib function and the driver code apart.
Make default settings configurable.
Use logging instead of printing bunch of msgs.
