
#include "hspace.h"


/* stringmaps for matching some commands and flags */
hstringmap hs_nav_modes[] =
{
  { "Goto Nav Point", (void*) HS_MODE_GOTO },
  { "Intercept Target", (void*) HS_MODE_INTERCEPT },
  { "Evade Target", (void*) HS_MODE_EVADE },
  { "Form on Nav Point", (void*) HS_MODE_FORMATION },
  { "All Stop", (void*) HS_MODE_ALLSTOP },
  { "Caution", (void*) HS_MODE_SLOW },
  { NULL, NULL },
};

hstringmap hs_prompt_flags[] =
{
  { "COMBAT", (void*) HS_PROMPT_COMBAT },
  { "SPACE", (void*) HS_PROMPT_SPACE },
  { "ALWAYS", (void*) HS_PROMPT_ALWAYS },
  { NULL, NULL }, 
};


/*****************************************************/
/* console commands */

/* command interface to use a slot */
void use_slot(dbref console, char *arg_left, hslot stat, int showmsg)
{
  hship *ship;
  hconsole *con;
  hweapon *gun;
  hcontact *q;
  double var;
  
  gun = NULL;
  
  ship = find_ship(console);
  if (!ship)
  {
    hs_std_notice(console, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  /* find out if we're firing from a console or not */
  /* get the appropriate gun and target lock */
  con = find_console(console);
  if (!con)
  {
    if (HasFlag(stat, HS_PRIMARY))
      gun = &(ship->primary);
    else if (HasFlag(stat, HS_SECONDARY))
      gun = &(ship->secondary);

    /* use ship weapon */
    if (!HasFlag(gun->type, HS_CAPACITOR | HS_BOOSTER) && (!ship->lock || !ship->lock->contact))
    {
        hs_std_notice(ship->objnum, "No target lock.");
        return;
    }
    
    q = ship->lock;
    
  } else {
    if (HasFlag(stat, HS_PRIMARY))
      gun = &(con->primary);
    else if (HasFlag(stat, HS_SECONDARY))
      gun = &(con->secondary);

    /* use console weapon */
    if (!HasFlag(gun->type, HS_CAPACITOR | HS_BOOSTER) && (!con->lock || !con->lock->contact))
    {
      hs_std_notice(con->objnum, "No target lock.");
      return;
    }
    
    q = con->lock;

  }
  
  if (!gun || !RealGoodObject(gun->objnum))
  {
    hs_std_notice(console, tprintf("Weapon not loaded. Use %sEQ%s and %sEQ <weapon>%s.", ANSI_CYAN, ANSI_NORMAL, ANSI_CYAN, ANSI_NORMAL));
    return;
  }
  
  var = parse_number(arg_left);
  fire_weapon(ship, con, gun, q, var, showmsg);

  return;
}


/* command interface to use both slots */
void use_both_slots(dbref console, char *arg_left)
{
  hship *ship;
  hconsole *con;
  hweapon *pri, *sec;
  hcontact *q;
  double var;
  int one, two, no_weapon;
  
  pri = NULL;
  sec = NULL;
  
  ship = find_ship(console);
  if (!ship)
  {
    hs_std_notice(console, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  /* find out if we're firing from a console or not */
  /* get the appropriate gun and target lock */
  con = find_console(console);
  if (!con)
  {
    /* use ship weapon */
    pri = &(ship->primary);
    sec = &(ship->secondary);

    if (!HasFlag(pri->type | sec->type, HS_CAPACITOR | HS_BOOSTER) && (!ship->lock || !ship->lock->contact))
    {
        hs_std_notice(ship->objnum, "No target lock.");
        return;
    }
    
    q = ship->lock;
  } else {
    /* use console weapon */
    pri = &(con->primary);
    sec = &(con->secondary);

    if (!HasFlag(pri->type | sec->type, HS_CAPACITOR | HS_BOOSTER) && (!con->lock || !con->lock->contact))
    {
      hs_std_notice(con->objnum, "No target lock.");
      return;
    }
    
    q = con->lock;
  }
  
  no_weapon = 0;
  if (!pri || !RealGoodObject(pri->objnum))
  {
    no_weapon = 1;
    pri = NULL;
  }
  
  if (!sec || !RealGoodObject(sec->objnum))
  {
    no_weapon = 1;
    sec = NULL;
  }
  
  if (no_weapon)
  {
    hs_std_notice(console, tprintf("Weapons not loaded. Use %sEQ%s and %sEQ <weapon>%s.", ANSI_CYAN, ANSI_NORMAL, ANSI_CYAN, ANSI_NORMAL));
    return;
  }
  
  var = parse_number(arg_left);
  one = fire_weapon(ship, con, pri, q, var, 0);
  two = fire_weapon(ship, con, sec, q, var, 0);

  if (one == 1 || two == 1)
  {
    /* as long as one of them is successful, we're fine */
    return;
  }
  else if (one == -1 || two == -1)
  {
    hs_std_notice(console, "Weapons still reloading/recharging.");
    return;
  }
  else if (one == -2 || two == -2)
  {
    hs_std_notice(console, "Target is out of range.");
    return;
  }
  else if (!one || !two)
  {
    hs_std_notice(console, tprintf("Weapons not loaded. Use %sEQ%s and %sEQ <weapon>%s.", ANSI_CYAN, ANSI_NORMAL, ANSI_CYAN, ANSI_NORMAL));
    return;
  }

  return;
}


/* travel through a wormhole */
void use_wormhole(dbref obj, char *arg_left)
{
  hship *ship;
  double r, rmin;
  hcontact *q, *qmin;
  hcelestial *cel;
  hbuff *b;
  
  if (!IsShip(obj))
  {
    hs_std_notice(obj, "Only the pilot may travel through jump points.");
    return;
  }
  
  ship = find_ship_by_nav(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (ship->linked)
  {
    hs_std_notice(obj, "Unable to jump while linked.");
    return;
  }
  
  b = find_buff(ship, &COOLDOWN_jump_drive);
  if (b)
  {
    hs_std_notice(obj, tprintf("Jump drive is still recharging (%s%.0f%%%s).",
          ANSI_HILITE, (1.0 - ((double)(b->duration) / (double)(COOLDOWN_jump_drive.duration))) * 100.0, ANSI_NORMAL));
    return;
  }
  
  rmin = 1.0e100;
  qmin = NULL;
  
  /* no wormhole specified, try to find the closest one */
  if (!arg_left || !*arg_left)
  {
    for (q = get_head_contact(ship); q; q = q->next)
    {
      if (!HasFlag(q->type, HS_WORMHOLE) || !q->contact)
        continue;
      
      r = ContactDistance(ship, q);
      if (r < rmin)
      {
        rmin = r;
        qmin = q;
      }
    }
    
    if (!qmin || rmin > hs_options.max_gate_dist)
    {
      hs_std_notice(obj, tprintf("There are no jump points in range (%d Mm).", hs_options.max_gate_dist));
      return;
    }
    
    q = qmin;
    
  } else {
    /* wormhole specified, find it */
    q = find_contact(ship, parse_integer(arg_left));
    
    if (!q || !HasFlag(q->type, HS_WORMHOLE))
    {
      hs_std_notice(obj, "That is not a valid jump point.");
      return;
    }
    
    rmin = ContactDistance(ship, q);
    if (rmin > hs_options.max_gate_dist)
    {
      hs_std_notice(obj, tprintf("That jump point is not in range (%d Mm).", hs_options.max_gate_dist));
      return;
    }
  }

  /* move through the wormhole to the otherside */
  cel = find_celestial(atr_parse_dbref(ContactCelestial(q)->objnum, "OTHERSIDE"));
  if (!cel || !cel->uid || !RealGoodObject(cel->uid->objnum))
  {
    hs_std_notice(obj, tprintf("That jump point does not have a stable end point!"));
    return;
  }
  
  notify_console(obj, tprintf("%s%s*%s %s", ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, hs_options.wormhole_travel));
  notify_contacts(ship, NULL, tprintf("%s%s%s has jumped through the %s%s%s%s jump point.",
        ANSI_HILITE, ship_name(ship), ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, Name(cel->uid->objnum), ANSI_NORMAL));
  move_ship(ship, cel->uid, cel->x, cel->y, cel->z);
  add_buff(ship, &COOLDOWN_jump_drive);
  
  return;
}


/* open or close docking bay doors */
void set_baydoors(dbref obj, int open)
{
  hship *ship;
  
  if (!IsConsole(obj) && !IsShip(obj))
    return;
  
  ship = find_ship(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (open)
  {
    if (HasFlag(ship->type, HS_BAY_OPEN))
    {
      hs_std_notice(obj, "Bay doors already open.");
      return;
    }
    
    FlagOn(ship->type, HS_BAY_OPEN);
    hs_std_notice(obj, "Opening bay doors.");
    notify_contacts(ship, NULL, "Opening bay doors.");
  }
  else
  {
    if (!HasFlag(ship->type, HS_BAY_OPEN))
    {
      hs_std_notice(obj, "Bay doors already closed.");
      return;
    }
    
    FlagOff(ship->type, HS_BAY_OPEN);
    hs_std_notice(obj, "Closing bay doors.");
    notify_contacts(ship, NULL, "Closing bay doors.");
  }
  
  return;
}


/* set the hatch linking status */
void set_boardinglink(dbref obj, int open)
{
  hship *ship;
  
  if (!IsConsole(obj) && !IsShip(obj))
    return;
  
  ship = find_ship(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (ship->linked)
  {
    hs_std_notice(obj, "You may not do that while linked.");
    return;
  }
  
  if (open)
  {
    if (HasFlag(ship->type, HS_LINK_EXTENDED))
    {
      hs_std_notice(obj, "Boarding link already extended.");
      return;
    }
    
    FlagOn(ship->type, HS_LINK_EXTENDED);
    hs_std_notice(obj, "Extending boarding link.");
    notify_contacts(ship, NULL, "Extending boarding link.");
  }
  else
  {
    if (!HasFlag(ship->type, HS_LINK_EXTENDED))
    {
      hs_std_notice(obj, "Boarding link already retracted.");
      return;
    }
    
    FlagOff(ship->type, HS_LINK_EXTENDED);
    hs_std_notice(obj, "Retracting boarding link.");
    notify_contacts(ship, NULL, "Retracting boarding link.");
  }
  
  return;
}


/* set a player's pvp status */
void set_pvp(dbref obj, int on)
{
  hship *ship;
  
  if (!IsConsole(obj) && !IsShip(obj))
    return;
  
  ship = find_ship(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (on)
  {
    if (HasFlag(ship->type, HS_PVP))
    {
      hs_std_notice(obj, "PVP already allowed.");
      return;
    }
    
    FlagOn(ship->type, HS_PVP);
    hs_std_notice(obj, "Allowing Player Vs Player combat.");
    notify_contacts(ship, NULL, "PVP enabled.");
  }
  else
  {
    if (!HasFlag(ship->type, HS_PVP))
    {
      hs_std_notice(obj, "PVP already forbidden.");
      return;
    }
    
    FlagOff(ship->type, HS_PVP);
    hs_std_notice(obj, "No longer allowing Player Vs Player combat.");
    notify_contacts(ship, NULL, "PVP disabled.");
  }
  
  return;
}


/* set or clear the ship's waypoint */
void set_waypoint(dbref obj, char *arg_left)
{
  hship *ship;
  char *r, *s;
  int fx, fy, fz;
  hcontact *q;
  
  if (!IsConsole(obj) && !IsShip(obj))
    return;
  
  ship = find_ship(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  /* clear waypoint, just blank the uid and let the sensors do their own update */
  if (!arg_left || !*arg_left)
  {
    if (ship->waypoint.uid)
      hs_std_notice(obj, "Nav point cleared.");

    ship->waypoint.uid = NULL;
    ship->wp_contact = NULL;
    return;
  }
  
  s = arg_left;
  r = split_token(&s, ' ');
  fx = parse_integer(r);
  if (s)
  {
    /* there are more numbers to read, read in a x/y/z coordinate */
    r = split_token(&s, ' ');
    fy = parse_integer(r);
    
    if (!s)
    {
      hs_std_notice(obj, "Invalid coordinate specified.");
      return;
    }

    r = split_token(&s, ' ');
    fz = parse_integer(r);
    
    ship->waypoint.uid = ship->uid;
    ship->waypoint.x = fx;
    ship->waypoint.y = fy;
    ship->waypoint.z = fz;
    
    if (ship->wp_contact && ship->wp_contact->contact)
    {
      if (ship->wp_contact->contact != &(ship->waypoint))
      {
        /* we are setting a x/y/z waypoint but our current waypoint is another contact */
        /* we should clear wp_contact so that sensors can pickup the waypoint */
        ship->wp_contact = NULL;
      }
    }
  } else {
    /* there are no more numbers, match a contact */
    q = find_contact(ship, fx);
    if (!q)
    {
      hs_std_notice(obj, "Invalid contact specified.");
      return;
    }
    
    ship->wp_contact = q;
  }
  
  hs_std_notice(obj, "Nav point changed.");
  
  return;
}


/* set desired heading for ship or console */
void set_heading(dbref obj, char *arg_left, char *arg_right)
{
  hship *ship;
  hconsole *con;
  int xyhead, zhead;
  
  if (IsShip(obj))
  {
    ship = find_ship_by_nav(obj);
    if (!ship)
      return;
    
    xyhead = parse_integer(arg_left);
    zhead = parse_integer(arg_right);
    
    if (xyhead > 359 || xyhead < 0)
    {
      hs_std_notice(obj, "Ship's azimuth must be between 0 and 359.");
      return;
    }
    
    if (zhead > 90 || zhead < -90)
    {
      hs_std_notice(obj, "Ship's elevation must be between -90 and 90.");
      return;
    }
    
    ship->desired_xyhead = (double) xyhead;
    ship->desired_zhead = (double) zhead;
    
    hs_std_notice(obj, tprintf("Ship's desired heading set to %s%d%sm%s%d%s.", ANSI_HILITE, xyhead, ANSI_NORMAL,
                                ANSI_HILITE, zhead, ANSI_NORMAL));
  }
  else if (IsConsole(obj))
  {
    con = find_console(obj);
    if (!con)
      return;
    
    xyhead = parse_integer(arg_left);
    zhead = parse_integer(arg_right);
    
    if (xyhead > 359 || xyhead < 0)
    {
      hs_std_notice(obj, "Console's azimuth must be between 0 and 359.");
      return;
    }
    
    if (zhead > 90 || zhead < -90)
    {
      hs_std_notice(obj, "Console's elevation must be between -90 and 90.");
      return;
    }
    
    con->desired_xyhead = (double) xyhead;
    con->desired_zhead = (double) zhead;

    hs_std_notice(obj, tprintf("Console's desired heading set to %s%d%sm%s%d%s.", ANSI_HILITE, xyhead, ANSI_NORMAL,
                                ANSI_HILITE, zhead, ANSI_NORMAL));
  }
  
  return;
}


/* set desired speed for ship */
void change_speed(hship *ship, double speed)
{
  double maximum;
  
  maximum = get_stat(ship, SYS_VELOCITY);
  
  if (speed > maximum)
    speed = maximum;
  else if (speed < -0.5*maximum)
    speed = -0.5*maximum;
  
  ship->desired_speed = speed;
  
  hs_std_notice(ship->objnum, tprintf("Ship's desired speed set to %s%.0f%s km/s.", ANSI_HILITE, speed, ANSI_NORMAL));
  
  if (ship->desired_speed > 0 && ship->speed < 0)
    notify_srooms(ship, NOTHING, hs_options.engine_forward);
  else if (ship->desired_speed < 0 && ship->speed > 0)
    notify_srooms(ship, NOTHING, hs_options.engine_reverse);
  else if (ship->desired_speed > ship->speed)
    notify_srooms(ship, NOTHING, hs_options.speed_increase);
  else
    notify_srooms(ship, NOTHING, hs_options.speed_decrease);
  
  return;
}

/* @console/speed interface */
void set_speed(dbref obj, char *arg_left)
{
  hship *ship;
  double speed;
  double maximum;
  char *s;
  
  if (!IsShip(obj))
  {
    hs_std_notice(obj, "Only the pilot may change the ship's speed.");
    return;
  }
  
  ship = find_ship_by_nav(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (!arg_left || !*arg_left)
  {
    hs_std_notice(obj, "You must specify a number.");
    return;
  }
  
  /* maximum speed */
  maximum = get_stat(ship, SYS_VELOCITY);
  if (!strncasecmp("MAXIMUM", arg_left, strlen(arg_left)))
  {
    speed = maximum;
  }
  else if (!strncasecmp("MINIMUM", arg_left, strlen(arg_left)))
  {
    speed = -0.5 * maximum;
  }
  else
  {
    speed = rint(strtod(arg_left, &s));
    if (!s)
    {
      hs_std_notice(obj, "You must specify a number.");
      return;
    }
  }

  if (speed > maximum)
  {
    hs_std_notice(obj, tprintf("Ship's maximum speed is %.0f km/s.", maximum));
    return;
  }
  else if (speed < (-0.5 * maximum))
  {
    hs_std_notice(obj, tprintf("Ship's minimum speed is %.0f km/s.", (-0.5*maximum)));
    return;
  }

  change_speed(ship, speed);
}

/* engage/disengage afterburners */
void change_afterburner(hship *ship, int engage)
{
  if (!ship || !HasFlag(ship->engine.type, HS_AFTERBURNER))
    return;
  
  if (engage)
  {
    if(!HasFlag(ship->type, HS_AFTERBURNING))
    {
      FlagOn(ship->type, HS_AFTERBURNING);
      hs_std_notice(ship->objnum, "Engaging afterburners.");
      notify_srooms(ship, NOTHING, hs_options.engage_afterburner);
      notify_contacts(ship, NULL, hs_options.notify_engage);
    } else {
      hs_std_notice(ship->objnum, "Afterburners are already in use.");
    }
  }
  else
  {
    if (HasFlag(ship->type, HS_AFTERBURNING))
    {
      FlagOff(ship->type, HS_AFTERBURNING);
      hs_std_notice(ship->objnum, "Disengaging afterburners.");
      notify_srooms(ship, NOTHING, hs_options.disengage_afterburner);
      notify_contacts(ship, NULL, hs_options.notify_disengage);
    } else {
      hs_std_notice(ship->objnum, "Afterburners are not currently in use.");
    }
  }

  return;
}

/* @console/burn interface */
void set_afterburner(dbref obj, int engage)
{
  hship *ship;
  double speed;
  double maximum;
  char *s;
  
  if (!IsShip(obj))
  {
    hs_std_notice(obj, "Only the pilot may engage afterburners.");
    return;
  }
  
  ship = find_ship_by_nav(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (!HasFlag(ship->engine.type, HS_AFTERBURNER))
  {
    hs_std_notice(obj, "This ship is not equipped with afterburners.");
    return;
  }
  
  /* maximum speed */
  change_afterburner(ship, engage);
}

/* boarding link */
void unlink_ship(dbref obj)
{
  hship *ship;
  hcontact *q, *qptr;

  if (!IsShip(obj))
  {
    hs_std_notice(obj, "Only the pilot may link the ship.");
    return;
  }
  
  ship = find_ship_by_nav(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (!ship->linked)
  {
    hs_std_notice(obj, "You are not currently linked with another ship.");
    return;
  }
  
  q = find_ship_contact(ship, ship->linked);
  hs_std_sensor(ship, q, tprintf("Unlinking from %s%s%s.", ANSI_HILITE, ship_name(ship->linked), ANSI_NORMAL));

  qptr = find_ship_contact(ship->linked, ship);
  hs_std_sensor(ship->linked, qptr, tprintf("%s%s%s has unlinked.", ANSI_HILITE, ship_name(ship), ANSI_NORMAL));
  
  notify_contacts(ship, ContactShip(q), tprintf("%s%s%s has unlinked from %s%s%s.",
        ANSI_HILITE, ship_name(ship), ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));

  ship->linked->linked = NULL;
  ship->linked = NULL;
  
  return;
}

/* boarding link */
void link_ship(dbref obj, char *arg_left)
{
  hship *ship;
  dbref bay;
  hcontact *q, *qptr;

  if (!IsShip(obj))
  {
    hs_std_notice(obj, "Only the pilot may link the ship.");
    return;
  }
  
  ship = find_ship_by_nav(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (!HasFlag(ship->type, HS_LINK_EXTENDED))
  {
    hs_std_notice(obj, "Our boarding link is not extended.");
    return;
  }
  
  if (ship->linked)
  {
    hs_std_notice(obj, "This ship is already linked to another.");
    return;
  }
  
  q = find_contact(ship, parse_integer(arg_left));
  if (!q || !q->contact || !HasFlag(q->type, HS_SHIP))
  {
    hs_std_notice(obj, "Invalid contact.");
    return;
  }
  
  if (ContactDistance(ship, q) > hs_options.max_board_dist)
  {
    hs_std_notice(obj, "Contact is not in range.");
    return;
  }
  
  if (!HasFlag(ContactShip(q)->type, HS_LINK_EXTENDED))
  {
    hs_std_notice(obj, "Contact boarding link is not extended.");
    return;
  }
  
  if (ContactShip(q)->linked)
  {
    hs_std_notice(obj, "That ship is already linked to another.");
    return;
  }
  
  hs_std_sensor(ship, q, tprintf("Linking with %s%s%s.", ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));
    
  qptr = find_ship_contact(ContactShip(q), ship);
  hs_std_sensor(ContactShip(q), qptr, tprintf("%s%s%s has linked with us.", ANSI_HILITE, ship_name(ship), ANSI_NORMAL));
    
  notify_contacts(ship, ContactShip(q), tprintf("%s%s%s has linked with %s%s%s.",
        ANSI_HILITE, ship_name(ship), ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));

  ship->linked = ContactShip(q);
  ContactShip(q)->linked = ship;
  
  return;
}


/* landing */
void land_ship(dbref obj, char *arg_left, char *arg_right)
{
  ATTR *a;
  hship *ship;
  dbref pad;
  hcontact *q;
  char *s, *r;
  char buff[512];
  int i, n;
  char *cbuf;

  if (!IsShip(obj))
  {
    hs_std_notice(obj, "Only the pilot may land the ship.");
    return;
  }
  
  ship = find_ship_by_nav(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (HasFlag(ship->type, HS_CAPITAL))
  {
    hs_std_notice(obj, "This ship is unable to land.");
    return;
  }
  
  if (ship->linked)
  {
    hs_std_notice(obj, "Unable to land while linked.");
    return;
  }
  
  q = find_contact(ship, parse_integer(arg_left));
  if (!q || !q->contact || !HasFlag(q->type, HS_CELESTIAL))
  {
    hs_std_notice(obj, "Invalid celestial contact.");
    return;
  }
  
  if (ContactDistance(ship, q) > hs_options.max_land_dist)
  {
    hs_std_notice(obj, "Contact is not in range.");
    return;
  }
  
  a = atr_get(ContactObj(q), "DROPPADS");
  if (!a)
  {
    hs_std_notice(obj, "That contact does not have any landing locations.");
    return;
  }
  strncpy(buff, atr_value(a), 511);
  s = buff;
  r = split_token(&s, ' ');

  i = parse_integer(arg_right);
  n = 0;
  do {
    if (i == n)
    {
      pad = parse_dbref(r);
      if (RealGoodObject(pad))
      {
        cbuf = contact_colorstring(ship, q);
        hs_std_sensor(ship, q, tprintf("Landing on %s%s%s.", cbuf, CelestialName(q->contact), ANSI_NORMAL));
        notify_contacts(ship, NULL, tprintf("%s%s%s has landed on %s%s%s.",
             ANSI_HILITE, ship_name(ship), ANSI_NORMAL, cbuf, CelestialName(q->contact), ANSI_NORMAL));
        enter_celestial(ship, ContactCelestial(q), pad);
        return;
      }
    }
    n++;
    
    if (!s)
    {
      hs_std_notice(obj, "Invalid landing pad.");
      return;
    }
    
    r = split_token(&s, ' ');
  } while (s);
  
  return;
}

/* docking */
void dock_ship(dbref obj, char *arg_left)
{
  hship *ship;
  dbref bay;
  hcontact *q, *qptr;
  int shipsize, targetsize, capacity, current;

  if (!IsShip(obj))
  {
    hs_std_notice(obj, "Only the pilot may dock the ship.");
    return;
  }
  
  ship = find_ship_by_nav(obj);
  if (!ship)
  {
    hs_std_notice(obj, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (ship->linked)
  {
    hs_std_notice(obj, "Unable to dock while linked.");
    return;
  }
  
  q = find_contact(ship, parse_integer(arg_left));
  if (!q || !q->contact || !HasFlag(q->type, HS_SHIP))
  {
    hs_std_notice(obj, "Invalid contact.");
    return;
  }
  
  if (ContactDistance(ship, q) > hs_options.max_dock_dist)
  {
    hs_std_notice(obj, "Contact is not in range.");
    return;
  }
  
  if (!HasFlag(ContactShip(q)->type, HS_BAY_OPEN))
  {
    hs_std_notice(obj, "Contact bay doors are closed.");
    return;
  }
  
  bay = atr_parse_dbref(ContactObj(q), "BAY");
  if (!IsRoom(bay))
  {
    hs_std_notice(obj, "That ship does not have a docking bay.");
    return;
  }
  
  capacity = atr_parse_integer(ContactObj(q), "BAY_CAPACITY", 0);
  if (capacity < 1)
  {
    hs_std_notice(obj, "That ship does not have a docking bay.");
    return;
  }
  
  shipsize = check_ship_size(ship);
  targetsize = check_ship_size(ContactShip(q));
  if (shipsize >= targetsize)
  {
    hs_std_notice(obj, "That ship is not large enough to hold this vessel.");
    return;
  }
  
  current = get_bay_capacity(ContactShip(q));
  if (current + shipsize > capacity)
  {
    hs_std_notice(obj, "That ship does not have sufficient docking bay space.");
    return;
  }

  hs_std_sensor(ship, q, tprintf("Docking with %s%s%s.", ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));
  
  qptr = find_ship_contact(ContactShip(q), ship);
  if (qptr)
    hs_std_sensor(ContactShip(q), qptr, tprintf("%s%s%s has docked with us.", ANSI_HILITE, ship_name(ship), ANSI_NORMAL));
  
  notify_contacts(ship, ContactShip(q), tprintf("%s%s%s has docked on %s%s%s.",
       ANSI_HILITE, ship_name(ship), ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));

  enter_ship(ship, ContactShip(q), bay);
  return;
}


/* launch a ship from a docking bay or landing pad */
void launch_ship(dbref con)
{
  hship *ship;
  dbref obj, bay, player;
  ATTR *a;
  
  if (!IsShip(con))
  {
    hs_std_notice(con, "Only the pilot may launch the ship.");
    return;
  }
  
  ship = find_ship_by_nav(con);
  if (!ship)
  {
    hs_std_notice(con, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (!ship->landed && !ship->docked)
  {
    hs_std_notice(con, "You can't launch now.");
    return;
  }
  
  obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
  bay = atr_parse_dbref(Location(obj), "BAY");
  if (!IsShip(bay) && !IsCelestial(bay))
  {
    hs_std_notice(con, "You can't launch from here.");
    return;
  }
  
  if (ship->landed)
  {
    if (!ship->landed->uid)
    {
      hs_std_notice(con, "You are landed on a planet outside the space-time continuum.");
      return;
    }
    else if (ship->landed->objnum != bay)
    {
      player = get_user(con);
      SPACEWALL(tprintf(
           "HSPACE: !ACHTUNG! %s(#%d) is trying to launch in the %s(#%d) from %s(#%d) on %s(#%d), but they were landed on %s(#%d)!!",
           Name(player), player, ship_name(ship), ship->objnum, Name(Location(obj)), Location(obj),
           celestial_name(find_celestial(bay)), bay, celestial_name(ship->landed), ship->landed->objnum));
      hs_std_notice(con, "You are trying to launch from a planet you're not landed on!");
      return;
    }
  }
  
  if (ship->docked)
  {
    if (!ship->docked->uid)
    {
      hs_std_notice(con, "You are docked on a landed ship.");
      return;
    }
    else if (!HasFlag(ship->docked->type, HS_BAY_OPEN))
    {
      hs_std_notice(con, "Bay doors are closed.");
      return;
    }
    else if (ship->docked->objnum != bay)
    {
      player = get_user(con);
      SPACEWALL(tprintf(
           "HSPACE: !ACHTUNG! %s(#%d) is trying to launch in the %s(#%d) from %s(#%d) on %s(#%d), but they were landed on %s(#%d)!!",
           Name(player), player, ship_name(ship), ship->objnum, Name(Location(obj)), Location(obj),
           ship_name(find_ship(bay)), bay, ship_name(ship->docked), ship->docked->objnum));
      hs_std_notice(con, "You are trying to undock from a ship you're not docked on!");
      return;
    }
  }
  
  hs_std_notice(con, "Launching...");
  leave_ship(ship);
  return;
}


/* look outside a ship if it is landed/docked */
void do_view(dbref con)
{
  hship *ship;
  dbref obj;
  dbref player;
  
  if (!IsConsole(con) && !IsShip(con))
    return;

  ship = find_ship(con);
  if (!ship)
  {
    hs_std_notice(con, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (ship->landing || (!ship->docked && !ship->landed))
  {
    hs_std_notice(con, "You see the emptiness of space.");
    return;
  }
  
  obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
  if (!RealGoodObject(obj))
  {
    hs_std_notice(con, "This ship does not have an external viewport.");
    return;
  }
  
  player = get_user(con);
  notify(player, "You peer outside the viewport and see...");
  look_room(player, Location(obj), LOOK_NORMAL, NULL);
}


/* move a ship if it is landed/docked */
void do_taxi(dbref con, char *which)
{
  hship *ship;
  dbref obj, player, loc;
  
  if (!IsShip(con))
  {
    hs_std_notice(con, "Only the pilot may taxi the ship.");
    return;
  }
  
  ship = find_ship_by_nav(con);
  if (!ship)
  {
    hs_std_notice(con, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (!ship->docked && !ship->landed)
  {
    hs_std_notice(con, "You can't taxi while in space.");
    return;
  }
  
  obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
  if (!RealGoodObject(obj))
  {
    hs_std_notice(con, "This ship is not capable of taxiing.");
    return;
  }
  
  player = get_user(con);
  loc = Location(obj);
  do_move(obj, which, MOVE_NORMAL, NULL);
  if (loc == Location(obj))
  {
    hs_std_notice(con, "You can not taxi that way.");
    return;
  }

  notify_except(obj, loc, obj, tprintf("%s fires its maneuvering thrusters and taxis away.", Name(obj)), 0);
  notify_except(obj, Location(obj), obj, tprintf("There is a load roar as the %s taxis in.", Name(obj)), 0);
  look_room(player, Location(obj), LOOK_NORMAL, NULL);
}


/* set navigation mode flags */
void set_navmode(dbref console, char *which)
{
  hship *ship;
  int len, f;
  
  if (!IsShip(console))
  {
    hs_std_notice(console, "Only the pilot may change navigation mode.");
    return;
  }
  
  ship = find_ship_by_nav(console);
  if (!ship)
  {
    hs_std_notice(console, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  if (!which || !*which)
  {
    /* clear navigation mode */
    FlagOff(ship->type, HS_ANY_MODE);
    hs_std_notice(console, "Navigational mode cleared.");
    return;
  }
  
  f = parse_flags(hs_nav_modes, which);
  if (FlagCount(f) != 1 || !HasFlag(f, HS_ANY_MODE))
  {
    hs_std_notice(console, "You must specify a navigational mode.");
    return;
  }

  FlagOff(ship->type, HS_ANY_MODE);
  FlagOn(ship->type, f);
  hs_std_notice(console, tprintf("Navigational mode changed: %s", STR(hs_nav_modes, f)));

  return;
}

/* eject cargo from the hold */
void dump_cargo(dbref con, char *which, char *amount)
{
  //ATTR *a, *b;
  ALIST *a, *b;
  char name[128];
  char *s, *r;
  
  int len;
  double mass, m;
  hship *ship;
  hcelestial *debris;
  
  int total, maxcargo;
  
  ship = find_ship(con);
  if (!ship)
    return;
  
  if (!ship->uid && !ship->docked)
  {
    hs_std_notice(con, "You may not dump cargo while landed.");
    return;
  }
  
  if (!which)
  {
    hs_std_notice(con, "You must specify a valid cargo type.");
    return;
  }
  len = strlen(which);
  if (len < 1)
  {
    hs_std_notice(con, "You must specify a valid cargo type.");
    return;
  }
  
  if (!amount)
  {
    hs_std_notice(con, "You must specify a valid mass. ");
    return;
  }
  
  mass = parse_number(amount);
  if (mass < 1.0)
  {
    hs_std_notice(con, "You must specify a valid mass. ");
    return;
  }
  
  //for (a = List(ship->objnum); a; a = AL_NEXT(a))
  ATTR_FOR_EACH (ship->objnum, a)
  {
    if (strncasecmp(AL_NAME(a), "CARGO_", 6))
      continue;
    
    strncpy(name, AL_NAME(a), 127);
    s = name;
    r = split_token(&s, '_');
    
    if (strncasecmp(s, which, len))
      continue;
    
    /* we have a valid cargo */
    m = parse_number(atr_value(a));
    if (mass > m)
    {
      hs_std_notice(con, "You don't have that much. ");
      return;
    }
    
    /* check if we are docked, put the cargo onto the host ship if there is room */
    if (ship->docked)
    {
      total = 0;
      
      //for (b = List(ship->docked->objnum); b; b = AL_NEXT(b))
      ATTR_FOR_EACH (ship->docked->objnum, b)
      {
        if (strncasecmp(AL_NAME(b), "CARGO_", 6))
          continue;
        
        total += parse_integer(atr_value(b));
      }
      maxcargo = atr_parse_integer(ship->docked->objnum, "MAX_CARGO", 0);
      
      if ((maxcargo - total) < mass)
      {
        hs_std_notice(con, tprintf("The carrier does not have adequate cargo hold capacity (%s%d%s remaining).",
              ANSI_HILITE, (maxcargo - total), ANSI_NORMAL));
        return;
      }
      
      /* the carrier has space, let's move the cargo */
      total = atr_parse_integer(ship->docked->objnum, (char*) AL_NAME(a), 0);
      atr_add(ship->docked->objnum, AL_NAME(a), tprintf("%.0f", (total+mass)), hs_options.space_wiz, 0);
      
      notify_consoles(ship, tprintf("%s%s$$%s Dumping %s%.0f%s units of %s%s%s to %s%s%s.",
            ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ANSI_HILITE, mass, ANSI_NORMAL,
            ANSI_HILITE, s, ANSI_NORMAL, ANSI_HILITE, ship_name(ship->docked), ANSI_NORMAL));
      
      notify_consoles(ship->docked, tprintf("%s%s$$%s Receiving %s%.0f%s units of %s%s%s from %s%s%s.",
            ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ANSI_HILITE, mass, ANSI_NORMAL,
            ANSI_HILITE, s, ANSI_NORMAL, ANSI_HILITE, ship_name(ship), ANSI_NORMAL));
      
    }
    else if (ship->uid)
    {
      /* create a cargo pod */
      debris = create_debris(ship->uid, ship->x, ship->y, ship->z);
      if (!debris)
      {
        hs_std_notice(con, "Failed to create pod.");
        return;
      }
      
      FlagOn(debris->type, HS_CARGO);
      
      notify_consoles(ship, tprintf("%s%s$$%s Dumping %s%.0f%s units of %s%s%s.",
            ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ANSI_HILITE, mass, ANSI_NORMAL,
            ANSI_HILITE, s, ANSI_NORMAL));
      
      debris->mass = mass;
      debris->radius = 120;
      
      debris->contents = (char *) malloc(sizeof(char) * (strlen(s)+1));
      if (!debris->contents)
        return;
      
      strcpy(debris->contents, s);

    }
    
    if ((m - mass) <= 0.0)
      atr_clr(ship->objnum, AL_NAME(a), hs_options.space_wiz);
    else
      atr_add(ship->objnum, AL_NAME(a), tprintf("%.0f", (m - mass)), hs_options.space_wiz, 0);

    return;
  }
  
  hs_std_notice(con, "You must specify a valid cargo type.");
  return;
}

/* add debris items */
void add_equipment(dbref player, dbref obj)
{
  ATTR *a;
  char *buff, *r, *s;
  dbref tmp;
  
  if (!IsPlayer(player))
    return;
  
  notify_format(player, "%s%s$$%s Salvaging %s%s%s.", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
        ANSI_HILITE, Name(obj), ANSI_NORMAL);
    
  a = atr_get(player, "EQUIPMENT");
  if (!a)
  {
    atr_add(player, "EQUIPMENT", unparse_dbref(obj), hs_options.space_wiz, 0);
    return;
  }
  
  buff = safe_atr_value(a, "EQUIPMENT");
  if (!buff)
  {
    atr_add(player, "EQUIPMENT", unparse_dbref(obj), hs_options.space_wiz, 0);
    return;
  }
  
  s = buff;
  r = split_token(&s, ' ');
  while (r)
  {
    tmp = parse_dbref(r);
    if (tmp == obj)
    {
      free(buff);
      return;
    }
    
    if (!s)
      break;
    
    r = split_token(&s, ' ');
  }

  atr_add(player, "EQUIPMENT", tprintf("%s #%d", atr_value(a), obj), hs_options.space_wiz, 0);
  free(buff);
  return;
}

/* add debris cash */
void add_cash(dbref player, int cash)
{
  int current;
  
  if (!IsPlayer(player))
    return;
    
  if (cash < 1)
    return;
  
  current = atr_parse_integer(player, "CASH", 0);
  atr_add(player, "CASH", unparse_number(current + cash), hs_options.space_wiz, 0);
  
  notify_format(player, "%s%s$$%s You receive %s%dc%s.", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
        ANSI_GREEN, cash, ANSI_NORMAL);
  
  return;
}

/* add debris cargo */
void add_cargo(hship *ship, char *which, int amount)
{
  int current;
  
  if (!ship)
    return;
  
  if (!which)
    return;
  
  if (amount < 1)
    return;
    
  if (strlen(which) < 1)
    return;
  
  current = atr_parse_integer(ship->objnum, tprintf("CARGO_%s", which), 0);
  atr_add(ship->objnum, tprintf("CARGO_%s", which), unparse_number(current + amount), hs_options.space_wiz, 0);

  notify_consoles(ship, tprintf("%s%s$$%s Tractored %s%d%s units of %s%s%s.",
        ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ANSI_HILITE, amount, ANSI_NORMAL,
        ANSI_HILITE, which, ANSI_NORMAL));
      
  return;
}

/* add debris mission items */
void add_missionitem(dbref player, dbref obj)
{
  ATTR *a;
  char *buff, *r, *s;
  int stackable;
  dbref tmp;
  
  if (!IsPlayer(player) || !IsMissionItem(obj))
    return;
  
  notify_format(player, "%s%s$$%s You received %s%s%s.", ANSI_HILITE, ANSI_MAGENTA, ANSI_NORMAL,
        ANSI_HILITE, Name(obj), ANSI_NORMAL);
  
  a = atr_get(player, "MISSIONITEMS");
  if (!a)
  {
    atr_add(player, "MISSIONITEMS", unparse_dbref(obj), hs_options.space_wiz, 0);
    return;
  }
  
  buff = safe_atr_value(a, "MISSIONITEMS");
  if (!buff)
  {
    atr_add(player, "MISSIONITEMS", unparse_dbref(obj), hs_options.space_wiz, 0);
    return;
  }
  
  stackable = atr_parse_integer(obj, "STACKABLE", 0);

  s = buff;
  r = split_token(&s, ' ');
  tmp = NOTHING;
  while (r)
  {
    tmp = parse_dbref(r);
    if (tmp == obj && !stackable)
    {
      return;
    }
    
    r = split_token(&s, ' ');
  }

  if (tmp == NOTHING)
  {
    atr_add(player, "MISSIONITEMS", unparse_dbref(obj), hs_options.space_wiz, 0);
  } else {
    atr_add(player, "MISSIONITEMS", tprintf("%s #%d", atr_value(a), obj), hs_options.space_wiz, 0);
  }
  free(buff);
  return;
}

/* do they have the mission? */
int has_mission(dbref player, dbref m)
{
  ATTR *a;
  dbref obj;
  char *buff, *s, *r;
  
  if (!IsPlayer(player))
    return 0;
  
  if (!IsMission(m))
    return 0;
  
  a = atr_get(player, "MISSIONS");
  if (!a)
    return 0;
    
  buff = safe_atr_value(a, "MISSIONS");
  if (!buff)
    return 0;
  
  s = buff;
  while (s)
  {
    r = split_token(&s, ' ');
    obj = parse_dbref(r);

    if (m == obj)
    {
      free(buff);
      return 1;
    }

    if (!s)
      break;
  }
  
  free(buff);
  return 0;
}

/* do they have the mission item? */
int has_missionitem(dbref player, dbref m)
{
  ATTR *a;
  dbref obj;
  char *buff, *s, *r;
  
  if (!IsPlayer(player))
    return 0;
  
  if (!IsMissionItem(m))
    return 0;
  
  a = atr_get(player, "MISSIONITEMS");
  if (!a)
    return 0;
    
  buff = safe_atr_value(a, "MISSIONITEMS");
  if (!buff)
    return 0;
  
  s = buff;
  r = split_token(&s, ' ');
  while (r)
  {
    obj = parse_dbref(r);
    if (m == obj)
    {
      free(buff);
      return 1;
    }
  }
  
  free(buff);
  return 0;
}

/* tractor cargo into the hold */
void tractor_cargo(dbref con, char *which)
{
  hcontact *q;
  hship *ship;
  hcelestial *cel;
  int i, cash, split;
  int current, max_cargo;
  dbref player, obj, m;
  //ATTR *a;
  ALIST *a;
  int tractored = 0;
  
  ship = find_ship(con);
  if (!ship)
    return;
  
  q = find_contact(ship, parse_integer(which));
  if (!q || !q->contact)
  {
    hs_std_notice(con, "Invalid contact.");
    return;
  }
  
  if (!HasFlag(q->type, HS_DEBRIS))
  {
    hs_std_notice(con, "Invalid contact.");
    return;
  }
  
  if (ContactDistance(ship, q) > hs_options.max_cargo_dist)
  {
    hs_std_notice(con, "Contact is not in range.");
    return;
  }
  
  /* we have a valid debris contact, tractor it */
  /* check for cargo/weapon in contents, and mission items if 
     there's a valid objnum for a ship */
  
  cel = ContactCelestial(q);

  if (cel->contents)
  {
    /* is it a dbref? then it must be a system or a weapon */
    obj = parse_dbref(cel->contents);
    cash = parse_integer(cel->contents);
    if (IsComponent(obj) || IsWeapon(obj))
    {
      /* it's an equipment item */
      /* for now we're going to just give it to the person who tractors */
      /* but i had thought to give a copy to everybody */
      /* maybe we do a round robin or some sort of random rolls */
      add_equipment(get_user(con), obj);
      tractored = 1;
      //add_equipment(get_user(ship->objnum), obj);
      //for (i = 0; i < ship->nconsoles; i++)
      //{
      //  add_equipment(get_user(ship->console[i].objnum), obj);
      //}
      
      /* clear the contents */
      free(cel->contents);
      cel->contents = NULL;
    }
    else if (cash > 0)
    {
      /* it's money, split it evenly among the players */
      player = get_user(ship->objnum);
      if (IsPlayer(player))
        split = 1;
      else
        split = 0;
      
      for (i = 0; i < ship->nconsoles; i++)
      {
        player = get_user(ship->console[i].objnum);
        if (IsPlayer(player))
        {
          split++;
        }
      }
      
      cash /= split;
      
      tractored = 1;
      /* and add the cash */
      add_cash(get_user(ship->objnum), cash);
      
      for (i = 0; i < ship->nconsoles; i++)
      {
        add_cash(get_user(ship->console[i].objnum), cash);
      }
      
      free(cel->contents);
      cel->contents = NULL;
    }
    else
    {
      /* it must be some type of cargo */
      max_cargo = atr_parse_integer(ship->objnum, "MAX_CARGO", 0);
      current = 0;
      
      //for (a = List(ship->objnum); a; a = AL_NEXT(a))
      ATTR_FOR_EACH (ship->objnum, a)
      {
        if (!strncasecmp(AL_NAME(a), "CARGO_", 6))
        {
          current += parse_integer(atr_value(a));
        }
      }
      
      if ((current + cel->mass) <= max_cargo)
      {
        tractored = 1;
        add_cargo(ship, cel->contents, cel->mass);

        /* clear out the cargo */
        free(cel->contents);
        cel->contents = NULL;
        cel->mass = 0.0;
      }
      else if ((max_cargo - current) > 0)
      {
        tractored = 1;
        add_cargo(ship, cel->contents, max_cargo - current);

        /* reduce cargo by the amount taken*/
        cel->mass -= (max_cargo - current);
      }
      else
      {
        hs_std_notice(con, "Cargo hold is at maximum capacity.");
        tractored = 1;
      }
    }
  }
  

  if (IsDrone(cel->objnum))
  {
    /* we have drone debris, check for mission items */
    /* only add them if the console is manned and the user has the mission */
    obj = atr_parse_dbref(cel->objnum, "MISSIONITEM");
    if (IsMissionItem(obj))
    {
      m = atr_parse_dbref(obj, "MISSION");
      if (IsMission(m))
      {
        tractored = 1;
        player = get_user(ship->objnum);
        if (has_mission(player, m))
        {
          add_missionitem(player, obj);
        }
        for (i = 0; i < ship->nconsoles; i++)
        {
          player = get_user(ship->console[i].objnum);
          if (has_mission(player, m))
          {
            add_missionitem(player, obj);
          }
        }
        
        cel->objnum = NOTHING;
      }
    }
  }
  else if (!cel->contents)
  {
    cel->radius = 0.0;
  }
  
  if (!tractored)
  {
    hs_std_notice(con, "You find nothing salvagable.");
  }
  
  return;
}

/****************************************************/
/* status displays */

#define dots	"...................."

/* show a display of cargo on the ship */
void cargo_manifest(dbref con)
{
  //ATTR *a;
  ALIST *a;
  char *r, *s;
  int count, amount, total;

  hship  *tship;
  dbref   player;
  char    tbuf1[256];
  char name[64];
  char name2[64];
  char value[32];
  char output[3][64];
  int column, len;

  tship = find_ship(con);

  if (!tship)
    return;

  player = get_user(con);

  sprintf(tbuf1,
	  "  %s%s.-----------------------------------------------------------------------.%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(name, "%s", ship_name(tship));
  sprintf(name2, "%s (%s)", name, atr_parse_string(tship->objnum, "IDENT"));
  sprintf(tbuf1, " %s%s/%s %-35s%s%s|%s%35s %s%s\\%s", ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, name2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
     atr_parse_string(tship->objnum, "CLASS"), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  sprintf(tbuf1,
	  "%s%s >--------%s %sCargo Hold Status %s---------%s%s+%s------------------------------------< %s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  
  count = 0;
  total = 0;
  column = 0;
  //for (a = List(tship->objnum); a; a = AL_NEXT(a))
  ATTR_FOR_EACH (tship->objnum, a)
  {
    if (strncasecmp(AL_NAME(a), "CARGO_", 6))
      continue;
    
    strncpy(name, AL_NAME(a), 64);
    s = name;
    r = split_token(&s, '_');
    
    if (!s || !*s)
      continue;
    
    strncpy(name2, s, 20);
    name2[21]='\0';
    len = strlen(name2);
    
    amount = parse_integer(atr_value(a));
    if (amount < 1)
      continue;
    
    sprintf(value, "%d", amount);
    len += strlen(value);
    
    if (len < 23)
      strncpy(name, dots, 23 - len);
    else
      strcpy(name, "");
    
    sprintf(output[column], "%s%s%s%s%s%s%s%s", ANSI_HILITE, ANSI_GREEN, name2, ANSI_NORMAL, name, ANSI_HILITE, value, ANSI_NORMAL);
    //notify_format(5, "DEBUG: cargo '%s'    value '%s'    dots '%s'", name2, value, name);
    
    count++;
    column++;
    total += amount;
    
    if (column > 2)
    {
      sprintf(tbuf1, "%s%s|%s  %s  %s  %s   %s%s|%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
        output[0], output[1], output[2], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf1);
      column = 0;
    }
  }

  if (column == 1)
  {
    sprintf(tbuf1, "%s%s|%s  %s                                                   %s%s|%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
          output[0], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  else if (column == 2)
  {
    sprintf(tbuf1, "%s%s|%s  %s  %s                           %s%s|%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
          output[0], output[1], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  else if (total < 1)
  {
    sprintf(tbuf1,
          "%s%s|  %sNo cargo present                                                         %s%s|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }

  sprintf(tbuf1, "%s%s|%s                                         %s%3d/%-3d%s Cargo hold capacity       %s%s|%s",
        ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, total, atr_parse_integer(tship->objnum, "MAX_CARGO", 0),
        ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  sprintf(tbuf1,
	  "%s%s`---------------------------------------------------------------------------'%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
}

/* show a brief ship status report */
void ship_status(dbref con)
{
  dbref   player;
  hcontact *q;
  hship  *tship;
  hship  *dship;
  double  sx, sy;
  char    tl1[64];
  char    tl2[64];
  char    tl3[64];
  char    tl4[64];
  char    tl5[64];
  char    tl6[64];
  char    tl7[64];
  char    tl8[64];
  char    tbuf[256];
  char    tbuf2[256];
  char    tbuf3[256];
  char    tnum1[32];
  char    tnum2[32];
  char    tnum3[32];
  char    astring[32];
  char name[64];
  int     xy, za, zhead, head;
  char    c1, c2, c3, c4, c5, c6, c7, c8, c9, ca, cb, cc, cd;
  int     x1, x2, x3, z1, z2, z3;
  dbref   lock;
  double  dval;
  int     range, dr, ty, tx;
  int incone;
  char    filler, planetchar, torpchar, shipchar;
  double speed;

  if ((tship = find_ship(con)) == NULL)
    return;

  player = get_user(con);

  dship = NULL;
  range = 100;
  incone = 0;
  head = 0;
  zhead = 0;
  planetchar = 'o';
  shipchar = '.';
  torpchar = 'x';

  if (tship->lock)
    lock = tship->lock->cnum;
  else
    lock = NOTHING;

  c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = c9 = ca = cb = cc = cd = ' ';
  if (tship->lock)
  {
    dship = ContactShip(tship->lock);
    za = zang(tship->x, tship->y, tship->z, dship->x, dship->y, dship->z);
    zhead = rint(tship->zhead / 45.0);
    zhead *= 45;
    if ((za == zhead) || (za > (zhead - 23) && za < (zhead + 23)))
    {
      cb = '*';
    }
    else if (za < zhead)
    {
      if (za < (zhead - 68) && za > (zhead - 113))
	cd = '*';
      else if (za >= (zhead - 68))
	cc = '*';
      else
	cd = '<';
    }
    else if (za > zhead)
    {
      if (za > (zhead + 68) && za < (zhead + 113))
	c9 = '*';
      else if (za <= (zhead + 68))
	ca = '*';
      else
	c9 = '<';
    }
    xy = xyang(tship->x, tship->y, dship->x, dship->y);
    xy -= tship->xyhead;
    if (xy < 0)
      xy += 360;
    head = rint(tship->xyhead / 45.0);
    if (head == 8)
      head = 0;
    head *= 45;
    if (xy >= 0 && xy < 23)
      c2 = '*';
    else if (xy >= 23 && xy < 68)
      c3 = '*';
    else if (xy >= 68 && xy < 113)
      c4 = '*';
    else if (xy >= 113 && xy < 158)
      c5 = '*';
    else if (xy >= 158 && xy < 203)
      c6 = '*';
    else if (xy >= 203 && xy < 248)
      c7 = '*';
    else if (xy >= 248 && xy < 293)
      c8 = '*';
    else if (xy >= 293 && xy < 338)
      c1 = '*';
    else
      c2 = '*';
    sprintf(tl1, "                             ");
    incone = get_firing_cone(tship, NULL, ContactShip(tship->lock));
    
    if (incone < 90)
      sprintf(astring, "%s%s", ANSI_HILITE, ANSI_RED);
    else
      sprintf(astring, "%s", ANSI_HILITE);
    
    if (head < 10)
      sprintf(tnum1, " %d ", head);
    else
      sprintf(tnum1, "%3d", head);
    if ((zhead + 90) < 10)
      sprintf(tnum2, "  %d ", zhead + 90);
    else if ((zhead + 90) < 100)
      sprintf(tnum2, " %-3d", zhead + 90);
    else
      sprintf(tnum2, "%-4d", zhead + 90);
    sprintf(tl2, "        %s          %s        ", tnum1, tnum2);
    sprintf(tl3, "      %s%c   %c   %c         %c   %c%s      ",
	    astring, c1, c2, c3, c9, ca, ANSI_NORMAL);
    sprintf(tl4, "%s%s        \\ | /           | /        %s",
	    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
    sprintf(tl6, "        %s%s/ | \\           | \\%s        ",
	    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
    sprintf(tl7, "      %s%c   %c   %c         %c   %c%s      ", ANSI_HILITE,
	    c7, c6, c5, cd, cc, ANSI_NORMAL);
    x3 = head + 180;
    if (x3 > 359)
      x3 -= 360;
    if ((zhead - 90) < 10 && (zhead - 90) > -10)
      sprintf(tnum1, "  %d ", zhead - 90);
    else if ((zhead - 90) < 100 && (zhead - 90) > -100)
      sprintf(tnum1, " %-3d", zhead - 90);
    else
      sprintf(tnum1, "%-4d", zhead - 90);
    if (x3 < 10)
      sprintf(tnum2, " %d ", x3);
    else
      sprintf(tnum2, "%3d", x3);
    sprintf(tl8, "        %s          %s        ", tnum2, tnum1);
  }
  else
  {
    for (dr = 0; dr < 32; dr++)
      tl1[dr] = ' ';
    tl1[dr] = '\0';
    for (dr = 0; dr < 34; dr++)
      tl2[dr] = ' ';
    tl2[dr] = '\0';
    for (dr = 0; dr < 35; dr++)
    {
      tl3[dr] = ' ';
      tl4[dr] = ' ';
      tl5[dr] = ' ';
      tl6[dr] = ' ';
      tl7[dr] = ' ';
    }
    tl3[dr] = '\0';
    tl4[dr] = '\0';
    tl5[dr] = '\0';
    tl6[dr] = '\0';
    tl7[dr] = '\0';
    for (dr = 0; dr < 34; dr++)
      tl8[dr] = ' ';
    tl8[dr] = '\0';
    dr = range / 4;
    for (q = get_head_contact(tship); q; q = q->next)
    {
      /* remove HS_CELESTIAL with 0xFF mask */
      if (HasFlag(q->type, HS_STAR | HS_PLANET))
      {
	sx = ContactCelestial(q)->x;
	sy = ContactCelestial(q)->y;
	tx = ((rint(ContactCelestial(q)->x - tship->x) / (range * 2 / 34.0))) + 17.5;
	ty = (rint(tship->y - ContactCelestial(q)->y) / dr) + 4;
	filler = planetchar;
      }
      else if (HasFlag(q->type, HS_WORMHOLE | HS_WAYPOINT | HS_ASTEROID | HS_ANOMALY | HS_DEBRIS))
      {
	sx = ContactCelestial(q)->x;
	sy = ContactCelestial(q)->y;
	tx = ((rint(ContactCelestial(q)->x - tship->x) / (range * 2 / 34.0))) + 17.5;
	ty = (rint(tship->y - ContactCelestial(q)->y) / dr) + 4;
	filler = torpchar;
      }
      else if (HasFlag(q->type, HS_ANY_SHIP))
      {
	sx = ContactShip(q)->x;
	sy = ContactShip(q)->y;
	tx = ((rint(ContactShip(q)->x - tship->x) / (range * 2 / 34.0))) + 17.5;
	ty = (rint(tship->y - ContactShip(q)->y) / dr) + 4;
	filler = shipchar;
      } else {
        sx = 0;
        sy = 0;
        tx = 0;
        ty = 0;
        filler = ' ';
      }

      if ((tship->x - sx) > range ||
	  (tship->x - sx) < (range * -1) ||
	  (tship->y - sy) > range ||
	  (tship->y - sy) < (range * -1))
	continue;

      switch (ty)
      {
      case 0:
	if (tx >= 3 && tx < 32)
	  tl1[tx] = filler;
	break;
      case 1:
	if (tx >= 1 && tx < 33)
	  tl2[tx] = filler;
	break;
      case 2:
	tl3[tx] = filler;
	break;
      case 3:
	tl4[tx] = filler;
	break;
      case 4:
	tl5[tx] = filler;
	break;
      case 5:
	tl6[tx] = filler;
	break;
      case 6:
	tl7[tx] = filler;
	break;
      default:
	tl8[tx] = filler;
	break;
      }
    }
  }
  /*
   * Calculate XY HUD 
   */
  x2 = (int) (rint(tship->xyhead / 10.0)) * 10;
  x1 = x2 - 10;
  x3 = x2 + 10;
  if (x1 < 0)
    x1 += 360;
  else if (x1 > 359)
    x1 -= 360;
  if (x3 < 0)
    x3 += 360;
  else if (x2 > 359)
    x3 -= 360;
  z2 = (int) (rint(tship->zhead / 10.0)) * 10;
  z1 = z2 + 10;
  z3 = z2 - 10;
  sprintf(tbuf,
	  "  %s%s.-----------------------------------------------------------------------.%s  ",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  
  sprintf(name, "%s", ship_name(tship));
  sprintf(tbuf2, "%s (%s)", name,
	  atr_parse_string(tship->objnum, "IDENT"));
  sprintf(tbuf, " %s%s/%s %-34s %s%s|%s %34s %s%s\\%s", ANSI_HILITE,
	  ANSI_BLUE, ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	  atr_parse_string(tship->objnum, "CLASS"), ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL);
  notify(player, tbuf);
  sprintf(tbuf,
	  "%s%s >--------%s %sNavigation Status %s---------%s%s+%s-----------%s%s+%s------------------------< %s",
   ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (x1 < 10)
    sprintf(tnum1, " %d ", x1);
  else
    sprintf(tnum1, "%-3d", x1);
  if (x2 < 10)
    sprintf(tnum2, " %d ", x2);
  else
    sprintf(tnum2, "%3d", x2);
  if (x3 < 10)
    sprintf(tnum3, " %d ", x3);
  else
    sprintf(tnum3, "%3d", x3);
  sprintf(tbuf,
	  "%s%s|%s                   %s        %s        %s     %s%s| %sX:%s %10.0f           %s%s|%s",
      ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tnum1, tnum2, tnum3, ANSI_HILITE,
	  ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, tship->x,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  sprintf(tbuf,
	  "%s%s|%s %4d%s%s__             |____%s%s.%s_____|_____%s%s.%s____|      %s| %sY:%s %10.0f           %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, z1, ANSI_HILITE, ANSI_GREEN,
  ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN,
	  ANSI_BLUE, ANSI_GREEN,
	  ANSI_NORMAL, tship->y, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (tship->xyhead < 10)
    sprintf(tnum1, " %.0f ", tship->xyhead);
  else
    sprintf(tnum1, "%3.0f", tship->xyhead);
  sprintf(tbuf,
	  "%s%s|       %s|%s                    > %s <              %s%s| %sZ:%s %10.0f           %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	  tnum1, ANSI_HILITE, ANSI_BLUE,
    ANSI_GREEN, ANSI_NORMAL, tship->z, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  sprintf(tbuf,
	  "%s%s|%s      %s-%s|          %s___________________________    |%s %s+%s- Course -%s%s+            %s|%s",
    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_BLUE,
	  ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (lock == NOTHING)
  {
    *tbuf2 = '\0';
    ty = 0;
    for (dr = 3; tl1[dr]; dr++)
    {
      if (tl1[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl1[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl1[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf2, tnum1);
    }
  }
  else
  {
    strcpy(tbuf2, tl1);
  }

  sprintf(tbuf,
	  "%s%s|%s %4d%s%s-->%s %-3.0f    %s%s/%s%s%s%s\\  | %sC:%s %3.0f/%-3.0f  %s%sD:%s %3.0f/%-3.0f  %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, z2, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	  tship->zhead, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE,
	  ANSI_GREEN, ANSI_NORMAL,
	  tship->xyhead, tship->zhead, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
      tship->desired_xyhead, tship->desired_zhead, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);

  if (lock == NOTHING)
  {
    *tbuf3 = '\0';
    ty = 0;
    for (dr = 1; tl2[dr]; dr++)
    {
      if (tl2[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl2[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl2[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf3, tnum1);
    }
  }
  else
    strcpy(tbuf3, tl2);
  
  speed = tship->speed;
  if (HasFlag(tship->type, HS_AFTERBURNING))
    speed *= hs_options.burn_multiplier;
  
  sprintf(tbuf2, "%.0f/%.0f (%.0f)", 
          speed, tship->desired_speed,
	  get_stat(tship, SYS_VELOCITY));
  sprintf(tbuf,
	"%s%s|%s      %s-%s|%s      %s%s/%s%s%s%s\\| %sV:%s %-19s  %s%s|%s",
  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf3, ANSI_HILITE, ANSI_BLUE,
       ANSI_GREEN, ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (lock == NOTHING)
  {
    *tbuf3 = '\0';
    ty = 0;
    for (dr = 0; tl3[dr]; dr++)
    {
      if (tl3[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl3[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl3[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf3, tnum1);
    }
  }
  else
    strcpy(tbuf3, tl3);
  sprintf(tbuf,
	  "%s%s|     %s__|     %s|%s%s%s%s|                         |%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL,
	  tbuf3, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (lock == NOTHING)
  {
    *tbuf3 = '\0';
    ty = 0;
    for (dr = 0; tl4[dr]; dr++)
    {
      if (tl4[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl4[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl4[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf3, tnum1);
    }
    sprintf(tbuf2, " %s%sMap Range:%s %-8d     ", ANSI_HILITE, ANSI_GREEN,
	    ANSI_NORMAL, range);
  }
  else
  {
    strcpy(tbuf3, tl4);
    sprintf(tbuf2, "   %s+%s- Target %4d -%s%s+%s     ", ANSI_HILITE,
	    ANSI_GREEN, lock, ANSI_NORMAL, ANSI_HILITE, ANSI_NORMAL);
  }
  sprintf(tbuf,
	  "%s%s|%s %4d        %s%s|%s%s%s%s|%s%s%s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, z3, ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, tbuf3, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf2,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (lock == NOTHING)
  {
    *tbuf3 = '\0';
    ty = 0;
    for (dr = 0; tl5[dr]; dr++)
    {
      if (dr == 17)
	sprintf(tnum1, "%s+%s", ANSI_HILITE, ANSI_NORMAL);
      else if (tl5[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl5[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl5[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf3, tnum1);
    }
    sprintf(tbuf,
	    "%s%s|             |%s%s%s%s|                         |%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf3, ANSI_HILITE,
	    ANSI_BLUE, ANSI_NORMAL);
  }
  else
  {
    x1 = head - 90;
    if (x1 < 0)
      x1 += 360;
    x2 = head + 90;
    if (x2 > 359)
      x2 -= 360;
    xy = xyang(tship->x, tship->y, dship->x, dship->y);
    za = zang(tship->x, tship->y, tship->z, dship->x, dship->y, dship->z);
    sprintf(tbuf,
	    "%s%s|             |%s %3d %s%c %s--%s %s+ %s--%s %s%c%s %-3d    %s+ %s--%s %s%c%s %-3d %s%s|   %sB: %s%3d/%-3d            %s%s|%s",
       ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, x1, ANSI_HILITE, c8, ANSI_GREEN,
	 ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, c4,
	 ANSI_NORMAL, x2, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, astring, cb,
	    ANSI_NORMAL, zhead, ANSI_HILITE, ANSI_BLUE, ANSI_GREEN,
	    ANSI_NORMAL, xy, za, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  }
  notify(player, tbuf);
  if (lock == NOTHING)
  {
    *tbuf3 = '\0';
    ty = 0;
    for (dr = 0; tl6[dr]; dr++)
    {
      if (tl6[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl6[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl6[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf3, tnum1);
    }
    strcpy(tbuf2, "                         ");
  }
  else
  {
    strcpy(tbuf3, tl6);
    sprintf(tnum1, "%.0f Mm",
	dist3d(tship->x, tship->y, tship->z, dship->x, dship->y, dship->z));
    sprintf(tbuf2, "   %s%sR:%s %-10s         ", ANSI_HILITE,
	    ANSI_GREEN, ANSI_NORMAL, tnum1);
  }
  
  sprintf(tbuf,
	  "%s%s| %sarmr:%s %3.0f%%  %s%s|%s%s%s%s|%s%s%s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	  100.0 * (get_stat(tship, SYS_ARMOR) / get_stat(tship, SYS_MAX_ARMOR)), ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, tbuf3, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL);
  notify(player, tbuf);
  if (lock == NOTHING)
  {
    *tbuf3 = '\0';
    ty = 0;
    for (dr = 0; tl7[dr]; dr++)
    {
      if (tl7[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl7[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl7[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf3, tnum1);
    }
    strcpy(tbuf2, "                         ");
  }
  else
  {
    strcpy(tbuf3, tl7);
    sprintf(tbuf2, "   %s%sH:%s %3.0f/%-3.0f            ", ANSI_HILITE,
	    ANSI_GREEN, ANSI_NORMAL, dship->xyhead, dship->zhead);
  }
  if (tship->shield.maxenergy == 0)
    dval = 0;
  else
    dval = 100 * (get_stat(tship, SYS_CAPACITY) / get_stat(tship, SYS_MAX_CAPACITY));
  sprintf(tbuf,
	  "%s%s| %sshld:%s %3.0f%%  %s%s|%s%s%s%s|%s%s%s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, dval,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf3, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	  tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (lock == NOTHING)
  {
    *tbuf3 = '\0';
    ty = 0;
    for (dr = 1; tl8[dr]; dr++)
    {
      if (tl8[dr] == shipchar)
	sprintf(tnum1, "%s%s%c%s", ANSI_HILITE, ANSI_RED, shipchar, ANSI_NORMAL);
      else if (tl8[dr] == planetchar)
	sprintf(tnum1, "%s%c%s", ANSI_CYAN, planetchar, ANSI_NORMAL);
      else if (tl8[dr] == torpchar)
	sprintf(tnum1, "%s%c%s", ANSI_YELLOW, torpchar, ANSI_NORMAL);
      else
	sprintf(tnum1, " ");
      strcat(tbuf3, tnum1);
    }
    strcpy(tbuf2, "                          ");
  }
  else
  {
    if (incone < 90)
      sprintf(tbuf2, "    %s* %s-- In Cone --%s %s*%s     ",
	      ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ANSI_HILITE, ANSI_NORMAL);
    else
      strcpy(tbuf2, "                          ");
    strcpy(tbuf3, tl8);
  }
  if (tship->shield.maxenergy == 0)
    dval = 0;
  else
    dval = 100 * (get_stat(tship, SYS_CAPACITY) / get_stat(tship, SYS_MAX_CAPACITY));
  sprintf(tbuf,
	  "%s%s|              \\%s%s%s%s/%s%s%s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, tbuf3, ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
  if (lock == NOTHING)
    sprintf(tbuf,
	    "%s%s`---------------\\\\.___________________________.//---------------------------'%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  else
    sprintf(tbuf,
	    "%s%s`---------------\\\\._____|%sXY%s|__%s%sTarget%s__|%sZ%s|_____.//---------------------------'%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN,
	    ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE,
	    ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf);
}

/* show a system status report for ships/drones */
void system_status(dbref con)
{
  hship  *tship;
  dbref   player;
  char    tbuf1[256];
  char    tbuf2[256];
  char name[64];
  char name2[64];
  double  t, t2;

  tship = find_ship(con);

  if (!tship)
    return;

  player = get_user(con);

  sprintf(tbuf1,
	  "  %s%s.-----------------------------------------------------------------------.%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(name, "%s", ship_name(tship));
  sprintf(tbuf2, "%s (%s)", name, atr_parse_string(tship->objnum, "IDENT"));
  sprintf(tbuf1, " %s%s/%s %-35s%s%s|%s%35s %s%s\\%s", ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
     atr_parse_string(tship->objnum, "CLASS"), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s >--------%s %sSystems Status %s------------%s%s+%s------------------------------------<%s",
   ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* hull and shield */
  sprintf(tbuf1,
	  "%s%s|  %sHull                  Status      %s.^.  %sShield                Status      %s|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s|  --------------------  ----------  | |  --------------------  ----------  |%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* hull and shield names */
  if (RealGoodObject(tship->hull.objnum))
    strncpy(name, Name(tship->hull.objnum), 32);
  else
    strncpy(name, "Hull offline.", 32);

  if (RealGoodObject(tship->shield.objnum))
    strncpy(name2, Name(tship->shield.objnum), 32);
  else
    strncpy(name2, "Shield offline.", 32);

  sprintf(tbuf1,
	  "%s%s|  %s%-32s  %s| |  %s%-32s  %s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, 
	  ANSI_BLUE, ANSI_GREEN, name2, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* hull and shield statistics */
  t = get_stat(tship, SYS_REPAIR);
  t2 = get_stat(tship, SYS_REGENERATION);
  sprintf(tbuf1,
	  "%s%s|               %sRepair:%s  %4.0f mm/s   %s%s| |         %sRegeneration:%s  %4.0f MW     %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = get_stat(tship, SYS_TOUGHNESS);
  t2 = get_stat(tship, SYS_ABSORPTION);
  sprintf(tbuf1,
	  "%s%s|            %sToughness:%s  %4.0f%%       %s%s| |           %sAbsorption:%s  %4.0f%%       %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = get_stat(tship, SYS_ABLATION);
  t2 = get_stat(tship, SYS_DEFLECTION);
  sprintf(tbuf1,
	  "%s%s|             %sAblation:%s  %4.0f%%       %s%s| |           %sDeflection:%s  %4.0f%%       %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = 100.0 * get_stat(tship, SYS_ARMOR) / get_stat(tship, SYS_MAX_ARMOR);

  t2 = 100.0 * get_stat(tship, SYS_CAPACITY) / get_stat(tship, SYS_MAX_CAPACITY);
  sprintf(tbuf1,
	  "%s%s \\               %sArmor:%s  %4.0f%%       %s%s| |             %sCapacity:%s  %4.0f%%      %s%s/ %s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);


  /* reactor and engine */
  sprintf(tbuf1,
	  "%s%s  >---------------------------------<   >---------------------------------<  %s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  " %s%s/ %sReactor               Status      %s| |  %sEngine                Status     %s\\%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s|  --------------------  ----------  | |  --------------------  ----------  |%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* reactor and engine names */
  if (RealGoodObject(tship->reactor.objnum))
    strncpy(name, Name(tship->reactor.objnum), 32);
  else
    strncpy(name, "Reactor offline.", 32);

  if (RealGoodObject(tship->engine.objnum))
    strncpy(name2, Name(tship->engine.objnum), 32);
  else
    strncpy(name2, "Engine offline.", 32);

  sprintf(tbuf1,
	  "%s%s|  %s%-32s  %s| |  %s%-32s  %s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, 
	  ANSI_BLUE, ANSI_GREEN, name2, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* reactor and engine statistics */
  t = get_stat(tship, SYS_RECHARGE);
  t2 = get_stat(tship, SYS_VELOCITY);
  sprintf(tbuf1,
	  "%s%s|             %sRecharge:%s  %4.0f MW     %s%s| |             %sVelocity:%s  %4.0f km/s   %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = get_stat(tship, SYS_DISCHARGE);
  t2 = get_stat(tship, SYS_THRUST);
  sprintf(tbuf1,
	  "%s%s|            %sDischarge:%s  %4.0f MW     %s%s| |               %sThrust:%s  %4.0f km/ss  %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = get_stat(tship, SYS_RESISTANCE);
  t2 = get_stat(tship, SYS_DISSIPATION);
  sprintf(tbuf1,
	  "%s%s|           %sResistance:%s  %4.0f Ohm    %s%s| |          %sDissipation:%s  %4.0f K/s    %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = 100.0 * get_stat(tship, SYS_ENERGY)  / get_stat(tship, SYS_MAX_ENERGY);
  t2 = 100.0 * get_stat(tship, SYS_HEAT) / get_stat(tship, SYS_MAX_HEAT);
  sprintf(tbuf1,
	  "%s%s \\               %sPower:%s  %4.0f%%       %s%s| |                 %sHeat:%s  %4.0f%%      %s%s/ %s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);


  /* sensor and computer */
  sprintf(tbuf1,
	  "%s%s  >---------------------------------<   >---------------------------------<  %s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s / %sSensor                Status      %s| |  %sComputer              Status     %s\\ %s",
          ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_GREEN, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s|  --------------------  ----------  | |  --------------------  ----------  |%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* sensor and computer names */
  if (RealGoodObject(tship->sensor.objnum))
    strncpy(name, Name(tship->sensor.objnum), 32);
  else
    strncpy(name, "Sensor offline.", 32);

  if (RealGoodObject(tship->computer.objnum))
    strncpy(name2, Name(tship->computer.objnum), 32);
  else
    strncpy(name2, "Computer offline.", 32);

  sprintf(tbuf1,
	  "%s%s|  %s%-32s  %s| |  %s%-32s  %s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, 
	  ANSI_BLUE, ANSI_GREEN, name2, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* sensor and computer statistics */
  t = get_stat(tship, SYS_TRACKING);
  t2 = get_stat(tship, SYS_DECRYPTION);
  sprintf(tbuf1,
	  "%s%s|             %sTracking:%s  %4.0f s      %s%s| |           %sDecryption:%s  %4.0f Tb/s   %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = get_stat(tship, SYS_RESOLUTION);
  t2 = get_stat(tship, SYS_ENCRYPTION);
  sprintf(tbuf1,
	  "%s%s|           %sResolution:%s  %4.0f nm     %s%s| |           %sEncryption:%s  %4.0f Tb/s   %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = get_stat(tship, SYS_SENSITIVITY);
  t2 = get_stat(tship, SYS_COMPRESSION);
  sprintf(tbuf1,
	  "%s%s|          %sSensitivity:%s  %4.0f nm     %s%s| |          %sCompression:%s  %4.0f%%       %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  t = 100.0 * get_stat(tship, SYS_FOCUS) / get_stat(tship, SYS_MAX_FOCUS);

  t2 = 100.0 * get_stat(tship, SYS_MEMORY) / get_stat(tship, SYS_MAX_MEMORY);
  sprintf(tbuf1,
	  "%s%s|                %sFocus:%s  %4.0f%%       %s%s`.'               %sMemory:%s  %4.0f%%       %s%s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL,
	  t, ANSI_HILITE, ANSI_BLUE, ANSI_WHITE, ANSI_NORMAL, 
	  t2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  sprintf(tbuf1,
	  "%s%s`-------------------------------------%s%s+%s-------------------------------------'%s",
  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

}

/* show a quick power status */
void quick_status(dbref con)
{
  hship  *tship;
  dbref   player;
  char    tbuf1[256];
  char    tbuf2[256];
  char name[64];
  char name2[64];
  double  t, t2;

  tship = find_ship(con);

  if (!tship)
    return;

  player = get_user(con);

  sprintf(tbuf1,
	  "  %s%s.-----------------------------------------------------------------------.%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(name, "%s", ship_name(tship));
  sprintf(tbuf2, "%s (%s)", name, atr_parse_string(tship->objnum, "IDENT"));
  sprintf(tbuf1, " %s%s/%s %-35s%s%s|%s%35s %s%s\\%s", ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
     atr_parse_string(tship->objnum, "CLASS"), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s >--------%s %sSystems Status %s------------%s%s+%s------------------------------------<%s",
   ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* hull and shield names */
  if (RealGoodObject(tship->hull.objnum))
    strncpy(name, Name(tship->hull.objnum), 27);
  else
    strncpy(name, "Hull offline.", 27);

  if (RealGoodObject(tship->shield.objnum))
    strncpy(name2, Name(tship->shield.objnum), 27);
  else
    strncpy(name2, "Shield offline.", 27);

  /* hull and shield statistics */
  t = 100.0 * get_stat(tship, SYS_ARMOR) / get_stat(tship, SYS_MAX_ARMOR);
  t2 = 100.0 * get_stat(tship, SYS_CAPACITY) / get_stat(tship, SYS_MAX_CAPACITY);
  sprintf(tbuf1,
	  "%s%s|  %s%-26s%s  %s%3.0f%%   %s|   %s%-26s%s  %s%3.0f%%  %s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, ANSI_NORMAL,
	  ANSI_HILITE, t, ANSI_BLUE, ANSI_GREEN, name2, ANSI_NORMAL, 
	  ANSI_HILITE, t2, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* reactor and engine */
  sprintf(tbuf1,
	  "%s%s|  --------------------------  ----   |   --------------------------  ----  |%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* reactor and engine names */
  if (RealGoodObject(tship->reactor.objnum))
    strncpy(name, Name(tship->reactor.objnum), 27);
  else
    strncpy(name, "Reactor offline.", 27);

  if (RealGoodObject(tship->engine.objnum))
    strncpy(name2, Name(tship->engine.objnum), 27);
  else
    strncpy(name2, "Engine offline.", 27);

  /* reactor and engine statistics */
  t = 100.0 * get_stat(tship, SYS_ENERGY)  / get_stat(tship, SYS_MAX_ENERGY);
  t2 = 100.0 * get_stat(tship, SYS_HEAT) / get_stat(tship, SYS_MAX_HEAT);
  sprintf(tbuf1,
	  "%s%s|  %s%-26s%s  %s%3.0f%%   %s|   %s%-26s%s  %s%3.0f%%  %s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, ANSI_NORMAL,
	  ANSI_HILITE, t, ANSI_BLUE, ANSI_GREEN, name2, ANSI_NORMAL, 
	  ANSI_HILITE, t2, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* sensor and computer */
  sprintf(tbuf1,
	  "%s%s|  --------------------------  ----   |   --------------------------  ----  |%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /* sensor and computer names */
  if (RealGoodObject(tship->sensor.objnum))
    strncpy(name, Name(tship->sensor.objnum), 27);
  else
    strncpy(name, "Sensor offline.", 27);

  if (RealGoodObject(tship->computer.objnum))
    strncpy(name2, Name(tship->computer.objnum), 27);
  else
    strncpy(name2, "Computer offline.", 27);

  /* sensor and computer statistics */
  t = 100.0 * get_stat(tship, SYS_FOCUS) / get_stat(tship, SYS_MAX_FOCUS);
  t2 = 100.0 * get_stat(tship, SYS_MEMORY) / get_stat(tship, SYS_MAX_MEMORY);
  sprintf(tbuf1,
	  "%s%s|  %s%-26s%s  %s%3.0f%%   %s|   %s%-26s%s  %s%3.0f%%  %s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, ANSI_NORMAL,
	  ANSI_HILITE, t, ANSI_BLUE, ANSI_GREEN, name2, ANSI_NORMAL, 
	  ANSI_HILITE, t2, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  sprintf(tbuf1,
	  "%s%s`-------------------------------------%s%s+%s-------------------------------------'%s",
  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

}

/* show the status of any buffs or effects */
void effect_status(dbref con)
{
  hship  *tship;
  dbref   player;
  char    tbuf1[256];
  char    tbuf2[128];
  char name[3][128];
  char ident[3][64];
  char duration[3][16];
  char stack[16];
  double  t, t2;
  hbuff *b;
  int len, count, total;

  tship = find_ship(con);

  if (!tship)
    return;

  player = get_user(con);

  /* print effect status header with ship name, ident, class */
  sprintf(tbuf1,
	  "  %s%s.-----------------------------------------------------------------------.%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(name[0], "%s", ship_name(tship));
  sprintf(tbuf2, "%s (%s)", name[0], atr_parse_string(tship->objnum, "IDENT"));
  sprintf(tbuf1, " %s%s/%s %-35s%s%s|%s%35s %s%s\\%s", ANSI_HILITE, ANSI_BLUE,
	  ANSI_NORMAL, tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
     atr_parse_string(tship->objnum, "CLASS"), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  /*|  12345678901234567890     123456789012345678901     12345678901234567890   */
  /* > ----- Missile --------.------- Residue ---------.--------- Beam ------- < */
  /*|  Incendiary Missile    |  Superheated Residue    |  Proton Beam           |*/
  /* > ----- Bypass ---------^------- Booster ---------^-------- Emitter ----- < */

  /* STANCE: show weapon slot stances here */
  count = 0;
  total = 0;
  for (b = tship->head_buff; b; b = b->next)
  {
    if (!b->buff || !HasFlag(b->buff->flags, HS_ANY_STANCE))
      continue;
    
    switch (HasFlag(b->buff->flags, HS_ANY_STANCE))
    {
    case HS_MISSILE_STANCE:
      sprintf(ident[count], "%s%s------%s %sMissile %s-----%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      break;
    case HS_CANNON_STANCE:
      sprintf(ident[count], "%s%s------%s %sCannon %s------%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      break;
    case HS_EMITTER_STANCE:
      sprintf(ident[count], "%s%s------%s %sEmitter %s-----%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      break;
    case HS_CAPACITOR_STANCE:
      sprintf(ident[count], "%s%s------%s %sBypass %s------%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      break;
    case HS_BOOSTER_STANCE:
      sprintf(ident[count], "%s%s------%s %sBooster %s-----%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      break;
    default:
      sprintf(ident[count], "%s%s------%s %sUnknown %s-----%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      break;
    }
    
    snprintf(name[count], 21, "%s", STR(hs_stances, b->buff));
    count++;
    total++;
    
    if (count > 2)
    {
      sprintf(tbuf1,
  	    "%s%s|--%s%40s%s%s--.--%s%-40s%s%s--.--%s%-40s%s%s---|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ident[0], ANSI_HILITE, ANSI_BLUE,
	    ANSI_NORMAL, ident[1], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ident[2],
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf1);

      sprintf(tbuf1,
  	    "%s%s|  %s%-20s  %s|  %s%-20s  %s|  %s%-21s  %s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name[0], ANSI_BLUE, ANSI_GREEN,
	     name[1], ANSI_BLUE, ANSI_GREEN, name[2], ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf1);
      count = 0;
    }
  }

  /* STANCE: print any partial lines remaining */
  if (count == 1)
  {
    sprintf(tbuf1,
          "%s%s|--%s%40s%s%s--.------------------------.-------------------------|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ident[0], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);

    sprintf(tbuf1,
           "%s%s|  %s%-20s  %s|                        |                         |%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name[0], ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  else if (count == 2)
  {
    sprintf(tbuf1,
          "%s%s|--%s%40s%s%s--.--%s%-40s%s%s--.-------------------------|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ident[0], ANSI_HILITE, ANSI_BLUE,
          ANSI_NORMAL, ident[1], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);

    sprintf(tbuf1,
          "%s%s|  %s%-20s  %s|  %s%-20s  %s|                         |%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name[0], ANSI_BLUE, ANSI_GREEN,
          name[1], ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  
  /* STANCE: print the seperator different depending on if we have stances */
  if (total > 0)
  {
    sprintf(tbuf1,
  	  "%s%s >----%s %sEffect Status %s----^------------------------^------------------------<%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  } else {
    sprintf(tbuf1,
          "%s%s >----%s %sEffect Status %s------------------------------------------------------<%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  
               /***************************/
               /* now do standard effects */
  
  
  /* EFFECT: print standard, non-stance, non-cooldown buffs */
  count = 0;
  total = 0;
  for (b = tship->head_buff; b; b = b->next)
  {
    if (!b->buff || HasFlag(b->buff->flags, HS_ANY_STANCE | HS_ANY_COOLDOWN))
      continue;
    
    if (HasFlag(b->buff->flags, HS_STACKABLE))
    {
      len = 17;
      sprintf(stack, " x%d", b->stacks);
    } else {
      len = 20;
      strcpy(stack, "");
    }

    if (HasFlag(b->buff->flags, HS_HACK))
      snprintf(tbuf2, len, "%s", STR(hs_hacks, b->buff));
    else
      snprintf(tbuf2, len, "%s", STR(hs_effects, b->buff));

    sprintf(name[count], "%s%s%s%s%s%s%s", ANSI_HILITE, ANSI_GREEN, tbuf2, ANSI_NORMAL, ANSI_HILITE, stack, ANSI_NORMAL);

    if (b->duration > 3600)
      sprintf(duration[count], "---");
    else if (b->duration > 60)
      sprintf(duration[count], "%2dm", b->duration / 60);
    else
      sprintf(duration[count], "%2ds", b->duration);

    count++;
    total++;
    
    /* EFFECT: check for a full line */
    if (count > 2)
    {
      sprintf(tbuf1,
  	    "%s%s| %s%3s%s %-41s  %s%s%3s%s %-41s  %s%s%3s%s %-41s %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, duration[0], ANSI_NORMAL, name[0], ANSI_HILITE, ANSI_YELLOW,
	    duration[1], ANSI_NORMAL, name[1], ANSI_HILITE, ANSI_YELLOW, duration[2], ANSI_NORMAL,
	    name[2], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf1);
      count = 0;
    }
  }

  /* EFFECT: check for partial lines */
  if (count == 1)
  {
    sprintf(tbuf1,
          "%s%s| %s%3s%s %-41s                                                   %s%s|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, duration[0], ANSI_NORMAL, name[0], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  else if (count == 2)
  {
    sprintf(tbuf1,
          "%s%s| %s%3s%s %-41s  %s%s%3s%s %-41s                          %s%s|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, duration[0], ANSI_NORMAL, name[0], ANSI_HILITE, ANSI_YELLOW,
          duration[1], ANSI_NORMAL, name[1], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  else if (total < 1)
  {
    /* EFFECT: print a blank entry to keep the exterior border formatting consistent */
    sprintf(tbuf1,
          "%s%s|  %sNo effects present                                                       %s%s|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  
               /********************/
               /* now do cooldowns */
  
  
  /* COOLDOWN: print cooldowns */
  count = 0;
  total = 0;
  for (b = tship->head_buff; b; b = b->next)
  {
    if (!b->buff || !HasFlag(b->buff->flags, HS_ANY_COOLDOWN))
      continue;
    
    len = 20;

    snprintf(tbuf2, len, "%s", STR(hs_cooldowns, b->buff));

    sprintf(name[count], "%s%s%s%s%s%s", ANSI_HILITE, ANSI_GREEN, tbuf2, ANSI_NORMAL, ANSI_HILITE, ANSI_NORMAL);

    if (b->duration > 3600)
      sprintf(duration[count], "---");
    else if (b->duration > 60)
      sprintf(duration[count], "%2dm", b->duration / 60);
    else
      sprintf(duration[count], "%2ds", b->duration);

    count++;
    total++;
    
    /* STANCE: print the seperator different depending on if we have stances */
    if (total == 1)
    {
      sprintf(tbuf1,
            "%s%s >----%s %sCooldown Status %s----------------------------------------------------<%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf1);
    }

    /* COOLDOWN: check for a full line */
    if (count > 2)
    {
      sprintf(tbuf1,
  	    "%s%s| %s%3s%s %-41s  %s%s%3s%s %-41s  %s%s%3s%s %-41s %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, duration[0], ANSI_NORMAL, name[0], ANSI_HILITE, ANSI_YELLOW,
	    duration[1], ANSI_NORMAL, name[1], ANSI_HILITE, ANSI_YELLOW, duration[2], ANSI_NORMAL,
	    name[2], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf1);
      count = 0;
    }
  }

  /* COOLDOWN: check for partial lines */
  if (count == 1)
  {
    sprintf(tbuf1,
          "%s%s| %s%3s%s %-41s                                                   %s%s|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, duration[0], ANSI_NORMAL, name[0], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  else if (count == 2)
  {
    sprintf(tbuf1,
          "%s%s| %s%3s%s %-41s  %s%s%3s%s %-41s                          %s%s|%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, duration[0], ANSI_NORMAL, name[0], ANSI_HILITE, ANSI_YELLOW,
          duration[1], ANSI_NORMAL, name[1], ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }
  
  sprintf(tbuf1,
	  "%s%s`---------------------------------------------------------------------------'%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

}

/* show a combat summary with weapons status and target information */
void combat_status(dbref console)
{
  char    tbuf1[256];
  char    tbuf2[256];
  hship  *ship = NULL;
  hship *dship;
  dbref   player;
  int n;
  char    status[32];
  char name[64];
  char rld[8], spd[8], pow[8], rng[8], dmg[8];
  hconsole *con;
  hweapon *gun[2];
  hcontact *lock;
  char *cbuf;
  int incone;
  double dist;
  double shield, armor, tshield, tarmor, speed, dspeed;

  if (!IsConsole(console) && !IsShip(console))
    return;

  player = get_user(console);
  if (!RealGoodObject(player))
    return;

  ship = find_ship(console);
  if (!ship)
  {
    hs_std_notice(console, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }

  con = find_console(console);
  if (!con)
  {
    gun[0] = &ship->primary;
    gun[1] = &ship->secondary;
    lock = ship->lock;
  } else {
    gun[0] = &con->primary;
    gun[1] = &con->secondary;
    lock = con->lock;
  }

  sprintf(tbuf1,
	  "  %s%s.-----------------------------------------------------------------------.%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(name, "%s", ship_name(ship));
  sprintf(tbuf2, "%s (%s)", name, atr_parse_string(ship->objnum, "IDENT"));
  sprintf(tbuf1,
	  " %s%s/%s %-34s %s%s|%s %34s %s%s\\%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	  tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, atr_parse_string(ship->objnum, "CLASS"),
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s >--------%s %sWeapons Status %s------------%s%s+%s------------------------------------< %s",
   ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
  sprintf(tbuf1,
	  "%s%s|%s  %s-%sWeapon Type%s%s-           -%sRld%s%s-  -%sSpd%s%s-  -%sPow%s%s-  -%sDam%s%s-  -%sRng%s%s-    -%sStatus%s%s-    %s|%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
          ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
          ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	  ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN,
	  ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);

  for (n = 0; n < 2; n++)
  {
    if (!RealGoodObject(gun[n]->objnum))
    {
      sprintf(tbuf1, "%s%s|%s  %s                                                %s%s|%s",
           ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,  n ? "Secondary weapon offline." : "Primary weapon offline.  ", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    } else {
    
      /* individual weapon listings */
      switch(HasFlag(gun[n]->type, HS_ANY_WEAPON))
      {
      case HS_WEAPON:
      case HS_WIRETAP:
        if (gun[n]->loading)
        {
          sprintf(status, "Loading [%2d]", gun[n]->loading);
        }
        else
          sprintf(status, "   Loaded");
        
        sprintf(rld, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_RELOAD));
        strcpy(spd, "---");
        strcpy(pow, "---");
        sprintf(rng, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_RANGE));
        sprintf(dmg, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_DAMAGE));
        break;
      
      case HS_MISSILE:
        if (!gun[n]->loading)
          strcpy(status, "   Loaded");
        else
          sprintf(status, "Loading [%2d]", gun[n]->loading);

        sprintf(rld, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_RELOAD));
        sprintf(spd, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_SPEED));
        strcpy(pow, "---");
        sprintf(rng, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_RANGE));
        sprintf(dmg, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_DAMAGE));
        break;

      case HS_CANNON:
      case HS_EMITTER:
        sprintf(status, " %3.0f / %-3.0f", gun[n]->curpower, get_wstat(ship, con, gun[n]->type | HS_MAX_POWER));
        strcpy(rld, "---");
        strcpy(spd, "---");
        sprintf(pow, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_POWER));
        sprintf(rng, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_RANGE));
        sprintf(dmg, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_DAMAGE));
        break;

      case HS_CAPACITOR:
        sprintf(status, " %3.0f / %-3.0f", gun[n]->curpower, get_wstat(ship, con, gun[n]->type | HS_MAX_POWER));
        strcpy(rld, "---");
        strcpy(spd, "---");
        sprintf(pow, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_POWER));
        strcpy(rng, "---");
        sprintf(dmg, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_DAMAGE));
        break;

      case HS_BOOSTER:
        if (find_buff(ship, &STANCE_boost_weapons))
          strcpy(status, "  Boosting");
        else
          strcpy(status, "    Idle");
        strcpy(rld, "---");
        strcpy(spd, "---");
        strcpy(pow, "---");
        strcpy(rng, "---");
        strcpy(dmg, "---");
        break;

      default:
        if (!gun[n]->loading)
          strcpy(status, "   Loaded");
        else
          sprintf(status, "Loading [%2d]", gun[n]->loading);

        sprintf(rld, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_RELOAD));
        strcpy(spd, "---");
        strcpy(pow, "---");
        sprintf(rng, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_RANGE));
        sprintf(dmg, "%3.0f", get_wstat(ship, con, gun[n]->type | HS_DAMAGE));
        break;
      }
      sprintf(tbuf1,
              "%s%s|%s  %-22s   %3s    %3s    %3s    %3s    %3s   %-12s  %s%s|%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, 
              Name(gun[n]->objnum), rld, spd, pow, dmg, rng, status,
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
    notify(player, tbuf1);
  }

  /* show target information */
  if (lock && lock->contact)
  {
    cbuf = contact_colorstring(ship, lock);
    dship = ContactShip(lock);
    
    sprintf(tbuf1,
  	  "%s%s`-.-----------------------------------%s%s+%s-----------------------------------.-'%s",
     ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);

    sprintf(name, "%s", ship_name(dship));
    sprintf(tbuf2, "%s (%s)", name, atr_parse_string(ContactObj(lock), "IDENT"));
    sprintf(tbuf1,
            "%s%s /%s %-34s %s%s|%s %34s %s%s\\%s", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
            tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, atr_parse_string(ContactObj(lock), "CLASS"),
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);

    sprintf(tbuf1,
	    "%s%s >---------%s %sTarget Report %s------------%s%s+%s------------------------------------< %s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);

    incone = get_firing_cone(ship, con, dship);
    if (incone < 90)
    {
      sprintf(tbuf1,
            "%s%s|%s    %s+%s-%s %sYou %s-%s%s+               +%s-%s %sTarget %s[%s%d%s%s]%s -%s%s+       *%s -- In Cone -- %s%s*    %s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_RED, ANSI_NORMAL, lock->cnum, ANSI_HILITE, ANSI_RED,
            ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ANSI_HILITE,
            ANSI_BLUE, ANSI_NORMAL);
    } else {
      sprintf(tbuf1,
            "%s%s|%s    %s+%s-%s %sYou %s-%s%s+               +%s-%s %sTarget %s[%s%d%s%s]%s -%s%s+                            %s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            ANSI_HILITE, ANSI_RED, ANSI_NORMAL, lock->cnum, ANSI_HILITE, ANSI_RED,
            ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, 
            ANSI_BLUE, ANSI_NORMAL);
    }
    notify(player, tbuf1);

    sprintf(tbuf1,
            "%s%s|    %sHeading:%s %3.0fm%-3.0f        %s%sHeading:%s %3.0fm%-3.0f                               %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
            ship->xyhead, ship->zhead, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            dship->xyhead, dship->zhead, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);

    speed = ship->speed;
    if (HasFlag(ship->type, HS_AFTERBURNING))
      speed *= hs_options.burn_multiplier;
    
    dspeed = dship->speed;
    if (HasFlag(dship->type, HS_AFTERBURNING))
      dspeed *= hs_options.burn_multiplier;
      
    dist = dist3d(ship->x, ship->y, ship->z, dship->x, dship->y, dship->z);
    sprintf(tbuf1,
    "%s%s|    %sSpeed  :%s %5.0f km/s     %s%sSpeed  :%s %5.0f km/s       %s%sRange  :%s %5.0f Mm    %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
     speed, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, dspeed,
            ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, dist,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);

    armor = 100.0 * ship->hull.energy / get_stat(ship, SYS_MAX_ARMOR);
    tarmor = 100.0 * dship->hull.energy / get_stat(dship, SYS_MAX_ARMOR);
    sprintf(tbuf1,
         "%s%s|    %sArmor  :%s %5.0f%%         %s%sArmor  :%s %5.0f%%           %s%sBearing:%s %3dm%-3d     %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, armor, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
            tarmor, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, xyang(ship->x, ship->y, dship->x, dship->y),
            zang(ship->x, ship->y, ship->z, dship->x, dship->y, dship->z),
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
    
    shield = 100.0 * ship->shield.energy / get_stat(ship, SYS_MAX_CAPACITY);
    tshield = 100.0 * dship->shield.energy / get_stat(dship, SYS_MAX_CAPACITY);
    sprintf(tbuf1,
            "%s%s|    %sShield :%s %5.0f%%         %s%sShield :%s %5.0f%%                                %s%s|%s",
            ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, shield, ANSI_HILITE, ANSI_GREEN,
            ANSI_NORMAL, tshield, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf1);
  }

  sprintf(tbuf1,
	  "%s%s`---------------------------------------------------------------------------'%s",
	  ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  notify(player, tbuf1);
}


/* scan a sensor contact */
void sensor_scan(dbref console, char *which)
{
  ATTR   *a;
  hship  *tship = NULL;
  hship  *dship = NULL;
  hcelestial *cel = NULL;
  char    name[64];
  char    tbuf[256];
  char    tbuf2[256];
  char    ident[64];
  char    class[64];
  char    sdist[64];
  double  dist;
  hcontact *q, *qptr;
  int     cnum;
  dbref   player;
  int     xya;
  double  t;
  int     n, counter;
  int     za;
  char *buff, *r, *s;
  dbref pad;
  char *cbuf;
  int i, ismanned;
  dbref obj;
  int cash;
  
  double speed;

  tship = find_ship(console);
  if (!tship)
    return;

  player = get_user(console);
  if (player == NOTHING)
    return;

  cnum = rint(strtod(which, &s));
  if (!which || !*which || !s || *s)
  {
    q = tship->lock;
  } else {
    q = find_contact(tship, cnum);
  }
  
  if (!q || !q->contact)
  {
    hs_std_notice(console, "Please specify valid contact.");
    return;
  }

  cbuf = contact_colorstring(tship, q);
  
  if (HasFlag(q->type, HS_ANY_SHIP))
  {
    /* scan a ship */
    dship = ContactShip(q);
    dist = ship_distance(tship, dship);
    qptr = find_ship_contact(dship, tship);
    if (qptr)
    {
      if (HasFlag(qptr->flags, HS_IDENTIFIED))
      {
        hs_std_sensor(dship, qptr, tprintf("We are being scanned by the %s.", ship_name(tship)));
      }
      else
      {
        hs_std_sensor(dship, qptr, "We are being scanned by an unidentified contact.");
      }
    }
    else
    {
      notify_consoles(dship, tprintf("%s%s-%s We are being scanned by an unknown source.",
             ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
    }

    if (HasFlag(q->flags, HS_IDENTIFIED))
    {
      strncpy(class, atr_parse_string(dship->objnum, "CLASS"), 63);
      strncpy(ident, atr_parse_string(dship->objnum, "IDENT"), 63);
      strncpy(name, ship_name(dship), 63);
    }
    else
    {
      strcpy(name, "Unknown");
      strcpy(ident, "Unknown");
      strcpy(class, "Unknown");
    }

    sprintf(tbuf,
	    "  %s%s.-------------------------------------------------------.%s", ANSI_HILITE,
	    ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf2, "%s (%s)", name, ident);
    sprintf(tbuf,
	    " %s%s/ %sContact: %s%s[%s%s%d%s%s]%s %-39s %s%s\\%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	    cbuf, ANSI_NORMAL, ANSI_HILITE, q->cnum, ANSI_NORMAL, cbuf, ANSI_NORMAL,
	    tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
	    "%s%s|%s                  %-40s %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	    class, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
      "%s%s|                                                           |%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
	    "%s%s|%s %s+%s-%s %sNavigational Report %s-%s%s+                                 %s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    xya = xyang(tship->x, tship->y, dship->x, dship->y);
    za = zang(tship->x, tship->y, tship->z, dship->x, dship->y, dship->z);
    sprintf(tbuf,
	    "%s%s| %sX:%s %10.0f  %s%sBearing:%s %3dm%-3d     %s%sRange:%s %6.0f         %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
       dship->x, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, xya, za, ANSI_HILITE,
       ANSI_GREEN, ANSI_NORMAL, dist, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    
    speed = dship->speed;
    if (HasFlag(dship->type, HS_AFTERBURNING))
      speed *= hs_options.burn_multiplier;
    
    sprintf(tbuf,
	    "%s%s| %sY:%s %10.0f  %s%sHeading:%s %3.0fm%-3.0f     %s%sSpeed:%s %6.0f hph     %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	    dship->y, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, dship->xyhead,
            dship->zhead, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, 
            speed, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
	    "%s%s| %sZ:%s %10.0f                                             %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	    dship->z, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    if (HasFlag(q->flags, HS_IDENTIFIED))
    {
      sprintf(tbuf,
      "%s%s|                                                           |%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      sprintf(tbuf,
	      "%s%s|%s %s+%s-%s %sSystems Assessment %s-%s%s+                                  %s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	      ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      sprintf(tbuf,
	      "%s%s| %sHull           :%s %7.0f%%    %s%sShields        :%s %7.0f%%    %s%s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	      dship->hull.energy / get_stat(dship, SYS_MAX_ARMOR) * 100.0, ANSI_HILITE, ANSI_GREEN,
	      ANSI_NORMAL, dship->shield.energy / get_stat(dship, SYS_MAX_CAPACITY) * 100.0, ANSI_HILITE,
	      ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      sprintf(tbuf,
	      "%s%s| %sReactor        :%s %7.0f%%    %s%sEngines        :%s %7.0f%%    %s%s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	      dship->reactor.energy / get_stat(dship, SYS_MAX_ENERGY) * 100.0, ANSI_HILITE, ANSI_GREEN,
              ANSI_NORMAL, dship->engine.energy / get_stat(dship, SYS_MAX_HEAT) * 100.0, ANSI_HILITE,
	      ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      sprintf(tbuf,
	      "%s%s| %sSensors        :%s %7.0f%%    %s%sComputer       :%s %7.0f%%    %s%s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
              dship->sensor.energy / get_stat(dship, SYS_MAX_FOCUS) * 100.0, ANSI_HILITE, ANSI_GREEN,
              ANSI_NORMAL, dship->computer.energy / get_stat(dship, SYS_MAX_MEMORY) * 100.0, ANSI_HILITE,
	      ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      
      
      if (RealGoodObject(dship->primary.objnum))
      {
        strncpy(name, Name(dship->primary.objnum), 15);
        if (dship->primary.maxpower > 0)
          sprintf(ident, "%3.0f / %-3.0f ", dship->primary.curpower, get_wstat(dship, NULL, HS_PRIMARY | HS_MAX_POWER));
        else
        {
          if (dship->primary.loading)
            sprintf(ident, "Load [%02ds]", dship->primary.loading);
          else
            strcpy(ident, "  Loaded  ");
        }
      } else {
        strcpy(name, "   <Primary>   ");
        strcpy(ident, "Offline");
      }

      if (RealGoodObject(dship->secondary.objnum))
      {
        strncpy(class, Name(dship->secondary.objnum), 15);
        if (dship->secondary.maxpower > 0)
          sprintf(sdist, "%3.0f / %-3.0f ", dship->secondary.curpower, get_wstat(dship, NULL, HS_SECONDARY | HS_MAX_POWER));
        else
        {
          if (dship->secondary.loading)
            sprintf(sdist, "Load [%02ds]", dship->secondary.loading);
          else
            strcpy(sdist, "  Loaded  ");
        }
      } else {
        strcpy(class, "  <Secondary>  ");
        strcpy(sdist, "Offline");
      }

      
      sprintf(tbuf,
	      "%s%s| %s%-15s:%s %10s  %s%s%-15s:%s %10s  %s%s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, ANSI_NORMAL,
              ident, ANSI_HILITE, ANSI_GREEN, class,
              ANSI_NORMAL, sdist, ANSI_HILITE,
	      ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      
      if (dship->nconsoles > 0)
      {
        sprintf(tbuf,
        "%s%s|                                                           |%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
        notify(player, tbuf);
      }
      
      /* now do the console weapons */
      for (i = 0; i < dship->nconsoles; i++)
      {
        if (!RealGoodObject(dship->console[i].objnum))
          continue;
          
        ismanned = RealGoodObject(get_user(dship->console[i].objnum));
        sprintf(name, "%s (%s)", Name(dship->console[i].objnum), ismanned ? "Manned" : "Unmanned"); 
        sprintf(tbuf,
                "%s%s|%s  %s%-55s  %s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, name, ANSI_BLUE, ANSI_NORMAL);
        notify(player, tbuf);
        
        /* if (!ismanned) continue; */
               
        if (RealGoodObject(dship->console[i].primary.objnum))
        {
          strncpy(name, Name(dship->console[i].primary.objnum), 63);
          if (dship->console[i].primary.maxpower > 0)
            sprintf(ident, "%3.0f / %-3.0f ", dship->console[i].primary.curpower, get_wstat(dship, &(dship->console[i]), HS_PRIMARY | HS_MAX_POWER));
          else
          {
            if (dship->console[i].primary.loading)
              sprintf(ident, "Load [%02ds]", dship->console[i].primary.loading);
            else
              strcpy(ident, "  Loaded  ");
          }
        } else {
          strcpy(name, "   <Primary>   ");
          strcpy(ident, "Offline");
        }

        if (RealGoodObject(dship->console[i].secondary.objnum))
        {
          strncpy(class, Name(dship->console[i].secondary.objnum), 63);
          if (dship->console[i].secondary.maxpower > 0)
            sprintf(sdist, "%3.0f / %-3.0f ", dship->console[i].secondary.curpower, get_wstat(dship, &(dship->console[i]), HS_SECONDARY | HS_MAX_POWER));
          else
          {
            if (dship->console[i].secondary.loading)
              sprintf(sdist, "Load [%02ds]", dship->console[i].secondary.loading);
            else
              strcpy(sdist, "  Loaded  ");
          }
        } else {
          strcpy(class, "  <Secondary>  ");
          strcpy(sdist, "Offline");
        }

        
        sprintf(tbuf,
                "%s%s| %s%-15s:%s %10s  %s%s%-15s:%s %10s  %s%s|%s",
                ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, name, ANSI_NORMAL,
                ident, ANSI_HILITE, ANSI_GREEN, class,
                ANSI_NORMAL, sdist, ANSI_HILITE,
                ANSI_BLUE, ANSI_NORMAL);
        notify(player, tbuf);
      }
    }
    sprintf(tbuf,
       "%s%s`-----------------------------------------------------------'%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);

  }
  else if (HasFlag(q->type, HS_ANY_CELESTIAL))
  {
    /* scan a celestial contact */
    cel = ContactCelestial(q);
    xya = xyang(tship->x, tship->y, cel->x, cel->y);
    if (xya == -1)
      xya = 0;
    za = zang(tship->x, tship->y, tship->z, cel->x, cel->y, cel->z);

    dist = ship_celestial_distance(tship, cel);

    sprintf(sdist, "%.0f Mm", dist);
    if (HasFlag(q->flags, HS_IDENTIFIED))
    {
      if (IsShip(cel->objnum) || IsDrone(cel->objnum))
      {
        strncpy(class, atr_parse_string(cel->objnum, "CLASS"), 63);
        strncpy(ident, atr_parse_string(cel->objnum, "IDENT"), 63);
      }
      else if (HasFlag(cel->type, HS_DEBRIS))
      {
        if (cel->contents)
        {
          strcpy(ident, "Full");
          strcpy(class, "");
        } else {
          strcpy(class, "");
          strcpy(ident, "Empty");
        }
      }
      else if (HasFlag(cel->type, HS_WAYPOINT))
      {
        strncpy(class, STR(hs_object_types, HasFlag(cel->type, HS_OBJECT_FLAGS)), 63);
        if (RealGoodObject(cel->objnum))
          strncpy(ident, atr_parse_string(cel->objnum, "IDENT"), 63);
        else
          strncpy(ident, "internal", 63);
          
      }
      else
      {
        strncpy(class, STR(hs_object_types, HasFlag(cel->type, HS_OBJECT_FLAGS)), 63);
        strncpy(ident, atr_parse_string(cel->objnum, "IDENT"), 63);
      }
      
      strncpy(name, celestial_name(cel), 63);
    }
    else
    {
      strcpy(name, "Unknown");
      strcpy(ident, "Unknown");
      strcpy(class, "Unknown");
    }

    sprintf(tbuf,
	    "  %s%s.-------------------------------------------------------.%s", ANSI_HILITE,
	    ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf2, "%s (%s)", name, ident);
    sprintf(tbuf,
	    " %s%s/ %sContact: %s%s[%s%s%d%s%s]%s %-39s %s%s\\%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, 
	    cbuf, ANSI_NORMAL, ANSI_HILITE, q->cnum, ANSI_NORMAL, cbuf, ANSI_NORMAL,
	    tbuf2, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
	    "%s%s|%s                  %-40s %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
	    class, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
      "%s%s|                                                           |%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
	    "%s%s|%s %s+%s-%s %sNavigational Report %s-%s%s+                                 %s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);

    sprintf(tbuf,
	    "%s%s| %sX:%s %10.0f  %s%sBearing:%s %3dm%-3d     %s%sRange:%s %6.0f         %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
       cel->x, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, xya, za, ANSI_HILITE,
       ANSI_GREEN, ANSI_NORMAL, dist, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
	    "%s%s| %sY:%s %10.0f  %s%s   Mass:%s %7.0f    %s%sRadius:%s %6.0f         %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL, cel->y, ANSI_HILITE, ANSI_GREEN,
	    ANSI_NORMAL, cel->mass, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, cel->radius,
            ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    
    if (cel->contents)
    {
      obj = parse_dbref(cel->contents);
      cash = parse_integer(cel->contents);
      if (IsComponent(obj) || IsWeapon(obj))
      {
        sprintf(tbuf,
  	      "%s%s| %sZ:%s %10.0f  %s%sSalvage:%s %-32s  %s%s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	      cel->z, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, Name(obj), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      }
      else if (cash > 0)
      {
        sprintf(tbuf,
  	      "%s%s| %sZ:%s %10.0f  %s%sSalvage:%s %10dc                       %s%s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	      cel->z, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, cash, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      }
      else
      {
        sprintf(tbuf,
  	      "%s%s| %sZ:%s %10.0f  %s%sSalvage:%s %-32s  %s%s|%s",
	      ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	      cel->z, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, cel->contents, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      }
    }
    else
    {
      sprintf(tbuf,
	    "%s%s| %sZ:%s %10.0f                                             %s%s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, ANSI_NORMAL,
	    cel->z, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
    
    notify(player, tbuf);

    if (!HasFlag(q->flags, HS_IDENTIFIED))
    {
      sprintf(tbuf,
         "%s%s`-----------------------------------------------------------'%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      return;
    }

    counter = 0;
    a = atr_get(cel->objnum, "DROPPADS");
    if (!a)
    {
      //notify(player, tprintf("%s%s|  %s*%s No landing locations                                   %s%s|%s",
      //   ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL));
      sprintf(tbuf,
         "%s%s`-----------------------------------------------------------'%s",
              ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
      notify(player, tbuf);
      return;
    }

    sprintf(tbuf,
         "%s%s|                                                           |%s",
         ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
    sprintf(tbuf,
	    "%s%s|%s %s+%s-%s %sLanding Pad Report %s-%s%s+                                  %s|%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	    ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);

    buff = safe_atr_value(a, "DROPPADS");
    s = buff;
    n = 0;
    r = split_token(&s, ' ');
    pad = parse_dbref(r);
    if (RealGoodObject(pad))
    {
      notify(player, tprintf("%s%s|  %s[%s%s%02d%s]%s - %-48s  %s%s|%s",
           ANSI_HILITE, ANSI_BLUE, ANSI_YELLOW, ANSI_NORMAL, ANSI_HILITE,
           n, ANSI_YELLOW, ANSI_NORMAL, Name(pad), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL));
    }

    while (s)
    {
      r = split_token(&s, ' ');
      pad = parse_dbref(r);
      if (RealGoodObject(pad))
      {
        n++;
        notify(player, tprintf("  %s%s[%s%s%-2d%s]%s - %s",
             ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ANSI_HILITE, n, ANSI_YELLOW, ANSI_NORMAL, Name(pad)));
      }
    }
    
    free(buff);

    sprintf(tbuf,
       "%s%s`-----------------------------------------------------------'%s",
	    ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    notify(player, tbuf);
  } else {
    hs_std_notice(console, "There is something wrong with this sensor contact. Please contact your flight mechanic immediately!");
  }
}


