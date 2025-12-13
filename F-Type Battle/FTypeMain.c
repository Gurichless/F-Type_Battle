#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>

#include <SDL3/SDL_main.h>  // This must be included
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#define MAX_RECTS 144
#define RECT_WIDTH 270
#define RECT_HEIGHT 180
#define MAX_CORNERS 24
#define WINDOW_WIDTH 3840
#define WINDOW_HEIGHT 2160
#define PLAYER_WIDTH 150
#define PLAYER_HEIGHT 150
#define ITEM_WIDTH 100
#define ITEM_HEIGHT 100
#define MAX_ITEMS 6
#define MAX_MONS 6
#define MON_WIDTH 200
#define MON_HEIGHT 200
#define PROJECTILE_WIDTH 50
#define PROJECTILE_HEIGHT 50
#define PROJECTILE_DELAY 1200
#define PLAYER_PROJ_DELAY 800
#define HEALTH_LOSS_DELAY 60
#define MAX_MON_HP 100
#define MAX_PLAYER_HP 100
#define MAX_MOVES 4
#define TRACK_WIDTH_MOD 40
#define TILE_WIDTH 100
#define TILE_HEIGHT 100
#define AREA_TILE_LIM 32160//important that it is ALL_ROWS * ALL_COLS or else buffer overflow
#define ALL_ROWS 134
#define ALL_COLS 240
#define ALL_AREAS 20
#define GM_HEIGHT 30.0
#define GM_WIDTH 30.0
#define MAX_GM 32160
#define GM_MARGIN 2
#define ALL_KEYITEMS 100
#define ALL_CHARACTERS 15
#define MAX_GOAL_ITEMS 3
#define MAX_DIAS 20
#define PICKUP_RADIUS 60
#define SPRITESHEET_FRAME_COLS 8 //8 frames per animation
#define SPRITESHEET_FRAME_ROWS 5 //4 directions, 5 different animations, plus idle animation (can change these if needed for other spritesheets)
#define SPRITE_WIDTH 100
#define SPRITE_HEIGHT 100
#define SPRITESHEET_MARGIN 2 
#define WORLD_VELOCITY_DEBUG 20.0
#define WORLD_VELOCITY 5.0
#define ALL_PROJ_STRS 20
#define ALL_UI_ELEMENTS 40
#define UI_WIDTH 1920
#define UI_HEIGHT 1080
#define WHITE { 255, 255, 255, 255 }
#define RED  { 255, 0, 0, 255 }
#define BLUE  { 0, 0, 255, 255 }
#define BLACK  { 0, 0, 0, 255 }
#define GREEN  { 0, 255, 0, 255 }
#define UI_SCROLL_MAX 3 //how many patterns or mons or listed objects can be on screen

static SDL_Color white = { 255, 255, 255, 255 };
static SDL_Color red = { 255, 0, 0, 255 };
static SDL_Color blue = { 0, 0, 255, 255 };
static SDL_Color black = {0, 0, 0, 255 };
static SDL_Color highlight = { 0,20,178, 175 };
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static TTF_Font* font = NULL;
static TTF_Font* gm_font = NULL;
float window_x_scale;
float window_y_scale;
int window_w, window_h;
int drawable_w, drawable_h;
SDL_Rect bounds;

//TRACK
float center_line_y;
float center_line_y2;

void render_rect(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color);
void render_track();

void scroll_track();
bool track_rendered = false;
int get_last_rect();
int check_player_at_end();
int track_one_rc;
int track_two_rc;


//data structure of generated track
typedef struct {
    SDL_FRect corner[MAX_RECTS];



}New_Corner;
New_Corner new_track[MAX_CORNERS];//bottom
New_Corner new_track2[MAX_CORNERS];//top
int create_corner(int corner_number);
int create_corner2(int corner_number);

float corner_x = 100.0;
float corner_x2 = 100.0;
float track_min;
float track_max;
float get_track_min();
float get_track_max();
bool track_generated = false;

int frame_delay = 10;
Uint32 now;
Uint32 last_time;

void move_player();
SDL_FRect player_rect;
SDL_FRect ship_mon_right;
SDL_FRect ship_mon_left;
bool player_mons_init;
void load_ship_mons();


//MOVEMENT
bool right_pressed;
bool left_pressed;
bool up_pressed;
bool down_pressed;
bool space_pressed;
bool acc_started;
bool deacc_started;
bool shift_pressed;
bool esc_pressed;
bool scrollup_pressed;
bool scrolldown_pressed;

float player_velocity = 0.2*20;//for button input controls(without culling must be higher due to hardware slowdown. might be something of a problem once release build)
float track_velocity = 10.0 * 2;
float max_track_vel = 25.0 * 2;
float acceleration = 0.0;//unused?
float projectile_velocity = 0.4 * 20;

Uint32 acc_start_time;
Uint32 acc_elapsed;
Uint32 deacc_start_time;
Uint32 deacc_elapsed;
void repel_around_player();
//COLLISIONS
int check_rect_overlap(SDL_FRect* a, SDL_FRect* one, SDL_FRect* two);
int track_coll_count;
bool coll_top;
bool coll_bottom;
bool coll_started_t;
Uint32 coll_start_time_t;
Uint32 coll_elapsed_t;
bool coll_started_b;
Uint32 coll_start_time_b;
Uint32 coll_elapsed_b;
Uint32 coll_tb_delay;
bool repelling;
Uint32 repel_start;
Uint32 repel_elapsed;
Uint32 repel_delay=20;


void handle_colls();
void accelerate();

float world_velocity = WORLD_VELOCITY_DEBUG;//DEBUG change back to 5.0
//ITEMS STRUCT
typedef struct {
    SDL_FRect items[MAX_ITEMS];
}TrackItems;
TrackItems track_items;
SDL_FRect gen_rand_item();
void populate_track_items();
bool track_populated;
void draw_track_items();
SDL_FRect gen_rand_mon_locations();
void populate_mons();
void draw_mons();
void check_item_pickups();

//MOVE STRUCT

typedef struct {
    char name[40];
    int power;
    char type[40];

}Move;

//MONS STRUCT
typedef struct {
    char name[40];
    char type[40];
    int HP;
    int att;
    int def;
    int spd;
    char wk[40];
    char res[40];
    bool encountered;//if true when shot on track, will be added to the battle
    int turn;//for turn order
    int att_ind;//for battle array to easily parse who attacks who
    int mov_ind;//for easy access to battle move index data
    bool is_in_party; //so not to have to work in player mon_array vs battle array
    SDL_FRect rect;
    Move moves[MAX_MOVES];
    char proj_type[40];//to load top and bottom_proj based on mons

}Monster;
Move moves[MAX_MOVES];

void render_text(float x, float y, SDL_Color color, const char* text, TTF_Font* font);

