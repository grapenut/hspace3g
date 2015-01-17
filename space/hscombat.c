
#include "hspace.h"

/* buff effect list */
hstringmap hs_effects[] =
{
  /* weapon side effects */
  { "Missile", &EFFECT_missile },
  { "Particle Beam", &EFFECT_emitter_beam },
  { "Residue", &EFFECT_residue },
  /* hack debuff */
  { "Shield Failure", &EFFECT_shield_failure },
  { "Reactor Failure", &EFFECT_reactor_failure },
  { "Engine Failure", &EFFECT_engine_failure },
  { "Sensor Failure", &EFFECT_sensor_failure },
  { "Computer Failure", &EFFECT_computer_failure },
  { "Weapon Failure", &EFFECT_weapon_failure },
  /* shunt buffs */
  { "Shield Recharge", &EFFECT_shield_recharge },
  { "Engine Overdrive", &EFFECT_engine_overdrive },
  /* cooldown buffs */
  { "Overheating", &EFFECT_missile_bonus },
  { "Evading", &EFFECT_evasion_bonus },
  { "Bursting", &EFFECT_speed_bonus },
  { "Overloaded", &EFFECT_cannon_power_bonus },
  { "Tracking", &EFFECT_cannon_accuracy_bonus },
  { "Piercing", &EFFECT_cannon_penetration_bonus },
  { "Rapid Fire", &EFFECT_cannon_reload_bonus },
  { "Overcharged", &EFFECT_capacitor_bonus },
  { "Boosted", &EFFECT_booster_bonus },
  { "Overriding", &EFFECT_emitter_bonus },
  { "Active Scanner", &EFFECT_ping_bonus },
  { "Active Target", &EFFECT_remote_ping_bonus },
  {NULL, NULL}
};

hstringmap hs_stances[] =
{
  /* missile stances, explosion type */
  { "Missile", &STANCE_standard_missile },
  { "Banshee Missile", &STANCE_shield_missile },
  { "Dumbfire Missile", &STANCE_hull_missile },
  { "Phase Missile", &STANCE_penetrate_missile },
  { "Torpedo", &STANCE_penetrate_missile },
  /* emitter stances, beam type */
  { "Proton Beam", &STANCE_standard_beam },
  { "Meson Beam", &STANCE_shield_beam },
  { "Plasma Beam", &STANCE_hull_beam },
  { "Neutron Beam", &STANCE_penetrate_beam },
  /* cannon stances, residue type */
  { "Mass Driver Cannon", &STANCE_standard_cannon },
  { "Ion Cannon", &STANCE_shield_cannon },
  { "Heavy Plasma Cannon", &STANCE_hull_cannon },
  { "Tachyon Cannon", &STANCE_penetrate_cannon },
  /* capacitor stances, bypass type */
  { "Shield Bypass", &STANCE_shield_bypass },
  { "Sensor Bypass", &STANCE_sensor_bypass },
  { "Engine Bypass", &STANCE_engine_bypass },
  { "Reactor Bypass", &STANCE_reactor_bypass },
  /* booster stances, boost type */
  { "Boost Engine", &STANCE_boost_engine },
  { "Boost Reactor", &STANCE_boost_reactor },
  { "Boost Sensor", &STANCE_boost_sensor },
  { "Boost Computer", &STANCE_boost_computer },
  { "Boost Weapons", &STANCE_boost_weapons },
  {NULL, NULL}
};

hstringmap hs_hacks[] =
{
  /* hacks */
  { "Shield Hack", &HACK_shield },
  { "Reactor Hack", &HACK_reactor },
  { "Engine Hack", &HACK_engine },
  { "Sensor Hack", &HACK_sensor },
  { "Computer Hack", &HACK_computer },
  { "Weapon Hack", &HACK_weapon },
  {NULL, NULL}
};

hstringmap hs_shunts[] =
{
  /* shunts */
  { "Shields", &EFFECT_shield_recharge },
  { "Engines", &EFFECT_engine_overdrive },
  {NULL, NULL}
};

