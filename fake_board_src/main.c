#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

__attribute__((import_module("env"), import_name("host_stdout_print")))
extern void host_stdout_print(const char * msg);

__attribute__((import_module("env"), import_name("new_host_stdout_print")))
extern void new_host_stdout_print(const char * msg);

volatile int x = 5;
volatile int y = 5;

volatile int led_value_raw = 12;

void led_value_i1(void)
{
    host_stdout_print("irq calling led_value_i1, take a long time execute: 6 secs");
    led_value_raw = 39;
    for(int i =0; i<6 * 1000; i++)
    {
        usleep(1000);
    }
}

void led_value_i2(void)
{
    host_stdout_print("irq calling led_value_i2, take a long time execute: 3 secs");
    led_value_raw = 42;
    for(int i =0; i<3 * 1000; i++)
    {
        usleep(1000);
    }
}

int led_value(void)
{
    return led_value_raw;
}

int new_led_value(void)
{
    return led_value_raw + 42;
}

void board_main(void)
{
    char buffer[256] = {0};

    new_led_value();
    snprintf(buffer, sizeof(buffer), "calling fun %s at addr: %p",
            "new_led_value", new_led_value);
    host_stdout_print(buffer);

    new_host_stdout_print("keep alive");

    while(1)
    {
        snprintf(buffer, sizeof(buffer), "led value (fun addr: %p) is: %d",
                led_value, led_value());
        host_stdout_print(buffer);
        usleep(200 * 1000);
    }
}
