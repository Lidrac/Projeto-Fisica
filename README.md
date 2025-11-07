# Simulação Interativa do Efeito Hall em C++/SFML

Este projeto é um "laboratório virtual" programado em C++ com a biblioteca gráfica SFML, que demonstra de forma interativa os princípios físicos do Efeito Hall.

O objetivo principal é visualizar as forças invisíveis (Magnética e Elétrica) que atuam sobre os portadores de carga e entender como o sistema busca um estado de equilíbrio, resultando na Tensão Hall.

## 🚀 Funcionalidades

- **Simulação em Tempo Real:** Observe a trajetória de um portador de carga (elétron ou lacuna) ser desviada por um campo magnético.
- **Visualização de Vetores:**
  - **Campo Magnético (B):** Representado por 'X' no condutor (entrando na tela).
  - **Corrente Convencional (I):** Seta amarela que inverte sua direção com base no portador de carga.
  - **Força Magnética (Fm):** Seta vermelha que mostra a força de deflexão.
  - **Força Elétrica (Fe):** Seta azul que mostra a força de oposição gerada pela Tensão Hall.
- **Controles Interativos:**
  - **Pausar/Rodar:** A barra de `Espaço` congela a simulação para análise.
  - **Ajuste de Parâmetros:** Aumente ou diminua o Campo Magnético (Setas Cima/Baixo) e a Corrente (Setas Esquerda/Direita).
  - **Troca de Portador:** A tecla `S` alterna entre Elétrons (negativos) e Lacunas (positivos), demonstrando a inversão da deflexão.
- **Análise de Dados:** O painel inferior exibe os cálculos de deslocamento (X e Y) da última "corrida" da partícula.

## 🛠️ Pré-requisitos e Instalação

Este projeto foi desenvolvido e testado no **Windows 10/11** com **VS Code**, **MinGW-w64 (g++)** e **SFML 3.0**.

### 1. Compilador

É necessário um compilador C++ de 64-bit. Recomendamos o **MSYS2** para instalar a versão mais recente do MinGW-w64 (g++):

1.  Instale o [MSYS2](https://www.msys2.org/).
2.  No terminal do MSYS2, instale a toolchain do g++:
    ```bash
    pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain
    ```
3.  Adicione o compilador ao seu PATH do Windows (ex: `C:\msys64\ucrt64\bin`).

### 2. Biblioteca SFML

1.  Baixe a versão **SFML 3.0** (ou mais recente) para **GCC (UCRT) - 64-bit** do [site oficial](https://www.sfml-dev.org/download.php).
2.  Descompacte a biblioteca.

### 3. Configuração do Projeto

1.  Clone ou baixe este repositório.
2.  Dentro da pasta do projeto, crie uma subpasta chamada `libs`.
3.  Mova a pasta descompactada da SFML (ex: `SFML-3.0.0`) para dentro de `libs`.
4.  Copie todos os arquivos `.dll` da pasta `SFML-3.0.0/bin` para a **pasta raiz** do seu projeto (onde o `main.exe` será criado).
5.  Obtenha um arquivo de fonte (ex: `arial.ttf` de `C:\Windows\Fonts`), copie-o para a pasta raiz do projeto e renomeie-o para `font.ttf`.

## ⚙️ Como Compilar e Executar (VS Code)

1.  Abra a pasta do projeto no VS Code.
2.  Certifique-se de que a extensão C/C++ da Microsoft está instalada.
3.  O projeto já contém uma pasta `.vscode` com os arquivos `tasks.json` e `launch.json` configurados.
4.  Pressione `Ctrl+Shift+B` para compilar o projeto.
5.  Pressione `F5` para executar a simulação.

## 🕹️ Controles da Simulação

- **Setas Cima/Baixo:** Aumenta/Diminui a intensidade do Campo Magnético (B).
- **Setas Esquerda/Direita:** Aumenta/Diminui a Corrente (I) / Velocidade da Partícula.
- **Tecla `S`:** Alterna entre Elétrons (azul) e Lacunas (vermelho).
- **Tecla `R`:** Reseta a simulação (zera a Tensão Hall e os portadores).
- **Barra de Espaço:** Pausa ou retoma a simulação.