//MON DATA
Monster monsters[MAX_MONS] = {
    {.name = "test0", .type = "type0", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 101, .wk ="A", .res ="B", .encountered = false,.turn= 1, .att_ind = NULL, .mov_ind= NULL,.is_in_party=false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name ="mov", .power=10, .type= "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test1", .type = "type1", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 104,.wk = "A", .res = "B", .encountered = false, .turn = 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "sine"},
    {.name = "test2", .type = "type2", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 109,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test3", .type = "type3", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 103,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL,.mov_ind = NULL, .is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test4", .type = "type4", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 106,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL, .mov_ind = NULL,.is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test5", .type = "type5", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 102,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
};
Monster battle_array[MAX_MONS+2];//with player mons added
Monster player_mons[2];
void populate_mon_array();
bool mon_arr_populated;
void render_battle_mons();
void init_player_mons();

//PROJECTILES
typedef struct {
    SDL_FRect rects;
    char type[40];//ie sine wave
}Projectile;
Projectile projectile;
Projectile player_proj;
Projectile ship_top_proj;
Projectile ship_bottom_proj;
bool proj_started;
Uint32 proj_start_time;
Uint32 proj_elapsed;
void check_hit_events();

//gets current mon on screen so coord can be loaded
int curr_mon_on_screen();
int get_other_on_screen();
void load_projectile();
bool projectiles_loaded = false;
int accel_projectile();
bool proj_pressed;
bool player_proj_started;
Uint32 player_proj_start_time;
Uint32 player_proj_elapsed;
//projectile variants
void accel_player_proj();


void load_player_proj();
bool player_loaded;
bool enemy_hit_player = false;
bool player_hit_enemy = false;
bool player_hit_other_enemy = false;
//projectile colls
void handle_proj_colls();//function handles timing of when HP is subtracted, and how longit is displayed
Uint32 proj_coll_start;
Uint32 proj_coll_elapsed;
bool proj_coll_started;
Uint32 proj_coll_start_en;
Uint32 proj_coll_elapsed_en;
bool proj_coll_started_en;

Uint32 saw_period = 30;
Uint32 saw_start_time;
Uint32 saw_elapsed;
int saw_sign = -1;

typedef struct {
    char str[40];
    bool owned;
}ProjectileString;
ProjectileString world_proj_strs[ALL_PROJ_STRS] = {
    {"normal"},
    {"sine"},
    {"saw"}
};
ProjectileString player_proj_strs[ALL_PROJ_STRS] = {
    {"normal"},
    {"sine"},
    {"saw"},
    {"foo"},
    {"bar"},
};



//GAME STATE ENUM
typedef enum {
    GAMESTATE_TRACK,
    GAMESTATE_BATTLE_START,
    GAMESTATE_WORLD,
    GAMESTATE_GRIDMAKER,
    GAMESTATE_SPRITESHEET


}GameState;

GameState game_state = GAMESTATE_TRACK;
//MENU STATE ENUM(to be checked when in relevant states in nested switch)
typedef enum {
    MENUSTATE_MAIN,
    MENUSTATE_FIGHT1,
    MENUSTATE_FIGHT2,
    MENUSTATE_FIGHT3,//the same as 1 but for the 2nd mon
    MENUSTATE_FIGHT4,//the same as 2 but for the 2nd mon
    MENUSTATE_APPLY_ATT,
    MENUSTATE_NONE,//empty menu state
}MenuState;

MenuState menu_state = MENUSTATE_MAIN;

//MOUSE
bool mouse_in_valid_zone = true;
float mouse_x = 0.0, mouse_y = 0.0;
void check_mouse();//checks if mouse in valid area and gets mouse gap
float player_mouse_gap; //modifies the rate of y change

//Health
int player_hp = MAX_PLAYER_HP;
const char player_hp_buff[40];
const char mon_hp_buff[40];
bool player_hp_started;
bool enemy_hp_started;
bool other_enemy_hp_started;
Uint32 player_hp_start_time;
Uint32 enemy_hp_start_time;
Uint32 player_hp_elapsed;
Uint32 enemy_hp_elapsed;
Uint32 enemy_hp_delay = 300;
Uint32 player_hp_delay = 300;
void collision_events();
//BATTLE

//RENDER LOCATIONS
typedef struct {
    SDL_FRect battle_locs[MAX_MONS+2];
    SDL_FRect battle_menu[5];
}RenderRects;
static RenderRects render_rects = {
    .battle_locs = {
        {.x = 100, .y = 400, .w = MON_WIDTH, .h = MON_HEIGHT},
        {.x = 100 + MON_WIDTH, .y = 400, .w = MON_WIDTH, .h = MON_HEIGHT},
        {.x = 100 + MON_WIDTH*2, .y = 400, .w = MON_WIDTH, .h = MON_HEIGHT},
        {.x = 100 + MON_WIDTH*3, .y = 400, .w = MON_WIDTH, .h = MON_HEIGHT},
        {.x = 100 + MON_WIDTH * 4, .y = 400, .w = MON_WIDTH, .h = MON_HEIGHT},
        {.x = 100 + MON_WIDTH * 5, .y = 400, .w = MON_WIDTH, .h = MON_HEIGHT},
        {.x = 100, .y = 900, .w = MON_WIDTH, .h = MON_HEIGHT},//player mon 1
        {.x = 100 + MON_WIDTH, .y = 900, .w = MON_WIDTH, .h = MON_HEIGHT}//player mon 2
    },
    .battle_menu = {
        //background
        {.x = WINDOW_WIDTH - 400, .y = WINDOW_HEIGHT -500, .w= 600, .h = 400},
        // MENU DESIGN, 4 rects that will be used for all information
    {.x = WINDOW_WIDTH - 400 + 50, .y = WINDOW_HEIGHT - 200 - 50 - 150, .w = 150, .h = 100  },
        {.x = WINDOW_WIDTH - 400 + 200, .y = WINDOW_HEIGHT - 200 - 50 - 150, .w = 150, .h = 100  },
        {.x = WINDOW_WIDTH - 400 + 50, .y = WINDOW_HEIGHT - 200 - 50, .w = 150, .h = 100  },
        {.x = WINDOW_WIDTH - 400 + 200, .y = WINDOW_HEIGHT - 200 - 50, .w = 150, .h = 100  },

        
        
    }


};
//MENU RENDERING
void render_menu_hl(int ind);
void render_bm_moves(int mon_ind);
void render_bm_main();
void render_bm_bg();

int hl_ind=1;//index of menu highlight
int player_mon_ind = 0;
bool supress_menu_press = false;
bool menu_sup_started;
Uint32 menu_sup_start_time;
Uint32 menu_sup_elapsed;
Uint32 menu_sup_delay=500;
void handle_menu_press();
void render_mon_hl(int ind);
int get_enemy_count();
int enemy_count;
bool enemy_count_gotten;
void decide_turn_order(size);
bool turns_decided;
Monster* dynamic_battle_array;
int allocate_dynamic_battle_arr();
int dynamic_size;
bool battle_hp_started;
Uint32 battle_hp_start_time;
Uint32 battle_hp_elapsed;
Uint32 battle_hp_delay = 500;
int apply_attacks(int i, Monster *target, Monster *attacker);
bool attacks_applied;
char battle_hp_buff[40];
bool hp_buff_filled;
int hp_ind;
void set_enemy_decisions(Monster *mon);
bool enemy_decisions_set;
bool check_all_defeated();
void reset_mon_inds();
bool flags_reset;
void reinit_track_vars();
void reinit_battle_vars();
int get_first_enemy_in_dba();
bool hp_subbed;;

//FRAME TIMING
Uint32 frame_time;
Uint32 last_time;
Uint32 frame_start;
const Uint32 target_frame_time = 8; // ~8 ms per frame (1000/120)
int opt_mult = 10;//for speed optimization

//TILES

typedef struct {
    int stop_ind;
    bool passed;
    char id[40]; //identifying 4 digit string
}DialogueStop;//a dialogue stop ind perhaps should never be 0, so we can check that condition, and decide if one exists at the next index in the ds array
typedef struct {
    int row;
    int col;
}KeyItemLoc;
typedef struct {
    char name[40];
    bool obtained;
    SDL_FRect rect;
    KeyItemLoc location;
    int ds;//index for which dialogue this passes
    DialogueStop dialogue_stop;//for id check
}KeyItem;

typedef struct {
    SDL_FRect* rects;//pointer for rects to be allocated
    char loc_name[40];//name of the area
    int row;
    int col;
    //SDL_Texture loc_texture;// the texture that will be used
    bool is_walkable;
    int area_w;
    int area_h;
    size_t size;//need this to be able to iterate conditionally on size for rendering
    SDL_FRect bottom_right;//the bottom rightmost rect for easier bounds checking
    bool is_unlockable;
    int map_index; //an int 0-5. 0 for center map, which means no offset, 1 for the north map, which means -y offset, 2 for east +x offset, 3 for south + y offset, 4 for west -x offset. 5 for interiors drawn a double length away off screen, offset is map_w and map_h
    KeyItem key;
    //TODO texture
} World_Area;

typedef struct {
    SDL_FRect rect;//pointer for rects to be allocated
    int row;//under tile row and col to be cross referenced with the world tiles
    int col;
} Under_Tile;


//arrays of the above types , the undertiles will be generated, and the hard coded world areas will be placed and rendered on their coordinates
World_Area world_areas[ALL_AREAS]={
    {NULL, "Test Area 0", 1, 1, true, 2,2,0, .bottom_right={.w=0, .h=0, .x=0, .y=0}, false},
    {NULL, "Test Area 1", 2, 3, true, 5,10,0,  .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Test Area 2",12, 8, true, 20,10,0,  .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0},false},
    {NULL, "Indust Ground 0", 32, 86, true, 13,13,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0},false},
    {NULL, "Indust Ground 1", 41, 99, true, 5,9,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 2", 45, 81, true, 10,10,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 3", 44, 104, true, 6,2,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 4", 48, 104, true, 6,2,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 5", 52, 104, true, 6,2,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 6", 50, 101, true, 3,15,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 7", 55, 66, true, 25,6,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 8", 61, 71, true, 2,5,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 9", 61, 78, true, 2,5,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Indust Ground 7", 61, 87, true, 4,3,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, false},
    {NULL, "Blocked Area 0",  22, 23, false, 4, 8,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, true},
    {NULL, "Blocked Area 1",  33, 33, false, 4, 8,0, .bottom_right = {.w = 0, .h = 0, .x = 0, .y = 0}, true},

};
Under_Tile under_tiles[AREA_TILE_LIM];
void init_under_tiles();
void init_world_areas();
int world_row = 0;
int world_col = 1;//skip top left
void render_world_areas();
bool world_initiated;
bool printed = false;
int curr_area_size = 0;
bool place_world_areas(int i, int row, int col);
int tile_y_offset;
void move_world();


typedef struct {
    int direction;//0,1,2,3,4 clockwise
    Uint32 timestamp; //so we can see which cam first, therefore give priority
}Input;
//we only need 2 elements in the array, because only the oldest input is accepted for world movement, if nothing is left NULL, we dont accept any more input
Input input_queue[2]= {{-1, NULL}, {-1, NULL}}; 
bool input_queue_full();
int queue_ind=-1;

void reset_inp_queue();

//GRID MAKER

SDL_FRect gm_rects[MAX_GM];
void render_gm_rects();
void set_gm_rects();
bool gm_rects_set;
char* gm_buffer[MAX_GM];
bool gm_buffers_set;
SDL_Texture* gm_textures[MAX_GM];
void create_gm_texts(float x, float y, SDL_Color color, const char* text, TTF_Font* font, int i);
bool gm_loaded;
int gm_map_w = ALL_COLS * GM_WIDTH + (ALL_COLS * GM_MARGIN);
int gm_map_h = ALL_ROWS * GM_HEIGHT + (ALL_ROWS * GM_MARGIN);
SDL_Texture* gm_target;
int output_gm_bmp(void);
bool bmp_output;

//WORLD CHARACTERS

typedef struct {
    char buffer[80];
}Dialogue;



typedef struct {
    char name[40];
    SDL_FRect rect;
    int lrow;
    int lcol;
    Dialogue dialogues[MAX_DIAS];//an array of 20 char buffers that are 80 chars wide each, will be initiated with a function TODO
    KeyItem goal_items[MAX_GOAL_ITEMS]; //an array of potential key items that the character wants to the player to obtain for matcing and deciding goals achieved
    KeyItem reward_items[MAX_GOAL_ITEMS]; //an array of potential reward key items for goals achieved
    DialogueStop dialogue_stops[MAX_GOAL_ITEMS]; //an array of 3 int/ bool, each int of which has the value of the stopping point of the index in dialogues, and a bool to flag if it can be passed based on key item conditions
    int ds_ind;//to track progression through additional dialogue stops(if any)
}Character;

typedef struct {
    SDL_FRect rect;
    int lrow;
    int lcol;
    KeyItem key_items[ALL_KEYITEMS];
}WorldPlayer;

WorldPlayer world_player = { .rect = {.h = TILE_HEIGHT ,.w = TILE_WIDTH, .x = (WINDOW_WIDTH / 2) - (TILE_WIDTH / 2), .y = (WINDOW_HEIGHT / 2) - (TILE_HEIGHT / 2) }, .lrow = 1,.lcol = 1 };//will decide inital starting point and other location force changes with lrow and lcol

Character characters [ALL_CHARACTERS] = {
    {NULL},
    {.name ="testy_tim", .rect = {.h = TILE_HEIGHT * 2,.w = TILE_WIDTH, .x = 0, .y = 0 } , 32, 86},
    {.name = "fooly_barton", .rect = {.h = TILE_HEIGHT * 2,.w = TILE_WIDTH, .x = 0, .y = 0 } , 55, 100}
};
KeyItem world_goal_items[ALL_KEYITEMS];
void init_char_dialogues();
void init_character_locs();
void render_characters();
void init_key_items();
bool supress_interact;
bool entered_dia;
void progress_character_dia();
int handle_character_interact();
int get_next_empty_keyitem();
int interact_char_ind;
void render_key_items();
void handle_key_item_pickup();
int dia_ind = -1;
void collisions_world();
bool world_vel_inverted;
Uint32 vel_inv_start;
Uint32 vel_inv_elapsed;
Uint32 vel_inv_delay=100;
bool world_player_coll;
void unlock_areas();

//SPRITESHEETS / ANIMATION
int spritesheet_size = SPRITESHEET_FRAME_ROWS * SPRITESHEET_FRAME_COLS;
SDL_FRect spritesheet_rects[SPRITESHEET_FRAME_ROWS * SPRITESHEET_FRAME_COLS];
void set_spritesheet_rects();
bool spritesheet_set;
void render_spritesheet();
SDL_Texture* spritesheet_target;
int spritesheet_w = SPRITESHEET_FRAME_COLS * SPRITE_WIDTH;
int spritesheet_h = SPRITESHEET_FRAME_ROWS * SPRITE_HEIGHT;
int output_spritesheet_bmp(void);
bool spritesheet_bmp_output;
SDL_FRect walking_src;
SDL_FRect walking_dst;
int walk_curr_frame=0;
Uint32 walk_last_time;
Uint32 walk_frame_delay = 100;
int walk_total_frames = SPRITESHEET_FRAME_COLS;
void render_animation(
    int total_frames,
    int* current_frame,
    Uint32* last_time,
    Uint32* now,
    Uint32 frame_delay,
    int x, int y,
    SDL_Texture* texture,
    SDL_Renderer* renderer,
    SDL_FRect src,
    SDL_FRect dst);
SDL_FRect create_dst(int frame_width, int frame_height, int current_frame, int x, int y);
SDL_FRect create_src(int frame_width, int frame_height, int current_frame, int x, int y);
int walk_surface_w = 0;
int walk_surface_h = 0;
SDL_Texture* walk_texture = NULL;

//USER INTERFACES

typedef struct {
    int layer; //if the layer is below 2, always render the element unconditionally and their blankets will be layered on top conditionally
    SDL_FRect rect;//large background menu rendered behind
    char label[120];
    int index; //initialize by function by iterating through the UI array
    bool got_activated; //when the element is clicked, set to true
    int blanket_ind;//the index of the UI that will render as a consequence of the click (can be -1 for condition check)
    bool is_sub_display;//if it is true, then it will check what its name is with strcmp, and then render the relevant list to be scrolled through
    SDL_Color color;
    bool interactable;
    bool offscreen;
}UI_Element;

#define ui_center_x ( WINDOW_WIDTH / 2) -( UI_WIDTH / 2)
#define ui_center_y ( WINDOW_HEIGHT / 2) - (UI_HEIGHT/ 2)
#define ui_left_x  (WINDOW_WIDTH / 2) - (UI_WIDTH / 2) 
#define ui_left_y  (WINDOW_HEIGHT / 2) - (UI_HEIGHT / 2) 
#define ui_right_x  (WINDOW_WIDTH / 2) - (UI_WIDTH / 2) + 400
#define ui_right_y  (WINDOW_HEIGHT / 2) - (UI_HEIGHT / 2) + 400

int element_diplay_ind;//the index of the current element being diplayed over the background out of the elements array. (is_clicked? - render index or blanket index of array)

//this type is for an array that can be populated dynamically based on what is being scrolled, like mons list or patterns list
typedef struct {
    char display_text[40];
    //later a texture member can go alongside here
}UI_Sub_Display;
UI_Sub_Display curr_sub_disp_buff [UI_SCROLL_MAX];
bool sub_buffer_filled;

//when the user scroll, each index gets increased or decreased by 1 depending on scroll direction, and the display_text members of the curr_sub_disp_buff are snprintfd to the index of the list in question
int scroll_indices [UI_SCROLL_MAX] = {0,1,2}; 

/*/these will be rendered using a function/functions, this data structure will help facilitate the UI rendering. 
if the layer is < 2, render the element
    if the label is strcmped to be equipped patterns, render the player_proj.type text there
    if the label is strcmped to be equipped mons, render the player_mons.name there 
if the element got activated and has a blanket, render that element until back button is pressed once
if the element is a sub display, check what its label name is, then snprintf the scroll indices of the curr_disp_buff , into the display_text member
if the user presses an equip (equip left or equip right separate for mons) or info button, the middle of the curr_disp_buff is selected/*/

UI_Element ui_elements[ALL_UI_ELEMENTS] = {
    {0,{.h = UI_HEIGHT, .w= UI_WIDTH, .x=ui_center_x, .y= ui_center_y},"background", 0, false, -1,false, WHITE,false},
    {1,{.h = 400, .w = 400 ,.x = ui_left_x, .y = ui_left_y},"equipped pattern", 1, false, 2,false, BLUE, true},
    {2,{.h = 400, .w = 400 ,.x = ui_left_x, .y = ui_left_y},"your patterns", 2, false, 6, true, RED, true},//this will display the list of pattern strings, scrollable, scrolling just changes which indices are being rendered onto this rect. can equip pattern with button event?
    {1,{.h = 400, .w = 400 ,.x = ui_left_x, .y = ui_right_y},"equipped mons", 3, false, 4,false, BLUE, true},
    {2,{.h = 400, .w = 400 ,.x = ui_left_x, .y = ui_right_y},"your mons", 4, false, 5,true,  RED, true}, //this will display the list of mon names, scrollable, scrolling just changes which indices are being rendered onto this rect, can equip mon with buton event?
    {3,{.h = 400, .w = 400 ,.x = ui_left_x, .y = ui_right_y},"mon info", 5, false, 7, false, GREEN, true },//when a mon is selected in the select index when scrolling, will render that mons info
    {4, {.h =400, .w =400, .x =ui_left_x-500, .y= ui_left_y}, "yn prompt pattern", 6, false, -1, false, BLUE, true},
    {4, {.h = 400, .w = 400, .x = ui_left_x-500, .y = ui_right_y}, "yn prompt mon", 7, false, -1, false, BLUE, true},


};
void render_ui();

bool ui_back_pressed;

Monster player_mon_list[MAX_MONS] = {
    {.name = "test0", .type = "type0", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 101, .wk = "A", .res = "B", .encountered = false,.turn = 1, .att_ind = NULL, .mov_ind = NULL,.is_in_party = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test1", .type = "type1", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 104,.wk = "A", .res = "B", .encountered = false, .turn = 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "sine"},
    {.name = "test2", .type = "type2", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 109,.wk = "A",.res = "B",.encountered = false,.turn = 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test3", .type = "type3", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 103,.wk = "A",.res = "B",.encountered = false,.turn = 1, .att_ind = NULL,.mov_ind = NULL, .is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test4", .type = "type4", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 106,.wk = "A",.res = "B",.encountered = false,.turn = 1, .att_ind = NULL, .mov_ind = NULL,.is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
    {.name = "test5", .type = "type5", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 102,.wk = "A",.res = "B",.encountered = false,.turn = 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}, "norm"},
}; //player_mons[2] are the equipped mons, this is the entire list, is in party determines equip

long ui_open;//even values are closed
int mid_scr_ind;
float offscreen_x = -1000;
float x_copy = 0.0; //for saving position of UI before moving offscreen
bool mon_list_size_gotten;
size_t mon_list_size;
size_t get_mon_list_size();
void handle_scroll_event();
float wheel_y;

typedef enum {
    UIS_CLOSED,
    UIS_IDLE,
    UIS_MON_LIST_OPEN,
    UIS_PATTERN_LIST_OPEN,
    

}UI_STATE;
UI_STATE ui_state = UIS_CLOSED;

int pattern_list_size;
int get_pattern_list_size();

bool pattern_list_size_gotten;

bool yesno;
//INITIALIZATION
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize SDL_ttf: %s", SDL_GetError());
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    SDL_GetWindowSize(window, &window_w, &window_h);             // logical
    SDL_GetWindowSizeInPixels(window, &drawable_w, &drawable_h); // physical
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);//CENTER

    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displayID);
    SDL_GetDisplayBounds(displayID, &bounds);
    window = SDL_CreateWindow("FTypeBattle", bounds.w, bounds.h, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Couldn't create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    //DISPLAY

//set window conditionally based on bounds
    if (bounds.w < 3840 && bounds.w>1920) {
        drawable_w = 2560;
        drawable_h = 1440;


    }
    else if (bounds.w < 2560 && bounds.w>1280) {
        drawable_w = 1920;
        drawable_h = 1080;
    }
    else if (bounds.w < 1920 && bounds.w>640) {
        drawable_w = 1280;
        drawable_h = 720;
    }
    else {
        drawable_w = bounds.w;
        drawable_h = bounds.h;
    }

    window_x_scale = (float)drawable_w / WINDOW_WIDTH;
    window_y_scale = (float)drawable_h / WINDOW_HEIGHT;
    SDL_SetRenderScale(renderer, window_x_scale, window_y_scale);

    SDL_Log("Logical size: %dx%d, Drawable size: %dx%d, Scale: %.2fx, %.2fy",
        window_w, window_h, drawable_w, drawable_h, window_x_scale, window_y_scale);
    font = TTF_OpenFont("./fonts/amiga.ttf", 35);
    if (!font) {
        SDL_Log("Couldn't load font: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    gm_font = TTF_OpenFont("./fonts/RobotoMono-Regular.ttf", 8);
    if (!gm_font) {
        SDL_Log("Couldn't load font: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return SDL_APP_FAILURE;
    }
    center_line_y = WINDOW_HEIGHT / 2 + 300;
    center_line_y2 = WINDOW_HEIGHT / 2 - 600;
    player_rect.x = 100.0, player_rect.y = WINDOW_HEIGHT / 2, player_rect.w = PLAYER_WIDTH, player_rect.h = PLAYER_HEIGHT;
    ship_mon_left.x = 100.0, ship_mon_left.y = WINDOW_HEIGHT / 2 - PLAYER_HEIGHT - 20, ship_mon_left.w = PLAYER_WIDTH, ship_mon_left.h = PLAYER_HEIGHT;
    ship_mon_right.x = 100.0, ship_mon_right.y = WINDOW_HEIGHT / 2 + PLAYER_HEIGHT + 20, ship_mon_right.w = PLAYER_WIDTH, ship_mon_right.h = PLAYER_HEIGHT;
    snprintf(player_proj.type,sizeof(player_proj.type), "saw");
    //snprintf(ship_top_proj.type, sizeof(ship_top_proj.type), "norm");
    //snprintf(ship_bottom_proj.type, sizeof(ship_bottom_proj.type), "norm");

    srand((unsigned)time(NULL)); // seed
    gm_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, gm_map_w, gm_map_h);
    spritesheet_target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, spritesheet_w, spritesheet_h);
    


    SDL_Surface* walk_surface = NULL;
    walk_surface = SDL_LoadBMP("./textures/spritesheet_testing.bmp");
    if (!walk_surface) {
        SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    walk_surface_w = walk_surface->w;
    walk_surface_h = walk_surface->h;

    walk_texture = SDL_CreateTextureFromSurface(renderer, walk_surface);
    if (!walk_surface) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(walk_surface);

    return SDL_APP_CONTINUE;
}

//GAME EVENTS
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {

        if (!input_queue_full()) {
            switch (event->key.key) {
            case SDLK_D:
                if (!coll_started_t && !coll_started_b) {
                    right_pressed = true;
                }
                break;
            case SDLK_A:
                if (!coll_started_t && !coll_started_b) {
                    left_pressed = true;
                }
                break;
            case SDLK_S:
                if (!coll_started_t && !coll_started_b) {
                    down_pressed = true;
                }
                break;
            case SDLK_W:
                if (!coll_started_t && !coll_started_b) {
                    up_pressed = true;
                }
                break;
            case SDLK_SPACE:
                space_pressed = true;
                break;
            case SDLK_LSHIFT:
                shift_pressed = true;
                break;
            case SDLK_ESCAPE:
                esc_pressed = true;
                break;
            default:
                break;
            }
        }
    }

    if (event->type == SDL_EVENT_KEY_UP) {
        reset_inp_queue();
        switch (event->key.key) {
        case SDLK_D:
            right_pressed = false;
            break;
        case SDLK_A:
            left_pressed = false;
            break;
        case SDLK_S:
            down_pressed = false;
            break;
        case SDLK_W:
            up_pressed = false;
            break;
        case SDLK_SPACE:
            space_pressed = false;
            break;
        default:
            break;
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {//TODO seperate this logic by game state switch
        SDL_FPoint p = { (float)event->button.x / window_x_scale, (float)event->button.y / window_y_scale };
        proj_pressed = true;
        
        for(int i =0; i<ALL_UI_ELEMENTS; i++){
            if(SDL_PointInRectFloat(&p, &ui_elements[i].rect) && ui_elements[i].interactable){
                ui_back_pressed = false;
                element_diplay_ind = i;
                if (strcmp(ui_elements[i].label, "yn prompt pattern") == 0 || strcmp(ui_elements[i].label, "yn prompt mon") == 0)  {

                    if (p.x > (ui_elements[i].rect.x + ui_elements[i].rect.w / 2)) {
                        yesno = false;
                        
                    }
                    else {
                        yesno = true;
                        SDL_Log("yesno true");
                        
                    }
                }
                break;
                

            }

            
        }
        ui_elements[element_diplay_ind].got_activated = true;
        if(event->button.button== SDL_BUTTON_RIGHT){
            ui_back_pressed = true;
        }
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        proj_pressed = false;
    }

    if(event->type == SDL_EVENT_MOUSE_WHEEL){
        SDL_Log("wheel detected");

        
        if(event->wheel.y > 0){
            SDL_Log("direction >0");
            scrollup_pressed = true;
        }

        if (event->wheel.y < 0) {
            SDL_Log("direction <0");
            scrolldown_pressed = true;
        }
        


    }

    switch(game_state){
    case GAMESTATE_TRACK:
    {
        check_item_pickups();
        check_hit_events();

        break;
    }
    }

    return SDL_APP_CONTINUE;
}
//APP ITERATE
SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);//set to the amiga color for the background of the main game screen
    SDL_RenderClear(renderer);


    SDL_GetMouseState(&mouse_x, &mouse_y);

    now = SDL_GetTicks();
    frame_start = SDL_GetTicks();

    //create animation sources and destinations at the start/top of each loop cycle
    walking_src = create_src(SPRITE_WIDTH, SPRITE_HEIGHT, walk_curr_frame, world_player.rect.x, world_player.rect.y);
    walking_dst = create_dst(SPRITE_WIDTH, SPRITE_HEIGHT, walk_curr_frame, world_player.rect.x, world_player.rect.y);

    //GAMESTATE SWITCH START
    switch (game_state) {

    case GAMESTATE_TRACK:
    {
        //generate track
        if (!track_generated) {
            menu_state = MENUSTATE_NONE;
            for (int i = 0; i < MAX_CORNERS; i++) {
                create_corner(i);
                create_corner2(i);
                SDL_Log("x %f", new_track[i].corner[0].x);

            }
            track_max = get_track_max();
            track_min = get_track_min();
            SDL_Log("Track generated");
            track_generated = true;
        }
        if (!track_populated) {
            populate_track_items();
            populate_mons();
            track_populated = true;
        }
        if(!player_mons_init){
            init_player_mons();
            load_ship_mons();
            player_mons_init = true;
        }
        render_track();
        render_rect(renderer, player_rect, white);
        render_rect(renderer, ship_mon_left, white);
        render_rect(renderer, ship_mon_right, white);
        render_rect(renderer, player_mons[0].rect, blue);
        render_rect(renderer, player_mons[1].rect, blue);
        move_player();
        handle_colls();
        repel_around_player();

        scroll_track();
        accelerate();

        draw_track_items();
        draw_mons();

        if (!projectiles_loaded) {
            load_projectile();
        }


        accel_projectile();
        if (!player_loaded) {
            load_player_proj();
        }
        accel_player_proj();
        handle_proj_colls();
        check_mouse();
        collision_events();
        if(check_player_at_end()==1){
            coll_started_t = false;
            coll_started_b = false;
            coll_bottom = false;
            coll_top = false;
            SDL_Log("Game_state: %d", game_state+1);
            supress_menu_press = true;//in case holding space
            game_state = GAMESTATE_BATTLE_START;
            break;
            
        }
        if(esc_pressed){
            ui_open += 1;
            ui_state = UIS_IDLE;
            if(!mon_list_size_gotten){
                mon_list_size = get_mon_list_size();
                mon_list_size_gotten = true;
            }
            if (!pattern_list_size_gotten) {
                pattern_list_size = get_pattern_list_size();
                pattern_list_size_gotten = true;
            }

        }
        if (ui_open % 2 != 0) {
            
            render_ui();
            handle_scroll_event();
        }
        else{
            //make sure a different sized list including new mons or patterns can be accounted for whenever ui is not open

            ui_back_pressed = true;


            mon_list_size_gotten = false;
            pattern_list_size_gotten = false;

            ui_state = UIS_CLOSED;
        }
        esc_pressed = false;

        break;
    }
    case GAMESTATE_BATTLE_START:
    {

        if(!mon_arr_populated){

            init_player_mons();
            populate_mon_array();
            if (!turns_decided) {
                dynamic_size = allocate_dynamic_battle_arr();
                decide_turn_order(dynamic_size);
                supress_menu_press = true;//in case holding space
                menu_state = MENUSTATE_MAIN;
                turns_decided = true;

            }

            mon_arr_populated = true;
        }
        if(!enemy_count_gotten){
            enemy_count = get_enemy_count();
            enemy_count_gotten = true;
        }

        render_bm_bg();
        render_battle_mons();

        switch(menu_state){

            case MENUSTATE_MAIN:
            {
                
                if (!flags_reset) {
                    reinit_battle_vars();


                    flags_reset = true;
                }
                render_bm_main();
                handle_menu_press();
                if(space_pressed && hl_ind ==1){
                    supress_menu_press = true;
                    SDL_Log("menu_state: %d", menu_state + 1);
                    menu_state = MENUSTATE_FIGHT1;
                    break;
                }
                if(space_pressed && hl_ind ==3){//run
                    reinit_track_vars();
                    flags_reset = false;
                    game_state = GAMESTATE_TRACK;
                }
                if (right_pressed && hl_ind < 3) {
                    if(!supress_menu_press){
                        hl_ind += 1;
                        supress_menu_press = true;
                    }
                }
                if (left_pressed && hl_ind > 1) {
                    if (!supress_menu_press) {
                        hl_ind -= 1;
                        supress_menu_press = true;
                    }
                }
                render_menu_hl(hl_ind);
                break;
            }
            case MENUSTATE_FIGHT1:
            {
                handle_menu_press();
                render_bm_moves(player_mon_ind);
                if (right_pressed && hl_ind < 4) {
                    if(!supress_menu_press){
                        hl_ind += 1;
                        
         
                            
                        supress_menu_press = true;
 
                    }
                }
                if (left_pressed && hl_ind >1) {
                    if (!supress_menu_press) {
                        hl_ind -= 1;
              
                            
                        supress_menu_press = true;
   
                    }
                }
                if(space_pressed){
                    if (!supress_menu_press){
                        for (int i = 0; i < dynamic_size; i++) {
                            if (dynamic_battle_array[i].is_in_party && strcmp(player_mons[player_mon_ind].name, dynamic_battle_array[i].name)==0) {//it may be better to do ID system instead of name
                                dynamic_battle_array[i].mov_ind = hl_ind-1;//set the mon's mov_ind to selected move
                            }
                        }
                        supress_menu_press = true;
                    }
                    hl_ind =  get_first_enemy_in_dba();
                    SDL_Log("menu_state: %d", menu_state + 1);
                    menu_state = MENUSTATE_FIGHT2;
                    break;

                }
                render_menu_hl(hl_ind);
                
                break;
            }
            case MENUSTATE_FIGHT2:
            {
                handle_menu_press();
                render_mon_hl(hl_ind);
                if (right_pressed && hl_ind < enemy_count) {//enemy_count used here
                    if (!supress_menu_press) {
                        hl_ind += 1;



                        supress_menu_press = true;

                    }
                }
                if (left_pressed && hl_ind > 1) {
                    if (!supress_menu_press) {
                        hl_ind -= 1;
                        if(dynamic_battle_array[hl_ind-1].spd ==0){
                            hl_ind -= 1;
                        }

                        supress_menu_press = true;

                    }
                }
                if (dynamic_battle_array[hl_ind].spd == 0 && hl_ind < enemy_count || dynamic_battle_array[hl_ind].is_in_party) {//skip
                    hl_ind += 1;
                }
                if(space_pressed){
                    if(!supress_menu_press){
                        for (int i = 0; i < dynamic_size; i++) {
                            if (dynamic_battle_array[i].is_in_party && strcmp(player_mons[player_mon_ind].name, dynamic_battle_array[i].name) == 0) {//it may be better to do ID system instead of name
                                dynamic_battle_array[i].att_ind = hl_ind;//set the mon's mov_ind to selected move
                            }
                        }
                        player_mon_ind = 1;//set as next so moves can be rendered in next state
                        supress_menu_press = true;
                    }
                    hl_ind = 1;
                    SDL_Log("menu_state: %d", menu_state + 1);
                    menu_state = MENUSTATE_FIGHT3;
                    break;
                }

                break;
            }
            case MENUSTATE_FIGHT3:
            {
                handle_menu_press();
                render_bm_moves(player_mon_ind);
                if (right_pressed && hl_ind < 4) {
                    if (!supress_menu_press) {
                        hl_ind += 1;



                        supress_menu_press = true;

                    }
                }
                if (left_pressed && hl_ind > 1) {
                    if (!supress_menu_press) {
                        hl_ind -= 1;


                        supress_menu_press = true;

                    }
                }
                if (space_pressed) {
                    if (!supress_menu_press) {
                        for (int i = 0; i < dynamic_size; i++) {
                            if (dynamic_battle_array[i].is_in_party && strcmp(player_mons[player_mon_ind].name, dynamic_battle_array[i].name) == 0) {
                                dynamic_battle_array[i].mov_ind = hl_ind-1;//set the mon's mov_ind to selected move
                            }
                        }
                        supress_menu_press = true;
                    }
                    hl_ind = get_first_enemy_in_dba();
                    SDL_Log("menu_state: %d", menu_state + 1);
                    menu_state = MENUSTATE_FIGHT4;
                    break;

                }
                render_menu_hl(hl_ind);

                break;
            }
            case MENUSTATE_FIGHT4:
            {
                handle_menu_press();
                render_mon_hl(hl_ind);
                if (right_pressed && hl_ind < enemy_count) {//enemy_count used here
                    if (!supress_menu_press) {
                        hl_ind += 1;



                        supress_menu_press = true;

                    }
                }
                if (left_pressed && hl_ind > 1) {
                    if (!supress_menu_press) {
                        hl_ind -= 1;


                        supress_menu_press = true;

                    }
                }
                if (dynamic_battle_array[hl_ind].spd == 0 && hl_ind < enemy_count || dynamic_battle_array[hl_ind].is_in_party) {//skip
                    hl_ind += 1;
                }
                if (space_pressed) {
                    if (!supress_menu_press) {
                        for (int i = 0; i < dynamic_size; i++) {
                            if (dynamic_battle_array[i].is_in_party && strcmp(player_mons[player_mon_ind].name, dynamic_battle_array[i].name) == 0) {//it may be better to do ID system instead of name
                                dynamic_battle_array[i].att_ind = hl_ind;//set the mon's mov_ind to selected move
                            }
                        }
                        supress_menu_press = true;
                    }
                    SDL_Log("menu_state: %d", menu_state + 1);
                    menu_state = MENUSTATE_APPLY_ATT;//enter game_state where the attacks will be played out based on what was done above
                    break;
                }

                break;
            }
            case MENUSTATE_APPLY_ATT:

            {

                // set enemy's decisions based on RNG
                if (!enemy_decisions_set) {
                    for (int m = 0; m < dynamic_size; m++) {
                        if (!dynamic_battle_array[m].is_in_party) {
                            set_enemy_decisions(&dynamic_battle_array[m]);
                        }
                    }
                    enemy_decisions_set = true;
                }
                //apply the effects of the attacks within the dynamic battle array based on the indices gathered above
                if (!attacks_applied) {

                    //apply attacks 1 by 1 through the now ordered battle array
                    if (hp_ind < dynamic_size) {
                        if (apply_attacks(hp_ind, &dynamic_battle_array[dynamic_battle_array[hp_ind].att_ind], &dynamic_battle_array[hp_ind]) == 1) {
                            battle_hp_elapsed = 0;
                            hp_subbed = false;
                            hp_ind += 1;
                        }
                    }
                    else {
                        battle_hp_elapsed = 0;
                        attacks_applied = true;
                    }
                }
                else if (attacks_applied) {

                    //go back to menustate_main if at least one enemy  has HP
                    if (check_all_defeated() == false){
                        flags_reset = false;
                        SDL_Log("menu_state main");
                        menu_state = MENUSTATE_MAIN;
                        break;
                    }
                    else if(check_all_defeated() == true) {
                        reinit_track_vars();
                        flags_reset = false;
                        game_state = GAMESTATE_TRACK;

                        break;
                    }
                }
                break;
            }
            case MENUSTATE_NONE:
            {

                break;
            }
            
        }

        
        break;
    }
    case GAMESTATE_WORLD:
    {
        if(! world_initiated){
            init_under_tiles();
            init_world_areas();
            /*for (int w = 0; w < ALL_AREAS; w++) {
               if (place_world_areas(w, world_row, world_col)) {
                    //w++;
                }

            }*/
            init_character_locs();
            init_char_dialogues();
            init_key_items();
            SDL_Log("world initiated");
            world_initiated = true;

        }

        render_world_areas();
        render_rect(renderer, world_player.rect, white);

        render_characters();
        render_key_items();
 
        interact_char_ind = handle_character_interact();
        //SDL_Log("interact char index : %d", interact_char_ind);
        if (interact_char_ind==0){
            entered_dia = false;
            
            supress_interact = false;
            dia_ind = -1;
            interact_char_ind = 0;
           // SDL_Log("Dia ind reset - %d", dia_ind);
        }


        progress_character_dia();
        
        handle_key_item_pickup();
        move_world();
       
        render_animation(walk_total_frames, &walk_curr_frame, &walk_last_time, &now, walk_frame_delay, world_player.rect.x, world_player.rect.y, walk_texture, renderer, walking_src, walking_dst);
        collisions_world();
        unlock_areas();
        
        break;
    }
    case GAMESTATE_GRIDMAKER:
    {
        if(!gm_rects_set){
            set_gm_rects();
            gm_rects_set = true;
        }

        render_gm_rects();
        if (!bmp_output && gm_rects_set) {
            output_gm_bmp();
            bmp_output = true;

        }


    }
    case GAMESTATE_SPRITESHEET:
    {
        if(!spritesheet_set){

            set_spritesheet_rects();
            spritesheet_set = true;
        }
        render_spritesheet();
        if(!spritesheet_bmp_output && spritesheet_set){
            output_spritesheet_bmp();
            spritesheet_bmp_output = true;
        }
    }

    }

    //END GAMESTATE SWITCH
    SDL_RenderPresent(renderer);
    frame_time = SDL_GetTicks() - frame_start;
    if (frame_time < target_frame_time)
    {
        SDL_Delay(target_frame_time - frame_time);
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    TTF_Quit();
    SDL_Quit();


}


//functions
/*TRACK FUNCTIONS*/
int create_corner(int corner_number) {

    int roll;//random roll for above or below the center for the curve
    float y1;
    float y2;
    roll = rand() % 6;
    int rect_count1;
    int rect_count2;
    int rect_count3;
    float y1_center_diff;
    float y2_y1_diff;
    float y2_center_diff;
    //corner x globally defined
    float corner_y;
    corner_y = center_line_y;
    int rect_index = 0;
    //below center line
    if (roll <= 3) {
        y1 = center_line_y + 50 + rand() % 200;
        y2 = y1 + 50 + rand() % 200;
    }
    //above center line
    else {
        y1 = center_line_y - 50 - rand() % 200;
        y2 = y1 - 50 - rand() % 200;
    }
    y1_center_diff = y1 > center_line_y ? y1 - center_line_y : center_line_y - y1;
    y2_y1_diff = y2 > y1 ? y2 - y1 : y1 - y2;
    y2_center_diff = y2 > center_line_y ? y2 - center_line_y : center_line_y - y2;
    rect_count1 = ceil(y1_center_diff / RECT_HEIGHT);
    rect_count2 = ceil(y2_y1_diff / RECT_HEIGHT);
    rect_count3 = ceil(y2_center_diff / RECT_HEIGHT);

    //below
    if (y1 > center_line_y) {

        //first void
        for (int i = 0; i < rect_count1; i++, rect_index++) {
            new_track[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track[corner_number].corner[rect_index].x = corner_x;
            new_track[corner_number].corner[rect_index].y = corner_y;
            corner_x += RECT_WIDTH;
            corner_y += RECT_HEIGHT - RECT_HEIGHT / 2;//heading down
        }

    }
    //above
    else {
        //first void
        for (int i = 0; i < rect_count1; i++, rect_index++) {
            new_track[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track[corner_number].corner[rect_index].x = corner_x;
            new_track[corner_number].corner[rect_index].y = corner_y;
            corner_x += RECT_WIDTH;
            corner_y -= RECT_HEIGHT - RECT_HEIGHT / 2;//heading up
        }
    }
    //second void
    //below
    if (y2 > y1) {

        for (int j = 0; j < rect_count2; j++, rect_index++) {
            new_track[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track[corner_number].corner[rect_index].x = corner_x;
            new_track[corner_number].corner[rect_index].y = corner_y;
            corner_x += RECT_WIDTH;
            corner_y += RECT_HEIGHT - RECT_HEIGHT / 2;
        }
    }
    //above
    else {
        for (int j = 0; j < rect_count2; j++, rect_index++) {
            new_track[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track[corner_number].corner[rect_index].x = corner_x;
            new_track[corner_number].corner[rect_index].y = corner_y;
            corner_x += RECT_WIDTH;
            corner_y -= RECT_HEIGHT - RECT_HEIGHT / 2;
        }

    }
    //third void
    //below
    if (y2 > center_line_y) {
        for (int k = 0; k < rect_count3; k++, rect_index++) {
            new_track[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track[corner_number].corner[rect_index].x = corner_x;
            new_track[corner_number].corner[rect_index].y = corner_y;
            corner_x += RECT_WIDTH;
            corner_y -= RECT_HEIGHT - RECT_HEIGHT / 2;//heading up
        }

    }
    //above
    else {
        for (int k = 0; k < rect_count3; k++, rect_index++) {
            new_track[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track[corner_number].corner[rect_index].x = corner_x;
            new_track[corner_number].corner[rect_index].y = corner_y;
            corner_x += RECT_WIDTH;
            corner_y += RECT_HEIGHT - RECT_HEIGHT / 2;//heading back down
        }
    }
    track_one_rc = rect_count1 + rect_count2 + rect_count3;

    return  0;

}
int create_corner2(int corner_number) {

    int roll;//random roll for above or below the center for the curve
    float y1;
    float y2;
    roll = rand() % 6;
    int rect_count1;
    int rect_count2;
    int rect_count3;
    float y1_center_diff;
    float y2_y1_diff;
    float y2_center_diff;
    //corner x globally defined
    float corner_y;
    corner_y = center_line_y2;
    int rect_index = 0;
    //below center line
    if (roll <= 3) {
        y1 = center_line_y2 + 50 + rand() % TRACK_WIDTH_MOD;
        y2 = y1 + 50 + rand() % TRACK_WIDTH_MOD;
    }
    //above center line
    else {
        y1 = center_line_y2 - 50 - rand() % TRACK_WIDTH_MOD;
        y2 = y1 - 50 - rand() % TRACK_WIDTH_MOD;
    }
    y1_center_diff = y1 > center_line_y2 ? y1 - center_line_y2 : center_line_y2 - y1;
    y2_y1_diff = y2 > y1 ? y2 - y1 : y1 - y2;
    y2_center_diff = y2 > center_line_y2 ? y2 - center_line_y2 : center_line_y2 - y2;
    rect_count1 = ceil(y1_center_diff / RECT_HEIGHT);
    rect_count2 = ceil(y2_y1_diff / RECT_HEIGHT);
    rect_count3 = ceil(y2_center_diff / RECT_HEIGHT);

    //below
    if (y1 > center_line_y2) {

        //first void
        for (int i = 0; i < rect_count1; i++, rect_index++) {
            new_track2[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track2[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track2[corner_number].corner[rect_index].x = corner_x2;
            new_track2[corner_number].corner[rect_index].y = corner_y;
            corner_x2 += RECT_WIDTH;
            corner_y += RECT_HEIGHT - RECT_HEIGHT / 2;//heading down
        }

    }
    //above
    else {
        //first void
        for (int i = 0; i < rect_count1; i++, rect_index++) {
            new_track2[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track2[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track2[corner_number].corner[rect_index].x = corner_x2;
            new_track2[corner_number].corner[rect_index].y = corner_y;
            corner_x2 += RECT_WIDTH;
            corner_y -= RECT_HEIGHT - RECT_HEIGHT / 2;//heading up
        }
    }
    //second void
    //below
    if (y2 > y1) {

        for (int j = 0; j < rect_count2; j++, rect_index++) {
            new_track2[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track2[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track2[corner_number].corner[rect_index].x = corner_x2;
            new_track2[corner_number].corner[rect_index].y = corner_y;
            corner_x2 += RECT_WIDTH;
            corner_y += RECT_HEIGHT - RECT_HEIGHT / 2;
        }
    }
    //above
    else {
        for (int j = 0; j < rect_count2; j++, rect_index++) {
            new_track2[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track2[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track2[corner_number].corner[rect_index].x = corner_x2;
            new_track2[corner_number].corner[rect_index].y = corner_y;
            corner_x2 += RECT_WIDTH;
            corner_y -= RECT_HEIGHT - RECT_HEIGHT / 2;
        }

    }
    //third void
    //below
    if (y2 > center_line_y2) {
        for (int k = 0; k < rect_count3; k++, rect_index++) {
            new_track2[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track2[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track2[corner_number].corner[rect_index].x = corner_x2;
            new_track2[corner_number].corner[rect_index].y = corner_y;
            corner_x2 += RECT_WIDTH;
            corner_y -= RECT_HEIGHT - RECT_HEIGHT / 2;//heading up
        }

    }
    //above
    else {
        for (int k = 0; k < rect_count3; k++, rect_index++) {
            new_track2[corner_number].corner[rect_index].w = RECT_WIDTH;
            new_track2[corner_number].corner[rect_index].h = RECT_HEIGHT;
            new_track2[corner_number].corner[rect_index].x = corner_x2;
            new_track2[corner_number].corner[rect_index].y = corner_y;
            corner_x2 += RECT_WIDTH;
            corner_y += RECT_HEIGHT - RECT_HEIGHT / 2;//heading back down
        }
    }

    track_two_rc = rect_count1 + rect_count2+ rect_count3;
    return  0;

}
void render_rect(SDL_Renderer* renderer, SDL_FRect rect, SDL_Color color) {
    if (renderer)
    {
        if (SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a) != true) {
            SDL_Log("Error setting render draw color: %s", SDL_GetError());
        }
        if (SDL_RenderFillRect(renderer, &rect) != true) {
            SDL_Log("Error filling rect: %s", SDL_GetError());
        }
    }
}

void render_track() {
    for (int i = 0; i < MAX_CORNERS; i++) {
        for (int j = 0; j < MAX_RECTS; j++) {
            //culling and drawing
            if (new_track[i].corner[j].x< WINDOW_WIDTH && new_track[i].corner[j].x >-500) {
                render_rect(renderer, new_track[i].corner[j], white);

            }
            if (new_track2[i].corner[j].x < WINDOW_WIDTH && new_track2[i].corner[j].x >-500) {
                render_rect(renderer, new_track2[i].corner[j], white);

            }



        }
    }
}

void scroll_track() {
    //scrolls the track and everything on it
    if (now - last_time >= frame_delay) {
        for (int i = 0; i < MAX_CORNERS; i++) {
            for (int j = 0; j < MAX_RECTS; j++) {
                new_track[i].corner[j].x -= track_velocity;
                new_track2[i].corner[j].x -= track_velocity;
            }
        }
        for (int k = 0; k < MAX_ITEMS; k++) {
            track_items.items[k].x -= track_velocity;
        }
        for (int l = 0; l < MAX_MONS; l++) {
            monsters[l].rect.x -= track_velocity;
        }

        projectile.rects.x -= track_velocity;
        player_proj.rects.x -= track_velocity;
        ship_top_proj.rects.x -= track_velocity;
        ship_bottom_proj.rects.x -= track_velocity;

        last_time = SDL_GetTicks();
    }
}

void move_player() {
    //not else if if diagonal movement wanted
    if (up_pressed) {
        player_rect.y -= player_velocity;
        ship_mon_left.y -= player_velocity;
        ship_mon_right.y -= player_velocity;
        player_mons[0].rect.y -= player_velocity;
        player_mons[1].rect.y -= player_velocity;
    }
    if (down_pressed) {
        player_rect.y += player_velocity;
        ship_mon_left.y += player_velocity;
        ship_mon_right.y += player_velocity;
        player_mons[0].rect.y += player_velocity;
        player_mons[1].rect.y += player_velocity;
    }
    if (right_pressed) {
        player_rect.x += player_velocity;
        ship_mon_left.x += player_velocity;
        ship_mon_right.x += player_velocity;
        player_mons[0].rect.x += player_velocity;
        player_mons[1].rect.x += player_velocity;
    }
    if (left_pressed) {
        player_rect.x -= player_velocity;
        ship_mon_left.x -= player_velocity;
        ship_mon_right.x -= player_velocity;
        player_mons[0].rect.x -= player_velocity;
        player_mons[1].rect.x -= player_velocity;
    }
}
int check_rect_overlap(SDL_FRect* a, SDL_FRect* one, SDL_FRect* two) {
    if (a->x > WINDOW_WIDTH && a->x < -500) {
        return 0;
    }
    if (two != NULL) {

        if (SDL_HasRectIntersectionFloat(a, two)) {
            return 1;
        }
    }
    if (SDL_HasRectIntersectionFloat(a, one)) {
        return 1;


    }

    return 0;
}

void accelerate() {
    if (space_pressed) {
        if (!acc_started) {
            acc_start_time = SDL_GetTicks();
            acc_started = true;
        }
        else {
            acc_elapsed = SDL_GetTicks() - acc_start_time;
        }
    }
    else {

        if (track_velocity > 10.0) {
            if (!deacc_started) {
                deacc_start_time = SDL_GetTicks();
                deacc_started = true;
            }
            else {
                deacc_elapsed = SDL_GetTicks() - deacc_start_time;
            }
        }

    }
    if (acc_elapsed >= 100) {
        if (track_velocity <= max_track_vel) {
            track_velocity += 5;
        }
        acc_start_time = SDL_GetTicks();
    }
    else if (deacc_elapsed >= 100) {
        if (!space_pressed) {
            if (track_velocity > 10.0) {
                deacc_start_time = SDL_GetTicks();
                track_velocity -= 3.5;//modify with ternary if there is a brake button/flag

            }
            else {

                track_velocity = 10.0;
            }
        }
    }

}

float get_track_min() {
    float min_y;
    min_y = new_track2[0].corner[0].y;
    for (int i = 0; i < MAX_CORNERS; i++) {
        for (int j = 0; j < MAX_RECTS; j++) {

            if (new_track2[i].corner[j].y < min_y) {
                min_y = new_track2[i].corner[j].y;
            }
        }
    }
    return min_y;

}
float get_track_max() {
    float max_y;
    max_y = new_track[0].corner[0].y;
    for (int i = 0; i < MAX_CORNERS; i++) {
        for (int j = 0; j < MAX_RECTS; j++) {

            if (new_track[i].corner[j].y > max_y) {
                max_y = new_track[i].corner[j].y;
            }
        }
    }
    return max_y;

}

void handle_colls() {
    if ((coll_top) || player_rect.y < track_min + RECT_HEIGHT  ) {
        if (!coll_started_t)
        {
            coll_start_time_t = SDL_GetTicks();
            coll_started_t = true;
        }
        if (coll_started_t) {
            player_rect.y += player_velocity * 1.5;


            coll_elapsed_t = SDL_GetTicks() - coll_start_time_t;
        }
        if (coll_elapsed_t > coll_tb_delay) {
            coll_started_t = false;
            coll_top = false;
        }

    }
    if ((coll_top) || ship_mon_left.y < track_min + RECT_HEIGHT) {
        if (!coll_started_t)
        {
            coll_start_time_t = SDL_GetTicks();
            coll_started_t = true;
        }
        if (coll_started_t) {

            ship_mon_left.y += player_velocity * 1.5;
            player_mons[0].rect.y += player_velocity * 1.5;
            coll_elapsed_t = SDL_GetTicks() - coll_start_time_t;
            player_mons[0].HP -= 1;
            char mon0_hp[40];
            snprintf(mon0_hp, sizeof(mon0_hp), "%d", player_mons[0].HP);
            render_text(ship_mon_left.x, ship_mon_left.y - 50, white, mon0_hp, font);
        }
        if (coll_elapsed_t > coll_tb_delay) {
            coll_started_t = false;
            coll_top = false;
        }

    }
    if ((coll_bottom) || player_rect.y + RECT_HEIGHT > track_max   ) {
        if (!coll_started_b)
        {
            coll_start_time_b = SDL_GetTicks();
            coll_started_b = true;
        }
        if (coll_started_b) {
            player_rect.y -= player_velocity * 1.5;

  
            coll_elapsed_b = SDL_GetTicks() - coll_start_time_b;
            player_mons[1].HP -= 1;
            char mon1_hp[40];
            snprintf(mon1_hp, sizeof(mon1_hp), "%d", player_mons[1].HP);
            render_text(ship_mon_right.x, ship_mon_right.y - 50, white, mon1_hp, font);
        }
        if (coll_elapsed_b > coll_tb_delay) {
            coll_started_b = false;
            coll_bottom = false;
        }

    }
    if ((coll_bottom) || ship_mon_right.y + RECT_HEIGHT > track_max) {
        if (!coll_started_b)
        {
            coll_start_time_b = SDL_GetTicks();
            coll_started_b = true;
        }
        if (coll_started_b) {

            ship_mon_right.y -= player_velocity * 1.5;
            player_mons[1].rect.y -= player_velocity * 1.5;
            coll_elapsed_b = SDL_GetTicks() - coll_start_time_b;
        }
        if (coll_elapsed_b > coll_tb_delay) {
            coll_started_b = false;
            coll_bottom = false;
        }

    }
}

SDL_FRect gen_rand_item() {
    SDL_FRect item_rect;
    int rand_rect_ind;
    int rand_corner_ind;
    rand_corner_ind = rand() % MAX_CORNERS;
    rand_rect_ind = rand() % 8;
    item_rect.x = new_track[rand_corner_ind].corner[rand_rect_ind].x;
    item_rect.y = new_track[rand_corner_ind].corner[rand_rect_ind].y - RECT_HEIGHT;
    item_rect.w = ITEM_WIDTH;
    item_rect.h = ITEM_HEIGHT;

    return item_rect;

}
void populate_track_items() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        track_items.items[i] = gen_rand_item();
    }
}
void draw_track_items() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        render_rect(renderer, track_items.items[i], white);
    }
}

SDL_FRect gen_rand_mon_locations() {
    SDL_FRect rect;
    int rand_rect_ind;
    int rand_corner_ind;
    rand_corner_ind = rand() % MAX_CORNERS;
    rand_rect_ind = rand() % 8;

    rect.w = MON_WIDTH;
    rect.h = MON_HEIGHT;
    rect.x = new_track[rand_corner_ind].corner[rand_rect_ind].x;
    rect.y = new_track[rand_corner_ind].corner[rand_rect_ind].y - MON_HEIGHT;

    return rect;

}

void populate_mons() {
    for (int i = 0; i < MAX_MONS; i++) {
        monsters[i].rect = gen_rand_mon_locations();
        SDL_Log("Monster %d at (%f, %f)", i, monsters[i].rect.x, monsters[i].rect.y);
    }
}
void draw_mons() {
    for (int i = 0; i < MAX_MONS; i++) {
        if (monsters[i].rect.x>WINDOW_WIDTH && !monsters[i].rect.x<-500){
            continue;//to save CPU
        }
        else if (monsters[i].rect.x<WINDOW_WIDTH && monsters[i].rect.x>-500) {
            render_rect(renderer, monsters[i].rect, red);
            render_text(monsters[i].rect.x, monsters[i].rect.y - 100, white, monsters[i].name, font);
        }
    }
}

void render_text(float x, float y, SDL_Color color, const char* text, TTF_Font* font)
{
    if (!text || !font || !renderer) {
        SDL_Log("Invalid parameter passed to render_text.");
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(font, text, 0, color, (12 * 60)); // 20 60px wide chars
    if (!surface) {
        //SDL_Log("Unable to create text surface: %s", SDL_GetError()); //muting this warning because it always triggers
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Unable to create texture from surface: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return;
    }

    // Important: set blendmode to blend
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    const SDL_FRect dest = { x, y, surface->w, surface->h }; //make sure this is const SDLFRect or no text will render
    SDL_RenderTexture(renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);

}

void load_projectile() {


    projectile.rects.w = PROJECTILE_WIDTH;
    projectile.rects.h = PROJECTILE_HEIGHT;
    projectile.rects.x = monsters[curr_mon_on_screen()].rect.x;
    projectile.rects.y = monsters[curr_mon_on_screen()].rect.y;



    projectiles_loaded = true;



}
void load_player_proj() {
    player_proj.rects.w = PROJECTILE_WIDTH;
    player_proj.rects.h = PROJECTILE_HEIGHT;
    player_proj.rects.x = player_rect.x;
    player_proj.rects.y = player_rect.y + (PROJECTILE_HEIGHT / 2);
    ship_top_proj.rects.w = PROJECTILE_WIDTH;
    ship_top_proj.rects.h = PROJECTILE_HEIGHT;
    ship_top_proj.rects.x = ship_mon_left.x;
    ship_top_proj.rects.y = ship_mon_left.y + (PROJECTILE_HEIGHT / 2);
    ship_bottom_proj.rects.w = PROJECTILE_WIDTH;
    ship_bottom_proj.rects.h = PROJECTILE_HEIGHT;
    ship_bottom_proj.rects.x = ship_mon_right.x;
    ship_bottom_proj.rects.y = ship_mon_right.y + (PROJECTILE_HEIGHT / 2);
    snprintf(ship_top_proj.type, sizeof(ship_top_proj.type), player_mons[0].proj_type);
    snprintf(ship_bottom_proj.type, sizeof(ship_bottom_proj.type), player_mons[1].proj_type);
    player_loaded = true;
}
int accel_projectile() {




    if (projectile.rects.x<WINDOW_WIDTH && projectile.rects.x>-500)
    {
        if (!proj_started) {
            proj_start_time = SDL_GetTicks();
            proj_started = true;
        }
        else {
            if (player_rect.x < projectile.rects.x) {
                projectile.rects.x -= projectile_velocity;


            }
            else if (player_rect.x > projectile.rects.x) {
                projectile.rects.x += projectile_velocity;


            }
            projectile.rects.y -= projectile_velocity;
            render_rect(renderer, projectile.rects, red);
        }
        proj_elapsed = SDL_GetTicks() - proj_start_time;
        if (proj_elapsed > PROJECTILE_DELAY) {
            projectiles_loaded = false;

            proj_start_time = SDL_GetTicks();
            proj_started = false;
            proj_elapsed = 0;
        }
    }



    if (monsters[curr_mon_on_screen()].rect.x < -100) {
        projectiles_loaded = false;
    }

    return 1;

}

void accel_player_proj() {
    if (proj_pressed) {
        if (!player_proj_started) {
            player_proj_start_time = SDL_GetTicks();
            player_proj_started = true;
        }
        else {
            if (mouse_in_valid_zone) {
                //check for proj type here TODO MORE PROJECTILE TYPES AND STRCMP CONDITION CHECKS HERE
                if(strcmp(player_proj.type , "sine") ==0){
                    player_proj.rects.y += sin(sqrt(player_proj.rects.x)) * track_velocity;
                }
                else if(strcmp(player_proj.type, "saw")==0){

                    player_proj.rects.y  = saw_sign == -1? player_proj.rects.y - track_velocity  : player_proj.rects.y + track_velocity ;
   
                }
                //top and bottom (mon) projs

                if (strcmp(ship_top_proj.type, "sine") == 0) {
                    ship_top_proj.rects.y += sin(sqrt(ship_top_proj.rects.x)) * track_velocity;
                }
                else if (strcmp(ship_top_proj.type, "saw") == 0) {

                    ship_top_proj.rects.y = saw_sign == -1 ? ship_top_proj.rects.y - track_velocity : ship_top_proj.rects.y + track_velocity;

                }

                if (strcmp(ship_bottom_proj.type, "sine") == 0) {
                    ship_bottom_proj.rects.y += sin(sqrt(ship_bottom_proj.rects.x)) * track_velocity;
                }
                else if (strcmp(ship_bottom_proj.type, "saw") == 0) {

                    ship_bottom_proj.rects.y = saw_sign == -1 ? ship_bottom_proj.rects.y - track_velocity : ship_bottom_proj.rects.y + track_velocity;

                }

                //mouse above
                if (mouse_y < player_rect.y) {
                    
                    player_proj.rects.x += projectile_velocity + track_velocity;
                    player_proj.rects.y -= player_mouse_gap;
                    ship_top_proj.rects.x += projectile_velocity + track_velocity;
                    ship_top_proj.rects.y -= player_mouse_gap;
                    ship_bottom_proj.rects.x += projectile_velocity + track_velocity;
                    ship_bottom_proj.rects.y -= player_mouse_gap;
                }
                //mouse_below
                else if (mouse_y > player_rect.y) {
                    player_proj.rects.x += projectile_velocity + track_velocity;
                    player_proj.rects.y += player_mouse_gap;
                    ship_top_proj.rects.x += projectile_velocity + track_velocity;
                    ship_top_proj.rects.y += player_mouse_gap;
                    ship_bottom_proj.rects.x += projectile_velocity + track_velocity;
                    ship_bottom_proj.rects.y += player_mouse_gap;
                }
            }
        }
        if (player_proj.rects.x< WINDOW_WIDTH && player_proj.rects.x > -500){
            render_rect(renderer, player_proj.rects, blue);
            render_rect(renderer, ship_top_proj.rects, blue);
            render_rect(renderer, ship_bottom_proj.rects, blue);
        }
        player_proj_elapsed = SDL_GetTicks() - player_proj_start_time;
        if (player_proj_elapsed > PLAYER_PROJ_DELAY) {
            player_loaded = false;
            player_proj_start_time = SDL_GetTicks();
            //the shot frequency ends each period here, variables can be reset or changed
            saw_sign *= -1;
            player_proj_started = false;
            player_proj_elapsed = 0;
        }
    }
}

int curr_mon_on_screen() {
    int mon_ind = 0;
    for (int k = 0; k < MAX_MONS; k++)
    {

       if (monsters[k].rect.x< WINDOW_WIDTH && monsters[k].rect.x > -500) {
            mon_ind = k;
            if (monsters[mon_ind].rect.x < -100) {
                projectiles_loaded = false;

            }
        }
    }
    return mon_ind;
}
void handle_proj_colls() {
    //deciding damage frequency
    if (player_hit_enemy == true) {
        if (!enemy_hp_started) {
            enemy_hp_start_time = SDL_GetTicks();
            enemy_hp_started = true;
        }
        SDL_Log("player hit enemy");
        if (!proj_coll_started) {
            proj_coll_start = SDL_GetTicks();
            proj_coll_started = true;
        }
        proj_coll_elapsed = SDL_GetTicks() - proj_coll_start;
        if (proj_coll_elapsed > HEALTH_LOSS_DELAY) {
            SDL_Log("enemy health lost");
            monsters[curr_mon_on_screen()].HP -= 2;//take away health
            monsters[curr_mon_on_screen()].encountered = true;//set encountered to true so that it can be added to the battle
            proj_coll_start = SDL_GetTicks();
            proj_coll_started = false;
            proj_coll_elapsed = 0;
        }

    }
    if (enemy_hit_player == true) {
        if (!player_hp_started) {
            player_hp_start_time = SDL_GetTicks();
            player_hp_started = true;
        }
        SDL_Log("enemy hit player");
        if (!proj_coll_started_en) {
            proj_coll_start_en = SDL_GetTicks();
            proj_coll_started_en = true;
        }
        proj_coll_elapsed_en = SDL_GetTicks() - proj_coll_start_en;
        if (proj_coll_elapsed_en > HEALTH_LOSS_DELAY) {
            SDL_Log("player health lost");
            player_hp -= 2;
            proj_coll_start_en = SDL_GetTicks();
            proj_coll_started_en = false;
            proj_coll_elapsed_en = 0;
        }


    }
    if (player_hit_other_enemy == true && get_other_on_screen()!=-1) {//function return check to potentially fix rare WILD POINTER to &monsters[get_other_on_screen()]
        if (!other_enemy_hp_started) {
            enemy_hp_start_time = SDL_GetTicks();
            other_enemy_hp_started = true;
        }
        SDL_Log("player hit enemy");
        if (!proj_coll_started) {
            proj_coll_start = SDL_GetTicks();
            proj_coll_started = true;
        }
        proj_coll_elapsed = SDL_GetTicks() - proj_coll_start;
        if (proj_coll_elapsed > HEALTH_LOSS_DELAY) {
            SDL_Log("enemy health lost");
            monsters[get_other_on_screen()].HP -= 2;//take away health
            monsters[get_other_on_screen()].encountered = true;//set encountered to true so that it can be added to the battle
            proj_coll_start = SDL_GetTicks();
            proj_coll_started = false;
            proj_coll_elapsed = 0;
        }

    }
    //displaying HP numbers
    if (enemy_hp_started) {

        snprintf(mon_hp_buff, sizeof(mon_hp_buff), "%d", monsters[curr_mon_on_screen()].HP);
        render_text(monsters[curr_mon_on_screen()].rect.x, monsters[curr_mon_on_screen()].rect.y, white, mon_hp_buff, font);
        enemy_hp_elapsed = SDL_GetTicks() - enemy_hp_start_time;
    }
    else if(other_enemy_hp_started && monsters[get_other_on_screen()].rect.x >0){//need the and statement or 0 draws in upper left of render present when no other on screen
        snprintf(mon_hp_buff, sizeof(mon_hp_buff), "%d", monsters[get_other_on_screen()].HP);
        render_text(monsters[get_other_on_screen()].rect.x, monsters[get_other_on_screen()].rect.y, white, mon_hp_buff, font);
        enemy_hp_elapsed = SDL_GetTicks() - enemy_hp_start_time;
    }
    if (enemy_hp_elapsed > enemy_hp_delay) {
        enemy_hp_started = false;
        enemy_hp_start_time = SDL_GetTicks();
        enemy_hp_elapsed = 0;
    }

    if (player_hp_started) {
        snprintf(player_hp_buff, sizeof(player_hp_buff), "%d", player_hp);
        render_text(player_rect.x, player_rect.y - 40, white, player_hp_buff, font);
        player_hp_elapsed = SDL_GetTicks() - player_hp_start_time;
    }
    if (player_hp_elapsed > player_hp_delay) {
        player_hp_started = false;
        player_hp_start_time = SDL_GetTicks();
        player_hp_elapsed = 0;
    }

}

void check_mouse() {
    if (mouse_x < player_rect.x) {
        mouse_in_valid_zone = false;
    }
    else {
        mouse_in_valid_zone = true;
        player_mouse_gap = abs(player_rect.y - mouse_y) / 100;

    }
}

int get_last_rect(){
    
    if(track_one_rc < track_two_rc){
           
        return 1;
    }
    else if (track_two_rc < track_one_rc) {

        return 2;
    }

    return 0;
}

int check_player_at_end(){

    if ( player_rect.x > new_track[MAX_CORNERS - 1].corner[track_one_rc - 1].x) {
        return 1;
    }
    
    if ( player_rect.x > new_track2[MAX_CORNERS - 1].corner[track_two_rc - 1].x) {
        return 1;
    }
  
    return 0;
}
int get_other_on_screen() {
    for (int i = 0; i < MAX_MONS; i++) {
        if (monsters[i].rect.x< WINDOW_WIDTH && monsters[i].rect.x > -500 && i != curr_mon_on_screen()) {
            return i;
        }
    }
    return-1;

}
//abstracted away and moved from events to app iterate
void collision_events() {
    for (int i = 0; i < MAX_CORNERS; i++) {
        for (int j = 0; j < MAX_RECTS; j++) {
            {
                if (abs(new_track[i].corner[j].x - ship_mon_right.x) <= RECT_WIDTH  && abs(new_track[i].corner[j].y - ship_mon_right.y) <= RECT_HEIGHT ) {
                    if (check_rect_overlap(&ship_mon_right, &new_track[i].corner[j], NULL) == 1) {
                        track_coll_count++;
                        coll_bottom = true;
                        //SDL_Log("Collision bottom %d", track_coll_count);
                        if (ship_mon_right.y - ship_mon_right.h > new_track[i].corner[j].y)
                        {
                            coll_bottom = true;
                        }
                    }

                }
                else if (abs(new_track2[i].corner[j].x - ship_mon_left.x) <= RECT_WIDTH  && abs((new_track2[i].corner[j].y+RECT_HEIGHT) - ship_mon_left.y) <= RECT_HEIGHT ) {
                    if (check_rect_overlap(&ship_mon_left, &new_track2[i].corner[j], NULL) == 1) {
                        track_coll_count++;
                        coll_top = true;
                        //SDL_Log("Collision top %d", track_coll_count);
                        if (ship_mon_left.y < new_track2[i].corner[j].y + ship_mon_left.h)
                        {
                            coll_top = true;
                        }
                    }

                }

            }
        }

    }
}
void load_ship_mons() {

    for (int i = 0; i < 2; i++) {
        if (i % 2 != 1) {
            player_mons[i].rect.x = ship_mon_left.x;
            player_mons[i].rect.y = ship_mon_left.y;
            player_mons[i].rect.w = ship_mon_left.w;
            player_mons[i].rect.h = ship_mon_left.h;
        }
        else {
            player_mons[i].rect.x = ship_mon_right.x;
            player_mons[i].rect.y = ship_mon_right.y;
            player_mons[i].rect.w = ship_mon_left.w;
            player_mons[i].rect.h = ship_mon_left.h;

        }
    }
}
void check_hit_events() {
    if (check_rect_overlap(&player_proj.rects, &monsters[curr_mon_on_screen()].rect, &monsters[curr_mon_on_screen()].rect) == 1) {
        player_hit_enemy = true;
    }
    else {
        player_hit_enemy = false;
    }
    if (check_rect_overlap(&player_rect, &projectile.rects, NULL) == 1) {
        enemy_hit_player = true;
    }
    else {
        enemy_hit_player = false;
    }
    if (get_other_on_screen() != -1) {
        if (check_rect_overlap(&player_proj.rects, &monsters[get_other_on_screen()].rect, &monsters[get_other_on_screen()].rect) == 1) {
            player_hit_other_enemy = true;
        }
        else if (check_rect_overlap(&ship_top_proj.rects, &monsters[get_other_on_screen()].rect, &monsters[get_other_on_screen()].rect) == 1) {
            player_hit_other_enemy = true;
        }
        else if (check_rect_overlap(&ship_bottom_proj.rects, &monsters[get_other_on_screen()].rect, &monsters[get_other_on_screen()].rect) == 1) {
            player_hit_other_enemy = true;
        }
        else {
            player_hit_other_enemy = false;
        }
    }
}
void check_item_pickups() {
    for (int k = 0; k < MAX_ITEMS; k++) {
        if (abs(player_rect.x - track_items.items[k].x) < PLAYER_WIDTH * 2 && abs(player_rect.y - track_items.items[k].y) < PLAYER_HEIGHT * 2) {
            if (check_rect_overlap(&player_rect, &track_items.items[k], NULL) == 1) {
                SDL_Log("Item Touched");
            }
        }
    }
}

void repel_around_player(){
    if(abs((ship_mon_left.y +ship_mon_left.h)- ship_mon_right.y) <5 || abs(ship_mon_right.y - (ship_mon_left.y + ship_mon_left.h)) < 5|| player_rect.y + PLAYER_HEIGHT > ship_mon_right.y|| player_rect.y < ship_mon_left.y + PLAYER_HEIGHT){
        if(!repelling){
            repel_start = SDL_GetTicks();
            repelling = true;
        }

    }
    if(repelling){
        repel_elapsed = SDL_GetTicks() - repel_start;
        if(repel_elapsed < repel_delay){
            if (abs((ship_mon_left.y + ship_mon_left.h+20) - ship_mon_right.y) < 5 ){
                ship_mon_left.y -= player_velocity;
                player_mons[0].rect.y -= player_velocity;

            }
            if(abs(ship_mon_right.y - (ship_mon_left.y + ship_mon_left.h - 20)) < 5){
                player_mons[1].rect.y += player_velocity;
                ship_mon_right.y += player_velocity;
            }
            if(player_rect.y + PLAYER_HEIGHT > ship_mon_right.y || player_rect.y  > ship_mon_right.y){
                player_rect.y -= player_velocity*2;
            }
            if (player_rect.y < ship_mon_left.y + PLAYER_HEIGHT || player_rect.y < ship_mon_left.y) {
                player_rect.y += player_velocity*2;
            }
            if(ship_mon_left.y> ship_mon_right.y - PLAYER_HEIGHT){
                ship_mon_left.y -= player_velocity;
                player_mons[0].rect.y -= player_velocity;
                player_mons[1].rect.y += player_velocity;
                ship_mon_right.y += player_velocity;
            }


        }

    }
    if (repel_elapsed > repel_delay) {
        repelling = false;
        repel_elapsed = 0;
    }

}
/*TRACK FUNCTIONS END*/

/*MON BATTLE FUNCTIONS*/
void populate_mon_array(){
    for(int i =0; i< MAX_MONS; i++)
    {
        if(monsters[i].encountered == true){
            battle_array[i] = monsters[i];
        }

    }
    battle_array[MAX_MONS ] = player_mons[0];
    battle_array[MAX_MONS + 1] = player_mons[1];
    battle_array[MAX_MONS].is_in_party = true;
    battle_array[MAX_MONS+1].is_in_party = true;//set the players mons is in party bool here
    for (int j = 0; j <MAX_MONS +2; j++) {
        SDL_Log("intial mbattle array spds: %d", battle_array[j].spd);
    }

}
void render_battle_mons(){
    int count=0;
    for(int i =0; i<dynamic_size; i++){
        if(dynamic_battle_array[i].encountered && !dynamic_battle_array[i].is_in_party){
            dynamic_battle_array[i].rect.x = render_rects.battle_locs[i].x;
            dynamic_battle_array[i].rect.y = render_rects.battle_locs[i].y;
            render_rect(renderer, dynamic_battle_array[i].rect, red);
            render_text(dynamic_battle_array[i].rect.x, dynamic_battle_array[i].rect.y, white, dynamic_battle_array[i].name, font);
        }

    }
    for(int j =0; j< dynamic_size; j++){
        if(dynamic_battle_array[j].is_in_party){

            dynamic_battle_array[j].rect.x = render_rects.battle_locs[MAX_MONS+count].x;
            dynamic_battle_array[j].rect.y = render_rects.battle_locs[MAX_MONS+count].y;
            render_rect(renderer, dynamic_battle_array[j].rect, blue);
            render_text(dynamic_battle_array[j].rect.x, dynamic_battle_array[j].rect.y, white, dynamic_battle_array[j].name, font);
            count++;
        }
    }
}

//set initial player party
void init_player_mons(){
    //arbitrary for now
    player_mons[0] = monsters[0];
    player_mons[1] = monsters[1];
    SDL_Log("Player mon 0,1 speeds %d %d", player_mons[0].spd, player_mons[1].spd);
}

//renders background of battle menu
void render_bm_bg(){
    render_rect(renderer, render_rects.battle_menu[0], white);
}
//render onto the battle menu rects the main info
void render_bm_main(){
    render_text(render_rects.battle_menu[1].x, render_rects.battle_menu[1].y, black, "Fight", font);
    render_text(render_rects.battle_menu[2].x, render_rects.battle_menu[2].y, black, "Item", font);
    render_text(render_rects.battle_menu[3].x, render_rects.battle_menu[3].y, black, "Run", font);
}

//when  in the state where move menu is selected, render the moves on the battle menu rects, based on an index of which of the two player mons turn it is
void render_bm_moves(int mon_ind){
    

    render_text(render_rects.battle_menu[1].x, render_rects.battle_menu[1].y, black, player_mons[mon_ind].moves[0].name, font);
    render_text(render_rects.battle_menu[2].x, render_rects.battle_menu[2].y, black, player_mons[mon_ind].moves[1].name, font);
    render_text(render_rects.battle_menu[3].x, render_rects.battle_menu[3].y, black, player_mons[mon_ind].moves[2].name, font);
    render_text(render_rects.battle_menu[4].x, render_rects.battle_menu[4].y, black, player_mons[mon_ind].moves[3].name, font);
}
//highlight a low alpha rect over the selkected index which icreases or decreases with A and D/L and R (first always render by default)
void render_menu_hl(int ind){
    render_rect(renderer, render_rects.battle_menu[ind], highlight);
}

void handle_menu_press(){
    if(supress_menu_press){
        left_pressed = false;
        right_pressed = false;
        space_pressed = false;
        if(!menu_sup_started){
            menu_sup_start_time = SDL_GetTicks();
            menu_sup_started = true;
        }
        else{
            menu_sup_elapsed = SDL_GetTicks() - menu_sup_start_time;

        }
    }
    if (menu_sup_elapsed >= menu_sup_delay) {
        supress_menu_press = false;
        menu_sup_elapsed = 0;
    }

}

//highlight the selected enemy mon to attack
void render_mon_hl(int ind){
    render_rect(renderer, dynamic_battle_array[ind].rect, highlight);

}

int get_enemy_count(){
    int c = 0;
    for(int i = 0; i< dynamic_size; i++){
        if(dynamic_battle_array[i].encountered==true && !dynamic_battle_array[i].is_in_party){
            c++;
        }
    }
    return c;
}

//hard to wrap head around since some of the mons will be null, so how to create and sort a clean array?
void decide_turn_order(size){


    Monster temp[MAX_MONS+1];
    int i, x;
    bool swapped;
    for(x=0; x<size;x++){
        swapped = false;
        for(i=0; i<size-1-x; i++){

                if(dynamic_battle_array[i].spd < dynamic_battle_array[i+1].spd ){

                    dynamic_battle_array[i].turn += 1;
                    temp[i] = dynamic_battle_array[i];
                    dynamic_battle_array[i] = dynamic_battle_array[i + 1];
                    dynamic_battle_array[i + 1] = temp[i];
                    swapped = true;
                }


            }
        if(!swapped){
            break;
        }
    }
    

    for(int j=0; j<size; j++){
        SDL_Log("%d", dynamic_battle_array[j].spd);
    }

}

int allocate_dynamic_battle_arr(void) {
    // Step 1: Count valid monsters
    int count = 0;
    for (int s = 0; s < MAX_MONS + 2; s++) {
        if (battle_array[s].spd != 0) {
            count++;
        }
    }

    // Step 2: Allocate exact amount
    dynamic_battle_array = malloc(count * sizeof(Monster));
    if (!dynamic_battle_array) {
        SDL_Log( "malloc failed\n");
        return SDL_APP_FAILURE;
    }

    // Step 3: Copy only the valid entries
    int j = 0; // write index
    for (int i = 0; i < MAX_MONS + 2; i++) {
        if (battle_array[i].spd != 0) {
            dynamic_battle_array[j] = battle_array[i];
            j++;
        }
    }
    SDL_Log("Succ");
    return j;


}

int apply_attacks(int i, Monster *target, Monster *attacker){
    
    
    

    //subtract from the HP of the attack index mon the move index of moves of the current monster
    if(! hp_subbed){
        target->HP -= attacker->moves[attacker->mov_ind].power;
        hp_subbed = true;
    }
    SDL_Log("%s HP reduced by %s attack ", target->name, attacker->name);
    if(!hp_buff_filled){
        snprintf(battle_hp_buff, sizeof(battle_hp_buff), "%d", target->HP);
        hp_buff_filled = true;
    }
    
    if(!battle_hp_started){
        battle_hp_start_time = SDL_GetTicks();
        battle_hp_started = true;
    }
    else{
        render_text(target->rect.x, target->rect.y-200, white, battle_hp_buff, font);
        battle_hp_elapsed = SDL_GetTicks() - battle_hp_start_time;
    }

    if(battle_hp_elapsed > battle_hp_delay){
            
        battle_hp_started = false;

        hp_buff_filled = false;

        return 1;

    }

    return 0;
}

void set_enemy_decisions(Monster *mon){
    int mov_size = 0;
    int mov_ind = 0;//the index of the move they will use
    int attack_ind = 0;//the index of the player mon that it will attack
    int c_1 = 0;
    int c_2 = 0;//two choices indexes of the dynamic battle array of player owned mons
    int roll = 0;//50 50 roll
    mov_size = sizeof(mon->moves) / sizeof(mon->moves[0]);

    mon->mov_ind = rand() % mov_size;

    for(int i =0; i<dynamic_size; i++){
        if(dynamic_battle_array[i].is_in_party){
            c_1 = i;
        }
    }
    for(int j =0; j<dynamic_size; j++){
        if(j!=c_1 && dynamic_battle_array[j].is_in_party){
            c_2 = j;
        }
    }
    roll = rand() % 2;
    if(roll>0){
        mon->att_ind = c_1;
    }
    else{
        mon->att_ind = c_2;
    }
}
bool check_all_defeated(){
    for(int i =0; i<dynamic_size; i++)
    {
        if(!dynamic_battle_array[i].is_in_party && !dynamic_battle_array[i].spd==0){
            if(dynamic_battle_array[i].HP>0){
                return false;
            }
        }
    
        
    }
    return true;
   

 


}
//TODO: CHeck all player mons dead fail condition


void reset_mon_inds(){
    for(int i = 0; i<dynamic_size; i++){
        dynamic_battle_array[i].att_ind = NULL;
        dynamic_battle_array[i].mov_ind = NULL;
    }
}

void reinit_track_vars(){
    mon_arr_populated = false;
    track_generated = false;
    track_populated = false;
    projectiles_loaded = false;
    player_loaded = false;
    player_rect.x = 100.0, player_rect.y = WINDOW_HEIGHT / 2, player_rect.w = PLAYER_WIDTH, player_rect.h = PLAYER_HEIGHT;
    ship_mon_left.x = 100.0, ship_mon_left.y = WINDOW_HEIGHT / 2 - PLAYER_HEIGHT - 20, ship_mon_left.w = PLAYER_WIDTH, ship_mon_left.h = PLAYER_HEIGHT;
    ship_mon_right.x = 100.0, ship_mon_right.y = WINDOW_HEIGHT / 2 + PLAYER_HEIGHT + 20, ship_mon_right.w = PLAYER_WIDTH, ship_mon_right.h = PLAYER_HEIGHT;
    center_line_y = WINDOW_HEIGHT / 2 + 300;
    center_line_y2 = WINDOW_HEIGHT / 2 - 600;
    corner_x = 100.0;
    corner_x2 = 100.0;
    player_mons_init = false;
    for(int i =0; i< MAX_MONS; i ++){
        monsters[i].HP = 100;//need to change this for player mons once out of this test phase
        monsters[i].encountered = false;
        battle_array[i].encountered = false;
        battle_array[i].is_in_party = false;
        battle_array[i].HP = 100;
        battle_array[i].spd = 0;
    }
    free(dynamic_battle_array);
    dynamic_battle_array = NULL;
    dynamic_size = 0;
}

void reinit_battle_vars(){
    hl_ind = 1;
    turns_decided = false;
    enemy_count_gotten = false;
    enemy_decisions_set = false;
    attacks_applied = false;
    supress_menu_press = false;
    space_pressed = false;
    right_pressed = false;
    left_pressed = false;
    right_pressed = false;
    up_pressed = false;
    player_mon_ind = 0;
    hp_ind = 0;
    reset_mon_inds();
}
//replace highlight index with first enemy index upon enemy monster selection state in battle with this funnction
int get_first_enemy_in_dba(){
    for(int i=0; i<dynamic_size; i++){
        if(dynamic_battle_array[i].encountered){
            return i;
        }
    }
    return -1;
}

/*MON BATTLE FUNCTIONS END*/

/*TILE FUNCTIONS*/

void init_under_tiles(){
    float x_base = 0.0;
    float y_base = 0.0;
    int row = 0;
    int col = 0;
    int row_mult = 0;
    for(int i=0;i<ALL_ROWS; i++){
        for(int j =0; j<ALL_COLS; j++){
            under_tiles[row_mult+col].rect.x += x_base;
            under_tiles[row_mult+ col].rect.y += y_base;
            under_tiles[row_mult+col].row= row;
            under_tiles[row_mult+ col].col = col;
            SDL_Log("current under tile x %f", under_tiles[row_mult + col].rect.x);
            SDL_Log("current under tile y %f", under_tiles[row_mult + col].rect.y);
            if(j==ALL_COLS-1){
                row_mult += ALL_COLS;
                y_base += TILE_HEIGHT;
                row += 1;
                SDL_Log("init under row %d", row);
            }
            
            x_base = j== ALL_COLS - 1 ? 0.0 : (x_base + TILE_WIDTH)*1.0;
            col = col == ALL_COLS - 1 ? 0.0 : (col + 1) *1.0;
            SDL_Log("init under col %d", col);
        }
    }

}

void init_world_areas(){
    int size=0;

    int col_mult = 0;
    int row_mult=0;
    int row_count=0;
    for(int i=0; i< ALL_AREAS; i++){
        for(int j=0; j< ALL_ROWS * ALL_COLS; j++){
            
            if(under_tiles[j].col == world_areas[i].col && under_tiles[j].row == world_areas[i].row){
                SDL_Log("under tile col - %d , world tile col - %d", under_tiles[j].col, world_areas[i].col);
                size = (world_areas[i].area_h * world_areas[i].area_w);
                
                world_areas[i].size = size;
                SDL_Log("size: %d", size);
                world_areas[i].rects = malloc(size * sizeof(SDL_FRect));//allocate the memory for all of the area tiles
                if(!world_areas[i].rects){
                    SDL_Log("malloc failed\n");
                    return SDL_APP_FAILURE;
                }
                for (int k=0, col_count=0; k < size; k++) {

                    world_areas[i].rects[k].x = under_tiles[j].rect.x+col_mult;// set all the rects in the area to the top left, will be set properly in render world areas
                    world_areas[i].rects[k].y = under_tiles[j].rect.y +row_mult;
                    world_areas[i].rects[k].w = TILE_WIDTH;
                    world_areas[i].rects[k].h = TILE_HEIGHT;
                    world_areas[i].bottom_right = world_areas[i].rects[size - 1];//set the bottom right member
                    SDL_Log("current rect x %f", world_areas[i].rects[k].x);
                    SDL_Log("current rect y %f", world_areas[i].rects[k].y);
                    if(++col_count >= world_areas[i].area_w){
                        if (++row_count >= world_areas[i].area_h) {
                            row_mult = 0;
                            row_count = 0;
                        }
                        row_mult += TILE_HEIGHT;
                        col_mult = 0;
                        col_count = 0;
                    }
                    else{
                        col_mult += TILE_WIDTH;
                    }


                }

            }


        }
    }
}



void render_world_areas(){
    
    
    for(int i=0; i< ALL_AREAS; i++){
        
        curr_area_size = world_areas[i].size;
        //SDL_Log("size: %d", size);
        for(int j=0; j<curr_area_size; j++){
            world_areas[i].is_walkable? render_rect(renderer, world_areas[i].rects[j], blue): render_rect(renderer, world_areas[i].rects[j], red);
            if (!printed) {
                SDL_Log("rendering world rect x %f", world_areas[i].rects[j].x);
                SDL_Log("rendering world rect y %f", world_areas[i].rects[j].y);
                

            }
        }

    }
    printed = true;


   
}




//place areas based on their top lefts and widths and heights (CURRENTLY COMMENTED OUT)
bool place_world_areas(int i, int row, int col) {

    tile_y_offset = world_areas[i].rects[0].y;
    if(col !=0){
        world_areas[i].rects[col].x = TILE_WIDTH * col;
    }
    if (++col > world_areas[i].area_w) {
        tile_y_offset += TILE_HEIGHT * row;
        if (++row > world_areas[i].area_h) {
            return true;
        }
        
        col = 1;//past top left, back to col x1
    }
    world_areas[i].rects[col].y = tile_y_offset;
    

    return false;
}




//GRID MAKER FUNCTIONS
void set_gm_rects(){
    float x = 0.0;
    float y = 0.0;


    for(int i=0; i<MAX_GM; i++){
        gm_rects[i].h = GM_HEIGHT;
        gm_rects[i].w = GM_WIDTH;
        gm_rects[i].x = x;
        gm_rects[i].y = y;
        
        x += GM_WIDTH+GM_MARGIN;
        if(x>= gm_map_w){
            y += GM_HEIGHT+GM_MARGIN;
            x = 0;

        }
        if (y >= gm_map_h) {
            break;
        }
        
    }
}
void render_gm_rects(){

    if(!gm_buffers_set){
        int xi = 0;
        int yi = 0;
        for (int i = 0; i < MAX_GM; i++) {
            gm_buffer[i] = malloc(40 * sizeof(char));
            if (!gm_buffer[i]) {
                SDL_Log("malloc failed\n");
                return SDL_APP_FAILURE;
            }
            snprintf(gm_buffer[i], 40, "%d,%d", xi, yi);

            if (++xi > ALL_COLS-1) {
                yi++;
                xi = 0;
            }
        }
        for (int j = 0; j < MAX_GM; j++)
        {
            create_gm_texts(gm_rects[j].x, gm_rects[j].y, black, gm_buffer[j], gm_font, j);
        }
        gm_buffers_set = true;
    }
    
    
    for (int i = 0; i < MAX_GM; i++) {

        render_rect(renderer, gm_rects[i], white);
        SDL_RenderTexture(renderer, gm_textures[i], NULL, &gm_rects[i]);

    }
    gm_loaded = true;


}
void create_gm_texts(float x, float y, SDL_Color color, const char* text, TTF_Font* font, int i ){

    if (!text || !font || !renderer) {
        SDL_Log("Invalid parameter passed to render_text.");
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Solid_Wrapped(font, text, 0, color, (12 * 60)); // 20 60px wide chars
    if (!surface) {
        SDL_Log("Unable to create text surface: %s", SDL_GetError()); //muting this warning because it always triggers
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Unable to create texture from surface: %s", SDL_GetError());
        SDL_DestroySurface(surface);
        return;
    }

    // Important: set blendmode to blend
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    gm_textures[i] = texture;


    SDL_DestroySurface(surface);
    
    
}
int output_gm_bmp(void) {
    // Render map into target
    if (SDL_SetRenderTarget(renderer, gm_target) != true) {
        SDL_Log("SDL_SetRenderTarget failed: %s", SDL_GetError());
        return 1;
    }


    // Draw everything into the target (same rendering code you use on screen)
    for (int i = 0; i < MAX_GM; i++) {
        render_rect(renderer, gm_rects[i], white);
        SDL_RenderTexture(renderer, gm_textures[i], NULL, &gm_rects[i]);
    }

    // --- IMPORTANT ---
    // Read pixels WHILE target is still bound
    SDL_Surface* surface = SDL_RenderReadPixels(renderer, NULL);
    if (!surface) {
        SDL_Log("SDL_RenderReadPixels failed: %s", SDL_GetError());
        SDL_SetRenderTarget(renderer, NULL);
        return 1;
    }

    // Save BMP
    if (SDL_SaveBMP(surface, "map_output.bmp") != 0) {
        SDL_Log("SDL_SaveBMP failed: %s", SDL_GetError());
    }

    SDL_DestroySurface(surface);

    // Restore default target BEFORE present
    SDL_SetRenderTarget(renderer, NULL);

    SDL_Log("Exported map_output.bmp successfully!");

    return 0;
}


//WORLD MOVEMENT
void move_world() {

        if (up_pressed) {

                if(++queue_ind<2){
                    input_queue[queue_ind].direction = 0;
                    input_queue[queue_ind].timestamp = SDL_GetTicks();
                }
     
                
        }
        if (down_pressed) {
      
                if (++queue_ind < 2) {
                    input_queue[queue_ind].direction = 2;
                    input_queue[queue_ind].timestamp = SDL_GetTicks();
                }

        }
        if (right_pressed) {
      
                if (++queue_ind < 2) {
                    input_queue[queue_ind].direction = 1;
                    input_queue[queue_ind].timestamp = SDL_GetTicks();
                }

        }
        if (left_pressed) {

                if (++queue_ind < 2) {
                    input_queue[queue_ind].direction = 3;
                    input_queue[queue_ind].timestamp = SDL_GetTicks();
                }

                
        }
        else{
            //IDLE ANIM
            walking_src.y = SPRITE_HEIGHT * 4;
        }
        
        if(input_queue[0].timestamp< input_queue[1].timestamp && input_queue[0].direction !=-1){
            
            switch(input_queue[0].direction){
            case 0:
            {
                walking_src.y = 0;
                
                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].y += world_velocity;
                    }
                }
                for(int n=0; n<ALL_CHARACTERS; n++){
                    characters[n].rect.y += world_velocity;

                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.y += world_velocity;
                }
                    
                    
                break;
            }
            case 1:
            {   //change to second row of sprite sheet
                walking_src.y = SPRITE_HEIGHT;
                

                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].x -= world_velocity;
                    }
                }
                for (int n = 0; n < ALL_CHARACTERS; n++) {
                    characters[n].rect.x -= world_velocity;
   
                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.x -= world_velocity;
                }
                break;
            }
            case 2:
            {
                walking_src.y = SPRITE_HEIGHT*2;
                

                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].y -= world_velocity;
                    }
                }
                for (int n = 0; n < ALL_CHARACTERS; n++) {
                    characters[n].rect.y -= world_velocity;
 
                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.y -= world_velocity;
                }
                break;
            }
            case 3:
            {
                walking_src.y = SPRITE_HEIGHT * 3;
                
                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].x += world_velocity;
                    }
                }
                for (int n = 0; n < ALL_CHARACTERS; n++) {
                    characters[n].rect.x += world_velocity;
     
                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.x += world_velocity;
                }
                break;
            }
            }
        }
        
        else{
            switch (input_queue[1].direction) {
            case 0:
            {
                walking_src.y = 0;
                
                
                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].y += world_velocity;
                    }
                }
                for (int n = 0; n < ALL_CHARACTERS; n++) {
                    characters[n].rect.y += world_velocity;
      
                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.y += world_velocity;
                }
                break;
            }
            case 1:
            {
                walking_src.y = SPRITE_HEIGHT ;
                
                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].x -= world_velocity;
                    }
                }
                for (int n = 0; n < ALL_CHARACTERS; n++) {
                    characters[n].rect.x -= world_velocity;
    
                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.x -= world_velocity;
                }
                break;
            }
            case 2:
            {
                walking_src.y = SPRITE_HEIGHT*2;
                
                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].y -= world_velocity;
                    }
                }
                for (int n = 0; n < ALL_CHARACTERS; n++) {
                    characters[n].rect.y -= world_velocity;
 
                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.y -= world_velocity;
                }
                break;
            }
            case 3:
            {
                walking_src.y = SPRITE_HEIGHT *3;
                
                for (int i = 0; i < ALL_AREAS; i++) {
                    for (int j = 0; j < world_areas[i].size; j++) {
                        world_areas[i].rects[j].x += world_velocity;
                    }
                }
                for (int n = 0; n < ALL_CHARACTERS; n++) {
                    characters[n].rect.x += world_velocity;

                }
                for (int m = 0; m < MAX_GOAL_ITEMS; m++) {
                    world_goal_items[m].rect.x += world_velocity;
                }
                break;
            }
            }
        }


}
bool input_queue_full(){

    if (input_queue[1].direction != -1){

        SDL_Log("inp queue full");
        return true;

    }
    else{
        return false;
    }
    
}

