
#include <stdint.h>
#include "hspace.h"

/* permanent space objects live inside fixed length arrays */

int hs_num_universes;
huniverse *hs_universes;

int hs_num_celestials;
hcelestial *hs_celestials;

int hs_num_ships;
hship *hs_ships;

int hs_num_missions;
hmission *hs_missions;

int hs_num_combat_ids;

/* string mapping for type specifiers and flags */
/* use STRINGMAP(label, type) */
hstringmap hs_object_types[] =
{
  /* space object types */
  STRINGMAP(DRONE),
  STRINGMAP(CAPITAL),
  STRINGMAP(STATION),
  STRINGMAP(STAR),
  STRINGMAP(PLANET),
  {"JUMP POINT", (void*) HS_WORMHOLE },
  STRINGMAP(ASTEROID),
  STRINGMAP(ANOMALY),
  {"NAV POINT", (void*) HS_WAYPOINT },
  STRINGMAP(DEBRIS),
  STRINGMAP(SHIP),
  STRINGMAP(CELESTIAL),
  STRINGMAP(SENSOR_JAM),
  {NULL, NULL}
};

hstringmap hs_weapon_types[] =
{
  /* weapon object types */
  STRINGMAP(WEAPON),
  STRINGMAP(CANNON),
  STRINGMAP(MISSILE),
  STRINGMAP(EMITTER),
  STRINGMAP(WIRETAP),
  STRINGMAP(CAPACITOR),
  STRINGMAP(BOOSTER),
  STRINGMAP(PROTOSLOT),
  STRINGMAP(TORPEDO),
  {NULL, NULL}
};

hstringmap hs_system_types[] =
{
  /* system object types */
  STRINGMAP(HULL),
  STRINGMAP(SHIELD),
  STRINGMAP(REACTOR),
  STRINGMAP(ENGINE),
  STRINGMAP(SENSOR),
  STRINGMAP(COMPUTER),
  STRINGMAP(PROTOSYS),
  STRINGMAP(AFTERBURNER),
  {NULL, NULL}
};

hstringmap hs_console_types[] =
{
  /* console object types */
  STRINGMAP(NAV),
  STRINGMAP(GUN),
  STRINGMAP(OPS),
  STRINGMAP(ENG),
  STRINGMAP(CIV),
  {NULL, NULL}
};


/*********************************************************************/
/* basic allocation and string conversion utilities */

/* init the hs_universes, hs_celestials, and hs_ships arrays for permanent objects */
int init_arrays()
{

  hs_num_universes = 0;
  hs_num_celestials = 0;
  hs_num_ships = 0;
  hs_num_missions = 0;
  
  hs_num_combat_ids = atr_parse_integer(hs_options.space_wiz, "HS_COMBAT_IDS", 1);

  hs_universes = (huniverse *) malloc(sizeof(huniverse) * hs_options.max_universes);
  if (!hs_universes)
  {
    SPACEWALL("HSPACE: ERROR. Unable to allocate universes!");
    return 0;
  }

  hs_celestials = (hcelestial *) malloc(sizeof(hcelestial) * hs_options.max_celestials);
  if (!hs_celestials)
  {
    SPACEWALL("HSPACE: ERROR. Unable to allocate celestials!");
    return 0;
  }

  hs_ships = (hship *) malloc(sizeof(hship) * hs_options.max_ships);
  if (!hs_ships)
  {
    SPACEWALL("HSPACE: ERROR. Unable to allocate ships!");
    return 0;
  }
  
  hs_missions = (hmission *) malloc(sizeof(hmission) * hs_options.max_missions);
  if (!hs_missions)
  {
    SPACEWALL("HSPACE: ERROR. Unable to allocate missions!");
    return 0;
  }
  
  return 1;
}

/* parse an integer directly from an attribute */
int atr_parse_integer(dbref obj, char *name, int default_value)
{
  ATTR *a;
  
  a = atr_get(obj, name);
  if (!a)
    return default_value;
  
  return parse_integer(atr_value(a));
}

/* parse a dbref directly from an attribute */
dbref atr_parse_dbref(dbref obj, char *name)
{
  ATTR *a;
  
  a = atr_get(obj, name);
  if (!a)
    return NOTHING;
  
  return parse_dbref(atr_value(a));
}

/* parse an attribute for a double value */
double atr_parse_double(dbref obj, char *name, double default_value)
{
  ATTR *a;
  
  a = atr_get(obj, name);
  if (!a)
    return default_value;
  
  return strtod(atr_value(a), NULL);
}

/* return the attribute string in a one-liner */
char *atr_parse_string(dbref obj, char *name)
{
  ATTR *a;
  
  a = atr_get(obj, name);
  if (!a)
    return "";
  
  return atr_value(a);
}

/* map a string to a value from an hstringmap table */
void *stringmap_value(hstringmap *stringmap, const char *s)
{
  hstringmap *map;
  
  if (!stringmap || !s)
    return NULL;
  
  for (map = stringmap; map && map->str; map++)
  {
    if (!strncasecmp(map->str, s, strlen(s)))
      return (void*)(map->val);
  }

  return NULL;
}

/* map a value to a string from an hstringmap table */
char *stringmap_string(hstringmap *stringmap, void *data)
{
  hstringmap *map;
  
  if (!stringmap || !data)
    return NULL;
  
  for (map = stringmap; map && map->str; map++)
  {
    if (map->val == data)
      return (char*)(map->str);
  }

  return NULL;
}

/* parse a string and return the flag mask */
int parse_flags(hstringmap *map, char *buff)
{
  char *r, *s;
  int f, flags;
  
  flags = 0x0;

  s = buff;
  r = split_token(&s, ' ');
  f = (int) (intptr_t) stringmap_value(map, r);
  if (f)
  {
    FlagOn(flags, f);
  }
  
  while (s)
  {
    r = split_token(&s, ' ');
    f = (int) (intptr_t) stringmap_value(map, r);
    if (f)
    {
      FlagOn(flags, f);
    }
  }
  
  return flags;
}

