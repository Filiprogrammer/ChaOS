#include "os.h"
#include "list.h"
#include "fileManager.h"
#include "task.h"
#include "math.h"
#include "string.h"

#define WINCOLSELECTED 0xFF2D3048
#define WINCOLUNSELECTED 0xEE494F76
#define WINTXTCOLSELECTED 0xFFFFFFFF
#define WINTXTCOLUNSELECTED 0xFFCCCCCC
#define WINBGCOL 0xFF777777
#define TASKBARHEIGHT 20
#define CLOSEBUTTONCOL 0xFFDD0022

struct window;
struct button;
typedef struct button {
    int16_t x;
    int16_t y;      // Position is relative to the container
    uint16_t width;
    uint16_t height;
    bool isDown;
    bool isEnabled;
    char* text;
    struct window* container;
    uint32_t color;
    uint32_t textColor;
    void (*action) (struct button* button, bool clicked);
} button_t;

typedef struct window {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t titleBarHeight;
    listHead_t* buttons;
    char* title;
    uint32_t bgColor;
} window_t;

struct taskbar {
    uint32_t color;
    listHead_t* items;
    size_t itemListSize;
    uint16_t height;
};

typedef struct taskbar_item {
    char* title;
    uint8_t* icon;
} taskbar_item_t;

static int16_t mouse_x;
static int16_t mouse_y;
static listHead_t* windowList;
static window_t* windowSelected = NULL;
static window_t* windowDrag = NULL;
static size_t windowListSize = 0;
static button_t* buttonDown = NULL;
static struct taskbar taskbar = {.color = 0xE0111111, .items = NULL, .itemListSize = 0, .height = TASKBARHEIGHT};

/*static const uint32_t ahjo[] = {32, 32,
0,0,0,0,0,0,291448871,1215248178,2122993475,-1734451377,-1599970479,-1751623094,2021738044,1113992748,207431206,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,23418697,259022931,242112847,-1869391797,-709396382,-206277539,-39360944,-23044536,-40084666,-241016500,-811112623,2123321667,308686123,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
23552076,443703893,1517577046,-1652131991,-947094160,-996571017,-207064483,-6924476,-7715789,-7257295,-7257808,-7388881,-7388110,-23177407,-390959531,2072989762,224471080,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
-2138408339,-762084234,-258109822,-87310934,-21320290,-20531548,-6519178,-7907769,-7848401,-8570835,-8899028,-8308693,-7718102,-7520466,-22454200,-458463662,1753168696,140387878,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
-21189219,-3424595,-39347828,-90600834,-75140764,-7308180,-5069930,-5267052,-7704733,-9223366,-9685714,-9620435,-7587029,-7652566,-6598854,-4559273,-560445117,1383740468,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
-392656514,-745833618,-1719899298,1315919692,-425891755,-4422297,-7510184,-5992319,-4214625,-6781577,-9289158,-8702163,-7259092,-7455957,-6730697,-4692144,-5814208,-779339720,74066988,0,0,0,0,0,0,0,0,0,0,0,0,0,
1148412757,376528978,40852560,1012802853,-20409996,-6265523,-5880013,-6859452,-6653075,-6979725,-8500673,-7258578,-6996436,-7193299,-6204867,-4627123,-6213077,-6672336,930361135,23275046,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,1298015781,-2842755,-6002609,-5421518,-5683919,-6731207,-7516866,-6995406,-6537170,-6799571,-6667215,-4888497,-4956856,-7202796,-7005927,-1182452941,728444205,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,962471462,-20475020,-3962775,-5224654,-5421518,-5683919,-5946320,-6143184,-6274513,-6405328,-5152696,-3900320,-6535366,-8253167,-7599092,-23646674,-1300090318,677981999,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,291317286,-459051429,-1987450,-5224140,-5223626,-5420747,-5617612,-5880012,-5879241,-4626353,-3307927,-4559785,-7917523,-8773857,-8252139,-6939363,-40488655,-1450820291,560543286,0,0,0,0,0,0,0,0,0,0,
0,0,0,22553122,1920747070,-70610317,-3969710,-4762820,-5223883,-5289161,-4694200,-3441053,-2584462,-3311008,-5553353,-7526618,-8575964,-8905186,-7730164,-7071463,-174836941,-1736495056,408954920,0,0,0,0,0,0,0,0,0,
0,0,0,0,274935082,-1903209659,-1135738,-1597314,-1993354,-2124682,-1992070,-1860740,-1866645,-3122620,-4636366,-6148055,-7527132,-8576478,-8450032,-7665656,-7070178,-275699415,-1921307347,274671399,0,0,0,0,0,0,0,0,
0,0,0,0,0,308949036,-138588105,-1409700,-1076878,-1143181,-1144975,-883594,-1016465,-2004905,-3256003,-4636367,-6148055,-7527132,-9036515,-8450032,-7664371,-7072234,-326162135,2071342124,190719783,0,0,0,0,0,0,0,
0,0,0,0,0,0,1988306974,-306695394,-18591948,-1220537,-1086890,-887449,-754060,-1148563,-2070699,-3256003,-4636367,-6148311,-8641757,-9102051,-8317675,-7665656,-7072234,-443865305,1718889002,157492774,0,0,0,0,0,0,
0,0,0,0,0,0,241182246,1870800926,-524996325,-35698386,-1352894,-1153454,-1019292,-885903,-1214869,-2071212,-3255746,-4636366,-7461596,-8641757,-8903644,-8384496,-7665142,-7203049,-628743132,1416767528,140846887,0,0,0,0,0,
0,0,0,0,0,0,0,190719525,1719478560,-692965863,-52673238,-1485252,-1285555,-1085855,-1018001,-1346455,-2137005,-3255746,-6082519,-7461596,-8509400,-8970979,-8384496,-7664885,-7268842,-813554909,1215309607,90515239,0,0,0,0,
0,0,0,0,0,0,0,0,123610662,1534601249,-827577062,-36159451,-1617609,-1352118,-1151905,-1084051,-1412505,-2202797,-4570574,-6082519,-7395032,-8576221,-8905700,-8384496,-7664885,-7399914,-964680925,1013655078,0,0,0,0,
0,0,0,0,0,0,0,0,0,89990951,1282746401,-979096551,-3000034,-1684686,-1484217,-1218468,-1084565,-1412761,-3255746,-4570574,-6080721,-7461340,-8510685,-8840164,-8449775,-7664885,-24242666,-1082318045,794896167,0,0,0,
0,0,0,0,0,0,0,0,0,0,56567334,980494370,-1214567910,-20171237,-1816530,-1550525,-1218725,-1084565,-2137004,-3255746,-4700617,-6082262,-7395804,-8445149,-8905443,-8449775,-7730678,-41085675,-1132846044,677389862,0,0,
0,0,0,0,0,0,0,0,0,0,0,22815780,712189986,-1466488293,-54119397,-1882837,-1550526,-1218725,-1346710,-2136748,-3320511,-4505038,-6016726,-7395804,-8510685,-8905443,-8449775,-7730935,-57928684,-1351146974,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,23537187,578037537,-1701696996,-88002536,-1949145,-1550526,-1084050,-1281173,-2464425,-3255746,-4505038,-6016726,-7395804,-8510685,-8970722,-8449775,-7731449,-158658030,358557222,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,23143201,426059300,-1886836707,-138530791,-1948631,-1218725,-1017999,-1673876,-2203054,-3190210,-4504782,-6016727,-7396060,-8575964,-9036258,-8515311,-7731449,-1938873565,207562535,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,341780520,-2021185251,-306434024,-1551038,-1218726,-1214865,-1412504,-2268848,-3190210,-4505038,-6016727,-7330268,-8575964,-9036258,-8449775,-578677740,1415979811,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,308225830,2139367709,-19054554,-1551297,-1545634,-1084308,-1544090,-2334640,-3190210,-4505037,-5951191,-7330268,-8575964,-8970722,-226554097,1953372955,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,257893926,-541709291,-2475233,-1944512,-1284519,-1150358,-1610653,-2334896,-3190209,-4439501,-5951191,-7330268,-8510685,-126478312,2137857052,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1702504737,-643028716,-3000290,-1815755,-1416876,-1348507,-1808288,-2334896,-3189953,-4439245,-5950934,-7330268,-193847007,2003313186,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,90055973,1366566945,-861589731,-3198954,-1948883,-1615540,-1480862,-1742495,-2334640,-3189953,-4438989,-5885398,-528997340,1499735333,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,56435748,1115303978,-1130419944,-20238314,-2080470,-1681590,-1480606,-1742494,-2334640,-3124417,-4438989,-1367791834,560014886,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,73805612,863578144,-1365563111,-54055145,-2080726,-1681846,-1480349,-1676701,-2334640,-20295362,1164650535,56501541,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40314917,695347234,-1500239589,-87937257,-2211799,-1681846,-1480091,-3186855,-678084808,73410343,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,408888613,1919887139,-913822685,-644861397,-1233177806,1734551849,291250979,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,39131931,73278757,129150381,74214508,0,0,0
};*/

