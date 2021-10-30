// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

SAVING GERBERS / ORDERING NEW BOARDS
====================================

    I used Sprint Layout 6.0 (Windows only) to layout the printed circuit board for this
    project. It has a nice UI, similar to ExpressPCB's, but has the option of saving
    gerbers/drill files, so that I can use any PCB printing company in the world for more
    cost effective board printing.

    This software is free to download and use to load and create PCB's, but saving is
    disabled unless you purchase the software (~$50 USD).  I think $50 is very reasonable,
    considering how much other PCB software costs. And $50 is way cheaper than the free
    ExpressPCB software ends up being when you pay $380 to have the boards printed,
    instead of more like $100 to have them printed elsewhere.

    SAVING OUT GERBERS/EXCELLON DRILL
    ---------------------------------
    Gerber files are used by the board fabrication company to print the boards.
    For this project, there's 7 files in total:

        * 2 Gerber files for each electrical level of the board (top and bottom).
          This defines where copper will remain to define the electrical traces,
          and are referred to as C1 and C2 for top and bottom copper layers.

        * 2 Gerber files defining the solder mask for top and bottom.
          The solder mask defines where varnish should be coated on the board, 
          to cover everything /except/ the solder joints.

        * 2 Gerber files for the silk screen pass, which defines where paint
          will be deposited to draw the text and diagrams on the board's front and back.

        * Finally, there's one 'excellon drill' file used to define where all
          the holes should be drilled, and what drill bit size is for each hole.

    Steps in Sprint 6.0 to save out these files:

        * Load the layout file:
          File -> Open -> 1a2-pic-IRF-rotary-interlink-REV-F1.lay6

        * Export the Gerbers:
          File -> Export -> Gerber Export

          1) Turn on all layers that are not grayed out.
          2) Be sure "solder mask offset" for "Pads" is 5.0 mils

          Change all the input fields marked with "***":

            Layer  (X) C1 Copper Top                                              Options
                   ***                                                              ( ) Mirror
                   (X) C2 Copper Bottom                                             ( ) Punch drill holes
                   ***
                   (X) S1 Silkscreen Top                                          Offset of solder mask
                   ***                                                              (X) Pads:      5.0 mil   <--- *** Change this! (Default is 10)
                   (X) S2 Silkscreen Bottom                                                        ***      
                   ***                                                              (X) SMD-Pads   3.9 mil

                    :                                                               (X) Others     3.9 mil
                     
                   (X) C1 - Solder mask
                   ***
                   (X) C2 - Solder mask
                   ***
                    :

                   (X) O - Outline
                   ***
                   (X) Include frame (board size)      <--- *** Don't miss this one - It keeps turning off by default!
                   ***

	    > Be sure to set checkbox: "Include frame (board size)". This always seems to clear itself
	    > each time you save, so be sure to set it each time!

            > 5 mils seems a better solder mask offset than 10. Prevents nearby traces from becoming
            > soldered to pads, and keeps some mask between all Amphenol connector pads.

	    FOR SURFACE MOUNT RESISTORS: You can leave the "smdmask" turned OFF, as that's only
	    for generating the metal mask used for solder paste. The few surface mount
	    components on these boards can be hand soldered easily.

        * File -> Export -> Excellon
                Make sure "Inches" selected

    The resulting files are combined into a single .zip file that I can
    then shop around to the various PCB print houses. 
    
    I use a Makefile to automatically generate the zip, but it can be done
    by hand too using tools like Winzip, pkzip, or the zip(1) program that
    comes with linux.

    I found PCBway works well for printing the boards, and they have instructions
    specifically for Sprint 6.0:
    https://www.pcbway.com/blog/help_center/How_to_generate_Gerber_from_Sprint_Layout_6_0.html

    [OPTIONAL] SAVING OUT JPG IMAGES
    --------------------------------
      I like saving out .jpg images of the PCB board so I can review them on the web from
      coffee shops and such to look for errors, and without needing the Sprint software
      to view it. To do this:

        I. 1a2-REV-XX-all.jpg
        ---------------------

        1) At bottom left, ensure all are visible: "C1", "S1", and "C2". "S2" and "O" should be OFF.
        2) At bottom left, ensure "C1" is selected. This ensures top copper draws over bottom copper.
        3) File -> Export -> JPG
        4) Resolution: 300 dpi (3rd slider position from left)
        5) Save as "1a2-REV-XX-all"

        II. 1a2-REV-XX-top-bot.jpg
        --------------------------
        Assumes above was just done, and settings still set as above:

        1) At bottom left, disable "S1" (so only C1,C2 are enabled for visible, and C1 still "active")
        2) File -> Export -> JPG
        3) Click OK for Resolution
        4) Save as "1a2-REV-XX-top-bot"

        III. 1a2-REV-XX-photo-top.jpg
        -----------------------------
        Assumes above was just done, and settings still set as above:

        1) Click "Photoview" and turn on "Top side C1/S1", "With silkscreen", Board: Green, Soldermask: Gold
        2) File -> Export -> JPG
        3) Click OK for Resolution
        4) Save as "1a2-REV-XX-photo-top"

        IV. 1a2-REV-XX-photo-bot.jpg
        -----------------------------
        Assumes above was just done, and settings still set as above:

        1) Turn on "Bottom side C2/S2"
        2) File -> Export -> JPG
        3) Click OK for Resolution
        4) Save as "1a2-REV-XX-photo-bot"

        V. 1a2-REV-XX-bom.txt
        ---------------------
        1) Options -> Component Panel
        2) For "Show:", enable Number, ID, Value, Package, Comment
        3) Click "Export" button
        4) In dialog:

            Exported Data:          Separator:             Text for Layer side
           -->  (ON)  Number            (off) Comma              Top: Top
           -->  (ON)  ID                (off) Semicolon       Bottom: Bottom
           -->  (ON)  Value         --> (ON)  Tab
                (off) Layer
                (off) Position      X/Y - Position:     Decimals: 2
                (off) Rotation          Unit: Mil         (OFF) Suppress trailing zeros
           -->  (ON)  Package
           -->  (ON)  Comment       Rotation:
                                        (off) Rotation with "R" prefix

                                    Filter
                                        (ON) SMD-Components                            (ON) Top
                                    --> (ON) Throughhole-Components                    (ON) Bottom
                                        (off) Only components with Pick+Place data

           When set correctly, under "Preview:" it should show at least 100 or more parts.. scroll down.

        5) Hit "Export.."
        6) Save as "1a2-REV-XX-bom", type "Textfiles (*.txt)"
        7) Close..

        VI. SEND CHANGES TO WEBSERVER
        -----------------------------
        Go into 'website' directory and run 'make send' as erco on harris.
        This should build the zip, bom -> html, and send that and jpg's to server.
        Adjust the index.html if needed.


