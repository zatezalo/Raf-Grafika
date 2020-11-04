#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>

#include <game_constants.h>

static rafgl_raster_t doge;
static rafgl_raster_t upscaled_doge;
static rafgl_raster_t raster, raster2;
static rafgl_raster_t checker;
static rafgl_raster_t mushroom;

static rafgl_texture_t texture;

static rafgl_spritesheet_t hero;
static rafgl_spritesheet_t hero_veci;
static rafgl_spritesheet_t hero_veci_flipped;
static rafgl_spritesheet_t explosion;

static rafgl_raster_t upscaled_hero;
static rafgl_raster_t upscaled_hero_flipped;

static rafgl_pixel_rgb_t boja;//.rgba = rafgl_RGBA(0, 255, 255, 0);


static int upscaled_hero_width = 0, upscaled_hero_height = 0;


#define NUMBER_OF_TILES 17
rafgl_raster_t tiles[NUMBER_OF_TILES];

#define TILE_SIZE 64

#define WORLD_HEIGHT RASTER_HEIGHT/TILE_SIZE
#define WORLD_WIDTH RASTER_WIDTH/TILE_SIZE

#define MUSHROOM_HEIGHT 80
#define MUSHROOM_WIDTH 60


#define WORLD_SIZE 128
int tile_world[WORLD_HEIGHT][WORLD_WIDTH];

static int raster_width = RASTER_WIDTH, raster_height = RASTER_HEIGHT;


void init_tilemap(void)
{
    int x, y;

    for(y = 0; y < WORLD_HEIGHT; y++)
    {
        for(x = 0; x < WORLD_WIDTH; x++)
        {
            if(randf() > 0.7f)
            {
                tile_world[y][x] = 3 + rand() % 3;
            }
            else
            {
                tile_world[y][x] = rand() % 3;
            }
        }
    }
}

void render_tilemap(rafgl_raster_t *raster)
{
    int x, y;

    rafgl_raster_t *draw_tile;

    for(y = 0; y < WORLD_HEIGHT; y++) {

        for(x = 0; x < WORLD_WIDTH; x++) {
            draw_tile = tiles + (tile_world[y][x] % NUMBER_OF_TILES);
            //boja.rgba = rafgl_RGB(0, 128, 0);
            rafgl_raster_draw_raster(raster, draw_tile, x * TILE_SIZE, y * TILE_SIZE - draw_tile->height + TILE_SIZE, boja);
        }
    }
}


void main_state_init(GLFWwindow *window, void *args)
{
    rafgl_raster_load_from_image(&doge, "res/images/doge.png");
    rafgl_raster_load_from_image(&checker, "res/images/checker32.png");
    rafgl_raster_load_from_image(&mushroom, "res/images/80x60_mushriim_final.png");

    rafgl_raster_init(&upscaled_doge, raster_width, raster_height);
    rafgl_raster_bilinear_upsample(&upscaled_doge, &doge);

    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&raster2, raster_width, raster_height);

    int i;

    char tile_path[256];

    for(i = 0; i < NUMBER_OF_TILES; i++)
    {
        sprintf(tile_path, "res/tiles/svgset%d.png", i);
        rafgl_raster_load_from_image(&tiles[i], tile_path);
    }

    init_tilemap();

    rafgl_spritesheet_init(&hero, "res/images/character.png", 10, 4);
    rafgl_spritesheet_init(&explosion, "res/images/313x223_explosion_final.png", 4, 2);// ovde za eksploziju dodato

    upscaled_hero_width = 1200;
    upscaled_hero_height = 512;


    rafgl_raster_init(&upscaled_hero, upscaled_hero_width, upscaled_hero_height);
    rafgl_raster_init(&upscaled_hero_flipped, upscaled_hero_width, upscaled_hero_height);

    rafgl_raster_bilinear_upsample(&upscaled_hero, &hero.sheet);

    hero_veci.sheet_height = upscaled_hero_height;
    hero_veci.sheet_width = upscaled_hero_width;

    hero_veci.sheet = upscaled_hero;

    hero_veci.frame_height = upscaled_hero_height / 4;
    hero_veci.frame_width = upscaled_hero_width / 10;

    hero_veci_flipped.sheet_height = upscaled_hero_height;
    hero_veci_flipped.sheet_width = upscaled_hero_width;

    hero_veci_flipped.sheet = upscaled_hero_flipped;

    hero_veci_flipped.frame_height = upscaled_hero_height / 4;
    hero_veci_flipped.frame_width = upscaled_hero_width / 10;

    rafgl_texture_init(&texture);
}


int pressed;
float location = 0;
float selector = 0;

int animation_running = 0;
int animation_frame = 0;
int direction = 0;

int animation_exposion = 0;

