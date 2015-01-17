
#include "hspace.h"

/***************************************************/
/* console commands */

COMMAND(cmd_com)
{
  if (!IsComm(executor))
  {
    notify(executor, "Your radio is not powered on. RADIO ON to activate it.");
    return;
  }
  
  send_com(executor, arg_left, arg_right);
}

COMMAND(cmd_console)
{
  if (!IsConsole(executor) && !IsShip(executor)) {
    notify(executor, "Permission denied.");
    return;
  }
  
  if (SW_ISSET(sw, SWITCH_ABORT))
  {
    set_afterburner(executor, 0);
  }
  else if (SW_ISSET(sw, SWITCH_BURN))
  {
    set_afterburner(executor, 1);
  }
  else if (SW_ISSET(sw, SWITCH_CLOSEBAY))
  {
    set_baydoors(executor, 0);
  }
  else if (SW_ISSET(sw, SWITCH_COMBATSTAT))
  {
    combat_status(executor);
  }
  else if (SW_ISSET(sw, SWITCH_COOLDOWN))
  {
    use_cooldown(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_DOCK))
  {
    dock_ship(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_DUMP))
  {
    dump_cargo(executor, arg_left, arg_right);
  }
  else if (SW_ISSET(sw, SWITCH_ESTATUS))
  {
    effect_status(executor);
  }
  else if (SW_ISSET(sw, SWITCH_ETA))
  {
    show_eta(executor);
  }
  else if (SW_ISSET(sw, SWITCH_EXTEND))
  {
    set_boardinglink(executor, 1);
  }
  else if (SW_ISSET(sw, SWITCH_FIRE))
  {
    //use_slot(executor, arg_left, HS_PRIMARY, 0);
    //use_slot(executor, arg_left, HS_SECONDARY, 0);
    use_both_slots(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_GATE))
  {
    use_wormhole(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_HACK))
  {
    use_hack(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_HEADING))
  {
    set_heading(executor, arg_left, arg_right);
  }
  else if (SW_ISSET(sw, SWITCH_LAND))
  {
    land_ship(executor, arg_left, arg_right);
  }
  else if (SW_ISSET(sw, SWITCH_LAUNCH))
  {
    launch_ship(executor);
  }
  else if (SW_ISSET(sw, SWITCH_LINK))
  {
    link_ship(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_LOCK))
  {
    set_lock(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_MANIFEST))
  {
    cargo_manifest(executor);
  }
  else if (SW_ISSET(sw, SWITCH_NAVMODE))
  {
    set_navmode(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_OPENBAY))
  {
    set_baydoors(executor, 1);
  }
  else if (SW_ISSET(sw, SWITCH_PRIMARY))
  {
    use_slot(executor, arg_left, HS_PRIMARY, 1);
  }
  else if (SW_ISSET(sw, SWITCH_PROMPT))
  {
    set_prompt(executor);
  }
  else if (SW_ISSET(sw, SWITCH_PVPOFF))
  {
    set_pvp(executor, 0);
  }
  else if (SW_ISSET(sw, SWITCH_PVPON))
  {
    set_pvp(executor, 1);
  }
  else if (SW_ISSET(sw, SWITCH_QUICKSTAT))
  {
    quick_status(executor);
  }
  else if (SW_ISSET(sw, SWITCH_RETRACT))
  {
    set_boardinglink(executor, 0);
  }
  else if (SW_ISSET(sw, SWITCH_SCAN))
  {
    sensor_scan(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_SECONDARY))
  {
    use_slot(executor, arg_left, HS_SECONDARY, 1);
  }
  else if (SW_ISSET(sw, SWITCH_SHUNT))
  {
    use_shunt(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_SPEED))
  {
    set_speed(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_SREP))
  {
    sensor_report(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_STANCE))
  {
    switch_stance(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_STATUS))
  {
    ship_status(executor);
  }
  else if (SW_ISSET(sw, SWITCH_SYSSTAT))
  {
    system_status(executor);
  }
  else if (SW_ISSET(sw, SWITCH_TAXI))
  {
    do_taxi(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_TRACTOR))
  {
    tractor_cargo(executor, arg_left);
  }
  else if (SW_ISSET(sw, SWITCH_UNLINK))
  {
    unlink_ship(executor);
  }
  else if (SW_ISSET(sw, SWITCH_VIEW))
  {
    do_view(executor);
  }
  else if (SW_ISSET(sw, SWITCH_WAYPOINT))
  {
    set_waypoint(executor, arg_left);
  } else {
    notify(executor, "Invalid command.");
  }

  /* send a prompt after console commands */
  hs_prompt(executor);
}