/* parse a flag string directly from an attribute */
int atr_parse_flags(dbref obj, hstringmap *map, char *name)
{
  ATTR *a;
  char *buff;
  int f;

  if (!map)
    return 0x0;

  a = atr_get(obj, name);
  if (!a)
    return 0x0;

  buff = safe_atr_value(a, "FLAGSTRING");
  
  f = parse_flags(map, buff);
  
  free(buff);
  
  return f;
}

/* parse a callback id from the callback stringmap */
hcallback atr_parse_callback(dbref obj, char *name)
{
  ATTR *a;
  hcallback call;

  a = atr_get(obj, name);
  if (!a)
    return NULL;

  call = stringmap_value(hs_callbacks, atr_value(a));
  
  return call;
}

/************************************************************/
/* universe routines */

/* load a new universe */
huniverse *load_universe(dbref player, dbref obj)
{
  huniverse *univ;
  int id;
  
  if (!RealGoodObject(obj) || !IsUniverse(obj))
  {
    notify(player, "HSPACE: That is not a universe!");
    return NULL;
  }

  /* check to see if it is already loaded */
  for (id = 0; id < hs_num_universes; id++)
  {
    if (hs_universes[id].objnum == obj)
    {
      notify(player, "HSPACE: That universe is already loaded.");
      return NULL;
    }
  }
  
  if (hs_num_universes >= hs_options.max_universes)
  {
    notify(player, "HSPACE: Unable to load, maximum universes.");
    return NULL;
  }
  
  univ = &hs_universes[hs_num_universes];
  atr_add(obj, "HSID", tprintf("%d", hs_num_universes), hs_options.space_wiz, 0);
  hs_num_universes++;

  univ->objnum = obj;
  
  univ->pvp = atr_parse_integer(obj, "PVP", 0);
  
  univ->num_ships = 0;
  univ->num_drones = 0;
  univ->num_celestials = 0;
  
  univ->head_ship = NULL;
  univ->head_drone = NULL;
  univ->head_celestial = NULL;
  univ->head_debris = NULL;
  
  notify(player, "HSPACE: Universe loaded.");
  return univ;
}

huniverse *find_universe(dbref obj)
{
  int id;
  
  if (!IsUniverse(obj))
    return NULL;
  
  /* check the nav for an HSID index shortcut */
  id = atr_parse_integer(obj, "HSID", -1);
  if (id >= 0 && id < hs_num_universes && hs_universes[id].objnum == obj)
    return &(hs_universes[id]);

  /* HSID is not the correct universe */
  /* search all ships for the correct one */
  for (id = 0; id < hs_num_universes; id++)
  {
    if (hs_universes[id].objnum == obj)
    {
      atr_add(obj, "HSID", tprintf("%d", id), hs_options.space_wiz, 0);
      return &(hs_universes[id]);
    }
  }
  
  atr_clr(obj, "HSID", hs_options.space_wiz);
  return NULL;
}

/**********************************************************/
/* celestial routines */

int load_celestial_attributes(hcelestial *cel, int reload)
{
  dbref obj;
  
  if (!cel)
    return 0;
  
  if (!RealGoodObject(cel->objnum))
    return 0;
  
  obj = cel->objnum;
  return 1;
}

/* load a new celestial */
hcelestial *load_celestial(dbref player, dbref obj)
{
  hcelestial *cel;
  int id, use_id, reload, type;
  huniverse *uid;
  
  if (!RealGoodObject(obj) || !IsCelestial(obj))
  {
    notify(player, "HSPACE: That is not a celestial!");
    return NULL;
  }

  type = HS_CELESTIAL | atr_parse_flags(obj, hs_object_types, "TYPE");
  if (!HasFlag(type, HS_ANY_CELESTIAL))
  {
    notify(player, "HSPACE: Unable to load, invalid object type");
    return NULL;
  }

  uid = find_universe(atr_parse_dbref(obj, "UNIVERSE"));
  if (!uid)
  {
    notify(player, "HSPACE: Unable to load, invalid universe.");
    return NULL;
  }
  
  /* check to see if it is already loaded */
  use_id = hs_num_celestials;
  reload = 0;
  for (id = 0; id < hs_num_celestials; id++)
  {
    if (hs_celestials[id].objnum == obj)
    {
      //notify(player, "HSPACE: That celestial is already loaded.");
      //return NULL;
      use_id = id;
      reload = 1;
    }
  }
  
  if (hs_num_celestials >= hs_options.max_celestials)
  {
    notify(player, "HSPACE: Unable to load, maximum celestials.");
    return NULL;
  }
  
  id = use_id;
  cel = &hs_celestials[id];

  /* setup basic attributes */
  cel->objnum = obj;
  cel->type = type;

  cel->x = atr_parse_double(obj, "X", 0.0);
  cel->y = atr_parse_double(obj, "Y", 0.0);
  cel->z = atr_parse_double(obj, "Z", 0.0);
  
  cel->mass = atr_parse_double(obj, "MASS", 0.0);
  cel->radius = atr_parse_double(obj, "RADIUS", 0.0);
  cel->contents = NULL;
  
  if (!reload)
  {
    cel->uid = NULL;
    cel->next = NULL;
  }

  if (Owner(obj) != hs_options.space_wiz)
  {
    chown_object(hs_options.space_wiz, obj, hs_options.space_wiz, 1);
  }

  atr_add(obj, "HSID", tprintf("%d", id), hs_options.space_wiz, 0);
  
  /* use the move_celestial() routine to make sure that universe
     linked lists are setup properly */
  move_celestial(cel, uid, cel->x, cel->y, cel->z);
  
  if (!reload)
    hs_num_celestials++;

  notify(player, "HSPACE: Celestial loaded.");
  return cel;
}

