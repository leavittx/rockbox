/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2005 Jonathan Bettencourt (jonrelay)
 *           (C) 2009 Port to Rockbox by: Yifu Huang
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

#include "plugin.h"
#include "lib/pluginlib_actions.h"

/* - - - PERIODIC TABLE DATA - - - */

typedef struct periodic_element_ {
    int group;
    int period;
    int number;
    char *symbol;
    char *name;
    char *mass;
    char *density;
    char *fpoint; /* freezing point */
    char *bpoint; /* boiling point */
    char block; /* s, p, d, f */
    int color; /* in periodic_color_palette below */
} periodic_element;

const periodic_element periodic_elements[] = {
    {  1,  1,   1, "H",   "Hydrogen",      "1.00794",     "0.0899",   "-259.14",  "-252.87",  's',  0 },
    { 18,  1,   2, "He",  "Helium",        "4.002602",    "0.1785",   "-272.2",   "-268.934", 's',  9 },
    {  1,  2,   3, "Li",  "Lithium",       "6.941",       "0.534",    "180.54",   "1347",     's',  1 },
    {  2,  2,   4, "Be",  "Beryllium",     "9.012182",    "1.848",    "1287",     "2472",     's',  2 },
    { 13,  2,   5, "B",   "Boron",         "10.811",      "2.34",     "2077",     "3870",     'p',  5 },
    { 14,  2,   6, "C",   "Carbon",        "12.0107",     "2.250",    "3727",     "3827",     'p',  6 },
    { 15,  2,   7, "N",   "Nitrogen",      "14.0067",     "1.2506",   "-209.86",  "-195.8",   'p',  6 },
    { 16,  2,   8, "O",   "Oxygen",        "15.9994",     "1.4291",   "-218.4",   "-182.96",  'p',  6 },
    { 17,  2,   9, "F",   "Fluorine",      "18.9984032",  "1.696",    "-219.62",  "-188.14",  'p',  7 },
    { 18,  2,  10, "Ne",  "Neon",          "20.1797",     "0.899990", "-248.67",  "-246.048", 'p',  9 },
    {  1,  3,  11, "Na",  "Sodium",        "22.98976928", "0.971",    "97.81",    "883",      's',  1 },
    {  2,  3,  12, "Mg",  "Magnesium",     "24.3050",     "1.738",    "650",      "1095",     's',  2 },
    { 13,  3,  13, "Al",  "Aluminum",      "26.9815386",  "2.6989",   "660.323",  "2520",     'p',  4 },
    { 14,  3,  14, "Si",  "Silicon",       "28.0855",     "2.329",    "1412",     "3266",     'p',  5 },
    { 15,  3,  15, "P",   "Phosphorus",    "30.973762",   "1.82",     "44.1",     "280.5",    'p',  6 },
    { 16,  3,  16, "S",   "Sulfur",        "32.065",      "2.07",     "112.8",    "444.674",  'p',  6 },
    { 17,  3,  17, "Cl",  "Chlorine",      "35.453",      "3.214",    "-100.98",  "-34.05",   'p',  7 },
    { 18,  3,  18, "Ar",  "Argon",         "39.948",      "1.7837",   "-189.2",   "-185.86",  'p',  9 },
    {  1,  4,  19, "K",   "Potassium",     "39.0983",     "0.862",    "63.65",    "765",      's',  1 },
    {  2,  4,  20, "Ca",  "Calcium",       "40.078",      "1.55",     "842",      "1503",     's',  2 },
    {  3,  4,  21, "Sc",  "Scandium",      "44.955912",   "2.989",    "1539",     "2831",     'd',  3 },
    {  4,  4,  22, "Ti",  "Titanium",      "47.867",      "4.54",     "1666",     "3289",     'd',  3 },
    {  5,  4,  23, "V",   "Vanadium",      "50.9415",     "6.11",     "1917",     "3420",     'd',  3 },
    {  6,  4,  24, "Cr",  "Chromium",      "51.9961",     "7.20",     "1857",     "2682",     'd',  3 },
    {  7,  4,  25, "Mn",  "Manganese",     "54.938045",   "7.44",     "1246",     "2062",     'd',  3 },
    {  8,  4,  26, "Fe",  "Iron",          "55.845",      "7.874",    "1536",     "2863",     'd',  3 },
    {  9,  4,  27, "Co",  "Cobalt",        "58.933195",   "8.9",      "1495",     "2930",     'd',  3 },
    { 10,  4,  28, "Ni",  "Nickel",        "58.6934",     "8.902",    "1455",     "2890",     'd',  3 },
    { 11,  4,  29, "Cu",  "Copper",        "63.546",      "8.96",     "1084.62",  "2571",     'd',  3 },
    { 12,  4,  30, "Zn",  "Zinc",          "65.38",       "7.13",     "419.527",  "907",      'd',  3 },
    { 13,  4,  31, "Ga",  "Gallium",       "69.723",      "5.904",    "29.7646",  "2208",     'p',  4 },
    { 14,  4,  32, "Ge",  "Germanium",     "72.64",       "5.323",    "937.4",    "2834",     'p',  5 },
    { 15,  4,  33, "As",  "Arsenic",       "74.92160",    "5.73",     "817",      "603",      'p',  5 },
    { 16,  4,  34, "Se",  "Selenium",      "78.96",       "4.79",     "220.2",    "684.9",    'p',  6 },
    { 17,  4,  35, "Br",  "Bromine",       "79.904",      "3.12",     "-7.2",     "58.78",    'p',  7 },
    { 18,  4,  36, "Kr",  "Krypton",       "83.798",      "3.733",    "-156.6",   "-153.35",  'p',  9 },
    {  1,  5,  37, "Rb",  "Rubidium",      "85.4678",     "1.532",    "38.89",    "688",      's',  1 },
    {  2,  5,  38, "Sr",  "Strontium",     "87.62",       "2.54",     "777",      "1414",     's',  2 },
    {  3,  5,  39, "Y",   "Yttrium",       "88.90585",    "4.469",    "1520",     "3388",     'd',  3 },
    {  4,  5,  40, "Zr",  "Zirconium",     "91.224",      "6.506",    "1852",     "4361",     'd',  3 },
    {  5,  5,  41, "Nb",  "Niobium",       "92.90638",    "8.57",     "2477",     "4744",     'd',  3 },
    {  6,  5,  42, "Mo",  "Molybdenum",    "95.96",       "10.22",    "2623",     "4682",     'd',  3 },
    {  7,  5,  43, "Tc",  "Technetium",    "[98]",        "11.500",   "2157",     "4265",     'd',  3 },
    {  8,  5,  44, "Ru",  "Ruthenium",     "101.07",      "12.41",    "2250",     "4155",     'd',  3 },
    {  9,  5,  45, "Rh",  "Rhodium",       "102.90550",   "12.41",    "1960",     "3697",     'd',  3 },
    { 10,  5,  46, "Pd",  "Palladium",     "106.42",      "12.02",    "1552",     "2964",     'd',  3 },
    { 11,  5,  47, "Ag",  "Silver",        "107.8682",    "10.50",    "961.78",   "2162",     'd',  3 },
    { 12,  5,  48, "Cd",  "Cadmium",       "112.411",     "8.65",     "321.03",   "767",      'd',  3 },
    { 13,  5,  49, "In",  "Indium",        "114.818",     "7.31",     "156.5985", "2072",     'p',  4 },
    { 14,  5,  50, "Sn",  "Tin",           "118.710",     "7.31",     "231.968",  "2603",     'p',  4 },
    { 15,  5,  51, "Sb",  "Antimony",      "121.760",     "6.691",    "630.74",   "1587",     'p',  5 },
    { 16,  5,  52, "Te",  "Tellurium",     "127.60",      "6.24",     "449.8",    "991",      'p',  5 },
    { 17,  5,  53, "I",   "Iodine",        "126.90447",   "4.93",     "113.6",    "184.35",   'p',  7 },
    { 18,  5,  54, "Xe",  "Xenon",         "131.293",     "5.887",    "-111.9",   "-108.1",   'p',  9 },
    {  1,  6,  55, "Cs",  "Caesium",       "132.9054519", "1.873",    "28.4",     "658",      's',  1 },
    {  2,  6,  56, "Ba",  "Barium",        "137.327",     "3.51",     "729",      "1898",     's',  2 },
    {  2,  9,  57, "La",  "Lanthanum",     "138.90547",   "6.145",    "920",      "3461",     'd',  3 },
    {  3,  9,  58, "Ce",  "Cerium",        "140.116",     "6.757",    "799",      "3426",     'f', 10 },
    {  4,  9,  59, "Pr",  "Praseodymium",  "140.90765",   "6.773",    "931",      "3520",     'f', 10 },
    {  5,  9,  60, "Nd",  "Neodymium",     "144.242",     "7.007",    "1021",     "3074",     'f', 10 },
    {  6,  9,  61, "Pm",  "Promethium",    "[145]",       "7.220",    "1100",     "3000",     'f', 10 },
    {  7,  9,  62, "Sm",  "Samarium",      "150.36",      "7.520",    "1072",     "1791",     'f', 10 },
    {  8,  9,  63, "Eu",  "Europium",      "151.964",     "5.243",    "822",      "1529",     'f', 10 },
    {  9,  9,  64, "Gd",  "Gadolinium",    "157.25",      "7.9004",   "1312",     "3266",     'f', 10 },
    { 10,  9,  65, "Tb",  "Terbium",       "158.92535",   "8.229",    "1356",     "3230",     'f', 10 },
    { 11,  9,  66, "Dy",  "Dysprosium",    "162.500",     "8.550",    "1412",     "2567",     'f', 10 },
    { 12,  9,  67, "Ho",  "Holmium",       "164.93032",   "8.795",    "1474",     "2700",     'f', 10 },
    { 13,  9,  68, "Er",  "Erbium",        "167.259",     "9.066",    "1529",     "2868",     'f', 10 },
    { 14,  9,  69, "Tm",  "Thulium",       "168.93421",   "9.321",    "1545",     "1950",     'f', 10 },
    { 15,  9,  70, "Yb",  "Ytterbium",     "173.054",     "6.965",    "819",      "1196",     'f', 10 },
    { 16,  9,  71, "Lu",  "Lutetium",      "174.9668",    "9.840",    "1663",     "3402",     'f', 10 },
    {  4,  6,  72, "Hf",  "Hafnium",       "178.49",      "13.31",    "2233",     "4603",     'd',  3 },
    {  5,  6,  73, "Ta",  "Tantalum",      "180.94788",   "16.654",   "2985",     "5510",     'd',  3 },
    {  6,  6,  74, "W",   "Tungsten",      "183.84",      "19.3",     "3407",     "5555",     'd',  3 },
    {  7,  6,  75, "Re",  "Rhenium",       "186.207",     "21.02",    "3180",     "5596",     'd',  3 },
    {  8,  6,  76, "Os",  "Osmium",        "190.23",      "22.57",    "3045",     "5012",     'd',  3 },
    {  9,  6,  77, "Ir",  "Iridium",       "192.217",     "22.42",    "2443",     "4437",     'd',  3 },
    { 10,  6,  78, "Pt",  "Platinum",      "195.084",     "21.45",    "1769",     "3827",     'd',  3 },
    { 11,  6,  79, "Au",  "Gold",          "196.966569",  "19.32",    "1064.18",  "2857",     'd',  3 },
    { 12,  6,  80, "Hg",  "Mercury",       "200.59",      "13.546",   "-38.842",  "356.58",   'd',  3 },
    { 13,  6,  81, "Tl",  "Thallium",      "204.3833",    "11.85",    "303.5",    "1473",     'p',  4 },
    { 14,  6,  82, "Pb",  "Lead",          "207.2",       "11.35",    "327.5",    "1750",     'p',  4 },
    { 15,  6,  83, "Bi",  "Bismuth",       "208.98040",   "9.747",    "271.4",    "1561",     'p',  4 },
    { 16,  6,  84, "Po",  "Polonium",      "[209]",       "9.32",     "254",      "962",      'p',  4 },
    { 17,  6,  85, "At",  "Astatine",      "[210]",       "-",        "302",      "337",      'p',  8 },
    { 18,  6,  86, "Rn",  "Radon",         "[222]",       "9.73",     "-71",      "-61.8",    'p',  9 },
    {  1,  7,  87, "Fr",  "Francium",      "[223]",       "2.410",    "27",       "657",      's',  1 },
    {  2,  7,  88, "Ra",  "Radium",        "[226]",       "5.00",     "700",      "1140",     's',  2 },
    {  2, 10,  89, "Ac",  "Actinium",      "[227]",       "10.07",    "1051",     "3200",     'd',  4 },
    {  3, 10,  90, "Th",  "Thorium",       "232.03806",   "11.720",   "1750",     "4789",     'f', 11 },
    {  4, 10,  91, "Pa",  "Protactinium",  "231.03588",   "15.370",   "1567",     "4227",     'f', 11 },
    {  5, 10,  92, "U",   "Uranium",       "238.02891",   "18.95",    "1132.3",   "4172",     'f', 11 },
    {  6, 10,  93, "Np",  "Neptunium",     "[237]",       "20.45",    "637",      "4000",     'f', 11 },
    {  7, 10,  94, "Pu",  "Plutonium",     "[244]",       "19.84",    "639.5",    "3231",     'f', 11 },
    {  8, 10,  95, "Am",  "Americiam",     "[243]",       "13.67",    "1176",     "2607",     'f', 11 },
    {  9, 10,  96, "Cm",  "Curium",        "[247]",       "13.51",    "1340",     "3110",     'f', 11 },
    { 10, 10,  97, "Bk",  "Berkelium",     "[247]",       "14.78",    "986",      "-",        'f', 11 },
    { 11, 10,  98, "Cf",  "Californium",   "[251]",       "15.1",     "900",      "-",        'f', 11 },
    { 12, 10,  99, "Es",  "Einsteinium",   "[252]",       "-",        "860",      "-",        'f', 11 },
    { 13, 10, 100, "Fm",  "Fermium",       "[257]",       "-",        "1527",     "-",        'f', 11 },
    { 14, 10, 101, "Md",  "Mendelevium",   "[258]",       "-",        "827",      "-",        'f', 11 },
    { 15, 10, 102, "No",  "Nobelium",      "[259]",       "-",        "827",      "-",        'f', 11 },
    { 16, 10, 103, "Lr",  "Lawrencium",    "[262]",       "-",        "1627",     "-",        'f', 11 },
    {  4,  7, 104, "Rf",  "Rutherfordium", "[267]",       "-",        "-",        "-",        'd',  3 },
    {  5,  7, 105, "Db",  "Dubnium",       "[268]",       "-",        "-",        "-",        'd',  3 },
    {  6,  7, 106, "Sg",  "Seaborgium",    "[271]",       "-",        "-",        "-",        'd',  3 },
    {  7,  7, 107, "Bh",  "Bhorium",       "[272]",       "-",        "-",        "-",        'd',  3 },
    {  8,  7, 108, "Hs",  "Hassium",       "[270]",       "-",        "-",        "-",        'd',  3 },
    {  9,  7, 109, "Mt",  "Meitnerium",    "[276]",       "-",        "-",        "-",        'd',  3 },
    { 10,  7, 110, "Ds",  "Darmstadtium",  "[281]",       "-",        "-",        "-",        'd',  3 },
    { 11,  7, 111, "Rg",  "Roentgenium",   "[280]",       "-",        "-",        "-",        'd',  3 },
    { 12,  7, 112, "Uub", "Ununbium",      "[285]",       "-",        "-",        "-",        'd',  3 },
    { 13,  7, 113, "Uut", "Ununtrium",     "[284]",       "-",        "-",        "-",        'p',  4 },
    { 14,  7, 114, "Uuq", "Ununquadium",   "[289]",       "-",        "-",        "-",        'p',  4 },
    { 15,  7, 115, "Uup", "Ununpentium",   "[288]",       "-",        "-",        "-",        'p',  4 },
    { 16,  7, 116, "Uuh", "Ununhexium",    "[293]",       "-",        "-",        "-",        'p',  4 },
    /* The 117th element does not find. */
    { 18,  7, 118, "Uuo", "Ununoctium",    "[294]",       "-",        "-",        "-",        'p',  9 }
};