PRINTING BOARDS AT PCB HOUSES
-----------------------------
Notes about board:
                           Inches
                           --------------
               Dimensions: 4.92" x 13.05"
                   Layers: 2
          Board Thickness: 0.062"
                Min Trace: 0.010"
           Surface Finish: HASL    <-- CHECK
           Smallest Holes: 0.012"
                  # Holes: 1001
          ITAR Controlled? No
                     Silk: White
         Solder Mask Type: LPI-Semi Gloss
        Solder Mask Color: Black

Typically you upload a .zip file of the Gerbers + "Excellon Drill" file
to have the boards printed.

Companies like PCBWay, WellPCB, Seeed Studios, Avanti Circuits, etc.

I used PCBWay for several board runs to do my prototypes and final boards,
and the general instructions for using them:

   o Go to the PCBWay.com website

   o Create an account first, if you don't already have one

   o Use their PCB instant quote

   o Fill out their inital form; I used their defaults.
     They have a default minimum hole size of .3mm (.012 inches), so I made
     sure the board's smallest vias were that size and no smaller.
     Pick any colors you want; default of green board and white silk is fine.
     I usually chose a quantity of 5 for prototypes, or 10 when I was pretty sure
     it was a good board.

   o Choose 'calculate' and it calculates your cost approximately.

   o Choose "Add To Cart"

   o Now you can upload the .zip file of gerbers

   o Then you wait -- they take maybe 10 minutes or more to load your zip file and check
     for errors, and then adjust your quote estimate if need be. Just reload the page
     until the person responds who checks your board for errors.

   o Once you get the all clear, you can then order the boards.
     Paypal works fine, that way they never see your credit card info.  I chose express
     DHL for delivery, and I typically used their 24 hour express service so that I can
     keep my intertia on the project.

   o While waiting for the boards to be printed, I'd spend those days reviewing
     my parts inventory, ordering any parts from digikey I might need before the boards
     come. I'd also check the board's schematics for errors, as each time I did a board run
     I was typically making changes, and the way I work, I typically start with protoboarded
     circuits, draw those into the board layout program, make some rough schematics from
     that, and re-proto the board based on the PCB layout I end up with, to be sure what I
     drew really will work. And then have the boards printed, and refine the schematics.
     Reviewing the schematics while the boards are bring printed also makes sure I have
     the design in my head so that when the boards come back, I'm primed to solder the
     boards up, and know what to check for during testing.

