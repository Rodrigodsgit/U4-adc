<table align="center">
  <tr>
    <td><img src="image.png" width="100"></td>
    <td align="center">
      <b>Embarca Tech - CEPEDI</b><br>
      Capacitação Inicial em Sistemas Embarcados<br>
      U4 - Microcontroladores <br>
      <b>Discente - Rodrigo Damasceno Sampaio (Grupo 9)</b>
    </td>
    <td><img src="image2.png" width="100"></td>
  </tr>
</table>

## 🌟 Projeto: Controle de LEDs PWM e Display OLED com Joystick
Este projeto integra um display OLED SSD1306 e LEDs RGB na placa BitDogLab (RP2040), utilizando um joystick e botões físicos. O sistema permite:

- Ajustar a intensidade dos LEDs Vermelho e Azul via PWM, de acordo com os valores lidos dos eixos X e Y do joystick.
- Exibir a posição do joystick no display OLED por meio de um quadrado móvel de 8x8 pixels.
- Alternar o estado do LED Verde e modificar o estilo da borda do display com o botão do joystick.
- Ativar ou desativar os LEDs PWM com o Botão A e reiniciar o dispositivo com o Botão B.

## 🎯 Objetivos

✅ Compreender o funcionamento do conversor analógico-digital (ADC) no RP2040.  
✅ Utilizar PWM para controle de intensidade dos LEDs.  
✅ Integrar um display OLED SSD1306 via protocolo I2C para exibir informações dinâmicas.  
✅ Implementar interrupções com debouncing para os botões, garantindo uma operação sem bloqueios.

## 📚 Descrição do Projeto
O projeto utiliza a placa BitDogLab (RP2040) para integrar múltiplos componentes:

**Joystick:**  
- Fornece valores analógicos para os eixos X e Y, os quais são usados para:
  - Controlar a intensidade do LED Vermelho (eixo X) e do LED Azul (eixo Y) via PWM.
  - Mover um quadrado de 8x8 pixels no display OLED, representando a posição do joystick.

**LEDs RGB:**  
- **LED Vermelho:** Intensidade ajustada pelo valor do eixo X do joystick.  
- **LED Azul:** Intensidade ajustada pelo valor do eixo Y do joystick.  
- **LED Verde:** Alterna estado (ligado/desligado) a cada pressionamento do botão do joystick.

**Botões:**  
- **Botão A (GPIO 5):** Ativa ou desativa o controle PWM dos LEDs Vermelho e Azul.  
- **Botão B (GPIO 6):** Aciona o modo Bootsel para reiniciar o dispositivo.  
- **Botão do Joystick (GPIO 22):** Alterna o estado do LED Verde e altera o estilo da borda do display OLED.

**Display OLED (SSD1306):**  
- Exibe um quadrado móvel que se desloca proporcionalmente aos valores capturados do joystick, com uma borda que alterna de estilo conforme o acionamento do botão do joystick.

| 🛠 Componentes e Conexões          | GPIO        |
|------------------------------------|-------------|
| 🔴 LED Vermelho                    | 13          |
| 🔵 LED Azul                        | 12          |
| 🟢 LED Verde                       | 11          |
| 🔘 Botão A                         | 5           |
| 🔘 Botão B                         | 6           |
| 🔘 Botão do Joystick               | 22          |
| 🖥 Display OLED (SDA)               | 14          |
| 🖥 Display OLED (SCL)               | 15          |
| 🎮 Joystick (Eixo X e Eixo Y - ADC)  | 26 e 27     |

## 🚀 Como Rodar o Projeto

### 📌 **1️⃣ Clonar o Repositório**
```bash
git clone https://github.com/Rodrigodsgit/U4-adc.git
cd semaforo
```

### 📌 **2️⃣ Configurar o Ambiente**
Instale a extensão da Raspberry Pi Pico Project no VsCode  

### 📌 3️⃣ Compilar o Código
Através da extensão, use o acesso rapido e clique em "Compile Project"

### 📌 4️⃣ Rode o Código
Através da extensão, use o acesso rapido e clique em "Run Project (USB)"

## 🚀 Video Demonstração


[🎥 Assista ao vídeo](https://youtube.com/shorts/-TScRmN6z1A)