#define ELEMENTS_COUNT sizeof(periodic_elements)/sizeof(periodic_element)

/* - - - PERIODIC TABLE VARIABLES - - - */
#if LCD_DEPTH > 1
static unsigned periodic_color_palette[12];
#endif
static unsigned int periodic_sel = 0;
static int font_height = 0;

/* - - - TINYFONT - - - */

const long int periodic_tinyfont_bitmaps[] = {
    0x00000000, 0x00220200, 0x00550000, 0x00575750, 0x00236200, 0x00514500, 0x00236300, 0x00220000,
    0x00244200, 0x00211200, 0x00272720, 0x00027200, 0x00000240, 0x00007000, 0x00000200, 0x00122400,
    0x00255200, 0x00262700, 0x00612700, 0x00631600, 0x00571100, 0x00761600, 0x00365200, 0x00712200,
    0x00725200, 0x00353100, 0x00020200, 0x00020240, 0x00024200, 0x00070700, 0x00021200, 0x00610200,
    
    0x00254300, 0x00257500, 0x00675600, 0x00344300, 0x00655600, 0x00764700, 0x00746400, 0x00345300,
    0x00577500, 0x00722700, 0x00722400, 0x00566500, 0x00444700, 0x00775500, 0x00575500, 0x00255200,
    0x00656400, 0x00255300, 0x00656500, 0x00363600, 0x00722200, 0x00555200, 0x00552200, 0x00557700,
    0x00522500, 0x00522200, 0x00714700, 0x00322300, 0x00422100, 0x00622600, 0x00250000, 0x00000700,
    
    0x00420000, 0x00063300, 0x00465600, 0x00034300, 0x00135300, 0x00026300, 0x00127200, 0x00035360,
    0x00465500, 0x00202200, 0x00202240, 0x00456500, 0x00222200, 0x00067500, 0x00065500, 0x00025200,
    0x00065640, 0x00035310, 0x00056400, 0x00032600, 0x00272100, 0x00055300, 0x00057200, 0x00057700,
    0x00052500, 0x00055360, 0x00072700, 0x00162100, 0x00222220, 0x00432400, 0x00630000, 0x00525250
};

