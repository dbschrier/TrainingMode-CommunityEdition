#include "../MexTK/mex.h"
#include "events.h"

void Exit(int value);
void ChangeDirection(GOBJ *event_menu, int value);

enum menu_options {
    OPT_FIRE_SPEED,
    OPT_LASER_HEIGHT,
    OPT_DIRECTION,
};

enum fire_speed {
    FIRE_SPEED_RANDOM,
    FIRE_SPEED_SLOW,
    FIRE_SPEED_MEDIUM,
    FIRE_SPEED_FAST,
};

enum laser_height {
    LASER_HEIGHT_RANDOM,
    LASER_HEIGHT_VERY_LOW,
    LASER_HEIGHT_LOW,
    LASER_HEIGHT_MID,
    LASER_HEIGHT_HIGH,
};

static char *Options_FireSpeed[] = { "Random", "Slow", "Medium", "Fast" };
static char *Options_LaserHeight[] = { "Random", "Very Low", "Low", "Mid", "High" };
static char *Options_Direction[] = { "Right", "Left" };

static EventOption Options_Main[] = {
    {
        .option_kind = OPTKIND_STRING,
        .value_num = sizeof(Options_FireSpeed) / 4,
        .option_name = "Fire Speed",
        .desc = "Change the rate of fire.",
        .option_values = Options_FireSpeed,
    },
    {
        .option_kind = OPTKIND_STRING,
        .value_num = sizeof(Options_LaserHeight) / 4,
        .option_name = "Laser Height",
        .desc = "Change the laser height.",
        .option_values = Options_LaserHeight,
    },
    {
        .option_kind = OPTKIND_STRING,
        .value_num = sizeof(Options_Direction) / 4,
        .option_name = "Direction",
        .desc = "Change which way falco shoots a laser.",
        .option_values = Options_Direction,
        .onOptionChange = ChangeDirection,
    },
    {
        .option_kind = OPTKIND_FUNC,
        .option_name = "Exit",
        .desc = "Return to the Event Select Screen.",
        .onOptionSelect = Exit,
    },
};

static EventMenu Menu_Main = {
    .name = "Powershield Training",
    .option_num = sizeof(Options_Main) / sizeof(EventOption),
    .options = &Options_Main,
};

static int falco_wait_delay = -1;
static int falco_shoot_delay = -1;
static int falco_fastfall_delay = -1;

void Event_Think(GOBJ *menu) {
    GOBJ *player = Fighter_GetGObj(0);
    FighterData *player_data = player->userdata;
    GOBJ *falco = Fighter_GetGObj(1);
    FighterData *falco_data = falco->userdata;

    Fighter_ZeroCPUInputs(falco_data);
    falco_data->flags.no_reaction_always = true;
    player_data->shield.health = 60;

    int state = falco_data->state_id;
    int state_frame = falco_data->TM.state_frame;

    bool ground_actionable = (state == ASID_LANDING && state_frame > 4) || state == ASID_WAIT;
    bool air_actionable = state == ASID_JUMPF || state == ASID_JUMPB || (state == ASID_KNEEBEND && state_frame == 5);
    bool can_fastfall = falco_data->phys.air_state == 1 && falco_data->phys.self_vel.Y <= 0.f;

    if (ground_actionable && falco_wait_delay > 0)
        falco_wait_delay--;

    if (air_actionable && falco_shoot_delay > 0)
        falco_shoot_delay--;

    if (can_fastfall && falco_fastfall_delay > 0)
        falco_fastfall_delay--;


    if (ground_actionable && falco_wait_delay == -1) {
        // set wait timer

        int delay_option = Options_Main[OPT_FIRE_SPEED].option_val;

        if (delay_option == FIRE_SPEED_RANDOM)
            falco_wait_delay = HSD_Randi(20);
        else if (delay_option == FIRE_SPEED_SLOW)
            falco_wait_delay = 20;
        else if (delay_option == FIRE_SPEED_MEDIUM)
            falco_wait_delay = 10;
        else if (delay_option == FIRE_SPEED_FAST)
            falco_wait_delay = 0;
    }

    if (ground_actionable && falco_wait_delay == 0) {
        // start jump

        falco_wait_delay = -1;
        falco_data->cpu.held |= PAD_BUTTON_Y;
    }

    if (air_actionable && falco_shoot_delay == -1) {
        // set shoot timer

        int delay_option = Options_Main[OPT_LASER_HEIGHT].option_val;

        if (delay_option == LASER_HEIGHT_RANDOM) {
            falco_shoot_delay = HSD_Randi(4) + 2;
            falco_fastfall_delay = 1;
        } else if (delay_option == LASER_HEIGHT_VERY_LOW) {
            falco_shoot_delay = 11;
            falco_fastfall_delay = 8;
        } else if (delay_option == LASER_HEIGHT_LOW) {
            falco_shoot_delay = 5;
            falco_fastfall_delay = 1;
        } else if (delay_option == LASER_HEIGHT_MID) {
            falco_shoot_delay = 4;
            falco_fastfall_delay = 1;
        } else if (delay_option == LASER_HEIGHT_HIGH) {
            falco_shoot_delay = 2;
            falco_fastfall_delay = 0;
        }
    }

    if (air_actionable && falco_shoot_delay == 0) {
        // start laser

        falco_shoot_delay = -1;
        falco_data->cpu.held |= PAD_BUTTON_B;
    }

    if (can_fastfall && falco_fastfall_delay == 0) {
        falco_data->cpu.lstickY = -127;
        falco_fastfall_delay = -1;
    }
}

void Exit(int value) {
    Match *match = MATCH;
    match->state = 3;
    Match_EndVS();
}

void ChangeDirection(GOBJ *event_menu, int value) {
    GOBJ *falco = Fighter_GetGObj(1);
    FighterData *falco_data = falco->userdata;

    falco_data->facing_direction *= -1.f;
    falco_data->phys.pos.X *= -1.f;
}

EventMenu *Event_Menu = &Menu_Main;
