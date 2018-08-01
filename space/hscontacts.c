
#include "hspace.h"


hstringmap hs_factions[] =
{
  { "TC", (void*) HS_IDENT_TC },
  { "EK", (void*) HS_IDENT_EK },
  { "BW", (void*) HS_IDENT_BW },
  { "SS", (void*) HS_IDENT_CIV },
  { "XX", (void*) HS_IDENT_PIRATE },
  {NULL, NULL}
};


/**********************************************************/
/* contact info utility routines */

int check_ship_size(hship *ship)
{
  int size;
  
  if (!ship)
    return 0;
  
  size = ship->nconsoles + 1;
  
  if (HasFlag(ship->type, HS_CAPITAL))
  {
    size += 4;
  }
  
  if (HasFlag(ship->type, HS_STATION))
  {
    size = 19;
  }
  
  return size;
}

/* friend = 1, enemy = 0, neutral = -1 */
int check_friend(hship *ship, hcontact *q)
{
  ATTR *a, *b;
  int fac, tfac;
  char id[8];
  hship *target;
  
  if (!ship || !q || !q->contact)
    return -1;
  
  if (!HasFlag(q->type, HS_SHIP))
    return -1;
  
  if (!HasFlag(q->flags, HS_IDENTIFIED))
    return -1;
  
  target = ContactShip(q);
  
  if (!RealGoodObject(ship->objnum) || !RealGoodObject(target->objnum))
    return -1;
    
  a = atr_get(ship->objnum, "IDENT");
  if (!a)
    return -1;

  b = atr_get(target->objnum, "IDENT");
  if (!b)
    return -1;
  
  strncpy(id, atr_value(a), 2);
  fac = parse_flags(hs_factions, id);

  strncpy(id, atr_value(b), 2);
  tfac = parse_flags(hs_factions, id);
  
  if (fac == tfac)
    return 1;
  
  switch (fac)
  {
  case HS_IDENT_TC:
    if (HasFlag(HS_ENEMIES_TC, tfac))
      return 0;
    break;
  case HS_IDENT_EK:
    if (HasFlag(HS_ENEMIES_EK, tfac))
      return 0;
    break;
  case HS_IDENT_BW:
    if (HasFlag(HS_ENEMIES_BW, tfac))
      return 0;
    break;
  case HS_IDENT_CIV:
    if (HasFlag(HS_ENEMIES_CIV, tfac))
      return 0;
    break;
  case HS_IDENT_PIRATE:
    if (HasFlag(HS_ENEMIES_PIRATE, tfac))
      return 0;
    break;
  default:
    return -1;
    break;
  }
  
  return -1;
}


/* return the name of a ship, celestial, or generic contact */
const char *ship_name(hship *ship)
{
  ATTR *a;
  dbref obj;
  
  if (!ship || !RealGoodObject(ship->objnum))
    return NULL;
  
  obj = atr_parse_dbref(ship->objnum, "SHIPOBJ");
  if (RealGoodObject(obj))
    return Name(obj);

  a = atr_get(ship->objnum, "NAME");
  if (a)
    return atr_value(a);

  return Name(ship->objnum);
  
  return "Unknown";
}

const char *celestial_name(hcelestial *cel)
{
  if (!cel)
    return NULL;
  
  switch (HasFlag(cel->type, HS_OBJECT_FLAGS))
  {
  case HS_WAYPOINT:
  case HS_WAYPOINT | HS_ANOMALY:
    if (RealGoodObject(cel->objnum))
      return Name(cel->objnum);

    return "Nav Point";
    break;
  case HS_DEBRIS:
    if (HasFlag(cel->type, HS_CARGO))
      return "Cargo Pod";
    else
      return "Debris";
    break;
  default:
    if (RealGoodObject(cel->objnum))
      return Name(cel->objnum);
    break;
  }

  return "Unknown";
}

const char *contact_name(hcontact *q)
{
  if (!q || !q->contact)
    return NULL;
  
  if (HasFlag(q->type, HS_ANY_SHIP))
    return ship_name(ContactShip(q));
  else if (HasFlag(q->type, HS_ANY_CELESTIAL))
    return celestial_name(ContactCelestial(q));
  
  return NULL;
}


/* return a contacts objnum */
dbref contact_objnum(hcontact *q)
{
  if (!q || !q->contact)
    return NOTHING;
  
  if (HasFlag(q->type, HS_ANY_SHIP))
    return ContactShip(q)->objnum;
  else if (HasFlag(q->type, HS_ANY_CELESTIAL))
    return ContactCelestial(q)->objnum;
  
  return NOTHING;
}


/* return the distance between 2 objects */
double ship_distance(hship *s, hship *t)
{
  if (!s || !t)
    return 1.0e100;
  
  return dist3d(s->x, s->y, s->z, t->x, t->y, t->z);
}

double ship_celestial_distance(hship *s, hcelestial *t)
{
  if (!s || !t)
    return 1.0e100;
  
  return dist3d(s->x, s->y, s->z, t->x, t->y, t->z);
}

double celestial_distance(hcelestial *s, hcelestial *t)
{
  if (!s || !t)
    return 1.0e100;
  
  return dist3d(s->x, s->y, s->z, t->x, t->y, t->z);
}

double contact_distance(hcontact *s, hcontact *t)
{
  if (!s || !t || !s->contact || !t->contact)
    return 1.0e100;
  
  if (HasFlag(s->type, HS_ANY_SHIP))
  {
    if (HasFlag(t->type, HS_ANY_SHIP))
      return ship_distance(ContactShip(s), ContactShip(t));
    else
      return ship_celestial_distance(ContactShip(s), ContactCelestial(t));
  }
  else
  {
    if (HasFlag(t->type, HS_ANY_SHIP))
      return ship_celestial_distance(ContactShip(t), ContactCelestial(s));
    else
      return celestial_distance(ContactCelestial(s), ContactCelestial(t));
  }

  return 1.0e100;
}


