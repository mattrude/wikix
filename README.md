**Wikix** is a 'C' based program written by Jeffrey Vernon Merkey that will read any XML dump provided by the foundation, extract all image names from the XML dump which it may reference, then generate a series of BASH or Bourne Unix style scripts which can be invoked to download all images from ''Wikimedia Commons'' and ''Wikipedia''.

The program relies on [cURL](http://curl.haxx.se/), an automated web spider, to download referenced images.  The program will also convert text based utf8 characters into actual utf8 strings for those dumps which may contain improperly formatted names for specific images.  The program can be configured to generate 16 parallel scripts which will download all images from Wikipedia.  The program includes Jeff Bezanson's utf8 libraries.

As of March 24, 2008, using a cable modem, the entire set of Wikipedia images can be downloaded in about 96 hours using this program (420 GB as of 3/24/08).  

## Compiling and Installation

### On Ubuntu

* Extract the contents of ''wikix.tar.gz''. Suppose the source code is extracted into ''/home/you/wikix''.
* Start your terminal program. e.g. ''Konsole'' (in KDE)
* You need to install some packages before you compile Wikix. Type in your terminal:

    sudo apt-get install libssl-dev build-essential curl

* Now goto the directory that contains the extracted source code, e.g. ''/home/you/wikix'', by typing

    cd /home/you/wikix

* Now type in your terminal.

    sudo make

Now if the compilation and linking of Wikix completes without errors then you will have a brand new executable - ''wikix'', in your ''/home/you/wikix'' (in this example) directory.

## Options

    # ./wikix -h
    USAGE:  wikix -htrciop < file.xml [ > script.out ]
                  -h   this help screen
                  -t   use xml dump to strip from tree
                  -r   wikipedia path
                  -c   commons path
                  -i   image path
                  -o   output path
                  -p   parallel (16 process) mode

## Example

The program would typically be invoked in a directory that you wish to use to host the images.  Wikix will construct a MediaWiki style directory structure which can be quickly imported into a MediaWiki Wikipedia installation (e.g., via *php rebuildImages.php --missing*):

    wikix -p < name_of_xml_file.xml > script.out &

The -p option tells wikix to create parallel scripts.  If you omit the -p option, it will create one very large file.  By default, the program is set up to mirror the English Wikipedia.  You can override the default settings by substituting path information for commons and the target Wikipedia site through the command line options.

The program will create a series of scripts as:

    image_sh
    image00
    image01
    image02
    image03
    image04
    image05
    image06
    image07
    image08
    image09
    image10
    image11
    image12
    image13
    image14
    image15

To start the download, simply type

    $./image_sh

In case you get the following error.

    ./image_sh: <line no.>: Syntax error: Bad fd number

Then please open the *image_sh* all the *imagexx* files and change the topmost line from

    #!/bin/sh

to

    #!/bin/bash

then re-type

    $./image_sh
