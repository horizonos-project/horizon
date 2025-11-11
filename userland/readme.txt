Hello developers and explorers of Horizon,

These are assembly files that you can build with the Horizon/i686/i386
assemblers and linkers to produce a flat freestanding binary.

These are test files for the userland, these will probably be deleted
once the userland can be consistently accessed.

To automate building, 'cd' into this directory and run 'make all'
and every file will be assembled into its own binary inside the dir:

    * userland/bin/[name].bin