MANAGING SCHEMATICS
===================

    I used ExpressSCH 7.0.2, Express PCB's schematic software (Windows only),
    to generate the schematics.
    
    The software is FREE from ExpressPCB's website.

    To generate images, I load the .sch file, and view each sheet, and choose File -> Export
    to save each image:

       * Load the .sch file

       * Choose each page in the schematic, then choose File -> Export schematic image,
	 saving each as 0001, 0002, etc.
	 
       * After that, I run a Makefile that converts the large .BMP images into smaller
         8 color PNGs so I can make them available on the website.

SOLDERING BOARDS
----------------

    There's two ways I solder up the boards, depending on what I'm doing.

	1) The typical case is creating a production board. The board is known to be a
	   working design, so it makes sense to just solder all the components at once.

	2) The odd case is I'm testing a new prototype board, which might be a dud design.
	   So I solder in JUST the components of a small section I want to test first,
	   so I can easily test it without committing to soldering all components. This way
	   if it's a dud board, I don't have to waste time and component inventory on a board
	   that will never be used.

    So here I'll cover just soldering a production board, as that's the typical case.

    PRODUCTION BOARD SOLDERING
    --------------------------

    I like to start with the smallest profile components first, so that when you flip
    the board over to solder the leads, the component can be evenly pressed from the
    other side using a soft cloth or folded papertowels. You want the components 
    pressed flat when you solder them in, or they won't lay evenly, and in the case
    of sockets, will stress the vias when inserting chips.

    So I tend to do them in this order:

    	> Surface mount resistors

	  I touch a small bead of solder on each pad, then position the resistor over
	  the two pads, and touch the soldering iron to the edge of the pad to 'suck'
	  the component onto the pad. This will be at an angle because the other pad
	  is still hard. Then heat the other pad to liquify the solder, then quickly
	  go back to the first pad (while the second is still liquid) to suck the 
	  component down flat.

	> 6 pin chip sockets

	  These are small and fall out easily, so I do them first.

	  I put at least two in the board, and hold them as I flip the board over onto a
	  soft cloth or folded papertowel that is just under those components so that it
	  presses them against the board. I then solder the leads on the solder side.

	  If in doubt about the component laying flat, I touch solder on a corner pin first,
	  let it harden to hold the socket in place, then flip the board over to view the
	  board sideways to see if the socket is resting flat. If it's not, I can touch
	  the iron to that one pin to loosen it, while pressing on the socket to flatten
	  it, making sure not to put a finger on the pin (it'll be hot!). The reason for
	  doing one pin is it's really hard to reseat a crooked component if all the pins
	  have already been soldered; it's hard to heat ALL the pins at once so you can
	  reseat before the solder hardens.. trust me.

	> Do all the larger chip sockets similarly.

	  Some sockets are nice in that they have pins you can bend to hold the sockets
	  flat and in place when you flip the board for soldering. If that's not an option,
	  solder a few at a time, using a cloth or folded napkin underneath to keep pressure
	  on the sockets to press them flat against the board while you solder the pins.

	> Do all the small through hole discrete components (resisitors, capacitors..).

          I like to solder all the resistors with the color bands facing the same way,
          the first band digit facing up or facing left. I try to be consistent, as it
          simply looks nicer that way.

	  For discretes I try to do a bunch at a time, having them all pre-inserted into
	  the holes, pre-bent into a "U" shape with needle nose pliers, and hold them in
	  place with my finger tips while I flip the board, and bend all the leads at 45
	  degree angles to hold them in place:


                                       \                  / 
                                        \                /  <-- bend leads 45 degrees
                    Solder Side          \              / 
	            ------------------------------------------------------------  Board
	            ------------------------------------------------------------
		    Component Side        |__::::::::__|
				             ::::::::

				             Resistor


          Then once a bunch of them are inserted this way, I solder them all in one pass,
          then trim off the leads with small wire cutters.

          NOTES:

              1) Keep a small cup nearby to put all the cutaways into, so they
                 don't end up flying everywhere, or getting under and between chip sockets
                 to cause shorts later.

              2) Keep the soldering area clean!! Don't let cut off leads or solder blobs
                 lie around, or they'll find their way into the board causing shorts later.

        > Solder the TO-220 transistors. 
        
          NOTES: 
              1) TO-220's are best laid flat and screwed down, to prevent them from flapping
                 around, but more importantly, to keep them from sliding sideways into
                 each other, shorting them out, as the heat sink tabs are the output of
                 the transistor.

              2) When making the 90 degree angle bend for the pins, don't bend the pins where
                 they meet the case. Bend them out at the point where the leads flare down
                 from fat to thin. e.g.

                                     ______________
                                    |       _      |
                                    |      (_)     |
                                    |______________|
                                    ||             |
                                    ||             |
                                    ||             |
                                    ||_____________|
                        wide leads --> | | | | | |\___ DON'T bend here
                                       |_| |_| |_| 
                                        |   |   |\___ BEND HERE
                         thin leads --> |   |   |
                                        |   |   |

          I pre-bend the leads with needle nose pliers before inserting them into the
          board. Once inserted, I screw the case down with nuts so they stay in position
          when I flip the board to solder them.

          Some notes on pre-bending the pins: When bending the leads, just bend along the
          thin part of the leads, not at the case:

                          ________                               ________
                 ________|        |                     ________|        |___
                 |_______|________|\   BAD !            |_______|________|   \   GOOD
                                    \                                        |
                                    |                                        |
                                    |                                        |

         To do this, I grip small needle nose across the fat pins:


                            plier's grip                     
                            on fat pins                                            ___________________
                            |                                                     /   ________________\
                           \|/                                                   /   /
                            _                                    _______________/   /
                  ________ / \                                  /________          /
         ________|        |===-------      <--- pins --->       _O_O_O___/         \
         |_______|________|\_/                                  \________________   \
                                                                                 \   \________________
                                                                                  \___________________/
                                                                                                   
                - SIDE VIEW -                                                  - BOTTOM VIEW -
                                                       

         To prevent metal-fatiguing the pins with a full 90 degree turn all at one point
         on the leads, I bend the pins down only 45 degrees with my finger, the needle nose
         prevents bending the wide part of the pins:


                              ::: <-- plier teeth
                     ________ :::
            ________|        |===
            |_______|________|::: \ <-- bend leads 45 degrees downward at pliers
                              :::  \
                                    \
                                     \


        ..then re-grip the pliers around the 45 degree bend, and bend again another 45 degrees:

                     ________
            ________|        |===  /\ <-- plier teeth
            |_______|________|    \\/
                                 /\\
                                 \/ |  <-- bend again at pliers 45 degrees downward
                                    |


        The result ends up being more of a curve than two flat 45 degree bends, giving a
        nice bend radius that can't really be render in ascii art:

                     ________
            ________|        |===
            |_______|________|    \  <-- a nicer looking curve
                                   \     than this ascii art can show
                                    |
                                    |

     > Solder the larger profile components (relays, large caps, connectors, SIP + DIP headers)

     > Insert all the chips into the sockets

     > Program the PIC chips, and insert them into their sockets

     > I do the press-fit 50 pin amphenol connectors last.

       In the beginning of the project, I used a large clamp and a simple wood jig with a slot
       cut in it to do the compression. Later I procured a drill press to make the process easier.



       CLAMP TECHNIQUE FOR PRESS-FITTING AMPHENOLS
       ============================================
       For this technique, the jig goes under the board, the slot aligning with the
       connector's holes for the pins to poke through once the connector presses fully into place:

                                      __________________
                                     /   ____________   \
                                    /____\           \   \
                                                     |    |  <-- clamp
           Press Fit Amphenol         __             |    |
           Connector (side view) -> _|__|_           |    |
                                    |____|           |    |
    PCB BOARD                         ||             |    |
    ===============================================  |    |
                            ________      ________   |    |
               wood --->   |        |____| slot   |  |    |
               jig         |______________________|  |    |
                                    _____            |    |
                                    \___/            |    |
                                    _///_            |    |
                                   |     |__________/     |
                                    \____________________/
                                     ///
                                     ///
                                     ///
                                    /   \===========()
                                    \___/

    So in three compressions, the connector can be pressed firmly into place.
    You have to make sure the clamp is CENTERED sideways on the connector, both above 
    and beneath, or the connector will tip sideways, which must be avoided.
    Stop and reposition as soon as you see that happening, or it'll bend the pins.

    When pressed in properly, the result looks like this:
    
                                      __________________
                       connector     /   ____________   \
                       pressed in   /____\           \   \
                                    _|__|_           |    |
    PCB BOARD                       |____|           |    |
    ==================================||============ |    |
               wood --->   |        |____| slot   |  |    |
               jig         |______________________|  |    |
                                    \___/            |    |
                                     ///             |    |
                                     ///             |    |
                                     ///             |    |
                                    _///_            |    |
                                   |     |__________/     |
                                    \____________________/
                                     ///
                                    /   \===========()
                                    \___/

     Note how the jig's slot is the key to doing this right, to leave clearance
     for the pins when they poke through the board, while still keeping uniform
     pressure on the lower part of the board.

     I found pressing first at the edges, then the center last works best, e.g.


                             Top View of Amphenol
                                   Connector

                                       __
                                      /  |
                                     /   |
                                    |  X |  <-- press here first
                                    |    |
                                    |    |
                                    |    |
                                    |    |
                                    |  X |  <-- press here last
                                    |    |
                                    |    |
                                    |    |
                                    |    |
                                    |  X |  <-- press here second
                                     \   |
                                      \__|


     Once pressed into place, you DO NOT solder the pins; that's the whole point
     of pressfit connectors, not having to make 50 solders for each connector.



     DRILL PRESS TECHNIQUE FOR PRESSING AMPHENOLS
     ============================================
     After a few months of doing boards with the clamp technique, I procured a 
     small drill press (a WEN model 4208, 8" 5-speed), and use that now instead
     of the clamp; just quicker, very repeatable, less clumsy than the clamp.

     I used the same wood jig above to place on the drill press table, and created
     a second jig that fits into the drill chuck which presses down on the connector:


                                          _
                                         | |  <-- 1/4" carriage bolt
                                         | |
                                         | |
                                         | |
                                         | |     nut
                              washer   __|_|__ / 
                                    \ |_|___|_|
              ________________________=========________________________
             |                           | |                           |
             |                      _____|_|_____                      |  <-- bar of hard wood 
             |_____________________|  \_______/  |_____________________|      (poplar I think)
                       
                                            \___ carriage bolt head
                                                 countersunk into wood jig



     The carriage bolt's threaded shank is inserted into the drill press chuck,
     so that when it presses down, the bottom surface of the bar of wood is used
     to press the top of the Amphenol connector evenly into the printed circuit board.

     The carriage bolt head is countersunk and the hole filled with bondo and sanded
     flat, so that the bottom surface is completely flat, to give an evenly flat surface
     for press fitting the connector.

     The result looking like this:


                        ____________________________________
                       /                                    \
                       |            DRILL PRESS             |
                       |___                                 |
                           |        ____________            |
                           |       /            |           |
                           |______/             |           |
                  chuck -->  |  |                \_________/
                           __|__|__                 |    |
                          |________|  <--(1)        |    |
                                                    |    |
                (2) ________#_#_#_#__               |    |
                           |______|   <--(3)        |    |
                       -----------------------      |    |
                      |_______________________|     |    |
                             table         |  |_____|____|_
                                           |______         |
                                                  |________|
                                                    |    |
                                                    |    |

                      (1) top wood jig (attached to chuck)
                      (2) printed circuit board ("#"=amphenols)
                      (3) bottom wood jig


    The top wood jig's bolt shank is inserted all the way into the chuck, and the
    chuck tightened down on the bolt shank, so that the bottom of the chuck is flush
    with the top of the wood jig. This way the hard downward pressure of the chuck
    presses against the wood, and not stripping the bolt shank's threads.

    I still have to press fit one connector at a time; as strong as the drill's
    compression is, it still takes a *lot* of pressure to do just one connector
    at a time.

    In fact it's enough pressure on the cantilevered table that it'd bend the table.
    I had to wedge a piece of wood under the table between it and the drill press base
    to give it more vertical stability.

    All of which is good; strong pressure on the connector means the connectors 
    /won't/ come out easily when users plug and unplug phones.


PROGRAMMING PIC CHIPS
=====================

The PIC chip vendor is a company called "Microchip" (microchip.com), and they also
sell a USB programmer which at the time of this writing (July 2019) is called the
"PIC KIT 4".

The programmer is about the size of a pack of cigarettes, and has a USB cable to
plug into a computer, and the other end can plug into a Zero Insertion Force (ZIF)
socket used to program the actual chips.

Microchip also supplies a free compiler/IDE environment for editing your programs,
and burning the compiled software onto the chip. At the time of this writing the tool
is called "MPLAB X".

So with the USB programmer plugged into the computer, and MPLAB X running, you can
write code, compile and burn it onto the chip. You can then take the programmed chip
and plug it into the 1A2 KSU board to use it.

During development I usually rotate between two chips; one to program, and one to
live in the board, and just rotate them each time I need to test changes to the firmware.

You can do all the development inside MPLAB X, though I later found a way to
use the command line to compile and program the chips, so that I don't have to
fumble around in their IDE to program the two PIC chips for my board, and just
use a DOS BATCH script that prompts for programming the chips easily.

A quick overview of using MPLAB X:

    (1) Create a new project (File -> New Project)

         Steps
          1. Categories: Microchip Embedded
          2. Projects: Standalone Project
          3. Select Device:
                Family: Baseline 8-bit MCUs (PIC 10/12/16)
                Device: PIC16F1709
          4. Select Header 
               Supported Debug Header: None    <-- skip this, we don't use it as a "debug header"
          5. Select Tool
               Under PICkit 4, there should be a serial number of your programmer, e.g.

                    Hardware Tools
                      |-- o Atmel-ICE
                      |-- oo ICD 3
                      |-- oo ICD 4
                      |-- PICkit 4
                      |     |__ SN:BUR1833339134  <-- Pick this
                      |
                      |-- oo PM3
                      :

          6. Set Compiler
               I used "XC8" (v2.10)

          7. Set Project Name and Folder

               This is the name that will appear in the list of projects.
               I suggest using a name without spaces (or use - or _ in place of spaces)
               so that the actual directory on disk doesn't have spaces in it (makes it
               easier to refer to in DOS BATCH scripts)

               In my case, I made separate projects for the CPU#1 and CPU#2 firmware.
               So I used:

                    1A2-REV-G-CPU1   -- for the CPU#1 firmware code
                    1A2-REV-G-CPU2   -- for the CPU#2 firmware code

               I left the other settings all at default values.

      (2) Now create a .c and .h file

          Right click on the project and choose New -> main.c
          Set the filename. In my case I used:

                    main-rev-g-cpu1.c  -- for CPU#1 firmware
                    main-rev-g-cpu2.c  -- for CPU#2 firmware

      (3) Open the new file in an editor, delete any default text the IDE put into
          the file, and replace it with the firmward code e.g. from github.

          Here's a "simple" test program to check the PIC16F1709; blinks and LED (RA2),
          and also reads an input from one pin (RA5) and writes the value to an
          output pin (RA0), and writes the INVERTED value to another pin (RA1):

