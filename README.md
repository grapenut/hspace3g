# hspace3g
Modern HSpace3 clone made by grapenut.

Full scale demonstration can be had by connecting to grapenut.org:4201.

INSTALLATION
------------

The following flags must be added prior to installation:
HS_ADMIN, HS_CELESTIAL, HS_COMM (@), HS_COMPONENT (&), HS_CONSOLE, HS_DRONE, HS_MISSION (y), HS_SHIP, HS_SHIPOBJ ($), HS_SIM, HS_UNIVERSE, HS_WEAPON (>)

You may quote the __parents/flags.txt__ file to add them automatically.

Move __hspace.hlp__ to __pennmush/game/txt/hlp/__.

Move __hspace.cnf__ to __pennmush/game/space/__.

Move the __space/__ directory to __pennmush/src/__.

Move __hspace.patch__ to __pennmush/src/__ and execute:
```
patch -p0 < hspace.patch
```

Change to the __pennmush/src/space/__ directory and type `make`. You may need to type
`make` in __pennmush/src/__ first (it's ok if it fails, just need to update command switches).

PARENTS
-------
Quote the parent files in the following order:
- celestials.txt
- components.txt
- weapons.txt
- console.txt

OBJECTS
-------
Just @create objects and set the appropriate flag. Use @space/load [\<object\>] to load/reload objects. HELP SPACE ADMIN and HELP SPACE CREATE will get you started on creating objects and setting the appropriate attributes.

-grapenut