void periodic_tinyfont_draw_char(int x, int y, char ch)
{
    int i,j;
    int r;
    long int bmp = periodic_tinyfont_bitmaps[ch-' '];
    for (i=7; i>=0; i--) {
        r = bmp & 15;
        bmp /= 16;
        for (j=3; j>=0; j--) {
            if (r & 1) {
                rb->lcd_drawpixel(x+j, y+i);
            }
            r /= 2;
        }
    }
}

void periodic_tinyfont_draw_string(int x, int y, char * s)
{
    char * t = s;
    int xx = x;
    int yy = y;
    while (*t) {
        if ((*t) == 10) {
            xx = x;
            yy += 8;
        } else {
            periodic_tinyfont_draw_char(xx, yy, *t);
            xx += 4;
        }
        t++;
    }
}

/*
void periodic_tinyfont_draw_chart(int x, int y, ttk_color col)
{
    unsigned char ch;
    int xx = x;
    int yy = y;
    for (ch = 32; ch < 128; ch++) {
        periodic_tinyfont_draw_char(xx, yy, ch, col);
        if ((ch % 16) == 15) {
            xx = x;
            yy += 8;
        } else {
            xx += 4;
        }
    }
}
*/

/* - - - KEYMAP - - - */
#if CONFIG_KEYPAD == RECORDER_PAD