void reset_inp_queue(){
    for (int i = 0; i < 2; i++) {
        input_queue[i].timestamp = NULL;
        input_queue[i].direction = -1;
    }
    queue_ind = -1;
}

void init_char_dialogues(){
    //testy tim
    snprintf(characters[1].dialogues[0].buffer, sizeof(characters[1].dialogues[0].buffer), "Hey, hows it goin?");
    snprintf(characters[1].dialogues[1].buffer, sizeof(characters[1].dialogues[1].buffer), "Get me that gold and I will give you the key to unlock that area");
    snprintf(characters[1].dialogues[2].buffer, sizeof(characters[1].dialogues[2].buffer), "Ok good, here's the key");

    snprintf(characters[2].dialogues[0].buffer, sizeof(characters[2].dialogues[0].buffer), "I need help.");
    snprintf(characters[2].dialogues[1].buffer, sizeof(characters[2].dialogues[1].buffer), "Get me that thingy and I will get you the key to unlock the other area.");
    snprintf(characters[2].dialogues[2].buffer, sizeof(characters[2].dialogues[2].buffer), "Ok thanks, here.");
}

void init_character_locs(){
    for(int i=0; i< ALL_ROWS * ALL_COLS; i++){

            for(int j=0; j<ALL_CHARACTERS; j++){
                if(under_tiles[i].col == characters[j].lcol && under_tiles[i].row == characters[j].lrow && (characters[j].lrow !=0 && characters[j].lcol !=0)){
                    characters[j].rect.x = under_tiles[i].rect.x;
                    characters[j].rect.y = under_tiles[i].rect.y;
                    SDL_Log("character %d x = %f y= %f", j, characters[j].rect.x, characters[j].rect.y);
                }
            }

    }
}

