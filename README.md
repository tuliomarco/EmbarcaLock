# EmbarcaLock: Sistema de Fechadura Digital com RP2040

## Descrição
O **EmbarcaLock** é um sistema de fechadura digital baseado no microcontrolador **RP2040**, projetado para oferecer segurança por meio de autenticação com senha. O sistema permite que o usuário cadastre, altere e valide uma senha armazenada na memória flash do microcontrolador. A interface de interação inclui um display OLED, botões, joystick e LEDs indicadores, proporcionando uma experiência intuitiva para o usuário.

## Funcionalidades Principais
- **Gerenciamento de Senha**: Cadastro, alteração e validação da senha armazenada na memória flash.
- **Interface de Usuário**: Controle por meio de um **joystick** e **botões físicos**.
- **Feedback Visual e Sonoro**: Indicação de estados do sistema por meio de um **display OLED**, uma **matriz de LEDs 5x5** e **efeitos sonoros**.
- **Estados do Sistema**:
  - **LED Verde** aceso indica que a tranca está **desbloqueada**.
  - **LED Vermelho** aceso indica que está **trancado**.
  - **Buzzer** emite sinais sonoros para confirmar ações do usuário.
- **Animações Dinâmicas**: Efeitos visuais na matriz de LEDs para reforçar a experiência interativa.

## Tecnologias Utilizadas
- **Microcontrolador**: Raspberry Pi Pico W (**RP2040**).
- **Linguagem de Programação**: **C** (sem bibliotecas terceiras).
- **Exibição Visual**: Display OLED **128x64** via **I2C**.
- **Entrada do Usuário**: Joystick e botões físicos.
- **Feedback Sensorial**: LEDs RGB, matriz de LEDs WS2812B e buzzers.

## Como Executar
1. **Monte o hardware** conforme o diagrama do projeto, conectando os componentes ao Raspberry Pi Pico W.
2. **Compile o código** utilizando o SDK do Raspberry Pi Pico e grave no microcontrolador.
3. **Inicialize o sistema** e observe as indicações visuais e sonoras:
   - O display OLED exibirá as opções de interação.
   - O LED correspondente indicará o estado da tranca.
4. **Interaja com o sistema**:
   - Utilize o **joystick** e os **botões** para navegar pelos menus e inserir a senha.
   - O sistema validará a entrada e alterará o estado da tranca conforme necessário.

## Estrutura do Código
O código está dividido em módulos para melhor organização e manutenção:

- **`embarca_lock`**: Módulo principal que gerencia a lógica do sistema.
- **`password_manager`**: Responsável pelo armazenamento, exclusão e validação da existência da senha.
- **`user_choose`**: Lida com a interação do usuário e navegação pelos menus.
- **`display_manager`**: Gerencia a exibição de informações no display OLED.
- **`matrix_animation`**: Controla os efeitos visuais na matriz de LEDs.
- **`sound_manager`**: Gera efeitos sonoros para feedback do usuário.

## Demonstração
Confira o funcionamento do projeto no seguinte [vídeo demonstrativo](https://drive.google.com/file/d/1nqWDOMA-obum3F-YeDDS868urNKg0rrV/view?usp=sharing).

---
