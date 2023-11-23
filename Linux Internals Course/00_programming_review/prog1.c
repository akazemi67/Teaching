#include <stdio.h>

void win() {
    printf("\033[0;32m ** You hit the Jackpot!:-D **  \033[0m\n");
}

void change_value(unsigned int *arr, int idx, int value){
    arr[idx] = value;
}

int main(){
    int arr[100];
    int idx, value;
    arr[0] = 0xdeadbeef;

    printf("Let's do some simple array assignments.\nGive me the index: ");
    scanf("%d", &idx);
    printf("Give me the value: ");
    scanf("%d", &value);

    change_value(arr, idx, value);
    printf("Value at %p changed!\n", &arr[idx]);
    return 0;
}

