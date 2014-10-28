///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "falclib.h"
#include "vu2.h"
#include "chandler.h"
#include "ui95_ext.h"
#include "uicomms.h"
#include "userids.h"
#include "textids.h"
#include "F4Error.h"
#include "F4Find.h"
#include "cmpclass.h"
#include "tac_class.h"
#include "te_defs.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int
critical_section_initialised;

static F4CSECTIONHANDLE
*vc_critical = NULL;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

victory_condition::victory_condition(tactical_mission *mis)
{
    victory_condition
    *vc;

    mission = mis;

    vc = mis->conditions;

    while ((vc) and (vc->succ))
    {
        vc = vc->succ;
    }

    if (vc)
    {
        pred = vc;
        succ = NULL;
        vc->succ = this;
    }
    else
    {
        mis->conditions = this;
        this->pred = NULL;
        this->succ = NULL;
    }

    active = FALSE;
    team = 0;
    type = vt_unknown;
    id = FalconNullId;
    feature_id = -1; // unassigned
    tolerance = 0;
    max_vehicles = 0;
    points = 0;

    if (pred)
        number = pred->number + 1;
    else
        number = 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

victory_condition::~victory_condition()
{
    if (pred)
    {
        pred->succ = succ;
    }
    else
    {
        mission->conditions = succ;
    }

    if (succ)
    {
        succ->pred = pred;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int victory_condition::get_active(void)
{
    return active;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_active(int val)
{
    active = val;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int victory_condition::get_team(void)
{
    return team;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum victory_type victory_condition::get_type(void)
{
    return type;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int victory_condition::get_sub_objective(void)
{
    return feature_id;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int victory_condition::get_tolerance(void)
{
    return tolerance;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int victory_condition::get_points(void)
{
    return points;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int victory_condition::get_number(void)
{
    return number;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

VU_ID victory_condition::get_vu_id(void)
{
    return id;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_points(int new_points)
{
    points = new_points;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_tolerance(int new_tolerance)
{
    tolerance = new_tolerance;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_sub_objective(int new_value)
{
    feature_id = new_value;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_vu_id(VU_ID new_id)
{
    id = new_id;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_type(victory_type new_type)
{
    type = new_type;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_team(int new_team)
{
    team = new_team;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::test(void)
{
    active = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::set_number(int num)
{
    number = num;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int check_victory_conditions(void)
{
    if (current_tactical_mission)
    {
        if (current_tactical_mission->get_game_over())
            return(current_tactical_mission->get_game_over());

        current_tactical_mission->evaluate_victory_conditions();
        current_tactical_mission->calculate_victory_points();
        return current_tactical_mission->determine_victor();
    }
    else
    {
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if 0 // Not currently supported anymore
void evaluate_flight_vc(WayPointClass *wp, double x, double y, double z, double s)
{
    if (TheCampaign.Flags bitand CAMP_TACTICAL)
    {
        if (current_tactical_mission)
        {
            current_tactical_mission->evaluate_parameters(wp, x, y, z, s);
        }
    }
}
#endif
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::enter_critical_section(void)
{
    if ( not vc_critical)
    {
        vc_critical = F4CreateCriticalSection("vc_critical");
    }

    F4EnterCriticalSection(vc_critical);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void victory_condition::leave_critical_section(void)
{
    if (vc_critical)
    {
        F4LeaveCriticalSection(vc_critical);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