#define PERIODIC_KEY_SELECT  BUTTON_PLAY
#define PERIODIC_KEY_MENU    BUTTON_OFF
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == ONDIO_PAD

#define PERIODIC_KEY_SELECT  BUTTON_MENU
#define PERIODIC_KEY_MENU    BUTTON_OFF
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == IAUDIO_X5M5_PAD

#define PERIODIC_KEY_SELECT  BUTTON_PLAY
#define PERIODIC_KEY_MENU    BUTTON_SELECT
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == IAUDIO_M3_PAD

#define PERIODIC_KEY_SELECT  BUTTON_PLAY
#define PERIODIC_KEY_MENU    BUTTON_MODE
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif (CONFIG_KEYPAD == IRIVER_H100_PAD) || \
      (CONFIG_KEYPAD == IRIVER_H300_PAD)

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_MODE
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == IRIVER_H10_PAD

#define PERIODIC_KEY_SELECT  BUTTON_FF
#define PERIODIC_KEY_MENU    BUTTON_REW
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif (CONFIG_KEYPAD == IPOD_1G2G_PAD) || \
      (CONFIG_KEYPAD == IPOD_3G_PAD)   || \
      (CONFIG_KEYPAD == IPOD_4G_PAD)

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_MENU
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == GIGABEAT_PAD

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_MENU
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == GIGABEAT_S_PAD
#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_BACK
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == PHILIPS_HDD1630_PAD

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_POWER
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == SANSA_E200_PAD

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_POWER
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == SANSA_FUZE_PAD

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_HOME
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == SANSA_C200_PAD

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_POWER
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == MROBE100_PAD