void init_key_items(){
    //NULL CHARACTER

    snprintf(characters[1].goal_items[0].name,sizeof(characters[1].goal_items[0].name),  "test key");
    snprintf(characters[1].reward_items[0].name, sizeof(characters[1].reward_items[0].name), "test reward");
    characters[1].goal_items[0].location.row =6;
    characters[1].goal_items[0].location.col =7;
    characters[1].dialogue_stops[0].stop_ind = 1;
    snprintf(characters[1].dialogue_stops[0].id, sizeof(characters[1].dialogue_stops[0].id), "0000");
    snprintf(characters[1].goal_items[0].dialogue_stop.id, sizeof(characters[1].goal_items[0].dialogue_stop.id), "0000");
    characters[1].goal_items[0].ds = 1;
    world_areas[14].key = characters[1].reward_items[0];

    snprintf(characters[2].goal_items[0].name, sizeof(characters[2].goal_items[0].name), "foobar key");
    snprintf(characters[2].reward_items[0].name, sizeof(characters[2].reward_items[0].name), "foobar reward");
    characters[2].goal_items[0].location.row = 5;
    characters[2].goal_items[0].location.col = 5;
    characters[2].dialogue_stops[0].stop_ind = 1;
    snprintf(characters[2].dialogue_stops[0].id, sizeof(characters[2].dialogue_stops[0].id), "0001");
    snprintf(characters[2].goal_items[0].dialogue_stop.id, sizeof(characters[2].goal_items[0].dialogue_stop.id), "0001");
    characters[2].goal_items[0].ds = 1;
    world_areas[15].key = characters[2].reward_items[0];
    

    
        
    int k = 0;
    for (int i = 0; i < ALL_CHARACTERS; i++) {
        for (int j =0; j<ALL_ROWS*ALL_COLS; j++){
            for (int p =0; p< MAX_GOAL_ITEMS; p++){
            
            if((characters[i].goal_items[k].location.row == under_tiles[j].row) && (characters[i].goal_items[p].location.col == under_tiles[j].col) && strcmp(characters[i].goal_items[p].name, "") !=0){
                world_goal_items[k] = characters[i].goal_items[p];
                world_goal_items[k].ds = characters[i].goal_items[p].ds;
                world_goal_items[k].rect.x = under_tiles[j].rect.x;
                world_goal_items[k].rect.y = under_tiles[j].rect.y;
                world_goal_items[k].rect.w = ITEM_WIDTH;
                world_goal_items[k].rect.h = ITEM_HEIGHT;
                SDL_Log("goal item %s placed at x =%f , y=%f ",&world_goal_items[k].name, world_goal_items[k].rect.x, world_goal_items[k].rect.y);
                k++;
                if(strcmp(characters[i].goal_items[p].name, "") == 0){
                    break;
                }
                }
            }
        }
    }
}



