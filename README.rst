alARM-Assembler
###########
An assembler for the alARM instruction set (for UC Davis ECS 154A, Fall '22)

alARM is a 16-bit ARM-like instruction set with the goal of building a working CPU in the digital logic simulation Logisim-Evolution. Because the instruction memory ROM can only take in hexadecimal machine code, this assembler was created to expedite programming for and testing of the alARM CPU.

By default, allows for a relaxed syntax that does not require any special delimiters (commas, brackets, etc.) and will not throw errors if the formally correct syntax is attempted but incomplete in some way. You can set "strict parsing" which requires the formally correct syntax by using the ``-s`` flag.

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

Feature Additions
==========
- 11/30/22 5:00pm - added ``CLC`` psuedo-instruction for clearing the carry flag (gets replaced with ``AND R0, R0, R0``).

Bug Fixes
==========
- 11/17/22 9:30pm - fixed a bug that was causing every operand token to display as mnemonic instead in encoder errors.
- 11/19/22 8:30pm - fixed ``CMP`` encoding to use Rn and Rm spaces instead of Rd and Rn.
- 11/20/22 6:45pm - fixed ``MOV Flags, Rn`` encoding to use Rn instead of Rd.
- 11/29/22 4:00pm - fixed negative immediate parsing for in strict mode.

ISA
==========

Registers and Memory
----------

.. list-table::
   :widths: 25 50
   
   * - **Data Registers**
     - 7 data registers ``R0-R6`` and program counter ``R7`` (read as ``PC+1``)
   * - **Instruction Memory**
     - 64K addresses for 16-bit instructions
   * - **Data Memory**
     - 64K of word-addressable data memory for 16-bit data words (top 2K words are reserved for IO)
   * - **Display IO**
     - Data address 0xFFFF reserved for printing a value on the front panel display

Status Register
----------
This 4-bit register stores the flags from the previous ALU operation. Their abbreviations, bit positions (from ``[3:0]``) and their meaning are as follows: 

.. list-table::
   :widths: 20 20 20 20 20
   :header-rows: 1
   
   * - BIT
     - 3
     - 2
     - 1
     - 0
   * - NAME
     - **N** egative
     - **Z** ero
     - **C** arry
     - o **V** erflow
     
Non-ALU Operations
----------

.. list-table::
   :widths: 25 25 50
   :header-rows: 1

   * - Mnemonic
     - Operands
     - Description
   * - ``NOP``
     -
     - No operation
   * - ``HALT``
     -
     - Halts program counter, terminating program
   * - ``MOV``
     - ``Rd, Rn``
     - Move data from ``Rn`` into ``Rd``
   * -
     - ``Rd, Imm``
     - Move 12-bit immediate value (signed decimal, hex or binary) into ``Rd``
   * -
     - ``Rd, Flags``
     - Move status flags, zero-extended to 16 bits, into Rd
   * -
     - ``Flags, Rn``
     - Move lowest 4 bits of Rn into status flags
   * - ``LDR``
     - ``Rd, [Rn, Rm]``
     - Load value from memory address ``Rn+Rm`` into ``Rd``
   * -
     - ``Rd, [Rn]``
     - ... ``Rm=0``
   * - ``STR``
     - ``Rd, [Rn, Rm]``
     - Store value from ``Rd`` into data memory at address ``Rn+Rm``
   * -
     - ``Rd, [Rn]``
     - ... ``Rm=0``
   * - ``B``
     - ``Imm``
     - Unconditional relative branch, set program counter to ``PC+1+Imm``
   * - 
     - ``Label``
     - Unconditional branch to program label
   * - ``BEQ``
     - ``Imm``
     - Relative branch when the Z flag of status register is **set** (``CMP R0, R1`` when ``R0==R1``)
   * - 
     - ``Label``
     - Branch to program label when the Z flag of status register is set
   * - ``BNE``
     - ``Imm``
     - Relative branch when the Z flag of status register is **cleared** (``CMP R0, R1`` when ``R0!=R1``)
   * - 
     - ``Label``
     - Branch to program label when the Z flag of status register is **cleared**
     
ALU Operations
----------

.. list-table::
   :widths: 25 25 50
   :header-rows: 1

   * - Mnemonic
     - Operands
     - Description
   * - ``ADD``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn + Rm``
   * - ``SUB``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn - Rm``
   * - ``MUL``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn * Rm`` (lower 16 bits of result)
   * - ``MULU``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn * Rm`` (upper 16 bits of result)
   * - ``DIV``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn / Rm``
   * - ``MOD``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn % Rm``
   * - ``AND``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn & Rm`` (bitwise and)
   * - ``OR``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn | Rm`` (bitwise or)
   * - ``EOR``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn ^ Rm`` (bitwise exclusive or)
   * - ``NOT``
     - ``Rd, Rn``
     - ``Rd <- ~Rn`` (bitwise not)
   * - ``LSL``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn << Rm`` (logical left shift by lowest 4 bits of Rm)
   * - ``LSR``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn >> Rm`` (logical right shift by lowest 4 bits of Rm)
   * - ``ASR``
     - ``Rd, Rn, Rm``
     - ``Rd <- Rn >> Rm`` (arithmetic right shift by lowest 4 bits of Rm)
   * - ``ROL``
     - ``Rd, Rn, Rm``
     - Rotate ``Rn`` to the left by lowest 4 bits of ``Rm`` and place into ``Rd``
   * - ``ROR``
     - ``Rd, Rn, Rm``
     - Rotate ``Rn`` to the right by lowest 4 bits of ``Rm`` and place into ``Rd``
   * - ``CMP``
     - ``Rn, Rm``
     - ``Rn - Rm`` (only sets the flag, doens't write the result)
     
Psuedo-Instructions and Aliases
----------

.. list-table::
   :widths: 25 25 50
   :header-rows: 1

   * - Mnemonic
     - Replacement
     - Description
   * - ``CLC``
     - ``AND R0, R0, R0``
     - "Clear Carry", used to avoid the implicit carry-in to the ALU for ADD and SUB operations

Notes
---------
- All operations are signed operations, unless otherwise specified.
- To load or store the ALU flags with the ``MOV`` instruction, you can reference ``Flags`` explicitly as an operand. For example, use ``MOV R0, Flags`` to load ``Flags`` into ``R0`` and use ``MOV Flags, R0`` to store ``R0`` into the ``Flags``. 

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
  
  > CLC
  0x00E: 0x2C00 | AND  R0, R0, R0
  
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
  