/* create a transient celestial contact */
hcelestial *create_debris(huniverse *uid, double x, double y, double z)
{
  hcelestial *cel;
  
  if (!uid)
    return NULL;
  
  cel = (hcelestial *) malloc(sizeof(hcelestial));
  if (!cel)
    return NULL;
  
  cel->objnum = NOTHING;
  cel->type = HS_DEBRIS | HS_CELESTIAL;
  
  cel->x = x;
  cel->y = y;
  cel->z = z;
  
  cel->mass = 0.0;
  cel->radius = 100.0;
  
  cel->contents = NULL;
  
  cel->uid = NULL;
  cel->next = NULL;
  
  move_celestial(cel, uid, x, y, z);
  
  return cel;
}

/* save a celestial to disk */
void dump_celestial(hcelestial *cel)
{
  dbref obj;

  if (!cel)
    return;
  
  obj = cel->objnum;
  
  /* object will be deleted on the next reboot */
  if (!IsCelestial(obj))
  {
  /*  atr_clr(obj, "UNIVERSE", hs_options.space_wiz);
    atr_clr(obj, "X", hs_options.space_wiz);
    atr_clr(obj, "Y", hs_options.space_wiz);
    atr_clr(obj, "Z", hs_options.space_wiz);
    atr_clr(obj, "MASS", hs_options.space_wiz); */
    return;
  }

  if (cel->uid)
    atr_add(obj, "UNIVERSE", unparse_dbref(cel->uid->objnum), hs_options.space_wiz, 0);
  else
    atr_add(obj, "UNIVERSE", unparse_dbref(NOTHING), hs_options.space_wiz, 0);
  
  atr_add(obj, "X", tprintf("%f", cel->x), hs_options.space_wiz, 0);
  atr_add(obj, "Y", tprintf("%f", cel->y), hs_options.space_wiz, 0);
  atr_add(obj, "Z", tprintf("%f", cel->z), hs_options.space_wiz, 0);
  
  //atr_add(obj, "MASS", tprintf("%f", cel->mass), hs_options.space_wiz, 0);
  //atr_add(obj, "RADIUS", tprintf("%f", cel->radius), hs_options.space_wiz, 0);
}

hcelestial *find_celestial(dbref obj)
{
  int id;
  
  if (!IsCelestial(obj))
    return NULL;
  
  /* check the nav for an HSID index shortcut */
  id = atr_parse_integer(obj, "HSID", -1);
  if (id >= 0 && id < hs_num_celestials && hs_celestials[id].objnum == obj)
    return &(hs_celestials[id]);

  /* HSID is not the correct celestial */
  /* search all ships for the correct one */
  for (id = 0; id < hs_num_celestials; id++)
  {
    if (hs_celestials[id].objnum == obj)
    {
      atr_add(obj, "HSID", tprintf("%d", id), hs_options.space_wiz, 0);
      return &(hs_celestials[id]);
    }
  }
  
  atr_clr(obj, "HSID", hs_options.space_wiz);
  return NULL;
}

/********************************************************/
/* ship routines */

int load_ship_attributes(hship *ship, int reload)
{
  dbref obj;
  dbref manner;
  dbref shipobj;
  int ptype, stype, type;
  int i, n;
  
  if (!ship)
    return 0;

  if (!RealGoodObject(ship->objnum))
    return 0;
  
  obj = ship->objnum;
  
  ship->combat_timer = 35;
  ship->cid = 0;
  
  ship->x = atr_parse_double(obj, "X", 0.0);
  ship->y = atr_parse_double(obj, "Y", 0.0);
  ship->z = atr_parse_double(obj, "Z", 0.0);
  
  ship->speed = 0.0;
  ship->desired_speed = 0.0;
  
  ship->vx = 0.0;
  ship->vy = 1.0;
  ship->vz = 0.0;

  ship->xyhead = atr_parse_double(obj, "XYHEAD", 0.0);
  ship->desired_xyhead = ship->xyhead;

  ship->zhead = atr_parse_double(obj, "ZHEAD", 0.0);
  ship->desired_zhead = ship->zhead;
  
  manner = get_user(obj);
  if (!RealGoodObject(manner))
    manner = obj;
    
  ptype = load_weapon(manner, &(ship->primary), HS_PRIMARY);
  stype = load_weapon(manner, &(ship->secondary), HS_SECONDARY);
  type = HasFlag(ptype | stype, HS_ANY_WEAPON);
  
  if (HasFlag(ptype, HS_CANNON))
  {
    /* this shouldn't happen, nav console with cannons */
    notify_console(obj, "Error. Unable to load a cannon on this console.");
    clear_weapon(&(ship->primary));
  }
  else if (HasFlag(stype, HS_CANNON))
  {
    /* this shouldn't happen, nav console with cannons */
    notify_console(obj, "Error. Unable to load a cannon on this console.");
    clear_weapon(&(ship->secondary));
  }

  ship->prompt = HasFlag(atr_parse_integer(manner, "HSPROMPT_FREQUENCY", 1), HS_PROMPT_FREQUENCY);
  FlagOn(ship->prompt, atr_parse_flags(manner, hs_prompt_flags, "HSPROMPT_FLAGS"));

  load_system(obj, &(ship->hull), HS_HULL);
  load_system(obj, &(ship->shield), HS_SHIELD);
  load_system(obj, &(ship->engine), HS_ENGINE);
  load_system(obj, &(ship->reactor), HS_REACTOR);
  load_system(obj, &(ship->sensor), HS_SENSOR);
  load_system(obj, &(ship->computer), HS_COMPUTER);
  
  if (!reload)
    ship->nconsoles = 0;
  else
  {
    n = ship->nconsoles;
    ship->nconsoles = 0;
    
    for (i = 0; i < n; i++)
    {
      load_console(hs_options.space_wiz, ship->console[i].objnum);
    }
  }
  
  ship->lock = NULL;
  
  ship->waypoint.objnum = NOTHING;
  ship->waypoint.type = HS_CELESTIAL | HS_WAYPOINT;
  ship->waypoint.uid = find_universe(atr_parse_dbref(obj, "WAYPOINT_UID"));
  ship->waypoint.x = atr_parse_double(obj, "WAYPOINT_X", 0.0);
  ship->waypoint.y = atr_parse_double(obj, "WAYPOINT_Y", 0.0);
  ship->waypoint.z = atr_parse_double(obj, "WAYPOINT_Z", 0.0);
  ship->waypoint.mass = 0.0;
  ship->waypoint.radius = 0.0;
  ship->waypoint.contents = NULL;
  ship->waypoint.next = NULL;
  
  ship->landed = NULL;
  ship->docked = NULL;
  ship->linked = NULL;
  
  ship->landing = 0;
  ship->launching = 0;
  
  ship->dropto = NOTHING;
  
  ship->heartbeat = atr_parse_callback(obj, "HEARTBEAT");
  ship->update_prox = atr_parse_callback(obj, "UPDATE_PROXIMITY");
  ship->update_contact = atr_parse_callback(obj, "UPDATE_CONTACT");

  shipobj = atr_parse_dbref(obj, "SHIPOBJ");
  if (RealGoodObject(shipobj) && Owner(shipobj) != hs_options.space_wiz)
  {
    chown_object(hs_options.space_wiz, shipobj, hs_options.space_wiz, 1);
  }

  if (!reload)
  {
    ship->uid = NULL;
    ship->next = NULL;

    ship->ncontacts = 0;
    ship->head_contact = NULL;
    ship->mothership = NULL;

    ship->nbuffs = 0;
    ship->head_buff = NULL;
    
    ship->wp_contact = NULL;
    
    ship->spawn = NULL;
  } else {
    clear_contacts(ship);
    clear_buffs(ship);
  }
  
  return 1;
}