hstringmap hs_cooldowns[] =
{
  /* pilot cooldowns */
  { "Overheat", &COOLDOWN_overheat },
  { "Juke", &COOLDOWN_juke },
  { "Jive", &COOLDOWN_jive },
  { "Burst", &COOLDOWN_burst },
  { "Vanish", &COOLDOWN_vanish },
  { "Reload", &COOLDOWN_reload },
  /* turret cooldowns */
  { "Overload", &COOLDOWN_overload },
  { "Sixoclock", &COOLDOWN_sixoclock },
  { "Sniper", &COOLDOWN_sniper },
  { "Pierce", &COOLDOWN_pierce },
  { "Rapid", &COOLDOWN_rapid },
  { "Ignite", &COOLDOWN_ignite },
  /* operations cooldowns */
  { "Override", &COOLDOWN_override },
  { "Quarantine", &COOLDOWN_quarantine },
  { "Reboot", &COOLDOWN_reboot },
  { "Cache", &COOLDOWN_cache },
  /* engineering cooldowns */
  { "Overcharge", &COOLDOWN_overcharge },
  { "Compensate", &COOLDOWN_compensate },
  { "Short", &COOLDOWN_short },
  { "Boost", &COOLDOWN_boost },
/* general cooldowns */
  { "Jump Drive", &COOLDOWN_jump_drive },
  { "Ping", &COOLDOWN_ping },
/*  { "", &COOLDOWN_ },
  { "", &COOLDOWN_ },*/
  {NULL, NULL}
};


hbuff_effect COOLDOWN_jump_drive =
{HS_NAV_COOLDOWN, 60, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_ping =
{HS_PRE | HS_NAV_COOLDOWN, 300, 0, 0.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect EFFECT_remote_ping_bonus =
{HS_BUFF | HS_REFRESH | HS_REMOTE, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_SENSOR, NOTHING, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0}
};

hbuff_effect EFFECT_ping_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_SENSOR, NOTHING, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0}
};

/*******************************************************/
/* weapon buff effects */

hbuff_effect EFFECT_missile =
{HS_POST, 3, 0, 0.0, NULL, NULL, buffcall_missile_explosion, NULLWEAPON, NULLSYSTEM};

hbuff_effect EFFECT_emitter_beam =
{HS_POST | HS_PERIODIC | HS_REFRESH, 1, 2, 0.0, NULL, buffcall_emitter_tick, buffcall_emitter_tick, NULLWEAPON, NULLSYSTEM};

hbuff_effect EFFECT_residue =
{HS_POST | HS_STACKABLE, 10, 0, 0.0, NULL, NULL, buffcall_residue_explosion, NULLWEAPON, NULLSYSTEM};


/*****************************************************/
/* general buff effects */

/* Weapon      NOTHING  rld  rng  dmg  acc  cnd  pow  cur  max  spd  loading */
/* System      NOTHING  cnd  rat  str  eff  nrg  max                         */

/* buffs for hacks */
hbuff_effect EFFECT_shield_failure =
{HS_DEBUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_SHIELD,  NOTHING, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0}
};

hbuff_effect EFFECT_reactor_failure =
{HS_DEBUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_REACTOR,  NOTHING, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0}
};

hbuff_effect EFFECT_engine_failure =
{HS_DEBUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_ENGINE,  NOTHING, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0}
};

hbuff_effect EFFECT_sensor_failure =
{HS_DEBUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_SENSOR,  NOTHING, 0.0, 0.0, 0.0, -100.0, 0.0, 0.0}
};

hbuff_effect EFFECT_computer_failure =
{HS_DEBUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_COMPUTER,  NOTHING, 0.0, -1.0, -1.0, 0.0, 0.0, 0.0}
};

hbuff_effect EFFECT_weapon_failure =
{HS_DEBUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HS_ANY_WEAPON, NOTHING, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0},
  NULLSYSTEM
};

/* buffs for shunts */
hbuff_effect EFFECT_shield_recharge =
{HS_BUFF | HS_SHUNT | HS_REFRESH, 10, 0, 10.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_SHIELD,  NOTHING, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0}
};

hbuff_effect EFFECT_engine_overdrive =
{HS_BUFF | HS_SHUNT | HS_REFRESH, 10, 0, 10.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_ENGINE,  NOTHING, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0}
};

/* Weapon      NOTHING  rld  rng  dmg  acc  cnd  pow  cur  max  spd  loading */
/* System      NOTHING  cnd  rat  str  eff  nrg  max                         */
// {0, 	       NOTHING, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0}
// {0,         NOTHING, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}

/* buffs for cooldowns */
hbuff_effect EFFECT_missile_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HS_MISSILE | HS_WEAPON, NOTHING, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};

hbuff_effect EFFECT_evasion_bonus =
{HS_REMOTE | HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HasFlag(HS_ANY_WEAPON, ~HS_MISSILE), NOTHING, 0.0, 0.0, 0.0, -0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};

