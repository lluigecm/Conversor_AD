#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

// Definições dos LEDs e Botões
#define RED_LED 13
#define GREEN_LED 11
#define BLUE_LED 12
#define BUTTON_A 5
#define JOYSTICK_BUTTON 22

// Definições do Joystick
#define JOYSTICK_Y 26
#define JOYSTICK_X 27
#define DEADZONE 250    // Zona morta do Joystick

#define DEBOUNCE 200    // Debounce de 200ms

// Definições do Display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15      // Definições I2C (DISPLAY OLED - SSD1306)
#define ADDRESS 0x3C

#define WRAP 4096       // Definição do valor de WRAP para o PWM
#define MAX_ADC 4095    // Valor máximo do ADC
#define SQUARE_SIZE 8   // Tamanho do quadrado  

ssd1306_t ssd;          // Inicialização da estrutura do display OLED

bool cor = true;
bool border_change =true;   // Variável para controle da borda do display
bool joystick_leds = true;  // Variável para controle dos LEDs do Joystick

uint last_time = 0;     // Variável para debounce

uint16_t joystick_center = MAX_ADC/2; // Centro do Joystick
uint16_t x_loc = 60;// ----|
uint16_t y_loc = 28;//     |Localização inicial do quadrado

volatile uint16_t joy_x, joy_y; // Variáveis para armazenar os valores do Joystick

void init_gpio_pwm(uint gpio){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, WRAP);
    pwm_set_enabled(slice_num, true);
}

uint16_t adc_to_pwm(uint16_t adc_value){ // Converte o valor do ADC para o PWM considerando a dead zone do joystick
    
    // se estiver dentro da deadzone, retorna zero -> Não movimenta o quadrado
    if(adc_value > joystick_center - DEADZONE && adc_value < joystick_center + DEADZONE)
        return 0;
    // Retorna o PWM para regiao menor que o centro -> Movimenta do quadrado para o lado esquerdo
    if(adc_value < joystick_center - DEADZONE)
        return (MAX_ADC - (adc_value * 2));
    // Retorna o PWM para regiao maior que o centro -> Movimenta do quadrado para o lado direito
    if (adc_value > joystick_center + DEADZONE)
        return (adc_value - (joystick_center + DEADZONE)) * 2;
}

void setup(){
//-------------------------------------------------------|
    init_gpio_pwm(RED_LED);//                            |
    init_gpio_pwm(BLUE_LED);//                           |
//                                                       |
    gpio_init(GREEN_LED);//                              |
    gpio_set_dir(GREEN_LED, GPIO_OUT);//                 | 
//                                                       |
    gpio_init(JOYSTICK_BUTTON);//                        |
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);//            |  Configuração dos LEDs, Botões e Joystick
    gpio_pull_up(JOYSTICK_BUTTON);//                     |
//                                                       |
    gpio_init(BUTTON_A);//                               |
    gpio_set_dir(BUTTON_A, GPIO_IN);//                   |
    gpio_pull_up(BUTTON_A);//                            |
//                                                       |
    adc_init();//                                        |
    adc_gpio_init(JOYSTICK_X);//                         |
    adc_gpio_init(JOYSTICK_Y);//                         |
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
    ssd1306_fill(&ssd, !cor); // Limpa o display---------------------------------------|
    ssd1306_rect(&ssd, 0, 0, WIDTH-1, HEIGHT-1, cor, false); //                        |
    ssd1306_draw_string(&ssd, "EMBARCATECH", 20, 10);//                                |
    ssd1306_draw_string(&ssd, "Tarefa", 20, 25);//                                     |    Configuração inicial do display
    ssd1306_draw_string(&ssd, "Capitulo 8", 20, 40);//                                 |
    ssd1306_send_data(&ssd); // Envia os dados para o display--------------------------|
}

void update_border(){
    if(border_change){
        ssd1306_rect(&ssd, 0, 0, WIDTH-1, HEIGHT-1, !cor, false); // Limpa a borda do display
    }else{
        ssd1306_rect(&ssd, 0, 0, WIDTH-1, HEIGHT-1, cor, false); // Desenha a borda do display
    }
    cor = !cor;
    ssd1306_send_data(&ssd); // Envia os dados para o display
}

bool debounce(){
    uint time = to_ms_since_boot(get_absolute_time());
    if(time - last_time >= DEBOUNCE){      // Debounce de 200ms
        last_time = time;
        return true;
    }
    return false;
}

void buttons_handler(uint gpio, uint32_t events){
    if(debounce()){
        if(gpio == JOYSTICK_BUTTON){
            gpio_put(GREEN_LED, !gpio_get(GREEN_LED)); // Inverte o estado do LED verde
            update_border(); // Atualiza a borda do display
        }
        if(gpio == BUTTON_A){
            if(joystick_leds){
                pwm_set_gpio_level(RED_LED, adc_to_pwm(joy_x));
                pwm_set_gpio_level(BLUE_LED, adc_to_pwm(joy_y));
            }else{
                pwm_set_gpio_level(RED_LED, 0);
                pwm_set_gpio_level(BLUE_LED, 0); 
            }
            joystick_leds = !joystick_leds;
        }
    }
}


int main()
{
    stdio_init_all();
    setup();
    
    display_init_config();

    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &buttons_handler); // Habilita a interrupção no botão do Joystick
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &buttons_handler); // Habilita a interrupção no botão A

    while (true) {
        adc_select_input(0);
        joy_y = adc_read();
        adc_select_input(1);
        joy_x = adc_read();


        sleep_ms(50); // pausa entre iterações
    }
}