/* load a standard ship into the main hs_ships array */
hship *load_ship(dbref player, dbref obj)
{
  hship *ship;
  int id, use_id, reload, success;
  huniverse *uid;
  hcelestial *landed;
  hship *docked;
  
  if (!RealGoodObject(obj) || !IsShip(obj))
  {
    notify(player, "HSPACE: That is not a ship!");
    return NULL;
  }

  uid = find_universe(atr_parse_dbref(obj, "UNIVERSE"));
  landed = find_celestial(atr_parse_dbref(obj, "LANDED"));
  docked = find_ship_by_nav(atr_parse_dbref(obj, "DOCKED"));

  if (!(uid || landed || docked))
  {
    notify(player, "HSPACE: Unable to load, invalid universe/landed/docked location.");
    return NULL;
  }
  /* check to see if it is already loaded */
  use_id = hs_num_ships;
  reload = 0;
  for (id = 0; id < hs_num_ships; id++)
  {
    if (hs_ships[id].objnum == obj)
    {
      //notify(player, "HSPACE: That ship is already loaded.");
      //return NULL;
      use_id = id;
      reload = 1;
    }
  }
  
  if (hs_num_ships >= hs_options.max_ships)
  {
    notify(player, "HSPACE: Unable to load, maximum ships.");
    return NULL;
  }
  
  id = use_id;
  ship = &hs_ships[id];
  
  /* minimum attributes required to call load_ship_attributes() */
  ship->type = HS_SHIP | atr_parse_flags(obj, hs_object_types, "TYPE");
  ship->objnum = obj;
  
  success = load_ship_attributes(ship, reload);
  if (!success)
  {
    notify(player, "HSPACE: Failed to load ship attributes.");
    return NULL;
  }

  if (Owner(obj) != hs_options.space_wiz)
  {
    chown_object(hs_options.space_wiz, obj, hs_options.space_wiz, 1);
  }
  
  atr_add(obj, "HSID", tprintf("%d", id), hs_options.space_wiz, 0);

  if (!reload)
    hs_num_ships++;

  if (uid)
  {
    /* use the move_ship() routine to make sure that universe
       linked lists are setup properly */
    move_ship(ship, uid, ship->x, ship->y, ship->z);
  }
  else if (landed)
  {
    ship->landed = landed;
  }
  else if (docked)
  {
    ship->docked = docked;
  }

  if (!reload)
    notify(player, "HSPACE: Ship loaded.");
  else
    notify(player, "HSPACE: Ship reloaded.");
    
  return ship;
}


/* create a drone ship */
hship *create_drone(dbref obj)
{
  hship *ship;
  huniverse *uid;
  
  if (!RealGoodObject(obj) || !IsDrone(obj))
    return NULL;

  uid = find_universe(atr_parse_dbref(obj, "UNIVERSE"));
  if (!uid)
    return NULL;

  ship = (hship *) malloc(sizeof(hship));
  if (!ship)
    return NULL;
  
  /* minimum attributes required to call load_ship_attributes() */
  ship->type = HS_SHIP | HS_DRONE;
  ship->objnum = obj;
  
  if (!load_ship_attributes(ship, 0))
  {
    free(ship);
    return NULL;
  }
  
  move_ship(ship, uid, ship->x, ship->y, ship->z);
  
  return ship;
}


/* spawn a drone ship */
hship *spawn_drone(dbref obj, huniverse *uid, double x, double y, double z)
{
  hship *ship;
  
  if (!RealGoodObject(obj) || !IsDrone(obj))
    return NULL;

  if (!uid)
    return NULL;

  ship = (hship *) malloc(sizeof(hship));
  if (!ship)
    return NULL;
  
  /* minimum attributes required to call load_ship_attributes() */
  ship->type = HS_SHIP | HS_DRONE;
  ship->objnum = obj;
  
  if (!load_ship_attributes(ship, 0))
  {
    free(ship);
    return NULL;
  }
  
  move_ship(ship, uid, x, y, z);
  
  return ship;
}