hbuff_effect EFFECT_speed_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON,
  {HS_ENGINE, NOTHING, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0}
};

hbuff_effect EFFECT_cannon_power_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HS_CANNON, NOTHING, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};

hbuff_effect EFFECT_cannon_accuracy_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HS_CANNON, NOTHING, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};

hbuff_effect EFFECT_cannon_penetration_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect EFFECT_cannon_reload_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HS_CANNON, NOTHING, -0.5, 0.0, -0.5, 0.0, 0.0, -0.5, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};

hbuff_effect EFFECT_capacitor_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HS_CAPACITOR, NOTHING, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};

hbuff_effect EFFECT_booster_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect EFFECT_emitter_bonus =
{HS_BUFF | HS_REFRESH, 10, 0, 0.0, NULL, NULL, NULL,
  {HS_CAPACITOR, NOTHING, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0}, NULLSYSTEM
};



/*****************************************************/
/* general purpose buff callbacks */

/* callback for cooldowns that just activate another buff */
HBUFF_CALL(activate_cooldown)
{
  hbuff *bptr;
  
  if (!ship || !buff || !buff->buff)
    return;
  
  if (buff->buff == &COOLDOWN_overheat)
  {
    bptr = add_buff(ship, &EFFECT_missile_bonus);
  }
  else if (buff->buff == &COOLDOWN_jive)
  {
    bptr = add_buff(ship, &EFFECT_evasion_bonus);
  }
  else if (buff->buff == &COOLDOWN_burst)
  {
    bptr = add_buff(ship, &EFFECT_speed_bonus);
  }
  else if (buff->buff == &COOLDOWN_overload)
  {
    bptr = add_buff(ship, &EFFECT_cannon_power_bonus);
  }
  else if (buff->buff == &COOLDOWN_sniper)
  {
    bptr = add_buff(ship, &EFFECT_cannon_accuracy_bonus);
  }
  else if (buff->buff == &COOLDOWN_pierce)
  {
    bptr = add_buff(ship, &EFFECT_cannon_penetration_bonus);
  }
  else if (buff->buff == &COOLDOWN_rapid)
  {
    bptr = add_buff(ship, &EFFECT_cannon_reload_bonus);
  }
  else if (buff->buff == &COOLDOWN_overcharge)
  {
    bptr = add_buff(ship, &EFFECT_capacitor_bonus);
  }
  else if (buff->buff == &COOLDOWN_boost)
  {
    bptr = add_buff(ship, &EFFECT_booster_bonus);
  }
  else if (buff->buff == &COOLDOWN_override)
  {
    bptr = add_buff(ship, &EFFECT_emitter_bonus);
  }
  else if (buff->buff == &COOLDOWN_ping)
  {
    bptr = add_buff(ship, &EFFECT_ping_bonus);
    bptr = add_buff(ship, &EFFECT_remote_ping_bonus);
  }
  else
  {
    bptr = NULL;
  }
}


/*****************************************************/
/* buff utility routines */

/* locate the buff effect definition in the database */
hbuff_effect *find_effect(hstringmap *map, char *which)
{
  hbuff_effect *b;
  
  if (!which || !*which)
    return NULL;
  
  if (!map)
    map = hs_effects;
  
  b = (hbuff_effect *) stringmap_value(map, which);

  return b;
}

/* find a buff on a ship using an hbuff_effect */
hbuff *find_buff(hship *ship, hbuff_effect *bptr)
{
  hbuff *b;
  
  if (!ship)
    return NULL;
  
  if (!bptr)
    return NULL;
  
  for (b = ship->head_buff; b; b = b->next)
  {
    if (!b->buff)
      continue;
      
    if (b->buff == bptr)
      return b;

    /* if the buff has the stance flag, return it even if it doesn't match */
    /* may only have 1 active stance (of each type) at a time */
    /* you could possibly have 1 stance for each weapon type */

    if (HasFlag(bptr->flags, HS_ANY_STANCE) && (HasFlag(b->buff->flags, HS_ANY_STANCE) == HasFlag(bptr->flags, HS_ANY_STANCE)))
      return b;
  } 

  return NULL;
}


