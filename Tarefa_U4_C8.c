#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

#define RED_LED 13
#define GREEN_LED 11
#define BLUE_LED 12

#define JOYSTICK_BUTTON 25
#define JOYSTICK_X 26
#define JOYSTICK_Y 27

#define BUTTON_A 5

#define DEBOUNCE 200

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15      // Definições I2C (DISPLAY OLED - SSD1306)
#define ADDRESS 0x3C

ssd1306_t ssd;          // Inicialização da estrutura do display OLED
bool cor = true;
uint last_time = 0;

void setup(){
    gpio_init(RED_LED);//--------------------------------|
    gpio_set_dir(RED_LED, GPIO_OUT);//                   |
//                                                       |
//                                                       | Configuração dos LEDs
    gpio_init(BLUE_LED);//                               |
    gpio_set_dir(BLUE_LED, GPIO_OUT);//                  |  
//-------------------------------------------------------|

    i2c_init(I2C_PORT, 400*1000); // Inicializa o barramento I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Define os pinos SDA e SCL
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA); // Habilita os pull-ups
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADDRESS, I2C_PORT); // Inicializa o display OLED
    ssd1306_config(&ssd);   // Configura o display OLED
    ssd1306_send_data(&ssd); // Envia os dados para o display
    
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd);   // Envia os dados para o display
}

void display_init_config(){
    ssd1306_fill(&ssd, cor); // Limpa o display----------------------------------------|
    ssd1306_rect(&ssd, 3, 3, 122, 58, !cor, cor); // Desenha um retângulo              |
    ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 10);//                                |
    ssd1306_draw_string(&ssd, "Tarefa", 20, 25);//                                      |    Configuração inicial do display
    ssd1306_draw_string(&ssd, "Capitulo 8", 20, 40);//                                   |
    ssd1306_send_data(&ssd); // Envia os dados para o display--------------------------|
}

bool debounce(){
    uint time = to_ms_since_boot(get_absolute_time());
    if(time - last_time > DEBOUNCE){      // Debounce de 200ms
        last_time = time;
        return true;
    }
    return false;
}


int main()
{
    stdio_init_all();
    setup();
    display_init_config();

    sleep_ms(2000);

    while (true) {
    }
}