/* dump ship variables to mush attributes */
void dump_ship(hship *ship)
{
  int c;
  dbref obj, cobj;
  
  if (!ship)
    return;
  
  obj = ship->objnum;
  
  if (!IsShip(obj))
    return;
  
  if (ship->uid)
    atr_add(obj, "UNIVERSE", unparse_dbref(ship->uid->objnum), hs_options.space_wiz, 0);
  else
    atr_add(obj, "UNIVERSE", unparse_dbref(NOTHING), hs_options.space_wiz, 0);
  
  atr_add(obj, "X", tprintf("%f", ship->x), hs_options.space_wiz, 0);
  atr_add(obj, "Y", tprintf("%f", ship->y), hs_options.space_wiz, 0);
  atr_add(obj, "Z", tprintf("%f", ship->z), hs_options.space_wiz, 0);
  atr_add(obj, "XYHEAD", tprintf("%g", ship->xyhead), hs_options.space_wiz, 0);
  atr_add(obj, "ZHEAD", tprintf("%g", ship->zhead), hs_options.space_wiz, 0);
  
  if (ship->waypoint.uid)
    atr_add(obj, "WAYPOINT_UID", unparse_dbref(ship->waypoint.uid->objnum), hs_options.space_wiz, 0);
  else
    atr_add(obj, "WAYPOINT_UID", unparse_dbref(NOTHING), hs_options.space_wiz, 0);

  atr_add(obj, "WAYPOINT_X", tprintf("%f", ship->waypoint.x), hs_options.space_wiz, 0);
  atr_add(obj, "WAYPOINT_Y", tprintf("%f", ship->waypoint.y), hs_options.space_wiz, 0);
  atr_add(obj, "WAYPOINT_Z", tprintf("%f", ship->waypoint.z), hs_options.space_wiz, 0);

  //atr_add(obj, "PRIMARY", unparse_dbref(ship->primary.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->primary.objnum))
    atr_add(ship->primary.objnum, "CONDITION", tprintf("%g", ship->primary.condition), hs_options.space_wiz, 0);

  //atr_add(obj, "SECONDARY", unparse_dbref(ship->secondary.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->secondary.objnum))
    atr_add(ship->secondary.objnum, "CONDITION", tprintf("%g", ship->secondary.condition), hs_options.space_wiz, 0);

  //atr_add(obj, "HULL", unparse_dbref(ship->hull.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->hull.objnum))
    atr_add(ship->hull.objnum, "CONDITION", tprintf("%g", ship->hull.condition), hs_options.space_wiz, 0);

  //atr_add(obj, "SHIELD", unparse_dbref(ship->shield.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->shield.objnum))
    atr_add(ship->shield.objnum, "CONDITION", tprintf("%g", ship->shield.condition), hs_options.space_wiz, 0);

  //atr_add(obj, "ENGINE", unparse_dbref(ship->engine.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->engine.objnum))
    atr_add(ship->engine.objnum, "CONDITION", tprintf("%g", ship->engine.condition), hs_options.space_wiz, 0);

  //atr_add(obj, "REACTOR", unparse_dbref(ship->reactor.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->reactor.objnum))
    atr_add(ship->reactor.objnum, "CONDITION", tprintf("%g", ship->reactor.condition), hs_options.space_wiz, 0);

  //atr_add(obj, "SENSOR", unparse_dbref(ship->sensor.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->sensor.objnum))
    atr_add(ship->sensor.objnum, "CONDITION", tprintf("%g", ship->sensor.condition), hs_options.space_wiz, 0);

  //atr_add(obj, "COMPUTER", unparse_dbref(ship->computer.objnum), hs_options.space_wiz, 0);
  if (RealGoodObject(ship->computer.objnum))
    atr_add(ship->computer.objnum, "CONDITION", tprintf("%g", ship->computer.condition), hs_options.space_wiz, 0);
  
  if (ship->landed)
    cobj = ship->landed->objnum;
  else
    cobj = NOTHING;
    
  atr_add(obj, "LANDED", unparse_dbref(cobj), hs_options.space_wiz, 0);
  
  if (ship->docked)
    cobj = ship->docked->objnum;
  else
    cobj = NOTHING;

  atr_add(obj, "DOCKED", unparse_dbref(cobj), hs_options.space_wiz, 0);
  
  for (c = 0; c < ship->nconsoles; c++)
  {
    cobj = ship->console[c].objnum;
    atr_add(cobj, "XYHEAD", tprintf("%g", ship->xyhead), hs_options.space_wiz, 0);
    atr_add(cobj, "ZHEAD", tprintf("%g", ship->zhead), hs_options.space_wiz, 0);

    //atr_add(cobj, "PRIMARY", unparse_dbref(ship->console[c].primary.objnum), hs_options.space_wiz, 0);
    if (RealGoodObject(ship->console[c].primary.objnum))
      atr_add(ship->console[c].primary.objnum, "CONDITION", tprintf("%g", ship->console[c].primary.condition), hs_options.space_wiz, 0);

    //atr_add(cobj, "SECONDARY", unparse_dbref(ship->secondary.objnum), hs_options.space_wiz, 0);
    if (RealGoodObject(ship->console[c].secondary.objnum))
      atr_add(ship->console[c].secondary.objnum, "CONDITION", tprintf("%g", ship->console[c].secondary.condition), hs_options.space_wiz, 0);
  }

  return;
}

/* locate the hship point for a given nav console dbref */
/* assumes IsShip(nav) is true! */
hship *find_ship_by_nav(dbref nav)
{
  ATTR *a;
  int id;
  
  /* check the nav for an HSID index shortcut */
  a = atr_get(nav, "HSID");
  if (a)
  {
    id = parse_integer(atr_value(a));
    if (id >= 0 && id < hs_num_ships && hs_ships[id].objnum == nav)
      return &(hs_ships[id]);
  }

  /* HSID is not the correct ship */
  /* search all ships for the correct one */
  for (id = 0; id < hs_num_ships; id++)
  {
    if (hs_ships[id].objnum == nav)
    {
      atr_add(nav, "HSID", tprintf("%d", id), hs_options.space_wiz, 0);
      return &(hs_ships[id]);
    }
  }

  /* this object was not found in the ship database! */
  /* we should remove the HSID, since it is invalid */
  /* but don't unflag, it will be loaded in the next reboot */
  if (a)
    atr_clr(nav, "HSID", hs_options.space_wiz);

  return NULL;
}

