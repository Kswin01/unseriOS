# unseriOS

This is a rather unserious operating system written for fun. Mostly going to be taking inspiration from
osdev wiki and seL4 (although I am ***definitely*** not going to implement a capability system).

Mostly just a learning exercise for me to revisit alot of kernel concepts that I have not had the
chance to touch since my undergraduate coursework.

# Building

A large issue seemingly came from bugs with an old QEMU version on my MAC. After spending
many days of pulling hair out, building QEMU from source solved the issue.

I am now using QEMU version: `QEMU emulator version 10.1.50 (v10.1.0-396-g190d5d7fd7)`.
As in the Makefile, this is an unsigned version of `qemu-system-aarch64`, meaning
that some features such as host acceleration are absent.

This was the configuration used when building QEMU: 
`../configure --target-list=aarch64-softmmu`.