COMMAND(cmd_man)
{
  man_console(executor, arg_left);
}

COMMAND(cmd_unman)
{
  unman_console(executor);
}

COMMAND(cmd_board)
{
  board_ship(executor, arg_left, arg_right);
}

COMMAND(cmd_disembark)
{
  disembark(executor);
}

COMMAND(cmd_eject)
{
  emergency_eject(executor);
}

COMMAND(cmd_space)
{  
  if (SW_ISSET(sw, SWITCH_VERSION))
    hs_version(executor);
  else if (SW_ISSET(sw, SWITCH_DUMP))
    hs_dump();
  else if (SW_ISSET(sw, SWITCH_LOAD))
    load_space_object(executor, arg_left);
  else if (SW_ISSET(sw, SWITCH_LIST))
    hs_list(executor, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_MOVE))
    hs_move(executor, arg_left, arg_right);
  else if (SW_ISSET(sw, SWITCH_RECONFIGURE))
    hs_load_config(executor);
  else if (SW_ISSET(sw, SWITCH_SPAWN))
    hs_spawn(executor, arg_left);
  else if (SW_ISSET(sw, SWITCH_START))
    set_cycle(executor, 1);
  else if (SW_ISSET(sw, SWITCH_HALT))
    set_cycle(executor, 0);
  else
    hs_status(executor);

  return;

}

/*******************************************************/
/* command routines */

void set_prompt(dbref console)
{
  hship *ship;
  hconsole *con;
  char *prompt;
  dbref player;

  if (IsShip(console))
  {
    con = NULL;
    ship = find_ship_by_nav(console);
    prompt = &(ship->prompt);
  }
  else if (IsConsole(console))
  {
    con = find_console(console);
    if (!con)
    {
      notify_console(console, "Unable to find console! Contact your local flight mechanic immediately!");
      return;
    }
    
    ship = find_ship(console);
    prompt = &(con->prompt);
  }
  else
  {
    ship = NULL;
    con = NULL;
  }
  
  if (!ship || !prompt)
  {
    notify_console(console, "Unable to find ship! Contact your local flight mechanic immediately!");
    return;
  }

  player = get_user(console);
  if (!IsPlayer(player))
  {
    notify_console(console, "Only players may receive prompts.");
    return;
  }

  *prompt = HasFlag(atr_parse_integer(player, "HSPROMPT_FREQUENCY", 1), HS_PROMPT_FREQUENCY);
  FlagOn(*prompt, atr_parse_flags(player, hs_prompt_flags, "HSPROMPT_FLAGS"));
  
  hs_std_notice(console, "Prompt flags reloaded.");
  return;
}