/* remove all buffs from a ship */
int clear_buffs(hship *ship)
{
  hbuff *b, *bptr;

  if (!ship)
    return 0;
  
  b = ship->head_buff;
  while (b)
  {
    bptr = b;
    b = b->next;
    free(bptr);
    ship->nbuffs--;
      
    if (ship->nbuffs < 0)
    {
      SPACEWALL("HSPACE: ERROR: clear_buffs(), nbuffs is negative, check for memory leaks!");
      break;
    }
  }

  ship->nbuffs = 0;
  ship->head_buff = NULL;

  return 1;
}

/* add a new effect to a ship */
hbuff *add_buff(hship *ship, hbuff_effect *bptr)
{
  hbuff *b;
  
  if (!ship)
    return NULL;
  
  if (!bptr)
    return NULL;
  
  b = find_buff(ship, bptr);
  if (!b || !HasFlag(bptr->flags, HS_STACKABLE | HS_REFRESH | HS_ANY_STANCE))
  {
    b = (hbuff *) malloc(sizeof(hbuff));
    if (!b)
    {
      SPACEWALL("HSPACE: ERROR: unable to allocate buff!");
      return NULL;
    }
    
    b->buff = bptr;
    b->owner = ship;
    b->stacks = 1;
    b->duration = bptr->duration;
    b->var1 = 0.0;
    b->var2 = 0.0;
    b->var3 = 0.0;
    b->var4 = 0.0;
    b->var5 = 0.0;
    
    b->next = ship->head_buff;
    ship->head_buff = b;
    ship->nbuffs++;

    if (HasFlag(b->buff->flags, HS_PRE) && b->buff->pre)
      b->buff->pre(ship, b);
    
    return b;
  }
  
  if (!b)
    return NULL;
  
  if (HasFlag(bptr->flags, HS_STACKABLE)) {
    b->stacks++;
  }

  if (HasFlag(bptr->flags, HS_REFRESH)) {
    b->duration = bptr->duration;
  }
  
  if (HasFlag(bptr->flags, HS_ANY_STANCE)) {
    b->buff = bptr;
  }
  
  if (b->buff->pre)
    b->buff->pre(ship, b);

  return b;
}

/* change any stance */
void switch_stance(dbref console, char *which)
{
  hship *ship;
  hconsole *con;
  hbuff *b;
  hbuff_effect *stance;
  int gtype;
  
  if (IsConsole(console))
  {
    con = find_console(console);
    if (!con)
    {
      hs_std_notice(console, "Invalid console object. Contact your local flight mechanic immediately!");
      return;
    }
    
    gtype = HasFlag(con->primary.type, HS_ANY_WEAPON) | HasFlag(con->secondary.type, HS_ANY_WEAPON);
    ship = find_ship(console);
  }
  else if (IsShip(console))
  {
    ship = find_ship_by_nav(console);
    
    if (ship)
    {
      gtype = HasFlag(ship->primary.type, HS_ANY_WEAPON) | HasFlag(ship->secondary.type, HS_ANY_WEAPON);
    }
  } else {
    ship = NULL;
  }
  
  if (!ship)
  {
    hs_std_notice(console, "Invalid ship object. Contact your local flight mechanic immediately!");
    return;
  }
  
  stance = find_effect(hs_stances, which);
  if (!stance || !HasFlag(stance->flags, HS_ANY_STANCE))
  {
    hs_std_notice(console, "Invalid stance.");
    return;
  }
  
  /* only allow stance switching if you have the correct weapon equipped */
  switch (HasFlag(stance->flags, HS_ANY_STANCE))
  {
  case HS_MISSILE_STANCE:
    if (!HasFlag(gtype, HS_MISSILE))
    {
      hs_std_notice(console, "No missiles equipped.");
      return;
    }
    break;
  case HS_CANNON_STANCE:
    if (!HasFlag(gtype, HS_CANNON))
    {
      hs_std_notice(console, "No cannon equipped.");
      return;
    }
    break;
  case HS_EMITTER_STANCE:
    if (!HasFlag(gtype, HS_EMITTER))
    {
      hs_std_notice(console, "No emitters equipped.");
      return;
    }
    break;
  case HS_CAPACITOR_STANCE:
    if (!HasFlag(gtype, HS_CAPACITOR))
    {
      hs_std_notice(console, "No capacitor equipped.");
      return;
    }
    break;
  case HS_BOOSTER_STANCE:
    if (!HasFlag(gtype, HS_BOOSTER))
    {
      hs_std_notice(console, "No booster equipped.");
      return;
    }
    break;
  default:
    hs_std_notice(console, "Unimplemented stance type. Contact your local flight mechanic immediately!");
    return;
    break;
  }
  
  /* everything checks out, add the stance */
  b = add_buff(ship, stance);
  if (!b || !b->buff)
  {
    hs_std_notice(console, "Could not change stance.");
    return;
  }
  
  hs_std_notice(console, tprintf("Selecting %s%s%s%s.", ANSI_HILITE, ANSI_GREEN,
          STR(hs_stances, b->buff), ANSI_NORMAL));
}

