
#include "hspace.h"

FUNCTION(fun_hs_cycle)
{
  safe_integer(hs_cycle_time, buff, bp);
}

FUNCTION(fun_hs_load)
{
  safe_integer(load_space_object(executor, args[0]), buff, bp);
}

FUNCTION(fun_hs_com)
{
  send_com(executor, args[0], args[1]);
}

FUNCTION(fun_hs_salvage)
{
  dbref obj, target, pad;
  hship *ship, *sptr;
  hcelestial *cel;
  
  obj = match_result(executor, args[0], TYPE_THING, MAT_EVERYTHING);
  target = match_result(executor, args[1], TYPE_THING, MAT_EVERYTHING);
  
  if (!IsShip(obj))
  {
    safe_str("#-1 INVALID SHIP", buff, bp);
    return;
  }
  
  ship = find_ship(obj);
  if (!ship)
  {
    safe_str("#-1 INVALID SHIP", buff, bp);
    return;
  }
  
  if (IsShip(target))
  {
    sptr = find_ship(target);
    if (!sptr)
    {
      safe_str("#-2 INVALID DESTINATION", buff, bp);
      return;
    }
    
    pad = atr_parse_dbref(sptr->objnum, "BAY");
    if (!IsRoom(pad))
    {
      pad = Location(sptr->objnum);
    }
    
    enter_ship(ship, sptr, pad);
  }
  else if (IsCelestial(target))
  {
    cel = find_celestial(target);
    if (!cel)
    {
      safe_str("#-2 INVALID DESTINATION", buff, bp);
      return;
    }
    
    pad = atr_parse_dbref(cel->objnum, "DROPPADS");
    if (!IsRoom(pad))
    {
      safe_str("#-2 INVALID DESTINATION", buff, bp);
      return;
    }
    
    enter_celestial(ship, cel, pad);
  }
  else
  {
    safe_str("#-2 INVALID DESTINATION", buff, bp);
    return;
  }
  
  safe_integer(1, buff, bp);
  
  FlagOff(ship->type, HS_DISABLED);
  FlagOff(ship->type, HS_DESTROYED);
  ship->hull.energy = 1.0;
  
  return;
}

/* get or activate cooldowns */
FUNCTION(fun_hs_cooldown)
{
  dbref obj;
  char *r, *s;
  hship *ship;
  hbuff *b;
  hcontact *q;
  
  s = args[0];
  r = split_token(&s, '/');

  obj = match_result(executor, r, TYPE_THING, MAT_EVERYTHING);
  if (!RealGoodObject(obj))
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }

  ship = find_ship(obj);
  if (!ship)
  {
    if (s)
    {
      q = find_contact(ship, parse_integer(s));
      if (!q || !q->contact)
      {
        safe_str("#-1 INVALID OBJECT", buff, bp);
        return;
      }

      if (!HasFlag(q->type, HS_ANY_SHIP))
      {
        safe_str("#-! INVALID OBJECT", buff, bp);
        return;
      }
      
      ship = ContactShip(q);
    }
  }

  if (!ship)
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }

  
  b = find_buff(ship, find_effect(hs_cooldowns, args[1]));
  if (!b)
  {
    safe_integer(0, buff, bp);
    return;
  }
  
  safe_integer(b->duration, buff, bp);
  return;
}

FUNCTION(fun_hs_get)
{
  dbref obj;
  char *r, *s;
  hship *ship;
  hcelestial *cel;
  hcontact *q;
  int use_ship = 1;
  
  s = args[0];
  r = split_token(&s, '/');

  obj = match_result(executor, r, TYPE_THING, MAT_EVERYTHING);
  if (!RealGoodObject(obj))
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }

  /* try to match a ship first */
  ship = find_ship(obj);
  if (!ship)
  {
    cel = find_celestial(obj);
    if (!cel)
    {
      safe_str("#-1 INVALID OBJECT", buff, bp);
      return;
    }
    
    /* using celestial */
    use_ship = 0;
  } else {
    /* we were given a ship, now check if we got a contact */
    use_ship = 1;

    if (s)
    {
      q = find_contact(ship, parse_integer(s));
      if (q && q->contact)
      {
        if (HasFlag(q->type, HS_ANY_SHIP))
        {
          ship = ContactShip(q);
          use_ship = 1;
        } else {
          cel = ContactCelestial(q);
          use_ship = 0;
        }
      } else {
        safe_str("#-1 INVALID OBJECT", buff, bp);
        return;
      }
    }
  }
  
  if (use_ship)
  {
    get_ship_attribute(ship, args[1], buff, bp);
  } else {
    get_celestial_attribute(cel, args[1], buff, bp);
  }
  
  return;
}

