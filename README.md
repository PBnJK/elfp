# elfp -- ELF file information reader

Small `readelf` clone written as a means of about the ELF file format. It's very
much incomplete, but is already able to decode all 3 headers!

## Usage

Run `elfp -h` for usage info

## Building

The project uses CMake, so it's pretty straightforward:

```sh
> mkdir build           # make a new "build" directory
> cd build              # enter the "build" directory
> cmake ..              # run cmake
> make                  # compile the code
> ./elfp                # run the tool!
```
But I suppose if you're the sort of person that's interested in a tool for parsing
ELF files, then you'd already know how to do this, hmm?
