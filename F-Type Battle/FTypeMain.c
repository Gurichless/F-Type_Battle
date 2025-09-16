#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>

#include <SDL3/SDL_main.h>  // This must be included
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

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

static SDL_Color white = { 255, 255, 255, 255 };
static SDL_Color red = { 255, 0, 0, 255 };
static SDL_Color blue = { 0, 0, 255, 255 };
static SDL_Color black = {0, 0, 0, 255 };
static SDL_Color highlight = { 0,20,178, 175 };
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static TTF_Font* font = NULL;
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
New_Corner new_track[MAX_CORNERS];
New_Corner new_track2[MAX_CORNERS];
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


//MOVEMENT
bool right_pressed;
bool left_pressed;
bool up_pressed;
bool down_pressed;
bool space_pressed;
bool acc_started;
bool deacc_started;

float player_velocity = 0.2*20;//for button input controls(without culling must be higher due to hardware slowdown. might be something of a problem once release build)
float track_velocity = 10.0 * 2;
float max_track_vel = 25.0 * 2;
float acceleration = 0.0;//unused?
float projectile_velocity = 0.4 * 20;

Uint32 acc_start_time;
Uint32 acc_elapsed;
Uint32 deacc_start_time;
Uint32 deacc_elapsed;

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
void handle_colls();
void accelerate();


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

}Monster;
Move moves[MAX_MOVES];

void render_text(float x, float y, SDL_Color color, const char* text);

//MON DATA
Monster monsters[MAX_MONS] = {
    {.name = "test0", .type = "type0", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 101, .wk ="A", .res ="B", .encountered = false,.turn= 1, .att_ind = NULL, .mov_ind= NULL,.is_in_party=false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name ="mov", .power=10, .type= "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}},
    {.name = "test1", .type = "type1", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 104,.wk = "A", .res = "B", .encountered = false, .turn = 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}},
    {.name = "test2", .type = "type2", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 109,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}},
    {.name = "test3", .type = "type3", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 103,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL,.mov_ind = NULL, .is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}},
    {.name = "test4", .type = "type4", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 106,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL, .mov_ind = NULL,.is_in_party = false,.rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}},
    {.name = "test5", .type = "type5", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 102,.wk = "A",.res = "B",.encountered = false,.turn= 1, .att_ind = NULL,.mov_ind = NULL,.is_in_party = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}, .moves = {{.name = "mov", .power = 10, .type = "typ" },{.name = "mov2", .power = 10, .type = "typ2" },{.name = "mov3", .power = 10, .type = "typ3" },{.name = "mov4", .power = 10, .type = "typ4" }}},
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
bool proj_started;
Uint32 proj_start_time;
Uint32 proj_elapsed;


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
void accel_player_proj_sin();

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



//GAME STATE ENUM
typedef enum {
    GAMESTATE_TRACK,
    GAMESTATE_BATTLE_START,


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
    font = TTF_OpenFont("./fonts/amiga.ttf", 35);
    if (!font) {
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
    srand((unsigned)time(NULL)); // seed
    return SDL_APP_CONTINUE;
}

//GAME EVENTS
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    if (event->type == SDL_EVENT_KEY_DOWN) {
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
        default:
            break;
        }
    }
    if (event->type == SDL_EVENT_KEY_UP) {
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
    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        proj_pressed = true;
    }
    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        proj_pressed = false;
    }
    
    for (int k = 0; k < MAX_ITEMS; k++) {
        if (abs(player_rect.x - track_items.items[k].x) < PLAYER_WIDTH * 2 && abs(player_rect.y - track_items.items[k].y) < PLAYER_HEIGHT * 2) {
            if (check_rect_overlap(&player_rect, &track_items.items[k], NULL) == 1) {
                SDL_Log("Item Touched");
            }
        }
    }
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
    if(get_other_on_screen()!=-1){
        if (check_rect_overlap(&player_proj.rects, &monsters[get_other_on_screen()].rect, &monsters[get_other_on_screen()].rect) == 1) {
            player_hit_other_enemy = true;
        }
        else{
            player_hit_other_enemy = false;
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
        render_track();
        render_rect(renderer, player_rect, white);
        move_player();
        handle_colls();


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
                                dynamic_battle_array[i].mov_ind = hl_ind;//set the mon's mov_ind to selected move
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
                                dynamic_battle_array[i].mov_ind = hl_ind;//set the mon's mov_ind to selected move
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

                //TODO set enemy's decisions based on RNG
                if (!enemy_decisions_set) {
                    for (int m = 0; m < dynamic_size; m++) {
                        if (!dynamic_battle_array[m].is_in_party) {
                            set_enemy_decisions(&dynamic_battle_array[m]);
                        }
                    }
                    enemy_decisions_set = true;
                }
                //TODO apply the effects of the attacks within the dynamic battle array based on the indices gathered above
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
        y1 = center_line_y2 + 50 + rand() % 200;
        y2 = y1 + 50 + rand() % 200;
    }
    //above center line
    else {
        y1 = center_line_y2 - 50 - rand() % 200;
        y2 = y1 - 50 - rand() % 200;
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

        last_time = SDL_GetTicks();
    }
}

