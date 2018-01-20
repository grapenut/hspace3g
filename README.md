# hspace3g
Modern HSpace3 clone made by grapenut.

Full scale demonstration can be had by connecting to grapenut.org:4201.

**Updated to work with PennMUSH as of 1/19/2018.**

INSTALLATION
------------

The following flags must be added prior to installation:
HS_ADMIN, HS_CELESTIAL, HS_COMM (@), HS_COMPONENT (&), HS_CONSOLE, HS_DRONE, HS_MISSION (y), HS_SHIP, HS_SHIPOBJ ($), HS_SIM, HS_UNIVERSE, HS_WEAPON (>)

You may quote the __parents/flags.txt__ file to add them automatically.

Copy __hspace.hlp__ to __pennmush/game/txt/hlp/__.

Copy __hspace.cnf__ to __pennmush/game/__.

Copy the __space/__ directory to __pennmush/src/__.

Copy __hspace.patch__ to __pennmush/src/__.

In __pennmush/src/__, execute:
```
patch < hspace.patch
```

In __pennmush/__, execute:
```
make hdrs/switches.h
```

In __pennmush/src/space/__, execute:
```
make
```

In __pennmush/__, execute:
```
make install
```

PARENTS
-------
Quote the parent files in the following order:
- celestials.txt
- components.txt
- weapons.txt
- console.txt

OBJECTS
-------
Just `@create` objects and `@set` the appropriate flag. Use `@space/load [\<object\>]` to load/reload objects.
`HELP SPACE ADMIN` and `HELP SPACE CREATE` will get you started on creating objects and setting the appropriate attributes.


INSTALL NOTES
-------------
In case the patch fails to update the __pennmush/src/SWITCHES__ file, I have provided a list of the switches used for space commands in __SWITCHES_SPACE__.
Move __SWITCHES_SPACE__ to __pennmush/src/__. Copy __pennmush/src/SWITCHES__ to __pennmush/src/SWITCHES_PENN__. Rebuild the SWITCHES file:
```
cat SWITCHES_PENN SWITCHES_SPACE | sort | uniq > SWITCHES
```



