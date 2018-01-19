#ifndef __HSDEFINES_H
#define __HSDEFINES_H

#include <stdint.h>

/**************************************************/
/* basic constants and aliases */

#define LIGHT_SPEED	299792.458 /* km/s */
#define	PI		3.14159265359
#define RAD2DEG		57.2957795131
#define DEG2RAD		0.01745329251

#define HSPACE_CONFIG_FILE "hspace.cnf"
#define HSPACE_VERSION "HSpace 3G"

#define SPACELOG 	"log/space.log"

#define MAX_CONSOLES	3

#define getrandom(x)	(get_random_u32(0,x))

#define SPACEWALL(x)  (flag_broadcast("HS_ADMIN", 0, x))

#define InCombat(x)		((x)->combat_timer < 30)


/**************************************************/
/* MUSH flag macros */

#define SetFlag(x,f)	(set_flag(hs_options.space_wiz, x, f, 0, 0, 0))
#define UnsetFlag(x,f)	(set_flag(hs_options.space_wiz, x, f, 1, 0, 0))

#define IsComponent(x)	(has_flag_by_name(x, "HS_COMPONENT", TYPE_THING))
#define IsWeapon(x)	(has_flag_by_name(x, "HS_WEAPON", TYPE_THING))
#define IsConsole(x)	(has_flag_by_name(x, "HS_CONSOLE", TYPE_THING))
#define IsShip(x)	(has_flag_by_name(x, "HS_SHIP", TYPE_THING))
#define IsDrone(x)	(has_flag_by_name(x, "HS_DRONE", TYPE_THING))
#define IsShipObj(x)	(has_flag_by_name(x, "HS_SHIPOBJ", TYPE_THING))
#define IsSim(x)	(has_flag_by_name(x, "HS_SIM", TYPE_THING))
#define IsCelestial(x)	(has_flag_by_name(x, "HS_CELESTIAL", TYPE_THING))
#define IsUniverse(x)	(has_flag_by_name(x, "HS_UNIVERSE", TYPE_THING))
#define IsMission(x)	(has_flag_by_name(x, "HS_MISSION", TYPE_THING))
#define IsMissionItem(x)	(has_flag_by_name(x, "MISSION_ITEM", TYPE_THING))

#define IsComm(x)	(has_flag_by_name(x, "HS_COMM", NOTYPE))

#define IsSpaceAdmin(x)	(has_flag_by_name(x, "HS_ADMIN", TYPE_PLAYER))


/***************************************************/
/* bit field manipulations and comparisons */

#define HasFlag(x,f)	((x) & (f))
#define FlagOn(x,f)	((x) |= (f))
#define FlagOff(x,f)	((x) &= ~(f))
#define FlagToggle(x,f)	((x) ^= (f))


/*************************************************/
/* sensor contact type convenience macros */

#define ShipName(x)	ship_name((hship*)(x))
#define CelestialName(x)	celestial_name((hcelestial*)(x))

#define ContactShip(x)	((hship*)((x)->contact))
#define ContactCelestial(x)	((hcelestial*)((x)->contact))

#define ContactName(x)	(HasFlag((x)->type, HS_ANY_SHIP) ? ship_name(ContactShip(x)) : celestial_name(ContactCelestial(x)))
#define ContactObj(x)	(HasFlag((x)->type, HS_ANY_SHIP) ? ContactShip(x)->objnum : ContactCelestial(x)->objnum)
#define ContactDistance(s,x)	(HasFlag((x)->type, HS_ANY_SHIP) ? ship_distance(s,ContactShip(x)) : ship_celestial_distance(s,ContactCelestial(x)))


/**************************************************/
/* stringmap definition macros */

#define STRINGMAP(x)		{#x, (void *) HS_ ## x}
#define PSTRINGMAP(x)		{#x, (void *) &HS_ ## x}

/* stringmap string macro */
#define STR(x,y)		(stringmap_string(x,(void *) (intptr_t)(y)))


/************************************************/
/* macros used by the buff system */

/* callback definition macro */
#define HBUFF_CALL(x)		void (buffcall_ ## x)(hship *ship, hbuff *buff)

#define NULLWEAPON		{0, NOTHING, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0}
#define NULLSYSTEM		{0, NOTHING, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}

/* buff user variable convenience macros */
#define Accuracy(x)	(x->var1)

#define ShieldDamage(x)	(x->var2)
#define Duration(x)	(x->var2)

#define HullDamage(x)	(x->var3)

#define Penetration(x)	(x->var4)

#define TotalDamage(x)	(x->var5)

/******************************************************/
/* macros used by drones */

/* callback definition */
#define HCALLBACK(x)		void (HS_ ## x)(hship *ship, hconsole *con, hcontact *q, char *event)

/* callback trigger */
#define TriggerEvent(e,s,q)     if (s && s->e) (s->e)(s,NULL,q,#e)
#define TriggerConsole(e,s,c,q)	if (s && c && c->e) (c->e)(s,c,q,#e)

#endif 