/* find hship pointer for various objects (ship, console, player) */
hship *find_ship(dbref obj)
{
  int id;
  int cid;
  dbref nav;
  hship *ship;
  
  if (IsShip(obj))
    return find_ship_by_nav(obj);

  /* check consoles for HSNAV attribute shortcut */
  if (IsConsole(obj))
  {
    nav = atr_parse_dbref(obj, "HSNAV");
    if (IsShip(nav))
    {
      ship = find_ship_by_nav(nav);
      if (ship)
      {
        for (cid = 0; cid < ship->nconsoles; cid++)
        {
          if (ship->console[cid].objnum == obj)
            return ship;
        }
      }
    }
    
    /* HSNAV is not a valid ship! */
    /* search all ships and all consoles */
    for (id = 0; id < hs_num_ships; id++)
    {
      for (cid = 0; cid < hs_ships[id].nconsoles; cid++)
      {
        if (hs_ships[id].console[cid].objnum == obj)
        {
          atr_add(obj, "HSNAV", unparse_dbref(hs_ships[id].objnum), hs_options.space_wiz, 0);
          return &(hs_ships[id]);
        }
      }
    }
    
    /* the console was not found! */
    /* don't remove any flags or attributes */
    /* they may become active after the next reload */
    return NULL;
  }
  
  if (IsShipObj(obj))
  {
    nav = atr_parse_dbref(obj, "HSNAV");
    if (IsShip(nav))
    {
      ship = find_ship_by_nav(nav);
      if (ship && atr_parse_dbref(ship->objnum, "SHIPOBJ") == obj)
        return ship;
    }
    
    /* HSNAV is not a valid ship! */
    /* search all shipobjs */
    for (id = 0; id < hs_num_ships; id++)
    {
      if (atr_parse_dbref(hs_ships[id].objnum, "SHIPOBJ") == obj)
      {
        atr_add(obj, "HSNAV", unparse_dbref(hs_ships[id].objnum), hs_options.space_wiz, 0);
        return &(hs_ships[id]);
      }
    }
    
    /* the shipobj was not found! */
    /* don't remove any flags or attributes */
    /* they may become active after the next reload */
    return NULL;
  }
  
  /* 1. check HSPACE attribute for nav */
  /* 2. check nav for ship */
  nav = atr_parse_dbref(obj, "HSPACE");
  if (IsShip(nav))
  {
    return find_ship_by_nav(nav);
  }
  
  return NULL;
}


/****************************************************************/
/* console routines */

hconsole *load_console(dbref player, dbref obj)
{
  hconsole *con;
  hship *ship;
  dbref nav;
  dbref manner;
  int ptype, stype, type;
  
  if (!RealGoodObject(obj) || !IsConsole(obj))
  {
    notify(player, "HSPACE: That is not a console!");
    return NULL;
  }

  ship = find_ship(obj);
  if (ship)
  {
      notify(player, "HSPACE: That console is already loaded.");
      return NULL;
  }
  
  nav = atr_parse_dbref(obj, "HSNAV");
  if (!IsShip(nav))
  {
    notify(player, "HSPACE: Unable to load, HSNAV does not contain an active nav console dbref.");
    return NULL;
  }
  
  ship = find_ship_by_nav(nav);
  if (!ship)
  {
    notify(player, "HSPACE: Unable to load, HSNAV does not contain an active nav console dbref.");
    return NULL;
  }

  if (ship->nconsoles >= MAX_CONSOLES)
  {
    notify(player, "HSPACE: Unable to load, maximum consoles reached.");
    return NULL;
  }
  
  con = &(ship->console[ship->nconsoles]);

  /* objnum, nav, type, lock, xyhead, zhead, primary, secondary */
  con->objnum = obj;
  con->nav = nav;
  
  
  /* don't need to do this, we will allow consoles to be untyped
     until someone mans them with certain equipment */
  /*con->type = atr_parse_flags(obj, hs_console_types, "TYPE");
  if (con->type < 0)
  {
    notify(player, "HSPACE: Unable to load, invalid console type");
    return NULL;
  }*/
  
  con->lock = NULL;
  
  con->xyhead = atr_parse_double(obj, "XYHEAD", 0.0);
  con->desired_xyhead = con->xyhead;
  con->zhead = atr_parse_double(obj, "ZHEAD", 0.0);
  con->desired_zhead = con->zhead;
  
  manner = get_user(obj);
  if (!RealGoodObject(manner))
    manner = obj;

  ptype = load_weapon(manner, &(con->primary), HS_PRIMARY);
  stype = load_weapon(manner, &(con->secondary), HS_SECONDARY);
  type = HasFlag(ptype | stype, HS_ANY_WEAPON);
  
  if (HasFlag(ptype, HS_MISSILE))
  {
    /* this shouldn't happen, aux console with missiles */
    notify(player, "Auxilitary console tried to load missiles! Clearing weapons.");
    notify_console(obj, "Error. Unable to load missiles on this console.");
    clear_weapon(&(con->primary));
  }
  else if (HasFlag(stype, HS_MISSILE))
  {
    /* this shouldn't happen, aux console with missiles */
    notify(player, "Auxilitary console tried to load missiles! Clearing weapons.");
    notify_console(obj, "Error. Unable to load missiles on this console.");
    clear_weapon(&(con->secondary));
  }
  
  /* default to civilian console type. can see things on sensors
     but no other abilities are available */
  if (HasFlag(type, HS_WIRETAP))
    con->type = HS_OPS;
  else if (HasFlag(type, HS_CAPACITOR))
    con->type = HS_ENG;
  else if (HasFlag(type, HS_CANNON | HS_EMITTER | HS_WEAPON))
    con->type = HS_GUN;
  else
    con->type = HS_CIV;
  
  con->prompt = HasFlag(atr_parse_integer(manner, "HSPROMPT_FREQUENCY", 1), HS_PROMPT_FREQUENCY);
  FlagOn(con->prompt, atr_parse_flags(manner, hs_prompt_flags, "HSPROMPT_FLAGS"));

  con->heartbeat = atr_parse_callback(obj, "HEARTBEAT");

  ship->nconsoles++;

  if (Owner(obj) != hs_options.space_wiz)
  {
    chown_object(hs_options.space_wiz, obj, hs_options.space_wiz, 1);
  }
  
  atr_add(obj, "HSPACE", unparse_dbref(ship->objnum), hs_options.space_wiz, 0);

  notify(player, "HSPACE: Console loaded.");
  return con;
}