#define PERIODIC_KEY_SELECT  BUTTON_SELECT
#define PERIODIC_KEY_MENU    BUTTON_MENU
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif CONFIG_KEYPAD == SAMSUNG_YH_PAD

#define PERIODIC_KEY_SELECT  BUTTON_PLAY
#define PERIODIC_KEY_MENU    BUTTON_REW
#define PERIODIC_KEY_BACK    BUTTON_LEFT
#define PERIODIC_KEY_FORWARD BUTTON_RIGHT

#elif (CONFIG_KEYPAD == ONDAVX747_PAD) || \
      (CONFIG_KEYPAD == ONDAVX777_PAD) || \
      (CONFIG_KEYPAD == MROBE500_PAD)
#define PERIODIC_KEY_MENU    BUTTON_POWER

#else
//#error No keymap defined!
//#define PERIODIC_KEY_SELECT  PLA_SELECT
#define PERIODIC_KEY_MENU    PLA_EXIT
//#define PERIODIC_KEY_BACK    BUTTON_LEFT
//#define PERIODIC_KEY_FORWARD BUTTON_RIGHT
#endif

#ifdef HAVE_TOUCHSCREEN
#ifndef PERIODIC_KEY_BACK
#define PERIODIC_KEY_BACK    BUTTON_MIDLEFT
#endif
#ifndef PERIODIC_KEY_FORWARD
#define PERIODIC_KEY_FORWARD BUTTON_MIDRIGHT
#endif
#ifndef PERIODIC_KEY_SELECT
#define PERIODIC_KEY_SELECT  BUTTON_CENTER
#endif
#endif

/* - - - SOME CONSTANTS - - - */
#ifdef HAVE_LCD_COLOR
#define PERIODIC_COLOR_BLACK LCD_RGBPACK(0,0,0)
#elif LCD_DEPTH > 1
#define PERIODIC_COLOR_BLACK LCD_BLACK
#endif

#ifdef HAVE_LCD_COLOR
#define PERIODIC_COLOR_WHITE LCD_RGBPACK(255,255,255)
#elif LCD_DEPTH > 1
#define PERIODIC_COLOR_WHITE LCD_WHITE
#endif