FUNCTION(fun_hs_set)
{
  dbref obj;
  int len;
  char *r, *s;
  hship *ship;
  hcelestial *cel;
  hcontact *q;
  int use_ship = 1;
  
  s = args[0];
  r = split_token(&s, '/');

  obj = match_result(executor, r, TYPE_THING, MAT_EVERYTHING);
  if (!RealGoodObject(obj))
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }

  /* try to match a ship first */
  ship = find_ship(obj);
  if (!ship)
  {
    cel = find_celestial(obj);
    if (!cel)
    {
      safe_str("#-1 INVALID OBJECT", buff, bp);
      return;
    }
    
    /* using celestial */
    use_ship = 0;
  } else {
    /* we were given a ship, now check if we got a contact */
    use_ship = 1;

    if (s)
    {
      q = find_contact(ship, parse_integer(s));
      if (q && q->contact)
      {
        if (HasFlag(q->type, HS_ANY_SHIP))
        {
          ship = ContactShip(q);
          use_ship = 1;
        } else {
          cel = ContactCelestial(q);
          use_ship = 0;
        }
      } else {
        safe_str("#-1 INVALID OBJECT", buff, bp);
        return;
      }
    }
  }
  
  if (use_ship)
  {
    set_ship_attribute(ship, args[1], args[2]);
  } else {
    set_celestial_attribute(cel, args[1], args[2]);
  }
}


FUNCTION(fun_hs_srep)
{
  hship *ship;
  dbref obj;
  hcontact *q;
  
  const char fsep[] = ":";
  const char osep[] = "|";
  
  if (!args[0])
  {
    obj = executor;
  } else {
    obj = match_result(executor, args[0], TYPE_THING, MAT_EVERYTHING);
  }
  
  if (!RealGoodObject(obj))
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }
  
  ship = find_ship(obj);
  if (!ship)
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }
  
  for (q = get_head_contact(ship); q; q = q->next)
  {
    if (!q->contact)
      continue;
    
    safe_integer(q->cnum, buff, bp);
    safe_str(fsep, buff, bp);
    safe_str(ContactName(q), buff, bp);
    safe_str(fsep, buff, bp);
    safe_integer(rint(ContactDistance(ship, q)), buff, bp);
    safe_str(fsep, buff, bp);
    safe_integer(contact_xyang(ship, q), buff, bp);
    safe_str(fsep, buff, bp);
    safe_integer(contact_zang(ship, q), buff, bp);

    if (q->next)
      safe_str(osep, buff, bp);
  }
}

/**************************************************/

void get_ship_attribute(hship *ship, char *which, char *buff, char **bp)
{
  int len, i;
  
  if (!ship)
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }
  
  if (!which)
  {
    safe_str("#-1 INVALID ATTRIBUTE", buff, bp);
    return;
  }
  
  len = strlen(which);
  if (len < 1)
  {
    safe_str("#-1 INVALID ATTRIBUTE", buff, bp);
    return;
  }
  
  if (!strncasecmp("OBJNUM", which, len))
  {
    safe_dbref(ship->objnum, buff, bp);
  }
  else if (!strncasecmp("NAME", which, len))
  {
    safe_str(ship_name(ship), buff, bp);
  }
  else if (!strncasecmp("TYPE", which, len))
  {
    if (HasFlag(ship->type, HS_DRONE))
      safe_str("DRONE", buff, bp);
    else
      safe_str("SHIP", buff, bp);
  }
  else if (!strncasecmp("UNIVERSE", which, len))
  {
    if (ship->uid)
      safe_dbref(ship->uid->objnum, buff, bp);
    else
      safe_dbref(NOTHING, buff, bp);
  }
  else if (!strncasecmp("X", which, len))
  {
    safe_integer(ship->x, buff, bp);
  }
  else if (!strncasecmp("Y", which, len))
  {
    safe_integer(ship->y, buff, bp);
  }
  else if (!strncasecmp("Z", which, len))
  {
    safe_integer(ship->z, buff, bp);
  }
  else if (!strncasecmp("SPEED", which, len))
  {
    safe_number(ship->speed, buff, bp);
  }
  else if (!strncasecmp("XYHEAD", which, len))
  {
    safe_integer(rint(ship->xyhead), buff, bp);
  }
  else if (!strncasecmp("ZHEAD", which, len))
  {
    safe_integer(rint(ship->zhead), buff, bp);
  }
  else if (!strncasecmp("LOCK", which, len))
  {
    if (ship->lock)
      safe_integer(ship->lock->cnum, buff, bp);
    else
      safe_str("#-1", buff, bp);
  }
  else if (!strncasecmp("WAYPOINT", which, len))
  {
    if (ship->wp_contact && ship->wp_contact->contact)
    {
      if (HasFlag(ship->wp_contact->type, HS_ANY_CELESTIAL))
      {
        safe_format(buff, bp, "#%d %.0f %.0f %.0f", ContactObj(ship->wp_contact), ContactCelestial(ship->wp_contact)->x,
              ContactCelestial(ship->wp_contact)->y, ContactCelestial(ship->wp_contact)->z);
      }
      else if (HasFlag(ship->wp_contact->type, HS_ANY_SHIP))
      {
        safe_format(buff, bp, "#%d %.0f %.0f %.0f", ContactObj(ship->wp_contact), ContactShip(ship->wp_contact)->x,
              ContactShip(ship->wp_contact)->y, ContactShip(ship->wp_contact)->z);
      }
    }
  }
  else if (!strncasecmp("CONSOLES", which, len))
  {
    for (i = 0; i < ship->nconsoles; i++)
    {
      if (i > 0)
        safe_str(" ", buff, bp);
      
      safe_dbref(ship->console[i].objnum, buff, bp);
    }
  }
  else if (!strncasecmp("LANDED", which, len))
  {
    if (ship->landed)
      safe_dbref(ship->landed->objnum, buff, bp);
    else
      safe_dbref(NOTHING, buff, bp);
  }
  else if (!strncasecmp("DOCKED", which, len))
  {
    if (ship->docked)
      safe_dbref(ship->docked->objnum, buff, bp);
    else
      safe_dbref(NOTHING, buff, bp);
  }
  else if (!strncasecmp("LINKED", which, len))
  {
    if (ship->linked)
      safe_dbref(ship->linked->objnum, buff, bp);
    else
      safe_dbref(NOTHING, buff, bp);
  }
  else if (!strncasecmp("ZHEAD", which, len))
  {
    safe_integer(rint(ship->zhead), buff, bp);
  }
  else if (!strncasecmp("NAVMODE", which, len))
  {
    safe_str(STR(hs_nav_modes, HasFlag(ship->type, HS_ANY_MODE)), buff, bp);
  }
  else if (!strncasecmp("AFTERBURNING", which, len))
  {
    if (HasFlag(ship->type, HS_AFTERBURNING))
      safe_integer(1, buff, bp);
    else
      safe_integer(0, buff, bp);
  }
  else if (!strncasecmp("SHIELD", which, len))
  {
    safe_integer(rint(ship->shield.energy / get_stat(ship, SYS_MAX_CAPACITY) * 100.0), buff, bp);
  }
  else if (!strncasecmp("HULL", which, len))
  {
    safe_integer(rint(ship->hull.energy / get_stat(ship, SYS_MAX_ARMOR) * 100.0), buff, bp);
  }
  else if (!strncasecmp("ENGINE", which, len))
  {
    safe_integer(rint(ship->engine.energy / get_stat(ship, SYS_MAX_HEAT) * 100.0), buff, bp);
  }
  else if (!strncasecmp("REACTOR", which, len))
  {
    safe_integer(rint(ship->reactor.energy / get_stat(ship, SYS_MAX_ENERGY) * 100.0), buff, bp);
  }
  else if (!strncasecmp("COMPUTER", which, len))
  {
    safe_integer(rint(ship->computer.energy / get_stat(ship, SYS_MAX_MEMORY) * 100.0), buff, bp);
  }
  else if (!strncasecmp("SENSOR", which, len))
  {
    safe_integer(rint(ship->sensor.energy / get_stat(ship, SYS_MAX_FOCUS) * 100.0), buff, bp);
  }
  else
  {
    safe_str("#-1 INVALID ATTRIBUTE", buff, bp);
  }

  return;
}