/* get xy and z angles between contacts */
int contact_xyang(hship *s, hcontact *t)
{
  double sx, sy;
  double tx, ty;
  
  if (!s || !t || !t->contact)
    return 0;
  
  sx = s->x;
  sy = s->y;
  
  if (HasFlag(t->type, HS_ANY_SHIP))
  {
    tx = ContactShip(t)->x;
    ty = ContactShip(t)->y;
  } else {
    tx = ContactCelestial(t)->x;
    ty = ContactCelestial(t)->y;
  }
  
  return xyang(sx, sy, tx, ty);
}

int contact_zang(hship *s, hcontact *t)
{
  double sx, sy, sz;
  double tx, ty, tz;
  
  if (!s || !t || !t->contact)
    return 0;
  
  sx = s->x;
  sy = s->y;
  sz = s->z;

  if (HasFlag(t->type, HS_ANY_SHIP))
  {
    tx = ContactShip(t)->x;
    ty = ContactShip(t)->y;
    tz = ContactShip(t)->z;
  } else {
    tx = ContactCelestial(t)->x;
    ty = ContactCelestial(t)->y;
    tz = ContactCelestial(t)->z;
  }
  
  return zang(sx, sy, sz, tx, ty, tz);
}


/* return the head_contact, accounting for mothership telemetry sharing */
hcontact *get_head_contact(hship *ship)
{
  if (!ship)
    return NULL;
  
  if (ship->mothership)
    return ship->mothership->head_contact;
  
  return ship->head_contact;
}



/****************************************************/
/* target lock routines */

/* return the target that ship is locked on to */
hship *get_lock(hship *ship)
{
  if (!ship || !ship->lock || !ship->lock->contact)
    return NULL;

  if (!HasFlag(ship->lock->type, HS_ANY_SHIP))
    return NULL;

  return ContactShip(ship->lock);
}


/* check if ship has weapons lock on target */
int has_lock(hship *ship, hship *target)
{
  hship *lock;
  
  if (!ship || !target)
    return 0;
  
  lock = get_lock(ship);
  if (!lock)
    return 0;
  
  if (lock == target)
    return 1;
  
  return 0;
}