#define CELLSIZE ((LCD_WIDTH-1)/18)
#define NUM_LINE ((LCD_HEIGHT - CELLSIZE * 10 + (CELLSIZE >> 1))/font_height)

#define CELSIUS "\xc2\xb0\x43"
#define CUBIC_CENTIMETER "\x63\x6d\xc2\xb3"

/* - - - PERIODIC TABLE HELPER FUNCTIONS - - - */

void periodic_makecols(void)
{
#ifdef HAVE_LCD_COLOR
    int i = 0;

    periodic_color_palette[i++] = LCD_RGBPACK(255,255,255); /* 0 hydrogen */
    periodic_color_palette[i++] = LCD_RGBPACK(221,221,221); /* 1 alkali metals */
    periodic_color_palette[i++] = LCD_RGBPACK(187,187,187); /* 2 earth metals */
    periodic_color_palette[i++] = LCD_RGBPACK(255,204,255); /* 3 transition metals */
    periodic_color_palette[i++] = LCD_RGBPACK(255,204,204); /* 4 other metals */
    periodic_color_palette[i++] = LCD_RGBPACK(255,204,153); /* 5 metalloids */
    periodic_color_palette[i++] = LCD_RGBPACK(255,255,204); /* 6 nonmetals */
    periodic_color_palette[i++] = LCD_RGBPACK(204,255,204); /* 7 halogens */
    periodic_color_palette[i++] = LCD_RGBPACK(204,204,153); /* 8 astatine */
    periodic_color_palette[i++] = LCD_RGBPACK(204,255,255); /* 9 noble gases */
    periodic_color_palette[i++] = LCD_RGBPACK(153,204,255); /* 10 lanthanides */
    periodic_color_palette[i++] = LCD_RGBPACK(204,153,255); /* 11 actinides */
#elif LCD_DEPTH > 1
    int i = 0;

    periodic_color_palette[i++] = LCD_WHITE;       /* 0 hydrogen */
    periodic_color_palette[i++] = LCD_DARKGRAY;    /* 1 alkali metals */
    periodic_color_palette[i++] = LCD_DARKGRAY;    /* 2 earth metals */
    periodic_color_palette[i++] = LCD_WHITE;       /* 3 transition metals */
    periodic_color_palette[i++] = LCD_WHITE;       /* 4 other metals */
    periodic_color_palette[i++] = LCD_DARKGRAY;    /* 5 metalloids */
    periodic_color_palette[i++] = LCD_WHITE;       /* 6 nonmetals */
    periodic_color_palette[i++] = LCD_WHITE;       /* 7 halogens */
    periodic_color_palette[i++] = LCD_DARKGRAY;    /* 8 astatine */
    periodic_color_palette[i++] = LCD_WHITE;       /* 9 noble gases */
    periodic_color_palette[i++] = LCD_DARKGRAY;    /* 10 lanthanides */
    periodic_color_palette[i++] = LCD_DARKGRAY;    /* 11 actinides */
#endif
}

int periodic_fix_period(int p)
{
    return (p>8)?(p-3):p;
}

int periodic_fix_group(int g, int p)
{
    return (p>8)?0:g;
}

char * periodic_group_name(int i)
{
    if (i==1) {
        return "Noble Gases";
    }
    else if (periodic_elements[i].period > 8)
    {
        switch (periodic_elements[i].period)
        {
            case 9:
                return "Lanthanoids";
                break;
            case 10:
                return "Actinoids";
                break;
        }
        return "";
    }
    else if (i>1)
    {
        switch (periodic_elements[i].group) {
            case 1:
                return "Alkali Metals";
                break;
            case 2:
                return "Alkaline Earth Metals";
                break;
            case 15:
                return "Pnictogens";
                break;
            case 16:
                return "Chalcogens";
                break;
            case 17:
                return "Halogens";
                break;
            case 18:
                return "Noble Gases";
                break;
        }
    }
    return "";
}

