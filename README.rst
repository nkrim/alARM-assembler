alARM-Assembler
###########
An assembler for the alARM instruction set (for UC Davis ECS 154A, Fall '22)

Installation
============
.. code-block:: console

  $ make
  
Usage
=====
.. code-block:: console

  $ ./alarmas src_file out_file [-l] [-s]

Options
=======
-l  Print listing to standard error. Includes label map, address to machine-code list, and re-assembled instructions
-s  Turn on strict parsing to force correct syntax. For example, without `-s` the instruction `ldr r0 r1 r2` would be accepted, but with `-s` it would require `ldr r0, [r1, r2]`.

Notes
==========