/*************************************************/
/* system and weapon stat utilities */

double get_system_stat(hsystem *sys, hstat stat)
{
  if (!sys)
    return 0.0;
  
  if (HasFlag(stat, HS_RATE))
    return sys->rate;
  else if (HasFlag(stat, HS_STRENGTH))
    return sys->strength;
  else if (HasFlag(stat, HS_EFFICIENCY))
    return sys->efficiency;
  else if (HasFlag(stat, HS_ENERGY))
    return sys->energy;
  else if (HasFlag(stat, HS_MAX_ENERGY))
    return sys->maxenergy;
  else
    return 0.0;
}

/* get a system stat by ship */
double get_stat(hship *ship, hstat stat)
{
  hsystem *sys;
  hbuff *b;
  double bonus;
  int i, use_bonus;
  
  if (!ship)
    return 0.0;
  
  sys = NULL;
  
  if (HasFlag(stat, HS_HULL))
    sys = &(ship->hull);
  else if (HasFlag(stat, HS_SHIELD))
    sys = &(ship->shield);
  else if (HasFlag(stat, HS_ENGINE))
    sys = &(ship->engine);
  else if (HasFlag(stat, HS_REACTOR))
    sys = &(ship->reactor);
  else if (HasFlag(stat, HS_SENSOR))
    sys = &(ship->sensor);
  else if (HasFlag(stat, HS_COMPUTER))
    sys = &(ship->computer);
  
  if (!sys)
    return 0.0;
  
  /* apply buffs */
  bonus = 0.0;
  for (b = ship->head_buff; b; b = b->next)
  {
    if (!b->buff)
      continue;
    
    if (HasFlag(b->buff->sys.type, stat) && !HasFlag(b->buff->flags, HS_REMOTE))
    {
      use_bonus = 0;

      /* booster stance bonuses require a loaded booster */
      if (HasFlag(b->buff->flags, HS_BOOSTER_STANCE))
      {
        if (HasFlag(ship->primary.type, HS_BOOSTER) || HasFlag(ship->secondary.type, HS_BOOSTER))
          use_bonus = 1;
        else
        {
          for (i = 0; i < ship->nconsoles; i++)
          {
            if (HasFlag(ship->primary.type, HS_BOOSTER) || HasFlag(ship->secondary.type, HS_BOOSTER))
            {
              use_bonus = 1;
              break;
            }
          }
        }
      } else {
        use_bonus = 1;
      }

      /* apply normal buff bonus */
      if (use_bonus)
        bonus += get_system_stat(&(b->buff->sys), stat);

    }
  }
  
  return ((1.0 + bonus) * get_system_stat(sys, stat));
}


/****************************************************/
/* weapon stats */

double get_weapon_stat(hweapon *gun, hwstat stat)
{
  if (!gun)
    return 0.0;
  
  if (HasFlag(stat, HS_RELOAD))
    return gun->reload;
  else if (HasFlag(stat, HS_RANGE))
    return gun->range;
  else if (HasFlag(stat, HS_DAMAGE))
    return gun->damage;
  else if (HasFlag(stat, HS_ACCURACY))
    return gun->accuracy;
  else if (HasFlag(stat, HS_POWER))
    return gun->power;
  else if (HasFlag(stat, HS_CUR_POWER))
    return gun->curpower;
  else if (HasFlag(stat, HS_MAX_POWER))
    return gun->maxpower;
  else if (HasFlag(stat, HS_SPEED))
    return gun->speed;
  else
    return 0.0;
}


