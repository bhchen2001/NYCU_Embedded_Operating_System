#ifndef ORDER_H

#define RESTAURANT_NUM 3
#define RESTAURANT_MENU_NUM 2
#define BUFFER_SIZE 256

typedef struct {
    char name[20];
    int distance;
    struct {
        char item[20];
        int price;
    } menu[RESTAURANT_MENU_NUM];
} Restaurant;

typedef struct {
    char item[20];
    int quantity;
} Order;

Restaurant restaurants[RESTAURANT_NUM] = {
    {"Dessert shop", 3, {{"cookie", 60}, {"cake", 80}}},
    {"Beverage shop", 5, {{"tea", 40}, {"boba", 70}}},
    {"Diner", 8, {{"fried-rice", 120}, {"Egg-drop-soup", 50}}}
};

void send_shop_list(int client_sockfd);
int process_order(int client_sockfd, char *command, Order *orders, int *current_restaurant);
void clear_orders(Order *orders, int *current_restaurant);
void handle_confirm(int client_sockfd, Order *orders, int current_restaurant);

void send_shop_list(int client_sockfd) {
    char response[BUFFER_SIZE];
    int offset = 0;
    memset(response, 0, BUFFER_SIZE);
    for (int i = 0; i < RESTAURANT_NUM; i++) {
        // append restaurant name and distance
        int n = snprintf(response + offset, BUFFER_SIZE, "%s:%dkm\n", restaurants[i].name, restaurants[i].distance);
        if (n < 0) {
            perror("error writing to buffer");
            exit(1);
        }
        offset += n;

        n = snprintf(response + offset, BUFFER_SIZE, "- %s:$%d|%s:$%d\n", 
                     restaurants[i].menu[0].item, restaurants[i].menu[0].price,
                     restaurants[i].menu[1].item, restaurants[i].menu[1].price);
        if (n < 0) {
            perror("error writing to buffer");
            exit(1);
        }
        offset += n;
    }

    if (send(client_sockfd, response, BUFFER_SIZE, 0) < 0) {
        perror("error sending message");
        exit(1);
    }
}

int process_order(int client_sockfd, char *command, Order *orders, int *current_restaurant) {
    char item[20];
    int quantity;
    char response[BUFFER_SIZE];
    int found_restaurant = -1;

    memset(response, 0, BUFFER_SIZE);

    // parse order command
    if (sscanf(command, "order %s %d", item, &quantity) != 2) {
        return 1;
    }

    // find restaurant that serves the item
    for (int i = 0; i < RESTAURANT_NUM; i++) {
        for (int j = 0; j < RESTAURANT_MENU_NUM; j++) {
            if (strcmp(restaurants[i].menu[j].item, item) == 0) {
                found_restaurant = i;
                break;
            }
        }
        if (found_restaurant != -1) break;
    }

    if (found_restaurant == -1) {
        perror("Item not found");
        return -1;
    }
    else if (*current_restaurant == -1) {
        *current_restaurant = found_restaurant;

        for (int i = 0; i < RESTAURANT_MENU_NUM; i++) {
            strcpy(orders[i].item, restaurants[found_restaurant].menu[i].item);
            orders[i].quantity = 0;
        }

        // update order quantity
        for (int i = 0; i < RESTAURANT_MENU_NUM; i++) {
            if (strcmp(orders[i].item, item) == 0) {
                orders[i].quantity += quantity;
                break;
            }
        }
    }
    else if (*current_restaurant == found_restaurant) {
        // update order quantity
        for (int i = 0; i < RESTAURANT_MENU_NUM; i++) {
            if (strcmp(orders[i].item, item) == 0) {
                orders[i].quantity += quantity;
                break;
            }
        }
    }

    // send order status
    int offset = 0;
    for (int i = 0; i < RESTAURANT_MENU_NUM; i++) {
        if (orders[i].quantity == 0) continue;
        int n = snprintf(response + offset, sizeof(response) - offset, "%s%s %d", 
                         (i > 0 && orders[i - 1].quantity) ? "|" : "",
                         orders[i].item, orders[i].quantity);
        if (n < 0) {
            perror("error writing to buffer");
            return -1;
        }
        offset += n;
    }
    if (snprintf(response + offset, sizeof(response) - offset, "\n") < 0) {
        perror("error writing to buffer");
        return -1;
    }
    if (send(client_sockfd, response, BUFFER_SIZE, 0) < 0) {
        perror("error sending message");
        return -1;
    }

    return 1;
}

void clear_orders(Order *orders, int *current_restaurant) {
    memset(orders, 0, sizeof(Order) * RESTAURANT_MENU_NUM);
    *current_restaurant = -1;
}

void handle_confirm(int client_sockfd, Order *orders, int current_restaurant) {
    char response[BUFFER_SIZE];

    // calculate total price
    int total_price = 0;
    for (int i = 0; i < RESTAURANT_MENU_NUM; i++) {
        total_price += restaurants[current_restaurant].menu[i].price * orders[i].quantity;
    }

    if (total_price == 0) {
        if (snprintf(response, BUFFER_SIZE, "Please order some meals\n") < 0) {
            perror("error writing to buffer");
            exit(1);
        }
        send(client_sockfd, response, BUFFER_SIZE, 0);
        return;
    }

    // send waiting message
    if (snprintf(response, BUFFER_SIZE, "Please wait a few minutes...\n") < 0) {
        perror("error writing to buffer");
        exit(1);
    }
    if (send(client_sockfd, response, BUFFER_SIZE, 0) < 0) {
        perror("error sending message");
        exit(1);
    }

    // wait based on restaurant distance
    sleep(restaurants[current_restaurant].distance);

    // send delivery completed message
    if (snprintf(response, BUFFER_SIZE, "Delivery has arrived and you need to pay %d$\n", total_price) < 0) {
        perror("error writing to buffer");
        exit(1);
    }

    if (send(client_sockfd, response, BUFFER_SIZE, 0) < 0) {
        perror("error sending message");
        exit(1);
    }
}

#endif