int hero_pos_x = RASTER_WIDTH / 2;
int hero_pos_y = RASTER_HEIGHT / 2;

int hero_speed = 300;

int hover_frames = 0;
int hover_frames_explosion = 0;

int poz_x = 600;
int poz_y = 600;

static int pom_poz_x = 0;
static int pom_poz_y = 0;

int broj_pogodaka = 0;
int gore_dole = 0;
int veci = 0;

int udario = 0;
int animation_frame_exposion = 0;
int row = 0;



void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    int x, y;

    float xn, yn;

    rafgl_pixel_rgb_t sampled1, sampled2, resulting1, resulting2;


    for(y = 0; y < raster_height; y++) {
        yn = 1.0f * y / raster_height;
        for(x = 0; x < raster_width; x++) {
            xn = 1.0f * x / raster_width;

            sampled1 = pixel_at_m(upscaled_doge, x, y);
            sampled2 = rafgl_point_sample(&doge, xn, yn);

            resulting1 = sampled1;
            resulting2 = sampled2;

            resulting1.rgba = rafgl_RGB(0, 0, 0);
            resulting1 = sampled1;

            pixel_at_m(raster, x, y) = resulting1;
            pixel_at_m(raster2, x, y) = resulting2;
        }
    }

    poz_x = poz_x % raster.width;
    poz_y = poz_y % raster.height;

    render_tilemap(&raster);

    // CRTANJE PEÈURKE
    if(!udario){

        rafgl_raster_draw_raster(&raster, &mushroom, poz_x - 30, poz_y - 40, boja);
    }
    else {
        boja.rgba = rafgl_RGB(rand() % 256, rand() % 256, rand() % 256);
    }


    // HIT BOKS ZA HEROJA I PEÈURKU
    if(!veci){
        if(hero_pos_x + 60 >= poz_x - 30 && hero_pos_y <= poz_y + 40 &&
            hero_pos_x <= poz_x + 30 && hero_pos_y  + 64 >= poz_y - 40){
            udario = 1;
            if(udario) {
                pom_poz_x = poz_x;
                pom_poz_y = poz_y;
            }
            poz_x = rand() % raster.width;
            poz_y = rand() % raster.height;
            gore_dole = 0;
        }
    }
    else {
        if(hero_pos_x + 120 >= poz_x - 30 && hero_pos_y <= poz_y + 40 &&
            hero_pos_x <= poz_x + 30 && hero_pos_y  + 128 >= poz_y - 40){
            udario = 1;
            if(udario) {
                pom_poz_x = poz_x;
                pom_poz_y = poz_y;
            }
            poz_x = rand() % raster.width;
            poz_y = rand() % raster.height;
        }
    }

    // RUŠENJE DRVEÆA KDA VELIKI HEROJ IM PRIDJE
    if(veci) {
        if(gore_dole == 0){
            if(tile_world[(hero_pos_y + 128) / TILE_SIZE][(hero_pos_x + 60) / TILE_SIZE] >= 3 ){
                tile_world[(hero_pos_y + 128) / TILE_SIZE][(hero_pos_x + 60) / TILE_SIZE] = rand() % 3;
            }
        }
        else {
            if(tile_world[(hero_pos_y) / TILE_SIZE][(hero_pos_x + 60) / TILE_SIZE] >= 3 ){
                tile_world[(hero_pos_y) / TILE_SIZE][(hero_pos_x + 60) / TILE_SIZE] = rand() % 3;
            }
        }
    }

    // BIRANJE IZMEDJU FLIPOVANOG I OBICNOG
    if(game_data->keys_down[RAFGL_KEY_F]){
        if(gore_dole == 0)
            gore_dole = 1;
        else
            gore_dole = 0;
    }

    animation_running = 1;

    // BIRANJE KRETANJA ZA FLIPOVANOG I OBICNOG UVEÆANOG HEROJA
    if(gore_dole == 0){
        if(game_data->keys_down[RAFGL_KEY_W])
        {
            if(veci){
                hero_pos_y = hero_pos_y - hero_speed * delta_time;
                direction = 2;
            }
            else {
                if(tile_world[(hero_pos_y + 64) / TILE_SIZE - 1][(hero_pos_x + 30) / TILE_SIZE] < 3){
                    hero_pos_y = hero_pos_y - hero_speed * delta_time;
                    direction = 2;
                }
            }
        }
        else if(game_data->keys_down[RAFGL_KEY_S])
        {
            if(veci){
                hero_pos_y = hero_pos_y + hero_speed * delta_time;
                direction = 0;
            }
            else {
                if(tile_world[(hero_pos_y ) / TILE_SIZE + 1][(hero_pos_x + 30) / TILE_SIZE] < 3){
                    hero_pos_y = hero_pos_y + hero_speed * delta_time;
                    direction = 0;
                }
            }
        }
        else if(game_data->keys_down[RAFGL_KEY_A])
        {
            if(veci){
                hero_pos_x = hero_pos_x - hero_speed * delta_time;
                direction = 1;
            }
            else {
                if(tile_world[(hero_pos_y + 32) / TILE_SIZE][(hero_pos_x + 60) / TILE_SIZE - 1] < 3){
                    hero_pos_x = hero_pos_x - hero_speed * delta_time;
                    direction = 1;
                }
            }
        }
        else if(game_data->keys_down[RAFGL_KEY_D])
        {
            if(veci){
                hero_pos_x = hero_pos_x + hero_speed * delta_time;
                direction = 3;
            }
            else {
                if(tile_world[(hero_pos_y + 32) / TILE_SIZE][(hero_pos_x ) / TILE_SIZE + 1] < 3){
                    hero_pos_x = hero_pos_x + hero_speed * delta_time;
                    direction = 3;
                }
            }
        }
        else
        {
            animation_running = 0;
        }

        if(animation_running)
        {
            if(hover_frames == 0)
            {
                animation_frame = (animation_frame + 1) % 10;
                hover_frames = 5;
            }
            else
            {
                hover_frames--;
            }

        }
    }
    else {
        if(game_data->keys_down[RAFGL_KEY_W])
        {
            hero_pos_y = hero_pos_y - hero_speed * delta_time;
            direction = 3;
        }
        else if(game_data->keys_down[RAFGL_KEY_S])
        {
            hero_pos_y = hero_pos_y + hero_speed * delta_time;
            direction = 1;
        }
        else if(game_data->keys_down[RAFGL_KEY_A])
        {
            hero_pos_x = hero_pos_x - hero_speed * delta_time;
            direction = 2;
        }
        else if(game_data->keys_down[RAFGL_KEY_D])
        {
            hero_pos_x = hero_pos_x + hero_speed * delta_time;
            direction = 0;
        }
        else
        {
            animation_running = 0;
        }

        if(animation_running)
        {
            if(hover_frames == 0)
            {
                animation_frame = (animation_frame + 1) % 10;
                hover_frames = 5;
            }
            else
            {
                hover_frames--;
            }

        }
    }

    // SMENJIVANJE SLIKA ZA ANIMACIJU EKSPLOZIJE
    if(udario){
        if(hover_frames_explosion == 0)
        {
            animation_frame_exposion++;
            if(animation_frame_exposion == 4){
                animation_frame_exposion = 0;
                row = 1;
            }
            hover_frames_explosion = 5;
        }
        else
        {
            hover_frames_explosion--;
        }
    }

    rafgl_pixel_rgb_t sampled, resulting3;

    // PRAVLJENJE FLIPOVANOG HEROJA
    for(y = 0; y < upscaled_hero_height; y++) {
        for(x = 0; x < upscaled_hero_width; x++) {
            sampled = pixel_at_m(upscaled_hero, x, upscaled_hero_height - y - 1);
            resulting3 = sampled;
            pixel_at_m(upscaled_hero_flipped, x,y) = resulting3;

        }
    }

    // BIRANJE IZMEDJU MALOG I VELIKOG HEROJA
    if(game_data->keys_down[RAFGL_KEY_B]){
        if(veci == 0)
            veci = 1;
        else
            veci = 0;
    }


    // ISCRTAVALJE MALOG VELIKOG I OKRENUTOG HEROJA
    if(!veci) {
        hero_speed = 450;
        rafgl_raster_draw_spritesheet(&raster, &hero, animation_frame, direction, hero_pos_x, hero_pos_y);// ovo sve crta
    }
    else {
        hero_speed = 150;
        if(gore_dole == 0){
            rafgl_raster_draw_spritesheet(&raster, &hero_veci, animation_frame, direction, hero_pos_x, hero_pos_y);
        }
        else {
            rafgl_raster_draw_spritesheet(&raster, &hero_veci_flipped, animation_frame, direction, hero_pos_x, hero_pos_y);
        }
    }


    // ICRTAVANJE ANIMACIJE EKSPLOZIJE
    if(udario){
        rafgl_raster_draw_spritesheet(&raster, &explosion, animation_frame_exposion, row, pom_poz_x - 39, pom_poz_y - 55);
        if(row == 1 && animation_frame_exposion == 3){
            udario = 0;
            row = 0;
            animation_frame_exposion = 0;
        }
    }

    if(!game_data->keys_down[RAFGL_KEY_SPACE])
        rafgl_texture_load_from_raster(&texture, &raster);
    else
        rafgl_texture_load_from_raster(&texture, &raster2);
}


void main_state_render(GLFWwindow *window, void *args)
{
    rafgl_texture_show(&texture);
}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_raster_cleanup(&raster2);
    rafgl_texture_cleanup(&texture);

}