void periodic_draw_element(unsigned int i, int bx, int by)
{
    int ex = (bx+((periodic_elements[i].group-1) * CELLSIZE));
    int ey = (by+((periodic_elements[i].period-1) * CELLSIZE));
#if LCD_DEPTH > 1
    int tx, ty;
    char tt[4];
#endif

    if (periodic_elements[i].period > 8)
        ey -= (CELLSIZE >> 1);

    if (i == periodic_sel)
    {
#if LCD_DEPTH > 1
        rb->lcd_set_foreground(PERIODIC_COLOR_BLACK);
#endif
#if LCD_DEPTH > 1
        rb->lcd_fillrect(ex, ey, CELLSIZE+1, CELLSIZE+1);
#else
        rb->lcd_set_drawmode(DRMODE_FG);
        rb->lcd_fillrect(ex, ey, CELLSIZE+1, CELLSIZE+1);
        rb->lcd_set_drawmode(DRMODE_SOLID);
#endif
    }
    else
    {
#if LCD_DEPTH > 1
        rb->lcd_set_foreground(periodic_color_palette[periodic_elements[i].color]);
        rb->lcd_fillrect(ex, ey, CELLSIZE+1, CELLSIZE+1);
        rb->lcd_set_foreground(PERIODIC_COLOR_BLACK);
#else
        rb->lcd_set_drawmode(DRMODE_BG+DRMODE_INVERSEVID);
        rb->lcd_fillrect(ex, ey, CELLSIZE+1, CELLSIZE+1);
        rb->lcd_set_drawmode(DRMODE_SOLID);
#endif
        rb->lcd_drawrect(ex, ey, CELLSIZE+1, CELLSIZE+1);
    }
#if LCD_DEPTH > 1
    if (CELLSIZE >= 8) {
        rb->strcpy(tt, periodic_elements[i].symbol);
        if (rb->strlen(tt) > 2) {
            tt[1] = tt[rb->strlen(tt)-1];
            tt[2] = 0;
            tx = ex + (CELLSIZE >> 1) - 4;
        } else if (tt[1] != 0) {
            tx = ex + (CELLSIZE >> 1) - 4;
        } else {
            tx = ex + (CELLSIZE >> 1) - 2;
        }
        ty = ey + (CELLSIZE >> 1) - 4;

        if (i == periodic_sel)
        {
            rb->lcd_set_foreground(PERIODIC_COLOR_WHITE);
        }
        else
        {
            rb->lcd_set_foreground(PERIODIC_COLOR_BLACK);
        }
        periodic_tinyfont_draw_string(tx, ty, tt);
    }
#endif
}

void periodic_draw_table(void)
{
    int bx = (LCD_WIDTH - CELLSIZE * 18) >> 1;
    int by = 1;
    unsigned int i;
    for (i = 0; i < ELEMENTS_COUNT; i++) {
        periodic_draw_element(i, bx, by);
    }
}