/* get a console's weapon stat */
double get_wstat(hship *ship, hconsole *con, hwstat stat)
{
  hship *dship;
  hweapon *gun;
  hbuff *b;
  double bonus, remote;
  int i, use_bonus;
  
  if (!ship)
    return 0.0;
  
  gun = NULL;
  dship = NULL;
  
  if (!con)
  {
    if (ship->lock && ship->lock->contact)
      dship = ContactShip(ship->lock);

    /* use ship weapon */
    if (HasFlag(stat, HS_PRIMARY))
      gun = &(ship->primary);
    else if (HasFlag(stat, HS_SECONDARY))
      gun = &(ship->secondary);
  } else {
    if (con->lock && con->lock->contact)
      dship = ContactShip(con->lock);
    
    /* use console weapon */
    if (HasFlag(stat, HS_PRIMARY))
      gun = &(con->primary);
    else if (HasFlag(stat, HS_SECONDARY))
      gun = &(con->secondary);
  }
  
  if (!gun)
    return 0.0;
  
  /* add up bonuses from buffs */
  bonus = 0.0;
  for (b = ship->head_buff; b; b = b->next)
  {
    if (!b->buff)
      continue;
    
    if (HasFlag(b->buff->gun.type, stat) && !HasFlag(b->buff->flags, HS_REMOTE))
    {
      use_bonus = 0;

      /* booster stance bonuses require a loaded booster */
      if (HasFlag(b->buff->flags, HS_BOOSTER_STANCE))
      {
        if (HasFlag(ship->primary.type, HS_BOOSTER) || HasFlag(ship->secondary.type, HS_BOOSTER))
          use_bonus = 1;
        else
        {
          for (i = 0; i < ship->nconsoles; i++)
          {
            if (HasFlag(ship->primary.type, HS_BOOSTER) || HasFlag(ship->secondary.type, HS_BOOSTER))
            {
              use_bonus = 1;
              break;
            }
          }
        }
      } else {
        use_bonus = 1;
      }

      /* apply normal buff bonus */
      if (use_bonus)
        bonus += get_weapon_stat(&(b->buff->gun), stat);

    }
  }
  
  /* add up the 'bonuses' from remote debuffs */
  remote = 0.0;
  if (dship)
  {
    for (b = dship->head_buff; b; b = b->next)
    {
    
      if (b->duration)
        use_bonus = 0;
    
      if (!b->buff)
        continue;
    
      if (HasFlag(b->buff->gun.type, stat) && HasFlag(b->buff->gun.type, HS_REMOTE))
      {
        use_bonus = 0;

        /* booster stance bonuses require a loaded booster */
        if (HasFlag(b->buff->flags, HS_BOOSTER_STANCE))
        {
          if (HasFlag(dship->primary.type, HS_BOOSTER) || HasFlag(dship->secondary.type, HS_BOOSTER))
            use_bonus = 1;
          else
          {
            for (i = 0; i < dship->nconsoles; i++)
            {
              if (HasFlag(dship->primary.type, HS_BOOSTER) || HasFlag(dship->secondary.type, HS_BOOSTER))
              {
                use_bonus = 1;
                break;
              }
            }
          }
        } else {
          use_bonus = 1;
        }

        /* apply normal buff bonus */
        if (use_bonus)
          remote += get_weapon_stat(&(b->buff->gun), stat);

      }
    }
  }
  
  return ((1.0 + (bonus+remote)) * get_weapon_stat(gun, stat));
}

/*******************************************************/
/* generic weapon utilities */

/* apply direct damage */
/* returns 0 for no damage done
           1 for damage absorbed by shields
           2 for critical shield damage (> 30% of target's total)
           3 for damage to hull
           4 for critical hull damage (> 30% of target's total)
           5 for disabling shot
           6 for destroying shot
*/
int do_damage(hship *ship, double shield, double hull, double penetration)
{
  double actual_shield, actual_hull;
  double max_shield, max_hull;
  int ret;

  if (!ship)
    return HS_NODAMAGE;
  
  if (shield > ship->shield.energy)
    actual_shield = ship->shield.energy;
  else
    actual_shield = shield;

  /* only do hull damage once shields are gone, unless using a penetrating weapon */
  /* if shield is removed, spent shield damage is subtracted from hull damage */
  if (ship->shield.energy > actual_shield)
    actual_hull = hull * penetration;
  else if (actual_shield > 0.0 && shield > 0.0)
    actual_hull = hull * (1.0 - actual_shield/shield);
  else
    actual_hull = hull;
  
  max_hull = get_stat(ship, SYS_MAX_ARMOR);
  max_shield = get_stat(ship, SYS_MAX_CAPACITY);
  
  /* check for each damage result type */
  if (ship->hull.energy > -max_hull && (ship->hull.energy - actual_hull) < -max_hull)
    ret = HS_KILLSHOT;
  else if (ship->hull.energy > 0.0 && ship->hull.energy < actual_hull)
    ret = HS_DISABLESHOT;
  else if (actual_hull > 0.3*max_hull)
    ret = HS_CRITHULL;
  else if (actual_hull > actual_shield)
    ret = HS_MINHULL;
  else if (actual_shield > 0.3*max_shield)
    ret = HS_CRITSHIELD;
  else if (actual_shield > 0.0)
    ret = HS_MINSHIELD;
  else
    ret = HS_NODAMAGE;
  
  ship->shield.energy -= actual_shield;
  ship->hull.energy -= actual_hull;
  
  return ret;
}