void render_characters(){
    for(int i=0; i<ALL_CHARACTERS; i++){

        if((abs(characters[i].rect.x - world_player.rect.x) < WINDOW_WIDTH) && (abs(characters[i].rect.y - world_player.rect.y) < WINDOW_HEIGHT) && (characters[i].lrow !=0 && characters[i].lcol !=0)){

            render_rect(renderer, characters[i].rect, red);
            //SDL_Log("Rendering %s", characters[i].name);
        }
    }
}

//returns index of which character
int handle_character_interact(){

    for (int i=0;  i < ALL_CHARACTERS; i++) {
        if((abs(world_player.rect.x - characters[i].rect.x) < TILE_WIDTH) && (abs(world_player.rect.y - characters[i].rect.y) < TILE_HEIGHT*2)){
 
            if(space_pressed){
                SDL_Log("entering dialogue");
                entered_dia = true;
                dia_ind = 0;
                    
                
            }

            return i;
        }
    }

        
    
    dia_ind = -1;
    return 0;

    
}
//takes the characters index returned from handle_character_interact as a param
void progress_character_dia(){
    
    if(entered_dia && interact_char_ind!=0){
        SDL_Log("Dia ind- %d", dia_ind);
        render_text(characters[interact_char_ind].rect.x, characters[interact_char_ind].rect.y, white, characters[interact_char_ind].dialogues[dia_ind].buffer, font);

        int ds_ind = characters[interact_char_ind].ds_ind;
        SDL_Log("ds_ind : %d", ds_ind);
        
        if(dia_ind != characters[interact_char_ind].dialogue_stops[ds_ind].stop_ind && characters[interact_char_ind].dialogue_stops[ds_ind].passed!=true && dia_ind !=-1){
            if (shift_pressed) {
                dia_ind++;//this seems to be triggering
                SDL_Log("Dia ind increased- %d", dia_ind);
                shift_pressed = false;
            }
        }
        else if( dia_ind >-1 && dia_ind == characters[interact_char_ind].dialogue_stops[ds_ind].stop_ind){
            SDL_Log("entering dialogue check condition 0");
            for(int k =0; k< MAX_GOAL_ITEMS; k++){

                    if( ( strcmp(characters[interact_char_ind].dialogue_stops[ds_ind].id, world_player.key_items[k].dialogue_stop.id) ==0)){
                        SDL_Log("entering dialogue check condition 1");
                        characters[interact_char_ind].dialogue_stops[ds_ind].passed = true;
                            
                        if (shift_pressed) {
                            int x = get_next_empty_keyitem();
                                 
                            world_player.key_items[x] = characters[interact_char_ind].reward_items[ds_ind];
                            world_player.key_items[x].obtained = true;
                            if(characters[interact_char_ind].dialogue_stops[ds_ind+1].stop_ind!=0){
                                characters[interact_char_ind].ds_ind += 1;
                            }
                            dia_ind++;
                                
                            shift_pressed = false;
                        }
                    }
                }
            }
        }
    
    


    
}

