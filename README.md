# Testing sequential/non-sequential processing of handlers via io_context and strand

The [boost libraries](https://boost.org) are used to test sequential and non-sequential
execution of code via io_context and io_context::strand.

If boost libraries are not installed system wide then the linked library should be
installed in the lib/ directory where the stest executable will search after checking
the system library directory.

After compiling run bin/stest without arguments to run test with sequential execution
via a strand.

For non-sequential execution call the executable with nonsequential as an argument, i.e.
./bin/stest nonsequential
