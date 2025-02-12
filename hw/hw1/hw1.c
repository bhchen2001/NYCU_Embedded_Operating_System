#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h> 

#include "writer.h"

#define RESTAURANT_NUM 3
#define RESTAURANT_MENU_NUM 2

/*
 *  Restaurant Struct
 *      - Name
 *      - Distance
 *      - Menu (including 2 items)
 */

typedef struct {
    char name[20];
    int distance;
    struct {
        char item[20];
        int price;
    } menu[RESTAURANT_MENU_NUM];
} Restaurant;

Restaurant restaurants[RESTAURANT_NUM] = {
    {"Dessert shop", 3, {{"cookie", 60}, {"cake", 80}}},
    {"Beverage shop", 5, {{"tea", 40}, {"boba", 70}}},
    {"Diner", 8, {{"fried rice", 120}, {"egg-drop soup", 50}}}
};

void press_any_key_to_continue() {
    struct termios oldt, newt; 
    tcgetattr(STDIN_FILENO, &oldt); 
    newt = oldt; 
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); 
    getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
} 

void show_main_menu() {
    printf("1. shop list\n");
    printf("2. order\n");
}

void show_shop_list() {
    int i = 0;
    for (i = 0; i < RESTAURANT_NUM; i++) {
        printf("%s: %dkm\n", restaurants[i].name, restaurants[i].distance);
    }
}

void show_restaurant_list() {
    int i = 0;
    printf("Please choose from 1~%d\n", RESTAURANT_NUM);
    for (i = 0; i < RESTAURANT_NUM; i++) {
        printf("%d. %s\n", i+1, restaurants[i].name);
    }
}

void show_restaurant_menu(int restaurant_choice) {
    int i = 0;
    printf("Please choose from 1~4\n");
    for (i = 0; i < RESTAURANT_MENU_NUM; i++) {
        printf("%d. %s: $%d\n", i+1, restaurants[restaurant_choice].menu[i].item, restaurants[restaurant_choice].menu[i].price);
    }
    printf("%d. confirm\n", ++i);
    printf("%d. cancel\n", ++i);
}

int main(int argc, char *argv[]) {
    int total_price = 0;
    int main_menu_choice, restaurant_choice, restaurant_menu_choice;
    char input_buffer[10];

    while(1) {
        show_main_menu();
        fgets(input_buffer, sizeof(input_buffer), stdin);
        sscanf(input_buffer, "%d", &main_menu_choice);

        switch (main_menu_choice) {
        
        /*
         *  1. shop list
         */
        case 1:
            show_shop_list();
            press_any_key_to_continue();
            break;
        
        /*
         *  2. order
         */
        case 2:
            /*
             *  Show the list of restaurants
             */
            show_restaurant_list();
            fgets(input_buffer, sizeof(input_buffer), stdin);
            sscanf(input_buffer, "%d", &restaurant_choice);
            restaurant_choice--;

            /*
             *  Show the menu of the selected restaurant
             */
            while(1) {
                int quantity;
                show_restaurant_menu(restaurant_choice);
                fgets(input_buffer, sizeof(input_buffer), stdin);
                sscanf(input_buffer, "%d", &restaurant_menu_choice);
                restaurant_menu_choice--;

                if (restaurant_menu_choice < RESTAURANT_MENU_NUM && restaurant_menu_choice >= 0) {
                    /*
                     *  Order meals
                     */
                    printf("How many?\n");
                    fgets(input_buffer, sizeof(input_buffer), stdin);
                    sscanf(input_buffer, "%d", &quantity);
                    total_price += restaurants[restaurant_choice].menu[restaurant_menu_choice].price * quantity;
                } else if (restaurant_menu_choice == RESTAURANT_MENU_NUM) {
                    /*
                     *  Confirm
                     *      - starting to deliver
                     *      - activate the led and 7-seg driver
                     */
                    if (total_price == 0) {
                        // printf("[DEBUG] Please order something\n");
                        continue;
                    }
                    printf("Please wait for a few minutes\n");
                    // printf("[DEBUG] Total price: %d\n", total_price);
                    led_writer(restaurants[restaurant_choice].distance);
                    seg_writer(total_price);
                    printf("Please pick up your meal\n");
                    total_price = 0;
                    press_any_key_to_continue();
                    break;
                } else if (restaurant_menu_choice == RESTAURANT_MENU_NUM + 1) {
                    /*
                     *  Cancel
                     */
                    total_price = 0;
                    break;
                } else {
                    // printf("[DEBUG] Invalid choice\n");
                }
            }
        }
    }
}