void handle_key_item_pickup(){

    for (int i = 0; i < ALL_KEYITEMS; i++) {
        
        if(strcmp(world_goal_items[i].name, "") !=0){
        if ((abs(world_player.rect.x - world_goal_items[i].rect.x) < PICKUP_RADIUS) && (abs(world_player.rect.y - world_goal_items[i].rect.y) < PICKUP_RADIUS)){

            if (!world_goal_items[i].obtained) {
                int x = get_next_empty_keyitem();

                world_player.key_items[x] = world_goal_items[i];
                SDL_Log("item obtained - %s", world_goal_items[i].name);
                world_player.key_items[x].obtained = true;
                world_goal_items[i].obtained = true;
                break;

            }
        }
        }

    }


    
}
//find the next free slot in the players key items
int get_next_empty_keyitem(){
    for(int i=0; i<ALL_KEYITEMS; i++){
        if(!world_player.key_items[i].obtained){
            return i;
        }

        
    }
    return -1;



}
void render_key_items(){

    for (int k = 0; k < ALL_KEYITEMS; k++) {
        //culling isnt right here so commented out
        //if ((abs(world_goal_items[k].rect.x - world_player.rect.x) < WINDOW_WIDTH) && (abs(world_goal_items[k].rect.y - world_player.rect.y) < WINDOW_HEIGHT) && (world_goal_items[k].location.row !=0 && world_goal_items[k].location.col != 0)){
            render_rect(renderer, world_goal_items[k].rect, red);
            //SDL_Log("Rendering item");
        //}
    }
    
}