void move_player() {
    //not else if if diagonal movement wanted
    if (up_pressed) {
        player_rect.y -= player_velocity;
    }
    if (down_pressed) {
        player_rect.y += player_velocity;
    }
    if (right_pressed) {
        player_rect.x += player_velocity;
    }
    if (left_pressed) {
        player_rect.x -= player_velocity;
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
    if ((coll_top) || player_rect.y < track_min + RECT_HEIGHT * 2) {
        if (!coll_started_t)
        {
            coll_start_time_t = SDL_GetTicks();
            coll_started_t = true;
        }
        if (coll_started_t) {
            player_rect.y += player_velocity * 1.5;
            coll_elapsed_t = SDL_GetTicks() - coll_start_time_t;
        }
        if (coll_elapsed_t > 100) {
            coll_started_t = false;
            coll_top = false;
        }

    }
    if ((coll_bottom) || player_rect.y > track_max - RECT_HEIGHT * 2) {
        if (!coll_started_b)
        {
            coll_start_time_b = SDL_GetTicks();
            coll_started_b = true;
        }
        if (coll_started_b) {
            player_rect.y -= player_velocity * 1.5;
            coll_elapsed_b = SDL_GetTicks() - coll_start_time_b;
        }
        if (coll_elapsed_b > 100) {
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
            render_text(monsters[i].rect.x, monsters[i].rect.y - 100, white, monsters[i].name);
        }
    }
}

void render_text(float x, float y, SDL_Color color, const char* text)
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
                
                //mouse above
                if (mouse_y < player_rect.y) {
                    
                    player_proj.rects.x += projectile_velocity + track_velocity;
                    player_proj.rects.y -= player_mouse_gap;
                }
                //mouse_below
                else if (mouse_y > player_rect.y) {
                    player_proj.rects.x += projectile_velocity + track_velocity;
                    player_proj.rects.y += player_mouse_gap;
                }
            }
        }
        if (player_proj.rects.x< WINDOW_WIDTH && player_proj.rects.x > -500){
            render_rect(renderer, player_proj.rects, blue);
        }
        player_proj_elapsed = SDL_GetTicks() - player_proj_start_time;
        if (player_proj_elapsed > PLAYER_PROJ_DELAY) {
            player_loaded = false;
            player_proj_start_time = SDL_GetTicks();

            player_proj_started = false;
            player_proj_elapsed = 0;
        }
    }
}
void accel_player_proj_sin() {
    if (proj_pressed) {
        if (!player_proj_started) {
            player_proj_start_time = SDL_GetTicks();
            player_proj_started = true;
        }
        else {
            if (mouse_in_valid_zone) {
                player_proj.rects.y += sin(sqrt(player_proj.rects.x)) * track_velocity;//sine wave, can perhaps merge this into previous func conditionally once setting up  chack of type of projectile
                //mouse above
                if (mouse_y < player_rect.y) {

                    player_proj.rects.x += projectile_velocity + track_velocity;
                    player_proj.rects.y -= player_mouse_gap;
                }
                //mouse_below
                else if (mouse_y > player_rect.y) {
                    player_proj.rects.x += projectile_velocity + track_velocity;
                    player_proj.rects.y += player_mouse_gap;
                }
            }
        }
        if (player_proj.rects.x< WINDOW_WIDTH && player_proj.rects.x > -500) {
            render_rect(renderer, player_proj.rects, blue);
        }
        player_proj_elapsed = SDL_GetTicks() - player_proj_start_time;
        if (player_proj_elapsed > PLAYER_PROJ_DELAY) {
            player_loaded = false;
            player_proj_start_time = SDL_GetTicks();

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
        render_text(monsters[curr_mon_on_screen()].rect.x, monsters[curr_mon_on_screen()].rect.y, white, mon_hp_buff);
        enemy_hp_elapsed = SDL_GetTicks() - enemy_hp_start_time;
    }
    else if(other_enemy_hp_started && monsters[get_other_on_screen()].rect.x >0){//need the and statement or 0 draws in upper left of render present when no other on screen
        snprintf(mon_hp_buff, sizeof(mon_hp_buff), "%d", monsters[get_other_on_screen()].HP);
        render_text(monsters[get_other_on_screen()].rect.x, monsters[get_other_on_screen()].rect.y, white, mon_hp_buff);
        enemy_hp_elapsed = SDL_GetTicks() - enemy_hp_start_time;
    }
    if (enemy_hp_elapsed > enemy_hp_delay) {
        enemy_hp_started = false;
        enemy_hp_start_time = SDL_GetTicks();
        enemy_hp_elapsed = 0;
    }

    if (player_hp_started) {
        snprintf(player_hp_buff, sizeof(player_hp_buff), "%d", player_hp);
        render_text(player_rect.x, player_rect.y - 40, white, player_hp_buff);
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
            render_text(dynamic_battle_array[i].rect.x, dynamic_battle_array[i].rect.y, white, dynamic_battle_array[i].name);
        }

    }
    for(int j =0; j< dynamic_size; j++){
        if(dynamic_battle_array[j].is_in_party){

            dynamic_battle_array[j].rect.x = render_rects.battle_locs[MAX_MONS+count].x;
            dynamic_battle_array[j].rect.y = render_rects.battle_locs[MAX_MONS+count].y;
            render_rect(renderer, dynamic_battle_array[j].rect, blue);
            render_text(dynamic_battle_array[j].rect.x, dynamic_battle_array[j].rect.y, white, dynamic_battle_array[j].name);
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
    render_text(render_rects.battle_menu[1].x, render_rects.battle_menu[1].y, black, "Fight");
    render_text(render_rects.battle_menu[2].x, render_rects.battle_menu[2].y, black, "Item");
    render_text(render_rects.battle_menu[3].x, render_rects.battle_menu[3].y, black, "Run");
}

//when  in the state where move menu is selected, render the moves on the battle menu rects, based on an index of which of the two player mons turn it is
void render_bm_moves(int mon_ind){
    

    render_text(render_rects.battle_menu[1].x, render_rects.battle_menu[1].y, black, player_mons[mon_ind].moves[0].name);
    render_text(render_rects.battle_menu[2].x, render_rects.battle_menu[2].y, black, player_mons[mon_ind].moves[1].name);
    render_text(render_rects.battle_menu[3].x, render_rects.battle_menu[3].y, black, player_mons[mon_ind].moves[2].name);
    render_text(render_rects.battle_menu[4].x, render_rects.battle_menu[4].y, black, player_mons[mon_ind].moves[3].name);
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
        render_text(target->rect.x, target->rect.y-200, white, battle_hp_buff);
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
    center_line_y = WINDOW_HEIGHT / 2 + 300;
    center_line_y2 = WINDOW_HEIGHT / 2 - 600;
    corner_x = 100.0;
    corner_x2 = 100.0;
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

int get_other_on_screen(){
    for(int i =0; i< MAX_MONS; i++){
        if (monsters[i].rect.x< WINDOW_WIDTH && monsters[i].rect.x > -500 && i != curr_mon_on_screen()){
            return i;
        }
    }
    return-1;

}
//abstracted away and moved from events to app iterate
void collision_events(){
    for (int i = 0; i < MAX_CORNERS; i++) {
        for (int j = 0; j < MAX_RECTS; j++) {
            {
                if (abs(new_track[i].corner[j].x - player_rect.x) <= RECT_WIDTH * 2 && abs(new_track[i].corner[j].y - player_rect.y) <= RECT_HEIGHT * 2) {
                    if (check_rect_overlap(&player_rect, &new_track[i].corner[j], &new_track2[i].corner[j]) == 1) {
                        track_coll_count++;
                        coll_bottom = true;
                        SDL_Log("Collision bottom %d", track_coll_count);
                        if (player_rect.y > new_track2[i].corner[j].y)
                        {
                            coll_bottom = true;
                        }
                    }

                }
                else if (abs(new_track2[i].corner[j].x - player_rect.x) <= RECT_WIDTH * 2 && abs(new_track2[i].corner[j].y - player_rect.y) <= RECT_HEIGHT * 2) {
                    if (check_rect_overlap(&player_rect, &new_track[i].corner[j], &new_track2[i].corner[j]) == 1) {
                        track_coll_count++;
                        coll_top = true;
                        SDL_Log("Collision top %d", track_coll_count);
                        if (player_rect.y < new_track2[i].corner[j].y)
                        {
                            coll_top = true;
                        }
                    }

                }

            }
        }

    }
}