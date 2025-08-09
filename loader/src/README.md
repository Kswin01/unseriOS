# Loader

TODO - come back and finish off making a more principled loader so that we don't
embed the initial task in the kernel image, and instead have a seperate loader
that copies out the kernel and initial tasks, sets up the MMU and then jumps
to the kernel.