/* locate the console for a given dbref */
hconsole *find_console(dbref obj)
{
  dbref nav;
  int id, cid;
  hship *ship;
  
  if (!IsConsole(obj))
    return NULL;
  
  nav = atr_parse_dbref(obj, "HSNAV");
  if (IsShip(nav))
  {
    ship = find_ship_by_nav(nav);
    if (ship)
    {
      for (cid = 0; cid < ship->nconsoles; cid++)
      {
        if (ship->console[cid].objnum == obj)
          return &(ship->console[cid]);
      }
    }
  }
  
  /* HSNAV is not a valid ship! */
  /* search all ships and all consoles */
  for (id = 0; id < hs_num_ships; id++)
  {
    for (cid = 0; cid < hs_ships[id].nconsoles; cid++)
    {
      if (hs_ships[id].console[cid].objnum == obj)
      {
        atr_add(obj, "HSNAV", unparse_dbref(hs_ships[id].objnum), hs_options.space_wiz, 0);
        return &(hs_ships[id].console[cid]);
      }
    }
  }
  
  /* the console was not found! */
  /* don't remove any flags or attributes */
  /* they may become active after the next reload */
  return NULL;
}

/***********************************************************/
/* system routines */

void clear_weapon(hweapon *gun)
{
  if (!gun)
    return;
    
  gun->objnum = NOTHING;
  gun->type = 0;
  gun->condition = 0.0;
  gun->reload = 0.0;
  gun->range = 0.0;
  gun->damage = 0.0;
  gun->accuracy = 0.0;
  gun->power = 0.0;
  gun->curpower = 0.0;
  gun->maxpower = 0.0;
  gun->speed = 0.0;
  gun->loading = 0;
  
}

int load_weapon(dbref console, hweapon *gun, hslot type)
{
  dbref obj;
  
  if (!gun)
    return 0;
  
  clear_weapon(gun);
  
  if (type == HS_PRIMARY)
    obj = atr_parse_dbref(console, "PRIMARY");
  else if (type == HS_SECONDARY)
    obj = atr_parse_dbref(console, "SECONDARY");
  else
    return 0;

  if (!IsWeapon(obj))
    return 0;

  /* type, objnum, reload, range, strength, accuracy, 
     power, curpower, maxpower, speed, loaded, loading */
  
  gun->type = type | atr_parse_flags(obj, hs_weapon_types, "TYPE");

  if (!HasFlag(gun->type, HS_ANY_WEAPON))
  {
    gun->type = 0;
    return 0;
  }
  
  gun->objnum = obj;

  gun->condition = atr_parse_double(obj, "CONDITION", 0.0);
  gun->reload = atr_parse_double(obj, "RELOAD", 0.0);
  gun->range = atr_parse_double(obj, "RANGE", 0.0);
  gun->damage = atr_parse_double(obj, "DAMAGE", 0.0);
  gun->accuracy = atr_parse_double(obj, "ACCURACY", 0.0);
  gun->power = atr_parse_double(obj, "POWER", 0.0);

  gun->curpower = 0.0;
  gun->maxpower = atr_parse_double(obj, "MAXPOWER", 0.0);

  gun->speed = atr_parse_double(obj, "SPEED", 0.0);

  gun->loading = gun->reload;

  return gun->type;
}


/* load  ship system */
int load_system(dbref nav, hsystem *sys, hsys type)
{
  int energy;
  dbref obj;
  
  if (!sys)
    return 0;
    
  sys->objnum = NOTHING;
  sys->condition = 0.0;
  sys->rate = 0.0;
  sys->strength = 0.0;
  sys->efficiency = 0.0;
  sys->energy = 0.0;
  sys->maxenergy = 0.0;
  
  energy = 0;
  
  switch (type) {
  case HS_HULL:
    obj = atr_parse_dbref(nav, "HULL");
    energy = 1;
    break;
  case HS_SHIELD:
    obj = atr_parse_dbref(nav, "SHIELD");
    energy = 1;
    break;
  case HS_ENGINE:
    obj = atr_parse_dbref(nav, "ENGINE");
    break;
  case HS_REACTOR:
    obj = atr_parse_dbref(nav, "REACTOR");
    break;
  case HS_SENSOR:
    obj = atr_parse_dbref(nav, "SENSOR");
    break;
  case HS_COMPUTER:
    obj = atr_parse_dbref(nav, "COMPUTER");
    break;
  default:
    return 0;
    break;
  }

  if (!IsComponent(obj))
    return 0;
  
  sys->type = type | HasFlag(atr_parse_flags(obj, hs_system_types, "TYPE"), ~HS_SYSTEMS);
  if (!HasFlag(sys->type, type))
  {
    sys->type = 0;
    return 0;
  }
  
  sys->objnum = obj;
  
  sys->condition = atr_parse_double(obj, "CONDITION", 0.0);
  sys->rate = atr_parse_double(obj, "RATE", 0.0);
  sys->strength = atr_parse_double(obj, "STRENGTH", 0.0);
  sys->efficiency = atr_parse_double(obj, "EFFICIENCY", 0.0);
  sys->maxenergy = atr_parse_double(obj, "MAXENERGY", 1.0);
  if (energy)
    sys->energy = sys->maxenergy;
  else
    sys->energy = 0.0;
  
  return 1;
}

/*********************************************************/