void collisions_world(){
    for(int i =0; i<ALL_AREAS; i++){
        if(!world_areas[i].is_walkable){

            if(((world_player.rect.y + TILE_HEIGHT)< world_areas[i].bottom_right.y) && ((world_player.rect.x + TILE_WIDTH/2) < world_areas[i].bottom_right.x) && ((world_player.rect.x + TILE_WIDTH / 2)  > world_areas[i].rects[0].x && (world_player.rect.y + TILE_HEIGHT) > world_areas[i].rects[0].y)) {

                for (int k = 0; k < world_areas[i].size; k++) {
                    if(abs(world_areas[i].rects[k].x - (world_player.rect.x+TILE_WIDTH)) < TILE_WIDTH && abs(world_areas[i].rects[k].y - (world_player.rect.y+TILE_HEIGHT)) < TILE_HEIGHT){
                        world_player_coll = true;
                    }

                }
                    
                
 
                  
            }
        }

    }
    if (world_player_coll){
        if (!world_vel_inverted) {
            world_velocity *= -1;
            SDL_Log("velocity inverted");
            vel_inv_start = SDL_GetTicks();
            vel_inv_elapsed = 0;
            world_vel_inverted = true;
        }
        vel_inv_elapsed = SDL_GetTicks() - vel_inv_start;
        if (vel_inv_elapsed > vel_inv_delay) {
            world_velocity = WORLD_VELOCITY_DEBUG;
            SDL_Log("velocity un-inverted");
            world_player_coll = false;
            world_vel_inverted = false;

        }
    }
    
}