/* activate a cooldown ability */
void use_cooldown(dbref console, char *which)
{
  hship *ship;
  hbuff_effect *bptr;
  hbuff *b;
  hconsole *con;
  double cost;
  int ctype;
  
  /* check for a valid console */
  if (IsConsole(console))
  {
    con = find_console(console);
    if (!console)
    {
      hs_std_notice(console, "Invalid console object. Please contact your flight mechanic immediately!");
      return;
    }
    
    ctype = con->type;
    ship = find_ship(console);
  }
  else if (IsShip(console))
  {
    ctype = HS_NAV;
    ship = find_ship_by_nav(console);
  } else {
    ship = NULL;
  }

  if (!ship)
  {
    hs_std_notice(console, "Invalid ship object. Please contact your flight mechanic immediately!");
    return;
  }
  
  /* find the cooldown */
  bptr = find_effect(hs_cooldowns, which);
  if (!bptr || !HasFlag(bptr->flags, HS_ANY_COOLDOWN))
  {
    hs_std_notice(console, "Please specify a valid cooldown ability.");
    return;
  }
  
  b = find_buff(ship, bptr);
  if (b)
  {
    hs_std_notice(console, tprintf("%s%s%s%s on cooldown. (%s%2ds%s)", ANSI_HILITE, ANSI_GREEN,
            STR(hs_cooldowns, b->buff), ANSI_NORMAL, ANSI_HILITE, b->duration, ANSI_NORMAL));
    return; 
  }
  
  /* check to see if we have enough power for this cooldown */
  switch (HasFlag(bptr->flags, HS_ANY_COOLDOWN))
  {
  case HS_NAV_COOLDOWN:
    if (ctype != HS_NAV)
    {
      hs_std_notice(console, "That ability must be used at the navigation helm.");
      return;
    }
    
    cost = bptr->cost;
    if ((ship->engine.energy + cost) > get_stat(ship, SYS_MAX_HEAT))
    {
      hs_std_notice(console, "Insufficient heat capacity.");
      return;
    }

    ship->engine.energy += cost;
    break;
  case HS_GUN_COOLDOWN:
    if (ctype != HS_GUN)
    {
      hs_std_notice(console, "That ability must be used at a gunnery turret.");
      return;
    }
    
    cost = bptr->cost;
    if ((ship->sensor.energy - cost) < 0.0)
    {
      hs_std_notice(console, "Insufficient sensor focus.");
      return;
    }

    ship->sensor.energy -= cost;
    break;
  case HS_OPS_COOLDOWN:
    if (ctype != HS_OPS)
    {
      hs_std_notice(console, "That ability must be used at an operations console.");
      return;
    }
    
    cost = bptr->cost * (1.0 - (get_stat(ship, SYS_COMPRESSION) / 100.0));
    if ((ship->computer.energy + cost) > get_stat(ship, SYS_MAX_MEMORY))
    {
      hs_std_notice(console, "Insufficient computer memory.");
      return;
    }

    ship->computer.energy += cost;
    break;
  case HS_ENG_COOLDOWN:
    if (ctype != HS_ENG)
    {
      hs_std_notice(console, "That ability must be used at an engineering console.");
      return;
    }
    
    cost = bptr->cost * (get_stat(ship, SYS_RESISTANCE) / 100.0);
    if ((ship->reactor.energy - cost) < 0.0)
    {
      hs_std_notice(console, "Insufficient reactor energy.");
      return;
    }

    ship->reactor.energy -= cost;
    break;
  default:
    hs_std_notice(console, "Invalid cooldown. Please contact your local flight mechanic immediately!");
    return;
    break;
  }

  /* everything checks out, add the cooldown buff */
  b = add_buff(ship, bptr);
  if (!b || !b->buff)
  {
    hs_std_notice(console, "Error executing the cooldown.");
    return;
  }
  
  notify_consoles(ship, tprintf("%s%s**%s %s%3s%s - %s%s%s%s activated.", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
          ANSI_HILITE, STR(hs_console_types, ctype), ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN,
          STR(hs_cooldowns, b->buff), ANSI_NORMAL));
}