/*static const uint32_t mcursor_pixels[] = {18, 25,
-16777216,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-12657665,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-10951681,-11875073,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-9633793,-14632197,-12139018,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-11864065,-262145,-14961168,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-13375233,-1,-16729350,-11486997,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-12001281,-4917249,-854017,-14041356,-11486996,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-11739905,-9048321,-1312513,-13777925,-11156491,-11486996,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-8988161,-11147268,-5839105,-11876611,-11024132,-11156491,-11486996,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-9513729,-10819846,-8068865,-11483139,-10958082,-11024132,-11156491,-11486996,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-10104833,-10819590,-7740929,-11483138,-10958338,-10958082,-11024132,-11156491,-11486996,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-10433540,-9249026,-8855041,-11286530,-10958082,-10958338,-10958082,-11024132,-11156491,-11486996,-12139790,-16777216,16777215,16777215,16777215,16777215,16777215,
-16777216,-10958599,-9577475,-9183489,-11286274,-10958082,-10958082,-10958338,-10958082,-11024132,-11156491,-11486996,-12139790,-16777216,16777215,16777215,16777215,16777215,
-16777216,-11746057,-10364675,-9577217,-11155202,-10958338,-10958082,-10958082,-10958082,-10958083,-11024133,-11156235,-11486996,-12139790,-16777216,16777215,16777215,16777215,
-16777216,-12534026,-11020035,-10102785,-11024130,-10958339,-11155204,-11024134,-10959113,-10828298,-10959371,-10959370,-10828813,-11158805,-11746319,-16777216,16777215,16777215,
-16777216,-13582859,-11741699,-11282177,-11024388,-11024647,-11156491,-11157265,-14040321,-14040065,-16727297,-16727554,-16728325,-16730374,-16732677,-14636801,-16777216,16777215,
-16777216,-13191179,-16723460,-10434305,-11024904,-11288335,-11355671,-11620642,-13785351,-13587974,-13587205,-13587202,-13587972,-13785092,-14047233,-14309889,-16739655,-16777216,
-16777216,-13585163,-10430724,-16724225,-11026706,-11619102,-12147752,-12998691,-16777216,-16777216,-16777216,-16777216,-16777216,-16777216,-16777216,-16777216,-16777216,-16777216,
-16777216,-13389066,-16724739,-16727809,-11291938,-12148525,-12868906,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-13389577,-16726785,-16731649,-12017715,-13197357,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-13587464,-16731393,-16736257,-12869427,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-14048001,-16736513,-12546070,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-10378043,-16737585,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-16739643,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,
-16777216,-16777216,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215,16777215};*/

