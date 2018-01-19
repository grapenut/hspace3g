
#include "hspace.h"


/************************************************/
/* cannon residue stances */

hbuff_effect STANCE_standard_cannon =
{HS_CANNON_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect STANCE_shield_cannon =
{HS_CANNON_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect STANCE_hull_cannon =
{HS_CANNON_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect STANCE_penetrate_cannon =
{HS_CANNON_STANCE | HS_REFRESH, INT_MAX, 0, 0.0, NULL, NULL, NULL, NULLWEAPON, NULLSYSTEM};


/************************************************************/
/* gunnery cooldowns */

hbuff_effect COOLDOWN_overload =
{HS_GUN_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_sixoclock =
{HS_GUN_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_purge_missile, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_sniper =
{HS_GUN_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_pierce =
{HS_GUN_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_rapid =
{HS_GUN_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_activate_cooldown, NULL, NULL, NULLWEAPON, NULLSYSTEM};

hbuff_effect COOLDOWN_ignite =
{HS_GUN_COOLDOWN | HS_PRE, 60, 0, 10.0, buffcall_ignite_residue, NULL, NULL, NULLWEAPON, NULLSYSTEM};

HBUFF_CALL(purge_missile)
{
  hbuff *b;
  
  if (!ship)
    return;
  
  for (b = ship->head_buff; b; b = b->next)
  {
    if (!b->buff)
      continue;
    
    if (b->buff == &EFFECT_missile)
    {
      b->duration = 0;
      b->owner = NULL;
      return;
    }
  }
}

HBUFF_CALL(ignite_residue)
{
  int i;
  hbuff *b;
  hconsole *con;
  
  if (!ship)
    return;
  
  /* cycle through consoles looking for gunnery turret */
  /* ignite residue on the lock target */
  
  for (i = 0; i < ship->nconsoles; i++)
  {
    if (HasFlag(ship->console[i].type, HS_GUN))
    {
      con = &(ship->console[i]);
      if (con->lock && con->lock->contact)
      {
        for (b = ContactShip(con->lock)->head_buff; b; b = b->next)
        {
          if (!b->buff)
            continue;
            
          if (b->buff == &EFFECT_residue)
          {
            b->duration = 0;
            return;
          }
        }
      }
    }
  }
}


/************************************************************/
/* cannon and residue routines */

/* fire a gun */
int fire_cannon(hship *ship, hconsole *con, hweapon *gun, hcontact *q)
{
  hcontact *qptr;
  double damage, damage_hull, damage_shield, power;
  int dtype, ticks;
  char name[64];
  char offense[256], defense[256];
  hbuff *b, *stance;
  
  if (!ship || !gun || !q || !q->contact)
    return 0;
  
  qptr = find_ship_contact(ContactShip(q), ship);

  /* calculate number of shots fired */
  power = get_wstat(ship, con, gun->type | HS_POWER);
  ticks = gun->curpower / power;
  if (ticks < 1)
    return 0;
  
  gun->curpower -= ticks * power;

  if (getrandom(100) > rint(get_wstat(ship, con, gun->type | HS_ACCURACY)))
  {
    switch (getrandom(4))
    {
    case 0:
      sprintf(offense, "(%s  0%s) Cannon blast misses wide to the port.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy cannon blast misses wide to the port.", ANSI_RED, ANSI_NORMAL);
      break;
    case 1:
      sprintf(offense, "(%s  0%s) Cannon blast misses wide to the starboard.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy cannon blast misses wide to the starboard.", ANSI_RED, ANSI_NORMAL);
      break;
    case 2:
      sprintf(offense, "(%s  0%s) Cannon blast misses off the enemy bow.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy cannon blast misses off the bow.", ANSI_RED, ANSI_NORMAL);
      break;
    case 3:
      sprintf(offense, "(%s  0%s) Cannon blast misses off the enemy stern.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy cannon blast misses off the stern.", ANSI_RED, ANSI_NORMAL);
      break;
    default:
      sprintf(offense, "(%s  0%s) Cannon blast misses wildly.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy cannon blast misses wildly.", ANSI_RED, ANSI_NORMAL);
      break;
    }

    hs_std_sensor(ship, q, offense);
    hs_std_sensor(ContactShip(q), qptr, defense);
    
    return 0;
  }
  
  /* default weapon damage is 100% to shields, 100% to hull, 0% penetration */
  /* lasers are energy damage, so apply energy damage reduction */
  damage = ticks * get_wstat(ship, con, gun->type | HS_DAMAGE) * (0.75 + getrandom(1000)/2000.0);
  damage_shield = damage * (1.0 - (get_stat(ContactShip(q), SYS_ABSORPTION) / 100.0));
  damage_hull = damage * (1.0 - (get_stat(ContactShip(q), SYS_ABLATION) / 100.0));

  /* 0 no damage, 1 shield, 2 crit shield, 3 hull, 4 crit hull, 5 disable, 6 destroy */
  dtype = do_damage(ContactShip(q), damage_shield, damage_hull, 0.0);
  switch (dtype)
  {
  case HS_KILLSHOT:
      sprintf(offense, "(%s%3.0f%s) Cannon blast has %s%sdestroyed%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ShipName(q->contact));
      sprintf(defense, "(%s%3.0f%s) Enemy cannon blast has %s%sdestroyed%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
    break;
  case HS_DISABLESHOT:
      sprintf(offense, "(%s%3.0f%s) Cannon blast has %s%sdisabled%s the %s!", ANSI_GREEN, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ShipName(q->contact));
      sprintf(defense, "(%s%3.0f%s) Enemy cannon blast has %s%sdisabled%s us!", ANSI_RED, damage_hull, ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
    break;
  case HS_CRITHULL:
      sprintf(offense, "(%s%3.0f%s) Cannon blast gouges the enemy hull!", ANSI_GREEN, damage_hull, ANSI_NORMAL);
      sprintf(defense, "(%s%3.0f%s) Enemy cannon blast gouges the hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_MINHULL:
      sprintf(offense, "(%s%3.0f%s) Cannon blast sears the enemy hull!", ANSI_GREEN, damage_hull, ANSI_NORMAL);
      sprintf(defense, "(%s%3.0f%s) Enemy cannon blast sears the hull!", ANSI_RED, damage_hull, ANSI_NORMAL);
    break;
  case HS_CRITSHIELD:
      sprintf(offense, "(%s%3.0f%s) Cannon blast gouges the enemy shields!", ANSI_GREEN, damage_shield, ANSI_NORMAL);
      sprintf(defense, "(%s%3.0f%s) Enemy cannon blast gouges the shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  case HS_MINSHIELD:
      sprintf(offense, "(%s%3.0f%s) Cannon blast is absorbed by the enemy shields!", ANSI_GREEN, damage_shield, ANSI_NORMAL);
      sprintf(defense, "(%s%3.0f%s) Enemy cannon blast is absorbed by the shields!", ANSI_RED, damage_shield, ANSI_NORMAL);
    break;
  default:
      sprintf(offense, "(%s  0%s) Cannon blast is ineffective.", ANSI_GREEN, ANSI_NORMAL);
      sprintf(defense, "(%s  0%s) Enemy cannon blast is ineffective.", ANSI_RED, ANSI_NORMAL);
    break;
  }
  
  hs_std_sensor(ship, q, offense);
  hs_std_sensor(ContactShip(q), qptr, defense);
  
  strncpy(name, ship_name(ship), 63);
  notify_contacts(ship, ContactShip(q), tprintf("%s%s%s fires cannons at %s%s%s!",
       ANSI_HILITE, name, ANSI_NORMAL, ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));

  /* if damage is successful, apply the residue debuff */
  if (dtype)
  {
    b = add_buff(ContactShip(q), &EFFECT_residue);
    if (!b)
      return dtype;
    
    b->owner = ship;
    
    stance = find_buff(ship, &STANCE_standard_cannon);
    if (stance)
    {
      if (stance->buff == &STANCE_shield_cannon)
      {
        /* high shield damage */
        ShieldDamage(b) += 0.5 * damage_shield;
      }
      else if (stance->buff == &STANCE_penetrate_cannon)
      {
        /* ignore shields */
        Penetration(b) = 1.0;
      }
      else if (stance->buff == &STANCE_hull_cannon)
      {
        /* high hull damage only */
        HullDamage(b) += 0.5 * damage_hull;
      } else {
        /* standard mass driver */
        ShieldDamage(b) += 0.25 * damage_shield;
        HullDamage(b) += 0.25 * damage_hull;
      }
    } else {
      /* standard mass driver */
      ShieldDamage(b) += 0.25 * damage_shield;
      HullDamage(b) += 0.25 * damage_hull;
    }
  }
  
  return dtype;
}

/* when plasma residue expires, it does some damage */
HBUFF_CALL(residue_explosion)
{
  hcontact *q, *qptr;
  char offense[256];
  char defense[256];
  int dtype;
  dbref obj;
  
  if (!ship || !buff || !buff->buff)
    return;
  
  q = find_ship_contact(buff->owner, ship);
  qptr = find_ship_contact(ship, buff->owner);
  
  dtype = do_damage(ship, ShieldDamage(buff), HullDamage(buff), Penetration(buff));
  switch (dtype)
  {
  case HS_KILLSHOT:
    sprintf(offense, "(%s%3.0f%s) Residue has %s%sdestroyed%s the %s!", ANSI_GREEN, HullDamage(buff), ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy residue has %s%sdestroyed%s us!", ANSI_RED, HullDamage(buff), ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL);
    break;
  case HS_DISABLESHOT:
    sprintf(offense, "(%s%3.0f%s) Residue has %s%sdisabled%s the %s!", ANSI_GREEN, HullDamage(buff), ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL, ShipName(q->contact));
    sprintf(defense, "(%s%3.0f%s) Enemy residue has %s%sdisabled%s us!", ANSI_RED, HullDamage(buff), ANSI_NORMAL, ANSI_HILITE, ANSI_YELLOW, ANSI_NORMAL);
    break;
  case HS_CRITHULL:
    sprintf(offense, "(%s%3.0f%s) Residue gouges the enemy hull!", ANSI_GREEN, HullDamage(buff), ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy residue gouges the hull!", ANSI_RED, HullDamage(buff), ANSI_NORMAL);
    break;
  case HS_MINHULL:
    sprintf(offense, "(%s%3.0f%s) Residue sears the enemy hull!", ANSI_GREEN, HullDamage(buff), ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy residue sears the hull!", ANSI_RED, HullDamage(buff), ANSI_NORMAL);
    break;
  case HS_CRITSHIELD:
    sprintf(offense, "(%s%3.0f%s) Residue gouges the enemy shields!", ANSI_GREEN, ShieldDamage(buff), ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy residue gouges the shields!", ANSI_RED, ShieldDamage(buff), ANSI_NORMAL);
    break;
  case HS_MINSHIELD:
    sprintf(offense, "(%s%3.0f%s) Residue is absorbed by the enemy shields!", ANSI_GREEN, ShieldDamage(buff), ANSI_NORMAL);
    sprintf(defense, "(%s%3.0f%s) Enemy residue is absorbed by the shields!", ANSI_RED, ShieldDamage(buff), ANSI_NORMAL);
    break;
  default:
    sprintf(offense, "(%s  0%s) Residue is ineffective.", ANSI_GREEN, ANSI_NORMAL);
    sprintf(defense, "(%s  0%s) Enemy residue is ineffective.", ANSI_RED, ANSI_NORMAL);
    break;
  }
  
  /* send the sensor messages */
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


HBUFF_CALL(gun_cooldown)
{
  return;
}








