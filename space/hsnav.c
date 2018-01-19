
#include "hspace.h"


/*******************************************************/
/* missile stances */

hbuff_effect STANCE_standard_missile =
{HS_MISSILE_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect STANCE_shield_missile =
{HS_MISSILE_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect STANCE_hull_missile =
{HS_MISSILE_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect STANCE_penetrate_missile =
{HS_MISSILE_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};


/*******************************************************/
/* navigation cooldowns */

hbuff_effect COOLDOWN_overheat =
{HS_NAV_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_juke =
{HS_NAV_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_purge_missile, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_jive =
{HS_NAV_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_burst =
{HS_NAV_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_vanish =
{HS_NAV_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_vanish, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_reload =
{HS_NAV_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_reload_nav, NULL, NULL, NULLWEAPON, NULLSYSTEM};


HBUFF_CALL(vanish)
{
  ship->combat_timer = 30;
}


HBUFF_CALL(reload_nav)
{
  int i;
  
  if (!ship)
    return;
  
  ship->primary.loading = 0;
  ship->primary.curpower = get_wstat(ship, NULL, HS_PRIMARY | HS_MAX_POWER);
  
  ship->secondary.loading = 0;
  ship->secondary.curpower = get_wstat(ship, NULL, HS_PRIMARY | HS_MAX_POWER);
}


/********************************************************/
/* laser gun and missile routines */

/* fire a gun */
int fire_gun(hship *ship, hconsole *con, hweapon *gun, hcontact *q)
{
  hcontact *qptr;
  double damage, damage_hull, damage_shield;
  int dtype;
  char name[64];
  char offense[256], defense[256];
  
  if (!ship || !gun || !q || !q->contact)
    return 0;
  
  qptr = find_ship_contact(ContactShip(q), ship);

  if (getrandom(100) > rint(get_wstat(ship, con, gun->type | HS_ACCURACY)))
  {
    switch (getrandom(4))
    {
    case 0:
      sprintf(offense, "(%s  0%s) Weapons fire misses wide to the port.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy weapons fire misses wide to the port.", ANSI_RED, ANSI_NORMAL);
      break;
    case 1:
      sprintf(offense, "(%s  0%s) Weapons fire misses wide to the starboard.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy weapons fire misses wide to the starboard.", ANSI_RED, ANSI_NORMAL);
      break;
    case 2:
      sprintf(offense, "(%s  0%s) Weapons fire misses off the enemy bow.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy weapons fire misses off the bow.", ANSI_RED, ANSI_NORMAL);
      break;
    case 3:
      sprintf(offense, "(%s  0%s) Weapons fire misses off the enemy stern.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy weapons fire misses off the stern.", ANSI_RED, ANSI_NORMAL);
      break;
    default:
      sprintf(offense, "(%s  0%s) Weapons fire misses wildly.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy weapons fire misses wildly.", ANSI_RED, ANSI_NORMAL);
      break;
    }

    /* notify the contacts */
    hs_std_sensor(ship, q, offense);
    hs_std_sensor(ContactShip(q), qptr, defense);
    
    return 0;
  }
  
  /* default weapon damage is 100% to shields, 100% to hull, 0% penetration */
  damage = get_wstat(ship, con, gun->type | HS_DAMAGE) * (0.75 + getrandom(1000)/2000.0);
  
  /* lasers are energy damage, so apply energy damage reduction */
  damage_shield = damage * (1.0 - (get_stat(ContactShip(q), SYS_ABSORPTION) / 100.0));
  damage_hull = damage * (1.0 - (get_stat(ContactShip(q), SYS_ABLATION) / 100.0));

  /* 0 no damage, 1 shield, 2 crit shield, 3 hull, 4 crit hull, 5 disable, 6 destroy */
  dtype = do_damage(ContactShip(q), damage_shield, damage_hull, 0.0);
  switch (dtype)
  {
  case HS_KILLSHOT:
    sprintf(offense, "(%s%3.0f%s) Weapons fire has %s%sdestroyed%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy weapons fire has %s%sdestroyed%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
    break;
  case HS_DISABLESHOT:
    sprintf(offense, "(%s%3.0f%s) Weapons fire has %s%sdisabled%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy weapons fire has %s%sdisabled%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
    break;
  case HS_CRITHULL:
    sprintf(offense, "(%s%3.0f%s) Weapons fire slams into the enemy hull!", ANSI_GREEN, damage_hull, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy weapons fire slams into the hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_MINHULL:
    sprintf(offense, "(%s%3.0f%s) Weapons fire impacts the enemy hull!", ANSI_GREEN, damage_hull, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy weapons fire impacts the hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_CRITSHIELD:
    sprintf(offense, "(%s%3.0f%s) Weapons fire slams into the enemy shields!", ANSI_GREEN, damage_shield, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy weapons fire slams into the shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  case HS_MINSHIELD:
    sprintf(offense, "(%s%3.0f%s) Weapons fire is absorbed by the enemy shields!", ANSI_GREEN, damage_shield, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy weapons fire is absorbed by the shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  default:
    sprintf(offense, "(%s%3.0f%s) Weapons fire is ineffective.", ANSI_GREEN, 0.0, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy weapons fire is ineffective.", ANSI_RED, 0.0, ANSI_NORMAL);
    break;
  }
  
  /* notify the contacts */
  hs_std_sensor(ship, q, offense);
  hs_std_sensor(ContactShip(q), qptr, defense);
  
  /* only notify other contacts if this is a primary weapon */
  if (HasFlag(gun->type, HS_PRIMARY))
  {
    strncpy(name, ship_name(ship), 63);
    notify_contacts(ship, ContactShip(q), tprintf("%s%s%s fires weapons at %s%s%s!",
         ANSI_HILITE, name, ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));
  }

  return dtype;
}


/* fire a missile */
int fire_missile(hship *ship, hconsole *con, hweapon *gun, hcontact *q)
{
  hcontact *qptr;
  hbuff *b, *stance;
  char name[64];
  double damage;
  
  if (!ship || !gun || !q || !q->contact)
  {
    return HS_NODAMAGE;
  }
  
  /* launch missile by adding debuff */
  b = add_buff(ContactShip(q), &EFFECT_missile);
  if (!b)
  {
    return HS_NODAMAGE;
  }
  
  b->owner = ship;
  Accuracy(b) = get_wstat(ship, con, gun->type | HS_ACCURACY);
  Penetration(b) = 0.0;
  damage = get_wstat(ship, con, gun->type | HS_DAMAGE) * (0.75 + getrandom(1000)/2000.0);

  /* ask for any stance buff, it will return the one 
     we have. must check ourselves to see which it is */
  stance = find_buff(ship, &STANCE_standard_missile);
  if (stance)
  {
    if (stance->buff == &STANCE_hull_missile) {
      /* high hull damage */
      ShieldDamage(b) = 0.5 * damage;
      HullDamage(b) = 1.5 * damage;
    }
    else if (stance->buff == &STANCE_shield_missile)
    {
      /* more shield damage */
      ShieldDamage(b) = 1.5 * damage;
      HullDamage(b) = 0.5 * damage;
    }
    else if (stance->buff == &STANCE_penetrate_missile)
    {
      /* lower damage, minor penetration */
      if (HasFlag(ContactShip(q)->type, HS_CAPITAL) && HasFlag(gun->type, HS_TORPEDO))
      {
        ShieldDamage(b) = 0.0;
        HullDamage(b) = 100.0 * damage;
      } else {
        ShieldDamage(b) = 0.5 * damage;
        HullDamage(b) = 0.5 * damage;
      }
      
      Penetration(b) = 1.0;
    } else {
      /* standard heat seeker */
      ShieldDamage(b) = damage;
      HullDamage(b) = damage;
    }
  } else {
    /* standard heat seeker */
    ShieldDamage(b) = damage;
    HullDamage(b) = damage;
  }
  
  b->duration = 3 + rint(ship_distance(ship, ContactShip(q)) / get_wstat(ship, con, gun->type | HS_SPEED));
  
  hs_std_sensor(ship, q, tprintf("(%s%2ds%s) Missile away!", ANSI_HILITE, b->duration, ANSI_NORMAL));
  
  /* only notify other contacts if it's the primary weapon slot */
  if (HasFlag(gun->type, HS_PRIMARY))
  {
    strncpy(name, ship_name(ship), 63);
    notify_contacts(ship, ContactShip(q), tprintf("%s%s%s fires a missile at %s%s%s!",
         ANSI_HILITE, name, ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));
  }

  qptr = find_ship_contact(ContactShip(q), ship);
  hs_std_sensor(ContactShip(q), qptr, tprintf("(%s%2ds%s) Enemy missile incoming!", ANSI_HILITE, b->duration, ANSI_NORMAL));
  
  return HS_NODAMAGE;  
}


/* callback for the explosion of a missile debuff */
HBUFF_CALL(missile_explosion)
{
  hcontact *q, *qptr;
  double damage_shield, damage_hull;
  int dtype;
  char offense[256], defense[256];
  dbref obj;
  
  if (!ship || !buff)
    return;
  
  q = find_ship_contact(buff->owner, ship);
  qptr = find_ship_contact(ship, buff->owner);
  
  if (getrandom(100) > rint(Accuracy(buff)))
  {
    switch (getrandom(4))
    {
    case 0:
      sprintf(offense, "(%s  0%s) Missile misses wide to the port.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy missile misses wide to the port.", ANSI_RED, ANSI_NORMAL);
      break;
    case 1:
      sprintf(offense, "(%s  0%s) Missile misses wide to the starboard.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy missile misses wide to the starboard.", ANSI_RED, ANSI_NORMAL);
      break;
    case 2:
      sprintf(offense, "(%s  0%s) Missile misses off the enemy bow.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy missile misses off the bow.", ANSI_RED, ANSI_NORMAL);
      break;
    case 3:
      sprintf(offense, "(%s  0%s) Missile misses off the enemy stern.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy missile misses off the stern.", ANSI_RED, ANSI_NORMAL);
      break;
    default:
      sprintf(offense, "(%s  0%s) Missile misses wildly.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy missile misses wildly.", ANSI_RED, ANSI_NORMAL);
      break;
    }

    hs_std_sensor(buff->owner, q, offense);
    hs_std_sensor(ship, qptr, defense);
  
    return;
  }
  
  /* default weapon damage is 100% to shields, 100% to hull, 0% penetration */
  damage_shield = ShieldDamage(buff);
  damage_hull = HullDamage(buff);
  
  /* missiles are kinetic damage, so apply kinetic damage reduction */
  damage_shield *= 1.0 - (get_stat(ship, SYS_DEFLECTION) / 100.0);
  damage_hull *= 1.0 - (get_stat(ship, SYS_TOUGHNESS) / 100.0);

  /* 0 no damage, 1 shield, 2 crit shield, 3 hull, 4 crit hull, 5 disable, 6 destroy */
  dtype = do_damage(ship, damage_shield, damage_hull, Penetration(buff));
  switch (dtype)
  {
  case HS_KILLSHOT:
    sprintf(offense, "(%s%3.0f%s) Missile has %s%sdestroyed%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy missile has %s%sdestroyed%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
    break;
  case HS_DISABLESHOT:
    sprintf(offense, "(%s%3.0f%s) Missile has %s%sdisabled%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy missile has %s%sdisabled%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
    break;
  case HS_CRITHULL:
    sprintf(offense, "(%s%3.0f%s) Missile slams into the enemy hull!", ANSI_GREEN, damage_hull, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy missile slams into the hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_MINHULL:
    sprintf(offense, "(%s%3.0f%s) Missile impacts the enemy hull!", ANSI_GREEN, damage_hull, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy missile impacts the hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_CRITSHIELD:
    sprintf(offense, "(%s%3.0f%s) Missile slams into the enemy shields!", ANSI_GREEN, damage_shield, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy missile slams into the shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  case HS_MINSHIELD:
    sprintf(offense, "(%s%3.0f%s) Missile is absorbed by the enemy shields!", ANSI_GREEN, damage_shield, ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy missile is absorbed by the shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  default:
    sprintf(offense, "(%s  0%s) Missile is ineffective.", ANSI_GREEN, ANSI_NORMAL);
    sprintf(defense, "(%s  0%s) Enemy missile is ineffective.", ANSI_RED, ANSI_NORMAL);
    break;
  }

  hs_std_sensor(buff->owner, q, offense);
  hs_std_sensor(ship, qptr, defense);
  
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

    
HBUFF_CALL(nav_cooldown)
{
  return;
}