__________________________________________________________________________ snip
/*
 * File:   main.c
 * Author: erco
 *
 * Created on Feb 28, 2019, 05:27 AM
 *
 * Program the inputs/outputs needed by the 1A2 board, and test them.
 *
 *                           _    _
 *                       V+ | |__| | GND
 *  TEST INPUT (IN) -- RA5  |      | RA0 -- (OUT) VALUE OF RA5 (DAT)
 *           X (IN) -- RA4  |      | RA1 -- (OUT) VALUE OF RA5 INVERTED (CLK)
 *    (MCLR) X (IN) -- RA3  |      | RA2 -- (OUT) LED OUTPUT
 *           X (IN) -- RC5  |      | RC0 -- (IN) X
 *           X (IN) -- RC4  |      | RC1 -- (IN) X
 *           X (IN) -- RC3  |      | RC2 -- (IN) X
 *           X (IN) -- RC6  |      | RB4 -- (IN) X
 *           X (IN) -- RC7  |      | RB5 -- (IN) X
 *           X (IN) -- RB7  |______| RB6 -- (IN) X
 *
 *                         PIC16F1709
 */

// This must be #defined before #includes
#define _XTAL_FREQ 4000000UL    // system oscillator speed in HZ (__delay_ms() needs this)

// --- The following section copy/pasted from MPLAB X menu: Production -> Set Configuration Bits -> Generate Source..
// CONFIG1
#pragma config FOSC     = INTOSC    // USE INTERNAL OSCILLATOR: Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE     = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE    = OFF       // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE    = ON        // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP       = OFF       // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN    = OFF       // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF       // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO     = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN    = OFF       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT     = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PPS1WAY = ON         // Peripheral Pin Select one-way control (The PPSLOCK bit cannot be cleared once it is set by software)
#pragma config ZCDDIS  = ON         // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR)
#pragma config PLLEN   = OFF        // Phase Lock Loop enable (4x PLL is enabled when software sets the SPLLEN bit)
#pragma config STVREN  = ON         // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV    = LO         // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR   = OFF        // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP     = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)
// --- end section

