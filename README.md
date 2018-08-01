# hspace3g
Modern HSpace3 clone made by grapenut.

Full scale demonstration can be had by connecting to grapenut.org:4201.

**Updated to work with PennMUSH as of 1/19/2018.**

## INSTALLATION
------------

Copy __hspace.hlp__ to __pennmush/game/txt/hlp/__.

Copy __hspace.cnf__ to __pennmush/game/__.

Copy the __space/__ directory to __pennmush/src/__.

Copy __hspace.patch__ to __pennmush/src/__.

Copy __SWITCHES_SPACE__ to __pennmush/src/__.

In __pennmush/src/__, execute:
```
patch < hspace.patch
```
If any hunks fail, check the .rej file and make the changes manually.

Add the command switches, making a copy of Penn's switch list first.
```
cp SWITCHES SWITCHES_PENN
cat SWITCHES_PENN SWITCHES_SPACE | sort | uniq > SWITCHES
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

## PARENTS
-------
Quote the parent files in the following order:
- celestials.txt
- components.txt
- weapons.txt
- console.txt

## OBJECTS
-------
Just `@create` objects and `@set` the appropriate flag. Use `@space/load [\<object\>]` to load/reload objects.
`HELP SPACE ADMIN` and `HELP SPACE CREATE` will get you started on creating objects and setting the appropriate attributes.

