#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>

#include <SDL3/SDL_main.h>  // This must be included
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_RECTS 144
#define RECT_WIDTH 270
#define RECT_HEIGHT 180
#define MAX_CORNERS 100
#define WINDOW_WIDTH 3840
#define WINDOW_HEIGHT 2160
#define PLAYER_WIDTH 200
#define PLAYER_HEIGHT 200
#define ITEM_WIDTH 100
#define ITEM_HEIGHT 100
#define MAX_ITEMS 6
#define MAX_MONS 6
#define MON_WIDTH 200
#define MON_HEIGHT 200
#define PROJECTILE_WIDTH 50
#define PROJECTILE_HEIGHT 50
#define PROJECTILE_DELAY 600
#define PLAYER_PROJ_DELAY 400
#define HEALTH_LOSS_DELAY 60
#define MAX_MON_HP 100
#define MAX_PLAYER_HP 100

static SDL_Color white = { 255, 255, 255, 255 };
static SDL_Color red = { 255, 0, 0, 255 };
static SDL_Color blue = { 0, 0, 255, 255 };
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

float player_velocity = 0.2;//for button input controls(without culling must be higher due to hardware slowdown. might be something of a problem once release build)
float track_velocity = 10.0;
float max_track_vel = 25.0;
float acceleration = 0.0;//unused?
float projectile_velocity = 0.4;

Uint32 acc_start_time;
Uint32 acc_elapsed;
Uint32 deacc_start_time;
Uint32 deacc_elapsed;

//COLLISIONS
int check_rect_overlap(SDL_FRect* a, SDL_FRect* one, SDL_FRect* two);
int track_coll_count;
bool coll_top;
bool coll_bottom;
bool coll_started;
Uint32 coll_start_time;
Uint32 coll_elapsed;
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

//MONS STRUCT
typedef struct {
    char name[40];
    char type[40];
    int HP;
    int att;
    int def;
    int spd;
    bool encountered;//if true when shot on track, will be added to the battle
    SDL_FRect rect;

}Monster;


void render_text(float x, float y, SDL_Color color, const char* text);

//MON DATA
Monster monsters[MAX_MONS] = {
    {.name = "test0", .type = "type0", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 100,.encountered = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}},
    {.name = "test1", .type = "type1", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 100,.encountered = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}},
    {.name = "test2", .type = "type2", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 100,.encountered = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}},
    {.name = "test3", .type = "type3", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 100,.encountered = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}},
    {.name = "test4", .type = "type4", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 100,.encountered = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}},
    {.name = "test5", .type = "type5", .HP = MAX_MON_HP, .att = 100, .def = 100, .spd = 100,.encountered = false, .rect = {.w = MON_WIDTH, .h = MON_HEIGHT, .x = 0,.y = 0}},
};

//PROJECTILES
typedef struct {
    SDL_FRect rects;
}Projectile;
Projectile projectile;
Projectile player_proj;
bool proj_started;
Uint32 proj_start_time;
Uint32 proj_elapsed;


//gets current mon on screen so coord can be loaded
int curr_mon_on_screen();

void load_projectile();
bool projectiles_loaded = false;
int accel_projectile();
bool proj_pressed;
bool player_proj_started;
Uint32 player_proj_start_time;
Uint32 player_proj_elapsed;
void accel_player_proj();
void load_player_proj();
bool player_loaded;
bool enemy_hit_player = false;
bool player_hit_enemy = false;
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
Uint32 player_hp_start_time;
Uint32 enemy_hp_start_time;
Uint32 player_hp_elapsed;
Uint32 enemy_hp_elapsed;
Uint32 enemy_hp_delay = 300;
Uint32 player_hp_delay = 300;

//BATTLE


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
            if (!coll_started) {
                right_pressed = true;
            }
            break;
        case SDLK_A:
            if (!coll_started) {
                left_pressed = true;
            }
            break;
        case SDLK_S:
            if (!coll_started) {
                down_pressed = true;
            }
            break;
        case SDLK_W:
            if (!coll_started) {
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
    for (int k = 0; k < MAX_ITEMS; k++) {
        if (abs(player_rect.x - track_items.items[k].x) < PLAYER_WIDTH * 2 && abs(player_rect.y - track_items.items[k].y) < PLAYER_HEIGHT * 2) {
            if (check_rect_overlap(&player_rect, &track_items.items[k], NULL) == 1) {
                SDL_Log("Item Touched");
            }
        }
    }
    if (check_rect_overlap(&monsters[curr_mon_on_screen()].rect, &player_proj.rects, NULL) == 1) {
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


    return SDL_APP_CONTINUE;
}
//APP ITERATE
SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);//set to the amiga color for the background of the main game screen
    SDL_RenderClear(renderer);


    SDL_GetMouseState(&mouse_x, &mouse_y);

    now = SDL_GetTicks();
    //GAMESTATE SWITCH START
    switch (game_state) {

    case GAMESTATE_TRACK:
    {
        //generate track
        if (!track_generated) {
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
        break;
    }
    }

    //END GAMESTATE SWITCH
    SDL_RenderPresent(renderer);
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
    if (SDL_HasRectIntersectionFloat(a, one)) {
        return 1;


    }
    if (two != NULL) {

        if (SDL_HasRectIntersectionFloat(a, two)) {
            return 1;
        }
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
        if (!coll_started)
        {
            coll_start_time = SDL_GetTicks();
            coll_started = true;
        }
        if (coll_started) {
            player_rect.y += player_velocity * 2;
            coll_elapsed = SDL_GetTicks() - coll_start_time;
        }
        if (coll_elapsed > 100) {
            coll_started = false;
            coll_top = false;
        }

    }
    if ((coll_bottom) || player_rect.y > track_max - RECT_HEIGHT * 2) {
        if (!coll_started)
        {
            coll_start_time = SDL_GetTicks();
            coll_started = true;
        }
        if (coll_started) {
            player_rect.y -= player_velocity * 2;
            coll_elapsed = SDL_GetTicks() - coll_start_time;
        }
        if (coll_elapsed > 100) {
            coll_started = false;
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
        if (monsters[i].rect.x<WINDOW_WIDTH && monsters[i].rect.x>-500) {
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
        render_rect(renderer, player_proj.rects, blue);
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
    //displaying HP numbers
    if (enemy_hp_started) {
        snprintf(mon_hp_buff, sizeof(mon_hp_buff), "%d", monsters[curr_mon_on_screen()].HP);
        render_text(monsters[curr_mon_on_screen()].rect.x, monsters[curr_mon_on_screen()].rect.y, white, mon_hp_buff);
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

