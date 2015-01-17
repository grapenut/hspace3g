
#include "hspace.h"


/*******************************************************/
/* emitter beam stances */

hbuff_effect STANCE_standard_beam =
{HS_EMITTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};
    
hbuff_effect STANCE_shield_beam =
{HS_EMITTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};
    
hbuff_effect STANCE_hull_beam =
{HS_EMITTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};
    
hbuff_effect STANCE_penetrate_beam =
{HS_EMITTER_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};


/*************************************************/
/* operations cooldowns */

hbuff_effect COOLDOWN_override =
{HS_OPS_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_quarantine =
{HS_OPS_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_purge_hack, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_reboot =
{HS_OPS_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_reload_ops, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_cache =
{HS_OPS_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_clear_memory, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_flush =
{HS_OPS_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_flush_memory, NULL, NULL, NULLWEAPON, NULLSYSTEM};


/*************************************************/
/* cooldown callbacks  */

HBUFF_CALL(purge_hack)
{
  hbuff *b;
  
  if (!ship)
    return;
  
  for (b = ship->head_buff; b; b = b->next)
  {
    if (!b->buff)
      continue;
    
    if (HasFlag(b->buff->flags, HS_DEBUFF))
    {
      b->duration = 0;
      b->owner = NULL;
      return;
    }
  }
}

HBUFF_CALL(reload_ops)
{
  int i;
  
  if (!ship)
    return;
  
  for (i = 0; i < ship->nconsoles; i++)
  {
    if (ship->console[i].type == HS_OPS)
    {
      ship->console[i].primary.loading = 0;
      ship->console[i].primary.curpower = get_wstat(ship, &(ship->console[i]), HS_PRIMARY | HS_MAX_POWER);
   
      ship->console[i].secondary.loading = 0;
      ship->console[i].secondary.curpower = get_wstat(ship, &(ship->console[i]), HS_PRIMARY | HS_MAX_POWER);
    }
  }
}

HBUFF_CALL(clear_memory)
{
  if (!ship)
    return;
  
  ship->computer.energy *= 0.5;
}

HBUFF_CALL(flush_memory)
{
  hbuff *b;
  
  if (!ship)
    return;
  
  ship->computer.energy = 0;
  
  /* cycle through buffs and clear out the hacks */
  for (b = ship->head_buff; b; b = b->next)
  {
    if (!b->buff)
      continue;
    
    if (HasFlag(b->buff->flags, HS_HACK))
    {
      b->duration = 0;
      b->owner = NULL;
    }
  }
}


/*************************************************/
/* hacks */

/* Weapon      NOTHING  rld  rng  dmg  acc  cnd  pow  cur  max  spd  loading */
/* System      NOTHING  cnd  rat  str  eff  nrg  max                         */

hbuff_effect HACK_shield =
{HS_HACK | HS_REFRESH, INT_MAX, 0, 50.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect HACK_reactor =
{HS_HACK | HS_REFRESH, INT_MAX, 0, 50.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect HACK_engine =
{HS_HACK | HS_REFRESH, INT_MAX, 0, 50.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect HACK_sensor =
{HS_HACK | HS_REFRESH, INT_MAX, 0, 50.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect HACK_computer =
{HS_HACK | HS_REFRESH, INT_MAX, 0, 50.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect HACK_weapon =
{HS_HACK | HS_REFRESH, INT_MAX, 0, 50.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};


/********************************************************/
/* wiretap, emitter and beam routines */

/* add a hack to the queue */
void use_hack(dbref console, char *which)
{
  hship *ship;
  hbuff_effect *bptr;
  hbuff *b;
  hconsole *con;
  double cost;
  
  /* check for a valid console and equipped wiretap */
  if (IsConsole(console))
  {
    con = find_console(console);
    if (!console)
    {
      hs_std_notice(console, "Invalid console object. Please contact your flight mechanic immediately!");
      return;
    }
    
    if (!HasFlag(con->primary.type, HS_WIRETAP) && !HasFlag(con->secondary.type, HS_WIRETAP))
    {
      hs_std_notice(console, "Must have a wiretap equipped to use hacks.");
      return;
    }
    
    ship = find_ship(console);
  }
  else if (IsShip(console))
  {
    ship = find_ship_by_nav(console);

    if (ship && !HasFlag(ship->primary.type, HS_WIRETAP) && !HasFlag(ship->secondary.type, HS_WIRETAP))
    {
      hs_std_notice(console, "Must have a wiretap equipped to use hacks.");
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
  
  bptr = find_effect(hs_hacks, which);
  if (!bptr || !HasFlag(bptr->flags, HS_HACK))
  {
    hs_std_notice(console, "Please specify a valid hack.");
    return;
  }
  
  if (find_buff(ship, bptr))
  {
    hs_std_notice(console, "That hack is already activated.");
    return;
  }
  
  /* check to see if we have enough power for this shunt */
  cost = bptr->cost * (1.0 - (get_stat(ship, SYS_COMPRESSION) / 100.0));
  if ((ship->computer.energy + cost) > get_stat(ship, SYS_MAX_MEMORY))
  {
    hs_std_notice(console, "Insufficient memory available.");
    return;
  }
  
  ship->computer.energy += cost;

  b = add_buff(ship, bptr);
  if (!b || !b->buff)
  {
    hs_std_notice(console, "Error adding the hack.");
    return;
  }
  
  notify_consoles(ship, tprintf("%s%s**%s %sOPS%s - Encrypting %s%s%s%s.", ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL,
          ANSI_HILITE, ANSI_NORMAL, ANSI_HILITE, ANSI_GREEN, STR(hs_hacks, b->buff), ANSI_NORMAL));
}


/* fire a wiretap, expending all of the queue hacks */
int fire_wiretap(hship *ship, hconsole *con, hweapon *gun, hcontact *q)
{
  hcontact *qptr;
  hbuff *b, *bptr;
  dbref console;
  char name[64];
  int count, acc;
  double dur;
  
  if (!ship || !gun || !q || !q->contact)
  {
    return HS_NODAMAGE;
  }
  
  if (con) console = con->objnum;
  else console = ship->objnum;
  
  /* accuracy and duration */
  acc = get_wstat(ship, con, gun->type | HS_ACCURACY) + get_stat(ship, SYS_ENCRYPTION) - get_stat(ContactShip(q), SYS_DECRYPTION);
  dur = get_wstat(ship, con, gun->type | HS_DAMAGE) / 100.0;

  /* cycle through and find all the hacking buffs and update owner/duration */
  count = 0;
  for (b = ship->head_buff; b; b = b->next)
  {
    if (!b->buff)
      continue;
    
    if (!HasFlag(b->buff->flags, HS_HACK))
      continue;
    
    /* set the local hack placeholder to expire */
    b->duration = 0;
    
    /* accuracy roll */
    if (getrandom(100) > acc)
    {
      hs_std_notice(console, tprintf("Failed to encrypt %s%s%s%s", ANSI_HILITE, ANSI_GREEN, STR(hs_hacks, b->buff), ANSI_NORMAL));
      continue;
    }
    
    /* find out which hack it is */
    if (b->buff == &HACK_shield)
      bptr = add_buff(ContactShip(q), &EFFECT_shield_failure);
    else if (b->buff == &HACK_reactor)
      bptr = add_buff(ContactShip(q), &EFFECT_reactor_failure);
    else if (b->buff == &HACK_engine)
      bptr = add_buff(ContactShip(q), &EFFECT_engine_failure);
    else if (b->buff == &HACK_sensor)
      bptr = add_buff(ContactShip(q), &EFFECT_sensor_failure);
    else if (b->buff == &HACK_computer)
      bptr = add_buff(ContactShip(q), &EFFECT_computer_failure);
    else if (b->buff == &HACK_weapon)
      bptr = add_buff(ContactShip(q), &EFFECT_weapon_failure);
    else
      bptr = NULL;
    
    if (!bptr)
    {
      hs_std_notice(console, "Error adding hack. Please contact your local flight mechanic immediately!");
      continue;
    }
    
    bptr->duration *= dur;

    /* decrease accuracy with each subsequent hack */
    acc -= 10;

    count++;
  }
  
  if (count < 1)
  {
    hs_std_notice(console, "There are no hacks to transmit.");
    return HS_NODAMAGE;
  }
  
  /* clear computer memory */
  ship->computer.energy = 0.0;
  
  hs_std_sensor(ship, q, tprintf("(%s%1dTB%s) Transmitting hack!", ANSI_HILITE, count, ANSI_NORMAL));

  /* only notify other contacts if it's a primary weapon */
  if (HasFlag(gun->type, HS_PRIMARY))
  {
    snprintf(name, 63, ship_name(ship));
    notify_contacts(ship, ContactShip(q), tprintf("%s%s%s hacks into %s%s%s!",
         ANSI_HILITE, name, ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));
  }
  
  qptr = find_ship_contact(ContactShip(q), ship);
  hs_std_sensor(ContactShip(q), qptr, tprintf("(%s%1dTB%s) Enemy is hacking systems!", ANSI_HILITE, count, ANSI_NORMAL));
  
  return HS_NODAMAGE;  
}


/* fire a particle emitter */
int fire_emitter(hship *ship, hconsole *con, hweapon *gun, hcontact *q)
{
  hcontact *qptr;
  hbuff *b, *stance;
  dbref console;
  char name[64];
  int ticks;
  double power, damage;
  
  if (!ship || !gun || !q || !q->contact)
  {
    return HS_NODAMAGE;
  }
  
  if (con) console = con->objnum;
  else console = ship->objnum;
  
  power = get_wstat(ship, con, gun->type | HS_POWER);
  ticks = gun->curpower / power / EFFECT_emitter_beam.frequency;
  if (ticks < 1)
    return 0;
    
  gun->curpower -= EFFECT_emitter_beam.frequency * ticks * power;
  
  /* add the buff. used the pointer to bypass stringmap lookup */
  b = add_buff(ship, &EFFECT_emitter_beam);
  if (!b)
  {
    hs_std_notice(console, "ERROR.");
    return HS_NODAMAGE;
  }
  
  b->owner = ContactShip(q);
  Accuracy(b) = get_wstat(ship, con, gun->type | HS_ACCURACY);
  Penetration(b) = 0.0;
  damage = get_wstat(ship, con, gun->type | HS_DAMAGE);

  /* ask for any emitter stance buff, it will return the one 
     we have. must check ourselves to see which it is */
  stance = find_buff(ship, &STANCE_standard_beam);
  if (stance)
  {
    if (stance->buff == &STANCE_shield_beam)
    {
      /* high shield damage */
      ShieldDamage(b) = 1.5 * damage;
      HullDamage(b) = 0.5 * damage;
    }
    else if (stance->buff == &STANCE_penetrate_beam)
    {
      /* ignore shields, low damage */
      ShieldDamage(b) = 0.5 * damage;
      HullDamage(b) = 0.5 * damage;
      Penetration(b) = 1.0;
    }
    else if (stance->buff == &STANCE_hull_beam)
    {
      /* high hull damage */
      ShieldDamage(b) = 0.5 * damage;
      HullDamage(b) = 1.5 * damage;
    } else {
      /* standard proton beam */
      ShieldDamage(b) = damage;
      HullDamage(b) = damage;
    }
  } else {
    ShieldDamage(b) = damage;
    HullDamage(b) = damage;
  }
  
  b->duration = ticks * EFFECT_emitter_beam.frequency;
  
  hs_std_sensor(ship, q, tprintf("(%s%2ds%s) Emitting particle beam!", ANSI_HILITE, b->duration, ANSI_NORMAL));

  /* only notify other contacts if it's a primary weapon */
  if (HasFlag(gun->type, HS_PRIMARY))
  {
    snprintf(name, 63, ship_name(ship));
    notify_contacts(ship, ContactShip(q), tprintf("%s%s%s emits particle beam at %s%s%s!",
         ANSI_HILITE, name, ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));
  }
  
  qptr = find_ship_contact(ContactShip(q), ship);
  hs_std_sensor(ContactShip(q), qptr, tprintf("(%s%2ds%s) Enemy emits particle beam!", ANSI_HILITE, b->duration, ANSI_NORMAL));
  
  return HS_NODAMAGE;  
}


/* callback for the periodic damage of an emitter debuff */
HBUFF_CALL(emitter_tick)
{
  hcontact *q, *qptr;
  double damage_shield, damage_hull;
  double penetration;
  int dtype;
  int acc_roll;
  char last_beam;
  double rand;
  char offense[256], defense[256];
  dbref obj;
  
  if (!ship || !buff || !buff->owner)
    return;
  
  q = find_ship_contact(ship, buff->owner);
  qptr = find_ship_contact(buff->owner, ship);
  

  acc_roll = getrandom(100);
  if (acc_roll > rint(Accuracy(buff)))
  {
    if (q && buff->duration <= 0)
    {
      hs_std_sensor(ship, q, tprintf("(%s  0%s) Emitter beam expired!", ANSI_GREEN, ANSI_NORMAL));
    }
    return;
  }
  
  if (buff->duration <= 0)
    last_beam = '*';
  else
    last_beam = ' ';
  
  /* default weapon damage is 100% to shields, 100% to hull, 0% penetration */
  damage_shield = ShieldDamage(buff);
  damage_hull = HullDamage(buff);
  penetration = Penetration(buff);
  
  /* emitters are energy damage, so apply energy damage reduction */
  rand = 0.75 + getrandom(1000)/2000.0;
  damage_shield *= rand - (get_stat(buff->owner, SYS_ABSORPTION) / 100.0);
  damage_hull *= rand - (get_stat(buff->owner, SYS_ABLATION) / 100.0);

  /* 0 no damage, 1 shield, 2 crit shield, 3 hull, 4 crit hull, 5 disable, 6 destroy */
  dtype = do_damage(buff->owner, damage_shield, damage_hull, penetration);
  switch (dtype)
  {
  case HS_KILLSHOT:
    sprintf(offense, "(%s%3.0f%s) Emitter beam has %s%sdestroyed%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy emitter beam has %s%sdestroyed%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
    break;
  case HS_DISABLESHOT:
    sprintf(offense, "(%s%3.0f%s) Emitter beam has %s%sdisabled%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy emitter beam has %s%sdisabled%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
    break;
  case HS_CRITHULL:
    sprintf(offense, "(%s%3.0f%s) Emitter beam slices through enemy hull!%c", ANSI_GREEN, damage_hull, ANSI_NORMAL, last_beam);
    sprintf(defense, "(%s%3.0f%s) Enemy emitter beam slices through hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_MINHULL:
    sprintf(offense, "(%s%3.0f%s) Emitter beam hits enemy hull!%c", ANSI_GREEN, damage_hull, ANSI_NORMAL, last_beam);
    sprintf(defense, "(%s%3.0f%s) Enemy emitter beam hits hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_CRITSHIELD:
    sprintf(offense, "(%s%3.0f%s) Emitter beam slices through enemy shields!%c", ANSI_GREEN, damage_shield, ANSI_NORMAL, last_beam);
    sprintf(defense, "(%s%3.0f%s) Enemy emitter beam slices through shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  case HS_MINSHIELD:
    sprintf(offense, "(%s%3.0f%s) Emitter beam absorbed by enemy shields!%c", ANSI_GREEN, damage_shield, ANSI_NORMAL, last_beam);
    sprintf(defense, "(%s%3.0f%s) Enemy emitter beam absorbed by shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  default:
    sprintf(offense, "(%s  0%s) Emitter beam ineffective.", ANSI_GREEN, ANSI_NORMAL);
    sprintf(defense, "(%s  0%s) Enemy emitter beam ineffective.", ANSI_RED, ANSI_NORMAL);
    break;
  }
  
  hs_std_sensor(ship, q, offense);
  hs_std_sensor(buff->owner, qptr, defense);
  
  /* check to see if the target was destroyed and execute the UFUN */
  if (dtype == HS_KILLSHOT)
  {
    obj = ship->objnum;
    if (RealGoodObject(obj))
    {
      execute_ufun(obj, "FN_DESTROY", buff->owner);
    }
  }

  return;
}


HBUFF_CALL(ops_cooldown)
{
  return;
}







