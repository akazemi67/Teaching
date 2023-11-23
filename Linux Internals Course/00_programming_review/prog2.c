#include <stdio.h>

void win() {
    printf("\033[0;32m ** You hit the Jackpot!:-D **  \033[0m\n");
}


int main(){
    char str[40];
    puts("Give me some input: ");
    gets(str);
    puts("Bye bye!");
    printf("By the way, main return address is: %p\n", __builtin_return_address(0));
    return 0;
}