static const uint8_t exit_button_pixels[] = {0x00, 0x10, 0x00, 0x10, 0x00,0x20,0xB2,0xB6,0xB6,0xD6,0xD6,0xD6,0xD6,0xD6,0xD6,0xD6,0xD6,0xB6,0x20,0x00,
    0x20,0xB2,0xB2,0xB2,0xB2,0xB6,0xB6,0xD6,0xD6,0xD6,0xB6,0xB6,0xB6,0xB2,0x92,0x44,
    0x8D,0x92,0xFF,0xFF,0x91,0xB2,0xB2,0xD2,0xD6,0xD6,0xB2,0xB2,0x91,0xFF,0xFF,0x8D,
    0x8D,0x8D,0xFF,0xFF,0xFF,0x8D,0xB1,0xB2,0xD2,0xB2,0xB2,0x8D,0xFF,0xFF,0xFF,0x6D,
    0x6D,0x8D,0x8D,0xFF,0xFF,0xFF,0x8D,0xAD,0xB1,0xAD,0xAD,0xFF,0xFF,0xFF,0x69,0x6D,
    0x44,0x64,0x64,0x64,0xFF,0xFF,0xFF,0x89,0x89,0x89,0xFF,0xFF,0xFF,0x64,0x64,0x44,
    0x44,0x64,0x84,0x89,0x89,0xFF,0xFF,0xFF,0x64,0xFF,0xFF,0xFF,0x64,0x64,0x64,0x44,
    0x44,0x64,0x89,0x89,0x89,0x89,0xFF,0xFF,0xFF,0xFF,0xFF,0x84,0x89,0x89,0x64,0x44,
    0x44,0x64,0x89,0xA9,0xA9,0x89,0x64,0xFF,0xFF,0xFF,0x64,0x89,0x89,0x89,0x64,0x44,
    0x44,0x64,0x89,0xA9,0xA9,0x89,0xFF,0xFF,0xFF,0xFF,0xFF,0x89,0x89,0x89,0x64,0x44,
    0x44,0x64,0x84,0x89,0x89,0xFF,0xFF,0xFF,0x64,0xFF,0xFF,0xFF,0x84,0x84,0x64,0x44,
    0x44,0x64,0x64,0x84,0xFF,0xFF,0xFF,0x84,0x89,0x89,0xFF,0xFF,0xFF,0x64,0x64,0x44,
    0x44,0x44,0x64,0xFF,0xFF,0xFF,0x64,0x89,0x89,0x89,0x84,0xFF,0xFF,0xFF,0x44,0x44,
    0x20,0x44,0xFF,0xFF,0xFF,0x64,0x64,0x89,0x89,0x89,0x84,0x64,0xFF,0xFF,0xFF,0x20,
    0x20,0x20,0xFF,0xFF,0x44,0x64,0x64,0x64,0x64,0x64,0x64,0x64,0x44,0xFF,0xFF,0x20,
    0x00,0x20,0x20,0x20,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x20,0x20,0x00};

//static uint8_t wallpaper_pixels[4+320*200];
uint8_t* wallpaper_pixels = NULL;
uint32_t* mcursor_pixels = NULL;

