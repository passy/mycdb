=====
MyCDB
=====

This is a learning project trying to re-implement a reader for the Constant Data
Base format by Daniel J. Berstein. If you're looking for a working
implementation, use the `official one <http://cr.yp.to/cdb.html>`_.

I built this to learn about low-level file, socket APIs and binary file formats.

This project consists of two parts:

    * mycdbget, which resembles cdbget, but doesn't verify the format as
      carefully as the original.
    * mycdbserver, a barely working TCP server that allows reading cdb values
      over network.