/* man console command */
void man_console(dbref player, char *where)
{
  hship *ship;
  hconsole *con;
  dbref   console;
  dbref   manner, manning;
  hweapon *pri, *sec;
  char *prompt;
  int ptype, stype, type;

  if (Typeof(player) != TYPE_PLAYER && hs_options.forbid_puppets)
  {
    notify(player, "Only players may man this console.");
    return;
  }
  
  console = match_result(player, where, TYPE_THING, MAT_NEAR_THINGS);

  switch (console)
  {
  case NOTHING:
    notify(player, "I don't see that here.");
    return;

  case AMBIGUOUS:
    notify(player, "I don't know which you mean!");
    return;
  }

  /* don't need to do this now because we want to allow you
     to reman consoles easily for equipment changes */

  /*manner = get_user(console);
  if (manner == player)
  {
    notify(player, "You are already manning that console.");
    return;
  }*/

  if (!IsConsole(console) && !IsShip(console))
  {
    notify(player, "You cannot man that.");
    return;
  }
  
  if (IsConsole(console))
  {
    con = find_console(console);
    if (!con)
    {
      notify(player, "That console has not been activated.");
      return;
    }
    
    pri = &(con->primary);
    sec = &(con->secondary);
    prompt = &(con->prompt);
    
    ship = find_ship(console);
  }
  else if (IsShip(console))
  {
    con = NULL;
    ship = find_ship_by_nav(console);
    
    if (ship)
    {
      pri = &(ship->primary);
      sec = &(ship->secondary);
      prompt = &(ship->prompt);
    }
    
  } else{
    ship = NULL;
  }
  
  if (!ship)
  {
    notify(player, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
    

  if (InCombat(ship))
  {
    notify(player, "Can't reload weapons in combat!");
    stype = sec->type;
    ptype = pri->type;
  } else {
    ptype = load_weapon(player, pri, HS_PRIMARY);
    stype = load_weapon(player, sec, HS_SECONDARY);
  }
  
  type = HasFlag(ptype | stype, HS_ANY_WEAPON);

  if (con)
  {
    /* this is an auxiliary console */
    /* no missiles on the aux consoles */
    if (HasFlag(type, HS_MISSILE))
    {
      clear_weapon(pri);
      clear_weapon(sec);
      
      notify(player, "You may not man an auxiliary console with missiles equipped.");
      return;
    }
    
    if (HasFlag(type, HS_WIRETAP))
      con->type = HS_OPS;
    else if (HasFlag(type, HS_CAPACITOR))
      con->type = HS_ENG;
    else if (HasFlag(type, HS_CANNON | HS_EMITTER | HS_WEAPON))
      con->type = HS_GUN;
    else
      con->type = HS_CIV;
    
  } else {
    /* this is a nav console */
    /* no cans, caps or taps */
    
    if (HasFlag(type, HS_CANNON))
    {
      clear_weapon(pri);
      clear_weapon(sec);
      
      notify(player, "You may not man a navigation console with a cannon equipped.");
      return;
    }
  }
  
  /* the equipment slots check out, and the console
     has been set to the appropriate type, if needed */

  if (prompt)
  {
    *prompt = HasFlag(atr_parse_integer(player, "HSPROMPT_FREQUENCY", 1), HS_PROMPT_FREQUENCY);
    FlagOn(*prompt, atr_parse_flags(player, hs_prompt_flags, "HSPROMPT_FLAGS"));
  }
  
  manning = get_console(player);
  if (manning != NOTHING)
  {
    manner = get_user(manning);
    if (manner == player)
    {
      con = find_console(manning);
      if (con)
      {
        load_weapon(manning, &(con->primary), HS_PRIMARY);
        load_weapon(manning, &(con->secondary), HS_SECONDARY);
      }
      
      set_user(manning, NOTHING);
      
      notify(player, tprintf("You unman the %s.", Name(manning)));
      notify_except(manning, Location(manning), player,
              tprintf("%s unmans the %s.", Name(player), Name(manning)), 0);
    }
  }
  
  /* set the HSPACE attribute in case something was fishy */
  if (!IsSim(ship->objnum))
  {
    atr_add(player, "HSPACE", unparse_dbref(ship->objnum), hs_options.space_wiz, 0);
  }

  set_user(console, player);

  execute_trigger(console, "AMAN", ship);

  if (con)
  {
    notify(player, tprintf("You man the %s (%s%s%s).", Name(console),
          ANSI_HILITE, STR(hs_console_types, con->type), ANSI_NORMAL));
    notify_except(console, Location(console), player,
    	  tprintf("%s mans the %s (%s%s%s).", Name(player), Name(console),
    	  ANSI_HILITE, STR(hs_console_types, con->type), ANSI_NORMAL), 0);
  } else {
    notify(player, tprintf("You man the %s.", Name(console)));
    notify_except(console, Location(console), player,
    	  tprintf("%s mans the %s.", Name(player), Name(console)), 0);
  }
}

/* unman console command */
void unman_console(dbref player)
{
  dbref curman, console;
  hship *ship;
  hconsole *con;

  console = atr_parse_dbref(player, "MANNING");
  if (!IsConsole(console) && !IsShip(console))
  {
    notify(player, "You are not manning a console.");
    return;
  }
  
  curman = get_user(console);
  
  if (curman != player)
  {
    notify(player, "You aren't manning that.");
    return;
  }
  
  ship = find_ship_by_nav(console);
  if (!ship)
  {
    con = find_console(console);
    if (!con)
    {
      notify(player, "That console has not been activated.");
      return;
    }
    
    load_weapon(console, &(con->primary), HS_PRIMARY);
    load_weapon(console, &(con->secondary), HS_SECONDARY);
  } else {
    load_weapon(console, &(ship->primary), HS_PRIMARY);
    load_weapon(console, &(ship->secondary), HS_SECONDARY);
  }

  set_user(console, NOTHING);
  notify(player, tprintf("You unman the %s.", Name(console)));
  notify_except(console, Location(console), player,
             tprintf("%s unmans the %s.", Name(player), Name(console)), 0);

  execute_trigger(console, "AUNMAN", ship);
}

/* board a ship through the ship object */
void board_ship(dbref player, char *which, char *code)
{
  ATTR *a;
  char *bcode;
  dbref obj, nav, bay;
  int security;
  
  /* find the shipobj */
  obj = match_result(player, which, TYPE_THING, MAT_NEAR_THINGS);
  if (!IsShipObj(obj))
  {
    notify(player, "Can't find that ship.");
    return;
  }
  
  /* see if it's got an HSNAV */
  nav = atr_parse_dbref(obj, "HSNAV");
  if (!IsShip(nav))
  {
    notify(player, "That is not a valid ship object.");
    return;
  }
  
  /* check the boarding code, if necessary */
  security = atr_parse_integer(nav, "SECURITY", 0);
  if (security)
  {
    a = atr_get(nav, "BOARDING_CODE");
    if (a)
    {
      bcode = atr_value(a);
      if (strcmp(bcode, code))
      {
        notify(player, "Invalid boarding code.");
        return;
      }
    }
  }
  
  /* check for a BAY on the nav, otherwise go to the nav's location */
  bay = atr_parse_dbref(nav, "BAY");
  if (!RealGoodObject(bay))
  {
    bay = Location(nav);
  }
  
  atr_add(player, "HSPACE", unparse_dbref(nav), hs_options.space_wiz, 0);
  notify_except(obj, Location(player), player, tprintf("%s boards the %s.", Name(player), Name(obj)), 0);
  notify_except(obj, bay, player, tprintf("%s boards through the main hatch.", Name(player)), 0);
  moveto(player, bay, hs_options.space_wiz, NULL);
}

/* leave a landed/docked ship through the hatch */
void disembark(dbref player)
{
  dbref nav, obj, newobj;
  hship *ship;
  ATTR *a;
  int security;
  
  /* check if we can disembark from here */
  a = atr_get(Location(player), "BAY");
  if (!a)
  {
    /* no BAY, check if we're near the nav console */
    ship = find_ship(player);
    if (ship)
    {
      if (Location(ship->objnum) != Location(player))
      {
        notify(player, "You can't disembark from here.");
        return;
      }
    }
  }
  else
  {
    /* there's a BAY here, make sure it's a good one */
    nav = parse_dbref(atr_value(a));
    if (!IsShip(nav))
    {
      notify(player, "You can't disembark from here.");
      return;
    }
  
    ship = find_ship_by_nav(nav);
  }
  
  if (!ship)
  {
    notify(player, "You can't disembark from here.");
    return;
  }
  
  /* no ditching in space, or early after launching, or prematurely when landing */
  if ((ship->uid || ship->landing || ship->launching) && !ship->linked)
  {
    notify(player, "You can't disembark while in space.");
    return;
  }
  
  obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
  if (!RealGoodObject(obj))
  {
    notify(player, "This ship can not be disembarked.");
    return;
  }
  
  /* check whether we're docking or landing, save the new space object */
  if (ship->landed)
  {
    newobj = ship->landed->objnum;
  }
  else if (ship->docked)
  {
    newobj = ship->docked->objnum;
  }
  else if (ship->linked)
  {
    newobj = ship->linked->objnum;
  } else {
    notify(player, "You can't disembark while in space.");
    return;
  }
  
  if (ship->linked)
  {
    /* check the boarding code, if necessary */
    security = atr_parse_integer(ship->linked->objnum, "SECURITY", 0);
    if (security)
    {
      notify(player, "Unable to use boarding link while the other ship has security enabled.");
      return;
    }
    
    security = atr_parse_integer(ship->objnum, "SECURITY", 0);
    if (security)
    {
      notify(player, "Unable to use boarding link while security is enabled.");
      return;
    }
    
    obj = atr_parse_dbref(ship->linked->objnum, "BAY");
    if (!RealGoodObject(obj))
      obj = Location(ship->linked->objnum);
      
    moveto(player, obj, hs_options.space_wiz, NULL);
  } else {
    moveto(player, Location(obj), hs_options.space_wiz, NULL);
  }
  
  /* finish up by setting HSPACE attribute and notifying everybody about the move */
  atr_add(player, "HSPACE", unparse_dbref(newobj), hs_options.space_wiz, 0);
  notify_except(obj, Location(player), player, tprintf("%s disembarks through the main hatch.", Name(player)), 0);
  notify_except(obj, Location(obj), player, tprintf("%s disembarks from the %s.", Name(player), Name(obj)), 0);
  
  return;
}

/* leave a flying ship through the escape pod */
void emergency_eject(dbref player)
{
  dbref nav, pad;
  hship *ship;
  hcelestial *cel, *min_cel;
  ATTR *a;
  char *r, *s;
  char buff[512];
  double dist, min_dist;
  dbref min_pad;
  hship *sptr, *min_ship;
  
  /* check for a BAY */
  a = atr_get(Location(player), "BAY");
  if (!a)
  {
    /* no BAY, see if we're next to the nav console */
    ship = find_ship(player);
    if (ship)
    {
      if (Location(ship->objnum) != Location(player))
      {
        notify(player, "You can't eject from here.");
        return;
      }
    }
  }
  else
  {
    /* there's a BAY, see if the ship is valid */
    nav = parse_dbref(atr_value(a));
    if (!IsShip(nav))
    {
      notify(player, "You can't eject from here.");
      return;
    }

    ship = find_ship_by_nav(nav);
  }
  
  if (!ship)
  {
    notify(player, "You can't eject from here.");
    return;
  }

  /* only eject when flying, not when landing or docking */
  if (!ship->uid || ship->landed || ship->docked)
  {
    notify(player, "You may only eject while flying.");
    return;
  }
  
  /* find a planet with a drop pad */
  min_pad = NOTHING;
  min_dist = 1000000.0;
  min_cel = NULL;
  for (cel = ship->uid->head_celestial; cel; cel = cel->next)
  {
    if (!HasFlag(cel->type, HS_PLANET))
      continue;
    
    pad = atr_parse_dbref(cel->objnum, "DROPPADS");
    if (!RealGoodObject(pad))
      continue;

    dist = ship_celestial_distance(ship, cel);

    if (dist < min_dist)
    {
      min_dist = dist;
      min_pad = pad;
      min_cel = cel;
    }
  }
  
  min_ship = NULL;
  for (sptr = ship->uid->head_ship; sptr; sptr = sptr->next)
  {
    if (min_cel)
      break;
    
    if (!HasFlag(sptr->type, HS_STATION | HS_CAPITAL))
      continue;
    
    pad = atr_parse_dbref(sptr->objnum, "BAY");
    if (!RealGoodObject(pad))
      continue;
    
    dist = ship_distance(ship, sptr);

    if (dist < min_dist)
    {
      min_cel = NULL;
      min_ship = sptr;
      min_dist = dist;
      min_pad = pad;
    }
  }
  
  if (!RealGoodObject(min_pad))
  {
    notify(player, "There is nowhere to eject to!");
    return;
  }

  /* finish up by setting HSPACE and notifying everybody of the move */
  if (min_ship)
  {
    atr_add(player, "HSPACE", unparse_dbref(min_ship->objnum), hs_options.space_wiz, 0);
  }
  else if (min_cel)
  {
    atr_add(player, "HSPACE", unparse_dbref(min_cel->objnum), hs_options.space_wiz, 0);
  }
  else
  {
    SPACEWALL("Weird problem in eject.");
    notify(player, "Bad space object. Contact an administrator.");
    return;
  }

  notify_except(Location(player), Location(player), player, tprintf("%s ejects in an emergency escape pod!", Name(player)), 0);
  notify_except(min_pad, min_pad, player, tprintf("%s crash lands in an emergency escape pod!", Name(player)), 0);
  if (min_ship)
  {
    notify_consoles(min_ship, tprintf("%s%s-%s Emergency ejection pod automatically tractored into the docking back.",
          ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
  }
  moveto(player, min_pad, hs_options.space_wiz, NULL);
}


/* spawn a single drone */
void hs_spawn(dbref player, char *which)
{
  dbref obj;
  hship *ship;
  
  obj = match_result(player, which, TYPE_THING, MAT_NEAR_THINGS);
  if (!RealGoodObject(obj))
  {
    notify(player, "HSPACE: Unable to find that drone template.");
    return;
  }
  
  if (!IsDrone(obj))
  {
    notify(player, "HSPACE: That is not a valid drone template.");
    return;
  }
  
  ship = create_drone(obj);
  if (!ship)
  {
    notify(player, "HSPACE: Failed to create the drone.");
    return;
  }
  
  notify_format(player, "HSPACE: Drone created at %s(#%d) %.0f %.0f %.0f.", Name(ship->uid->objnum), ship->uid->objnum, ship->x, ship->y, ship->z);
  return;
}

/* list various hspace objects */
void hs_list(dbref executor, char *arg_left, char *arg_right)
{
  huniverse *uid;
  hcelestial *cel;
  hship *ship;
  int i;
  
  dbref obj = NOTHING;
  
  /* list various informations */
  if (!arg_left || !*arg_left) {
  
    notify(executor, "HSPACE: You must specify one of: configuration, universes, celestials, ships");

  /* list configuration info */
  } else if (!strncasecmp("CONFIGURATION", arg_left, strlen(arg_left))) {

    hs_print_config(executor, arg_right);

  /* list universe details */
  } else if (!strncasecmp("UNIVERSES", arg_left, strlen(arg_left))) {

    notify(executor, "HSPACE: Listing universes...");
    for (i = 0; i < hs_num_universes; i++)
    {
      uid = &hs_universes[i];
      notify_format(executor, "%-3d: %s(#%d)   %d celestials, %d ships, %d drones", i, Name(uid->objnum), uid->objnum, uid->num_celestials, uid->num_ships, uid->num_drones);
    }

  /* list celestial details */
  } else if (!strncasecmp("CELESTIALS", arg_left, strlen(arg_left))) {

    notify(executor, "HSPACE: Listing celestials...");
    for (i = 0; i < hs_num_celestials; i++)
    {
      cel = &hs_celestials[i];
      if (cel->uid)
      {
        notify_format(executor, "%-3d: %s(#%d) %d flags, [%s(#%d)] %.0f,%.0f,%.0f", i, Name(cel->objnum), 
                      cel->objnum, cel->type, Name(cel->uid->objnum), cel->uid->objnum,
                      cel->x, cel->y, cel->z);
      }
      else
        notify(executor, "HSPACE: Error! Invalid universe.");
    }

  /* list ship details */
  } else if (!strncasecmp("SHIPS", arg_left, strlen(arg_left))) {

    notify(executor, "HSPACE: Listing ships...");
    for (i = 0; i < hs_num_ships; i++)
    {
      ship = &hs_ships[i];
      if (ship->uid)
        obj = ship->uid->objnum;
      else if (ship->landed)
        obj = ship->landed->objnum;
      else if (ship->docked)
        obj = atr_parse_dbref(ship->docked->objnum, "SHIPOBJ");
      
      notify_format(executor, "%-3d: %s(#%d) [%s(#%d)] (%9.0f %9.0f %9.0f) %.0fm%.0f (%.0fm%.0f) @ %.0f (%.0f) Mm/s", i, 
                    ship_name(ship), ship->objnum, RealGoodObject(obj) ? Name(obj) : "*Unknown*", obj, ship->x, ship->y, ship->z,
                    ship->xyhead, ship->zhead, ship->desired_xyhead, ship->desired_zhead, ship->speed, ship->desired_speed);
    }
  }

  return;
}


void hs_move(dbref executor, char *arg_left, char *arg_right)
{
  huniverse *uid;
  hcelestial *cel;
  hship *dship;
  hship *ship;
  int i, dx, dy, dz;
  dbref obj;
  dbref udb, ndb;
  char *r, *s;
  int len;
  dbref bot, top;

  obj = match_result(executor, arg_left, TYPE_THING, MAT_EVERYTHING);
  if (!IsShip(obj) && !IsCelestial(obj) && !IsConsole(obj))
  {
    notify(executor, "HSPACE: Unable to move that object.");
    return;
  }
  
  s = arg_right;
  r = split_token(&s, '/');
  if (!r || !*r)
  {
    notify(executor, "HSPACE: No destination specified.");
    return;
  }
  
  len = strlen(r);
  
  ndb = match_result(executor, r, TYPE_THING, MAT_EVERYTHING);
  
  if (!RealGoodObject(ndb))
  {
    ndb = parse_dbref(r);
  }
  
  if (!RealGoodObject(ndb))
  {
    bot = 0;
    top = db_top;
  } else {
    bot = ndb;
    top = bot + 1;
  }
  
  uid = NULL;
  /* cycle through the db and try to match a space object */
  for (udb = bot; udb < top; udb++)
  {
    if (strncasecmp(Name(udb), r, len) && udb != ndb)
      continue;
    
    if (IsUniverse(udb))
    {
      uid = find_universe(udb);
    }
    else if (IsCelestial(udb))
    {
      cel = find_celestial(udb);
      if (cel)
      {
        uid = cel->uid;
        dx = cel->x;
        dy = cel->y;
        dz = cel->z;
      }
    }
    else if (IsShip(udb) || IsShipObj(udb))
    {
      dship = find_ship(udb);
      if (dship)
      {
        dx = dship->x;
        dy = dship->y;
        dz = dship->z;
        uid = dship->uid;
      }
    }
    else
    {
      /* false positive */
      continue;
    }
  
    break;
  }
  
  if (!uid)
  {
    notify(executor, "HSPACE: Unable to locate destination.");
    return;
  }
  
  if (s)
  {
    r = split_token(&s, ' ');
    if (r)
    {
      dx = parse_integer(r);
    
      r = split_token(&s, ' ');
      if (!r || !*r)
      {
        dy = 0;
        dz = 0;
      } else {
        dy = parse_integer(r);
    
        r = split_token(&s, ' ');
        if (!r || !*r)
          dz = 0;
        else
          dz = parse_integer(r);
      }
    } else {
      /* all blanks */
      dx = 0;
      dy = 0;
      dz = 0;
    }
  }
  
  if (IsShip(obj) || IsConsole(obj))
  {
    ship = find_ship(obj);
    move_ship(ship, uid, dx, dy, dz);
  }
  else if (IsCelestial(obj))
  {
    cel = find_celestial(obj);
    move_celestial(cel, uid, dx, dy, dz);
  }
  
  notify(executor, "HSPACE: Moved.");
}

/* decay a message string */
char *decay_msg(char *msg, double decay)
{
  static char newmsg[2048];
  char   *ptr, *ptr2;
  char    word[2048];
  int     i, len;

  ptr2 = newmsg;
  i = 0;
  len = 0;
  for (ptr = msg; (len < 1900) && *ptr; ptr++)
  {
    if (*ptr == ' ')
    {
      word[i++] = ' ';
      word[i] = '\0';
      if (getrandom(100) < decay)
	strcpy(word, "... ");
      for (i = 0; word[i]; i++)
      {
	*ptr2 = word[i];
	ptr2++;
      }
      i = 0;
    }
    else
    {
      word[i++] = *ptr;
      len++;
    }
  }
  len += i;
  if (len < 1900)
  {
    word[i++] = ' ';
    word[i] = '\0';
    if (getrandom(100) < decay)
      strcpy(word, "...");
    for (i = 0; word[i]; i++)
    {
      *ptr2 = word[i];
      ptr2++;
    }
  }
  *ptr2 = '\0';
  return newmsg;
}


/* send a standard radio communication */
void send_com(dbref from, char *arg_left, char *arg_right)
{
  dbref com, obj;
  hship *ship;
  hcelestial *cel;
  huniverse *uid;
  
  double xmit, rcv;
  char contact[32];
  char *r, *s;
  char buff[128];
  ATTR *a;
  double sx, sy, sz, tx, ty, tz, dist;
  char pre[128];
  char *mesg;
  int sent_to_from, send_to_com;
  
  if (!IsComm(from))
  {
    notify(from, "You do not have the HS_COMM flag.");
    return;
  }
  
  uid = NULL;
  obj = atr_parse_dbref(from, "HSPACE");
  if (!RealGoodObject(obj))
  {
    notify(from, "You do not have a valid space id. Board, disembark, eject, or man a console.");
    return;
  }
  
  if (IsShip(obj))
  {
    ship = find_ship_by_nav(obj);
    if (!ship)
    {
      notify(from, "Your space id is not a valid ship.");
      return;
    }
    
    if (ship->uid)
    {
      sx = ship->x;
      sy = ship->y;
      sz = ship->z;
      uid = ship->uid;
    }
    else if (ship->landed)
    {
      sx = ship->landed->x;
      sy = ship->landed->y;
      sz = ship->landed->z;
      uid = ship->landed->uid;
    }
    else if (ship->docked)
    {
      sx = ship->docked->x;
      sy = ship->docked->y;
      sz = ship->docked->z;
      uid = ship->docked->uid;
    }
    
    strncpy(contact, ship_name(ship), 10);
  }
  else if (IsCelestial(obj))
  {
    cel = find_celestial(obj);
    if (!cel)
    {
      notify(from, "Your space id is not a valid celestial.");
      return;
    }
    
    sx = cel->x;
    sy = cel->y;
    sz = cel->z;
    uid = cel->uid;

    strncpy(contact, celestial_name(cel), 10);
  }
  contact[10] = '\0';
  
  if (!uid)
  {
    notify(from, "Your space id does not have a valid uid.");
    return;
  }
  
  if (arg_left && arg_right && *arg_right)
  {
    xmit = strtod(arg_left, &s);
    
    if (s && *s)
    {
      return;
    }
    mesg = arg_right;
  } else {
    xmit = atr_parse_double(from, "TRANSMIT", 0.0);
    mesg = arg_left;
  }
  if (xmit < 100.0 || xmit > 999.9)
  {
    notify(from, "Transmission frequency must be between 100 and 999 MHz.");
    return;
  }
  
  a = atr_get(from, "CALLSIGN");
  if (!a)
  {
    snprintf(pre, 127,
         "%s%s[%s%5.1f MHz%s%s]-[%s%-10s%s]-[%s ",
         ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, xmit, ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, contact,
         ANSI_BLUE, ANSI_NORMAL);
  }
  else
  {
    snprintf(pre, 127,
         "%s%s[%s%5.1f MHz%s%s]-[%s%-10s%s]-[%s %s<%s%s%s>%s ",
         ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, xmit, ANSI_HILITE, ANSI_BLUE, ANSI_GREEN, contact,
         ANSI_BLUE, ANSI_NORMAL, ANSI_CYAN, ANSI_NORMAL, atr_value(a), ANSI_CYAN, ANSI_NORMAL);
  }

  /* go through the comm list and check each one */
  sent_to_from = 0;
  for (com = 0; com < db_top; com++)
  {
    if (!IsComm(com))
      continue;
    
    /* check if the user is in the same uid */
    obj = atr_parse_dbref(com, "HSPACE");
    if (IsShip(obj))
    {
      ship = find_ship_by_nav(obj);
      if (!ship)
        continue;
      
      if (ship->uid && ship->uid != uid)
        continue;
      else if (ship->landed && ship->landed->uid != uid)
        continue;
      else if (ship->docked && ship->docked->uid != uid)
        continue;
      
      if (ship->uid)
      {
        tx = ship->x;
        ty = ship->y;
        tz = ship->z;
      }
      else if (ship->landed)
      {
        tx = ship->landed->x;
        ty = ship->landed->y;
        tz = ship->landed->z;
      }
      else if (ship->docked)
      {
        tx = ship->docked->x;
        ty = ship->docked->y;
        tz = ship->docked->z;
      }
    }
    else if (IsCelestial(obj))
    {
      cel = find_celestial(obj);
      if (!cel)
        continue;
      
      if (cel->uid != uid)
        continue;
      
      tx = cel->x;
      ty = cel->y;
      tz = cel->z;
    } else {
      continue;
    }
    
    dist = dist3d(sx, sy, sz, tx, ty, tz) / hs_options.max_comm_dist;
    if (dist > 1.0)
      continue;

    a = atr_get(com, "FREQUENCY");
    if (!a)
      continue;
    
    /* check all frequencies to see if we need to send to this com */
    send_to_com = 0;
    snprintf(buff, 127, atr_value(a));
    s = buff;
    while (s)
    {
      r = split_token(&s, ' ');
      rcv = parse_number(r);
      
      /* check to see if we're on the right frequency */
      if (fabs(rcv - xmit) < 0.1)
      {
        send_to_com = 1;
        break;
      }
    }
    
    if (send_to_com)
    {
      if (com == from)
        sent_to_from = 1;
      
      notify_format(com, "%s%s%s%s]%s", pre, decay_msg(mesg, dist), ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
    }
  }
  
  if (!sent_to_from)
  {
    notify_format(from, "You send \"%s\" on frequency %5.1f.", mesg, xmit);
  }
}