void desktop_update(){
    // Desktop Wallpaper
    if(wallpaper_pixels) draw_picture_part(wallpaper_pixels, 0, 0, 0, 0, 320, 200);
    
    for(uint32_t i = 1; i <= windowListSize; ++i){
        window_t* window = list_getElement(windowList, i);
        if((i == windowListSize) && windowSelected){
            setgraphcolor(WINCOLSELECTED);
            draw_rectangle(window->x, window->y, window->width, window->titleBarHeight);
            setgraphcolor(WINTXTCOLSELECTED);
        }else{
            setgraphcolor(WINCOLUNSELECTED);
            draw_rectangle(window->x, window->y, window->width, window->titleBarHeight);
            setgraphcolor(WINTXTCOLUNSELECTED);
        }
        draw_text_part(window->x+4, window->y+2, window->title, 0, 0, 320, 200);
        setgraphcolor(window->bgColor);
        draw_rectangle(window->x, window->y + window->titleBarHeight, window->width, window->height);
        size_t buttonListSize = list_getSize(window->buttons);
        for(uint32_t j = 1; j <= buttonListSize; ++j) {
            button_t* button = list_getElement(window->buttons, j);
            if(button->isDown){
                uint8_t a = button->color >> 24;
                uint8_t r = button->color >> 16;
                uint8_t g = button->color >> 8;
                uint8_t b = button->color;
                setgraphcolor((a << 24) | ((r+128) << 16) | ((g+128) << 8) | (b+128));
            } else {
                setgraphcolor(button->color);
            }
            int16_t tmpX = MAX(window->x + button->x, window->x);
            int16_t tmpY = MAX(window->y + window->titleBarHeight + button->y, window->y);
            draw_rectangle(tmpX, tmpY,
            MAX((int16_t)MIN(button->x + button->width,  window->width ) - (int16_t)(tmpX - window->x                         ), 0),
            MAX((int16_t)MIN(button->y + button->height, window->height) - (int16_t)(tmpY - window->y - window->titleBarHeight), 0));
            if(button->text){
                if(button->isDown){
                    uint8_t a = button->textColor >> 24;
                    uint8_t r = button->textColor >> 16;
                    uint8_t g = button->textColor >> 8;
                    uint8_t b = button->textColor;
                    setgraphcolor((a << 24) | ((r+128) << 16) | ((g+128) << 8) | (b+128));
                } else {
                    setgraphcolor(button->textColor);
                }
                draw_text_part(window->x+button->x, window->y+window->titleBarHeight+button->y, button->text,
                               window->x+button->x, window->y+window->titleBarHeight+button->y, button->width, button->height);
            }
        }
    }
    // Draw taskbar
    setgraphcolor(taskbar.color);
    draw_rectangle(0, 200-taskbar.height, 320, taskbar.height);
    
    for(uint32_t i = 1; i <= taskbar.itemListSize; ++i){
        taskbar_item_t* item = list_getElement(taskbar.items, i);
        //setgraphcolor(0xFF000000 | random());
        if(item->icon){
            draw_picture_part(item->icon, (i-1)*taskbar.height, 200-taskbar.height, 0, 0, TASKBARHEIGHT, TASKBARHEIGHT);
        } else {
            setgraphcolor(0xFF777777);
            draw_rectangle((i-1)*taskbar.height, 200-taskbar.height, taskbar.height, taskbar.height);
        }
        //draw_rectangle((i-1)*taskbar.height, 200-taskbar.height, taskbar.height, taskbar.height);
    }
    // Draw mouse cursor
    if(mcursor_pixels) draw_picture32(mcursor_pixels, mouse_x, mouse_y);
}

void desktop_update_part(int16_t x, int16_t y, uint16_t width, uint16_t height, bool drawWallpaper){
    if(drawWallpaper && wallpaper_pixels){
        draw_picture_part(wallpaper_pixels, 0, 0, x, y, width, height);
    }
    
    for(uint32_t i = 1; i <= windowListSize; ++i){
        window_t* window = list_getElement(windowList, i);
        if(((window->x + window->width) >= x) && (window->x <= (x + width))){
            if(((window->y + window->titleBarHeight + window->height) >= y) && (window->y <= (y + height))){
                //Fenster befindet sich im Bereich, der gezeichnet werden soll
                if(y < (window->y + window->titleBarHeight)){
                    //Titelbalken befindet sich im Bereich, der gezeichnet werden soll
                    if((i == windowListSize) && windowSelected){
                        setgraphcolor(WINCOLSELECTED);
                        draw_rectangle(MAX(window->x, x),
                        MAX(window->y, y),
                        MIN(window->x + window->width, x + width) - MAX(window->x, x),
                        MIN(window->y + window->titleBarHeight, y + height) - MAX(window->y, y));
                        setgraphcolor(WINTXTCOLSELECTED);
                    }else{
                        setgraphcolor(WINCOLUNSELECTED);
                        draw_rectangle(MAX(window->x, x),
                        MAX(window->y, y),
                        MIN(window->x + window->width, x + width) - MAX(window->x, x),
                        MIN(window->y + window->titleBarHeight, y + height) - MAX(window->y, y));
                        setgraphcolor(WINTXTCOLUNSELECTED);
                    }
                    draw_text_part(window->x + 4, window->y + 2, window->title,
                        MAX(window->x + 4, x), MAX(window->y + 2, y),
                        MAX(x + width - MAX(window->x + 4, x), 0), MAX(y + height - MAX(window->y + 2, y), 0));
                }
                
                if(y + height >= (window->y + window->titleBarHeight)){
                    setgraphcolor(window->bgColor);
                    draw_rectangle(MAX(window->x, x),
                    MAX(window->y + window->titleBarHeight, y),
                    MIN(window->x + window->width, x + width) - MAX(window->x, x),
                    MIN(window->y + window->titleBarHeight + window->height, y + height) - MAX(window->y + window->titleBarHeight, y));
                }
                
                size_t buttonListSize = list_getSize(window->buttons);
                for(uint32_t j = 1; j <= buttonListSize; ++j) {
                    button_t* button = list_getElement(window->buttons, j);
                    if(x < (window->x + button->x + button->width)
                    && (x + width) >= (window->x + button->x)
                    && y < (window->y + window->titleBarHeight + button->y + button->height)
                    && (y + height) >= (window->y + window->titleBarHeight + button->y)) {
                        if(button->isDown){
                            uint8_t a = button->color >> 24;
                            uint8_t r = button->color >> 16;
                            uint8_t g = button->color >> 8;
                            uint8_t b = button->color;
                            setgraphcolor((a << 24) | ((r+128) << 16) | ((g+128) << 8) | (b+128));
                        } else {
                            setgraphcolor(button->color);
                        }
                        int16_t tmpX = MAX(window->x + button->x, MAX(window->x, x));
                        int16_t tmpY = MAX(window->y + window->titleBarHeight + button->y, MAX(window->y, y));
                        
                        int16_t tmpW =
                        MAX(
                            (int16_t)MIN(
                                window->x+MIN(
                                    button->x + button->width,
                                    window->width
                                ),
                                x + width
                            ) - (int16_t)tmpX,
                            0
                        );
                        int16_t tmpH =
                        MAX(
                            (int16_t)MIN(
                                window->y+window->titleBarHeight+MIN(
                                    button->y + button->height,
                                    window->height
                                ),
                                y + height
                            ) - (int16_t)tmpY,
                            0
                        );
                        
                        draw_rectangle(tmpX, tmpY, tmpW, tmpH);
                        if(button->text){
                            if(button->isDown){
                                uint8_t a = button->textColor >> 24;
                                uint8_t r = button->textColor >> 16;
                                uint8_t g = button->textColor >> 8;
                                uint8_t b = button->textColor;
                                setgraphcolor((a << 24) | ((r+128) << 16) | ((g+128) << 8) | (b+128));
                            } else {
                                setgraphcolor(button->textColor);
                            }
                            draw_text_part(window->x+button->x, window->y+window->titleBarHeight+button->y, button->text,
                                           tmpX, tmpY, tmpW, tmpH);
                        }
                    }
                }
            }
        }
    }
    
    if((y+height) > 200-taskbar.height){
        // Draw taskbar
        setgraphcolor(taskbar.color);
        draw_rectangle(x, MAX(200-taskbar.height, y), width, (y+height)-(200-taskbar.height));
        
        for(uint32_t i = 1; i <= taskbar.itemListSize; ++i){
            taskbar_item_t* item = list_getElement(taskbar.items, i);
            if(x+width >= (int16_t)((i-1)*taskbar.height) && x < (int16_t)(i*taskbar.height)){
                int16_t tmpX = MAX((i-1)*taskbar.height, x);
                int16_t tmpY = MAX(200-taskbar.height, y);
                if(item->icon){
                    draw_picture_part(item->icon, (i-1)*taskbar.height, 200-taskbar.height,
                    tmpX-(i-1)*taskbar.height, tmpY-(200-taskbar.height),
                    MIN(i*taskbar.height, x+width)-tmpX, MIN(200, y+height)-tmpY);
                } else {
                    setgraphcolor(0xFF777777);
                    draw_rectangle(tmpX, tmpY,
                    MIN(i*taskbar.height, x+width)-tmpX, MIN(200, y+height)-tmpY);
                }
                //setgraphcolor(0xFF000000 | random());
                //draw_rectangle(tmpX, tmpY,
                //MIN(i*taskbar.height, x+width)-tmpX, MIN(200, y+height)-tmpY);
            }
        }
    }
    
    if(mcursor_pixels){
        if(((mouse_x+18) >= x) && (mouse_x <= (x+width))){
            if(((mouse_y+25) >= y) && (mouse_y <= (y+height))){
                // Draw mouse cursor
                draw_picture_part32(mcursor_pixels, mouse_x, mouse_y, x-mouse_x, y-mouse_y, width, height);
            }
        }
    }
}