void periodic_draw(void)
{
    char buf[40];

    periodic_draw_table();

    /* show the info for the current element */

    /* X coordinate of name/group */
    int nx = (LCD_WIDTH >> 1) - CELLSIZE * 7 + 5;

    /* X coordinate of first column */
    int fx = 0;
#if LCD_WIDTH >= 220
    int sx = (LCD_WIDTH > 220)? 120 : 90;
#endif

    /* Y coordinate of first/second top column */
    int ty = LCD_HEIGHT - font_height * NUM_LINE;

    periodic_element e;
    e = periodic_elements[periodic_sel];

#ifdef HAVE_LCD_COLOR
    rb->lcd_set_foreground(PERIODIC_COLOR_WHITE);
#endif

    /* display name and group up top */
#if LCD_WIDTH > 160
    rb->snprintf(buf, 40, "Name: %s", e.name);
    rb->lcd_putsxy(nx, 0, buf);
    rb->snprintf(buf, 40, "Group: %d %s", periodic_fix_group(e.group, e.period),
                 periodic_group_name(periodic_sel));
    rb->lcd_putsxy(nx, font_height, buf);
#else
    if (NUM_LINE > 0)
    {
        rb->snprintf(buf, 40, "%s", e.name);
        rb->lcd_putsxy(nx, 0, buf);
        rb->snprintf(buf, 40, "%d %s", periodic_fix_group(e.group, e.period),
                     periodic_group_name(periodic_sel));
        rb->lcd_putsxy(nx, font_height, buf);
    }
    else
    {
        rb->snprintf(buf, 40, "%d %s %s", e.number, e.symbol, e.name);
        rb->lcd_putsxy(nx, 0, buf);
        rb->snprintf(buf, 40, "%s amu", e.mass);
        rb->lcd_putsxy(nx, font_height, buf);
    }
#endif

    /* first col of info */
    if (NUM_LINE >= 8)
    {
        rb->snprintf(buf, 40, "Number: %d", e.number);
        rb->lcd_putsxy(fx, ty, buf);
        rb->snprintf(buf, 40, "Symbol: %s", e.symbol);
        rb->lcd_putsxy(fx, ty + font_height, buf);
        rb->snprintf(buf, 40, "Period: %d", periodic_fix_period(e.period));
        rb->lcd_putsxy(fx, ty + font_height*2, buf);
        rb->snprintf(buf, 40, "Block:  %c", e.block);
        rb->lcd_putsxy(fx, ty + font_height*3, buf);

        rb->snprintf(buf, 40, "Mass:    %s amu", e.mass);
        rb->lcd_putsxy(fx, ty + font_height*4, buf);
        rb->snprintf(buf, 40, "Density: %s g/%s", e.density, CUBIC_CENTIMETER);
        rb->lcd_putsxy(fx, ty + font_height*5, buf);
        rb->snprintf(buf, 40, "F Point: %s %s", e.fpoint, CELSIUS);
        rb->lcd_putsxy(fx, ty + font_height*6, buf);
        rb->snprintf(buf, 40, "B Point: %s %s", e.bpoint, CELSIUS);
        rb->lcd_putsxy(fx, ty + font_height*7, buf);
    }
    else
    {
#if LCD_WIDTH >= 220  /* shuld be NUM_LINE >= 4 */
        rb->snprintf(buf, 40, "Number: %d", e.number);
        rb->lcd_putsxy(fx, ty, buf);
        rb->snprintf(buf, 40, "Symbol: %s", e.symbol);
        rb->lcd_putsxy(fx, ty + font_height, buf);
        rb->snprintf(buf, 40, "Period: %d", periodic_fix_period(e.period));
        rb->lcd_putsxy(fx, ty + font_height*2, buf);
        rb->snprintf(buf, 40, "Block:  %c", e.block);
        rb->lcd_putsxy(fx, ty + font_height*3, buf);
#else
        if (NUM_LINE >= 4)
        {
            rb->snprintf(buf, 40, "%d %s  P: %d B: %c", e.number, e.symbol,
                         periodic_fix_period(e.period), e.block);
            rb->lcd_putsxy(fx, ty, buf);
            rb->snprintf(buf, 40, "M: %s amu", e.mass);
            rb->lcd_putsxy(fx, ty + font_height, buf);
            rb->snprintf(buf, 40, "D: %s g/%s", e.density, CUBIC_CENTIMETER);
            rb->lcd_putsxy(fx, ty + font_height*2, buf);
            rb->snprintf(buf, 40, "F/B: %s %s / %s %s", e.fpoint, CELSIUS, e.bpoint, CELSIUS);
            rb->lcd_putsxy(fx, ty + font_height*3, buf);
        }
        else
        {
            if (NUM_LINE >= 1)
            {
                rb->snprintf(buf, 40, "%d %s %s amu", e.number, e.symbol, e.mass);
                rb->lcd_putsxy(fx, ty, buf);
            }
            if (NUM_LINE >= 2)
            {
                rb->snprintf(buf, 40, "D: %s g/%s", e.density, CUBIC_CENTIMETER);
                rb->lcd_putsxy(fx, ty + font_height, buf);
            }
            if (NUM_LINE >= 3)
            {
                rb->snprintf(buf, 40, "F/B: %s %s / %s %s", e.fpoint, CELSIUS, e.bpoint, CELSIUS);
                rb->lcd_putsxy(fx, ty + font_height*2, buf);
            }
        }
#endif
    }

    /* second col of info */
#if LCD_WIDTH >= 220
    if (NUM_LINE < 8)
    {
        rb->snprintf(buf, 40, "Mass:    %s amu", e.mass);
        rb->lcd_putsxy(sx, ty, buf);
        rb->snprintf(buf, 40, "Density: %s g/%s", e.density, CUBIC_CENTIMETER);
        rb->lcd_putsxy(sx, ty + font_height, buf);
        rb->snprintf(buf, 40, "F Point: %s %s", e.fpoint, CELSIUS);
        rb->lcd_putsxy(sx, ty + font_height*2, buf);
        rb->snprintf(buf, 40, "B Point: %s %s", e.bpoint, CELSIUS);
        rb->lcd_putsxy(sx, ty + font_height*3, buf);
    }
#endif
}

enum plugin_status plugin_start(const void* parameter)
{
    /* if you don't use the parameter, you can do like
       this to avoid the compiler warning about it */
    (void)parameter;

    periodic_sel = 0;
    periodic_makecols();

    struct font *pf = rb->font_get(FONT_UI);
    font_height = pf->height;

    while (1) {
        rb->lcd_clear_display();
        periodic_draw();
        rb->lcd_update();
        int button = rb->button_get(true);
        switch (button)
        {
            case PERIODIC_KEY_SELECT:
                break;
            case PERIODIC_KEY_MENU:
                return PLUGIN_OK;
                break;
            case PERIODIC_KEY_BACK:
            case (PERIODIC_KEY_BACK|BUTTON_REPEAT):
                if (periodic_sel > 0)
                {
                    periodic_sel--;
                }
                else
                {
                    periodic_sel = ELEMENTS_COUNT - 1;
                }
                break;
            case PERIODIC_KEY_FORWARD:
            case (PERIODIC_KEY_FORWARD|BUTTON_REPEAT):
                if (periodic_sel < ELEMENTS_COUNT - 1)
                {
                    periodic_sel++;
                }
                else
                {
                    periodic_sel = 0;
                }
                break;
            default:
                if (rb->default_event_handler(button) == SYS_USB_CONNECTED) {
                    return PLUGIN_USB_CONNECTED;
                }
                break;
        }
    }
    return PLUGIN_OK;
}
