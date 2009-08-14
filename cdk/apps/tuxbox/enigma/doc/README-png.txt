PNGs sollten 8bit sein, meist wohl mit palette (wenn die nicht gerade fuers
LCD sind).

die meisten Bildbearbeitungsprogramme (inkl. The Gimp, Photoshop,
Paint Shop Pro, Gif Movie Gear, ...) sind wohl zu bloed, echte transparency
auch bei indexed zu interstuetzen.

das einzige programm was das kann (neben einem $$$ konvertierungsprogramm
fuer nextstep) ist "pngquant"
(http://www.libpng.org/pub/png/apps/pngquant.html)


Bei dem Teil muesste man noch patchen dass es keine 4bpp bilder erzeugt wenn
<=16 farben im bild sind (colors = MIN (reqcolors, colors) oder so raus, immer
reqcolors nehmen.)

bei bedarf mach ich dadraus nen patch.

tmb