void change_lock(hship *ship, hconsole *con, hcontact *q)
{
  int cnum;
  hcontact *qptr, **lptr;
  dbref console;
  
  if (!ship)
    return;
    
  if (!con)
  {
    console = ship->objnum;
    lptr = &(ship->lock);
  }
  else
  {
    console = con->objnum;
    lptr = &(con->lock);
  }

  /* no target specified, must be clearing the lock */
  if (!q || !q->contact)
  {
    /* if we have a current lock, notify them of our mercy */
    if (*lptr)
    {
      if ((*lptr)->contact)
      {
        qptr = find_ship_contact(ContactShip(*lptr), ship);
        if (qptr)
        {
          if (HasFlag(qptr->flags, HS_IDENTIFIED))
          {
            hs_std_sensor(ContactShip(*lptr), qptr, tprintf("%s has released target lock.", ContactName(qptr)));
          } else {
            hs_std_sensor(ContactShip(*lptr), qptr, "Unidentified contact has released target lock.");
          }
        } else {
          notify_consoles(ContactShip(*lptr), tprintf("%s%s-%s An unknown source has released target lock.",
                          ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
        }
      }
      
      hs_std_notice(console, "Target lock released.");
      *lptr = NULL;
    } else {
      hs_std_notice(console, "No target locked.");
    }
    
    return;
  }
  
  if (!HasFlag(q->type, HS_ANY_SHIP))
  {
    hs_std_notice(console, "Sensor contact is not a valid target.");
    return;
  }
  
  if (ship->uid && !ship->uid->pvp)
  {
    if (!HasFlag(ship->type, HS_DRONE) && !HasFlag(q->type, HS_DRONE))
    {
      if (!HasFlag(ship->type, HS_PVP))
      {
        hs_std_notice(console, "You have not enabled Player Vs Player combat.");
        return;
      }
      
      if (!HasFlag(ContactShip(q)->type, HS_PVP))
      {
        hs_std_notice(console, "Target has not enabled Player Vs Player combat.");
        return;
      }
    }
  }
  
  /* find the requested sensor contact */
  /* notify former target victim that they are not being targetted anymore */
  if (*lptr)
  {
    if ((*lptr)->contact)
    {
      qptr = find_ship_contact(ContactShip(*lptr), ship);
      if (qptr)
      {
        if (HasFlag(qptr->flags, HS_IDENTIFIED))
        {
          hs_std_sensor(ContactShip(*lptr), qptr, tprintf("%s has released target lock.", ContactName(qptr)));
        } else {
          hs_std_sensor(ContactShip(*lptr), qptr, "Unidentified contact has released target lock.");
        }
      } else {
        notify_consoles(ContactShip(*lptr), tprintf("%s%s-%s An unknown source has released target lock.",
                        ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL));
      }
    }
  }
  
  /* notify the victim they are being targetted */
  *lptr = q;
  qptr = find_ship_contact(ContactShip(q), ship);
  if (qptr)
  {
    if (HasFlag(qptr->flags, HS_IDENTIFIED))
    {
      hs_std_sensor(ContactShip(q), qptr, tprintf("%s%s!!%s %s has a target lock %s%s!!%s", ANSI_HILITE, ANSI_RED,
                    ANSI_NORMAL, ContactName(qptr), ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
    } else {
      hs_std_sensor(ContactShip(q), qptr, tprintf("%s%s!!%s Unidentified contact has a target lock %s%s!!%s", ANSI_HILITE, ANSI_RED,
                    ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
    }
    
    /* set a minimum threat level for the victim */
    if (qptr->threat < 1.0)
      qptr->threat = 1.0;

  } else {
    notify_consoles(ContactShip(q), tprintf("%s%s!!%s An unknown source has a target lock %s%s!!%s", ANSI_HILITE,
                  ANSI_RED, ANSI_NORMAL, ANSI_HILITE, ANSI_RED, ANSI_NORMAL));
  }
  

  /* notify console that they now have a target lock */
  if (HasFlag(q->flags, HS_IDENTIFIED))
  {
    hs_std_notice(console, tprintf("Target locked on %s%s%s!", ANSI_HILITE, ShipName(q->contact), ANSI_NORMAL));
  } else {
    hs_std_notice(console, "Target locked on unidentified contact!");
  }
  
  return;
}

/* command interface for target lock */
void set_lock(dbref console, char *which)
{
  hconsole *con;
  hship *ship;
  hcontact *q, **lptr;
  int cnum;
  
  /* setup the lptr pointer so we can easily set the lock for both
     navigation consoles and auxiliary consoles. */
  if (IsConsole(console))
  {
    con = find_console(console);
    ship = find_ship(console);
    if (!con || !ship)
      return;
  }
  else if (IsShip(console))
  {
    con = NULL;
    ship = find_ship_by_nav(console);
    if (!ship)
      return;
  }
  else
    return;
  
  if (!which || !*which)
  {
    /* clear the lock */
    q = NULL;
  } else {
    cnum = parse_integer(which);
    q = find_contact(ship, cnum);
    if (!q || !q->contact)
    {
      hs_std_notice(console, "Sensor contact is not found.");
      return;
    }
  }
  
  change_lock(ship, con, q);
}



/*******************************************************/
/* sensor contact routines */

char *contact_colorstring(hship *ship, hcontact *q)
{
  static char cbuf[32];
  int check;
  
  if (!ship || !q)
    return "";

  if (HasFlag(q->type, HS_ANY_SHIP))
  {
    check = check_friend(ship, q);
    if (has_lock(ContactShip(q), ship))
      sprintf(cbuf, "%s%s", ANSI_HILITE, ANSI_RED);
    else if (!check)
      sprintf(cbuf, "%s", ANSI_RED);
    else if (check > 0)
      sprintf(cbuf, "%s%s", ANSI_HILITE, ANSI_GREEN);
    else
      sprintf(cbuf, "%s", ANSI_GREEN);
      
  }
  else if (HasFlag(q->type, HS_CELESTIAL))
  {
    /* remove the HS_SHIP and HS_CELESTIAL flags with HS_OBJECT_FLAGS mask */
    switch (HasFlag(q->type, HS_OBJECT_FLAGS))
    {
    case HS_STAR:
      sprintf(cbuf, "%s", ANSI_HILITE);
      break;
    case HS_PLANET:
      sprintf(cbuf, "%s", ANSI_CYAN);
      break;
    case HS_WORMHOLE:
      sprintf(cbuf, "%s%s", ANSI_HILITE, ANSI_BLUE);
      break;
    case HS_ASTEROID:
      sprintf(cbuf, "%s%s", ANSI_HILITE, ANSI_YELLOW);
      break;
    case HS_WAYPOINT:
    case HS_WAYPOINT | HS_ANOMALY:
      sprintf(cbuf, "%s%s", ANSI_HILITE, ANSI_MAGENTA);
      break;
    case HS_ANOMALY:
      sprintf(cbuf, "%s%s", ANSI_HILITE, ANSI_BLACK);
      break;
    case HS_DEBRIS:
      sprintf(cbuf, "%s", ANSI_YELLOW);
      break;
    default:
      sprintf(cbuf, "%s", ANSI_NORMAL);
      break;
    }
  } else {
    return "";
  }
    
  return cbuf;
}

int check_ship_range(hship *ship, hship *target)
{
  double detect, ident, r, sensor;
  
  if (!ship || !target)
    return 2;
  
  if (ship->uid && ship->uid != target->uid)
    return 0;  
  
  if (!HasFlag(ship->type, HS_DRONE) && HasFlag(target->type, HS_CAPITAL | HS_STATION))
  {
    detect = 180.0;
    ident = 60.0;
  } else {
    detect = 60.0;
    ident = 20.0;
  }
  
  if (HasFlag(target->type, HS_DRONE))
  {
    ident *= 2.0;
  }
  
  if (HasFlag(ship->type, HS_SENSOR_JAM))
  {
    detect *= 0.5;
    ident *= 0.5;
  }

  r = ship_distance(ship, target);
  sensor = get_stat(ship, SYS_SENSITIVITY);
  
  detect *= (1.0 + sensor/4.0) * (1.0 + target->nconsoles/4.0);
  ident *= (1.0 + sensor/4.0) * (1.0 + target->nconsoles/4.0);

  if (r > detect)
    return 0;
  else if (r > ident)
    return 1;
  return 2;
  
  return 0;
}

int check_celestial_range(hship *ship, hcelestial *target)
{
  double detect, ident, r, sensor;
  
  if (!ship || !target)
    return 0;
  
  if (ship->uid && ship->uid != target->uid)
    return 0;
  
  /* HS_OBJECT_FLAGS mask to remove HS_CELESTIAL */
  switch (HasFlag(target->type, HS_OBJECT_FLAGS))
  {
  case HS_WAYPOINT | HS_ANOMALY:
  case HS_WAYPOINT:
    detect = 100000000.0;
    ident = 100000000.0;
    break;
  case HS_STAR:
    detect = 10000.0;
    ident = 10000.0;
    break;
  case HS_PLANET:
    detect = 1000.0;
    ident = 700.0;
    break;
  case HS_WORMHOLE:
    detect = 150.0;
    ident = 100.0;
    break;
  case HS_ASTEROID:
  case HS_ANOMALY:
    detect = 200.0 + target->radius;
    ident = 100.0 + target->radius;
    break;
  case HS_DEBRIS:
    detect = 30.0;
    ident = 30.0;
    break;
  default:
    detect = 30.0;
    ident = 30.0;
    break;
  }
  
  if (HasFlag(ship->type | target->type, HS_SENSOR_JAM))
  {
    detect *= 0.5;
    ident *= 0.5;
  }

  r = ship_celestial_distance(ship, target);
  sensor = get_stat(ship, SYS_SENSITIVITY);
  
  detect *= (1.0 + sensor/4.0);
  ident *= (1.0 + sensor/4.0);
  
  if (r > detect)
    return 0;
  else if (r > ident)
    return 1;
  return 2;
  
  return 0;
}

/* returns a sensor entry for a ship when supplied a contact number */
hcontact *find_contact(hship *ship, int cnum)
{
  hcontact *ptr;
  
  if (!ship)
    return NULL;
  
  for (ptr = get_head_contact(ship); ptr; ptr = ptr->next)
  {
    if (ptr->cnum == cnum)
      return ptr;
  }

  return NULL;
}


/* returns a sensor entry for a ship when supplied another ship */
hcontact *find_ship_contact(hship *ship, hship *target)
{
  hcontact *ptr;
  
  if (!ship || !target)
    return NULL;
  
  for (ptr = get_head_contact(ship); ptr; ptr = ptr->next)
  {
    if (ptr->contact == (void *)target)
      return ptr;
  }

  return NULL;
}

/* returns a sensor entry for a ship when supplied a celestial */
hcontact *find_celestial_contact(hship *ship, hcelestial *target)
{
  hcontact *ptr;
  
  if (!ship || !target)
    return NULL;
  
  for (ptr = get_head_contact(ship); ptr; ptr = ptr->next)
  {
    if (ptr->contact == (void *)target)
      return ptr;
  }

  return NULL;
}


/* create a random cnum */
int generate_cnum(hship *ship)
{
  hcontact *q;
  int     found;
  int     num;

  while (1)
  {
    found = 0;
    num = getrandom(8999) + 1000;
    for (q = get_head_contact(ship); q; q = q->next)
    {
      if (q->cnum == num)
      {
	found = 1;
	break;
      }
    }
    if (!found)
      break;
  }
  return num;
}


/* add a new ship type contact to a list */
hcontact *add_ship_contact(hship *ship, hship *target)
{
  hcontact *q;
  
  if (!ship || !target)
    return NULL;
  
  q = (hcontact *) malloc(sizeof(hcontact));
  if (!q)
  {
    SPACEWALL("HSPACE: ERROR: unable to allocate sensor contact!");
    return NULL;
  }
      
  q->type = target->type;
  q->contact = (void *) target;
  q->cnum = generate_cnum(ship);
  q->flags = 0;
  //FlagOff(q->flags, HS_IDENTIFIED | HS_UPDATED);
  q->threat = 0.0;
  q->next = get_head_contact(ship);

  if (ship->mothership)
  {
    ship->mothership->head_contact = q;
    ship->mothership->ncontacts++;
  } else {
    ship->head_contact = q;
    ship->ncontacts++;
  }

  return q;
}

/* add a new ship type contact to a list */
hcontact *add_celestial_contact(hship *ship, hcelestial *target)
{
  hcontact *q;
  
  if (!ship || !target)
    return NULL;
  
  q = (hcontact *) malloc(sizeof(hcontact));
  if (!q)
  {
    SPACEWALL("HSPACE: ERROR: unable to allocate sensor contact!");
    return NULL;
  }
      
  q->type = target->type;
  q->contact = (void *) target;
  q->cnum = generate_cnum(ship);
  q->flags = 0;
  //FlagOff(q->flags, HS_IDENTIFIED | HS_UPDATED);
  q->threat = 0.0;
  q->next = get_head_contact(ship);

  if (ship->mothership)
  {
    ship->mothership->head_contact = q;
    ship->mothership->ncontacts++;
  } else {
    ship->head_contact = q;
    ship->ncontacts++;
  }

  return q;
}


/* remove contact from the list and delete it */
int del_contact(hship *ship, hcontact *contact)
{
  hcontact *q, *qptr;
  
  if (!ship || !contact || !ship->head_contact)
    return 0;

  /* early exit clause */
  if (ship->head_contact == contact)
  {
    ship->head_contact = contact->next;
    free(contact);
    ship->ncontacts--;
    return 1;
  }

  q = ship->head_contact;
  while (q)
  {
    if (q->next == contact)
    {
      qptr = q;
      q = q->next;
      free(contact);
      ship->ncontacts--;
      return 1;
    }
  }

  return 0;
}


/* clear a ship's contact list */
int clear_contacts(hship *ship)
{
  hcontact *q, *qptr;
  int i;

  if (!ship)
    return 0;
    
  ship->lock = NULL;
  ship->ncontacts = 0;
  ship->wp_contact = NULL;

  for (i = 0; i < ship->nconsoles; i++)
  {
    ship->console[i].lock = NULL;
  }

  if (ship->mothership)
  {
    if (!ship->head_contact)
      return 1;
  }
  
  q = ship->head_contact;
  while (q)
  {
    qptr = q;
    q = q->next;
    free(qptr);
  }

  ship->head_contact = NULL;
  return 1;
}


/* update a single ship contact */
hcontact *update_ship_contact(hship *ship, hship *sptr)
{
  int inrange;
  hcontact *q, *qptr;
  
  if (!ship || !sptr)
    return NULL;
  
  if (HasFlag(sptr->type, HS_DESTROYED))
    return NULL;
  
  inrange = check_ship_range(ship, sptr);
  q = find_ship_contact(ship, sptr);
  if (!q)
  {
    /* don't have this as a contact, check the range */
    if (inrange)
    {
      qptr = add_ship_contact(ship, sptr);
      if (!qptr || !qptr->contact)
        return NULL;
      
      /* trigger new_contact event */
      FlagOn(qptr->flags, HS_NEW);

      if (inrange > 1)
      {
        FlagOn(qptr->flags, HS_IDENTIFIED);
        hs_std_sensor(ship, qptr, tprintf("New contact identified as %s.", ShipName(qptr->contact)));
        
        /* trigger identified_contact event as well */
        TriggerEvent(update_contact,ship,qptr);
      } else {
        hs_std_sensor(ship, qptr, "Unidentified new contact!");
      }
      
      /* mark the new contact as updated */
      FlagOn(qptr->flags, HS_UPDATED);

      return qptr;
    }
  } else {
    /* already have contact, maybe update it? */

    /* mark the existing contact as updated */
    FlagOn(q->flags, HS_UPDATED);
    
    /* contact is not identified, but is in range for it */
    if (!HasFlag(q->flags, HS_IDENTIFIED) && inrange > 1)
    {
      FlagOn(q->flags, HS_IDENTIFIED);
      hs_std_sensor(ship, q, tprintf("Contact identified as %s.", ShipName(q->contact)));
      
      /* trigger identified_contact event as well */
      TriggerEvent(update_contact,ship,q);
    }
    /* contact is no longer in range */
    else if (!inrange)
    {
      FlagOn(q->flags, HS_DELETED);
      //FlagOff(q->flags, HS_UPDATED);
    }
    
    return q;
  }
  
  return NULL;
}


/* update a single celestial contact */
hcontact *update_celestial_contact(hship *ship, hcelestial *cptr)
{
  int inrange;
  hcontact *q, *qptr;

  if (!ship || !cptr)
    return NULL;

  if (HasFlag(cptr->type, HS_DESTROYED))
    return NULL;

  inrange = check_celestial_range(ship, cptr);
  q = find_celestial_contact(ship, cptr);
  if (!q)
  {
    /* don't have this as a contact, check the range */
    if (inrange)
    {
      qptr = add_celestial_contact(ship, cptr);
      if (!qptr || !qptr->contact)
        return NULL;

      /* trigger new_contact event */
      FlagOn(qptr->flags, HS_NEW);

      if (inrange > 1)
      {
        FlagOn(qptr->flags, HS_IDENTIFIED);
        hs_std_sensor(ship, qptr, tprintf("New contact identified as %s.", CelestialName(qptr->contact)));
        
        /* trigger identified_contact event as well */
        TriggerEvent(update_contact,ship,qptr);
      } else {
        hs_std_sensor(ship, qptr, "Unidentified new contact!");
      }
      
      /* mark the new contact as updated */
      FlagOn(qptr->flags, HS_UPDATED);
      
      return qptr;
    }
  } else {
    /* already have contact, maybe update it? */

    /* mark the existing contact as updated */
    FlagOn(q->flags, HS_UPDATED);
    
    /* contact is not identified, but is in range for it */
    if (!HasFlag(q->flags, HS_IDENTIFIED) && inrange > 1)
    {
      FlagOn(q->flags, HS_IDENTIFIED);
      hs_std_sensor(ship, q, tprintf("Contact identified as %s.", CelestialName(q->contact)));
      
      /* trigger identified_contact event as well */
      TriggerEvent(update_contact,ship,q);
    }
    /* contact is no longer in range */
    else if (!inrange)
    {
      FlagOn(q->flags, HS_DELETED);
      //FlagOff(q->flags, HS_UPDATED);
    }
    
    return q;
  }

  return NULL;
}


/* locate new contacts and add them to the list */
void update_contacts(hship *ship)
{
  hcontact *q, *qptr, **qq;
  hship *sptr;
  hcelestial *cptr;
  int i, n;
  int *nptr;

  if (!ship)
    return;
  
  /* if we don't have a universe, are docked, or are landed
     destroy existing contacts and return */
  if (!ship->uid || ship->landed || ship->docked)
  {
    if (ship->head_contact)
      clear_contacts(ship);
    
    return;
  }
    
  /* cycle through ships in the current universe and assess them */
  for (sptr = ship->uid->head_ship; sptr; sptr = sptr->next)
  {
    if (ship == sptr)
      continue;
    
    update_ship_contact(ship, sptr);
  }
  
  for (sptr = ship->uid->head_drone; sptr; sptr = sptr->next)
  {
    if (ship == sptr)
      continue;
    
    update_ship_contact(ship, sptr);
  }

  /* now cycle through celestials */
  for (cptr = ship->uid->head_celestial; cptr; cptr = cptr->next)
  {
    update_celestial_contact(ship, cptr);
  }
  
  for (cptr = ship->uid->head_debris; cptr; cptr = cptr->next)
  {
    update_celestial_contact(ship, cptr);
  }


  /*
    so we check if we have a wp_contact and if that contact is in our universe,
      if so, we leave it alone
    if we don't have a wp_contact, then check if our internal celestial waypoint
      is in the same universe, and use that.
  */
  
  /* handle waypoint contact */
  if (ship->mothership && ship->wp_contact != ship->mothership->wp_contact)
  {
    ship->wp_contact = ship->mothership->wp_contact;
  }
  else if (ship->wp_contact)
  {
    if (HasFlag(ship->wp_contact->type, HS_ANY_CELESTIAL))
    {
      if (ship->uid == ContactCelestial(ship->wp_contact)->uid)
      {
        /* already have celestial waypoint in the same universe */
        FlagOn(ship->wp_contact->flags, HS_UPDATED);
      }
    }
    else if (HasFlag(ship->wp_contact->type, HS_ANY_SHIP))
    {
      if (ship->uid == ContactShip(ship->wp_contact)->uid)
      {
        /* we already have a ship waypoint contact in the same universe */
        FlagOn(ship->wp_contact->flags, HS_UPDATED);
      }
    }
  } else {
    /* we don't have a wp_contact that's in the same universe */
    /* check to see if our internal celestial waypoint is in */
    /* the same universe and add the contact if it is  */
    if (ship->uid == ship->waypoint.uid)
    {
      /* make sure the waypoint isn't already on sensors */
      q = find_celestial_contact(ship, &ship->waypoint);
      if (!q)
      {
        /* we don't already have the waypoint contact */
        /* add it to sensors */
        qptr = add_celestial_contact(ship, &ship->waypoint);
        if (qptr)
        {
          ship->wp_contact = qptr;
          
          /* trigger new_waypoint */
          //TriggerEvent(new_waypoint,ship,qptr);
          FlagOn(qptr->flags, HS_IDENTIFIED | HS_UPDATED);
          hs_std_sensor(ship, qptr, "New nav point identified.");
        }
      } else {
        /* we already have it on sensors, but didn't have wp_contact set. just update it */
        ship->wp_contact = q;
        FlagOn(q->flags, HS_UPDATED);
      }
      
      /* check to see if we have a spawn, and set their wp_contacts */
      if (ship->spawn)
      {
        for (i = 0; i < ship->spawn->ndrones; i++)
        {
          if (ship->spawn->drones[i])
          {
            ship->spawn->drones[i]->wp_contact = ship->wp_contact;
          }
        }
      }
    }
  }
  
  /* clean up existing contacts by checking if they were updated before */
  /* if not, they are stale and should be removed */
  if (ship->mothership)
  {
    qq = &(ship->mothership->head_contact);
    nptr = &(ship->mothership->ncontacts);
  }
  else
  {
    qq = &(ship->head_contact);
    nptr = &(ship->ncontacts);
  }

  for (q = get_head_contact(ship); q; )
  {
    qptr = q;
    q = q->next;

    if (HasFlag(qptr->flags, HS_UPDATED) && !HasFlag(qptr->flags, HS_DELETED))
    {
      FlagOff(qptr->flags, HS_UPDATED);
      
      /* handle new_contact notifications
         done here so it will delay the message if the
         contacts see one another at the same time */

      if (HasFlag(qptr->flags, HS_NEW))
      {
        FlagOn(qptr->flags, HS_NEWTRIGGER);
        FlagOff(qptr->flags, HS_NEW);
      }
      else if (HasFlag(qptr->flags, HS_NEWTRIGGER))
      {
        FlagOff(qptr->flags, HS_NEWTRIGGER);
        TriggerEvent(update_contact,ship,qptr);
      }
      
      /* set a new last contact variable before we advance */
      qq = &(qptr->next);
    }
    else
    {
      if (qptr == ship->wp_contact)
      {
        ship->wp_contact = NULL;
        
        if (ship->spawn)
        {
          for (i = 0; i < ship->spawn->ndrones; i++)
          {
            if (ship->spawn->drones[i])
            {
              ship->spawn->drones[i]->wp_contact = NULL;
            }
          }
        }
      }
      
      if (ship->spawn)
      {
        for (n = 0; n < ship->spawn->ndrones; n++)
        {
          sptr = ship->spawn->drones[n];
          /* check for nav and aux console locks and clear them */
          if (sptr->lock == qptr)
            change_lock(sptr, NULL, NULL);
          
          for (i = 0; i < sptr->nconsoles; i++)
          {
            if (sptr->console[i].lock == qptr)
            {
              change_lock(sptr, &(sptr->console[i]), NULL);
            }
          }
        }
      } else {
        if (qptr && HasFlag(qptr->flags, HS_INSIDE))
        {
          hs_std_sensor(ship, qptr, tprintf("Leaving the proximity of %s.", ContactName(qptr)));
          FlagOff(qptr->flags, HS_INSIDE); 
          
          if (HasFlag(qptr->type, HS_ASTEROID))
          {
            FlagOff(ship->type, HS_MODE_SLOW);
            
            if (HasFlag(qptr->type, HS_SENSOR_JAM))
              FlagOff(ship->type, HS_SENSOR_JAM);
          }
          else if (HasFlag(qptr->type, HS_ANOMALY) && HasFlag(qptr->type, HS_SENSOR_JAM))
          {
            FlagOff(ship->type, HS_SENSOR_JAM);
          }
  
          TriggerEvent(update_prox,ship,qptr);
        }

        /* check for nav and aux console locks and clear them */
        if (ship->lock == qptr)
          change_lock(ship, NULL, NULL);
        
        for (i = 0; i < ship->nconsoles; i++)
        {
          if (ship->console[i].lock == qptr)
          {
            change_lock(ship, &(ship->console[i]), NULL);
          }
        }
      }

      if (HasFlag(qptr->flags, HS_IDENTIFIED))
        hs_std_sensor(ship, qptr, tprintf("Lost contact %s.", ContactName(qptr)));
      else
        hs_std_sensor(ship, qptr, "Unidentified contact lost.");
      
      /* Trigger lost_contact event */
      TriggerEvent(update_contact,ship,qptr);

      /* remove qptr from the list and delete it */
      (*qq) = q;
      free(qptr);
      (*nptr)--;
    }
      
    if ((*nptr) < 0)
    {
      SPACEWALL("HSPACE: ERROR: update_contacts(), ncontacts is negative, check for memory leaks!");
      break;
    }
  }

  return;

}


/* quick sort algorithm */
int sort_internal(double **a, hcontact ***contacts, int l, int r)
{
   int i, j, t;
   double pivot;
   hcontact *c;
   
   pivot = (*a)[l];
   i = l; j = r+1;

   while(1)
   {
     do ++i;
     while ((*a)[i] <= pivot && i <= r);
     
     do --j;
     while ((*a)[j] > pivot);

     if(i >= j)
       break;

     t = (*a)[i];
     c = (*contacts)[i];

     (*a)[i] = (*a)[j];
     (*contacts)[i] = (*contacts)[j];
     
     (*a)[j] = t;
     (*contacts)[j] = c;
   }

   t = (*a)[l];
   c = (*contacts)[l];

   (*a)[l] = (*a)[j];
   (*contacts)[l] = (*contacts)[j];

   (*a)[j] = t;
   (*contacts)[j] = c;
   
   return j;
}

/* main entry point to sort an array of contacts by their distances */
void sort_by_distance(double **a, hcontact ***contacts, int l, int r)
{
   int j;

   if(l < r)
   {
     // divide and conquer
     j = sort_internal(a, contacts, l, r);
     sort_by_distance(a, contacts, l, j-1);
     sort_by_distance(a, contacts, j+1, r);
   }
	
}


/* display the standard sensor report */
/* by default shows all contacts, but you can supply hobj types */
/* to filter the output and show only those */
void sensor_report(dbref cship, char *arg_left)
{
  dbref   player;
//  double  range;
  char    name[64];
  char    ident[64];
  hship  *tship = NULL;
  char    sdist[32];
  hcontact *q;
  char    buff[256];
  double  fx = 0, fy = 0, fz = 0, sx = 0, sy = 0, sz = 0;
  int     xy = 0, za = 0;
  int     ncon, i;
  char hs;
  char    sizesym;
  static char sizebuf[28] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
  double  dist = 0;
  double radius;
  char *cbuf;
  int secs, mins, hours;
  int filter;
  int shipsize;
  double speed;
  
  double *rangelist;
  hcontact **qlist;
  hcelestial *cel;

  tship = find_ship(cship);
  if (!tship)
  {
    notify(cship, "Can't find your ship pointer!");
    return;
  }

  player = get_user(cship);
  if (player == NOTHING)
  {
    notify(cship, "Can't find your user!");
    return;
  }

  ncon = tship->ncontacts;
  if (ncon == 0)
  {
    hs_std_error(cship, "No ship contacts on sensors.");
    return;
  }
  
  filter = HS_ANY_OBJECT;
  if (arg_left && *arg_left)
  {
    filter = parse_flags(hs_object_types, arg_left);
    if (filter == 0)
      filter = HS_ANY_OBJECT;
  }

  rangelist = (double *) malloc(sizeof(double) * ncon);
  qlist = (hcontact **) malloc(sizeof(hcontact*) * ncon);

  fx = tship->x;
  fy = tship->y;
  fz = tship->z;

  /* make a list of contacts, possibly filtered by type */
  /* save the distance so we can sort the contacts */
  i = 0;
  for (q = get_head_contact(tship); q; q = q->next)
  {
    if (!HasFlag(q->type, filter) || !q->contact)
      continue;

    rangelist[i] = ContactDistance(tship, q);
    qlist[i] = q;
    
    i++;
  }
  ncon = i;
  
  /* set the universe system tag on the top line of the report */
  if (tship->uid && RealGoodObject(tship->uid->objnum))
  {
    sprintf(ident, "%s%s%20s%s", ANSI_HILITE, ANSI_GREEN, Name(tship->uid->objnum), ANSI_NORMAL);
  }
  else
    strcpy(ident, "");
  
  /* top line of the report */
  speed = tship->speed;
  if (HasFlag(tship->type, HS_AFTERBURNING))
    speed *= hs_options.burn_multiplier;
  
  sprintf(buff, " %s%sSensor Contacts:%s %3d         %s%sH:%s %-3.0f%s%sm%s%3.0f  %s%sS:%s %5.0f km/s %s",
          ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ncon, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, tship->xyhead, ANSI_HILITE,
          ANSI_GREEN, ANSI_NORMAL, tship->zhead, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, speed, ident);
  notify(player, buff);

  notify_format(player, "%s%s---------------------------------------------------------------------------%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);
  
  /* sort remaining contacts by distance */
  sort_by_distance(&rangelist, &qlist, 0, ncon-1);
  
  /* print remaining contacts in sorted order */
  for (i = 0; i < ncon; i++)
  {
    q = qlist[i];

    if (!q || !q->contact)
      continue;
    
    if (!HasFlag(q->flags, HS_IDENTIFIED))
    {
      strcpy(name, "Unidentified");
    }
    else
    {
      strncpy(name, (char *) ContactName(q), 63);
    }
    
    cbuf = contact_colorstring(tship, q);

    if (HasFlag(q->type, HS_ANY_SHIP))
    {
      sx = ContactShip(q)->x;
      sy = ContactShip(q)->y;
      sz = ContactShip(q)->z;
      
      xy = xyang(fx, fy, sx, sy);
      if (xy == -1)
        xy = 0;
      
      za = zang(fx, fy, fz, sx, sy, sz);
      
      if (has_lock(tship, ContactShip(q)))
        hs = '*';
      else if (tship->wp_contact == q)
        hs = '!';
      else if (HasFlag(ContactShip(q)->type, HS_PVP))
        hs = '+';
      else
        hs = ' ';
      
      shipsize = check_ship_size(ContactShip(q));
      if (shipsize > 26 || shipsize < 0)
        shipsize = 0;
      
      sizesym = sizebuf[shipsize];
      
      dist = dist3d(fx, fy, fz, sx, sy, sz);

      if (dist < 10000.0)
        sprintf(sdist, "%5.0f Mm", dist);
      else if (dist < 100000.0)
        sprintf(sdist, "%5.1f Gm", dist / 1000.0);
      else
        sprintf(sdist, "%5.0f Gm", dist / 1000.0);
      
      speed = ContactShip(q)->speed;
      if (HasFlag(ContactShip(q)->type, HS_AFTERBURNING))
        speed *= hs_options.burn_multiplier;

      sprintf(buff,
	      " %c%s%c%s%s[%s%d%s]%s %-19.19s %s%sB:%s %-3d%s%sm%s%3d  %s%sR:%s %-8.8s    %s%sH:%s %-3.0f%s%sm%s%3.0f  %s%sS:%s %-7.0f",
	      sizesym, ANSI_HILITE, hs, ANSI_NORMAL,
              cbuf, ANSI_NORMAL, q->cnum, cbuf, ANSI_NORMAL, name, ANSI_HILITE, ANSI_GREEN,
              ANSI_NORMAL, xy, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, za, ANSI_HILITE,
              ANSI_GREEN, ANSI_NORMAL, sdist, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	      ContactShip(q)->xyhead, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	      ContactShip(q)->zhead, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
	      speed);
    } else {
      sx = ContactCelestial(q)->x;
      sy = ContactCelestial(q)->y;
      sz = ContactCelestial(q)->z;
      xy = xyang(fx, fy, sx, sy);
      if (xy == -1)
        xy = 0;
      za = zang(fx, fy, fz, sx, sy, sz);

      if (tship->wp_contact == q)
        hs = '!';
      else
        hs = ' ';

      dist = dist3d(fx, fy, fz, sx, sy, sz);

      if (dist < 10000.0)
        sprintf(sdist, "%5.0f Mm", dist);
      else if (dist < 100000.0)
        sprintf(sdist, "%5.1f Gm", dist / 1000.0);
      else
        sprintf(sdist, "%5.0f Gm", dist / 1000.0);
      
      /* remove HS_CELESTIAL with HS_OBJECT_FLAGS mask */
      switch (HasFlag(q->type, HS_OBJECT_FLAGS))
      {
      case HS_STAR:
        sizesym = '*';
        sprintf(ident, "  %s%sM:%s %2.0f Solar", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ContactCelestial(q)->mass);
        break;
      case HS_PLANET:
        sizesym = 'o';
        //sprintf(ident, "  %s%sM:%s %g Earth", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ContactCelestial(q)->mass);
        if (tship->speed != 0.0)
        {
          secs = dist / tship->speed * 1000.0;
          if (HasFlag(tship->type, HS_AFTERBURNING))
            secs /= 3.0;
          
          mins = secs/60;
          hours = mins/60;
          if (secs > 3600)
          {
            mins -= hours*60;
            secs -= hours*60 + mins*60;
            sprintf(ident, "%s%sETA:%s %dh%dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, hours, mins, secs);
          }
          else if (secs > 60)
          {
            secs -= mins*60;
            sprintf(ident, "%s%sETA:%s %dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, mins, secs);
          } else {
            sprintf(ident, "%s%sETA:%s %ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, secs);
          }
        } else {
          sprintf(ident, "%s%sETA:%s -", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        }
        break;
      case HS_WORMHOLE:
        sizesym = '>';
        /*cel = find_celestial(atr_parse_dbref(ContactObj(q), "OTHERSIDE"));

        if (!cel || !cel->uid || !RealGoodObject(cel->uid->objnum) || !HasFlag(q->flags, HS_IDENTIFIED))
          sprintf(name, "> Unidentified");
        else
          sprintf(name, "> %s", Name(cel->uid->objnum));*/
        
        if (tship->speed != 0.0)
        {
          secs = dist / tship->speed * 1000.0;
          if (HasFlag(tship->type, HS_AFTERBURNING))
            secs /= 3.0;
          
          mins = secs/60;
          hours = mins/60;
          if (secs > 3600)
          {
            mins -= hours*60;
            secs -= hours*60 + mins*60;
            sprintf(ident, "%s%sETA:%s %dh%dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, hours, mins, secs);
          }
          else if (secs > 60)
          {
            secs -= mins*60;
            sprintf(ident, "%s%sETA:%s %dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, mins, secs);
          } else {
            sprintf(ident, "%s%sETA:%s %ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, secs);
          }
        } else {
          sprintf(ident, "%s%sETA:%s -", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        }
        break;
      case HS_ASTEROID:
        sizesym = 'x';
        // (3*(mass / density)/(4*pi))^(1/3)
        //radius = cbrt(3.0/4.0/PI * ContactCelestial(q)->mass / 2);
        sprintf(ident, "  %s%sM:%s %2.0f Eg  %s%sRad:%s %2.0f Mm", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                ContactCelestial(q)->mass, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ContactCelestial(q)->radius);
        break;
      case HS_ANOMALY:
        sizesym = '?';
        sprintf(ident, "  %s%sM:%s %2.0f Solar  %s%sRad:%s %2.0f Mm", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                ContactCelestial(q)->mass, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ContactCelestial(q)->radius);
        //sprintf(ident, "%s%sRad:%s %.0f Mm", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ContactCelestial(q)->radius);
        break;
      case HS_WAYPOINT:
      case HS_WAYPOINT | HS_ANOMALY:
        /* maybe use ident to show the mission? */
        sizesym = '!';

        if (tship->speed != 0.0)
        {
          secs = dist / tship->speed * 1000.0;
          if (HasFlag(tship->type, HS_AFTERBURNING))
            secs /= 3.0;
          
          mins = secs/60;
          hours = mins/60;
          if (secs > 3600)
          {
            mins -= hours*60;
            secs -= hours*60 + mins*60;
            sprintf(ident, "%s%sETA:%s %dh%dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, hours, mins, secs);
          }
          else if (secs > 60)
          {
            secs -= mins*60;
            sprintf(ident, "%s%sETA:%s %dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, mins, secs);
          } else {
            sprintf(ident, "%s%sETA:%s %ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, secs);
          }
        } else {
          sprintf(ident, "%s%sETA:%s -", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
        }
        break;
      case HS_DEBRIS:
        sizesym = '$';
        sprintf(ident, "  %s%sM:%s %2.0f Mg    %s%sT:%s %3.0f s", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL,
                ContactCelestial(q)->mass, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, ContactCelestial(q)->radius);
        break;
      default:
        sizesym = ' ';
        strncpy(ident, "", 63);
        break;
      }
      
      sprintf(buff,
	      " %c%s%c%s%s[%s%d%s]%s %-19.19s %s%sB:%s %-3d%s%sm%s%3d  %s%sR:%s %-8.8s  %s",
	      sizesym, ANSI_HILITE, hs, ANSI_NORMAL, cbuf, ANSI_NORMAL, q->cnum, cbuf, ANSI_NORMAL, name, ANSI_HILITE, ANSI_GREEN,
	      ANSI_NORMAL, xy, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, za, ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, sdist, ident);
    }
    
    notify(player, buff);
  }

  notify_format(player, "%s%s---------------------------------------------------------------------------%s",
          ANSI_HILITE, ANSI_BLUE, ANSI_NORMAL);

  free(rangelist);
  free(qlist);
}

void show_eta(dbref cship)
{
  dbref player;
  hship *tship;
  double dist;
  int secs, mins, hours;

  tship = find_ship(cship);
  if (!tship)
  {
    notify(cship, "Can't find your ship pointer!");
    return;
  }

  player = get_user(cship);
  if (player == NOTHING)
  {
    notify(cship, "Can't find your user!");
    return;
  }
  
  if (!tship->wp_contact)
  {
    notify(player, "No nav point selected.");
    return;
  }
  
  dist = ContactDistance(tship, tship->wp_contact);
  
  if (tship->speed != 0.0)
  {
    secs = dist / tship->speed * 1000.0;
    if (HasFlag(tship->type, HS_AFTERBURNING))
      secs /= 3.0;
    
    mins = secs/60;
    hours = mins/60;
    if (secs > 3600)
    {
      mins -= hours*60;
      secs -= hours*60 + mins*60;
      notify_format(player, "%s%sETA:%s %dh%dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, hours, mins, secs);
    }
    else if (secs > 60)
    {
      secs -= mins*60;
      notify_format(player, "%s%sETA:%s %dm%ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, mins, secs);
    } else {
      notify_format(player, "%s%sETA:%s %ds", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL, secs);
    }
  } else {
    if (dist > hs_options.max_waypoint_dist)
    {
      notify_format(player, "%s%sETA:%s - (Not moving)", ANSI_HILITE, ANSI_GREEN, ANSI_NORMAL);
    } else {
      notify(player, "You are within range.");
    }
  }

}