/* load a spawn's attributes */
int load_spawn(hspawn *spawn, int i, int reload)
{
  int n;
  char tbuf[32];
  dbref obj;
  
  if (!spawn)
  {
    return 0;
  }
  
  if (!spawn->mission)
  {
    return 0;
  }
  
  obj = spawn->mission->objnum;
  if (!IsMission(obj))
  {
    return 0;
  }
  
  if (i < 0 || i > spawn->mission->nspawns-1)
  {
    return 0;
  }
  
  sprintf(tbuf, "DRONE_%d", i);
  spawn->objnum = atr_parse_dbref(obj, tbuf);
  
  sprintf(tbuf, "RESPAWN_%d", i);
  spawn->respawn = atr_parse_integer(obj, tbuf, 0);
  spawn->last_respawn = spawn->respawn;
  
  sprintf(tbuf, "UNIVERSE_%d", i);
  spawn->uid = find_universe(atr_parse_dbref(obj, tbuf));
  
  sprintf(tbuf, "X_%d", i);
  spawn->x = atr_parse_double(obj, tbuf, 0.0);
  
  sprintf(tbuf, "Y_%d", i);
  spawn->y = atr_parse_double(obj, tbuf, 0.0);
  
  sprintf(tbuf, "Z_%d", i);
  spawn->z = atr_parse_double(obj, tbuf, 0.0);
  
  sprintf(tbuf, "COUNT_%d", i);
  spawn->ndrones = atr_parse_integer(obj, tbuf, 0);
  
  if (spawn->ndrones > 0)
  {
    if (!reload || !spawn->drones)
    {
      spawn->drones = (hship **) malloc(sizeof(hship *) * spawn->ndrones);
    }
    
    if (!spawn->drones)
    {
      spawn->ndrones = 0;
    } else if (!reload) {
      for (n = 0; n < spawn->ndrones; n++)
      {
        spawn->drones[n] = NULL;
      }
    }
  } else {
    spawn->drones = NULL;
  }
  
  return 1;
}

/* load a mission and generate all the spawns */

hmission *load_mission(dbref player, dbref obj)
{
  hmission *m;
  dbref drone;
  int n, i, ndrones;
  int id, use_id, reload;
  char tbuf[32];
  
  if (!RealGoodObject(obj) || !IsMission(obj))
  {
    notify(player, "HSPACE: That is not a valid mission.");
    return NULL;
  }
  
  ndrones = 0;
  do {
    sprintf(tbuf, "DRONE_%d", ndrones);
    drone = atr_parse_dbref(obj, tbuf);
    ndrones++;
  } while (RealGoodObject(drone));
  ndrones--;
  
  if (ndrones < 1)
  {
    notify(player, "HSPACE: That mission does not require spawns.");
    return NULL;
  }
  
  /* check to see if it is already loaded */
  use_id = hs_num_missions;
  reload = 0;
  for (id = 0; id < hs_num_missions; id++)
  {
    if (hs_missions[id].objnum == obj)
    {
      //notify(player, "HSPACE: That ship is already loaded.");
      //return NULL;
      use_id = id;
      reload = 1;
    }
  }
  
  if (hs_num_missions >= hs_options.max_missions)
  {
    notify(player, "HSPACE: Unable to load, maximum missions.");
    return NULL;
  }
  
  id = use_id;
  m = &hs_missions[id];
  
  /* minimum attributes required to call load_ship_attributes() */
  m->objnum = obj;
  
  if (!reload || !m->spawns)
  {
    m->spawns = (hspawn *) malloc(sizeof(hspawn) * ndrones);
    if (!m->spawns)
    {
      notify(player, "HSPACE: Memory error!!");
      return NULL;
    }
  }
  m->nspawns = ndrones;

  m->objnum = obj;
  
  for (i = 0; i < ndrones; i++)
  {
    m->spawns[i].mission = m;

    if (!load_spawn(&(m->spawns[i]), i, reload))
    {
      notify(player, "HSPACE: Invalid spawn definition!");
      
      for (n = 0; n < i; n++)
      {
        free(m->spawns[n].drones);
      }
      
      free(m->spawns);
      return NULL;
    }
  }
  
  atr_add(obj, "HSID", tprintf("%d", id), hs_options.space_wiz, 0);

  if (!reload)
    hs_num_missions++;
    
  return m;
}


/*********************************************************/

int load_space_object(dbref player, const char *which)
{
  huniverse *uid;
  hcelestial *cel;
  hship *ship;
  hconsole *console;
  hmission *mission;
  dbref obj;
  
  obj = match_result(player, which, TYPE_THING, MAT_EVERYTHING);
  
  if (IsUniverse(obj))
  {
    uid = load_universe(player, obj);
    if (!uid)
      return 0;
    return 1;
  }
  else if (IsCelestial(obj))
  {
    cel = load_celestial(player, obj);
    if (!cel)
      return 0;
    return 1;
  }
  else if (IsShip(obj))
  {
    ship = load_ship(player, obj);
    if (!ship)
      return 0;
    return 1;
  }
  else if (IsConsole(obj))
  {
    console = load_console(player, obj);
    if (!console)
      return 0;
    return 1;
  }
  else if (IsDrone(obj))
  {
    ship = create_drone(obj);
    if (!ship)
    {
      notify(player, "HSPACE: Failed to create the drone.");
      return 0;
    }
    
    notify_format(player, "HSPACE: Drone created at %s(#%d) %.0f %.0f %.0f.", Name(ship->uid->objnum), ship->uid->objnum, ship->x, ship->y, ship->z);
    return 1;
  }
  else if (IsMission(obj))
  {
    mission = load_mission(player, obj);
    if (!mission)
    {
      notify(player, "HSPACE: Failed to load the mission.");
      return 0;
    }
    
    notify_format(player, "HSPACE: Mission loaded: %s", Name(mission->objnum));
    return 1;
  }
  
  notify(player, "HSPACE: That object cannot be loaded!");
  return 0;
}