#include <xc.h>         // microchip's header file for our PIC chip
                        // (our PIC chip device# is passed to the compiler from the IDE's project settings)

// Initialize the PIC chip..
void Init() {
    OPTION_REGbits.nWPUEN = 0;   // Enable WPUEN (weak pullup enable) by clearing bit
    
    // Set PIC chip oscillator speed (we use 4MHz internal oscillator)
    OSCCONbits.IRCF   = 0b1101;  // 0000=31kHz LF, 0111=500kHz MF (default on reset), 1011=1MHz HF, 1101=4MHz, 1110=8MHz, 1111=16MHz HF
    OSCCONbits.SPLLEN = 0;       // disable 4xPLL (PLLEN in config words must be OFF)
    OSCCONbits.SCS    = 0b10;    // 10=int osc, 00=FOSC determines oscillator
    
    // NOTE: in the following TRISA/B/C data direction registers,
    //       '1' configures an input, '0' configures an output.
    //       'X' indicates a don't care/not implemented on this chip hardware.
    //
    TRISA  = 0b11111000;  // data direction for port A (0=output, 1=input)
    WPUA   = 0b11111000;  // enable 'weak pullup resistors' for all inputs
    //         ||||||||_ RA0 (OUT) VALUE OUTPUT
    //         |||||||__ RA1 (OUT) INVERTED VALUE OUTPUT
    //         ||||||___ RA2 (OUT) LED OUTPUT
    //         |||||____ X
    //         ||||_____ RA4 (IN) unused
    //         |||______ RA5 (IN) VALUE INPUT
    //         ||_______ X
    //         |________ X
    
    TRISB  = 0b11111111;  // data direction for port B (0=output, 1=input)
    WPUB   = 0b11111111;  // enable 'weak pullup resistors' for all inputs
    //         ||||||||_ X
    //         |||||||__ X
    //         ||||||___ X
    //         |||||____ X
    //         ||||_____ RB4 (OUT) unused
    //         |||______ RB5 (IN)  unused
    //         ||_______ RB6 (OUT) unused
    //         |________ RB7 (IN)  unused
    
    TRISC  = 0b11111111;  // data direction for port C (0=output, 1=input)
    WPUC   = 0b11111111;  // enable 'weak pullup resistors' for all inputs
    //         ||||||||_ RC0 (OUT) unused
    //         |||||||__ RC1 (OUT) unused
    //         ||||||___ RC2 (OUT) unused
    //         |||||____ RC3 (IN)  unused
    //         ||||_____ RC4 (IN)  unused
    //         |||______ RC5 (IN)  unused
    //         ||_______ RC6 (IN)  unused
    //         |________ RC7 (OUT) unused

    // Disable analog stuff
    ANSELA = 0x0;
    ANSELB = 0x0;
    ANSELC = 0x0;
    ADCON0 = 0x0;   // disable ADC
    
    // Disable slew rate controls
    SLRCONA = 0x0;
    SLRCONB = 0x0;
    SLRCONC = 0x0;
}

