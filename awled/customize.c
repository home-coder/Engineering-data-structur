/*************************************************************************
 > Filename   : customize.c
 > Author     : oneface - one_face@sina.com
 > Company    : 一尊还酹江月
 > Time       : 2018-04-02 13:59:23
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
#define LED_CHIP_NUMS	2

typedef struct {
	byte r;
	byte g;
	byte b;
}ledreg_data;

typedef enum {
	LED_COLOR_NONE = 0,
	LED_COLOR_HALF,
	LED_COLOR_FULL,
}LED_COLOR_LEVEL;

//--think
typedef struct {
	LED_COLOR_LEVEL r;	
	LED_COLOR_LEVEL g;	
	LED_COLOR_LEVEL b;	
}ledcolor_info;

typedef struct {
	byte led_nums;
	ledreg_data *p_ledreg_data;
	ledcolor_info *p_ledcolor_info;
}ledif_info;

typedef struct {
	byte reg_base;
	byte reg_end;
	byte reg_nums;
	const ledreg_data *ledreg_map; 
}ledreg_struct;

typedef struct {
	byte chip_id;
	byte led_nums;
	ledreg_struct ledreg_info;
}ledhw_info;

typedef enum {
	AW9818_1 = 0,
	AW9818_2,
}CHIP_ID;

ledif_info *led_info = NULL;

static const byte gamma_brightness[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //9
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //19
	1, 1, 1, 1, 2, 2, 2, 3, //27
	3, 3, 4, 4, 4, 5, 5, 5, //35
	6, 6, 7, 7, 8, 9, 10, 11,
	11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 21, 22, 23, 24, 26, 27,
	28, 29, 31, 32, 33, 35, 36, 38,
	39, 41, 42, 44, 45, 47, 48, 50,
	52, 54, 55, 57, 59, 61, 62, 63,
};

typedef enum {
	red = 0,
	orange,
	yellow,
	green,
	blue,
	indigo,
	purple,
	LED_MAX_COLOR
}LED_COLOR;

const ledcolor_info led_colors[LED_MAX_COLOR] = {                                         
	{LED_COLOR_FULL, LED_COLOR_NONE, LED_COLOR_NONE},   // red
	{LED_COLOR_FULL, LED_COLOR_HALF, LED_COLOR_NONE},   // orange
	{LED_COLOR_FULL, LED_COLOR_FULL, LED_COLOR_NONE},   // yellow                                                                                                                                                   
	{LED_COLOR_NONE, LED_COLOR_FULL, LED_COLOR_NONE},   // green                                         
	{LED_COLOR_NONE, LED_COLOR_NONE, LED_COLOR_FULL},   // blue
	{LED_COLOR_NONE, LED_COLOR_FULL, LED_COLOR_FULL},   //indigo                                         
	{LED_COLOR_FULL, LED_COLOR_NONE, LED_COLOR_FULL},   //purple                                         
};

const ledreg_data ledreg_map_9818_1[] = {
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
	{0x1, 0x2, 0x3},
	{0x5, 0x6, 0x7},
};

const ledreg_data ledreg_map_9819_2[] = {
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x0, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
	{0x9, 0x8, 0x7},
	{0x5, 0x4, 0x3},
};

const ledhw_info ledhw_info_struct[LED_CHIP_NUMS] = {
	{
		AW9818_1,
		18,
		{0x10, 0x40, (0x40-0x10+1), ledreg_map_9818_1},
	},
	{
		AW9818_2,
		18,
		{0x20, 0x60, (0x60-0x20+1), ledreg_map_9819_2},
	},
};

static const ledhw_info *get_ledhw_info(byte i)
{
	return &ledhw_info_struct[i];
}

static CHIP_ID get_chip_id(byte i)
{
	const ledhw_info *p_ledhw_info = get_ledhw_info(i);
	return p_ledhw_info->chip_id;
}

static byte get_led_nums(byte i)
{
	const ledhw_info *p_ledhw_info = get_ledhw_info(i);
	return p_ledhw_info->led_nums;
}

static void set_ledif_info(ledif_info *ledif_info_struct)
{
	led_info = ledif_info_struct;	
}

static void ledif_info_init()
{
	byte i, len;

	ledif_info *ledif_info_struct = (ledif_info *)malloc(sizeof(ledif_info) * LED_CHIP_NUMS);
	if (NULL != ledif_info_struct) {
		for (i = 0; i < LED_CHIP_NUMS; i++) {
			ledif_info_struct[i].led_nums = get_led_nums(i);
			len = sizeof(ledreg_data) * (ledif_info_struct[i].led_nums);
			ledif_info_struct[i].p_ledreg_data = (ledreg_data *)malloc(len);
			if (NULL != ledif_info_struct[i].p_ledreg_data) {
				memset((void *)ledif_info_struct[i].p_ledreg_data, 0, len);
			} else {
				printf("ledif_info_struct[%d].p_ledreg_data is NULL\n", i);
			}
			len = sizeof(ledcolor_info) * (ledif_info_struct[i].led_nums);
			ledif_info_struct[i].p_ledcolor_info = (ledcolor_info *)malloc(len);
			if (NULL != ledif_info_struct[i].p_ledcolor_info) {
				memset((void *)ledif_info_struct[i].p_ledcolor_info, 0, len);
			} else {
				printf("ledif_info_struct[%d].p_ledcolor_info is NULL\n", i);
			}
		}
	}

	set_ledif_info(ledif_info_struct);
}

static byte convert_data(byte brightness, LED_COLOR_LEVEL color)
{
	switch (color) {
		case LED_COLOR_NONE:
			brightness = 0;
			break;
		case LED_COLOR_HALF:
			brightness /= 2;
			break;
		case LED_COLOR_FULL:
			brightness = brightness;
			break;
		default:
			break;
	}

	return brightness;
}

static void show_regmap_and_regdata(byte led_index, ledreg_data *p_ledreg_data)
{
	const ledhw_info *p_ledhw_info = NULL;
	byte r, g, b;

	if (led_index < get_led_nums(0)) {
		p_ledhw_info = get_ledhw_info(0);
	} else {
		p_ledhw_info = get_ledhw_info(1);
		led_index -= get_led_nums(0);
	}

	//get regmap(r,g,b) and data(r,g,b)

	const ledreg_data *ldata = NULL;
	ldata = p_ledhw_info->ledreg_info.ledreg_map + led_index;
	printf("reg: (0x%x, 0x%x, 0x%x)\n", ldata->r, ldata->g, ldata->b);

	r = p_ledreg_data->r;
	g = p_ledreg_data->g;
	b = p_ledreg_data->b;
	printf("data: (0x%x, 0x%x, 0x%x)\n", r, g, b);
}

static void leddata_operate(byte led_index, byte brightness, const ledcolor_info color)
{
	ledif_info *ledif_info_struct = NULL;

	if (led_index < get_led_nums(0)) {
		ledif_info_struct = &led_info[0];
	} else {
		ledif_info_struct = &led_info[1];
	}

	//get brightness by level
	brightness = gamma_brightness[brightness];

	if (NULL != ledif_info_struct && NULL != ledif_info_struct->p_ledreg_data) {
		ledif_info_struct->p_ledreg_data->r = convert_data(brightness, color.r);
		ledif_info_struct->p_ledreg_data->g = convert_data(brightness, color.g);
		ledif_info_struct->p_ledreg_data->b = convert_data(brightness, color.b);

		show_regmap_and_regdata(led_index, ledif_info_struct->p_ledreg_data);
	} else if (ledif_info_struct == NULL) {
		printf("some struct is NULL\n");
	}

	//do like send i2c etc...
}

int main()
{
	ledif_info_init();

	leddata_operate(22, 32, led_colors[yellow]);

	return 0;
}