/* fire a general weapon slot */
int fire_weapon(hship *ship, hconsole *con, hweapon *gun, hcontact *q, double var, int showmsg)
{
  int damage;
  dbref console, obj;
  char err[256];
  hcontact *qptr;
  
  if (!ship || !gun)
    return 0;
  
  if (!HasFlag(gun->type, HS_CAPACITOR | HS_BOOSTER))
  {
    if (!q || !q->contact)
      return 0;
  }
  
  if (!con)
    console = ship->objnum;
  else
    console = con->objnum;

  /* check if the weapon is loaded */
  if (gun->loading > 0)
  {
    if (showmsg) hs_std_notice(console, "Weapon is still loading.");
    return -1;
  }

  /* check if the weapon has power */
  if (gun->curpower < get_wstat(ship, con, gun->type | HS_POWER))
  {
    if (showmsg) hs_std_notice(console, "Weapon has insufficient power.");
    return -1;
  }

  /* check if the target is in range of the weapon */
  if (!HasFlag(gun->type, HS_CAPACITOR | HS_BOOSTER))
  {
    if (get_wstat(ship, con, gun->type | HS_RANGE) < ship_distance(ship, ContactShip(q)))
    {
      if (showmsg) hs_std_notice(console, "Target is out of range.");
      return -2;
    }
  }
  
  switch (HasFlag(gun->type, HS_ANY_WEAPON))
  {
  case HS_WEAPON:
    /* direct damage, reload */
    /* normal hspace weapon */
    damage = fire_gun(ship, con, gun, q);
    break;
  case HS_CANNON:
    /* direct damage, recharge */
    /* hold down the autofire as long as power is available */
    damage = fire_cannon(ship, con, gun, q);
    break;
  case HS_MISSILE:
    /* bloom damage */
    /* time bomb */
    damage = fire_missile(ship, con, gun, q);
    break;
  case HS_EMITTER:
    /* channeled periodic damage */
    /* arcane missiles */
    damage = fire_emitter(ship, con, gun, q);
    break;
  case HS_WIRETAP:
    /* cc/debuffs (no damage) */
    /* fear, sheep, root, DoTs, sunder armor */
    damage = fire_wiretap(ship, con, gun, q);
    break;
  case HS_CAPACITOR:
    /* direct healing */
    damage = fire_capacitor(ship, con, gun, NULL);
    break;
  case HS_BOOSTER:
    /* periodic healing */
    damage = fire_booster(ship, con, gun, NULL);
    break;
  default:
    hs_std_notice(console, "Invalid weapon configuration.");
    return 0;
    break;
  }
  
  /* just do this for everybody */
  gun->loading = rint(get_wstat(ship, con, gun->type | HS_RELOAD));
  
  if (q && q->contact)
  {
    /* reset combat timers */
    ship->combat_timer = 0;
    ContactShip(q)->combat_timer = 0;

    if (!ship->cid && ContactShip(q)->cid)
      ship->cid = ContactShip(q)->cid;
    else if (ship->cid && !ContactShip(q)->cid)
      ContactShip(q)->cid = ship->cid;
    else if (!ship->cid && !ContactShip(q)->cid)
    {
      /* neither have a CID, start a new one */
      ship->cid = hs_num_combat_ids;
      ContactShip(q)->cid = hs_num_combat_ids++; /* increment here after setting both values */
    }
    /* if there is a conflict, switch to the lower numbered cid */
    else if (ship->cid < ContactShip(q)->cid)
    {
      ContactShip(q)->cid = ship->cid;
    }
    else if (ship->cid > ContactShip(q)->cid)
    {
      ship->cid = ContactShip(q)->cid;
    }
    
    /* increase target threat */
    qptr = find_ship_contact(ContactShip(q), ship);
    if (qptr)
      qptr->threat += damage;
  
    /* check to see if the target was destroyed and execute the UFUN */
    if (damage == HS_KILLSHOT)
    {
      obj = ContactObj(q);
      if (RealGoodObject(obj))
      {
        execute_ufun(obj, "FN_DESTROY", ship);
      }
    }
  }

  return 1;
}