void main(void) {
    unsigned char count = 0;

    // Initialize the chip speed and program inputs/outputs
    Init();

    // main loop (500ms): flash LED, copy input to outputs 
    while (1) {
        LATAbits.LATA0 = PORTAbits.RA5;     // Read RA5, write its value to RA0
        LATAbits.LATA1 = PORTAbits.RA5 ^ 1; // Read RA5, write its inverted value to RA1
        LATAbits.LATA2 = count & 1;         // Turn RA2 on and off every 500ms (LED blink)
        __delay_ms(500);
        count++;
    }
}
__________________________________________________________________________ snip

      To configure the ZIF socket so that the chip can be programmed with the power from
      the PICkit 4:

        1) In the "Projects" pane, right-click on the project and choose PRoperties

        2) In the "Project Properties" dialog, under the "Categories" pane:

                > Click on "PICkit4"
                > Under "Options for PICkit4", set the "Option Categories" to "Power"
                > For "Power target circuit from PICkit 4", check the box
                > Leave the "Voltage Level" at "5.0"
                > Hit "Apply", and hit OK out of the dialog

      Now you should be able to click the "Build and Program" icon, or right click
      on the project and choose "Make and Program Device".

      If it succeeds, the chip is programmed, and can be moved to the board for testing.


MANAGING FIRMWARE CODE IN GIT
=============================

TODO:
    Programming the PIC chips
    Managing source code, git, Makefile
