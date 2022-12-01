alARM-Assembler
###########
An assembler for the alARM instruction set (for UC Davis ECS 154A, Fall '22)

Installation
============
.. code-block:: console

  $ git clone https://github.com/nkrim/alARM-assembler.git
  $ cd alARM-assembler
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

Feature Additions
==========
- 11/30/22 5:00pm - added ``CLC`` psuedo-instruction for clearing the carry flag (gets replaced with ``AND R0, R0, R0``).

Bug Fixes
==========
- 11/17/22 9:30pm - fixed a bug that was causing every operand token to display as mnemonic instead in encoder errors.
- 11/19/22 8:30pm - fixed ``CMP`` encoding to use Rn and Rm spaces instead of Rd and Rn.
- 11/20/22 6:45pm - fixed ``MOV Flags, Rn`` encoding to use Rn instead of Rd.
- 11/29/22 4:00pm - fixed negative immediate parsing for in strict mode.

Tests
==========
Includes five test files: 

- ``testinsts.s`` which includes every instruction in every format in order to ensure proper encoding.
- ``testerrors.s`` which should initiate an error on every line of the program, so it starts entirely commented in order to test for specific errors.
- ``teststrict.s`` which includes strictly formatted instructions and should be tested with the ``-s`` flag set.
- ``teststricterrors.s`` which should intiate an error on every line only when the ``-s`` flag is set.
- ``testhandencoded.s`` which has some instructions paired up with their hand-encoded hex in the comments, written by Dominic Quintero.
- ``teststress.s`` which has 65536 instructions, enough to fill alARM instruction memory, so it is good for timing performance.

Examples
==========

*The instructions shown below are assembled from larger files, though they are presented here alone with their listing/error output merely for examples. However, interactive assembling in the terminal is a planned feature.*

.. code-block:: console

  > ldr r1 r5 r6
  0x00E: 0x114E | LDR  r1, [r5, r6]
  
  > ldr r1[r5,r6]
  0x00E: 0x114E | LDR  r1, [r5, r6]
  
  > B   0b110
  0x05D: 0x4006 | B    0x006 ; (6)
  
  > MOV R0, 0x828
  0x004: 0x8828 | MOV  R0, 0x828 ; (-2008)
  
  > MOV r1, -34
  0x005: 0x9FDE | MOV  R1, 0xFDE ; (-34)
  
  > end:BNE eNd
  0x063: 0x7FFF | BNE  0xFFF ; (-1 -> END)
  
  > mov r0 r1 r2
  Error: line[12]: could not match operand format for mnemonic 'mov':
  -->  mov r0 r1 r2
           ^~~~~~~~
  --- Expected one of the following formats:
  -----> mov Rd, Rn
  -----> mov Rd, Flags
  -----> mov Flags, Rd
  -----> mov Rd, Imm
  
  > r0: mov r0 r1
  Error: line[3]: illegal label name 'r0', reserved by ISA:
  -->  r0: mov r0 r1 
       ^~~
  
  > MOV R3 0x1000
  Error: line[27]: could not encode 2nd operand '0x1000', hex value has too many nibbles (max = 3):
  --> MOV R3 0x1000
             ^~~~~~
             
  > ldr r1 r5 r6 ; with -s flag on
  Error: line[8]: could not match operand format for mnemonic 'ldr':
  --> ldr r1 r5 r6     
          ^~~~~~~~
  --- Expected one of the following formats:
  -----> ldr Rd, [Rn]
  -----> ldr Rd, [Rn, Rm]
  
