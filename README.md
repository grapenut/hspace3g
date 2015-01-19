# hspace3g
Modern HSpace3 clone made by grapenut.

INSTALLATION
------------
Add the following flags using @flag/add:
HS_ADMIN, HS_CELESTIAL, HS_COMM (@), HS_COMPONENT (&), HS_CONSOLE, HS_DRONE, HS_MISSION (y), HS_SHIP, HS_SHIPOBJ ($), HS_SIM, HS_UNIVERSE, HS_WEAPON (>)

Move hspace.hlp to pennmush/game/txt/hlp/

Move hspace.cnf to pennmush/game/space/

Move the space/ directory to pennmush/src/

Move hspace.patch to pennmush/src/ and execute:
  patch -p0 < hspace.patch

Change to the pennmush/src/space/ directory and type make. You may need to type make in pennmush/src/ first (it's ok if it fails, just need to update command switches).

PARENTS
-------
Quote the parent files. You will need to update dbrefs on attributes to match those of the created objects.

OBJECTS
-------
Just @create objects and set the appropriate flag. Use @space/load [\<object\>] to load/reload objects. HELP SPACE ADMIN and HELP SPACE CREATE will get you started on creating objects and setting the appropriate attributes.

-grapenut