taskbar_item_t* desktop_taskbar_item_create(char* title){
    taskbar_item_t* item = malloc(sizeof(taskbar_item_t), 0);
    if(title){
        size_t title_len = strlen(title);
        item->title = malloc(title_len+1, 0);
        strcpy(item->title, title);
        item->title[title_len] = 0;
    } else {
        item->title = NULL;
    }
    item->icon = NULL;
    /*item->icon[0] = TASKBARHEIGHT >> 8;
    item->icon[1] = TASKBARHEIGHT;
    item->icon[2] = TASKBARHEIGHT >> 8;
    item->icon[3] = TASKBARHEIGHT;
    for(uint16_t i = 4+TASKBARHEIGHT; i < TASKBARHEIGHT*(TASKBARHEIGHT-1)+4; ++i){
        item->icon[i] = 0x20;
    }
    for(uint16_t i = 4; i < TASKBARHEIGHT+4; ++i){
        item->icon[i] = 0xFF;
        item->icon[TASKBARHEIGHT*(TASKBARHEIGHT-1)+i] = 0xFF;
        item->icon[4+TASKBARHEIGHT*(i-4)] = 0xFF;
        item->icon[3+TASKBARHEIGHT+TASKBARHEIGHT*(i-4)] = 0xFF;
    }*/
    list_append(taskbar.items, item);
    ++taskbar.itemListSize;
    return item;
}

button_t* desktop_button_create(window_t* window, int16_t x, int16_t y, uint16_t width, uint16_t height, char* text) {
    if(window == NULL) return NULL;
    button_t* button = malloc(sizeof(button_t), 0);
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    if(text) {
        size_t text_len = strlen(text);
        button->text = malloc(text_len+1, 0);
        strcpy(button->text, text);
        button->text[text_len] = 0;
    } else {
        button->text = NULL;
    }
    button->isDown = false;
    button->isEnabled = true;
    button->container = window;
    button->color = 0xFFAAAAAA;
    button->textColor = 0xFF111111;
    button->action = NULL;
    list_append(window->buttons, button);
    return button;
}

void desktop_button_destroy(button_t* button) {
    if(button == NULL) return;
    list_delete(button->container->buttons, button);
    if(button->text) free(button->text);
    free(button);
}

void desktop_window_destroy(window_t* window){
    if(window == NULL) return;
    if(window->title) free(window->title);
    free(window);
    list_delete(windowList, window);
    list_deleteAll(window->buttons);
    --windowListSize;
    windowSelected = list_getElement(windowList, 1);
    desktop_update();
}

void desktop_exit_button_handler(button_t* button, bool clicked){
    if(!clicked) desktop_window_destroy(button->container);
}