void set_ship_attribute(hship *ship, char *which, char *value)
{

}

void get_celestial_attribute(hcelestial *cel, char *which, char *buff, char **bp)
{
  int len;
  
  if (!cel)
  {
    safe_str("#-1 INVALID OBJECT", buff, bp);
    return;
  }
  
  if (!which)
  {
    safe_str("#-1 INVALID ATTRIBUTE", buff, bp);
    return;
  }
  
  len = strlen(which);
  if (len < 1)
  {
    safe_str("#-1 INVALID ATTRIBUTE", buff, bp);
    return;
  }
  
  if (!strncasecmp("OBJNUM", which, len))
  {
    safe_dbref(cel->objnum, buff, bp);
  }
  else if (!strncasecmp("NAME", which, len))
  {
    safe_str(celestial_name(cel), buff, bp);
  }
  else if (!strncasecmp("TYPE", which, len))
  {
    safe_str(STR(hs_object_types, HasFlag(cel->type, HS_OBJECT_FLAGS)), buff, bp);
  }
  else if (!strncasecmp("UNIVERSE", which, len))
  {
    if (cel->uid)
      safe_dbref(cel->uid->objnum, buff, bp);
    else
      safe_dbref(NOTHING, buff, bp);
  }
  else if (!strncasecmp("X", which, len))
  {
    safe_integer(cel->x, buff, bp);
  }
  else if (!strncasecmp("Y", which, len))
  {
    safe_integer(cel->y, buff, bp);
  }
  else if (!strncasecmp("Z", which, len))
  {
    safe_integer(cel->z, buff, bp);
  }
  else if (!strncasecmp("MASS", which, len))
  {
    safe_number(cel->mass, buff, bp);
  }
  else if (!strncasecmp("RADIUS", which, len))
  {
    safe_number(cel->radius, buff, bp);
  }
  else if (!strncasecmp("CONTENTS", which, len))
  {
    if (cel->contents)
      safe_str(cel->contents, buff, bp);
  } else {
    safe_str("#-1 INVALID ATTRIBUTE", buff, bp);
  }

  return;
}

void set_celestial_attribute(hcelestial *cel, char *which, char *value)
{

}

