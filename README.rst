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

======  ===========
Flag    Description
``-l``  Print program listing to standard error. Includes label map, address to machine-code list, and re-assembled instructions
``-s``  Turn on strict parsing to force correct syntax. For example, without ``-s`` the instruction ``LDR R0 R1 R2`` would be accepted, but with ``-s`` the assembler would require ``LDR R0, [R1, R2]`` (however, the assembler is always case-insensitive).
======  ===========


Notes
==========
- To load or store the ALU flags with the ``MOV`` instruction, you can reference ``Flags`` explicitly as an operand. For example, use ``MOV R0, Flags`` to load ``Flags`` into ``R0`` and use ``MOV Flags, R0`` to store ``R0`` into the ``Flags``. 