window_t* desktop_window_create(int16_t x, int16_t y, uint16_t width, uint16_t height, char* title){
    window_t* new_window = malloc(sizeof(window_t), 0);
    new_window->x = x;
    new_window->y = y;
    new_window->width = width;
    new_window->height = height;
    new_window->titleBarHeight = 20;
    new_window->bgColor = WINBGCOL;
    if(title) {
        size_t title_len = strlen(title);
        new_window->title = malloc(title_len+1, 0);
        strcpy(new_window->title, title);
        new_window->title[title_len] = 0;
    }
    new_window->buttons = list_create();
    list_append(windowList, new_window);
    ++windowListSize;
    button_t* button = desktop_button_create(new_window, width-18, -new_window->titleBarHeight+2, 16, 16, NULL);
    button->color = CLOSEBUTTONCOL;
    //uint8_t r = button->color >> 16;
    //uint8_t g = button->color >> 8;
    //uint8_t b = button->color;
    //button->textColor = 0xFF000000 | ((r+128) << 16) | ((g+128) << 8) | (b+128);
    button->action = &desktop_exit_button_handler;
    return new_window;
}

static void desktop_window_to_foreground(window_t* window){
    uint32_t i = list_findElement(windowList, window);
    if(!i) return; //Element was not found
    if(i != windowListSize){
        list_delete(windowList, window);
        list_append(windowList, window);
    }
}

static void onmouse_move(int16_t x, int16_t y){
    int16_t mouse_x_prev = mouse_x;
    int16_t mouse_y_prev = mouse_y;
    mouse_x = x;
    mouse_y = y;
    if(windowDrag){
        windowDrag->x += (mouse_x-mouse_x_prev);
        windowDrag->y += (mouse_y-mouse_y_prev);
        desktop_update_part(windowDrag->x-(mouse_x-mouse_x_prev), windowDrag->y-(mouse_y-mouse_y_prev), windowDrag->width, windowDrag->height+windowDrag->titleBarHeight, true);
        desktop_update_part(windowDrag->x, windowDrag->y, windowDrag->width, windowDrag->height+windowDrag->titleBarHeight, false);
    }
    desktop_update_part(mouse_x_prev, mouse_y_prev, 18, 25, true);
    desktop_update_part(mouse_x, mouse_y, 18, 25, false);
}

static void onmouse_click(uint8_t btn, bool pressed){
    window_t* windowSelectedPrev = windowSelected;
    
    if(!pressed && btn == 0 && buttonDown){
        buttonDown->isDown = false;
        desktop_update_part(buttonDown->container->x + buttonDown->x, buttonDown->container->y + buttonDown->container->titleBarHeight + buttonDown->y, buttonDown->width, buttonDown->height, false);
        desktop_update_part(mouse_x, mouse_y, 18, 25, false);
        if(buttonDown->action){
            if((mouse_x >= buttonDown->container->x+buttonDown->x) && (mouse_x < buttonDown->container->x+buttonDown->x+buttonDown->width)
            && (mouse_y >= buttonDown->container->y+buttonDown->container->titleBarHeight+buttonDown->y) && (mouse_y < buttonDown->container->y+buttonDown->container->titleBarHeight+buttonDown->y+buttonDown->height)) {
                buttonDown->action(buttonDown, false);
            }
        }
        buttonDown = NULL;
    }
    
    if(mouse_y < 200-taskbar.height){
        
        for(size_t i = windowListSize; i >= 1; --i) {
            window_t* window = list_getElement(windowList, i);
            if((mouse_x >= window->x) && (mouse_x < (window->x+window->width))
            && (mouse_y >= window->y) && (mouse_y < (window->y+window->titleBarHeight+window->height))) {
                
                if(pressed){
                    windowSelected = window;
                    
                    if(windowSelected) {
                        desktop_window_to_foreground(windowSelected);
                        //Drag window
                        if(btn == 0 && mouse_y < window->y+window->titleBarHeight){ //if primary button has been pressed
                            windowDrag = windowSelected;
                            mouse_set_bounds(319, 199-taskbar.height);
                        }
                    }
                    
                    if(btn == 0){
                        size_t buttonListSize = list_getSize(window->buttons);
                        for(uint32_t j = 1; j <= buttonListSize; ++j) {
                            button_t* button = list_getElement(window->buttons, j);
                            
                            if(mouse_x < (window->x + button->x + button->width)
                            && mouse_x >= (window->x + button->x)
                            && mouse_y < (window->y + window->titleBarHeight + button->y + button->height)
                            && mouse_y + 25 >= (window->y + window->titleBarHeight + button->y)) {
                                button->isDown = true;
                                buttonDown = button;
                                windowDrag = NULL;
                                mouse_set_bounds(319, 199);
                                if(button->action) button->action(button, true);
                                break;
                            }
                        }
                    }
                    
                    if(windowSelected != windowSelectedPrev) desktop_update();
                    else {
                        desktop_update_part(window->x, window->y, window->width, window->titleBarHeight + window->height, false);
                    }
                } else {
                    if(btn == 0){ //if primary button has been released
                        windowDrag = NULL;
                        mouse_set_bounds(319, 199);
                    }
                }
                
                return;
            }
        }
        
    }
    
    if(pressed){
        windowSelected = NULL;
        if(windowSelectedPrev) desktop_update();
    }
}

inline uint8_t colorToRRRGGGBB(uint32_t rgb){
    uint8_t r = rgb >> 16;
    uint8_t g = rgb >> 8;
    uint8_t b = rgb;
    return (r & 0xE0) | ((g & 0xE0) >> 3) | ((b & 0xC0) >> 6);
}