void unlock_areas() {

    for (int i = 0; i < ALL_AREAS; i++) {

      

        for (int k = 0; k < world_areas[i].size; k++) {
            if (abs(world_areas[i].rects[k].x - (world_player.rect.x+TILE_WIDTH/2)) < TILE_WIDTH && abs(world_areas[i].rects[k].y - (world_player.rect.y + TILE_HEIGHT)) < TILE_HEIGHT *8) {
                for (int j = 0; j < ALL_KEYITEMS; j++) {
                    if (strcmp(world_areas[i].key.name, world_player.key_items[j].name) == 0) {
                        if (space_pressed) {
                            world_areas[i].is_walkable = true;
                        }
                    }
                }
            }

        }

    }
}

void set_spritesheet_rects(){
    int x = 0;
    int y = 0;
    for(int i =0; i< spritesheet_size; i++){
        spritesheet_rects[i].w = SPRITE_WIDTH;
        spritesheet_rects[i].h = SPRITE_HEIGHT;
        spritesheet_rects[i].x = x;
        spritesheet_rects[i].y = y;
        if (x + SPRITE_WIDTH + SPRITESHEET_MARGIN > (SPRITESHEET_FRAME_COLS * SPRITE_WIDTH)+ SPRITESHEET_MARGIN){
            y += SPRITE_HEIGHT + SPRITESHEET_MARGIN;
            x = 0;
        }
        else{
            x += SPRITE_WIDTH + SPRITESHEET_MARGIN;
        }

    }
}

void render_spritesheet(){
    for (int i = 0; i < spritesheet_size; i++){
        render_rect(renderer, spritesheet_rects[i], white);
    }
}
int output_spritesheet_bmp(void) {
    // Render map into target
    if (SDL_SetRenderTarget(renderer, spritesheet_target) != true) {
        SDL_Log("SDL_SetRenderTarget failed: %s", SDL_GetError());
        return 1;
    }


    // Draw everything into the target (same rendering code you use on screen)
    for (int i = 0; i < spritesheet_size; i++) {
        render_rect(renderer, spritesheet_rects[i], white);

    }

    // --- IMPORTANT ---
    // Read pixels WHILE target is still bound
    SDL_Surface* surface = SDL_RenderReadPixels(renderer, NULL);
    if (!surface) {
        SDL_Log("SDL_RenderReadPixels failed: %s", SDL_GetError());
        SDL_SetRenderTarget(renderer, NULL);
        return 1;
    }

    // Save BMP
    if (SDL_SaveBMP(surface, "spritesheet_output.bmp") != 0) {
        SDL_Log("SDL_SaveBMP failed: %s", SDL_GetError());
    }

    SDL_DestroySurface(surface);

    // Restore default target BEFORE present
    SDL_SetRenderTarget(renderer, NULL);

    SDL_Log("Exported spritesheet_output.bmp successfully!");

    return 0;
}

SDL_FRect create_src(int frame_width, int frame_height, int current_frame, int x, int y) {
    SDL_FRect src = {
       .x = current_frame * frame_width,
       .y = 0,   //y can be changed to multiples of height depending on input, to get at different rows of sheet
       .w = frame_width,
       .h = frame_height
    };

    return src;

}
SDL_FRect create_dst(int frame_width, int frame_height, int current_frame, int x, int y) {

    SDL_FRect dst = {
    .x = x,
    .y = y,
    .w = frame_width,
    .h = frame_height
    };

    return dst;

}
void render_animation(
    int total_frames,
    int* current_frame,
    Uint32* last_time,
    Uint32* now,
    Uint32 frame_delay,
    int x, int y,
    SDL_Texture* texture,
    SDL_Renderer* renderer,
    SDL_FRect src,
    SDL_FRect dst) {
    SDL_RenderTexture(renderer, texture, &src, &dst);

    if (*now - *last_time >= frame_delay) {
        //SDL_Log("next frame %d", *current_frame + 1);
        *current_frame = (*current_frame + 1) % total_frames;
        *last_time = SDL_GetTicks();
    }
}

void render_ui(){
        /*/these will be rendered using a function/functions, this data structure will help facilitate the UI rendering.
    if the layer is < 2, render the element
        if the label is strcmped to be equipped patterns, render the player_proj.type text there
        if the label is strcmped to be equipped mons, render the player_mons.name there
    if the element got activated and has a blanket, render that element until back button is pressed once
    if the element is a sub display, check what its label name is, then snprintf the scroll indices of the curr_disp_buff , into the display_text member
    if the user presses an equip (equip left or equip right separate for mons) or info button, the middle of the curr_disp_buff is selected/*/
    
    for(int i=0; i< ALL_UI_ELEMENTS; i++){
       
        if(ui_elements[i].layer <2){
            
            render_rect(renderer, ui_elements[i].rect, ui_elements[i].color);
            
            if(strcmp(ui_elements[i].label, "equipped pattern") ==0){
                render_text(ui_elements[i].rect.x, ui_elements[i].rect.y, black, player_proj.type, font);
            }
            
            if (strcmp(ui_elements[i].label, "equipped mons") == 0) {
                render_text(ui_elements[i].rect.x, ui_elements[i].rect.y, black, player_mons[0].name, font);
                render_text(ui_elements[i].rect.x, ui_elements[i].rect.y +100, black, player_mons[1].name, font);
            }

        }
        if(ui_elements[i].got_activated && ui_elements[i].blanket_ind >-1){
            
            
            //the blanket index will be used unless back is pressed, and if it doesnt have a subdisplay the below logic wont happen
            int blnk = ui_back_pressed? i: ui_elements[i].blanket_ind;
            float offset_newline = 0.0;
            render_rect(renderer, ui_elements[blnk].rect, ui_elements[blnk].color);//TODO until go back is pressed
            
            if (ui_elements[blnk].blanket_ind>-1){
                x_copy = ui_elements[blnk].rect.x;
                ui_elements[i].rect.x = offscreen_x;
            }
            
            if(ui_elements[blnk].is_sub_display){

  
                if (strcmp(ui_elements[blnk].label, "your patterns") ==0 && ui_state!= UIS_MON_LIST_OPEN){

                    ui_state = UIS_PATTERN_LIST_OPEN;

                    if(!sub_buffer_filled){//on scroll, refilled and reset

                        for(int j =0; j< UI_SCROLL_MAX; j++){
                            int scr_ind = scroll_indices[j];
                            mid_scr_ind = scroll_indices[1];//the middle in list that gets activated has its info displayed
                            //try to avoid potential out of bounds here
                            if( strcmp(player_proj_strs[scr_ind].str, "") !=0){
                                snprintf(curr_sub_disp_buff[j].display_text, sizeof(curr_sub_disp_buff[j].display_text), player_proj_strs[scr_ind].str);
                            }
                        }
                        sub_buffer_filled = true;
                    }
                    for (int p = 0; p < UI_SCROLL_MAX; p++) {
                        render_text(ui_elements[blnk].rect.x, ui_elements[blnk].rect.y +offset_newline, black, curr_sub_disp_buff[p].display_text, font );
                        offset_newline += 100;
                    }

                }
                if (strcmp(ui_elements[blnk].label, "your mons") == 0 && ui_state != UIS_PATTERN_LIST_OPEN) {

                    ui_state = UIS_MON_LIST_OPEN;

                    if (!sub_buffer_filled) {//on scroll, refilled and reset

                        for (int k = 0; k < UI_SCROLL_MAX; k++) {
                            int scr_ind = scroll_indices[k];
                            SDL_Log("Scroll index [k] = %i within your mons strcmp check", scroll_indices[k]);
                            mid_scr_ind = scroll_indices[1];//the middle in list that gets activated has its info displayed
                            //try to avoid potential out of bounds here
                            if (player_mon_list[scr_ind].spd != 0){
                                snprintf(curr_sub_disp_buff[k].display_text, sizeof(curr_sub_disp_buff[k].display_text), player_mon_list[scr_ind].name);
                            }
                        }
                        sub_buffer_filled = true;
                    }
                    for (int m = 0; m < UI_SCROLL_MAX; m++) {
                        render_text(ui_elements[blnk].rect.x, ui_elements[blnk].rect.y + offset_newline, black, curr_sub_disp_buff[m].display_text, font);
                        offset_newline += 100;
                    }
                   
                }
 
            }
            if (strcmp(ui_elements[blnk].label, "mon info") == 0) { //< not a sub display, so goes outside the condition check
                render_text(ui_elements[blnk].rect.x, ui_elements[blnk].rect.y, black, player_mon_list[mid_scr_ind].type, font);
            }
            if (strcmp(ui_elements[blnk].label, "yn prompt pattern") == 0){
                render_text(ui_elements[blnk].rect.x, ui_elements[blnk].rect.y, black, "Equip?", font);
                if (yesno) {
                    snprintf(player_proj.type, sizeof(player_proj.type), player_proj_strs[mid_scr_ind].str);
                    SDL_Log("Player pattern changed");
                    ui_open += 1;
                    yesno = false;
                }
            }
            if (strcmp(ui_elements[blnk].label, "yn prompt mon") == 0) {
                render_text(ui_elements[blnk].rect.x, ui_elements[blnk].rect.y, black, "Equip?", font);
                if (yesno) {
                    player_mons[0] = player_mon_list[mid_scr_ind];
                    load_ship_mons();
                    SDL_Log("Player mon changed");
                    ui_open += 1;
                    yesno = false;
                }
            }

 
        }


        if(ui_back_pressed){
            sub_buffer_filled = false;
            for (int u = 0; u < ALL_UI_ELEMENTS; u++) {
                if(ui_elements[u].rect.x == offscreen_x){
                    (ui_elements[u].rect.x = x_copy);
                }
                element_diplay_ind = 0;
                ui_elements[u].got_activated = false;
            }
            ui_state = UIS_IDLE;
        }

    }




}

void handle_scroll_event(){
    if(ui_state == UIS_MON_LIST_OPEN){
        //SDL_Log("Mon list size %i", mon_list_size);
        if (scrolldown_pressed ) {
            if (scroll_indices[0] != mon_list_size - UI_SCROLL_MAX) {
                sub_buffer_filled = false;
                for (int i = 0; i < UI_SCROLL_MAX; i++) {
                    
  
      
                    scroll_indices[i] += 1;
    
                    SDL_Log("Scroll index [i] = %i", scroll_indices[i]);
            
                }
            }
        }
        if(scrollup_pressed){
            if (scroll_indices[UI_SCROLL_MAX-1] != UI_SCROLL_MAX - 1) {
            
        
                sub_buffer_filled = false;
                for(int j=0; j<UI_SCROLL_MAX; j++){


                    scroll_indices[j] -= 1;

                    SDL_Log("Scroll index [j] = %i", scroll_indices[j]);
           
                }
            }
        }
    }
    if (ui_state == UIS_PATTERN_LIST_OPEN) {
        //SDL_Log("patt list size %i", pattern_list_size);
        if (scrolldown_pressed) {
            if (scroll_indices[0] != pattern_list_size - UI_SCROLL_MAX) {
                sub_buffer_filled = false;
                for (int i = 0; i < UI_SCROLL_MAX; i++) {
                    


                    scroll_indices[i] += 1;

                    SDL_Log("Scroll index [i] = %i", scroll_indices[i]);

                }
            }
        }
        if (scrollup_pressed) {
            if (scroll_indices[UI_SCROLL_MAX - 1] != UI_SCROLL_MAX - 1) {


                sub_buffer_filled = false;
                for (int j = 0; j < UI_SCROLL_MAX; j++) {


                    scroll_indices[j] -= 1;

                    SDL_Log("Scroll index [j] = %i", scroll_indices[j]);

                }
            }
        }

    }

    scrolldown_pressed = false;
    scrollup_pressed = false;
}

size_t get_mon_list_size(){
    size_t size = 0;
    size = sizeof(player_mon_list) / sizeof(Monster);
    return size;
}

int get_pattern_list_size() {
    int sc = 0;
    for(int i=0; i< ALL_PROJ_STRS; i++){

        if(isalpha(player_proj_strs[i].str[1])){
            sc++;
        }

    }
    
    return sc;
}