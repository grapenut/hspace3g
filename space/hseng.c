
#include "hspace.h"


/*************************************************/
/* booster stances */

/* Weapon     NOTHING  rld  rng  dmg  acc  cnd  pow  cur  max  spd  loading */
/* System     NOTHING  cnd  rat  str  eff  nrg  max                         */

hbuff_effect STANCE_boost_engine =
{HS_BOOSTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_ENGINE, NOTHING, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0}
};

hbuff_effect STANCE_boost_reactor =
{HS_BOOSTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_REACTOR, NOTHING, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5}
};

hbuff_effect STANCE_boost_sensor =
{HS_BOOSTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_SENSOR, NOTHING, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5}
};

hbuff_effect STANCE_boost_computer =
{HS_BOOSTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_COMPUTER, NOTHING, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5}
};

hbuff_effect STANCE_boost_weapons =
{HS_BOOSTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL,
  {HS_ANY_WEAPON, NOTHING, 0.5, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};


/*******************************************************/
/* capacitor stances */

hbuff_effect STANCE_sensor_bypass =
{HS_CAPACITOR_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};
    
hbuff_effect STANCE_reactor_bypass =
{HS_CAPACITOR_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};
    
hbuff_effect STANCE_engine_bypass =
{HS_CAPACITOR_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};
    
hbuff_effect STANCE_shield_bypass =
{HS_CAPACITOR_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};


/***********************************************/
/* engineering cooldowns */

hbuff_effect COOLDOWN_overcharge =
{HS_ENG_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_compensate =
{HS_ENG_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_purge_hack, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_short =
{HS_ENG_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_reload_eng, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_boost =
{HS_ENG_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};


HBUFF_CALL(reload_eng)
{
  int i;
  
  if (!ship)
    return;
  
  for (i = 0; i < ship->nconsoles; i++)
  {
    if (ship->console[i].type == HS_ENG)
    {
      ship->console[i].primary.loading = 0;
      ship->console[i].primary.curpower = get_wstat(ship, &(ship->console[i]), HS_PRIMARY | HS_MAX_POWER);
   
      ship->console[i].secondary.loading = 0;
      ship->console[i].secondary.curpower = get_wstat(ship, &(ship->console[i]), HS_PRIMARY | HS_MAX_POWER);
    }
  }
}


/***********************************************/
/* shunts and shunt callbacks */

/* Weapon     NOTHING  rld  rng  dmg  acc  cnd  pow  cur  max  spd  loading */
/* System     NOTHING  cnd  rat  str  eff  nrg  max                         */

/**************************************************/
/* shunt, capacitor and booster routines */

/* add a shunt to the queue */
void use_shunt(dbref console, char *which)
{
  hship *ship;
  hbuff_effect *bptr;
  hbuff *b;
  hconsole *con;
  double cost;
  
  /* check for a valid console and capacitor */
  if (IsConsole(console))
  {
    con = find_console(console);
    if (!console)
    {
      hs_std_notice(console, "Invalid console object. Please contact your flight mechanic immediately!");
      return;
    }
    
    if (!HasFlag(con->primary.type, HS_CAPACITOR) && !HasFlag(con->secondary.type, HS_CAPACITOR))
    {
      hs_std_notice(console, "Must have a capacitor equipped to use shunts.");
      return;
    }
    
    ship = find_ship(console);
  }
  else if (IsShip(console))
  {
    ship = find_ship_by_nav(console);
    
    if (ship && !HasFlag(ship->primary.type, HS_CAPACITOR) && !HasFlag(ship->secondary.type, HS_CAPACITOR))
    {
      hs_std_notice(console, "Must have a capacitor equipped to use shunts.");
      return;
    }
  } else {
    ship = NULL;
  }

  if (!ship)
  {
    hs_std_notice(console, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  /* find the shunt */
  bptr = find_effect(hs_shunts, which);
  if (!bptr || !HasFlag(bptr->flags, HS_SHUNT))
  {
    hs_std_notice(console, "Please specify a valid shunt.");
    return;
  }
  
  /* check to see if we have enough power for this shunt */
  cost = bptr->cost * (get_stat(ship, SYS_RESISTANCE) / 100.0);
  if ((ship->reactor.energy - cost) < 0.0)
  {
    hs_std_notice(console, "Insufficient reactor energy.");
    return;
  }

  ship->reactor.energy -= cost;
  
  /* everything checks out, add the shunt buff */
  b = add_buff(ship, bptr);
  if (!b || !b->buff)
  {
    hs_std_notice(console, "Error adding the shunt.");
    return;
  }
  
  notify_consoles(ship, tprintf("%s%s**%s %sENG%s - Shunting %s%s%s%s.", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
          ANSI_HILITE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, STR(hs_shunts, b->buff), ANSI_NORMAL));
}


/* fire the capacitor, transferring energy to systems based on stance */
int fire_capacitor(hship *ship, hconsole *con, hweapon *gun, hcontact *q)
{
  hbuff_effect *stance;
  hbuff *b;
  int ticks;
  double power;
  
  if (!ship || !gun)
  {
    return HS_NODAMAGE;
  }
  
  power = get_wstat(ship, con, gun->type | HS_POWER);
  ticks = gun->curpower / power;
  if (ticks < 1)
    return HS_NODAMAGE;
  
  /* must do this in case we don't use up all the energy */
  gun->curpower -= ticks * power;
  
  /* now find the system and add some power */
  b = find_buff(ship, &STANCE_shield_bypass);
  if (b)
  {
    stance = b->buff;
  
    if (stance == &STANCE_reactor_bypass)
    {
      power = ticks * get_stat(ship, SYS_RECHARGE);
      ship->reactor.energy += power;
    }
    else if (stance == &STANCE_engine_bypass)
    {
      power = ticks * get_stat(ship, SYS_DISSIPATION);
      ship->engine.energy -= power;
    }
    else if (stance == &STANCE_sensor_bypass)
    {
      power = ticks * get_stat(ship, SYS_TRACKING);
      ship->sensor.energy += power;
    } else {
      /* assume shield bypass */
      power = ticks * get_wstat(ship, con, gun->type | HS_DAMAGE);
      ship->shield.energy += power;
      stance = &STANCE_shield_bypass;
    }
  } else {
    /* assume shield bypass */
    power = ticks * get_wstat(ship, con, gun->type | HS_DAMAGE);
    ship->shield.energy += power;
    stance = &STANCE_shield_bypass;
  }
  
  notify_consoles(ship, tprintf("%s%s**%s %sENG%s - (%s%3.0f%s) %s%s%s%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL, ANSI_HILITE, ANSI_NORMAL, ANSI_CYAN, power, ANSI_NORMAL,
          ANSI_HILITE, ANSI_GREEN, STR(hs_stances, stance), ANSI_NORMAL));
  
  return HS_NODAMAGE;  
}

/* firing a booster does nothing */
/* TODO maybe firing the booster could expire the booster stance
   giving some sort of more powerful buff for a short duration? */
int fire_booster(hship *ship, hconsole *con, hweapon *gun, hcontact *q)
{
  return HS_NODAMAGE;
}