uint32_t* readBMP32(char* filepath) {
    file_t file_inst = {{0}, {0}, 0, 0};
    if(!file_find(&file_inst, filepath)) return NULL;
    if(file_inst.size > 0x100000) return NULL; // Less than 16 MiB
    
    uint8_t* file_contents = malloc(file_inst.size, 0);
    file_readContents(&file_inst, file_contents, 0, file_inst.size);
    
    if(file_contents[0] != 'B' || file_contents[1] != 'M') return NULL;
    
    uint32_t bfOffBits = file_contents[10] | (file_contents[11] << 8) | (file_contents[12] << 16) | (file_contents[13] << 24);
    int32_t biWidth = file_contents[18] | (file_contents[19] << 8) | (file_contents[20] << 16) | (file_contents[21] << 24);
    int32_t biHeight = file_contents[22] | (file_contents[23] << 8) | (file_contents[24] << 16) | (file_contents[25] << 24);
    uint16_t biBitCount = file_contents[28] | (file_contents[29] << 8);
    
    if(biWidth*abs(biHeight) > 0x100000) return NULL; // Limit resolution
    
    uint32_t* picture = malloc(2+4*biWidth*abs(biHeight), 0);
    
    picture[0] = biWidth;
    picture[1] = abs(biHeight);
    
    if (biBitCount == 24) {
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                int32_t n = y * ((biWidth*3 + 3) & ~3);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = 3*x + n;
                    picture[2+x+((biHeight-y-1)*biWidth)] = file_contents[bfOffBits + m] | (file_contents[bfOffBits + m + 1] << 8) | (file_contents[bfOffBits + m + 2] << 16) | 0xFF000000;
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                int32_t n = y * ((biWidth*3 + 3) & ~3);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = 3*x + n;
                    picture[2+x+y*biWidth] = file_contents[bfOffBits + m] | (file_contents[bfOffBits + m + 1] << 8) | (file_contents[bfOffBits + m + 2] << 16) | 0xFF000000;
                }
            }
        }
    } else if (biBitCount == 8) {
        uint32_t colorTable[256];
        for(uint16_t i = 0; i < 256; ++i){
            colorTable[i] = file_contents[0x36 + i*4] | (file_contents[0x37 + i*4] << 8) | (file_contents[0x38 + i*4] << 16) | (file_contents[0x39 + i*4] << 24);
        }
        
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    picture[2+x+((biHeight-y-1)*biWidth)] = colorTable[file_contents[bfOffBits + x + y * biWidth]];
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    picture[2+x+y*biWidth] = colorTable[file_contents[bfOffBits + x + y * biWidth]];
                }
            }
        }
    } else if(biBitCount == 4) {
        uint8_t colorTable[16];
        for(uint16_t i = 0; i < 16; ++i){
            colorTable[i] = file_contents[0x36 + i*4] | (file_contents[0x37 + i*4] << 8) | (file_contents[0x38 + i*4] << 16) | (file_contents[0x39 + i*4] << 24);
        }
        
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t n = x + y * biWidth;
                    uint8_t b = file_contents[bfOffBits + n/2];
                    picture[2+x+((biHeight-y-1)*biWidth)] = colorTable[(b >> (((n+1)%2)*4)) & 0x0F];
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t n = x + y * biWidth;
                    uint8_t b = file_contents[bfOffBits + n/2];
                    picture[2+x+y*biWidth] = colorTable[(b >> (((n+1)%2)*4)) & 0x0F];
                }
            }
        }
    } else if (biBitCount == 1) {
        uint8_t colorTable[2];
        for(uint16_t i = 0; i < 2; ++i){
            colorTable[i] = file_contents[0x36 + i*4] | (file_contents[0x37 + i*4] << 8) | (file_contents[0x38 + i*4] << 16) | (file_contents[0x39 + i*4] << 24);
        }
        
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                int32_t n = y * ((biWidth + 31) & ~31);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = x + n;
                    uint8_t b = file_contents[bfOffBits + m / 8];
                    picture[2+x+((biHeight-y-1)*biWidth)] = colorTable[(b >> (7-(m % 8))) & 0x01];
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                int32_t n = y * ((biWidth + 31) & ~31);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = x + n;
                    uint8_t b = file_contents[bfOffBits + m / 8];
                    picture[2+x+y*biWidth] = colorTable[(b >> (7-(m % 8))) & 0x01];
                }
            }
        }
    }
    return picture;
}

uint8_t* readBMP(char* filepath) {
    file_t file_inst = {{0}, {0}, 0, 0};
    if(!file_find(&file_inst, filepath)) return NULL;
    if(file_inst.size > 0x100000) return NULL; // Less than 16 MiB
    
    uint8_t* file_contents = malloc(file_inst.size, 0);
    file_readContents(&file_inst, file_contents, 0, file_inst.size);
    
    if(file_contents[0] != 'B' || file_contents[1] != 'M') return NULL;
    
    uint32_t bfOffBits = file_contents[10] | (file_contents[11] << 8) | (file_contents[12] << 16) | (file_contents[13] << 24);
    int32_t biWidth = file_contents[18] | (file_contents[19] << 8) | (file_contents[20] << 16) | (file_contents[21] << 24);
    int32_t biHeight = file_contents[22] | (file_contents[23] << 8) | (file_contents[24] << 16) | (file_contents[25] << 24);
    uint16_t biBitCount = file_contents[28] | (file_contents[29] << 8);
    
    if(biWidth*abs(biHeight) > 0x100000) return NULL; // Limit resolution
    
    uint8_t* picture = malloc(4+biWidth*abs(biHeight), 0);
    
    picture[0] = biWidth >> 8;
    picture[1] = biWidth;
    picture[2] = abs(biHeight) >> 8;
    picture[3] = abs(biHeight);
    
    if (biBitCount == 24) {
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                int32_t n = y * ((biWidth + 31) & ~31);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = x + n;
                    picture[4+x+((biHeight-y-1)*biWidth)] = colorToRRRGGGBB(file_contents[bfOffBits + 3*m] | (file_contents[bfOffBits + 3*m + 1] << 8) | (file_contents[bfOffBits + 3*m + 2] << 16) | 0xFF000000);
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                int32_t n = y * ((biWidth + 31) & ~31);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = x + n;
                    picture[4+x+y*biWidth] = colorToRRRGGGBB(file_contents[bfOffBits + 3*m] | (file_contents[bfOffBits + 3*m + 1] << 8) | (file_contents[bfOffBits + 3*m + 2] << 16) | 0xFF000000);
                }
            }
        }
    } else if (biBitCount == 8) {
        uint8_t colorTable[256];
        for(uint16_t i = 0; i < 256; ++i){
            uint8_t b = file_contents[0x36 + i*4];
            uint8_t g = file_contents[0x37 + i*4];
            uint8_t r = file_contents[0x38 + i*4];
            colorTable[i] = (r & 0xE0) | ((g & 0xE0) >> 3) | ((b & 0xC0) >> 6);
        }
        
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    picture[4+x+((biHeight-y-1)*biWidth)] = colorTable[file_contents[bfOffBits + x + y * biWidth]];
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    picture[4+x+y*biWidth] = colorTable[file_contents[bfOffBits + x + y * biWidth]];
                }
            }
        }
    } else if(biBitCount == 4) {
        uint8_t colorTable[16];
        for(uint16_t i = 0; i < 16; ++i){
            uint8_t b = file_contents[0x36 + i*4];
            uint8_t g = file_contents[0x37 + i*4];
            uint8_t r = file_contents[0x38 + i*4];
            colorTable[i] = (r & 0xE0) | ((g & 0xE0) >> 3) | ((b & 0xC0) >> 6);
        }
        
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t n = x + y * biWidth;
                    uint8_t b = file_contents[bfOffBits + n/2];
                    picture[4+x+((biHeight-y-1)*biWidth)] = colorTable[(b >> (((n+1)%2)*4)) & 0x0F];
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t n = x + y * biWidth;
                    uint8_t b = file_contents[bfOffBits + n/2];
                    picture[4+x+y*biWidth] = colorTable[(b >> (((n+1)%2)*4)) & 0x0F];
                }
            }
        }
    } else if (biBitCount == 1) {
        uint8_t colorTable[2];
        for(uint16_t i = 0; i < 2; ++i){
            uint8_t b = file_contents[0x36 + i*4];
            uint8_t g = file_contents[0x37 + i*4];
            uint8_t r = file_contents[0x38 + i*4];
            colorTable[i] = (r & 0xE0) | ((g & 0xE0) >> 3) | ((b & 0xC0) >> 6);
        }
        
        if (biHeight > 0) {
            // bottom-up
            for (int32_t y = biHeight - 1; y >= 0; --y) {
                int32_t n = y * ((biWidth + 31) & ~31);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = x + n;
                    uint8_t b = file_contents[bfOffBits + m / 8];
                    picture[4+x+((biHeight-y-1)*biWidth)] = colorTable[(b >> (7-(m % 8))) & 0x01];
                }
            }
        } else {
            // top-down
            biHeight = -biHeight;
            for (int32_t y = 0; y < biHeight; ++y) {
                int32_t n = y * ((biWidth + 31) & ~31);
                for (int32_t x = 0; x < biWidth; ++x) {
                    int32_t m = x + n;
                    uint8_t b = file_contents[bfOffBits + m / 8];
                    picture[4+x+y*biWidth] = colorTable[(b >> (7-(m % 8))) & 0x01];
                }
            }
        }
    }
    return picture;
}

void desktop_enable(){
    if(video_get_mode() != 2) video_set_mode(2);
    mouse_setup(319, 199, 1);
    mouse_x = 160;
    mouse_y = 100;
    mouse_add_move_listener(&onmouse_move);
    mouse_add_click_listener(&onmouse_click);
    mouse_switch(1);
    if(!windowList) windowList = list_create();
    windowSelected = 0;
    if(!taskbar.items) taskbar.items = list_create();
    
    taskbar_item_t* item = desktop_taskbar_item_create(NULL);
    item->icon = readBMP("~~/STARTICO.BMP");
    
    /*file_t file_inst = {{0}, {0}, 0, 0};
    if(file_find(&file_inst, "~~/WALLPAPR.PIC")) {
        file_readContents(&file_inst, wallpaper_pixels, 0, MIN(file_inst.size, 320*200+4));
    } else {
        puts("Not found\n");
        wallpaper_pixels[0] = 1;
        wallpaper_pixels[1] = 64;
        wallpaper_pixels[2] = 0;
        wallpaper_pixels[3] = 200;
        for(uint32_t i = 4; i < 320*200+4; ++i){
            wallpaper_pixels[i] = (i-4) / 320;
        }
    }*/
    
    wallpaper_pixels = readBMP("~~/WALLPAPR.BMP");
    
    mcursor_pixels = readBMP32("~~/MOUSEPTR.BMP");
    
    desktop_window_create(125, 60, 80, 50, "Hello");
    desktop_window_create(100, 50, 100, 74, "Window 2");
    desktop_update();
    //draw_picture_part32(ahjo, 0, 0, 0, 0, 320, 200);
}
